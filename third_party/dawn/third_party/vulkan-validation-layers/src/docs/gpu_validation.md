<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2025 Valve Corporation -->
<!-- Copyright 2015-2025 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# GPU Assisted Validation

While most validation can be done on the CPU, some things like the content of a buffer or how a shader invocation accesses a descriptor array cannot be known until a command buffer is executed. The Validation Layers have a dedicated tool to perform validation in those cases: **GPU Assisted Validation** (aka GPU-AV).

GPU-AV directly inspects GPU resources and instrument shaders to validate their run time invocations, and reports back any error. Due to dedicated state tracking, shader instrumentation, necessity to read back data from the GPU, etc, there is a performance overhead, so **GPU-AV is off by default**.

## How to use

For an overview of how to configure layers, refer to the [Layers Overview and Configuration](https://vulkan.lunarg.com/doc/sdk/latest/windows/layer_configuration.html) document.

The GPU-AV settings are managed by configuring the Validation Layer. These settings are described in the [VK_LAYER_KHRONOS_validation](https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html#user-content-layer-details) document.

GPU-AV settings can also be managed using the [Vulkan Configurator](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) included with the Vulkan SDK.

> Note - it is **highly** recommended to not have both normal Core validation and GPU-AV on together as performance will be slow.

## Requirements

There are several limitations that may impede the operation of GPU Assisted Validation:

- Vulkan 1.1+ required
- A Descriptor Slot
    - GPU-AV requires one descriptor set, which means if an application is using all sets in `VkPhysicalDeviceLimits::maxBoundDescriptorSets`, GPU-AV will not work.
    - There is a `VK_LAYER_GPUAV_RESERVE_BINDING_SLOT` that will reduce `maxBoundDescriptorSets` by one if the app uses that value to determine its logic.
- `fragmentStoresAndAtomics` and `vertexPipelineStoresAndAtomics` so we can write out information from those stages.
- `timelineSemaphore` so we don't have a big `vkQueueWaitIdle` after each submission.

There are various other feature requirements, but if not met, GPU-AV will turn off the parts of GPU-AV for you automatically (and produce a warning message).

If a feature is not enabled, we will try to enable it for you at device creation time.

## Types of GPU-AV

GPU-AV has many things it validates, all of which have different implementation designs.

TODO - Add various internal sections

## Selective Shader Instrumentation

With the `khronos_validation.gpuav_select_instrumented_shaders`/`VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS` feature, an application can control which shaders are instrumented and thus, will return GPU-AV errors.

With the feature enabled, all SPIR-V will not be modified by default.

Inside your `VkShaderModuleCreateInfo` or `vkCreateShadersEXT` pass in a `VkValidationFeaturesEXT` into the `pNext` with `VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT` to have the shader instrumented.

```c++
// Example
VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
VkValidationFeaturesEXT features = {};
features.enabledValidationFeatureCount = 1;
features.pEnabledValidationFeatures = enabled;

VkShaderModuleCreateInfo module_ci = {};
module_ci.pNext = &features;
```

## Validating Vulkan calls made by GPU Assisted Validation

Since GPU-AV itself utilizes the Vulkan API to perform its tasks,
Vulkan function calls have to valid. To ensure that, those calls have to
go through another instance of the Vulkan Validation Layer. We refer to this
as "self validation".

How to setup self validation:
- Build the self validation layer:
    - Make sure to use a Release build
        - Otherwise might be really slow with double validation
    - Use the the `-DBUILD_SELF_VVL=ON` cmake option when generating the CMake project
        - The build will produce a manifest file used by the Vulkan loader, `VkLayer_dev_self_validation.json`.
        The `name` field in this file is `VK_LAYER_DEV_self_validation` to differentiate the self validation layer from the one you work on.
            - If the name were the same, the loader/os would mark both layers as duplicates and not load the second instance
- Then use it:
    - you need to ask the loader to load the self validation layer, and tell it where to find it.
        Do this by modifying the `VK_INSTANCE_LAYERS` and `VK_LAYER_PATH`, like so for instance:
```bash
# Windows
VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation;VK_LAYER_DEV_self_validation
VK_LAYER_PATH=C:\Path\To\Vulkan-ValidationLayers\build\debug\layers\Debug;C:\Path\To\Vulkan-ValidationLayers\build_self_vvl\layers\Release

# Linux
VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation:VK_LAYER_DEV_self_validation
VK_LAYER_PATH=/Path/To/Vulkan-ValidationLayers/build/debug/layers/Debug:/Path/To/Vulkan-ValidationLayers/build_self_vvl/layers/Release
```

⚠️ Make sure to load the self validation layer **after** the validation layer you work on, by putting its name in `VK_INSTANCE_LAYERS` after the validation layer you work on. Otherwise your Vulkan calls will not be intercepted by the self validation layer.
To make sure you did it properly, you can use the environment variable `VK_LOADER_DEBUG=layer` to see how the loader sets up layers.
