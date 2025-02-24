export const description = `
Unit tests for namespaced logging system.

Also serves as a larger test of async test functions, and of the logging system.
`;

import { SkipTestCase } from '../common/framework/fixture.js';
import { makeTestGroup } from '../common/framework/test_group.js';
import { Logger } from '../common/internal/logging/logger.js';
import { assert } from '../common/util/util.js';

import { UnitTest } from './unit_test.js';

export const g = makeTestGroup(UnitTest);

g.test('construct').fn(t => {
  const mylog = new Logger({ overrideDebugMode: true });
  const [, res1] = mylog.record('one');
  const [, res2] = mylog.record('two');

  t.expect(mylog.results.get('one') === res1);
  t.expect(mylog.results.get('two') === res2);
  t.expect(res1.logs === undefined);
  t.expect(res1.status === 'running');
  t.expect(res1.timems < 0);
  t.expect(res2.logs === undefined);
  t.expect(res2.status === 'running');
  t.expect(res2.timems < 0);
});

g.test('empty').fn(t => {
  const mylog = new Logger({ overrideDebugMode: true });
  const [rec, res] = mylog.record('one');

  rec.start();
  t.expect(res.status === 'running');
  rec.finish();

  t.expect(res.status === 'notrun');
  t.expect(res.timems >= 0);
});

g.test('passed').fn(t => {
  const mylog = new Logger({ overrideDebugMode: true });
  const [rec, res] = mylog.record('one');

  rec.start();
  rec.passed();
  rec.finish();

  t.expect(res.status === 'pass');
  t.expect(res.timems >= 0);
});

g.test('pass').fn(t => {
  const mylog = new Logger({ overrideDebugMode: true });
  const [rec, res] = mylog.record('one');

  rec.start();
  rec.debug(new Error('hello'));
  t.expect(res.status === 'running');
  rec.finish();

  t.expect(res.status === 'pass');
  t.expect(res.timems >= 0);
});

g.test('skip').fn(t => {
  const mylog = new Logger({ overrideDebugMode: true });
  const [rec, res] = mylog.record('one');

  rec.start();
  rec.skipped(new SkipTestCase());
  rec.finish();

  t.expect(res.status === 'skip');
  t.expect(res.timems >= 0);
});

// Tests if there's some skips and at least one pass it's pass.
g.test('skip_pass').fn(t => {
  const mylog = new Logger({ overrideDebugMode: true });
  const [rec, res] = mylog.record('one');

  rec.start();
  rec.skipped(new SkipTestCase());
  rec.debug(new Error('hello'));
  rec.skipped(new SkipTestCase());
  rec.finish();

  t.expect(res.status === 'pass');
  t.expect(res.timems >= 0);
});

g.test('warn').fn(t => {
  const mylog = new Logger({ overrideDebugMode: true });
  const [rec, res] = mylog.record('one');

  rec.start();
  rec.warn(new Error('hello'));
  rec.skipped(new SkipTestCase());
  rec.finish();

  t.expect(res.status === 'warn');
  t.expect(res.timems >= 0);
});

g.test('fail,expectationFailed').fn(t => {
  const mylog = new Logger({ overrideDebugMode: true });
  const [rec, res] = mylog.record('one');

  rec.start();
  rec.expectationFailed(new Error('bye'));
  rec.warn(new Error());
  rec.skipped(new SkipTestCase());
  rec.finish();

  t.expect(res.status === 'fail');
  t.expect(res.timems >= 0);
});

g.test('fail,validationFailed').fn(t => {
  const mylog = new Logger({ overrideDebugMode: true });
  const [rec, res] = mylog.record('one');

  rec.start();
  rec.validationFailed(new Error('bye'));
  rec.warn(new Error());
  rec.skipped(new SkipTestCase());
  rec.finish();

  t.expect(res.status === 'fail');
  t.expect(res.timems >= 0);
});

g.test('fail,threw').fn(t => {
  const mylog = new Logger({ overrideDebugMode: true });
  const [rec, res] = mylog.record('one');

  rec.start();
  rec.threw(new Error('bye'));
  rec.warn(new Error());
  rec.skipped(new SkipTestCase());
  rec.finish();

  t.expect(res.status === 'fail');
  t.expect(res.timems >= 0);
});

g.test('debug')
  .paramsSimple([
    { debug: true, _logsCount: 5 }, //
    { debug: false, _logsCount: 3 },
  ])
  .fn(t => {
    const { debug, _logsCount } = t.params;

    const mylog = new Logger({ overrideDebugMode: debug });
    const [rec, res] = mylog.record('one');

    rec.start();
    rec.debug(new Error('hello'));
    rec.expectationFailed(new Error('bye'));
    rec.warn(new Error());
    rec.skipped(new SkipTestCase());
    rec.debug(new Error('foo'));
    rec.finish();

    t.expect(res.status === 'fail');
    t.expect(res.timems >= 0);
    assert(res.logs !== undefined);
    t.expect(res.logs.length === _logsCount);
  });
