import fs from 'fs/promises';
import { WebSocketServer } from 'ws';

const wss = new WebSocketServer({ port: 59497 });

const timestamp = new Date().toISOString().slice(0, 19).replace(/[:]/g, '-');
const filename = `wslog-${timestamp}.txt`;
const f = await fs.open(filename, 'w');
console.log(`Writing to ${filename}`);
console.log('Ctrl-C to stop');

process.on('SIGINT', () => {
  console.log(`\nWritten to ${filename}`);
  process.exit();
});

wss.on('connection', async ws => {
  ws.on('message', data => {
    const s = data.toString();
    f.write(s + '\n');
    console.log(s);
  });
});
