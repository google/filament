/* eslint-disable no-lone-blocks */
'use strict';

const assert = require('assert');
const testUtil = require('./testUtil');

module.exports = require('./common').runTest(test);

async function test (binding) {
  const Test = binding.objectwrap.Test;

  const testValue = (obj, clazz) => {
    assert.strictEqual(obj.testValue, true);
    assert.strictEqual(obj[clazz.kTestValueInternal], false);
  };

  const testAccessor = (obj, clazz) => {
    // read-only, write-only
    {
      obj.testSetter = 'instance getter';
      assert.strictEqual(obj.testGetter, 'instance getter');
      assert.strictEqual(obj.testGetterT, 'instance getter');

      obj.testSetter = 'instance getter 2';
      assert.strictEqual(obj.testGetter, 'instance getter 2');
      assert.strictEqual(obj.testGetterT, 'instance getter 2');

      assert.throws(() => clazz.prototype.testGetter, /Invalid argument/);
      assert.throws(() => clazz.prototype.testGetterT, /Invalid argument/);
    }

    // read write-only
    {
      let error;
      // eslint-disable-next-line no-unused-vars
      try { const read = obj.testSetter; } catch (e) { error = e; }
      // no error
      assert.strictEqual(error, undefined);

      // read is undefined
      assert.strictEqual(obj.testSetter, undefined);
    }

    // write read-only
    {
      let error;
      try { obj.testGetter = 'write'; } catch (e) { error = e; }
      assert.strictEqual(error.name, 'TypeError');

      try { obj.testGetterT = 'write'; } catch (e) { error = e; }
      assert.strictEqual(error.name, 'TypeError');
    }

    // rw
    {
      obj.testGetSet = 'instance getset';
      assert.strictEqual(obj.testGetSet, 'instance getset');

      obj.testGetSet = 'instance getset 2';
      assert.strictEqual(obj.testGetSet, 'instance getset 2');

      obj.testGetSetT = 'instance getset 3';
      assert.strictEqual(obj.testGetSetT, 'instance getset 3');

      obj.testGetSetT = 'instance getset 4';
      assert.strictEqual(obj.testGetSetT, 'instance getset 4');

      assert.throws(() => { clazz.prototype.testGetSet = 'instance getset'; }, /Invalid argument/);
      assert.throws(() => { clazz.prototype.testGetSetT = 'instance getset'; }, /Invalid argument/);
    }

    // rw symbol
    {
      obj[clazz.kTestAccessorInternal] = 'instance internal getset';
      assert.strictEqual(obj[clazz.kTestAccessorInternal], 'instance internal getset');

      obj[clazz.kTestAccessorInternal] = 'instance internal getset 2';
      assert.strictEqual(obj[clazz.kTestAccessorInternal], 'instance internal getset 2');

      obj[clazz.kTestAccessorTInternal] = 'instance internal getset 3';
      assert.strictEqual(obj[clazz.kTestAccessorTInternal], 'instance internal getset 3');

      obj[clazz.kTestAccessorTInternal] = 'instance internal getset 4';
      assert.strictEqual(obj[clazz.kTestAccessorTInternal], 'instance internal getset 4');
    }

    // own property
    {
      obj.testSetter = 'own property value';
      // Make sure the properties are enumerable.
      assert(Object.getOwnPropertyNames(obj).indexOf('ownProperty') >= 0);
      assert(Object.getOwnPropertyNames(obj).indexOf('ownPropertyT') >= 0);

      // Make sure the properties return the right value.
      assert.strictEqual(obj.ownProperty, 'own property value');
      assert.strictEqual(obj.ownPropertyT, 'own property value');
    }
  };

  const testMethod = (obj, clazz) => {
    assert.strictEqual(obj.testMethod('method'), 'method instance');
    assert.strictEqual(obj[clazz.kTestMethodInternal]('method'), 'method instance internal');
    obj.testVoidMethodT('method<>(const char*)');
    assert.strictEqual(obj.testMethodT(), 'method<>(const char*)');
    obj[clazz.kTestVoidMethodTInternal]('method<>(Symbol)');
    assert.strictEqual(obj[clazz.kTestMethodTInternal](), 'method<>(Symbol)');
    assert.throws(() => clazz.prototype.testMethod('method'));
    assert.throws(() => clazz.prototype.testMethodT());
    assert.throws(() => clazz.prototype.testVoidMethodT('method<>(const char*)'));
  };

  const testEnumerables = (obj, clazz) => {
    // Object.keys: only object without prototype
    assert(Object.keys(obj).length === 2);
    assert(Object.keys(obj).includes('ownProperty'));
    assert(Object.keys(obj).indexOf('ownPropertyT') >= 0);

    // for..in: object and prototype
    {
      const keys = [];
      for (const key in obj) {
        keys.push(key);
      }

      assert(keys.length === 6);
      // on prototype
      assert(keys.includes('testGetSet'));
      assert(keys.includes('testGetter'));
      assert(keys.includes('testValue'));
      assert(keys.includes('testMethod'));
      // on object only
      assert(keys.includes('ownProperty'));
      assert(keys.includes('ownPropertyT'));
    }
  };

  const testConventions = (obj, clazz) => {
    // test @@toStringTag
    {
      assert.strictEqual(obj[Symbol.toStringTag], 'TestTag');
      assert.strictEqual('' + obj, '[object TestTag]');
    }

    // test @@iterator
    {
      obj.testSetter = 'iterator';
      const values = [];

      for (const item of obj) {
        values.push(item);
      }

      assert.deepEqual(values, ['iterator']);
    }
  };

  const testStaticValue = (clazz) => {
    assert.strictEqual(clazz.testStaticValue, 'value');
    assert.strictEqual(clazz[clazz.kTestStaticValueInternal], 5);
  };

  const testStaticAccessor = (clazz) => {
    // read-only, write-only
    {
      const tempObj = {};
      clazz.testStaticSetter = tempObj;
      assert.strictEqual(clazz.testStaticGetter, tempObj);
      assert.strictEqual(clazz.testStaticGetterT, tempObj);

      const tempArray = [];
      clazz.testStaticSetter = tempArray;
      assert.strictEqual(clazz.testStaticGetter, tempArray);
      assert.strictEqual(clazz.testStaticGetterT, tempArray);
    }

    // read write-only
    {
      let error;
      // eslint-disable-next-line no-unused-vars
      try { const read = clazz.testStaticSetter; } catch (e) { error = e; }
      // no error
      assert.strictEqual(error, undefined);

      // read is undefined
      assert.strictEqual(clazz.testStaticSetter, undefined);
    }

    // write-read-only
    {
      let error;
      try { clazz.testStaticGetter = 'write'; } catch (e) { error = e; }
      assert.strictEqual(error.name, 'TypeError');
      try { clazz.testStaticGetterT = 'write'; } catch (e) { error = e; }
      assert.strictEqual(error.name, 'TypeError');
    }

    // rw
    {
      clazz.testStaticGetSet = 9;
      assert.strictEqual(clazz.testStaticGetSet, 9);

      clazz.testStaticGetSet = 4;
      assert.strictEqual(clazz.testStaticGetSet, 4);

      clazz.testStaticGetSetT = -9;
      assert.strictEqual(clazz.testStaticGetSetT, -9);

      clazz.testStaticGetSetT = -4;
      assert.strictEqual(clazz.testStaticGetSetT, -4);
    }

    // rw symbol
    {
      clazz[clazz.kTestStaticAccessorInternal] = 'static internal getset';
      assert.strictEqual(clazz[clazz.kTestStaticAccessorInternal], 'static internal getset');
      clazz[clazz.kTestStaticAccessorTInternal] = 'static internal getset <>';
      assert.strictEqual(clazz[clazz.kTestStaticAccessorTInternal], 'static internal getset <>');
    }
  };

  const testStaticMethod = (clazz) => {
    clazz.testStaticVoidMethod(52);
    assert.strictEqual(clazz.testStaticGetter, 52);
    clazz[clazz.kTestStaticVoidMethodInternal](94);
    assert.strictEqual(clazz.testStaticGetter, 94);
    assert.strictEqual(clazz.testStaticMethod('method'), 'method static');
    assert.strictEqual(clazz[clazz.kTestStaticMethodInternal]('method'), 'method static internal');
    clazz.testStaticVoidMethodT('static method<>(const char*)');
    assert.strictEqual(clazz.testStaticMethodT(), 'static method<>(const char*)');
    clazz[clazz.kTestStaticVoidMethodTInternal]('static method<>(Symbol)');
    assert.strictEqual(clazz[clazz.kTestStaticMethodTInternal](), 'static method<>(Symbol)');
  };

  const testStaticEnumerables = (clazz) => {
    // Object.keys
    assert.deepEqual(Object.keys(clazz), [
      'testStaticValue',
      'testStaticGetter',
      'testStaticGetSet',
      'testStaticMethod',
      'canUnWrap'
    ]);

    // for..in
    {
      const keys = [];
      for (const key in clazz) {
        keys.push(key);
      }

      assert.deepEqual(keys, [
        'testStaticValue',
        'testStaticGetter',
        'testStaticGetSet',
        'testStaticMethod',
        'canUnWrap'
      ]);
    }
  };

  async function testFinalize (clazz) {
    let finalizeCalled = false;
    await testUtil.runGCTests([
      'test finalize',
      () => {
        const finalizeCb = function (called) {
          finalizeCalled = called;
        };

        // Scope Test instance so that it can be gc'd.
        // eslint-disable-next-line no-new
        (() => { new Test(finalizeCb); })();
      },
      () => assert.strictEqual(finalizeCalled, true)
    ]);
  }

  const testUnwrap = (obj, clazz) => {
    obj.testSetter = 'unwrapTest';
    assert(clazz.canUnWrap(obj, 'unwrapTest'));
  };

  const testObj = (obj, clazz) => {
    testValue(obj, clazz);
    testAccessor(obj, clazz);
    testMethod(obj, clazz);

    testEnumerables(obj, clazz);

    testConventions(obj, clazz);
    testUnwrap(obj, clazz);
  };

  async function testClass (clazz) {
    testStaticValue(clazz);
    testStaticAccessor(clazz);
    testStaticMethod(clazz);

    testStaticEnumerables(clazz);
    await testFinalize(clazz);
  }

  // `Test` is needed for accessing exposed symbols
  testObj(new Test(), Test);
  await testClass(Test);

  // Make sure the C++ object can be garbage collected without issues.
  await testUtil.runGCTests(['one last gc', () => {}, () => {}]);
}
