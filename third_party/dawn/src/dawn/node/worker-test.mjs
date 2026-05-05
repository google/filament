import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createRequire } from 'node:module';
import { parentPort } from 'node:worker_threads';

const require = createRequire(import.meta.url);

async function run() {
  const __dirname = dirname(fileURLToPath(import.meta.url));
  const dawnNodePath = join(__dirname, '..', '..', '..', 'out', 'active', 'dawn.node');
  const { create } = require(dawnNodePath);
  const gpu = create([]);
  const adapter = await gpu.requestAdapter();
  if (adapter) {
    parentPort.postMessage('success');
  } else {
    parentPort.postMessage('failure');
  }
}
run().catch(err => {
  parentPort.postMessage(err.message || String(err));
  process.exit(1);
});