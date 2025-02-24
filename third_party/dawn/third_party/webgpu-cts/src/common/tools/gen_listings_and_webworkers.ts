import * as fs from 'fs';
import * as path from 'path';
import * as process from 'process';

import { crawl } from './crawl.js';

function usage(rc: number): void {
  console.error(`Usage: tools/gen_listings_and_webworkers [options] [OUT_DIR] [SUITE_DIRS...]

For each suite in SUITE_DIRS, generate listings into OUT_DIR/{suite}/listing.js,
and generate Web Worker proxies in OUT_DIR/{suite}/webworker/**/*.as_worker.js for
every .spec.js file. (Note {suite}/webworker/ is reserved for this purpose.)

Example:
  tools/gen_listings_and_webworkers gen/ src/unittests/ src/webgpu/

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

const myself = 'src/common/tools/gen_listings_and_webworkers.ts';

const outDir = argv[2];

for (const suiteDir of argv.slice(3)) {
  // Run concurrently for each suite (might be a tiny bit more efficient)
  void crawl(suiteDir).then(listing => {
    const suite = path.basename(suiteDir);

    // Write listing.js
    const outFile = path.normalize(path.join(outDir, `${suite}/listing.js`));
    fs.mkdirSync(path.join(outDir, suite), { recursive: true });
    fs.writeFileSync(
      outFile,
      `\
// AUTO-GENERATED - DO NOT EDIT. See ${myself}.

export const listing = ${JSON.stringify(listing, undefined, 2)};
`
    );

    // Write suite/webworker/**/*.as_worker.js
    // (Note we avoid ".worker.js" because that conflicts with WPT test naming patterns.)
    for (const entry of listing) {
      if ('readme' in entry) continue;

      const outFileDir = path.join(
        outDir,
        suite,
        'webworker',
        ...entry.file.slice(0, entry.file.length - 1)
      );
      const outFile = path.join(outDir, suite, 'webworker', ...entry.file) + '.as_worker.js';

      const relPathToSuiteRoot = Array<string>(entry.file.length).fill('..').join('/');

      fs.mkdirSync(outFileDir, { recursive: true });
      fs.writeFileSync(
        outFile,
        `\
// AUTO-GENERATED - DO NOT EDIT. See ${myself}.

import { g } from '${relPathToSuiteRoot}/${entry.file.join('/')}.spec.js';
import { wrapTestGroupForWorker } from '${relPathToSuiteRoot}/../common/runtime/helper/wrap_for_worker.js';

wrapTestGroupForWorker(g);
`
      );
    }
  });
}
