import * as fs from 'fs';
import * as process from 'process';
import * as readline from 'readline';

import { TestMetadataListing } from '../framework/metadata.js';
import { parseQuery } from '../internal/query/parseQuery.js';
import { TestQueryMultiCase, TestQuerySingleCase } from '../internal/query/query.js';
import { CaseTimingLogLine } from '../internal/test_group.js';
import { assert } from '../util/util.js';

// For information on listing_meta.json file maintenance, please read
// tools/merge_listing_times first.

function usage(rc: number): never {
  console.error(`Usage: tools/merge_listing_times [options] SUITES... -- [TIMING_LOG_FILES...]

Options:
  --help          Print this message and exit.

Reads raw case timing data for each suite in SUITES, from all TIMING_LOG_FILES
(see below), and merges it into the src/*/listing_meta.json files checked into
the repository. The timing data in the listing_meta.json files is updated with
the newly-observed timing data *if the new timing is slower*. That is, it will
only increase the values in the listing_meta.json file, and will only cause WPT
chunks to become smaller.

If there are no TIMING_LOG_FILES, this just regenerates (reformats) the file
using the data already present.

In more detail:

- Reads per-case timing data in any of the SUITES, from all TIMING_LOG_FILES
  (ignoring skipped cases), and averages it over the number of subcases.
  In the case of cases that have run multiple times, takes the max of each.
- Compiles the average time-per-subcase for each test seen.
- For each suite seen, loads its listing_meta.json, takes the max of the old and
  new data, and writes it back out.

See 'docs/adding_timing_metadata.md' for how to generate TIMING_LOG_FILES files.
`);
  process.exit(rc);
}

const kHeader = `{
  "_comment": "SEMI AUTO-GENERATED. This list is NOT exhaustive. Please read docs/adding_timing_metadata.md.",
`;
const kFooter = `\
  "_end": ""
}
`;

const argv = process.argv;
if (argv.some(v => v.startsWith('-') && v !== '--') || argv.every(v => v !== '--')) {
  usage(0);
}
const suites = [];
const timingLogFilenames = [];
let seenDashDash = false;
for (const arg of argv.slice(2)) {
  if (arg === '--') {
    seenDashDash = true;
    continue;
  } else if (arg.startsWith('-')) {
    usage(0);
  }

  if (seenDashDash) {
    timingLogFilenames.push(arg);
  } else {
    suites.push(arg);
  }
}
if (!seenDashDash) {
  usage(0);
}

void (async () => {
  // Read the log files to find the log line for each *case* query. If a case
  // ran multiple times, take the one with the largest average subcase time.
  const caseTimes = new Map<string, CaseTimingLogLine>();
  for (const timingLogFilename of timingLogFilenames) {
    const rl = readline.createInterface({
      input: fs.createReadStream(timingLogFilename),
      crlfDelay: Infinity,
    });

    for await (const line of rl) {
      const parsed: CaseTimingLogLine = JSON.parse(line);

      const prev = caseTimes.get(parsed.q);
      if (prev !== undefined) {
        const timePerSubcase = parsed.timems / Math.max(1, parsed.nonskippedSubcaseCount);
        const prevTimePerSubcase = prev.timems / Math.max(1, prev.nonskippedSubcaseCount);

        if (timePerSubcase > prevTimePerSubcase) {
          caseTimes.set(parsed.q, parsed);
        }
      } else {
        caseTimes.set(parsed.q, parsed);
      }
    }
  }

  // Accumulate total times per test. Map of suite -> query -> {totalTimeMS, caseCount}.
  const testTimes = new Map<string, Map<string, { totalTimeMS: number; subcaseCount: number }>>();
  for (const suite of suites) {
    testTimes.set(suite, new Map());
  }
  for (const [caseQString, caseTime] of caseTimes) {
    const caseQ = parseQuery(caseQString);
    assert(caseQ instanceof TestQuerySingleCase);
    const suite = caseQ.suite;
    const suiteTestTimes = testTimes.get(suite);
    if (suiteTestTimes === undefined) {
      continue;
    }

    const testQ = new TestQueryMultiCase(suite, caseQ.filePathParts, caseQ.testPathParts, {});
    const testQString = testQ.toString();

    const prev = suiteTestTimes.get(testQString);
    if (prev !== undefined) {
      prev.totalTimeMS += caseTime.timems;
      prev.subcaseCount += caseTime.nonskippedSubcaseCount;
    } else {
      suiteTestTimes.set(testQString, {
        totalTimeMS: caseTime.timems,
        subcaseCount: caseTime.nonskippedSubcaseCount,
      });
    }
  }

  for (const suite of suites) {
    const currentMetadata: TestMetadataListing = JSON.parse(
      fs.readFileSync(`./src/${suite}/listing_meta.json`, 'utf8')
    );

    const metadata = { ...currentMetadata };
    for (const [testQString, { totalTimeMS, subcaseCount }] of testTimes.get(suite)!) {
      const avgTime = totalTimeMS / Math.max(1, subcaseCount);
      if (testQString in metadata) {
        metadata[testQString].subcaseMS = Math.max(metadata[testQString].subcaseMS, avgTime);
      } else {
        metadata[testQString] = { subcaseMS: avgTime };
      }
    }

    writeListings(suite, metadata);
  }
})();

function writeListings(suite: string, metadata: TestMetadataListing) {
  const output = fs.createWriteStream(`./src/${suite}/listing_meta.json`);
  try {
    output.write(kHeader);
    const keys = Object.keys(metadata).sort();
    for (const k of keys) {
      if (k.startsWith('_')) {
        // Ignore json "_comments".
        continue;
      }
      assert(k.indexOf('"') === -1);
      output.write(`  "${k}": { "subcaseMS": ${metadata[k].subcaseMS.toFixed(3)} },\n`);
    }
    output.write(kFooter);
  } finally {
    output.close();
  }
}
