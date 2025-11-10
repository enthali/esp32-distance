#!/usr/bin/env python3
"""
HTTP Proxy with Retry Logic for ESP32 QEMU Network Tunnel

Forwards localhost:8080 -> 192.168.100.2:80
Allows VS Code Simple Browser to access ESP32 webserver via GitHub port forwarding.

Features:
- Automatic retry with exponential backoff
- Handles ESP32/QEMU restarts gracefully
- Quiet mode (--quiet): Only logs errors
- Errors logged to temp/proxy_errors.log
"""
import http.server
import urllib.request
import urllib.error
import socketserver
import time
import sys
import os
from datetime import datetime
from pathlib import Path

PORT = 8080
ESP32_URL = "http://192.168.100.2"
MAX_RETRIES = 5
INITIAL_BACKOFF = 0.5  # seconds

# Parse command line arguments
QUIET_MODE = '--quiet' in sys.argv

# Determine project directory dynamically (three levels up from tools/qemu/network/)
SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_DIR = SCRIPT_DIR.parent.parent.parent
ERROR_LOG = PROJECT_DIR / "temp" / "proxy_errors.log"

def log_error(message):
    """Log errors to file and optionally to console"""
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    log_message = f"[{timestamp}] {message}\n"
    
    # Always log to file
    ERROR_LOG.parent.mkdir(parents=True, exist_ok=True)
    with open(ERROR_LOG, 'a') as f:
        f.write(log_message)
    
    # Also print if not in quiet mode
    if not QUIET_MODE:
        print(message, file=sys.stderr)

def log_info(message):
    """Log info messages (only in verbose mode)"""
    if not QUIET_MODE:
        print(message)

class ProxyHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        target_url = ESP32_URL + self.path
        
        # Try to fetch from ESP32 with retry logic
        retry_count = 0
        backoff = INITIAL_BACKOFF
        last_error = None
        
        while retry_count <= MAX_RETRIES:
            try:
                with urllib.request.urlopen(target_url, timeout=10) as response:
                    # Success! Forward the response
                    self.send_response(response.status)
                    for header, value in response.headers.items():
                        self.send_header(header, value)
                    self.end_headers()
                    self.wfile.write(response.read())
                    
                    if not QUIET_MODE:
                        log_info(f"✓ {self.path} -> {response.status}")
                    return
                    
            except urllib.error.URLError as e:
                last_error = e
                if retry_count < MAX_RETRIES:
                    if not QUIET_MODE:
                        log_info(f"Retry {retry_count+1}/{MAX_RETRIES} for {self.path} (waiting {backoff:.1f}s)...")
                    time.sleep(backoff)
                    backoff *= 2  # Exponential backoff
                    retry_count += 1
                else:
                    # Max retries reached, return error
                    error_msg = f"Proxy error after {MAX_RETRIES} retries: {e}"
                    log_error(error_msg)
                    try:
                        self.send_error(502, f"ESP32 unreachable: {e}")
                    except (BrokenPipeError, ConnectionResetError):
                        # Client disconnected, ignore
                        pass
                    return
                    
            except Exception as e:
                error_msg = f"Unexpected proxy error: {e}"
                log_error(error_msg)
                try:
                    self.send_error(502, f"Proxy Error: {e}")
                except (BrokenPipeError, ConnectionResetError):
                    # Client disconnected, ignore
                    pass
                return
    
    def do_POST(self):
        """Handle POST requests (for configuration API) - Direct TCP forwarding"""
        import socket
        
        # Read POST body from client
        content_length = int(self.headers.get('Content-Length', 0))
        post_data = self.rfile.read(content_length) if content_length > 0 else b''
        
        retry_count = 0
        backoff = INITIAL_BACKOFF
        
        while retry_count <= MAX_RETRIES:
            try:
                # Create raw HTTP request manually for ESP32
                request_line = f"POST {self.path} HTTP/1.0\r\n"
                headers = f"Host: 192.168.100.2\r\n"
                headers += f"Content-Type: {self.headers.get('Content-Type', 'application/json')}\r\n"
                headers += f"Content-Length: {len(post_data)}\r\n"
                headers += f"Connection: close\r\n"
                headers += "\r\n"
                
                http_request = request_line.encode('utf-8') + headers.encode('utf-8') + post_data
                
                # Connect directly to ESP32
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(10)
                sock.connect(('192.168.100.2', 80))
                sock.sendall(http_request)
                
                # Read response headers first
                response_data = b''
                header_complete = False
                while not header_complete:
                    chunk = sock.recv(1024)
                    if not chunk:
                        raise Exception("Connection closed before headers received")
                    response_data += chunk
                    if b'\r\n\r\n' in response_data:
                        header_complete = True
                
                # Parse headers to get Content-Length
                # Find header/body separator in bytes
                header_sep = b'\r\n\r\n'
                header_end_idx = response_data.find(header_sep)
                if header_end_idx == -1:
                    raise Exception("Invalid HTTP response: no header separator found")
                
                # Split headers and initial body data (both as bytes)
                response_headers_bytes = response_data[:header_end_idx]
                response_body_start = response_data[header_end_idx + len(header_sep):]
                
                # Decode headers to string for parsing
                response_headers = response_headers_bytes.decode('utf-8', errors='ignore')
                
                # Extract status code
                status_line = response_headers.split('\r\n')[0]
                status_code = int(status_line.split()[1])
                
                # Extract Content-Length if present
                content_length = None
                for line in response_headers.split('\r\n')[1:]:
                    if line.lower().startswith('content-length:'):
                        content_length = int(line.split(':', 1)[1].strip())
                        break
                
                # Read remaining body based on Content-Length (work with bytes)
                body_data = response_body_start
                if content_length is not None:
                    # We know exactly how much to read
                    while len(body_data) < content_length:
                        remaining = content_length - len(body_data)
                        chunk = sock.recv(min(remaining, 4096))
                        if not chunk:
                            break
                        body_data += chunk
                    response_body = body_data[:content_length].decode('utf-8', errors='ignore')
                else:
                    # No Content-Length, read until connection closes (old behavior)
                    while True:
                        chunk = sock.recv(4096)
                        if not chunk:
                            break
                        body_data += chunk
                    response_body = body_data.decode('utf-8', errors='ignore')
                
                sock.close()
                
                # Forward response to client
                self.send_response(status_code)
                for line in response_headers.split('\r\n')[1:]:
                    if ':' in line:
                        key, value = line.split(':', 1)
                        self.send_header(key.strip(), value.strip())
                self.end_headers()
                self.wfile.write(response_body.encode('utf-8'))
                
                if not QUIET_MODE:
                    log_info(f"✓ POST {self.path} -> {status_code}")
                return
                    
            except (socket.timeout, ConnectionRefusedError, OSError) as e:
                if retry_count < MAX_RETRIES:
                    if not QUIET_MODE:
                        log_info(f"Retry {retry_count+1}/{MAX_RETRIES} for POST {self.path} (waiting {backoff:.1f}s)...")
                    time.sleep(backoff)
                    backoff *= 2
                    retry_count += 1
                else:
                    error_msg = f"Proxy error after {MAX_RETRIES} retries: {e}"
                    log_error(error_msg)
                    try:
                        self.send_error(502, f"ESP32 unreachable: {e}")
                    except (BrokenPipeError, ConnectionResetError):
                        pass
                    return
                    
            except Exception as e:
                error_msg = f"Unexpected proxy error on POST: {e}"
                log_error(error_msg)
                try:
                    self.send_error(502, f"Proxy Error: {e}")
                except (BrokenPipeError, ConnectionResetError):
                    pass
                return
    
    def do_OPTIONS(self):
        """Handle OPTIONS requests (CORS preflight)"""
        target_url = ESP32_URL + self.path
        
        try:
            req = urllib.request.Request(target_url, method='OPTIONS')
            with urllib.request.urlopen(req, timeout=5) as response:
                self.send_response(response.status)
                for header, value in response.headers.items():
                    self.send_header(header, value)
                self.end_headers()
                
                if not QUIET_MODE:
                    log_info(f"✓ OPTIONS {self.path} -> {response.status}")
        except Exception as e:
            # Fallback: Send basic CORS headers
            self.send_response(200)
            self.send_header('Access-Control-Allow-Origin', '*')
            self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
            self.send_header('Access-Control-Allow-Headers', 'Content-Type')
            self.end_headers()
            if not QUIET_MODE:
                log_info(f"✓ OPTIONS {self.path} -> 200 (fallback)")
    
    def log_message(self, format, *args):
        """Override to respect quiet mode"""
        if not QUIET_MODE:
            print(f"{self.address_string()} - {format % args}")

if __name__ == "__main__":
    # Always log startup (even in quiet mode) for debugging
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    startup_msg = f"[{timestamp}] HTTP Proxy starting on http://localhost:{PORT} -> {ESP32_URL}"
    
    # Create log directory and write startup message
    ERROR_LOG.parent.mkdir(parents=True, exist_ok=True)
    with open(ERROR_LOG, 'w') as f:  # Overwrite on startup
        f.write(startup_msg + "\n")
    
    log_info(f"HTTP Proxy starting on http://localhost:{PORT}")
    log_info(f"Forwarding to ESP32 at {ESP32_URL}")
    if QUIET_MODE:
        log_info(f"Quiet mode: Errors logged to {ERROR_LOG}")
    log_info("Press Ctrl+C to stop\n")
    
    try:
        # Enable SO_REUSEADDR to allow immediate port reuse after restart
        socketserver.TCPServer.allow_reuse_address = True
        
        with socketserver.TCPServer(("", PORT), ProxyHandler) as httpd:
            # Log successful binding
            success_msg = f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Proxy successfully bound to port {PORT}"
            with open(ERROR_LOG, 'a') as f:
                f.write(success_msg + "\n")
            
            try:
                httpd.serve_forever()
            except KeyboardInterrupt:
                log_info("\nProxy stopped")
    except OSError as e:
        error_msg = f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] FATAL: Failed to bind to port {PORT}: {e}"
        with open(ERROR_LOG, 'a') as f:
            f.write(error_msg + "\n")
        log_error(f"Failed to start proxy: {e}")
        sys.exit(1)
