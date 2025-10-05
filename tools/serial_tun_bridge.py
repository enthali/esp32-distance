#!/usr/bin/env python3
"""
Serial-TUN Bridge for ESP32 QEMU IP Tunnel

Bridges IP packets between QEMU UART1 (TCP port 5556) and a Linux TUN device.
This provides full TCP/IP connectivity to the simulated ESP32.

Architecture:
- Reads IP frames from TCP socket (QEMU UART1)
- Writes frames to TUN device (Linux network stack)
- Bidirectional: TUN → Serial also works

Framing Protocol:
- [LENGTH:2 bytes big-endian][DATA:N bytes]
- Maximum frame size: 1500 bytes (MTU)

Usage:
    sudo ./serial_tun_bridge.py

Requirements:
    - Root privileges (for TUN device creation)
    - pytun or python-pytun package

Author: ESP32 Distance Project
Date: 2025
"""

import socket
import struct
import sys
import os
import select
import signal
import logging
import argparse

# Setup logging (will be configured based on command line arguments)
logger = logging.getLogger(__name__)

# Configuration
SERIAL_HOST = 'localhost'
SERIAL_PORT = 5556
TUN_NAME = 'tun0'
TUN_IP = '192.168.100.1'
TUN_NETMASK = '255.255.255.0'
ESP32_IP = '192.168.100.2'
MAX_FRAME_SIZE = 1500

# Ethernet Header Constants (for lwIP compatibility)
# ESP32 lwIP expects Ethernet frames, but TUN gives us raw IP packets
ETH_HEADER_SIZE = 14
HOST_MAC = bytes([0x02, 0x00, 0x00, 0x00, 0x00, 0x01])  # Fake MAC for host
ESP32_MAC = bytes([0x02, 0x00, 0x00, 0x00, 0x00, 0x02])  # Fake MAC for ESP32
ETH_TYPE_IP = 0x0800  # EtherType for IPv4

# TUN device handling
try:
    from pytun import TunTapDevice, IFF_TUN, IFF_NO_PI
    HAVE_PYTUN = True
except ImportError:
    HAVE_PYTUN = False
    logger.warning("pytun not available, using manual TUN device creation")

