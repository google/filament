/* eslint-disable no-console, n/no-restricted-import */

import * as fs from 'fs';

import { dataCache } from '../framework/data_cache.js';
import { getResourcePath, setBaseResourcePath } from '../framework/resources.js';
import { globalTestConfig } from '../framework/test_config.js';
import { DefaultTestFileLoader } from '../internal/file_loader.js';
import { prettyPrintLog } from '../internal/logging/log_message.js';
import { Logger } from '../internal/logging/logger.js';
import { LiveTestCaseResult } from '../internal/logging/result.js';
import { parseQuery } from '../internal/query/parseQuery.js';
import { parseExpectationsForTestQuery } from '../internal/query/query.js';
import { Colors } from '../util/colors.js';
import { setDefaultRequestAdapterOptions, setGPUProvider } from '../util/navigator_gpu.js';
import { assert, unreachable } from '../util/util.js';

import sys from './helper/sys.js';

function usage(rc: number): never {
  console.log(`Usage:
  tools/run_${sys.type} [OPTIONS...] QUERIES...
  tools/run_${sys.type} 'unittests:*' 'webgpu:buffers,*'
Options:
  --colors                  Enable ANSI colors in output.
  --compat                  Runs tests in compatibility mode.
  --coverage                Emit coverage data.
  --verbose                 Print result/log of every test as it runs.
  --list                    Print all testcase names that match the given query and exit.
  --list-unimplemented      Print all unimplemented tests
  --debug                   Include debug messages in logging.
  --print-json              Print the complete result JSON in the output.
  --expectations            Path to expectations file.
  --gpu-provider            Path to node module that provides the GPU implementation.
  --gpu-provider-flag       Flag to set on the gpu-provider as <flag>=<value>
  --unroll-const-eval-loops Unrolls loops in constant-evaluation shader execution tests
  --enforce-default-limits  Enforce the default limits (note: powerPreference tests may fail)
  --force-fallback-adapter  Force a fallback adapter
  --log-to-websocket        Log to a websocket
  --quiet                   Suppress summary information in output
`);
  return sys.exit(rc);
}

if (!sys.existsSync('src/common/runtime/cmdline.ts')) {
  console.log('Must be run from repository root');
  usage(1);
}
setBaseResourcePath('out-node/resources');

// The interface that exposes creation of the GPU, and optional interface to code coverage.
interface GPUProviderModule {
  // @returns a GPU with the given flags
  create(flags: string[]): GPU;
  // An optional interface to a CodeCoverageProvider
  coverage?: CodeCoverageProvider;
}

interface CodeCoverageProvider {
  // Starts collecting code coverage
  begin(): void;
  // Ends collecting of code coverage, returning the coverage data.
  // This data is opaque (implementation defined).
  end(): string;
}

type listModes = 'none' | 'cases' | 'unimplemented';

Colors.enabled = false;

let verbose = false;
let emitCoverage = false;
let listMode: listModes = 'none';
let printJSON = false;
let quiet = false;
let loadWebGPUExpectations: Promise<unknown> | undefined = undefined;
let gpuProviderModule: GPUProviderModule | undefined = undefined;

const queries: string[] = [];
const gpuProviderFlags: string[] = [];
for (let i = 0; i < sys.args.length; ++i) {
  const a = sys.args[i];
  if (a.startsWith('-')) {
    if (a === '--colors') {
      Colors.enabled = true;
    } else if (a === '--coverage') {
      emitCoverage = true;
    } else if (a === '--verbose') {
      verbose = true;
    } else if (a === '--list') {
      listMode = 'cases';
    } else if (a === '--list-unimplemented') {
      listMode = 'unimplemented';
    } else if (a === '--debug') {
      globalTestConfig.enableDebugLogs = true;
    } else if (a === '--print-json') {
      printJSON = true;
    } else if (a === '--expectations') {
      const expectationsFile = new URL(sys.args[++i], `file://${sys.cwd()}`).pathname;
      loadWebGPUExpectations = import(expectationsFile).then(m => m.expectations);
    } else if (a === '--gpu-provider') {
      const modulePath = sys.args[++i];
      gpuProviderModule = require(modulePath);
    } else if (a === '--gpu-provider-flag') {
      gpuProviderFlags.push(sys.args[++i]);
    } else if (a === '--quiet') {
      quiet = true;
    } else if (a === '--unroll-const-eval-loops') {
      globalTestConfig.unrollConstEvalLoops = true;
    } else if (a === '--compat') {
      globalTestConfig.compatibility = true;
    } else if (a === '--force-fallback-adapter') {
      globalTestConfig.forceFallbackAdapter = true;
    } else if (a === '--enforce-default-limits') {
      globalTestConfig.enforceDefaultLimits = true;
    } else if (a === '--log-to-websocket') {
      globalTestConfig.logToWebSocket = true;
    } else {
      console.log('unrecognized flag: ', a);
      usage(1);
    }
  } else {
    queries.push(a);
  }
}

