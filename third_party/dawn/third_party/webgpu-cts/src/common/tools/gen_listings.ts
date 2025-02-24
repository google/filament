import * as fs from 'fs';
import * as path from 'path';
import * as process from 'process';

import { crawl } from './crawl.js';

function usage(rc: number): void {
  console.error(`Usage: tools/gen_listings [options] [OUT_DIR] [SUITE_DIRS...]

For each suite in SUITE_DIRS, generate listings and write each listing.js
into OUT_DIR/{suite}/listing.js. Example:
  tools/gen_listings gen/ src/unittests/ src/webgpu/

Options:
  --help          Print this message and exit.
`);
  process.exit(rc);
}

const argv = process.argv;
if (argv.indexOf('--help') !== -1) {
  usage(0);
}

{
  // Ignore old argument that is now the default
  const i = argv.indexOf('--no-validate');
  if (i !== -1) {
    argv.splice(i, 1);
  }
}

if (argv.length < 4) {
  usage(0);
}

const myself = 'src/common/tools/gen_listings.ts';

const outDir = argv[2];

for (const suiteDir of argv.slice(3)) {
  // Run concurrently for each suite (might be a tiny bit more efficient)
  void crawl(suiteDir).then(listing => {
    const suite = path.basename(suiteDir);
    const outFile = path.normalize(path.join(outDir, `${suite}/listing.js`));
    fs.mkdirSync(path.join(outDir, suite), { recursive: true });
    fs.writeFileSync(
      outFile,
      `\
// AUTO-GENERATED - DO NOT EDIT. See ${myself}.

export const listing = ${JSON.stringify(listing, undefined, 2)};
`
    );
  });
}
