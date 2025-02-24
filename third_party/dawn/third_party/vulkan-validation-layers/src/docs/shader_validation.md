# Shader Validation

Shader validation is run at `vkCreateShaderModule` and `vkCreate*Pipelines` time. It makes sure both the `SPIR-V` is valid
as well as the `VkPipeline` object interface with the shader. Note, this is all done on the CPU and different than
[GPU-Assisted Validation](gpu_validation.md).

## Standalone VUs with spirv-val

There are many [VUID labeled](https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#spirvenv-module-validation-standalone) as `VUID-StandaloneSpirv-*` and all the
[Built-In Variables](https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#interfaces-builtin-variables)
VUIDs that can be validated on a single shader module and require no runtime information.

All of these validations are passed off to `spirv-val` in [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools/).

## Validation Cache

To improve performance from run-to-run we make use of the `VK_EXT_validation_cache` extension.

During `vkCreateShaderModule` time the user can pass in a `VkShaderModuleValidationCacheCreateInfoEXT` object. Realistically, most apps will not do this, so the Validation Layers do this for apps. At `vkCreateDevice`/`vkDestroyDevice` we load/save a `uint32_t` hash of every `VkShaderModuleCreateInfo::pCode`. Before running validation on it, we check if it is a known/valid shader and if so, skip checking it again.

There are a few settings in `spirv-val` (ex. you can use `--allow-localsizeid` with `VK_KHR_maintenance4`) that change if the `SPIR-V` is legal or not. Because of this, we use both the `SPIRV-Tools` commit version, as well as the device features/extensions, to determine if the cache is valid or not. In practice, it will not matter too much for real apps as they normally don't toggle these few features on/off between runs.

## spirv-opt

There are a few special places where `spirv-opt` is run to reduce recreating work already done in `SPIRV-Tools`.
These can be found by searching for `RegisterPass` in the code

Currently these are

- Specialization constants
  - This `spirv-opt` pass is used to inject the constants from the pipeline layout.
  - Some checks require the runtime spec constant values
- Flatten OpGroupDecorations
  - Detects if group decorations were used; however, group decorations were [deprecated](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#OpGroupDecorate) early on in the development of the SPIR-v specification.
- Debug Printf
  - instruments the shaders to write the `printf` values out to a buffer

## Different sections

The code is currently split up into the following main sections

- `layers/state_tracker/shader_instruction.cpp`
    - This contains information about each SPIR-V instruction.
- `layers/state_tracker/shader_module.cpp`
    - This contains information about the `VkShaderModule` object
- `layers/state_tracker/shader_stage_state.cpp`
    - This contains information about the shader stage, this can be either from a Pipeline and/or Shader Object
- `layers/core_checks/cc_spirv.cpp`
    - This takes the following above and does the actual validation. All errors are produced here.
    - `layers/vulkan/generated/spirv_validation_helper.cpp`
        - This is generated file provides a way to generate checks for things found in the `vk.xml` related to SPIR-V
- `layers/vulkan/generated/spirv_grammar_helper.cpp`
    - This is a general util file that is [generated](generated_code.md) from the SPIR-V grammar

## Types of Shader Validation

All Shader Validation can be broken into 4 types of checks

- SPIR-V with runtime properties
  - Things like features and limits
- Shader interface
  - Ex. going between a Vertex and Fragment shader
- Interaction with Pipeline creation structs
  - Vertex input, fragment output, etc
- Draw time
  - making sure bound descriptor matches up what is being touched

## Design details

When dealing with shader validation there are a few concepts to understand and not confuse

- `EntryPoints`
  - Tied to a shader stage (fragment, vertex, etc)
  - Knows which variables and instructions are touched in stage
    - There might be things in a `ShaderModule` not related to shader stage validation
- `SPIR-V Module`
  - `SPIRV_MODULE_STATE`
  - This object takes in SPIR-V, parses it, creates `EntryPoint` objects, validates what we can
    - We do validation first as sometimes a bad SPIR-V can [crash a driver](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2835)
    - can contain [multiple EntryPoints](https://github.com/KhronosGroup/SPIRV-Guide/blob/master/chapters/entry_execution.md#instructions-with-multiple-execution-modes)
  - contains SPIR-V instructions (in an array of `uint32_t` words)
  - knows the relationship between instructions
- `Shader Module` and `Shader Object`
  - `VkShaderModule` object (`SHADER_MODULE_STATE`) or `VkShaderEXT` object (`SHADER_OBJECT_STATE`)
  - **can** hold a `SPIR-V module` reference
    - `Pipeline Library` (GPL) (`VK_EXT_graphics_pipeline_library`)
        - part of a pipeline that can be reused
    - `ShaderModuleIdentifier` (`VK_EXT_shader_module_identifier`)
        - lets app use a hash instead of having the driver re-create the `ShaderModule`
        - not possible to validate as the VVL don't know what the `ShaderModule` is
- `Pipeline`
  - contains 1 or more `Shader Module` object
  - decides both which `Shader Module` and `EntryPoint` are used
  - has other state not known if validating just the shader object

When dealing with validation, it is important to know what should be validated, and when.

If validation only cares about... :

- the SPIR-V itself, is mapped to the `SPIRV_MODULE_STATE`
- if two stages interface, needs to be done when all stages are there
  - For `Pipeline Library` it might need to wait until linking
- descriptors variables, use `EntryPoint`
- the stage of a shader module is always known, regardless of even using `ShaderModuleIdentifier`
- Pipeline can have fields that are related to shaders, but [don't actually require the SPIR-V](https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/6226)

### Variables

There are 2 types of Variables

- `Resource Interface` variables (mapped to descriptors)
- `Stage Interface` variables (input and output between shader)
  - Can be either a `BuiltIn` or `User Defined` variables

For each `EntryPoint` we walk the functions and find all `Variables` accessed (load, store, atomic).

Infomaration to note:
- It is possible to have multiple `EntryPoints` pointing to the same interface variable.
- 2 different accesses (ex. `OpLoad`) can point to same `Variable`
- 2 `Image operation` can point to 2 different `Variables`

### Image Accesses

Any variable in a shader pointing to an Image is a `Resource Interface` variable.
There are validation checks that need care only if the variable is accessed.
This requires a `OpImage*` instruction to access the variable.

Most Accesses look like

```
OpImage* -> OpLoad -> OpAccessChain (optional) -> OpVariable
```

There are a few exceptions:

An Image Fetch has an `OpImage` prior to the `OpLoad`

```
OpImageFetch -> OpImage -> OpLoad -> OpVariable
```

Image Atomics use `OpImageTexelPointer` instead of `OpLoad`

```
OpAtomicLoad -> OpImageTexelPointer -> OpAccessChain (optional) -> OpVariable
```

The biggest thing to consider is using either a

- `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER`
- `VK_DESCRIPTOR_TYPE_SAMPLER` and `VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE` combo

```
// VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
OpImageSampleExplicitLod -> OpLoad -> OpAccessChain (optional) -> OpVariable -> OpTypePointer -> OpTypeSampledImage

// VK_DESCRIPTOR_TYPE_SAMPLER and VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
OpImageSampleImplicitLod -> OpSampledImage -> OpTypeSampledImage
                                           -> OpLoad -> OpAccessChain (optional) -> OpVariable (image)
                                           -> OpLoad -> OpAccessChain (optional) -> OpVariable (sampler)
```

Both contain a `OpTypeSampledImage`, which is how we know a `VkSampler` is being used with the variable

But it is also possible to have the Image and Samplers mix and match

```
ImageAccess -> Image_0
            -> Sampler_0

ImageAccess -> Image_0
            -> Sampler_1

ImageAccess -> Image_0 (non-sampled access)
```

This is handled by having the `Resource Interface` variable track if it has a `OpTypeSampledImage`, `OpTypeImage` or `OpTypeSampler`

- If it has `OpTypeSampledImage`, there is **no way** for it to be part of a `SAMPLER`/`SAMPLED_IMAGE` combo
- If it has a `OpTypeImage` or `OpTypeSampler`, we need to know if they are **accessed together**
    - This means the the `ValidateDescriptor` logic needs to know every `OpTypeSampler` variable accessed together with a `OpTypeImage` variable
    - There is no case where only a `OpTypeSampler` variable can be used by itself, so no need to track it the other way

If the Image Access is in a function, it might point to multiple `OpVariable`

### Atomics

There are 2 types of atomic accesses: "Image" and "Non-Image"

Image atomics are described above how they use `OpImageTexelPointer` instead of `OpLoad`

Non-Image atomics will look like

```
OpAtomicLoad -> OpAccessChain (optional) -> OpVariable
```