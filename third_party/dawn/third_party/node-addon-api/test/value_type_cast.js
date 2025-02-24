'use strict';

const assert = require('assert');
const napiChild = require('./napi_child');

module.exports = require('./common').runTestWithBuildType(test);

function test (buildType) {
  const binding = require(`./build/${buildType}/binding_type_check.node`);
  const testTable = {
    typeCastBoolean: {
      positiveValues: [true, false],
      negativeValues: [{}, [], 1, 1n, 'true', null, undefined]
    },
    typeCastNumber: {
      positiveValues: [1, NaN],
      negativeValues: [{}, [], true, 1n, '1', null, undefined]
    },
    typeCastBigInt: {
      positiveValues: [1n],
      negativeValues: [{}, [], true, 1, '1', null, undefined]
    },
    typeCastDate: {
      positiveValues: [new Date()],
      negativeValues: [{}, [], true, 1, 1n, '1', null, undefined]
    },
    typeCastString: {
      positiveValues: ['', '1'],
      negativeValues: [{}, [], true, 1, 1n, null, undefined]
    },
    typeCastSymbol: {
      positiveValues: [Symbol('1')],
      negativeValues: [{}, [], true, 1, 1n, '1', null, undefined]
    },
    typeCastObject: {
      positiveValues: [{}, new Date(), []],
      negativeValues: [true, 1, 1n, '1', null, undefined]
    },
    typeCastArray: {
      positiveValues: [[1]],
      negativeValues: [{}, true, 1, 1n, '1', null, undefined]
    },
    typeCastArrayBuffer: {
      positiveValues: [new ArrayBuffer(0)],
      negativeValues: [new Uint8Array(1), {}, [], null, undefined]
    },
    typeCastTypedArray: {
      positiveValues: [new Uint8Array(0)],
      negativeValues: [new ArrayBuffer(1), {}, [], null, undefined]
    },
    typeCastDataView: {
      positiveValues: [new DataView(new ArrayBuffer(0))],
      negativeValues: [new ArrayBuffer(1), null, undefined]
    },
    typeCastFunction: {
      positiveValues: [() => {}],
      negativeValues: [{}, null, undefined]
    },
    typeCastPromise: {
      positiveValues: [Promise.resolve()],
      // napi_is_promise distinguishes Promise and thenable.
      negativeValues: [{ then: () => {} }, null, undefined]
    },
    typeCastBuffer: {
      positiveValues: [Buffer.from('')],
      // napi_is_buffer doesn't distinguish between Buffer and TypedArrays.
      negativeValues: [new ArrayBuffer(1), null, undefined]
    },
    typeCastExternal: {
      positiveValues: [binding.external],
      negativeValues: [{}, null, undefined]
    },
    typeCastTypeArrayOfUint8: {
      // TypedArrayOf<uint8_t>::CheckCast doesn't distinguish between Uint8ClampedArray and Uint8Array.
      positiveValues: [new Uint8Array(0), new Uint8ClampedArray(0)],
      negativeValues: [new Int8Array(1), null, undefined]
    }
  };

  if (process.argv[2] === 'child') {
    child(binding, testTable, process.argv[3], process.argv[4], parseInt(process.argv[5]));
    return;
  }

  for (const [methodName, { positiveValues, negativeValues }] of Object.entries(testTable)) {
    for (const idx of positiveValues.keys()) {
      const { status } = napiChild.spawnSync(process.execPath, [__filename, 'child', methodName, 'positiveValues', idx]);
      assert.strictEqual(status, 0, `${methodName} positive value ${idx} test failed`);
    }
    for (const idx of negativeValues.keys()) {
      const { status, signal, stderr } = napiChild.spawnSync(process.execPath, [__filename, 'child', methodName, 'negativeValues', idx], {
        encoding: 'utf8'
      });
      if (process.platform === 'win32') {
        assert.strictEqual(status, 128 + 6 /* SIGABRT */, `${methodName} negative value ${idx} test failed`);
      } else {
        assert.strictEqual(signal, 'SIGABRT', `${methodName} negative value ${idx} test failed`);
      }
      assert.ok(stderr.match(/FATAL ERROR: .*::CheckCast.*/));
    }
  }
}

async function child (binding, testTable, methodName, type, idx) {
  binding[methodName](testTable[methodName][type][idx]);
}
