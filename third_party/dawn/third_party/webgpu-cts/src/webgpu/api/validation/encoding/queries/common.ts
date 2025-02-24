import { GPUTest } from '../../../../gpu_test.js';

export function createQuerySetWithType(
  t: GPUTest,
  type: GPUQueryType,
  count: GPUSize32
): GPUQuerySet {
  return t.createQuerySetTracked({
    type,
    count,
  });
}

export function beginRenderPassWithQuerySet(
  t: GPUTest,
  encoder: GPUCommandEncoder,
  querySet?: GPUQuerySet
): GPURenderPassEncoder {
  const view = t
    .createTextureTracked({
      format: 'rgba8unorm' as const,
      size: { width: 16, height: 16, depthOrArrayLayers: 1 },
      usage: GPUTextureUsage.RENDER_ATTACHMENT,
    })
    .createView();
  return encoder.beginRenderPass({
    colorAttachments: [
      {
        view,
        clearValue: { r: 1.0, g: 0.0, b: 0.0, a: 1.0 },
        loadOp: 'clear',
        storeOp: 'store',
      },
    ],
    occlusionQuerySet: querySet,
  });
}
