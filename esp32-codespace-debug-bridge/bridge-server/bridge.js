/**
 * ESP32 Codespace Debug Bridge Server
 * 
 * Creates a WebSocket ↔ TCP bridge for ESP32 debugging in GitHub Codespaces.
 * Browser connects via WebSerial to ESP32, sends data via WebSocket to this server,
 * which forwards it to GDB via TCP on localhost:3333.
 */

const net = require('net');
const WebSocket = require('ws');

// Configuration
const TCP_PORT = 3333;  // GDB will connect here (localhost:3333)
const WS_PORT = 8081;   // WebSocket port (will be forwarded in Codespaces)
const WS_PATH = '/ws';  // WebSocket endpoint path

console.log('🚀 ESP32 Codespace Debug Bridge Server');
console.log('=====================================');

// Create TCP server for GDB connections
const tcpServer = net.createServer();
let tcpSocket = null;
let clientCount = 0;

tcpServer.on('connection', (socket) => {
    tcpSocket = socket;
    clientCount++;
    const clientId = clientCount;
    
    console.log(`📡 GDB connected (client ${clientId}) from ${socket.remoteAddress}:${socket.remotePort}`);
    
    // TCP data from GDB → forward to all WebSocket clients (browser)
    socket.on('data', (chunk) => {
        console.log(`📤 GDB→WS: ${chunk.length} bytes`);
        wss.clients.forEach(ws => {
            if (ws.readyState === WebSocket.OPEN) {
                ws.send(chunk);
            }
        });
    });
    
    socket.on('close', () => {
        console.log(`📡 GDB disconnected (client ${clientId})`);
        tcpSocket = null;
    });
    
    socket.on('error', (err) => {
        console.error(`❌ TCP socket error (client ${clientId}):`, err.message);
    });
});

tcpServer.listen(TCP_PORT, '127.0.0.1', () => {
    console.log(`🔌 TCP server listening on localhost:${TCP_PORT} (for GDB)`);
});

// Create WebSocket server for browser connections
const wss = new WebSocket.Server({ 
    port: WS_PORT, 
    path: WS_PATH,
    verifyClient: (info) => {
        console.log(`🌐 WebSocket connection attempt from ${info.req.connection.remoteAddress}`);
        return true;
    }
});

wss.on('connection', (ws, req) => {
    console.log(`🌐 Browser connected via WebSocket from ${req.connection.remoteAddress}`);
    
    // WebSocket data from browser → forward to TCP (GDB)
    ws.on('message', (message) => {
        const buffer = Buffer.from(message);
        console.log(`📥 WS→GDB: ${buffer.length} bytes`);
        
        if (tcpSocket && tcpSocket.writable) {
            tcpSocket.write(buffer);
        } else {
            console.log(`⚠️  No GDB connection available, dropping ${buffer.length} bytes`);
        }
    });
    
    ws.on('close', () => {
        console.log('🌐 Browser disconnected');
    });
    
    ws.on('error', (err) => {
        console.error('❌ WebSocket error:', err.message);
    });
    
    // Send welcome message
    ws.send(JSON.stringify({
        type: 'status',
        message: 'ESP32 Debug Bridge connected',
        tcpConnected: !!tcpSocket
    }));
});

console.log(`🌐 WebSocket server listening on port ${WS_PORT}${WS_PATH}`);
console.log('');
console.log('📋 Next steps:');
console.log('1. Forward port 8081 in Codespaces');
console.log('2. Open web client: https://<codespace>-8081.githubpreview.dev/');
console.log('3. Connect ESP32 via USB and click "Connect Serial & WebSocket"');
console.log('4. In terminal: xtensa-esp32-elf-gdb build/your-app.elf');
console.log('5. In GDB: target remote localhost:3333');
console.log('');

// Graceful shutdown
process.on('SIGINT', () => {
    console.log('\n🛑 Shutting down bridge server...');
    
    wss.clients.forEach(ws => {
        ws.close();
    });
    
    if (tcpSocket) {
        tcpSocket.end();
    }
    
    tcpServer.close(() => {
        console.log('✅ Bridge server stopped');
        process.exit(0);
    });
});

// Error handling
tcpServer.on('error', (err) => {
    console.error('❌ TCP server error:', err.message);
    if (err.code === 'EADDRINUSE') {
        console.error(`Port ${TCP_PORT} is already in use. Is another GDB session running?`);
    }
});

wss.on('error', (err) => {
    console.error('❌ WebSocket server error:', err.message);
});