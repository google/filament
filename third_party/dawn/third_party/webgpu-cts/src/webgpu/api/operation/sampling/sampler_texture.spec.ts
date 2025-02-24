export const description = `
Tests samplers with textures.

- test that you can use the maximum number of textures
  with the maximum number of samplers.
`;

import { makeTestGroup } from '../../../../common/framework/test_group.js';
import { assert, range } from '../../../../common/util/util.js';
import { GPUTest, MaxLimitsTestMixin, TextureTestMixin } from '../../../gpu_test.js';
import { TexelView } from '../../../util/texture/texel_view.js';

export const g = makeTestGroup(TextureTestMixin(MaxLimitsTestMixin(GPUTest)));

g.test('sample_texture_combos')
  .desc(
    `
Test that you can use the maximum number of textures with the maximum number of samplers.

The test works by making the maximum number of texture+sampler combos.
Each texture is [maxSamplersPerShaderStage, 1] in size where each texel is [textureId, samplerId]
A function "useCombo<StageNum>(comboId)" is made that returns stage[stageNum].combo[comboId].texel[id, 0]
or to put it another way, it returns the nth texel from the nth combo for that stage.

These are read in both the vertex shader and fragment shader and written to a [maxSamplerPerShaderStage, 2]
texture where the top row is the values from the vertex shader and the bottom row from the fragment shader.

The result should be a texture that has a value in each texel unique to a particular combo
`
  )
  .fn(t => {
    const { device } = t;
    const { maxSampledTexturesPerShaderStage, maxSamplersPerShaderStage, maxBindingsPerBindGroup } =
      device.limits;

    assert(maxSampledTexturesPerShaderStage < 0xfffe);
    assert(maxSamplersPerShaderStage < 0xfffe);

    const maxTestableCombosPerStage = t.isCompatibility
      ? Math.min(maxSampledTexturesPerShaderStage, maxSamplersPerShaderStage)
      : maxSampledTexturesPerShaderStage * maxSamplersPerShaderStage;

    const textures: GPUTexture[] = [];
    const declarationLines: string[] = [];
    const groups: GPUBindGroupEntry[][] = [[]];
    const layouts: GPUBindGroupLayoutEntry[][] = [[]];
    const textureIds = new Set<string>();
    const samplerIds = new Set<string>();
    // per stage, per texel, each texel has 2 numbers, the texture id, and sampler id
    const expected: number[][][] = [[], []];

    function addResource(stage: number, resourceId: string, resource: GPUTextureView | GPUSampler) {
      let bindGroupEntries = groups[groups.length - 1];
      let bindGroupLayoutEntries = layouts[groups.length - 1];
      if (bindGroupEntries.length === maxBindingsPerBindGroup) {
        bindGroupEntries = [];
        bindGroupLayoutEntries = [];
        groups.push(bindGroupEntries);
        layouts.push(bindGroupLayoutEntries);
      }
      const resourceType = resource instanceof GPUSampler ? 'sampler' : 'texture_2d<f32>';
      const binding = bindGroupEntries.length;
      declarationLines.push(
        `    @group(${groups.length - 1}) @binding(${binding}) var ${resourceId}: ${resourceType};`
      );
      bindGroupEntries.push({
        binding,
        resource,
      });
      bindGroupLayoutEntries.push({
        binding,
        visibility: stage === 0 ? GPUShaderStage.VERTEX : GPUShaderStage.FRAGMENT,
        ...(resource instanceof GPUSampler
          ? {
              sampler: {},
            }
          : {
              texture: {},
            }),
      });
    }

    function addTexture(stage: number, textureNum: number) {
      const textureId = `tex${stage}_${textureNum}`;
      if (!textureIds.has(textureId)) {
        textureIds.add(textureId);
        const texture = t.createTextureTracked({
          format: 'rgba8unorm',
          size: [maxSamplersPerShaderStage, 1],
          usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST,
        });
        textures.push(texture);
        // Encode an rgba8unorm texture with rg16uint data where each texel is
        // [(textureId + 1) | (stage << 15), {samplerId + 1}]
        // The +1 is to avoid 0.
        const data = new Uint16Array(maxSamplersPerShaderStage * 2);
        const rg = (textureNum + 1) | (stage << 15);
        for (let x = 0; x < maxSamplersPerShaderStage; ++x) {
          const offset = x * 2;
          const samplerNum = x + 1;
          data[offset + 0] = rg;
          data[offset + 1] = samplerNum;
        }
        device.queue.writeTexture({ texture }, data, {}, [maxSamplersPerShaderStage]);
        addResource(stage, textureId, texture.createView());
      }
      return textureId;
    }

    const kAddressModes = ['repeat', 'clamp-to-edge', 'mirror-repeat'] as const;
    const getAddressMode = (hash: number, depth: number) => {
      return kAddressModes[
        ((hash / Math.pow(kAddressModes.length, depth)) | 0) % kAddressModes.length
      ];
    };

    function addSampler(stage: number, samplerNum: number) {
      const samplerId = `smp${stage}_${samplerNum}`;
      if (!samplerIds.has(samplerId)) {
        const samplerNum = samplerIds.size;
        samplerIds.add(samplerId);
        // try to make each sampler unique. This is because some backends
        // coalesce samplers with the same settings.
        const addressHash = samplerNum >> 3;
        const sampler = device.createSampler({
          minFilter: samplerNum & 1 ? 'linear' : 'nearest',
          magFilter: samplerNum & 2 ? 'linear' : 'nearest',
          mipmapFilter: samplerNum & 4 ? 'linear' : 'nearest',
          addressModeU: getAddressMode(addressHash, 0),
          addressModeV: getAddressMode(addressHash, 1),
          addressModeW: getAddressMode(addressHash, 2),
        });
        addResource(stage, samplerId, sampler);
      }
      return samplerId;
    }

    // Note: We are storing textureId, samplerId in the texture. That suggests we could use rgba32uint
    // texture but we can't do that because we want to be able to set the samplers to linear.
    // Similarly we can't use rgba32float since they're not filterable by default.
    // So, we encode via rgba8unorm where rg is a 16bit textureId and ba is a 16bit samplerId
    const code = `
    // maxTestableCombosPerStage: ${maxTestableCombosPerStage}

    fn sample(t: texture_2d<f32>, s: sampler, validId: u32, currentId: u32, c: vec4f) -> vec4f {
      let size = textureDimensions(t, 0);
      let uv = vec2f((f32(currentId) + 0.5) / f32(size.x), 0.5);
      let v = textureSampleLevel(t, s, uv, 0);
      return select(c, v, currentId == validId);
    }

    ${range(
      2,
      stage => `
      fn useCombos${stage}(id: u32) -> vec4f {
        var c: vec4f;
${range(maxTestableCombosPerStage, i => {
  const texNum = (i / maxSamplersPerShaderStage) | 0;
  const textureId = addTexture(stage, texNum);
  const smpNum = i % maxSamplersPerShaderStage;
  const samplerId = addSampler(stage, smpNum);
  expected[stage].push([(texNum + 1) | (stage << 15), smpNum + 1]);
  return `        c = sample(${textureId}, ${samplerId}, ${i}, id, c);`;
}).join('\n')}
        return c;
      }
    `
    ).join('\n\n')}

${declarationLines.join('\n')}

    struct VOut {
      @builtin(position) pos: vec4f,
      @location(0) value: vec4f,
    };

    @vertex fn vs(@builtin(instance_index) iNdx: u32) -> VOut {
      return VOut(
        vec4f(0, 0, 0, 1),
        useCombos0(iNdx),
      );
    }

    @fragment fn fs(vin: VOut) -> @location(0) vec4u {
      let ndx = u32(vin.pos.x);
      let f = select(vin.value, useCombos1(ndx), vin.pos.y > 1.0);

      // We're putting two u16 values in the source data but as rgba8unorm.
      // Convert them back to u32 then split them back into two u16s
      let bytes = pack4x8unorm(f);
      return vec4u(bytes & 0xffff, bytes >> 16, 0, 0);
    }
    `;

    const module = device.createShaderModule({ code });
    const bindGroupLayouts = layouts.map(entries => device.createBindGroupLayout({ entries }));

    const pipeline = device.createRenderPipeline({
      layout: device.createPipelineLayout({ bindGroupLayouts }),
      vertex: {
        module,
      },
      fragment: {
        module,
        targets: [{ format: 'rg16uint' }],
      },
      primitive: { topology: 'point-list' },
    });

    const bindGroups = groups.map((entries, i) =>
      device.createBindGroup({
        layout: pipeline.getBindGroupLayout(i),
        entries,
      })
    );

    const renderTarget = t.createTextureTracked({
      format: 'rg16uint',
      size: [maxTestableCombosPerStage, 2],
      usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC,
    });
    textures.push(renderTarget);

    const encoder = device.createCommandEncoder();
    const pass = encoder.beginRenderPass({
      colorAttachments: [
        {
          view: renderTarget.createView(),
          loadOp: 'clear',
          storeOp: 'store',
        },
      ],
    });
    pass.setPipeline(pipeline);
    bindGroups.forEach((bindGroup, i) => pass.setBindGroup(i, bindGroup));
    for (let y = 0; y < 2; ++y) {
      for (let x = 0; x < maxTestableCombosPerStage; ++x) {
        pass.setViewport(x, y, 1, 1, 0, 1);
        pass.draw(1, 1, 0, x);
      }
    }
    pass.end();

    device.queue.submit([encoder.finish()]);

    const expectedData = new Uint16Array(maxTestableCombosPerStage * 2 * 2);
    for (let stage = 0; stage < 2; ++stage) {
      expected[stage].forEach(([tid, sid], i) => {
        const offset = (maxTestableCombosPerStage * stage + i) * 2;
        expectedData[offset + 0] = tid;
        expectedData[offset + 1] = sid;
      });
    }

    const expTexelView = TexelView.fromTextureDataByReference(
      'rg16uint',
      new Uint8Array(expectedData.buffer),
      {
        bytesPerRow: maxTestableCombosPerStage * 4,
        rowsPerImage: 2,
        subrectOrigin: [0, 0, 0],
        subrectSize: [maxTestableCombosPerStage, 2],
      }
    );

    const size = [maxSamplersPerShaderStage, 2];
    t.expectTexelViewComparisonIsOkInTexture({ texture: renderTarget }, expTexelView, size);

    textures.forEach(texture => texture.destroy());
  });
