// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>

#include <vulkan/utility/vk_struct_helper.hpp>

#include <limits>

TEST(struct_helper, structure_type_matches) {
    ASSERT_EQ(VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, vku::GetSType<VkInstanceCreateInfo>());

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    ASSERT_EQ(VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, device_create_info.sType);

    VkImageDrmFormatModifierExplicitCreateInfoEXT image_drm_format_modifier_explicit_create_info = vku::InitStructHelper{};

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&image_drm_format_modifier_explicit_create_info);
    ASSERT_EQ(VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, image_create_info.sType);
    ASSERT_EQ(&image_drm_format_modifier_explicit_create_info, image_create_info.pNext);

    auto buffer_create_info = vku::InitStruct<VkBufferCreateInfo>(
        nullptr, static_cast<VkBufferCreateFlags>(VK_BUFFER_CREATE_SPARSE_BINDING_BIT), std::numeric_limits<uint64_t>::max(),
        static_cast<VkBufferUsageFlags>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VK_SHARING_MODE_EXCLUSIVE, 0U, nullptr);
    ASSERT_EQ(VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, buffer_create_info.sType);
    ASSERT_EQ(static_cast<VkBufferCreateFlags>(VK_BUFFER_CREATE_SPARSE_BINDING_BIT), buffer_create_info.flags);
    ASSERT_EQ(std::numeric_limits<uint64_t>::max(), buffer_create_info.size);
    ASSERT_EQ(static_cast<VkBufferUsageFlags>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), buffer_create_info.usage);
    ASSERT_EQ(VK_SHARING_MODE_EXCLUSIVE, buffer_create_info.sharingMode);
    ASSERT_EQ(0u, buffer_create_info.queueFamilyIndexCount);
    ASSERT_EQ(nullptr, buffer_create_info.pQueueFamilyIndices);
}

TEST(struct_helper, find_struct_in_pnext_chain) {
    VkPhysicalDeviceIDProperties id_props = vku::InitStructHelper();
    VkPhysicalDeviceDriverProperties driver_props = vku::InitStructHelper(&id_props);
    VkPhysicalDeviceCustomBorderColorPropertiesEXT custom_border_color_props = vku::InitStructHelper(&driver_props);
    VkPhysicalDeviceMultiDrawPropertiesEXT multi_draw_props = vku::InitStructHelper(&custom_border_color_props);
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&multi_draw_props);

    ASSERT_EQ(&id_props, vku::FindStructInPNextChain<VkPhysicalDeviceIDProperties>(props2.pNext));
    ASSERT_EQ(&driver_props, vku::FindStructInPNextChain<VkPhysicalDeviceDriverProperties>(props2.pNext));
    ASSERT_EQ(&custom_border_color_props,
              vku::FindStructInPNextChain<VkPhysicalDeviceCustomBorderColorPropertiesEXT>(props2.pNext));
    ASSERT_EQ(&multi_draw_props, vku::FindStructInPNextChain<VkPhysicalDeviceMultiDrawPropertiesEXT>(props2.pNext));

    ASSERT_EQ(reinterpret_cast<VkBaseOutStructure*>(&id_props), vku::FindLastStructInPNextChain(props2.pNext));
}

TEST(struct_helper, find_const_struct_in_pnext_chain) {
    VkImageViewUsageCreateInfo image_view_usage_create_info = vku::InitStructHelper();
    VkImageViewSlicedCreateInfoEXT image_view_sliced_create_info = vku::InitStructHelper(&image_view_usage_create_info);
    VkSamplerYcbcrConversionInfo sampler_ycbcr_conversion_info = vku::InitStructHelper(&image_view_sliced_create_info);
    VkOpaqueCaptureDescriptorDataCreateInfoEXT opaque_capture_descriptor_data_create_info =
        vku::InitStructHelper(&sampler_ycbcr_conversion_info);
    VkImageViewCreateInfo image_view = vku::InitStructHelper(&opaque_capture_descriptor_data_create_info);

    ASSERT_EQ(static_cast<const void*>(&image_view_usage_create_info),
              vku::FindStructInPNextChain<VkImageViewUsageCreateInfo>(image_view.pNext));
    ASSERT_EQ(static_cast<const void*>(&image_view_sliced_create_info),
              vku::FindStructInPNextChain<VkImageViewSlicedCreateInfoEXT>(image_view.pNext));
    ASSERT_EQ(static_cast<const void*>(&sampler_ycbcr_conversion_info),
              vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(image_view.pNext));
    ASSERT_EQ(static_cast<const void*>(&opaque_capture_descriptor_data_create_info),
              vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(image_view.pNext));

    ASSERT_EQ(reinterpret_cast<VkBaseOutStructure*>(&image_view_usage_create_info),
              vku::FindLastStructInPNextChain(const_cast<void*>(image_view.pNext)));
}

struct SomeVkTypes {
    VkImageViewUsageCreateInfo t0{};
    VkImageDrmFormatModifierExplicitCreateInfoEXT t1 = vku::InitStructHelper();
    VkBufferCreateInfo t2 = vku::InitStruct<VkBufferCreateInfo>();
    inline static const auto t3 = vku::InitStruct<VkInstanceCreateInfo>();
    inline static const VkDeviceCreateInfo t4 = vku::InitStructHelper();
};

TEST(struct_helper, struct_defaults_correct) {
    SomeVkTypes s;

    ASSERT_EQ(s.t0.sType, VK_STRUCTURE_TYPE_APPLICATION_INFO);  // should be zero because we didn't initialize it
    ASSERT_EQ(s.t1.sType, VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT);
    ASSERT_EQ(s.t2.sType, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    ASSERT_EQ(s.t3.sType, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
    ASSERT_EQ(s.t4.sType, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
}

#if VK_USE_64_BIT_PTR_DEFINES == 1

TEST(struct_helper, get_object_type) {
    ASSERT_EQ(vku::GetObjectType<VkInstance>(), VK_OBJECT_TYPE_INSTANCE);
    ASSERT_EQ(vku::GetObjectType<VkPerformanceConfigurationINTEL>(), VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL);
    ASSERT_EQ(vku::GetObjectType<VkSwapchainKHR>(), VK_OBJECT_TYPE_SWAPCHAIN_KHR);
    ASSERT_EQ(vku::GetObjectType<VkAccelerationStructureKHR>(), VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR);
    ASSERT_EQ(vku::GetObjectType<VkAccelerationStructureNV>(), VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV);
}

#endif  // VK_USE_64_BIT_PTR_DEFINES == 1
