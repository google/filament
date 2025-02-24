/* Test helpers ported from test/common/index.js in Node.js project. */
'use strict';
const assert = require('assert');
const path = require('path');
const { access } = require('node:fs/promises');
const { spawn } = require('child_process');
const { EOL } = require('os');
const readline = require('readline');

const escapeBackslashes = (pathString) => pathString.split('\\').join('\\\\');

const noop = () => {};

const mustCallChecks = [];

function runCallChecks (exitCode) {
  if (exitCode !== 0) return;

  const failed = mustCallChecks.filter(function (context) {
    if ('minimum' in context) {
      context.messageSegment = `at least ${context.minimum}`;
      return context.actual < context.minimum;
    } else {
      context.messageSegment = `exactly ${context.exact}`;
      return context.actual !== context.exact;
    }
  });

  failed.forEach(function (context) {
    console.log('Mismatched %s function calls. Expected %s, actual %d.',
      context.name,
      context.messageSegment,
      context.actual);
    console.log(context.stack.split(EOL).slice(2).join(EOL));
  });

  if (failed.length) process.exit(1);
}

exports.installAysncHooks = function (asyncResName) {
  const asyncHooks = require('async_hooks');
  return new Promise((resolve, reject) => {
    let id;
    const events = [];
    /**
     * TODO(legendecas): investigate why resolving & disabling hooks in
     * destroy callback causing crash with case 'callbackscope.js'.
     */
    let destroyed = false;
    const hook = asyncHooks.createHook({
      init (asyncId, type, triggerAsyncId, resource) {
        if (id === undefined && type === asyncResName) {
          id = asyncId;
          events.push({ eventName: 'init', type, triggerAsyncId, resource });
        }
      },
      before (asyncId) {
        if (asyncId === id) {
          events.push({ eventName: 'before' });
        }
      },
      after (asyncId) {
        if (asyncId === id) {
          events.push({ eventName: 'after' });
        }
      },
      destroy (asyncId) {
        if (asyncId === id) {
          events.push({ eventName: 'destroy' });
          destroyed = true;
        }
      }
    }).enable();

    const interval = setInterval(() => {
      if (destroyed) {
        hook.disable();
        clearInterval(interval);
        resolve(events);
      }
    }, 10);
  });
};

exports.mustCall = function (fn, exact) {
  return _mustCallInner(fn, exact, 'exact');
};
exports.mustCallAtLeast = function (fn, minimum) {
  return _mustCallInner(fn, minimum, 'minimum');
};

function _mustCallInner (fn, criteria, field) {
  if (typeof fn === 'number') {
    criteria = fn;
    fn = noop;
  } else if (fn === undefined) {
    fn = noop;
  }
  if (criteria === undefined) {
    criteria = 1;
  }

  if (typeof criteria !== 'number') { throw new TypeError(`Invalid ${field} value: ${criteria}`); }

  const context = {
    [field]: criteria,
    actual: 0,
    stack: (new Error()).stack,
    name: fn.name || '<anonymous>'
  };

  // add the exit listener only once to avoid listener leak warnings
  if (mustCallChecks.length === 0) process.on('exit', runCallChecks);

  mustCallChecks.push(context);

  return function () {
    context.actual++;
    return fn.apply(this, arguments);
  };
}

exports.mustNotCall = function (msg) {
  return function mustNotCall () {
    assert.fail(msg || 'function should not have been called');
  };
};

const buildTypes = {
  Release: 'Release',
  Debug: 'Debug'
};

async function checkBuildType (buildType) {
  try {
    await access(path.join(path.resolve('./test/build'), buildType));
    return true;
  } catch {
    return false;
  }
}

