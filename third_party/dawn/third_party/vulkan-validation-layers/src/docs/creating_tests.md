# Creating Tests

This is an "up-to-speed" document for writing tests to validate the Validation Layers

[Information how to run the tests](../tests/README.md)

## Rule #1

The first rule is to make sure you are actually running the tests on the built version of the Validation Layers you want. Set the environment variable `VK_LOADER_DEBUG` to `layer` and check that the output of the tests report that the path of the validation layer matches what is expected.

The tests automatically set `VK_LAYER_PATH` to the validation layer in the build tree. However if you wish to use a different validation layer than the one that was built, or if you wish to use multiple layers in the tests at the same time, you must set `VK_LAYER_PATH` or `VK_ADD_LAYER_PATH` to include each path to the desired layers, including the validation layer.

Make sure you have the correct `VK_LAYER_PATH` set on Windows or Linux (on Android the layers are baked into the APK so there is nothing to worry about)

```bash
export VK_LAYER_PATH=/path/to/Vulkan-ValidationLayers/build/layers/
```

There is nothing worse than debugging why your layers are not reporting the VUID you added due to not actually testing that code!

## Google Test Overview

The tests take advantage of the Google Test (gtest) Framework which breaks each test into a `TEST_F(VkLayerTest, TestName)` "Test Fixture". This just means that for every test there will be a class that holds many useful member variables.

To run a test labeled `TEST_F(VkLayerTest, Foo)` is as simple as going `--gtest_filter=VkLayerTest.Foo`

## VkRenderFramework

The `VkRenderFramework` class is "base class" that abstract most things in order to allow a test writer to focus on the small part of coded needed for the test.

For most tests, it is as simple as going

```cpp
RETURN_IF_SKIP(Init());

// or

RETURN_IF_SKIP(InitFramework());
RETURN_IF_SKIP(InitState());

// For Best Practices tests
RETURN_IF_SKIP(InitBestPracticesFramework());
RETURN_IF_SKIP(InitState());
```

to set it up. This will create the `VkInstance` and `VkDevice` for you.

There are other useful helper functions such as `InitSurface()`, `InitSwapchain()`, `InitRenderTarget()`, and more. Please view the class source for more of an overview of what it currently supports

## Extensions

Adding extension support is quite easy.

Here is an example of adding `VK_KHR_sampler_ycbcr_conversion` with all the extensions it requires as well. Note, most extensions will have only 1 or 2 dependency extensions needed

```cpp
// Setup extensions, including dependent instance and device extensions. This call should be made before any call to InitFramework
AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
AddRequiredFeature(vkt::Feature:: samplerYcbcrConversion);

// Among other things, this will create the VkInstance and VkPhysicalDevice that will be used for the test.
// Also will check that all extensions and their dependencies were enabled successfully
RETURN_IF_SKIP(InitFramework());

// Finish initializing state, including creating the VkDevice (whith extensions added) that will be used for the test
RETURN_IF_SKIP(InitState());
```

The pattern breaks down to
- Check and add Instance extensions to list
- Init Framework which creates `VkInstance`
- Check and add Device extensions to list
- Init State which creates the `VkDevice`
- **Optional**: skip if test is not worth moving out without extension support (more below)

### Pattern for optional extensions

Sometimes it is worth checking for an extension, but still running the parts of a test if the extension is not supported

```cpp
AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
// Check required (not optional) extensions are still supported
RETURN_IF_SKIP(Init());

// need to wait until after phyiscal device creation to know if it was enabled
const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

// Validate core copy command
m_errorMonitor->SetDesiredError(vuid);
vk::CmdCopyBuffer( /* */ );
m_errorMonitor->VerifyFound();

// optional test using VK_KHR_copy_commands2
if (copy_commands2) {
    m_errorMonitor->SetDesiredError(vuid);
    vk::CmdCopyBuffer2KHR( /* */  );
    m_errorMonitor->VerifyFound();
}
```

### Vulkan Version

As [raised in a previous issue](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1553), when a Vulkan version is enabled, all extensions that are required are exposed. (For example, if the test is created as a Vulkan 1.1 application, then the `VK_KHR_16bit_storage` extension would be supported regardless as it was promoted to Vulkan 1.1 core)

If a certain version of Vulkan is needed a test writer can call