let codeCoverage: CodeCoverageProvider | undefined = undefined;

if (globalTestConfig.compatibility || globalTestConfig.forceFallbackAdapter) {
  // MAINTENANCE_TODO: remove compatibilityMode (and the typecast) once no longer needed.
  setDefaultRequestAdapterOptions({
    compatibilityMode: globalTestConfig.compatibility,
    featureLevel: globalTestConfig.compatibility ? 'compatibility' : 'core',
    forceFallbackAdapter: globalTestConfig.forceFallbackAdapter,
  } as GPURequestAdapterOptions);
}

if (gpuProviderModule) {
  setGPUProvider(() => gpuProviderModule!.create(gpuProviderFlags));
  if (emitCoverage) {
    codeCoverage = gpuProviderModule.coverage;
    if (codeCoverage === undefined) {
      console.error(
        `--coverage specified, but the GPUProviderModule does not support code coverage.
Did you remember to build with code coverage instrumentation enabled?`
      );
      sys.exit(1);
    }
  }
}

dataCache.setStore({
  load: (path: string) => {
    return new Promise<Uint8Array>((resolve, reject) => {
      fs.readFile(getResourcePath(`cache/${path}`), (err, data) => {
        if (err !== null) {
          reject(err.message);
        } else {
          resolve(data);
        }
      });
    });
  },
});

if (verbose) {
  dataCache.setDebugLogger(console.log);
}

if (queries.length === 0) {
  console.log('no queries specified');
  usage(0);
}

(async () => {
  const loader = new DefaultTestFileLoader();
  assert(queries.length === 1, 'currently, there must be exactly one query on the cmd line');
  const filterQuery = parseQuery(queries[0]);
  const testcases = await loader.loadCases(filterQuery);
  const expectations = parseExpectationsForTestQuery(
    await (loadWebGPUExpectations ?? []),
    filterQuery
  );

  const log = new Logger();

  const failed: Array<[string, LiveTestCaseResult]> = [];
  const warned: Array<[string, LiveTestCaseResult]> = [];
  const skipped: Array<[string, LiveTestCaseResult]> = [];

  let total = 0;

  if (codeCoverage !== undefined) {
    codeCoverage.begin();
  }

  for (const testcase of testcases) {
    const name = testcase.query.toString();
    switch (listMode) {
      case 'cases':
        console.log(name);
        continue;
      case 'unimplemented':
        if (testcase.isUnimplemented) {
          console.log(name);
        }
        continue;
      default:
        break;
    }

    const [rec, res] = log.record(name);
    await testcase.run(rec, expectations);

    if (verbose) {
      printResults([[name, res]]);
    }

    total++;
    switch (res.status) {
      case 'pass':
        break;
      case 'fail':
        failed.push([name, res]);
        break;
      case 'warn':
        warned.push([name, res]);
        break;
      case 'skip':
        skipped.push([name, res]);
        break;
      default:
        unreachable('unrecognized status');
    }
  }

  if (codeCoverage !== undefined) {
    const coverage = codeCoverage.end();
    console.log(`Code-coverage: [[${coverage}]]`);
  }

  if (listMode !== 'none') {
    return;
  }

  assert(total > 0, 'found no tests!');

  // MAINTENANCE_TODO: write results out somewhere (a file?)
  if (printJSON) {
    console.log(log.asJSON(2));
  }

  if (!quiet) {
    if (skipped.length) {
      console.log('');
      console.log('** Skipped **');
      printResults(skipped);
    }
    if (warned.length) {
      console.log('');
      console.log('** Warnings **');
      printResults(warned);
    }
    if (failed.length) {
      console.log('');
      console.log('** Failures **');
      printResults(failed);
    }

    const passed = total - warned.length - failed.length - skipped.length;
    const pct = (x: number) => ((100 * x) / total).toFixed(2);
    const rpt = (x: number) => {
      const xs = x.toString().padStart(1 + Math.log10(total), ' ');
      return `${xs} / ${total} = ${pct(x).padStart(6, ' ')}%`;
    };
    console.log('');
    console.log(`** Summary **
Passed  w/o warnings = ${rpt(passed)}
Passed with warnings = ${rpt(warned.length)}
Skipped              = ${rpt(skipped.length)}
Failed               = ${rpt(failed.length)}`);
  }

  if (failed.length || warned.length) {
    sys.exit(1);
  }
})().catch(ex => {
  console.log(ex.stack ?? ex.toString());
  sys.exit(1);
});

function printResults(results: Array<[string, LiveTestCaseResult]>): void {
  for (const [name, r] of results) {
    console.log(`[${r.status}] ${name} (${r.timems}ms). Log:`);
    if (r.logs) {
      for (const l of r.logs) {
        console.log(prettyPrintLog(l));
      }
    }
  }
}
