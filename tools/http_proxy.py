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

PORT = 8080
ESP32_URL = "http://192.168.100.2"
MAX_RETRIES = 5
INITIAL_BACKOFF = 0.5  # seconds

# Parse command line arguments
QUIET_MODE = '--quiet' in sys.argv
ERROR_LOG = "/workspaces/esp32-distance/temp/proxy_errors.log"

def log_error(message):
    """Log errors to file and optionally to console"""
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    log_message = f"[{timestamp}] {message}\n"
    
    # Always log to file
    os.makedirs(os.path.dirname(ERROR_LOG), exist_ok=True)
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
    
    def log_message(self, format, *args):
        """Override to respect quiet mode"""
        if not QUIET_MODE:
            print(f"{self.address_string()} - {format % args}")

if __name__ == "__main__":
    # Clear error log on startup
    if os.path.exists(ERROR_LOG):
        os.remove(ERROR_LOG)
    
    log_info(f"HTTP Proxy starting on http://localhost:{PORT}")
    log_info(f"Forwarding to ESP32 at {ESP32_URL}")
    if QUIET_MODE:
        log_info(f"Quiet mode: Errors logged to {ERROR_LOG}")
    log_info("Press Ctrl+C to stop\n")
    
    with socketserver.TCPServer(("", PORT), ProxyHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            log_info("\nProxy stopped")
