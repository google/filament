'use strict';

const assert = require('assert');

if (process.argv[2] === 'fatal') {
  const binding = require(process.argv[3]);
  binding.error.throwFatalError();
}

module.exports = require('./common').runTestWithBindingPath(test);

const napiVersion = Number(process.env.NAPI_VERSION ?? process.versions.napi);

function test (bindingPath) {
  const binding = require(bindingPath);
  binding.error.testErrorCopySemantics();
  binding.error.testErrorMoveSemantics();

  assert.throws(() => binding.error.throwApiError('test'), function (err) {
    return err instanceof Error && err.message.includes('Invalid');
  });

  assert.throws(() => binding.error.lastExceptionErrorCode(), function (err) {
    return err instanceof TypeError && err.message === 'A boolean was expected';
  });

  assert.throws(() => binding.error.throwJSError('test'), function (err) {
    return err instanceof Error && err.message === 'test';
  });

  assert.throws(() => binding.error.throwTypeErrorCStr('test'), function (err) {
    return err instanceof TypeError && err.message === 'test';
  });

  assert.throws(() => binding.error.throwRangeErrorCStr('test'), function (err) {
    return err instanceof RangeError && err.message === 'test';
  });

  assert.throws(() => binding.error.throwRangeError('test'), function (err) {
    return err instanceof RangeError && err.message === 'test';
  });

  assert.throws(() => binding.error.throwTypeErrorCtor(new TypeError('jsTypeError')), function (err) {
    return err instanceof TypeError && err.message === 'jsTypeError';
  });

  assert.throws(() => binding.error.throwRangeErrorCtor(new RangeError('rangeTypeError')), function (err) {
    return err instanceof RangeError && err.message === 'rangeTypeError';
  });

  if (napiVersion > 8) {
    assert.throws(() => binding.error.throwSyntaxErrorCStr('test'), function (err) {
      return err instanceof SyntaxError && err.message === 'test';
    });

    assert.throws(() => binding.error.throwSyntaxError('test'), function (err) {
      return err instanceof SyntaxError && err.message === 'test';
    });

    assert.throws(() => binding.error.throwSyntaxErrorCtor(new SyntaxError('syntaxTypeError')), function (err) {
      return err instanceof SyntaxError && err.message === 'syntaxTypeError';
    });
  }

  assert.throws(
    () => binding.error.doNotCatch(
      () => {
        throw new TypeError('test');
      }),
    function (err) {
      return err instanceof TypeError && err.message === 'test' && !err.caught;
    });

  assert.throws(
    () => binding.error.catchAndRethrowError(
      () => {
        throw new TypeError('test');
      }),
    function (err) {
      return err instanceof TypeError && err.message === 'test' && err.caught;
    });

  const err = binding.error.catchError(
    () => { throw new TypeError('test'); });
  assert(err instanceof TypeError);
  assert.strictEqual(err.message, 'test');

  const msg = binding.error.catchErrorMessage(
    () => { throw new TypeError('test'); });
  assert.strictEqual(msg, 'test');

  assert.throws(() => binding.error.throwErrorThatEscapesScope('test'), function (err) {
    return err instanceof Error && err.message === 'test';
  });

  assert.throws(() => binding.error.catchAndRethrowErrorThatEscapesScope('test'), function (err) {
    return err instanceof Error && err.message === 'test' && err.caught;
  });

  const p = require('./napi_child').spawnSync(
    process.execPath, [__filename, 'fatal', bindingPath]);
  assert.ifError(p.error);
  assert.ok(p.stderr.toString().includes(
    'FATAL ERROR: Error::ThrowFatalError This is a fatal error'));

  assert.throws(() => binding.error.throwDefaultError(false),
    /Cannot convert undefined or null to object/);

  assert.throws(() => binding.error.throwDefaultError(true),
    /A number was expected/);
}