async function whichBuildType () {
  let buildType = 'Release';
  const envBuildType = process.env.NODE_API_BUILD_CONFIG || (process.env.npm_config_debug === 'true' ? 'Debug' : 'Release');
  if (envBuildType) {
    if (Object.values(buildTypes).includes(envBuildType)) {
      if (await checkBuildType(envBuildType)) {
        buildType = envBuildType;
      } else {
        throw new Error(`The ${envBuildType} build doesn't exist.`);
      }
    } else {
      throw new Error('Invalid value for NODE_API_BUILD_CONFIG environment variable. It should be set to Release or Debug.');
    }
  }
  return buildType;
}

exports.whichBuildType = whichBuildType;

exports.runTest = async function (test, buildType, buildPathRoot = process.env.BUILD_PATH || '') {
  buildType = buildType || await whichBuildType();
  const bindings = [
    path.join(buildPathRoot, `../build/${buildType}/binding.node`),
    path.join(buildPathRoot, `../build/${buildType}/binding_noexcept.node`),
    path.join(buildPathRoot, `../build/${buildType}/binding_noexcept_maybe.node`),
    path.join(buildPathRoot, `../build/${buildType}/binding_custom_namespace.node`)
  ].map(it => require.resolve(it));

  for (const item of bindings) {
    await Promise.resolve(test(require(item), { bindingPath: item }))
      .finally(exports.mustCall());
  }
};

exports.runTestWithBindingPath = async function (test, buildType, buildPathRoot = process.env.BUILD_PATH || '') {
  buildType = buildType || await whichBuildType();

  const bindings = [
    path.join(buildPathRoot, `../build/${buildType}/binding.node`),
    path.join(buildPathRoot, `../build/${buildType}/binding_noexcept.node`),
    path.join(buildPathRoot, `../build/${buildType}/binding_noexcept_maybe.node`),
    path.join(buildPathRoot, `../build/${buildType}/binding_custom_namespace.node`)
  ].map(it => require.resolve(it));

  for (const item of bindings) {
    await test(item);
  }
};

exports.runTestWithBuildType = async function (test, buildType) {
  buildType = buildType || await whichBuildType();

  await Promise.resolve(test(buildType))
    .finally(exports.mustCall());
};

// Some tests have to run in their own process, otherwise they would interfere
// with each other. Such tests export a factory function rather than the test
// itself so as to avoid automatic instantiation, and therefore interference,
// in the main process. Two examples are addon and addon_data, both of which
// use Napi::Env::SetInstanceData(). This helper function provides a common
// approach for running such tests.
exports.runTestInChildProcess = function ({ suite, testName, expectedStderr, execArgv }) {
  return exports.runTestWithBindingPath((bindingName) => {
    return new Promise((resolve) => {
      bindingName = escapeBackslashes(bindingName);
      // Test suites are assumed to be located here.
      const suitePath = escapeBackslashes(path.join(__dirname, '..', 'child_processes', suite));
      const child = spawn(process.execPath, [
        '--expose-gc',
        ...(execArgv ?? []),
        '-e',
        `require('${suitePath}').${testName}(require('${bindingName}'))`
      ]);
      const resultOfProcess = { stderr: [] };

      // Capture the exit code and signal.
      child.on('close', (code, signal) => resolve(Object.assign(resultOfProcess, { code, signal })));

      // Capture the stderr as an array of lines.
      readline
        .createInterface({ input: child.stderr })
        .on('line', (line) => {
          resultOfProcess.stderr.push(line);
        });
    }).then(actual => {
      // Back up the stderr in case the assertion fails.
      const fullStderr = actual.stderr.map(item => `from child process: ${item}`);
      const expected = { stderr: expectedStderr, code: 0, signal: null };

      if (!expectedStderr) {
        // If we don't care about stderr, delete it.
        delete actual.stderr;
        delete expected.stderr;
      } else {
        // Otherwise we only care about expected lines in the actual stderr, so
        // filter out everything else.
        actual.stderr = actual.stderr.filter(line => expectedStderr.includes(line));
      }

      assert.deepStrictEqual(actual, expected, `Assertion for child process test ${suite}.${testName} failed:${EOL}` + fullStderr.join(EOL));
    });
  });
};
