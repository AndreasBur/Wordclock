/* Serves the web console locally with the real firmware behind it.
 *
 * The page comes off disk, so editing it is a browser reload. /commands and the web socket
 * are answered by the webhost process, which runs the firmware core - so what the page sees
 * is what the device would send.
 *
 * The web socket is implemented here rather than pulled in, so this needs nothing installed.
 *
 * Usage: node serve.js <page.html> <webhost binary> [port]
 */
'use strict';

const http = require('http');
const crypto = require('crypto');
const fs = require('fs');
const { spawn } = require('child_process');

const [pagePath, hostPath, portArg] = process.argv.slice(2);
const port = Number(portArg || 8080);

const clock = spawn(hostPath, [], { stdio: ['pipe', 'pipe', 'inherit'] });
const sockets = new Set();
/* One waiter list per description, because both are asked for by the same channel. */
const described = { commands: null, display: null };
const waiters = { commands: [], display: [] };
let asked = null;

let pending = '';
clock.stdout.on('data', (chunk) => {
    pending += chunk.toString();
    let end;
    while ((end = pending.indexOf('\n')) !== -1) {
        const line = pending.slice(0, end);
        pending = pending.slice(end + 1);

        if (line[0] === '#') {
            described[asked] = line.slice(1);
            while (waiters[asked].length) { waiters[asked].pop()(described[asked]); }
        } else if (line[0] === '>') {
            const text = line.slice(1);
            console.log('  <-', text);
            for (const socket of sockets) { sendFrame(socket, text); }
        } else if (line[0] === 'F') {
            const bytes = Buffer.from(line.slice(1), 'hex');
            let lit = 0;
            for (let i = 0; i < bytes.length; i += 3) { if (bytes[i] || bytes[i + 1] || bytes[i + 2]) { lit++; } }
            console.log(`  <- frame, ${lit} lit, to ${sockets.size} client(s)`);
            for (const socket of sockets) { sendFrame(socket, bytes); }
        }
    }
});

function describe(what) {
    if (described[what] !== null) { return Promise.resolve(described[what]); }
    asked = what;
    clock.stdin.write((what === 'commands' ? 'C' : 'D') + '\n');
    return new Promise((resolve) => waiters[what].push(resolve));
}

/* ---- the web socket, by hand ------------------------------------------------------- */
const WS_GUID = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11';

function accept(request, socket) {
    const key = request.headers['sec-websocket-key'];
    const digest = crypto.createHash('sha1').update(key + WS_GUID).digest('base64');

    socket.write(
        'HTTP/1.1 101 Switching Protocols\r\n' +
        'Upgrade: websocket\r\nConnection: Upgrade\r\n' +
        `Sec-WebSocket-Accept: ${digest}\r\n\r\n`
    );
    socket.setNoDelay(true);
    sockets.add(socket);
    console.log('  ++ client connected');
    /* Ask for the display as it stands: the firmware only broadcasts on a change, so a
       client arriving between two changes would otherwise see an empty panel. */
    clock.stdin.write('R\n');

    let buffer = Buffer.alloc(0);
    socket.on('data', (chunk) => {
        buffer = Buffer.concat([buffer, chunk]);
        for (;;) {
            const frame = readFrame(buffer);
            if (!frame) { break; }
            buffer = buffer.subarray(frame.consumed);

            if (frame.opcode === 0x8) { socket.end(); return; }
            if (frame.opcode === 0x1) {
                const text = frame.payload.toString();
                console.log('  ->', text);
                clock.stdin.write(text + '\n');
            }
        }
    });
    const drop = () => { sockets.delete(socket); console.log('  -- client gone'); };
    socket.on('close', drop);
    socket.on('error', drop);
}

function readFrame(buffer) {
    if (buffer.length < 2) { return null; }

    const opcode = buffer[0] & 0x0f;
    const masked = (buffer[1] & 0x80) !== 0;
    let length = buffer[1] & 0x7f;
    let offset = 2;

    if (length === 126) {
        if (buffer.length < 4) { return null; }
        length = buffer.readUInt16BE(2);
        offset = 4;
    } else if (length === 127) {
        /* A console line is never this long; refusing is better than pretending. */
        return null;
    }

    const maskLength = masked ? 4 : 0;
    if (buffer.length < offset + maskLength + length) { return null; }

    const mask = masked ? buffer.subarray(offset, offset + 4) : null;
    const payload = Buffer.from(buffer.subarray(offset + maskLength, offset + maskLength + length));
    if (mask) { for (let i = 0; i < payload.length; i++) { payload[i] ^= mask[i % 4]; } }

    return { opcode, payload, consumed: offset + maskLength + length };
}

function sendFrame(socket, data) {
    const binary = Buffer.isBuffer(data);
    const payload = binary ? data : Buffer.from(data);
    const opcode = binary ? 0x82 : 0x81;
    let header;

    if (payload.length < 126) {
        header = Buffer.from([opcode, payload.length]);
    } else {
        header = Buffer.alloc(4);
        header[0] = opcode;
        header[1] = 126;
        header.writeUInt16BE(payload.length, 2);
    }
    socket.write(Buffer.concat([header, payload]));
}

/* ---- HTTP -------------------------------------------------------------------------- */
const server = http.createServer(async (request, response) => {
    if (request.url === '/' || request.url === '/index.html') {
        /* Read per request, so a saved edit needs a reload and nothing else. */
        /* Never cached. This server exists to look at a page being changed, and a browser
           deciding on its own how long to keep the last one is the whole cost of that: an
           edit that appears not to have worked is indistinguishable from one that did not.
           The clock itself serves the page from flash and has no such problem. */
        response.writeHead(200, {
            'Content-Type': 'text/html; charset=utf-8',
            'Cache-Control': 'no-store',
        });
        response.end(fs.readFileSync(pagePath));
        return;
    }
    if (request.url === '/commands' || request.url === '/display') {
        response.writeHead(200, { 'Content-Type': 'application/json' });
        response.end(await describe(request.url.slice(1)));
        return;
    }
    response.writeHead(404).end();
});

server.on('upgrade', (request, socket) => {
    if (request.url === '/ws') { accept(request, socket); } else { socket.destroy(); }
});

server.listen(port, () => {
    console.log(`wordclock web console on http://localhost:${port}/`);
    console.log('the firmware core is running behind it; commands take the device path');
});
