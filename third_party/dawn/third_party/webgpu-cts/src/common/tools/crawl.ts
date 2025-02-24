// Node can look at the filesystem, but JS in the browser can't.
// This crawls the file tree under src/suites/${suite} to generate a (non-hierarchical) static
// listing file that can then be used in the browser to load the modules containing the tests.

import * as fs from 'fs';
import * as path from 'path';

import { loadMetadataForSuite } from '../framework/metadata.js';
import { SpecFile } from '../internal/file_loader.js';
import { TestQueryMultiCase, TestQueryMultiFile } from '../internal/query/query.js';
import { validQueryPart } from '../internal/query/validQueryPart.js';
import { TestSuiteListingEntry, TestSuiteListing } from '../internal/test_suite_listing.js';
import { assert, unreachable } from '../util/util.js';

const specFileSuffix = __filename.endsWith('.ts') ? '.spec.ts' : '.spec.js';

async function crawlFilesRecursively(dir: string): Promise<string[]> {
  const subpathInfo = await Promise.all(
    (await fs.promises.readdir(dir)).map(async d => {
      const p = path.join(dir, d);
      const stats = await fs.promises.stat(p);
      return {
        path: p,
        isDirectory: stats.isDirectory(),
        isFile: stats.isFile(),
      };
    })
  );

  const files = subpathInfo
    .filter(
      i =>
        i.isFile &&
        (i.path.endsWith(specFileSuffix) ||
          i.path.endsWith(`${path.sep}README.txt`) ||
          i.path === 'README.txt')
    )
    .map(i => i.path);

  return files.concat(
    await subpathInfo
      .filter(i => i.isDirectory)
      .map(i => crawlFilesRecursively(i.path))
      .reduce(async (a, b) => (await a).concat(await b), Promise.resolve([]))
  );
}

export async function crawl(
  suiteDir: string,
  opts: { validate: boolean; printMetadataWarnings: boolean } | null = null
): Promise<TestSuiteListingEntry[]> {
  if (!fs.existsSync(suiteDir)) {
    throw new Error(`Could not find suite: ${suiteDir}`);
  }

  let validateTimingsEntries;
  if (opts?.validate) {
    const metadata = loadMetadataForSuite(suiteDir);
    if (metadata) {
      validateTimingsEntries = {
        metadata,
        testsFoundInFiles: new Set<string>(),
      };
    }
  }

  // Crawl files and convert paths to be POSIX-style, relative to suiteDir.
  const filesToEnumerate = (await crawlFilesRecursively(suiteDir))
    .map(f => path.relative(suiteDir, f).replace(/\\/g, '/'))
    .sort();

  const entries: TestSuiteListingEntry[] = [];
  for (const file of filesToEnumerate) {
    // |file| is the suite-relative file path.
    if (file.endsWith(specFileSuffix)) {
      const filepathWithoutExtension = file.substring(0, file.length - specFileSuffix.length);
      const pathSegments = filepathWithoutExtension.split('/');

      const suite = path.basename(suiteDir);

      if (opts?.validate) {
        const filename = `../../${suite}/${filepathWithoutExtension}.spec.js`;

        assert(!process.env.STANDALONE_DEV_SERVER);
        const mod = (await import(filename)) as SpecFile;
        assert(mod.description !== undefined, 'Test spec file missing description: ' + filename);
        assert(mod.g !== undefined, 'Test spec file missing TestGroup definition: ' + filename);

        mod.g.validate(new TestQueryMultiFile(suite, pathSegments));

        for (const { testPath } of mod.g.collectNonEmptyTests()) {
          const testQuery = new TestQueryMultiCase(suite, pathSegments, testPath, {}).toString();
          if (validateTimingsEntries) {
            validateTimingsEntries.testsFoundInFiles.add(testQuery);
          }
        }
      }

      for (const p of pathSegments) {
        assert(validQueryPart.test(p), `Invalid directory name ${p}; must match ${validQueryPart}`);
      }
      entries.push({ file: pathSegments });
    } else if (path.basename(file) === 'README.txt') {
      const dirname = path.dirname(file);
      const readme = fs.readFileSync(path.join(suiteDir, file), 'utf8').trim();

      const pathSegments = dirname !== '.' ? dirname.split('/') : [];
      entries.push({ file: pathSegments, readme });
    } else {
      unreachable(`Matched an unrecognized filename ${file}`);
    }
  }

  if (validateTimingsEntries) {
    const zeroEntries = [];
    const staleEntries = [];
    for (const [metadataKey, metadataValue] of Object.entries(validateTimingsEntries.metadata)) {
      if (metadataKey.startsWith('_')) {
        // Ignore json "_comments".
        continue;
      }
      if (metadataValue.subcaseMS <= 0) {
        zeroEntries.push(metadataKey);
      }
      if (!validateTimingsEntries.testsFoundInFiles.has(metadataKey)) {
        staleEntries.push(metadataKey);
      }
    }
    if (zeroEntries.length && opts?.printMetadataWarnings) {
      console.warn(
        'WARNING: subcaseMS ≤ 0 found in listing_meta.json (see docs/adding_timing_metadata.md):'
      );
      for (const metadataKey of zeroEntries) {
        console.warn(`  ${metadataKey}`);
      }
    }

    if (opts?.printMetadataWarnings) {
      const missingEntries = [];
      for (const metadataKey of validateTimingsEntries.testsFoundInFiles) {
        if (!(metadataKey in validateTimingsEntries.metadata)) {
          missingEntries.push(metadataKey);
        }
      }
      if (missingEntries.length) {
        console.error(
          'WARNING: Tests missing from listing_meta.json (see docs/adding_timing_metadata.md):'
        );
        for (const metadataKey of missingEntries) {
          console.error(`  ${metadataKey}`);
        }
      }
    }

    if (staleEntries.length) {
      console.error('ERROR: Non-existent tests found in listing_meta.json. Please update:');
      for (const metadataKey of staleEntries) {
        console.error(`  ${metadataKey}`);
      }
      unreachable();
    }
  }

  return entries;
}

export function makeListing(filename: string): Promise<TestSuiteListing> {
  // Don't validate. This path is only used for the dev server and running tests with Node.
  // Validation is done for listing generation and presubmit.
  return crawl(path.dirname(filename));
}
