import { setBaseResourcePath } from '../../framework/resources.js';
import { DefaultTestFileLoader } from '../../internal/file_loader.js';
import { parseQuery } from '../../internal/query/parseQuery.js';
import { assert } from '../../util/util.js';

import { setupWorkerEnvironment, WorkerTestRunRequest } from './utils_worker.js';

// Should be WorkerGlobalScope, but importing lib "webworker" conflicts with lib "dom".
/* eslint-disable-next-line @typescript-eslint/no-explicit-any */
declare const self: any;

const loader = new DefaultTestFileLoader();

setBaseResourcePath('../../../resources');

async function reportTestResults(this: MessagePort | Worker, ev: MessageEvent) {
  const { query, expectations, ctsOptions } = ev.data as WorkerTestRunRequest;

  const log = setupWorkerEnvironment(ctsOptions);

  const testcases = Array.from(await loader.loadCases(parseQuery(query)));
  assert(testcases.length === 1, 'worker query resulted in != 1 cases');

  const testcase = testcases[0];
  const [rec, result] = log.record(testcase.query.toString());
  await testcase.run(rec, expectations);

  this.postMessage({
    query,
    result: {
      ...result,
      logs: result.logs?.map(l => l.toRawData()),
    },
  });
}

self.onmessage = (ev: MessageEvent) => {
  void reportTestResults.call(ev.source || self, ev);
};

self.onconnect = (event: MessageEvent) => {
  const port = event.ports[0];

  port.onmessage = (ev: MessageEvent) => {
    void reportTestResults.call(port, ev);
  };
};
