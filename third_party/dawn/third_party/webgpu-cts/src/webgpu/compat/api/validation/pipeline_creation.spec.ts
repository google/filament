export const description = `
Tests that createComputePipeline(async), and createRenderPipeline(async)
reject pipelines that are invalid in compat mode

- test that depth textures can not be used with non-comparison samplers

TODO:
- test that a shader that has more than min(maxSamplersPerShaderStage, maxSampledTexturesPerShaderStage)
  texture+sampler combinations generates a validation error.
`;

import { makeTestGroup } from '../../../../common/framework/test_group.js';
import {
  kShortShaderStages,
  kShortShaderStageToShaderStage,
} from '../../../shader/execution/expression/call/builtin/texture_utils.js';
import { CompatibilityTest } from '../../compatibility_test.js';

export const g = makeTestGroup(CompatibilityTest);

g.test('depth_textures')
  .desc('Tests that depth textures can not be used with non-comparison samplers in compat mode.')
  .params(u =>
    u //
      .combineWithParams([
        {
          sampleWGSL: 'textureSample(t, s, vec2f(0))', // should pass
          textureType: 'texture_2d<f32>',
        },
        {
          sampleWGSL: 'textureSample(t, s, vec2f(0))',
          textureType: 'texture_depth_2d',
        },
        {
          sampleWGSL: 'textureSample(t, s, vec3f(0))',
          textureType: 'texture_depth_cube',
        },
        {
          sampleWGSL: 'textureSample(t, s, vec2f(0), 0)',
          textureType: 'texture_depth_2d_array',
        },
        {
          sampleWGSL: 'textureSample(t, s, vec2f(0), vec2i(0, 0))',
          textureType: 'texture_depth_2d',
        },
        {
          sampleWGSL: 'textureSample(t, s, vec2f(0), 0, vec2i(0, 0))',
          textureType: 'texture_depth_2d_array',
        },
        {
          sampleWGSL: 'textureSampleLevel(t, s, vec2f(0), 0)',
          textureType: 'texture_depth_2d',
        },
        {
          sampleWGSL: 'textureSampleLevel(t, s, vec3f(0), 0)',
          textureType: 'texture_depth_cube',
        },
        {
          sampleWGSL: 'textureSampleLevel(t, s, vec2f(0), 0, 0)',
          textureType: 'texture_depth_2d_array',
        },
        {
          sampleWGSL: 'textureSampleLevel(t, s, vec2f(0), 0, vec2i(0, 0))',
          textureType: 'texture_depth_2d',
        },
        {
          sampleWGSL: 'textureSampleLevel(t, s, vec2f(0), 0, 0, vec2i(0, 0))',
          textureType: 'texture_depth_2d_array',
        },
        {
          sampleWGSL: 'textureGather(t, s, vec2f(0))',
          textureType: 'texture_depth_2d',
        },
        {
          sampleWGSL: 'textureGather(t, s, vec3f(0))',
          textureType: 'texture_depth_cube',
        },
        {
          sampleWGSL: 'textureGather(t, s, vec2f(0), 0)',
          textureType: 'texture_depth_2d_array',
        },
        {
          sampleWGSL: 'textureGather(t, s, vec2f(0), vec2i(0, 0))',
          textureType: 'texture_depth_2d',
        },
        {
          sampleWGSL: 'textureGather(t, s, vec2f(0), 0, vec2i(0, 0))',
          textureType: 'texture_depth_2d_array',
        },
      ])
      .combine('stage', kShortShaderStages)
      .filter(t => t.sampleWGSL.startsWith('textureGather') || t.stage === 'f')
      .combine('async', [false, true] as const)
  )
  .fn(t => {
    const { sampleWGSL, textureType, stage: shortStage, async } = t.params;
    const stage = kShortShaderStageToShaderStage[shortStage];

    const usageWGSL = `_ = ${sampleWGSL};`;
    const module = t.device.createShaderModule({
      code: `
        @group(0) @binding(0) var t: ${textureType};
        @group(1) @binding(0) var s: sampler;

        // make sure it's fine such a combination exists but it's not used.
        fn unused() {
          ${usageWGSL};
        }

        @vertex fn vs() -> @builtin(position) vec4f {
            ${stage === 'vertex' ? usageWGSL : ''}
            return vec4f(0);
        }

        @fragment fn fs() -> @location(0) vec4f {
            ${stage === 'fragment' ? usageWGSL : ''}
            return vec4f(0);
        }

        @compute @workgroup_size(1) fn cs() {
            ${stage === 'compute' ? usageWGSL : ''};
        }
      `,
    });

    const success = !t.isCompatibility || textureType === 'texture_2d<f32>';
    switch (stage) {
      case 'compute':
        t.doCreateComputePipelineTest(async, success, {
          layout: 'auto',
          compute: {
            module,
          },
        });
        break;
      case 'fragment':
      case 'vertex':
        t.doCreateRenderPipelineTest(async, success, {
          layout: 'auto',
          vertex: {
            module,
          },
          fragment: {
            module,
            targets: [{ format: 'rgba8unorm' }],
          },
        });
        break;
    }
  });