```cpp
SetTargetApiVersion(VK_API_VERSION_1_1);
// Will skip if the supported instance version is not high enough, but actual device supported version may be lower
RETURN_IF_SKIP(InitFramework());
```

Later in the test the actual Vulkan version supported by the device can be checked

```cpp
if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
    // Only can be ran on Vulkan 1.0 or 1.1
}
```

### Promoted extensions

The test framework now automatically handles extensions promoted to core versions by not enabling the extensions if the target instance Vulkan version (set through `SetTargetApiVersion`) or the Vulkan version supported by the device (queriable using `DeviceValidationVersion`) already includes the instance or device extension's functionality, respectively.
This applies to both requirements requested using `AddRequiredExtensions` or `AddOptionalExtensions`, as well as optional requirements checked using `IsExtensionsEnabled`.

In order to enforce enabling extensions even when they are included in the effective Vulkan version (which should only be necessary for very specific test cases), the test case can call the `AllowPromotedExtensions` function.

### Getting Function Pointers

When using a version that has promoted the function, one can just directly use the call.

In the case of enabling the extensions, all the functions for those extensions will call `vkGetDeviceProcAddr` automatically.

```cpp
// If VK_API_VERSION_1_1 or later is set
vk::BindImageMemory2(...);

// If VK_KHR_bind_memory2 is enabled
vk::BindImageMemory2KHR(...);
```

## Error Monitor

The `ErrorMonitor *m_errorMonitor` in the `VkRenderFramework` becomes your best friend when you write tests

This small class handles checking all things to VUID and are ultimately what will "pass or fail" a test

The few common patterns that will cover 99% of cases are:

- **By default**, all Vulkan API calls are expected to succeed. In the past, one would have to "wrap" API calls in `ExpectSuccess`/`VerifyNotFound` to ensure an API call did not trigger any errors. This is no longer the case. e.g.,
```cpp
// m_errorMonitor->ExpectSuccess(); <- implicit
vk::CreateSampler(device(), &sci, nullptr, &samplers[0]);
// m_errorMonitor->VerifyNoutFound(); <- implicit
```
The `ExpectSuccess` and `VerifyNotFound` calls are now implicit.
- For checking a call that invokes a VUID error
```cpp
m_errorMonitor->SetDesiredError("VUID-VkSamplerCreateInfo-addressModeU-01646");
// The following API call is expected to trigger 01646 and _only_ 01646
vk::CreateSampler(device(), &sci, NULL, &BadSampler);
m_errorMonitor->VerifyFound();

// All calls after m_errorMonitor->VerifyFound() are expected to not trigger any errors. e.g., the following API call should succeed with no validation errors being triggered.
vk::CreateImage(device(), &ci, nullptr, &mp_image);

```
- When it is possible another VUID will be triggered that you are not testing. This usually happens due to making something invalid can cause a chain effect causing other things to be invalid as well.
    - Note: If the `SetUnexpectedError` is never called it will not fail the test
```cpp
m_errorMonitor->SetUnexpectedError("VUID-VkImageMemoryRequirementsInfo2-image-01590");
m_errorMonitor->SetDesiredError("VUID-VkImageMemoryRequirementsInfo2-image-02280");
vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);
m_errorMonitor->VerifyFound();
```

- When you expect multpile VUID to be triggered. This is also be a case if you expect the same VUID to be called twice.
    - Note: If both VUID are not found the test will fail
```cpp
m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00905");
m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00907");
vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
m_errorMonitor->VerifyFound();
```

- When a VUID is dependent on an extension being present
    - Note: The start of the test might already have a boolean that checks for extension support
```cpp
const char* vuid = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) ? "VUID-vkCmdCopyImage-dstImage-01733" : "VUID-vkCmdCopyImage-dstImage-01733";
m_errorMonitor->SetDesiredError(vuid);
m_command_buffer.CopyImage(image_2.image(), VK_IMAGE_LAYOUT_GENERAL, image_1.image(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
m_errorMonitor->VerifyFound();
```

