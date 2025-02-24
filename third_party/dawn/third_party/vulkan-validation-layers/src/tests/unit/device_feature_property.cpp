/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"

class NegativeDeviceFeatureProperty : public VkLayerTest {
  public:
    VkDevice m_second_device = VK_NULL_HANDLE;
    VkDeviceCreateInfo m_second_device_ci = vku::InitStructHelper();
    vkt::QueueCreateInfoArray *m_queue_info = nullptr;
    void InitDeviceFeatureProperty();

    ~NegativeDeviceFeatureProperty() {
        if (m_queue_info) {
            delete m_queue_info;
        }
    }
};

void NegativeDeviceFeatureProperty::InitDeviceFeatureProperty() {
    RETURN_IF_SKIP(Init());
    m_queue_info = new vkt::QueueCreateInfoArray(m_device->Physical().queue_properties_);
    m_second_device_ci.queueCreateInfoCount = m_queue_info->Size();
    m_second_device_ci.pQueueCreateInfos = m_queue_info->Data();
    m_second_device_ci.enabledExtensionCount = static_cast<uint32_t>(m_device_extension_names.size());
    m_second_device_ci.ppEnabledExtensionNames = m_device_extension_names.data();
}

TEST_F(NegativeDeviceFeatureProperty, ShadingRateImage) {
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceShadingRateImageFeaturesNV shading_rate_image_features = vku::InitStructHelper();
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = vku::InitStructHelper(&shading_rate_image_features);
    GetPhysicalDeviceFeatures2(fsr_features);
    if (!shading_rate_image_features.shadingRateImage || !fsr_features.pipelineFragmentShadingRate) {
        GTEST_SKIP() << "Features not supported";
    }
    m_second_device_ci.pNext = &fsr_features;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-shadingRateImage-04478");
    if (fsr_features.primitiveFragmentShadingRate) {
        m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-shadingRateImage-04479");
    }
    if (fsr_features.attachmentFragmentShadingRate) {
        m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-shadingRateImage-04480");
    }
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, SparseImageInt64Atomics) {
    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT atomic_int64_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(atomic_int64_features);
    if (!atomic_int64_features.sparseImageInt64Atomics) {
        GTEST_SKIP() << "Features not supported";
    }
    m_second_device_ci.pNext = &atomic_int64_features;
    atomic_int64_features.shaderImageInt64Atomics = VK_FALSE;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-None-04896");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, SparseImageFloat32Atomics) {
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomic_float_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(atomic_float_features);
    if (!atomic_float_features.sparseImageFloat32Atomics) {
        GTEST_SKIP() << "Features not supported";
    }
    m_second_device_ci.pNext = &atomic_float_features;
    atomic_float_features.shaderImageFloat32Atomics = VK_FALSE;
    atomic_float_features.sparseImageFloat32AtomicAdd = VK_FALSE;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-None-04897");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, SparseImageFloat32AtomicAdd) {
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomic_float_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(atomic_float_features);
    if (!atomic_float_features.sparseImageFloat32AtomicAdd) {
        GTEST_SKIP() << "Features not supported";
    }
    m_second_device_ci.pNext = &atomic_float_features;
    atomic_float_features.shaderImageFloat32AtomicAdd = VK_FALSE;
    atomic_float_features.sparseImageFloat32Atomics = VK_FALSE;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-None-04898");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, SparseImageFloat32AtomicMinMax) {
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT atomic_float2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(atomic_float2_features);
    if (!atomic_float2_features.sparseImageFloat32AtomicMinMax) {
        GTEST_SKIP() << "Features not supported";
    }
    m_second_device_ci.pNext = &atomic_float2_features;
    atomic_float2_features.shaderImageFloat32AtomicMinMax = VK_FALSE;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-sparseImageFloat32AtomicMinMax-04975");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, PipelineBinaryInternalCacheControl) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PIPELINE_BINARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineBinaries);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDevicePipelineBinaryPropertiesKHR pipeline_binary_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(pipeline_binary_properties);
    if (pipeline_binary_properties.pipelineBinaryInternalCacheControl) {
        GTEST_SKIP() << "pipelineBinaryInternalCacheControl is VK_TRUE";
    }

    VkDevicePipelineBinaryInternalCacheControlKHR pbicc = vku::InitStructHelper();
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&pbicc);
    m_second_device_ci.pNext = &features2;
    pbicc.disableInternalCache = true;

    m_errorMonitor->SetDesiredError("VUID-VkDevicePipelineBinaryInternalCacheControlKHR-disableInternalCache-09602");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, DescriptorBuffer) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_AMD_SHADER_FRAGMENT_MASK_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceDescriptorBufferFeaturesEXT dbf = vku::InitStructHelper();
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&dbf);
    m_second_device_ci.pNext = &features2;
    dbf.descriptorBuffer = true;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-None-08095");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, PromotedFeaturesExtensions11) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    std::vector<const char *> device_extensions;
    if (DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME)) {
        device_extensions.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
    }

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper();
    features11.shaderDrawParameters = VK_FALSE;

    m_second_device_ci.pNext = &features11;
    m_second_device_ci.ppEnabledExtensionNames = device_extensions.data();
    m_second_device_ci.enabledExtensionCount = device_extensions.size();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-04476");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");  // for android loader
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, PromotedFeaturesExtensions12) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    std::vector<const char *> device_extensions;
    if (DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME)) {
        device_extensions.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
        m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02831");
    }
    if (DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME)) {
        device_extensions.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
        m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02832");
    }
    if (DeviceExtensionSupported(Gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)) {
        device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        device_extensions.push_back(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
        m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02833");
    }
    if (DeviceExtensionSupported(Gpu(), nullptr, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME)) {
        device_extensions.push_back(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
        m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02834");
    }
    if (DeviceExtensionSupported(Gpu(), nullptr, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME)) {
        device_extensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
        m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02835");
    }

    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    features12.drawIndirectCount = VK_FALSE;
    features12.samplerMirrorClampToEdge = VK_FALSE;
    features12.descriptorIndexing = VK_FALSE;
    features12.samplerFilterMinmax = VK_FALSE;
    features12.shaderOutputViewportIndex = VK_FALSE;
    features12.shaderOutputLayer = VK_TRUE;  // Set true since both shader_viewport features need to true

    m_second_device_ci.pNext = &features12;
    m_second_device_ci.ppEnabledExtensionNames = device_extensions.data();
    m_second_device_ci.enabledExtensionCount = device_extensions.size();

    m_errorMonitor->SetUnexpectedError("Failed to create device chain");  // for android loader
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, PhysicalDeviceFeatures2) {
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    VkPhysicalDeviceFeatures2 pd_features2 = vku::InitStructHelper();
    m_second_device_ci.pNext = &pd_features2;

    // VUID-VkDeviceCreateInfo-pNext-pNext
    m_errorMonitor->SetDesiredError("its parent extension VK_KHR_get_physical_device_properties2 has not been enabled");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");  // for android loader
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, VertexAttributeDivisor) {
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = vku::InitStructHelper();
    m_second_device_ci.pNext = &vadf;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, PhysicalDeviceVulkan11Features) {
    TEST_DESCRIPTION("Use both VkPhysicalDeviceVulkan11Features mixed with the old struct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDevice16BitStorageFeatures sixteen_bit = vku::InitStructHelper();
    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper(&sixteen_bit);
    m_second_device_ci.pNext = &features11;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-02829");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");  // for android loader
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, PhysicalDeviceVulkan12Features) {
    TEST_DESCRIPTION("Use both VkPhysicalDeviceVulkan12Features mixed with the old struct");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDevice8BitStorageFeatures eight_bit = vku::InitStructHelper();
    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper(&eight_bit);
    m_second_device_ci.pNext = &features12;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-02830");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");  // for android loader
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, PhysicalDeviceVulkan13Features) {
    TEST_DESCRIPTION("Use both VkPhysicalDeviceVulkan13Features mixed with the old struct");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering = vku::InitStructHelper();
    VkPhysicalDeviceVulkan13Features features13 = vku::InitStructHelper(&dynamic_rendering);
    m_second_device_ci.pNext = &features13;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-06532");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");  // for android loader
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, Maintenance1AndNegativeViewport) {
    TEST_DESCRIPTION("Attempt to enable AMD_negative_viewport_height and Maintenance1_KHR extension simultaneously");
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    if (!((DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) &&
          (DeviceExtensionSupported(Gpu(), nullptr, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME)))) {
        GTEST_SKIP() << "Maintenance1 and AMD_negative viewport height extensions not supported";
    }

    std::array extension_names = {VK_KHR_MAINTENANCE_1_EXTENSION_NAME, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME};
    m_second_device_ci.enabledExtensionCount = extension_names.size();
    m_second_device_ci.ppEnabledExtensionNames = extension_names.data();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-00374");
    // The following unexpected error is coming from the LunarG loader. Do not make it a desired message because platforms that do
    // not use the LunarG loader (e.g. Android) will not see the message and the test will fail.
    m_errorMonitor->SetUnexpectedError("Failed to create device chain.");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, BufferDeviceAddressExtAndKhr) {
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    if (!((DeviceExtensionSupported(Gpu(), nullptr, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)) &&
          (DeviceExtensionSupported(Gpu(), nullptr, VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)))) {
        GTEST_SKIP() << "Both buffer device address extensions not enabled";
    }

    std::array extension_names = {VK_KHR_DEVICE_GROUP_EXTENSION_NAME, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                                  VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME};
    m_second_device_ci.enabledExtensionCount = extension_names.size();
    m_second_device_ci.ppEnabledExtensionNames = extension_names.data();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-03328");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain.");  // for android loader
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, NegativeViewportHeight11) {
    TEST_DESCRIPTION("Attempt to enable AMD_negative_viewport_height with api version 1.1");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    if (!DeviceExtensionSupported(Gpu(), nullptr, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME)) {
        GTEST_SKIP() << "AMD_negative viewport height extensions not supported";
    }

    const char *extension_names = VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME;
    m_second_device_ci.enabledExtensionCount = 1;
    m_second_device_ci.ppEnabledExtensionNames = &extension_names;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-01840");
    // The following unexpected error is coming from the LunarG loader. Do not make it a desired message because platforms that do
    // not use the LunarG loader (e.g. Android) will not see the message and the test will fail.
    m_errorMonitor->SetUnexpectedError("Failed to create device chain.");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, BufferDeviceAddress) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    features12.bufferDeviceAddress = VK_TRUE;
    m_second_device_ci.pNext = &features12;

    const char *extension_names = VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
    m_second_device_ci.enabledExtensionCount = 1;
    m_second_device_ci.ppEnabledExtensionNames = &extension_names;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-04748");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, PhysicalDeviceGlobalPriorityQueryFeaturesKHR) {
    TEST_DESCRIPTION(
        "VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR has an EXT and KHR extension that can enable it, but we forgot both");
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    if (!DeviceExtensionSupported(VK_KHR_GLOBAL_PRIORITY_EXTENSION_NAME) &&
        !DeviceExtensionSupported(VK_EXT_GLOBAL_PRIORITY_QUERY_EXTENSION_NAME)) {
        GTEST_SKIP() << "VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR not supported";
    }
    VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR query_feature = vku::InitStructHelper();
    query_feature.globalPriorityQuery = VK_TRUE;
    m_second_device_ci.pNext = &query_feature;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, MissingExtensionPhysicalDeviceFeature) {
    TEST_DESCRIPTION("Add feature to vkCreateDevice withouth extension");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    if (!DeviceExtensionSupported(VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_EXT_astc_decode_mode not supported";
    }
    // likely to never be promoted to core
    VkPhysicalDeviceASTCDecodeFeaturesEXT astc_feature = vku::InitStructHelper();
    astc_feature.decodeModeSharedExponent = VK_TRUE;
    m_second_device_ci.pNext = &astc_feature;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, MissingExtensionPromoted) {
    TEST_DESCRIPTION("Add feature to vkCreateDevice withouth extension (for a promoted extension)");
    SetTargetApiVersion(VK_API_VERSION_1_2);  // VK_KHR_maintenance4 added in 1.3
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    if (!DeviceExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_maintenance4 not supported";
    }
    VkPhysicalDeviceMaintenance4Features maintenance4_feature = vku::InitStructHelper();
    maintenance4_feature.maintenance4 = VK_TRUE;
    m_second_device_ci.pNext = &maintenance4_feature;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, Features11WithoutVulkan12) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    TEST_DESCRIPTION("VkPhysicalDeviceVulkan11Features was added in Vulkan1.2");
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper();
    m_second_device_ci.pNext = &features11;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, Robustness2WithoutRobustness) {
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceRobustness2FeaturesEXT robustness_2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(robustness_2_features);
    if (!robustness_2_features.robustBufferAccess2) {
        GTEST_SKIP() << "robustBufferAccess2 not supported.";
    }

    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&robustness_2_features);
    m_second_device_ci.pNext = &features2;

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceRobustness2FeaturesEXT-robustBufferAccess2-04000");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, EnabledFeatures) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    m_second_device_ci.pNext = &features2;

    auto features = vkt::PhysicalDevice(Gpu()).Features();
    m_second_device_ci.pEnabledFeatures = &features;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-00373");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, VariablePointer) {
    TEST_DESCRIPTION("Checks VK_KHR_variable_pointers features.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceVariablePointersFeatures variable_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(variable_features);
    if (!variable_features.variablePointers) {
        GTEST_SKIP() << "variablePointer feature not supported";
    }
    variable_features.variablePointersStorageBuffer = VK_FALSE;
    m_second_device_ci.pNext = &variable_features;

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceVariablePointersFeatures-variablePointers-01431");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, MultiviewMeshShader) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.multiviewMeshShader) {
        GTEST_SKIP() << "multiviewMeshShader not supported";
    }
    mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;

    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&mesh_shader_features);
    m_second_device_ci.pNext = &features2;

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceMeshShaderFeaturesEXT-multiviewMeshShader-07032");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, PrimitiveFragmentShadingRateMeshShader) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.primitiveFragmentShadingRateMeshShader) {
        GTEST_SKIP() << "primitiveFragmentShadingRateMeshShader not supported";
    }
    mesh_shader_features.multiviewMeshShader = VK_FALSE;

    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&mesh_shader_features);
    m_second_device_ci.pNext = &features2;

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceMeshShaderFeaturesEXT-primitiveFragmentShadingRateMeshShader-07033");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, RobustBufferAccessUpdateAfterBind) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceDescriptorIndexingProperties di_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(di_props);
    if (di_props.robustBufferAccessUpdateAfterBind) {
        GTEST_SKIP() << "robustBufferAccessUpdateAfterBind is VK_TRUE";
    }

    VkPhysicalDeviceDescriptorIndexingFeatures di_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(di_features);
    if (!di_features.descriptorBindingUniformBufferUpdateAfterBind) {
        GTEST_SKIP() << "Features not supported";
    }

    auto features = vkt::PhysicalDevice(Gpu()).Features();
    if (!features.robustBufferAccess) {
        GTEST_SKIP() << "robustBufferAccess not supported";
    }

    m_second_device_ci.pEnabledFeatures = &features;
    m_second_device_ci.pNext = &di_features;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-robustBufferAccess-10247");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, RobustBufferAccessUpdateAfterBind12) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceVulkan12Properties props_12 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props_12);
    if (props_12.robustBufferAccessUpdateAfterBind) {
        GTEST_SKIP() << "robustBufferAccessUpdateAfterBind is VK_TRUE";
    }

    VkPhysicalDeviceVulkan12Features features_12 = vku::InitStructHelper();
    auto features = GetPhysicalDeviceFeatures2(features_12);
    if (!features_12.descriptorBindingUniformBufferUpdateAfterBind) {
        GTEST_SKIP() << "Features not supported";
    }
    features.features.robustBufferAccess = VK_TRUE;
    m_second_device_ci.pNext = &features;
    m_second_device_ci.pEnabledFeatures = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-robustBufferAccess-10247");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceFeatureProperty, Create14DeviceDuplicatedFeatures) {
    SetTargetApiVersion(VK_API_VERSION_1_4);
    RETURN_IF_SKIP(InitDeviceFeatureProperty());

    VkPhysicalDeviceHostImageCopyFeatures features_hic = vku::InitStructHelper();
    VkPhysicalDeviceVulkan14Features features_14 = vku::InitStructHelper(&features_hic);
    m_second_device_ci.pNext = &features_14;
    m_second_device_ci.pEnabledFeatures = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-10360");
    vk::CreateDevice(Gpu(), &m_second_device_ci, nullptr, &m_second_device);
    m_errorMonitor->VerifyFound();
}
