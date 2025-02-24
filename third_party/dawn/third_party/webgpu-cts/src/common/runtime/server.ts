/* eslint-disable no-console, n/no-restricted-import */

import * as fs from 'fs';
import * as http from 'http';
import { AddressInfo } from 'net';

import { dataCache } from '../framework/data_cache.js';
import { getResourcePath, setBaseResourcePath } from '../framework/resources.js';
import { globalTestConfig } from '../framework/test_config.js';
import { DefaultTestFileLoader } from '../internal/file_loader.js';
import { prettyPrintLog } from '../internal/logging/log_message.js';
import { Logger } from '../internal/logging/logger.js';
import { LiveTestCaseResult, Status } from '../internal/logging/result.js';
import { parseQuery } from '../internal/query/parseQuery.js';
import { TestQueryWithExpectation } from '../internal/query/query.js';
import { TestTreeLeaf } from '../internal/tree.js';
import { Colors } from '../util/colors.js';
import { setDefaultRequestAdapterOptions, setGPUProvider } from '../util/navigator_gpu.js';

import sys from './helper/sys.js';

function usage(rc: number): never {
  console.log(`Usage:
  tools/server [OPTIONS...]
Options:
  --colors                  Enable ANSI colors in output.
  --compat                  Run tests in compatibility mode.
  --coverage                Add coverage data to each result.
  --verbose                 Print result/log of every test as it runs.
  --debug                   Include debug messages in logging.
  --gpu-provider            Path to node module that provides the GPU implementation.
  --gpu-provider-flag       Flag to set on the gpu-provider as <flag>=<value>
  --unroll-const-eval-loops Unrolls loops in constant-evaluation shader execution tests
  --enforce-default-limits  Enforce the default limits (note: powerPreference tests may fail)
  --force-fallback-adapter  Force a fallback adapter
  --log-to-websocket        Log to a websocket
  --u                       Flag to set on the gpu-provider as <flag>=<value>

Provides an HTTP server used for running tests via an HTTP RPC interface
First, load some tree or subtree of tests:
  http://localhost:port/load?unittests:basic:*
To run a single test case, perform an HTTP GET or POST at the URL:
  http://localhost:port/run?unittests:basic:test,sync
To shutdown the server perform an HTTP GET or POST at the URL:
  http://localhost:port/terminate
`);
  return sys.exit(rc);
}

interface RunResult {
  // The result of the test
  status: Status;
  // Any additional messages printed
  message: string;
  // The time it took to execute the test
  durationMS: number;
  // Code coverage data, if the server was started with `--coverage`
  // This data is opaque (implementation defined).
  coverageData?: string;
}

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

if (!sys.existsSync('src/common/runtime/cmdline.ts')) {
  console.log('Must be run from repository root');
  usage(1);
}
setBaseResourcePath('out-node/resources');

Colors.enabled = false;

let emitCoverage = false;
let verbose = false;
let gpuProviderModule: GPUProviderModule | undefined = undefined;

const gpuProviderFlags: string[] = [];
for (let i = 0; i < sys.args.length; ++i) {
  const a = sys.args[i];
  if (a.startsWith('-')) {
    if (a === '--colors') {
      Colors.enabled = true;
    } else if (a === '--compat') {
      globalTestConfig.compatibility = true;
    } else if (a === '--coverage') {
      emitCoverage = true;
    } else if (a === '--force-fallback-adapter') {
      globalTestConfig.forceFallbackAdapter = true;
    } else if (a === '--enforce-default-limits') {
      globalTestConfig.enforceDefaultLimits = true;
    } else if (a === '--log-to-websocket') {
      globalTestConfig.logToWebSocket = true;
    } else if (a === '--gpu-provider') {
      const modulePath = sys.args[++i];
      gpuProviderModule = require(modulePath);
    } else if (a === '--gpu-provider-flag') {
      gpuProviderFlags.push(sys.args[++i]);
    } else if (a === '--debug') {
      globalTestConfig.enableDebugLogs = true;
    } else if (a === '--unroll-const-eval-loops') {
      globalTestConfig.unrollConstEvalLoops = true;
    } else if (a === '--help') {
      usage(1);
    } else if (a === '--verbose') {
      verbose = true;
    } else {
      console.log(`unrecognized flag: ${a}`);
    }
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

// eslint-disable-next-line @typescript-eslint/require-await
(async () => {
  const log = new Logger();
  const testcases = new Map<string, TestTreeLeaf>();

  async function runTestcase(
    testcase: TestTreeLeaf,
    expectations: TestQueryWithExpectation[] = []
  ): Promise<LiveTestCaseResult> {
    const name = testcase.query.toString();
    const [rec, res] = log.record(name);
    await testcase.run(rec, expectations);
    return res;
  }

  const server = http.createServer(
    async (request: http.IncomingMessage, response: http.ServerResponse) => {
      if (request.url === undefined) {
        response.end('invalid url');
        return;
      }

      const loadCasesPrefix = '/load?';
      const runPrefix = '/run?';
      const terminatePrefix = '/terminate';

      if (request.url.startsWith(loadCasesPrefix)) {
        const query = request.url.substr(loadCasesPrefix.length);
        try {
          const webgpuQuery = parseQuery(query);
          const loader = new DefaultTestFileLoader();
          for (const testcase of await loader.loadCases(webgpuQuery)) {
            testcases.set(testcase.query.toString(), testcase);
          }
          response.statusCode = 200;
          response.end();
        } catch (err) {
          response.statusCode = 500;
          response.end(`load failed with error: ${err}\n${(err as Error).stack}`);
        }
      } else if (request.url.startsWith(runPrefix)) {
        const name = request.url.substr(runPrefix.length);
        try {
          const testcase = testcases.get(name);
          if (testcase) {
            if (codeCoverage !== undefined) {
              codeCoverage.begin();
            }
            const start = performance.now();
            const result = await runTestcase(testcase);
            const durationMS = performance.now() - start;
            const coverageData = codeCoverage !== undefined ? codeCoverage.end() : undefined;
            let message = '';
            if (result.logs !== undefined) {
              message = result.logs.map(log => prettyPrintLog(log)).join('\n');
            }
            const status = result.status;
            const res: RunResult = { status, message, durationMS, coverageData };
            response.statusCode = 200;
            response.end(JSON.stringify(res));
          } else {
            response.statusCode = 404;
            response.end(`test case '${name}' not found`);
          }
        } catch (err) {
          response.statusCode = 500;
          response.end(`run failed with error: ${err}`);
        }
      } else if (request.url.startsWith(terminatePrefix)) {
        server.close();
        sys.exit(1);
      } else {
        response.statusCode = 404;
        response.end('unhandled url request');
      }
    }
  );

  server.listen(0, () => {
    const address = server.address() as AddressInfo;
    console.log(`Server listening at [[${address.port}]]`);
  });
})().catch(ex => {
  console.error(ex.stack ?? ex.toString());
  sys.exit(1);
});
