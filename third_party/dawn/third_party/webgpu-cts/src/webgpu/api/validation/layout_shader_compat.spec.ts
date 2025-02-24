export const description = `
TODO:
- interface matching between pipeline layout and shader
    - x= bind group index values, binding index values, multiple bindings
    - x= {superset, subset}
`;

import { makeTestGroup } from '../../../common/framework/test_group.js';
import {
  kShaderStageCombinations,
  kShaderStages,
  ValidBindableResource,
} from '../../capability_info.js';
import { GPUConst } from '../../constants.js';
import { MaxLimitsTestMixin } from '../../gpu_test.js';

import { ValidationTest } from './validation_test.js';

type BindableResourceType = ValidBindableResource | 'readonlyStorageBuf';
const kBindableResources = [
  'uniformBuf',
  'storageBuf',
  'readonlyStorageBuf',
  'filtSamp',
  'nonFiltSamp',
  'compareSamp',
  'sampledTex',
  'sampledTexMS',
  'readonlyStorageTex',
  'writeonlyStorageTex',
  'readwriteStorageTex',
] as const;

const bindGroupLayoutEntryContents = {
  compareSamp: {
    sampler: {
      type: 'comparison',
    },
  },
  filtSamp: {
    sampler: {
      type: 'filtering',
    },
  },
  nonFiltSamp: {
    sampler: {
      type: 'non-filtering',
    },
  },
  sampledTex: {
    texture: {
      sampleType: 'unfilterable-float',
    },
  },
  sampledTexMS: {
    texture: {
      sampleType: 'unfilterable-float',
      multisampled: true,
    },
  },
  storageBuf: {
    buffer: {
      type: 'storage',
    },
  },
  readonlyStorageBuf: {
    buffer: {
      type: 'read-only-storage',
    },
  },
  uniformBuf: {
    buffer: {
      type: 'uniform',
    },
  },
  readonlyStorageTex: {
    storageTexture: {
      format: 'r32float',
      access: 'read-only',
    },
  },
  writeonlyStorageTex: {
    storageTexture: {
      format: 'r32float',
      access: 'write-only',
    },
  },
  readwriteStorageTex: {
    storageTexture: {
      format: 'r32float',
      access: 'read-write',
    },
  },
} as const;

class F extends ValidationTest {
  createPipelineLayout(
    bindingInPipelineLayout: BindableResourceType,
    visibility: number
  ): GPUPipelineLayout {
    return this.device.createPipelineLayout({
      bindGroupLayouts: [
        this.device.createBindGroupLayout({
          entries: [
            {
              binding: 0,
              visibility,
              ...bindGroupLayoutEntryContents[bindingInPipelineLayout],
            },
          ],
        }),
      ],
    });
  }

  GetBindableResourceShaderDeclaration(bindableResource: BindableResourceType): string {
    switch (bindableResource) {
      case 'compareSamp':
        return 'var tmp : sampler_comparison';
      case 'filtSamp':
      case 'nonFiltSamp':
        return 'var tmp : sampler';
      case 'sampledTex':
        return 'var tmp : texture_2d<f32>';
      case 'sampledTexMS':
        return 'var tmp : texture_multisampled_2d<f32>';
      case 'storageBuf':
        return 'var<storage, read_write> tmp : vec4u';
      case 'readonlyStorageBuf':
        return 'var<storage, read> tmp : vec4u';
      case 'uniformBuf':
        return 'var<uniform> tmp : vec4u;';
      case 'readonlyStorageTex':
        return 'var tmp : texture_storage_2d<r32float, read>';
      case 'writeonlyStorageTex':
        return 'var tmp : texture_storage_2d<r32float, write>';
      case 'readwriteStorageTex':
        return 'var tmp : texture_storage_2d<r32float, read_write>';
    }
  }
}

const BindingResourceCompatibleWithShaderStages = function (
  bindingResource: BindableResourceType,
  shaderStages: number
): boolean {
  if ((shaderStages & GPUConst.ShaderStage.VERTEX) > 0) {
    switch (bindingResource) {
      case 'writeonlyStorageTex':
      case 'readwriteStorageTex':
      case 'storageBuf':
        return false;
      default:
        break;
    }
  }
  return true;
};

export const g = makeTestGroup(MaxLimitsTestMixin(F));