- There should be a single Vulkan function between m_errorMontior calls to ensure the test is only applied to the single function call being tested.
- Keep it simple. Try to make each test as small and concise as possible.
- Avoid testing VUIDs in "batches" such as:
```cpp
m_errorMonitor->SetDesiredError("VUID-VkCommandBufferBeginInfo-flags-06003");
m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceRenderingInfo-colorAttachmentCount-06004");
m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceRenderingInfo-variableMultisampleRate-06005");
m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06007");
m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceRenderingInfo-multiview-06008");
m_errorMonitor->SetDesiredError("VUID-VkCommandBufferInheritanceRenderingInfo-viewMask-06009");
...
vk::BeginCommandBuffer(secondary_cmd_buffer, &cmd_buffer_begin_info);
m_errorMonitor->VerifyFound();
```
If there is a problem with one of these checks later on, this method makes it difficult and more
time-consuming to figure out _which_ check is problematic. It also makes it difficult to understand
which Vulkan parameter/setting triggered which error. It should be relatively obvious to
tell which line(s) of code caused a validation error to be triggered (and if it isn't, comments should be
used to make it obvious).
- Make it clear how the VUID you're testing is triggered. e.g.,
```cpp
// Try to get layout for plane 3 when we only have 2
VkImageSubresource subresource{};
subresource.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT;
VkSubresourceLayout layout{};
m_errorMonitor->SetDesiredError("VUID-vkGetImageSubresourceLayout-tiling-09433");
vk::GetImageSubresourceLayout(m_device->handle(), image.handle(), &subresource, &layout);
m_errorMonitor->VerifyFound();
```
Here it is obvious that the `aspectMask` parameter is the cause of 02271.

### Viewing VU Messages

When `SetDesiredError` is used, nothing is displayed if the test is successful. To see the messages regardless use `--print-vu`

```bash
./tests/vk_layer_validation_tests --print-vu --gtest_filter=Tests
```

## Device Profiles API

There are times a test writer will want to test a case where an implementation returns a certain support for a format feature or limit. Instead of just hoping to find a device that supports a certain case, there is the Device Profiles API layer. This layer will allow a test writer to inject certain values for format features and/or limits.

### Device Profile Format Feature
Here is an example of how To enable it to allow overriding format features (limits are the same idea, just different function names):
```cpp
RETURN_IF_SKIP(Init());

// Load required functions
PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
    GTEST_SKIP() << "Failed to load device profile layer.";
}
```

This is an example of injecting the `VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT` feature for the `VK_FORMAT_R32G32B32A32_UINT` format. This will force the Validations Layers to act as if the implementation had support for this feature later in the test's code.
```cpp
VkFormatProperties fmt_props;
fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, &fmt_props);
fmt_props.optimalTilingFeatures |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, fmt_props);
```

If you are in need of `VkFormatProperties3` the following is an example how to use the layer

```cpp
PFN_vkSetPhysicalDeviceFormatProperties2EXT fpvkSetPhysicalDeviceFormatProperties2EXT = nullptr;
PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT fpvkGetOriginalPhysicalDeviceFormatProperties2EXT = nullptr;
if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatProperties2EXT, fpvkGetOriginalPhysicalDeviceFormatProperties2EXT)) {
    GTEST_SKIP() << "Failed to load device profile layer.";
}

VkFormatProperties3 fmt_props_3 = vku::InitStructHelper();
VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);

// Removes unwanted support
fpvkGetOriginalPhysicalDeviceFormatProperties2EXT(gpu(), image_format, &fmt_props);
// VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT == VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT
// Need to edit both VkFormatFeatureFlags/VkFormatFeatureFlags2
fmt_props.formatProperties.optimalTilingFeatures = (fmt_props.formatProperties.optimalTilingFeatures & ~VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT);
fmt_props_3.optimalTilingFeatures = (fmt_props_3.optimalTilingFeatures & ~VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT);
// Was added with VkFormatFeatureFlags2 so only need to edit here
fmt_props_3.optimalTilingFeatures = (fmt_props_3.optimalTilingFeatures & ~VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT);
fpvkSetPhysicalDeviceFormatProperties2EXT(gpu(), image_format, fmt_props);

```

### Device Profile Limits

When using the device profile layer for limits, the test maybe need to call `vkSetPhysicalDeviceLimitsEXT` prior to creating the `VkDevice` for some validation state tracking

```cpp
RETURN_IF_SKIP(InitFramework());

// Load required functions
PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
    GTEST_SKIP() << "Failed to load device profile layer.";
}

VkPhysicalDeviceProperties props;
fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
props.limits.maxPushConstantsSize = 16; // example
fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

RETURN_IF_SKIP(InitState());
```
