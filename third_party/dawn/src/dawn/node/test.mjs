// This file can run with from the dawn folder, after building dawn node, with
// node src/dawn/node/test.mjs
// This assumes you have a symbolic link from out/active to your build folder.
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createRequire } from 'node:module';
import { describe, it, before, after } from 'node:test';

const require = createRequire(import.meta.url);

const __dirname = dirname(fileURLToPath(import.meta.url));
const dawnNodePath = join(__dirname, '..', '..', '..', 'out', 'active', 'dawn.node');
const { create, globals } = require(dawnNodePath);

// Save these before they might get replaced by the following assignment
const globalsBeforeGlobalAssignment = {
  DOMException,
  EventTarget,
  Error,
  Event,
};

Object.assign(globalThis, globals);

const assert = {
  ok(cond, msg) {
    if (typeof cond === 'function') {
      assert.ok(cond(), msg ?? cond.toString());
    } else if (!cond) {
      throw new Error(msg ?? '');
    }
  }
};

describe('tests', async () => {
  let device;
  let navigator;
  let adapter

  before(async () => {
    navigator = { gpu: create([]) };
    adapter = await navigator.gpu.requestAdapter();
    if (!adapter) {
      return;
    }

    device = await adapter.requestDevice({label: "test-device"});
  });

  after(async () => {
    device.destroy();
    device = undefined;
    navigator = undefined;
    adapter = undefined;

    await new Promise(r => setTimeout(r, 1000));
  })

  await describe('global tests', async () => {
    // Check that these globals were not replaced when we added dawn.node's globals
    assert.ok(() => globalsBeforeGlobalAssignment.EventTarget === EventTarget);
    assert.ok(() => globalsBeforeGlobalAssignment.Event === Event);
    assert.ok(() => globalsBeforeGlobalAssignment.Error === Error);
    assert.ok(() => globalsBeforeGlobalAssignment.DOMException === DOMException);
  });

  await describe('device tests', async () => {

    await it('basic device tests', () => {
      assert.ok(device.adapterInfo.description !== undefined);
      assert.ok(device.label === 'test-device');
      assert.ok(() => device instanceof EventTarget);
    });

    await it('can not construct device', () => {
      // check we can't make things we're not supposed to be able to.
      let err;
      try {
        const gd = new GPUDevice();
        assert(gd || !gd, 'should never get here');  // use gd
      } catch (e) {
        err = e;
      }
      assert.ok(() => err instanceof TypeError);
    });

  });

  await describe('popErrorScope ', async () => {

    await it('popErrorScope returns error', async () => {
      device.pushErrorScope('validation');
      device.createTexture({
        format: 'rgba8unorm',
        usage: GPUTextureUsage.TEXTURE_BINDING,
        size: [device.limits.maxTextureDimension2D + 1],
      });
      const err = await device.popErrorScope();
      assert.ok(() => err instanceof GPUValidationError);
    });

    await it('popErrorScope twice returns error', async () => {
      device.pushErrorScope('out-of-memory');
      device.pushErrorScope('internal');
      device.pushErrorScope('validation');
      device.pushErrorScope('validation');
      device.createTexture({
        format: 'rgba8unorm',
        usage: GPUTextureUsage.TEXTURE_BINDING,
        size: [device.limits.maxTextureDimension2D + 1],
      });
      const err1 = await device.popErrorScope();
      assert.ok(() => err1 instanceof GPUValidationError);
      const err2 = await device.popErrorScope();
      assert.ok(() => !err2);
      const err3 = await device.popErrorScope();
      assert.ok(() => !err3);
      const err4 = await device.popErrorScope();
      assert.ok(() => !err4);
    });

    await it('works when popErrorScope on empty stack', async () => {
      try {
        await device.popErrorScope();
        assert.ok(false, 'should not get here');
      } catch (e) {
        assert.ok(e.name === 'OperationError');
      }
    })

  });

  await describe('error tests', { skip: false }, async () => {

    await it('basic error tests', () => {
      const e = new GPUValidationError('test1');
      assert.ok(() => e instanceof GPUError);
      assert.ok(() => e.message === 'test1');
    });

  });

  await describe('GPUPipelineError tests', async () => {

    await it('can create a GPUPipelineError', () => {
      const e = new GPUPipelineError("msg1", { reason: 'validation' });
      assert.ok(() => e instanceof GPUPipelineError);
      assert.ok(() => e instanceof DOMException);
      assert.ok(() => e.name === 'GPUPipelineError');
      assert.ok(() => e.message === 'msg1')
      assert.ok(() => e.reason === 'validation')
    });

    await it('generates GPUPipelineError', { skip: false }, async () => {

      const vertexModule = device.createShaderModule({
        code: `
        @group(0) @binding(0) var<uniform> myUniform : vec4f;
        @vertex fn vs() -> @builtin(position) vec4f {
          return myUniform;
        }
        `,
      });

      const fragmentModule = device.createShaderModule({
        code: `
        @group(0) @binding(0) var tex: texture_2d<f32>;
        @fragment fn fs() -> @location(0) vec4f {
          return textureLoad(tex, vec2u(0), 0);
        }
        `,
      });

      const promise = device.createRenderPipelineAsync({
        layout: 'auto',
        vertex: { module: vertexModule },
        fragment: { module: fragmentModule, targets: [ { format: 'rgba8unorm' } ]},
      });

      try {
        const pipeline = await promise;
        assert(false, 'should not get here');
      } catch (e) {
        //console.log(e);
        //console.log('name:', e.name);
        //console.log('message:', e.message);
        //console.log('reason:', e.reason);
        assert.ok(() => e instanceof GPUPipelineError);
        assert.ok(() => e instanceof DOMException);
        assert.ok(() => e.reason === 'validation');
      }

    });

  });

  await describe('event tests', { skip: false }, async () => {

    await it('can pass Event through device', async () => {
      const e = await new Promise(resolve => {
        device.addEventListener('uncapturedFoo', resolve);
        device.dispatchEvent(new Event('uncapturedFoo', { error: { message: 'yo (1)!'} } ));
      });
      assert.ok(!!e, 'Event worked');
    });

    await it('can pass CustomEvent through device', async () => {
      const e = await new Promise(resolve => {
        device.addEventListener('uncapturedEvent', resolve, { once: true });
        device.dispatchEvent(new CustomEvent('uncapturedEvent', { detail: {error: { message: 'yo!'} } }));
      });
      assert.ok(e?.detail.error.message === 'yo!', 'CustomEvent worked');
    });

    await it('can pass extended Event through device', async () => {
      class MyEvent extends Event {
        constructor(type, msg) {
          super(type);
          this.message = msg;
        }
      }
      const target = new EventTarget();
      const e = await new Promise(resolve => {
        target.addEventListener('custom', resolve, { once: true });
        target.dispatchEvent(new MyEvent('custom', 'hello'));
      });
      assert.ok(() => e.message === 'hello');
    });

    await it('can pass dispatch user create GPUUncapturedErrorEvent', async () => {
      const gEvent = new GPUUncapturedErrorEvent(
        'uncapturederror',
        { error: new GPUValidationError('test2') }
      );

      assert.ok(() => gEvent instanceof Event);
      assert.ok(() => gEvent.error.message === 'test2');

      const e = await new Promise(resolve => {
        device.addEventListener('uncapturederror', resolve, { once: true } );
        device.dispatchEvent(gEvent);
      });
      assert.ok(e?.error.message === 'test2');
    });

    await it('receives uncaptured events', async () => {
      const e = await new Promise(resolve => {
        device.addEventListener('uncapturederror', (e) => {
          e.preventDefault();
          resolve(e);
        }, { once: true, passive: false });
        const texture = device.createTexture({
          format: 'rgba8unorm',
          usage: GPUTextureUsage.TEXTURE_BINDING,
          size: [device.limits.maxTextureDimension2D + 1, 1],
        });
        texture.destroy();
      });
      assert.ok(() => e?.error.message.includes('maxTextureDimension2D'));
    });

  });

});
