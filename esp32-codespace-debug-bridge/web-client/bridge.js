/**
 * ESP32 Debug Bridge - Web Client
 * 
 * Connects to ESP32 via Web Serial API and forwards data to Codespace via WebSocket
 */

class ESP32DebugBridge {
    constructor() {
        this.serialPort = null;
        this.webSocket = null;
        this.reader = null;
        this.writer = null;
        this.isConnected = false;
        
        this.initializeUI();
    }
    
    initializeUI() {
        // Get UI elements
        this.connectBtn = document.getElementById('connect-btn');
        this.disconnectBtn = document.getElementById('disconnect-btn');
        this.clearLogBtn = document.getElementById('clear-log');
        this.baudRateSelect = document.getElementById('baud-rate');
        
        this.serialStatus = document.getElementById('serial-status');
        this.websocketStatus = document.getElementById('websocket-status');
        this.dataFlow = document.getElementById('data-flow');
        this.logContainer = document.getElementById('log');
        
        // Bind event listeners
        this.connectBtn.addEventListener('click', () => this.connect());
        this.disconnectBtn.addEventListener('click', () => this.disconnect());
        this.clearLogBtn.addEventListener('click', () => this.clearLog());
        
        // Check Web Serial support
        if (!('serial' in navigator)) {
            this.log('❌ Web Serial API not supported. Please use Chrome or Edge browser.', 'error');
            this.connectBtn.disabled = true;
        }
        
        this.log('🌐 ESP32 Debug Bridge initialized');
    }
    
    async connect() {
        try {
            this.log('🔌 Starting connection...');
            
            // 1. Request and open serial port
            await this.connectSerial();
            
            // 2. Connect to WebSocket bridge
            await this.connectWebSocket();
            
            // 3. Start data forwarding
            this.startDataForwarding();
            
            this.isConnected = true;
            this.updateUI();
            this.log('✅ Bridge connected successfully!', 'success');
            
        } catch (error) {
            this.log(`❌ Connection failed: ${error.message}`, 'error');
            await this.disconnect();
        }
    }
    
    async connectSerial() {
        const baudRate = parseInt(this.baudRateSelect.value);
        
        this.log(`📡 Requesting serial port (${baudRate} baud)...`);
        
        // Request port selection from user
        this.serialPort = await navigator.serial.requestPort();
        
        // Open the port
        await this.serialPort.open({ 
            baudRate: baudRate,
            dataBits: 8,
            stopBits: 1,
            parity: 'none',
            flowControl: 'none'
        });
        
        this.serialStatus.textContent = 'Connected';
        this.serialStatus.className = 'status-value status-connected';
        
        this.log(`📡 Serial port opened (${baudRate} baud)`);
    }
    
    async connectWebSocket() {
        // Determine WebSocket URL - WebSocket server runs on port 8081, web client on 8082
        const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
        const hostname = location.hostname;
        const wsUrl = `${protocol}//${hostname.replace('-8082.', '-8081.')}/ws`;
        
        this.log(`🌐 Connecting to WebSocket: ${wsUrl}`);
        
        return new Promise((resolve, reject) => {
            this.webSocket = new WebSocket(wsUrl);
            this.webSocket.binaryType = 'arraybuffer';
            
            this.webSocket.onopen = () => {
                this.websocketStatus.textContent = 'Connected';
                this.websocketStatus.className = 'status-value status-connected';
                this.log('🌐 WebSocket connected');
                resolve();
            };
            
            this.webSocket.onerror = (error) => {
                this.log(`❌ WebSocket error: ${error}`, 'error');
                reject(new Error('WebSocket connection failed'));
            };
            
            this.webSocket.onclose = () => {
                this.websocketStatus.textContent = 'Disconnected';
                this.websocketStatus.className = 'status-value status-disconnected';
                this.log('🌐 WebSocket disconnected');
            };
            
            this.webSocket.onmessage = async (event) => {
                // Data from Codespace (GDB) -> ESP32
                const data = new Uint8Array(event.data);
                this.dataFlow.textContent = `GDB→ESP32: ${data.length}B`;
                
                if (this.writer) {
                    await this.writer.write(data);
                }
            };
            
            // Timeout after 5 seconds
            setTimeout(() => {
                if (this.webSocket.readyState !== WebSocket.OPEN) {
                    reject(new Error('WebSocket connection timeout'));
                }
            }, 5000);
        });
    }
    
    async startDataForwarding() {
        // Get streams
        this.reader = this.serialPort.readable.getReader();
        this.writer = this.serialPort.writable.getWriter();
        
        this.log('🔄 Starting data forwarding...');
        
        // Forward data from ESP32 -> Codespace (GDB)
        this.forwardSerialToWebSocket();
    }
    
    async forwardSerialToWebSocket() {
        try {
            while (this.reader && this.isConnected) {
                const { value, done } = await this.reader.read();
                
                if (done) {
                    this.log('📡 Serial stream ended');
                    break;
                }
                
                if (value && this.webSocket && this.webSocket.readyState === WebSocket.OPEN) {
                    this.webSocket.send(value);
                    this.dataFlow.textContent = `ESP32→GDB: ${value.length}B`;
                }
            }
        } catch (error) {
            if (this.isConnected) {
                this.log(`❌ Serial read error: ${error.message}`, 'error');
            }
        }
    }
    
    async disconnect() {
        this.log('🔌 Disconnecting...');
        this.isConnected = false;
        
        // Close serial port
        if (this.reader) {
            try {
                await this.reader.cancel();
                this.reader.releaseLock();
            } catch (e) {}
            this.reader = null;
        }
        
        if (this.writer) {
            try {
                await this.writer.close();
            } catch (e) {}
            this.writer = null;
        }
        
        if (this.serialPort) {
            try {
                await this.serialPort.close();
            } catch (e) {}
            this.serialPort = null;
        }
        
        // Close WebSocket
        if (this.webSocket) {
            this.webSocket.close();
            this.webSocket = null;
        }
        
        this.updateUI();
        this.dataFlow.textContent = '-';
        this.log('🔌 Disconnected');
    }
    
    updateUI() {
        this.connectBtn.disabled = this.isConnected;
        this.disconnectBtn.disabled = !this.isConnected;
        this.baudRateSelect.disabled = this.isConnected;
        
        if (!this.isConnected) {
            this.serialStatus.textContent = 'Disconnected';
            this.serialStatus.className = 'status-value status-disconnected';
            this.websocketStatus.textContent = 'Disconnected';
            this.websocketStatus.className = 'status-value status-disconnected';
        }
    }
    
    log(message, type = 'info') {
        const timestamp = new Date().toLocaleTimeString();
        const logEntry = document.createElement('div');
        logEntry.className = `log-entry log-${type}`;
        logEntry.textContent = `[${timestamp}] ${message}`;
        
        this.logContainer.appendChild(logEntry);
        this.logContainer.scrollTop = this.logContainer.scrollHeight;
        
        // Keep only last 100 log entries
        while (this.logContainer.children.length > 100) {
            this.logContainer.removeChild(this.logContainer.firstChild);
        }
    }
    
    clearLog() {
        this.logContainer.innerHTML = '';
        this.log('📝 Log cleared');
    }
}

// Initialize bridge when page loads
document.addEventListener('DOMContentLoaded', () => {
    window.debugBridge = new ESP32DebugBridge();
});

// Handle page unload
window.addEventListener('beforeunload', () => {
    if (window.debugBridge) {
        window.debugBridge.disconnect();
    }
});