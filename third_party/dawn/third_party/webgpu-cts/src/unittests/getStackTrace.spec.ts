export const description = `
Tests for getStackTrace.
`;

import { makeTestGroup } from '../common/framework/test_group.js';
import { extractImportantStackTrace } from '../common/internal/stack.js';

import { UnitTest } from './unit_test.js';

export const g = makeTestGroup(UnitTest);

g.test('stacks')
  .paramsSimple([
    {
      case: 'node_fail',
      _expectedLines: 3,
      _stack: `Error:
   at CaseRecorder.fail (/Users/kainino/src/cts/src/common/framework/logger.ts:99:30)
   at RunCaseSpecific.exports.g.test.t [as fn] (/Users/kainino/src/cts/src/unittests/logger.spec.ts:80:7)
   at RunCaseSpecific.run (/Users/kainino/src/cts/src/common/framework/test_group.ts:121:18)
   at processTicksAndRejections (internal/process/task_queues.js:86:5)`,
    },
    {
      // MAINTENANCE_TODO: make sure this test case actually matches what happens on windows
      case: 'node_fail_backslash',
      _expectedLines: 3,
      _stack: `Error:
   at CaseRecorder.fail (C:\\Users\\kainino\\src\\cts\\src\\common\\framework\\logger.ts:99:30)
   at RunCaseSpecific.exports.g.test.t [as fn] (C:\\Users\\kainino\\src\\cts\\src\\unittests\\logger.spec.ts:80:7)
   at RunCaseSpecific.run (C:\\Users\\kainino\\src\\cts\\src\\common\\framework\\test_group.ts:121:18)
   at processTicksAndRejections (internal\\process\\task_queues.js:86:5)`,
    },
    {
      case: 'node_fail_processTicksAndRejections',
      _expectedLines: 5,
      _stack: `Error: expectation had no effect: suite1:foo:
    at Object.generateMinimalQueryList (/Users/kainino/src/cts/src/common/framework/generate_minimal_query_list.ts:72:24)
    at testGenerateMinimalQueryList (/Users/kainino/src/cts/src/unittests/loading.spec.ts:289:25)
    at processTicksAndRejections (internal/process/task_queues.js:93:5)
    at RunCaseSpecific.fn (/Users/kainino/src/cts/src/unittests/loading.spec.ts:300:3)
    at RunCaseSpecific.run (/Users/kainino/src/cts/src/common/framework/test_group.ts:144:9)
    at /Users/kainino/src/cts/src/common/runtime/cmdline.ts:62:25
    at async Promise.all (index 29)
    at /Users/kainino/src/cts/src/common/runtime/cmdline.ts:78:5`,
    },
    {
      case: 'node_throw',
      _expectedLines: 2,
      _stack: `Error: hello
    at RunCaseSpecific.g.test.t [as fn] (/Users/kainino/src/cts/src/unittests/test_group.spec.ts:51:11)
    at RunCaseSpecific.run (/Users/kainino/src/cts/src/common/framework/test_group.ts:121:18)
    at processTicksAndRejections (internal/process/task_queues.js:86:5)`,
    },
    {
      case: 'firefox_fail',
      _expectedLines: 3,
      _stack: `fail@http://localhost:8080/out/common/framework/logger.js:104:30
expect@http://localhost:8080/out/common/framework/default_fixture.js:59:16
@http://localhost:8080/out/unittests/util.spec.js:35:5
run@http://localhost:8080/out/common/framework/test_group.js:119:18`,
    },
    {
      case: 'firefox_throw',
      _expectedLines: 1,
      _stack: `@http://localhost:8080/out/unittests/test_group.spec.js:48:11
run@http://localhost:8080/out/common/framework/test_group.js:119:18`,
    },
    {
      case: 'safari_fail',
      _expectedLines: 3,
      _stack: `fail@http://localhost:8080/out/common/framework/logger.js:104:39
expect@http://localhost:8080/out/common/framework/default_fixture.js:59:20
http://localhost:8080/out/unittests/util.spec.js:35:11
http://localhost:8080/out/common/framework/test_group.js:119:20
asyncFunctionResume@[native code]
[native code]
promiseReactionJob@[native code]`,
    },
    {
      case: 'safari_throw',
      _expectedLines: 1,
      _stack: `http://localhost:8080/out/unittests/test_group.spec.js:48:20
http://localhost:8080/out/common/framework/test_group.js:119:20
asyncFunctionResume@[native code]
[native code]
promiseReactionJob@[native code]`,
    },
    {
      case: 'chrome_fail',
      _expectedLines: 4,
      _stack: `Error
    at CaseRecorder.fail (http://localhost:8080/out/common/framework/logger.js:104:30)
    at DefaultFixture.expect (http://localhost:8080/out/common/framework/default_fixture.js:59:16)
    at RunCaseSpecific.fn (http://localhost:8080/out/unittests/util.spec.js:35:5)
    at RunCaseSpecific.run (http://localhost:8080/out/common/framework/test_group.js:119:18)
    at async runCase (http://localhost:8080/out/common/runtime/standalone.js:37:17)
    at async http://localhost:8080/out/common/runtime/standalone.js:102:7`,
    },
    {
      case: 'chrome_throw',
      _expectedLines: 6,
      _stack: `Error: hello
    at RunCaseSpecific.fn (http://localhost:8080/out/unittests/test_group.spec.js:48:11)
    at RunCaseSpecific.run (http://localhost:8080/out/common/framework/test_group.js:119:18)"
    at async Promise.all (index 0)
    at async TestGroupTest.run (http://localhost:8080/out/unittests/test_group_test.js:6:5)
    at async RunCaseSpecific.fn (http://localhost:8080/out/unittests/test_group.spec.js:53:15)
    at async RunCaseSpecific.run (http://localhost:8080/out/common/framework/test_group.js:119:7)
    at async runCase (http://localhost:8080/out/common/runtime/standalone.js:37:17)
    at async http://localhost:8080/out/common/runtime/standalone.js:102:7`,
    },
    {
      case: 'multiple_lines',
      _expectedLines: 8,
      _stack: `Error: hello
    at RunCaseSpecific.fn (http://localhost:8080/out/unittests/test_group.spec.js:48:11)
    at RunCaseSpecific.fn (http://localhost:8080/out/unittests/test_group.spec.js:48:11)
    at RunCaseSpecific.fn (http://localhost:8080/out/unittests/test_group.spec.js:48:11)
    at RunCaseSpecific.run (http://localhost:8080/out/common/framework/test_group.js:119:18)"
    at async Promise.all (index 0)
    at async TestGroupTest.run (http://localhost:8080/out/unittests/test_group_test.js:6:5)
    at async RunCaseSpecific.fn (http://localhost:8080/out/unittests/test_group.spec.js:53:15)
    at async RunCaseSpecific.run (http://localhost:8080/out/common/framework/test_group.js:119:7)
    at async runCase (http://localhost:8080/out/common/runtime/standalone.js:37:17)
    at async http://localhost:8080/out/common/runtime/standalone.js:102:7`,
    },
  ])
  .fn(t => {
    const ex = new Error();
    ex.stack = t.params._stack;
    t.expect(ex.stack === t.params._stack);
    const stringified = extractImportantStackTrace(ex);
    const parts = stringified.split('\n');

    t.expect(parts.length === t.params._expectedLines);
    const last = parts[parts.length - 1];
    t.expect(last.indexOf('/unittests/') !== -1 || last.indexOf('\\unittests\\') !== -1);
  });