g.test('pipeline_layout_shader_exact_match')
  .desc(
    `
  Test that the binding type in the pipeline layout must match the related declaration in shader.
  Note that read-write storage textures in the pipeline layout can match write-only storage textures
  in the shader.
  `
  )
  .params(u =>
    u
      .combine('bindingInPipelineLayout', kBindableResources)
      .combine('bindingInShader', kBindableResources)
      .beginSubcases()
      .combine('pipelineLayoutVisibility', kShaderStageCombinations)
      .combine('shaderStageWithBinding', kShaderStages)
      .combine('isBindingStaticallyUsed', [true, false] as const)
      .unless(
        p =>
          // We don't test using non-filtering sampler in shader because it has the same declaration
          // as filtering sampler.
          p.bindingInShader === 'nonFiltSamp' ||
          !BindingResourceCompatibleWithShaderStages(
            p.bindingInPipelineLayout,
            p.pipelineLayoutVisibility
          ) ||
          !BindingResourceCompatibleWithShaderStages(p.bindingInShader, p.shaderStageWithBinding)
      )
  )
  .fn(t => {
    const {
      bindingInPipelineLayout,
      bindingInShader,
      pipelineLayoutVisibility,
      shaderStageWithBinding,
      isBindingStaticallyUsed,
    } = t.params;

    if (t.isCompatibility) {
      const bindingUsedWithVertexStage =
        (shaderStageWithBinding & GPUShaderStage.VERTEX) !== 0 ||
        (pipelineLayoutVisibility & GPUShaderStage.VERTEX) !== 0;
      const bindingUsedWithFragmentStage =
        (shaderStageWithBinding & GPUShaderStage.FRAGMENT) !== 0 ||
        (pipelineLayoutVisibility & GPUShaderStage.FRAGMENT) !== 0;
      const bindingIsStorageBuffer =
        bindingInPipelineLayout === 'readonlyStorageBuf' ||
        bindingInPipelineLayout === 'storageBuf';
      const bindingIsStorageTexture =
        bindingInPipelineLayout === 'readonlyStorageTex' ||
        bindingInPipelineLayout === 'readwriteStorageTex' ||
        bindingInPipelineLayout === 'writeonlyStorageTex';
      t.skipIf(
        bindingUsedWithVertexStage &&
          bindingIsStorageBuffer &&
          t.device.limits.maxStorageBuffersInVertexStage === 0,
        'Storage buffers can not be used in vertex shaders because maxStorageBuffersInVertexStage === 0'
      );
      t.skipIf(
        bindingUsedWithVertexStage &&
          bindingIsStorageTexture &&
          t.device.limits.maxStorageTexturesInVertexStage === 0,
        'Storage textures can not be used in vertex shaders because maxStorageTexturesInVertexStage === 0'
      );
      t.skipIf(
        bindingUsedWithFragmentStage &&
          bindingIsStorageBuffer &&
          t.device.limits.maxStorageBuffersInFragmentStage === 0,
        'Storage buffers can not be used in fragment shaders because maxStorageBuffersInFragmentStage === 0'
      );
      t.skipIf(
        bindingUsedWithFragmentStage &&
          bindingIsStorageTexture &&
          t.device.limits.maxStorageTexturesInFragmentStage === 0,
        'Storage textures can not be used in fragment shaders because maxStorageTexturesInFragmentStage === 0'
      );
    }

    const layout = t.createPipelineLayout(bindingInPipelineLayout, pipelineLayoutVisibility);
    const bindResourceDeclaration = `@group(0) @binding(0) ${t.GetBindableResourceShaderDeclaration(
      bindingInShader
    )}`;
    const staticallyUseBinding = isBindingStaticallyUsed ? '_ = tmp; ' : '';
    const isAsync = false;

    let success = true;
    if (isBindingStaticallyUsed) {
      success = bindingInPipelineLayout === bindingInShader;

      // Filtering and non-filtering both have the same shader declaration.
      success ||= bindingInPipelineLayout === 'nonFiltSamp' && bindingInShader === 'filtSamp';

      // Promoting storage textures that are read-write in the layout can be readonly in the shader.
      success ||=
        bindingInPipelineLayout === 'readwriteStorageTex' &&
        bindingInShader === 'writeonlyStorageTex';

      // The shader using the resource must be included in the visibility in the layout.
      success &&= (pipelineLayoutVisibility & shaderStageWithBinding) > 0;
    }

    switch (shaderStageWithBinding) {
      case GPUConst.ShaderStage.COMPUTE: {
        const computeShader = `
        ${bindResourceDeclaration};
        @compute @workgroup_size(1)
        fn main() {
          ${staticallyUseBinding}
        }
        `;
        t.doCreateComputePipelineTest(isAsync, success, {
          layout,
          compute: {
            module: t.device.createShaderModule({
              code: computeShader,
            }),
          },
        });
        break;
      }
      case GPUConst.ShaderStage.VERTEX: {
        const vertexShader = `
        ${bindResourceDeclaration};
        @vertex
        fn main() -> @builtin(position) vec4f {
          ${staticallyUseBinding}
          return vec4f();
        }
        `;
        t.doCreateRenderPipelineTest(isAsync, success, {
          layout,
          vertex: {
            module: t.device.createShaderModule({
              code: vertexShader,
            }),
          },
          depthStencil: { format: 'depth32float', depthWriteEnabled: true, depthCompare: 'always' },
        });
        break;
      }
      case GPUConst.ShaderStage.FRAGMENT: {
        const fragmentShader = `
        ${bindResourceDeclaration};
        @fragment
        fn main() -> @location(0) vec4f {
          ${staticallyUseBinding}
          return vec4f();
        }
        `;
        t.doCreateRenderPipelineTest(isAsync, success, {
          layout,
          vertex: {
            module: t.device.createShaderModule({
              code: `
                @vertex
                fn main() -> @builtin(position) vec4f {
                  return vec4f();
                }`,
            }),
          },
          fragment: {
            module: t.device.createShaderModule({
              code: fragmentShader,
            }),
            targets: [
              {
                format: 'rgba8unorm',
              },
            ],
          },
        });
        break;
      }
    }
  });