class SerialTunBridge:
    def __init__(self):
        self.serial_sock = None
        self.tun = None
        self.running = False

    def create_tun_device_manual(self):
        """Create TUN device manually using ioctl and system commands"""
        import subprocess
        import fcntl
        import struct
        
        logger.info(f"Creating TUN device {TUN_NAME} manually...")
        
        # Open /dev/net/tun
        try:
            tun_fd = os.open('/dev/net/tun', os.O_RDWR)
        except Exception as e:
            logger.error(f"Failed to open /dev/net/tun: {e}")
            return None
        
        # Configure TUN device using ioctl
        IFF_TUN = 0x0001
        IFF_NO_PI = 0x1000
        TUNSETIFF = 0x400454ca
        
        try:
            ifr = struct.pack('16sH', TUN_NAME.encode(), IFF_TUN | IFF_NO_PI)
            fcntl.ioctl(tun_fd, TUNSETIFF, ifr)
        except Exception as e:
            logger.error(f"Failed to configure TUN device: {e}")
            os.close(tun_fd)
            return None
        
        # Set IP address and bring up
        ret = subprocess.call(['ip', 'addr', 'add', f'{TUN_IP}/24', 'dev', TUN_NAME])
        if ret != 0:
            logger.warning(f"Failed to set IP address (may already exist)")
        
        ret = subprocess.call(['ip', 'link', 'set', TUN_NAME, 'up'])
        if ret != 0:
            logger.error(f"Failed to bring up TUN device")
            os.close(tun_fd)
            return None
        
        logger.info(f"TUN device {TUN_NAME} created: {TUN_IP}/24")
        return tun_fd

    def create_tun_device(self):
        """Create and configure TUN device"""
        if HAVE_PYTUN:
            try:
                self.tun = TunTapDevice(name=TUN_NAME, flags=IFF_TUN | IFF_NO_PI)
                self.tun.addr = TUN_IP
                self.tun.netmask = TUN_NETMASK
                self.tun.mtu = MAX_FRAME_SIZE
                self.tun.up()
                logger.info(f"TUN device {TUN_NAME} created: {TUN_IP}/{TUN_NETMASK}")
                return self.tun
            except Exception as e:
                logger.error(f"Failed to create TUN device with pytun: {e}")
                return None
        else:
            # Manual TUN creation
            tun_fd = self.create_tun_device_manual()
            if tun_fd is None:
                return None
            
            # Wrap in file object
            class TunDevice:
                def __init__(self, fd):
                    self.fd = fd
                    self.file = os.fdopen(fd, 'r+b', buffering=0)
                
                def read(self, size):
                    return os.read(self.fd, size)
                
                def write(self, data):
                    return os.write(self.fd, data)
                
                def fileno(self):
                    return self.fd
                
                def close(self):
                    os.close(self.fd)
            
            self.tun = TunDevice(tun_fd)
            return self.tun

    def connect_to_serial(self):
        """Connect to QEMU UART TCP socket - retry forever"""
        logger.info(f"Connecting to QEMU UART at {SERIAL_HOST}:{SERIAL_PORT}...")
        
        attempt = 0
        while True:  # Retry forever
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect((SERIAL_HOST, SERIAL_PORT))
                logger.info(f"Connected to QEMU UART (after {attempt} attempts)")
                return sock
            except ConnectionRefusedError:
                attempt += 1
                if attempt == 1 or attempt % 10 == 0:  # Log first and every 10th attempt
                    logger.warning(f"Connection refused, retrying... (attempt {attempt})")
                import time
                time.sleep(1)
            except Exception as e:
                logger.error(f"Failed to connect to serial: {e}")
                import time
                time.sleep(1)

    def serial_to_tun(self):
        """Read Ethernet frame from serial, extract IP packet, write to TUN"""
        try:
            # Read frame length (2 bytes, big-endian) - MUST read exactly 2 bytes
            len_data = b''
            while len(len_data) < 2:
                chunk = self.serial_sock.recv(2 - len(len_data))
                if not chunk:
                    return False
                len_data += chunk
            
            frame_len = struct.unpack('>H', len_data)[0]
            
            if frame_len == 0 or frame_len > MAX_FRAME_SIZE + ETH_HEADER_SIZE:
                logger.warning(f"Invalid frame length: {frame_len}")
                return True
            
            # Read Ethernet frame
            eth_frame = b''
            while len(eth_frame) < frame_len:
                chunk = self.serial_sock.recv(frame_len - len(eth_frame))
                if not chunk:
                    return False
                eth_frame += chunk
            
            logger.info(f"Serial→TUN: Received {frame_len} bytes Ethernet frame")
            
            # Strip Ethernet header (14 bytes) to get IP packet
            if frame_len < ETH_HEADER_SIZE:
                logger.warning(f"Frame too short for Ethernet header: {frame_len}")
                return True
                
            ip_packet = eth_frame[ETH_HEADER_SIZE:]
            ip_len = len(ip_packet)
            
            # Log packet info with protocol names
            src_ip = "?"
            dst_ip = "?"
            protocol = "?"
            proto_name = "?"
            if ip_len >= 20:  # Minimum IP header
                try:
                    version = (ip_packet[0] >> 4) & 0xF
                    if version == 4:
                        src_ip = ".".join(str(b) for b in ip_packet[12:16])
                        dst_ip = ".".join(str(b) for b in ip_packet[16:20])
                        protocol = ip_packet[9]
                        # Map protocol numbers to names
                        proto_map = {1: "ICMP", 6: "TCP", 17: "UDP"}
                        proto_name = proto_map.get(protocol, f"Proto{protocol}")
                        proto_map = {1: "ICMP", 6: "TCP", 17: "UDP"}
                        proto_name = proto_map.get(protocol, f"Proto{protocol}")
                except:
                    pass
            
            logger.info(f"Serial→TUN: {ip_len} bytes IP, {proto_name} (proto={protocol}), {src_ip}→{dst_ip}")
            
            # Write IP packet to TUN
            self.tun.write(ip_packet)
            
            return True
            
        except Exception as e:
            logger.error(f"Error in serial_to_tun: {e}")
            return False

    def tun_to_serial(self):
        """Read IP packet from TUN, add Ethernet header, write to serial"""
        try:
            # Read from TUN (raw IP packet)
            ip_packet = self.tun.read(MAX_FRAME_SIZE)
            
            if not ip_packet:
                return True
            
            ip_len = len(ip_packet)
            
            # Log packet info with protocol names
            src_ip = "?"
            dst_ip = "?"
            protocol = "?"
            proto_name = "?"
            if ip_len >= 20:  # Minimum IP header
                try:
                    version = (ip_packet[0] >> 4) & 0xF
                    if version == 4:
                        src_ip = ".".join(str(b) for b in ip_packet[12:16])
                        dst_ip = ".".join(str(b) for b in ip_packet[16:20])
                        protocol = ip_packet[9]
                        # Map protocol numbers to names
                        proto_map = {1: "ICMP", 6: "TCP", 17: "UDP"}
                        proto_name = proto_map.get(protocol, f"Proto{protocol}")
                except:
                    pass
            
            logger.info(f"TUN→Serial: {ip_len} bytes IP, {proto_name} (proto={protocol}), {src_ip}→{dst_ip}")
            
            # Build Ethernet frame: [Dest MAC][Src MAC][EtherType][IP Packet]
            eth_header = ESP32_MAC + HOST_MAC + struct.pack('>H', ETH_TYPE_IP)
            eth_frame = eth_header + ip_packet
            frame_len = len(eth_frame)
            
            logger.info(f"TUN→Serial: Added Ethernet header, total {frame_len} bytes")
            
            # Send length header (big-endian)
            len_data = struct.pack('>H', frame_len)
            self.serial_sock.sendall(len_data)
            
            # Send Ethernet frame
            self.serial_sock.sendall(eth_frame)
            
            return True
            
        except Exception as e:
            logger.error(f"Error in tun_to_serial: {e}")
            return False

    def cleanup(self):
        """Cleanup resources"""
        logger.info("Cleaning up...")
        
        self.running = False
        
        if self.serial_sock:
            self.serial_sock.close()
            self.serial_sock = None
        
        if self.tun:
            if HAVE_PYTUN:
                self.tun.down()
                self.tun.close()
            else:
                self.tun.close()
                import subprocess
                subprocess.call(['ip', 'tuntap', 'del', 'dev', TUN_NAME, 'mode', 'tun'])
            self.tun = None

    def run(self):
        """Main bridge loop with automatic reconnection"""
        # Setup signal handlers
        signal.signal(signal.SIGINT, lambda s, f: self.cleanup())
        signal.signal(signal.SIGTERM, lambda s, f: self.cleanup())
        
        # Create TUN device (only once)
        if not self.create_tun_device():
            logger.error("Failed to create TUN device")
            return 1
        
        self.running = True
        
        # Main loop with reconnection
        while self.running:
            # Connect (or reconnect) to serial
            self.serial_sock = self.connect_to_serial()
            if not self.serial_sock:
                logger.error("Failed to connect to serial, retrying...")
                import time
                time.sleep(2)
                continue
            
            logger.info("Bridge active. Press Ctrl+C to stop.")
            logger.info(f"ESP32 should be accessible at {ESP32_IP}")
            
            # Bridge loop for this connection
            while self.running:
                # Wait for data on either serial or TUN
                rlist = [self.serial_sock, self.tun]
                readable, _, _ = select.select(rlist, [], [], 1.0)
                
                for r in readable:
                    if r == self.serial_sock:
                        if not self.serial_to_tun():
                            logger.warning("Serial connection lost, reconnecting...")
                            try:
                                self.serial_sock.close()
                            except:
                                pass
                            self.serial_sock = None
                            break
                    
                    elif r == self.tun:
                        if not self.tun_to_serial():
                            logger.error("TUN device error")
                            self.running = False
                            break
                
                # If serial_sock is None, break inner loop to reconnect
                if self.serial_sock is None:
                    break
        
        self.cleanup()
        return 0

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(
        description='Serial-TUN Bridge for ESP32 QEMU IP Tunnel',
        epilog='Note: This script requires root privileges to create TUN device'
    )
    parser.add_argument(
        '--quiet', '-q',
        action='store_true',
        help='Quiet mode: only log errors (to /workspaces/esp32-distance/temp/tun_errors.log)'
    )
    parser.add_argument(
        '--port', '-p',
        type=int,
        default=SERIAL_PORT,
        help=f'QEMU UART TCP port (default: {SERIAL_PORT})'
    )
    args = parser.parse_args()
    
    # Configure logging based on quiet mode
    if args.quiet:
        # In quiet mode: only errors to file
        error_log = '/workspaces/esp32-distance/temp/tun_errors.log'
        os.makedirs(os.path.dirname(error_log), exist_ok=True)
        logging.basicConfig(
            level=logging.ERROR,
            format='%(asctime)s - %(levelname)s - %(message)s',
            filename=error_log,
            filemode='a'
        )
    else:
        # Verbose mode: info to console
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s'
        )
    
    # Check for root
    if os.geteuid() != 0:
        logger.error("This script must be run as root (for TUN device)")
        if not args.quiet:
            print("Error: This script must be run as root (for TUN device)", file=sys.stderr)
            print("Try: sudo ./serial_tun_bridge.py", file=sys.stderr)
        return 1
    
    if not args.quiet:
        print(f"Starting TUN bridge (port {args.port})...")
        print(f"TUN device: {TUN_NAME} ({TUN_IP})")
        print(f"ESP32 IP: {ESP32_IP}")
        print("Press Ctrl+C to stop\n")
    
    bridge = SerialTunBridge()
    return bridge.run()

if __name__ == '__main__':
    sys.exit(main())