function numCombosToNumber(maxCombos: number, numCombos: string): number {
  const m = /^max([+-]?)(\d+)$/.exec(numCombos);
  if (m) {
    if (m[1] === '+') {
      return maxCombos + parseInt(m[2]);
    } else if (m[1] === '-') {
      return maxCombos - parseInt(m[2]);
    } else {
      return maxCombos;
    }
  }
  return parseInt(numCombos);
}

function numNonSampledToNumber(maxTextures: number, numNonSampled: '0' | '1' | 'max') {
  switch (numNonSampled) {
    case '0':
      return 0;
    case '1':
      return 1;
    case 'max':
      return maxTextures;
  }
}

g.test('texture_sampler_combos')
  .desc(
    `
Test that you can not use more texture+sampler combos than
min(maxSamplersPerShaderStage, maxSampledTexturesPerShaderStage)
in compatibility mode.

The spec, copy and pasted here:

maxCombinationsPerStage = min(maxSampledTexturesPerShaderStage, maxSamplersPerShaderStage)
for each stage of the pipeline:
  sum = 0
  for each texture binding in the pipeline layout which is visible to that stage:
    sum += max(1, number of texture sampler combos for that texture binding)
  for each external texture binding in the pipeline layout which is visible to that stage:
    sum += 1 // for LUT texture + LUT sampler
    sum += 3 * max(1, number of external_texture sampler combos) // for Y+U+V
  if sum > maxCombinationsPerStage
    generate a validation error.

`
  )
  .params(u =>
    // prettier-ignore
    u
      .combineWithParams([
        ...([
          { pass: true,  numCombos: 'max',   numNonSampled: '1',   numExternal: 0, useSame: false },
          { pass: false, numCombos: 'max+1', numNonSampled: '1',   numExternal: 0, useSame: false },
          { pass: true,  numCombos: '1',     numNonSampled: 'max', numExternal: 0, useSame: false },
          { pass: false, numCombos: '2',     numNonSampled: 'max', numExternal: 0, useSame: false },
          { pass: true,  numCombos: 'max-4', numNonSampled: '0',   numExternal: 1, useSame: false },
          { pass: false, numCombos: 'max-3', numNonSampled: '0',   numExternal: 1, useSame: false },
          { pass: true,  numCombos: 'max-8', numNonSampled: '0',   numExternal: 2, useSame: false },
          { pass: false, numCombos: 'max-7', numNonSampled: '0',   numExternal: 2, useSame: false },
          { pass: true,  numCombos: 'max-7', numNonSampled: '0',   numExternal: 2, useSame: true },
          { pass: false, numCombos: 'max-6', numNonSampled: '0',   numExternal: 2, useSame: true },
        ] as const).map(p => ([{ ...p, stages: 'vertex' }, { ...p, stages: 'fragment' }, { ...p, stages: 'compute' }] as const)).flat(),
        {   pass: true,  numCombos: 'max',   numNonSampled: '1',   numExternal: 0, useSame: false, stages: 'vertex,fragment' },
      ] as const)
      .combine('async', [false, true] as const)
  )
  .fn(t => {
    const { device } = t;
    const { pass, stages, numCombos, numNonSampled, numExternal, useSame, async } = t.params;
    const { maxSampledTexturesPerShaderStage, maxSamplersPerShaderStage } = device.limits;

    const maxTexturesPerStage = maxSampledTexturesPerShaderStage - numExternal * 3;
    const maxCombos = Math.min(maxSampledTexturesPerShaderStage, maxSamplersPerShaderStage);
    const numCombinations = numCombosToNumber(maxCombos, numCombos);
    const numNonSampledTextures = numNonSampledToNumber(
      maxSampledTexturesPerShaderStage,
      numNonSampled
    );

    const textureDeclarations: string[][] = [[], []];
    const samplerDeclarations: string[][] = [[], []];
    const usages: string[][] = [[], []];
    const bindGroupLayoutEntries: GPUBindGroupLayoutEntry[][] = [[], [], [], []];
    const numStages = stages === 'compute' ? 1 : 2;
    const visibilityByStage =
      stages === 'compute'
        ? [GPUShaderStage.COMPUTE]
        : [GPUShaderStage.VERTEX, GPUShaderStage.FRAGMENT];

    const addTextureDeclaration = (stage: number, binding: number) => {
      textureDeclarations[stage].push(
        `@group(${stage * 2}) @binding(${binding}) var t${stage}_${binding}: texture_2d<f32>;`
      );
      bindGroupLayoutEntries[stage * 2].push({
        binding,
        visibility: visibilityByStage[stage],
        texture: {},
      });
    };

    const addSamplerDeclaration = (stage: number, binding: number) => {
      samplerDeclarations[stage].push(
        `@group(${stage * 2 + 1}) @binding(${binding}) var s${stage}_${binding}: sampler;`
      );
      bindGroupLayoutEntries[stage * 2 + 1].push({
        binding,
        visibility: visibilityByStage[stage],
        sampler: {},
      });
    };

    const addExternalTextureDeclaration = (stage: number, binding: number, id: number) => {
      textureDeclarations[stage].push(
        `@group(${stage * 2}) @binding(${binding}) var e${stage}_${id}: texture_external;`
      );
      bindGroupLayoutEntries[stage * 2].push({
        binding,
        visibility: visibilityByStage[stage],
        externalTexture: {},
      });
    };

    for (let stage = 0; stage < numStages; ++stage) {
      let count = 0;
      for (let t = 0; count < numCombinations && t < maxTexturesPerStage; ++t) {
        addTextureDeclaration(stage, t);
        for (let s = 0; count < numCombinations && s < maxSamplersPerShaderStage; ++s) {
          if (t === 0) {
            addSamplerDeclaration(stage, s);
          }
          usages[stage].push(
            `  c += textureSampleLevel(t${stage}_${t}, s${stage}_${s}, vec2f(0), 0);`
          );
          ++count;
        }
      }

      for (let t = 0; t < numNonSampledTextures; ++t) {
        if (t >= textureDeclarations[stage].length) {
          addTextureDeclaration(stage, t);
        }
        usages[stage].push(`  c += textureLoad(t${stage}_${t}, vec2u(0), 0);`);
      }

      for (let t = 0; t < numExternal; ++t) {
        if (t === 0 || !useSame) {
          const et = textureDeclarations[stage].length + t;
          addExternalTextureDeclaration(stage, et, t);
        }
        const eBinding = useSame ? 0 : t;
        const sBinding = useSame ? t : 0;
        usages[stage].push(
          `  c += textureSampleBaseClampToEdge(e${stage}_${eBinding}, s${stage}_${sBinding}, vec2f(0));`
        );
      }
    }

    const code = `
${textureDeclarations[0].join('\n')}
${textureDeclarations[1].join('\n')}

${samplerDeclarations[0].join('\n')}
${samplerDeclarations[1].join('\n')}

fn usage0() -> vec4f {
  var c: vec4f;
  ${usages[0].join('\n')}
  return c;
}

fn usage1() -> vec4f {
  var c: vec4f;
  ${usages[1].join('\n')}
  return c;
}

@vertex fn vs() -> @builtin(position) vec4f {
  _ = ${stages.includes('vertex') ? 'usage0()' : 'vec4f(0)'};
  return vec4f(0);
}

@fragment fn fs() -> @location(0) vec4f {
  return ${stages.includes('fragment') ? 'usage1()' : 'vec4f(0)'};
}

@compute @workgroup_size(1) fn cs() {
  _ = usage0();
}

`;

    // MAINTENANCE_TODO: remove this. It's only needed because of a bug in dawn
    // with auto layouts.
    const layout = device.createPipelineLayout({
      bindGroupLayouts: bindGroupLayoutEntries.map(entries =>
        device.createBindGroupLayout({ entries })
      ),
    });

    const module = device.createShaderModule({ code });
    if (stages === 'compute') {
      t.doCreateComputePipelineTest(async, pass || !t.isCompatibility, {
        layout,
        compute: { module },
      });
    } else {
      t.doCreateRenderPipelineTest(async, pass || !t.isCompatibility, {
        layout,
        vertex: { module },
        fragment: { module, targets: [{ format: 'rgba8unorm' }] },
      });
    }
  });
