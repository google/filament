/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021 ARM, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/ray_tracing_objects.h"

class NegativeDescriptorBuffer : public DescriptorBufferTest {};

TEST_F(NegativeDescriptorBuffer, SetLayout) {
    TEST_DESCRIPTION("Descriptor buffer set layout tests.");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                   nullptr};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08000");
        vk::CreateDescriptorSetLayout(device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                   nullptr};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08000");
        vk::CreateDescriptorSetLayout(device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
        const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08001");
        vk::CreateDescriptorSetLayout(device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        const VkDescriptorSetLayoutCreateFlags flags =
            VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT | VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08002");
        vk::CreateDescriptorSetLayout(device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        VkSampler samplers[2] = {sampler.handle(), sampler.handle()};
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, samplers};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
        const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-flags-08005");
        vk::CreateDescriptorSetLayout(device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        VkSampler samplers[2] = {sampler.handle(), sampler.handle()};
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, samplers};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
        const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-flags-08006");
        vk::CreateDescriptorSetLayout(device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
        const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-flags-08007");
        vk::CreateDescriptorSetLayout(device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }

    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        vkt::DescriptorSetLayout dsl1(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
        vkt::DescriptorSetLayout dsl2(*m_device, binding, 0);

        VkPipelineLayout pipeline_layout;
        const std::array<VkDescriptorSetLayout, 2> set_layouts{dsl1.handle(), dsl2.handle()};
        VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
        plci.setLayoutCount = size32(set_layouts);
        plci.pSetLayouts = set_layouts.data();

        m_errorMonitor->SetDesiredError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-08008");
        vk::CreatePipelineLayout(device(), &plci, NULL, &pipeline_layout);
        m_errorMonitor->VerifyFound();
    }

    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        vkt::DescriptorSetLayout dsl1(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

        VkDescriptorPoolSize pool_size = {binding.descriptorType, binding.descriptorCount};
        const auto dspci =
            vku::InitStruct<VkDescriptorPoolCreateInfo>(nullptr, static_cast<VkDescriptorPoolCreateFlags>(0), 1U, 1U, &pool_size);
        vkt::DescriptorPool pool(*m_device, dspci);

        VkDescriptorSet ds = VK_NULL_HANDLE;
        const auto alloc_info = vku::InitStruct<VkDescriptorSetAllocateInfo>(nullptr, pool.handle(), 1U, &dsl1.handle());

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-08009");
        vk::AllocateDescriptorSets(device(), &alloc_info, &ds);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorBuffer, GetSupportSetLayout) {
    TEST_DESCRIPTION("call vkGetDescriptorSetLayoutSupport on Descriptor buffer set layout tests.");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                               nullptr};
    const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
    VkDescriptorSetLayoutSupport support = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08000");
    vk::GetDescriptorSetLayoutSupport(device(), &dslci, &support);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, SetLayoutInlineUniformBlockEXT) {
    TEST_DESCRIPTION("Descriptor buffer set layout tests.");
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::inlineUniformBlock);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkPhysicalDeviceInlineUniformBlockPropertiesEXT inlineUniformProps = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(inlineUniformProps);

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
                                               inlineUniformProps.maxInlineUniformBlockSize + 4, VK_SHADER_STAGE_FRAGMENT_BIT,
                                               nullptr};
    const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, 0U, 1U, &binding);
    VkDescriptorSetLayout dsl;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutBinding-descriptorType-08004");
    vk::CreateDescriptorSetLayout(device(), &dslci, nullptr, &dsl);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, SetLayoutMutableDescriptorEXT) {
    TEST_DESCRIPTION("Descriptor buffer set layout tests.");
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::mutableDescriptorType);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutCreateFlags flags =
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT | VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT;
    const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
    VkDescriptorSetLayout dsl;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08003");
    vk::CreateDescriptorSetLayout(device(), &dslci, nullptr, &dsl);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, NotEnabled) {
    TEST_DESCRIPTION("Tests for when descriptor buffer is not enabled");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()};
    vkt::DescriptorSetLayout dsl(*m_device, binding,
                                 VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                     VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT);

    {
        VkDeviceSize size;

        m_errorMonitor->SetDesiredError("VUID-vkGetDescriptorSetLayoutSizeEXT-None-08011");
        vk::GetDescriptorSetLayoutSizeEXT(device(), dsl.handle(), &size);
        m_errorMonitor->VerifyFound();
    }

    {
        VkDeviceSize offset;

        m_errorMonitor->SetDesiredError("VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-None-08013");
        vk::GetDescriptorSetLayoutBindingOffsetEXT(device(), dsl.handle(), 0, &offset);
        m_errorMonitor->VerifyFound();
    }

    {
        uint8_t buffer[128];
        VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();
        dgi.type = VK_DESCRIPTOR_TYPE_SAMPLER;
        dgi.data.pSampler = &sampler.handle();

        m_errorMonitor->SetDesiredError("VUID-vkGetDescriptorEXT-None-08015");
        vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.samplerDescriptorSize, &buffer);
        m_errorMonitor->VerifyFound();
    }

    {
        VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
        plci.setLayoutCount = 1;
        plci.pSetLayouts = &dsl.handle();
        vkt::PipelineLayout pipeline_layout(*m_device, plci);

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-None-08068");
        vk::CmdBindDescriptorBufferEmbeddedSamplersEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                       pipeline_layout.handle(), 0);
        m_errorMonitor->VerifyFound();

        m_command_buffer.End();
    }
}

TEST_F(NegativeDescriptorBuffer, NotEnabledBufferDeviceAddress) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    RETURN_IF_SKIP(Init());

    vkt::Buffer d_buffer(*m_device, 4096,
                         VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
                         vkt::device_address);

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper();
    dbbi.address = d_buffer.Address();
    dbbi.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    d_buffer.Memory().destroy();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-None-08047");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDescriptorBuffer, NotEnabledGetBufferOpaqueCaptureDescriptorDataEXT) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    uint8_t data[256];
    vkt::Buffer temp_buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkBufferCaptureDescriptorDataInfoEXT bcddi = vku::InitStructHelper();
    bcddi.buffer = temp_buffer.handle();

    m_errorMonitor->SetDesiredError("VUID-vkGetBufferOpaqueCaptureDescriptorDataEXT-None-08072");
    m_errorMonitor->SetDesiredError("VUID-VkBufferCaptureDescriptorDataInfoEXT-buffer-08075");
    vk::GetBufferOpaqueCaptureDescriptorDataEXT(device(), &bcddi, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, NotEnabledGetImageOpaqueCaptureDescriptorDataEXT) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    uint8_t data[256];

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = 128;
    image_create_info.extent.height = 128;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.format = VK_FORMAT_D32_SFLOAT;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image image(*m_device, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkImageCaptureDescriptorDataInfoEXT icddi = vku::InitStructHelper();
    icddi.image = image.handle();

    m_errorMonitor->SetDesiredError("VUID-vkGetImageOpaqueCaptureDescriptorDataEXT-None-08076");
    m_errorMonitor->SetDesiredError("VUID-VkImageCaptureDescriptorDataInfoEXT-image-08079");
    vk::GetImageOpaqueCaptureDescriptorDataEXT(device(), &icddi, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, NotEnabledGetImageViewOpaqueCaptureDescriptorDataEXT) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());
    uint8_t data[256];

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = 128;
    image_create_info.extent.height = 128;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.format = VK_FORMAT_D32_SFLOAT;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image image(*m_device, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // missing VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
    vkt::ImageView dsv = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    VkImageViewCaptureDescriptorDataInfoEXT icddi = vku::InitStructHelper();
    icddi.imageView = dsv.handle();

    m_errorMonitor->SetDesiredError("VUID-vkGetImageViewOpaqueCaptureDescriptorDataEXT-None-08080");
    m_errorMonitor->SetDesiredError("VUID-VkImageViewCaptureDescriptorDataInfoEXT-imageView-08083");
    vk::GetImageViewOpaqueCaptureDescriptorDataEXT(device(), &icddi, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, NotEnabledGetSamplerOpaqueCaptureDescriptorDataEXT) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    uint8_t data[256];

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    VkSamplerCaptureDescriptorDataInfoEXT scddi = vku::InitStructHelper();
    scddi.sampler = sampler.handle();

    m_errorMonitor->SetDesiredError("VUID-vkGetSamplerOpaqueCaptureDescriptorDataEXT-None-08084");
    m_errorMonitor->SetDesiredError("VUID-VkSamplerCaptureDescriptorDataInfoEXT-sampler-08087");
    vk::GetSamplerOpaqueCaptureDescriptorDataEXT(device(), &scddi, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, NotEnabledGetAccelerationStructureOpaqueCaptureDescriptorDataEXT) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);

    RETURN_IF_SKIP(Init());

    auto blas = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096);
    blas->Build();

    uint8_t data[256];

    VkAccelerationStructureCaptureDescriptorDataInfoEXT ascddi = vku::InitStructHelper();
    ascddi.accelerationStructure = blas->handle();

    m_errorMonitor->SetDesiredError("VUID-vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT-None-08088");
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructure-08091");
    vk::GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(device(), &ascddi, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, NotEnabledDescriptorBufferCaptureReplay) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    uint32_t data[128];
    const auto ocddci = vku::InitStruct<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(nullptr, &data);

    {
        VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
        buffer_ci.size = 4096;
        buffer_ci.flags = VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        buffer_ci.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

        buffer_ci.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
        CreateBufferTest(*this, &buffer_ci, "VUID-VkBufferCreateInfo-flags-08099");

        buffer_ci.flags = 0;

        if (descriptor_buffer_properties.bufferlessPushDescriptors) {
            m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-usage-08102");
        }
        buffer_ci.usage =
            VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
        CreateBufferTest(*this, &buffer_ci, "VUID-VkBufferCreateInfo-usage-08101");

        buffer_ci.pNext = &ocddci;
        buffer_ci.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
        CreateBufferTest(*this, &buffer_ci, "VUID-VkBufferCreateInfo-pNext-08100");
    }

    {
        VkImageCreateInfo image_create_info = vku::InitStructHelper();
        image_create_info.flags |= VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = 128;
        image_create_info.extent.height = 128;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.format = VK_FORMAT_D32_SFLOAT;
        image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        image_create_info.pNext = &ocddci;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-08104");

        image_create_info.pNext = &ocddci;
        image_create_info.flags &= ~VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-pNext-08105");
    }

    {
        vkt::Image temp_image(*m_device, 64, 64, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        VkImageViewCreateInfo dsvci = vku::InitStructHelper();
        dsvci.flags |= VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        dsvci.image = temp_image.handle();
        dsvci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        dsvci.format = VK_FORMAT_D32_SFLOAT;
        dsvci.subresourceRange.layerCount = 1;
        dsvci.subresourceRange.baseMipLevel = 0;
        dsvci.subresourceRange.levelCount = 1;
        dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        CreateImageViewTest(*this, &dsvci, "VUID-VkImageViewCreateInfo-flags-08106");

        dsvci.pNext = &ocddci;
        dsvci.flags &= ~VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        CreateImageViewTest(*this, &dsvci, "VUID-VkImageViewCreateInfo-pNext-08107");
    }

    {
        auto sampler_ci = SafeSaneSamplerCreateInfo();
        sampler_ci.flags |= VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;

        CreateSamplerTest(*this, &sampler_ci, "VUID-VkSamplerCreateInfo-flags-08110");

        sampler_ci.pNext = &ocddci;
        sampler_ci.flags &= ~VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        CreateSamplerTest(*this, &sampler_ci, "VUID-VkSamplerCreateInfo-pNext-08111");
    }
}

TEST_F(NegativeDescriptorBuffer, NotEnabledDescriptorBufferCaptureReplayAS) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);

    RETURN_IF_SKIP(Init());

    uint32_t data[128];
    const auto ocddci = vku::InitStruct<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(nullptr, &data);

    vkt::Buffer as_buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfoKHR asci = vku::InitStructHelper();
    asci.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
    asci.buffer = as_buffer.handle();

    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoKHR-createFlags-08108");
    vk::CreateAccelerationStructureKHR(device(), &asci, NULL, &as);
    m_errorMonitor->VerifyFound();

    asci.pNext = &ocddci;
    asci.createFlags &= ~VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoKHR-pNext-08109");
    vk::CreateAccelerationStructureKHR(device(), &asci, NULL, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, BufferlessPushDescriptorsOff) {
    TEST_DESCRIPTION("When bufferlessPushDescriptors is not supported.");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorBufferPushDescriptors);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer(&kDisableMessageLimit));

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);
    if (descriptor_buffer_properties.bufferlessPushDescriptors) {
        GTEST_SKIP() << "bufferlessPushDescriptors is supported";
    }

    m_command_buffer.Begin();

    vkt::Buffer d_buffer(*m_device, 4096,
                         VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                             VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT,
                         vkt::device_address);

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper();
    dbbi.address = d_buffer.Address();
    dbbi.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT;

    std::vector<VkDescriptorBufferBindingInfoEXT> binding_infos;
    for (uint32_t i = 0; i < descriptor_buffer_properties.maxDescriptorBufferBindings + 1; i++) {
        binding_infos.push_back(dbbi);
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-maxSamplerDescriptorBufferBindings-08048");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-maxResourceDescriptorBufferBindings-08049");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-None-08050");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingInfoEXT-bufferlessPushDescriptors-08056",
                                    descriptor_buffer_properties.maxDescriptorBufferBindings + 1);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-bufferCount-08051");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), binding_infos.size(), binding_infos.data());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, BufferlessPushDescriptors) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorBufferPushDescriptors);

    RETURN_IF_SKIP(InitFramework(&kDisableMessageLimit));
    RETURN_IF_SKIP(InitState());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);
    if (!descriptor_buffer_properties.bufferlessPushDescriptors) {
        GTEST_SKIP() << "bufferlessPushDescriptors is not supported";
    }

    m_command_buffer.Begin();

    vkt::Buffer d_buffer(*m_device, 4096,
                         VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
                         vkt::device_address);

    VkDescriptorBufferBindingPushDescriptorBufferHandleEXT dbbpdbh = vku::InitStructHelper();
    dbbpdbh.buffer = d_buffer.handle();

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper(&dbbpdbh);
    dbbi.address = d_buffer.Address();
    dbbi.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    std::vector<VkDescriptorBufferBindingInfoEXT> binding_infos;
    for (uint32_t i = 0; i < descriptor_buffer_properties.maxDescriptorBufferBindings + 1; i++) {
        binding_infos.push_back(dbbi);
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-maxSamplerDescriptorBufferBindings-08048");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-maxResourceDescriptorBufferBindings-08049");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-bufferCount-08051");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingPushDescriptorBufferHandleEXT-bufferlessPushDescriptors-08059",
                                    descriptor_buffer_properties.maxDescriptorBufferBindings + 1);
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), binding_infos.size(), binding_infos.data());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingPushDescriptorBufferHandleEXT-bufferlessPushDescriptors-08059");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, DescriptorBufferOffsetAlignment) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);
    if (descriptor_buffer_properties.descriptorBufferOffsetAlignment == 1) {
        GTEST_SKIP() << "descriptorBufferOffsetAlignment is 1";
    }

    m_command_buffer.Begin();

    vkt::Buffer d_buffer(*m_device, 4096,
                         VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
                         vkt::device_address);

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper();
    dbbi.address = d_buffer.Address() + 1;  // make alignment bad
    dbbi.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingInfoEXT-address-08057");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, BindingInfoUsage) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    m_command_buffer.Begin();

    vkt::Buffer buffer(*m_device, 4096, 0, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper();
    dbbi.address = buffer.Address();
    dbbi.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingInfoEXT-usage-08122");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08055");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);
    m_errorMonitor->VerifyFound();

    dbbi.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingInfoEXT-usage-08123");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08055");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);
    m_errorMonitor->VerifyFound();

    dbbi.usage = VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT;
    if (descriptor_buffer_properties.bufferlessPushDescriptors == VK_FALSE) {
        m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingInfoEXT-bufferlessPushDescriptors-08056");
    }
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingInfoEXT-usage-08124");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08055");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, BindingInfoUsage2) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9228");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    m_command_buffer.Begin();

    vkt::Buffer buffer(*m_device, 4096, 0, vkt::device_address);

    VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper(&buffer_usage_flags);
    dbbi.address = buffer.Address();

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingInfoEXT-usage-08122");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08055");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, BindingInfoUsageMultiBuffers) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    m_command_buffer.Begin();

    vkt::Buffer buffer_good1(*m_device, 4096, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);
    vkt::Buffer buffer_good2(*m_device, 4096, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);
    vkt::Buffer buffer_bad(*m_device, 4096, 0, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper();
    dbbi.address = buffer_bad.Address();
    dbbi.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingInfoEXT-usage-08122");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08055");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, CmdBindDescriptorBufferEmbeddedSamplers) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    m_command_buffer.Begin();

    VkDescriptorSetLayoutBinding binding1 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    vkt::DescriptorSetLayout dsl1(*m_device, binding1, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    VkDescriptorSetLayoutBinding binding2 = {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()};
    vkt::DescriptorSetLayout dsl2(*m_device, binding2,
                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                      VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT);

    const VkDescriptorSetLayout set_layouts[2] = {dsl1.handle(), dsl2.handle()};
    VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
    plci.setLayoutCount = 2;
    plci.pSetLayouts = set_layouts;
    vkt::PipelineLayout pipeline_layout(*m_device, plci);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-set-08070");
    vk::CmdBindDescriptorBufferEmbeddedSamplersEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                   pipeline_layout.handle(), 0);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-set-08071");
    vk::CmdBindDescriptorBufferEmbeddedSamplersEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                   pipeline_layout.handle(), 2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, CmdSetDescriptorBufferOffsets) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    m_command_buffer.Begin();

    VkDescriptorSetLayoutBinding binding1 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    vkt::DescriptorSetLayout dsl1(*m_device, binding1, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    VkDescriptorSetLayoutBinding binding2 = {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()};
    vkt::DescriptorSetLayout dsl2(*m_device, binding2,
                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                      VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT);

    const VkDescriptorSetLayout set_layouts[2] = {dsl1.handle(), dsl2.handle()};
    VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
    plci.setLayoutCount = 2;
    plci.pSetLayouts = set_layouts;
    vkt::PipelineLayout pipeline_layout(*m_device, plci);

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper();
    dbbi.address = buffer.Address();
    dbbi.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);

    uint32_t index = 0;
    VkDeviceSize offset = 0;

    if (descriptor_buffer_properties.descriptorBufferOffsetAlignment != 1) {
        index = 0;
        offset = 1;
        m_errorMonitor->SetDesiredError("VUID-vkCmdSetDescriptorBufferOffsetsEXT-pOffsets-08061");
        vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(),
                                             0, 1, &index, &offset);
        m_errorMonitor->VerifyFound();
    }

    index = descriptor_buffer_properties.maxDescriptorBufferBindings;
    offset = 0;
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetDescriptorBufferOffsetsEXT-pBufferIndices-08064");
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetDescriptorBufferOffsetsEXT-pBufferIndices-08065");
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                         &index, &offset);
    m_errorMonitor->VerifyFound();

    uint32_t indices[3] = {0};
    VkDeviceSize offsets[3] = {0};

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetDescriptorBufferOffsetsEXT-firstSet-08066");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdSetDescriptorBufferOffsetsEXT-pOffsets-08063");
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 3,
                                         indices, offsets);
    m_errorMonitor->VerifyFound();

    const std::optional<uint32_t> compute_qfi = m_device->ComputeOnlyQueueFamily();
    if (compute_qfi) {
        vkt::CommandPool command_pool(*m_device, compute_qfi.value());
        ASSERT_TRUE(command_pool.initialized());
        vkt::CommandBuffer command_buffer(*m_device, command_pool);
        index = 0;
        offset = 0;

        command_buffer.Begin();
        vk::CmdBindDescriptorBuffersEXT(command_buffer.handle(), 1, &dbbi);

        m_errorMonitor->SetDesiredError("VUID-vkCmdSetDescriptorBufferOffsetsEXT-pipelineBindPoint-08067");
        vk::CmdSetDescriptorBufferOffsetsEXT(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                                             1, &index, &offset);
        m_errorMonitor->VerifyFound();
        command_buffer.End();
    }

    {
        const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
        const vkt::DescriptorSetLayout set_layout_no_flag(*m_device, {binding});
        const vkt::PipelineLayout pipeline_layout_2(*m_device, {&set_layout_no_flag, &set_layout_no_flag});

        const uint32_t indices_2[2] = {0, 0};
        const VkDeviceSize offsets_2[2] = {0, 0};
        vk::CmdBindDescriptorBuffersEXT(m_command_buffer, 1, &dbbi);
        // complain about set layout for set 0
        m_errorMonitor->SetDesiredError("VUID-vkCmdSetDescriptorBufferOffsetsEXT-firstSet-09006");
        // complain about set layout for set 1
        m_errorMonitor->SetDesiredError("VUID-vkCmdSetDescriptorBufferOffsetsEXT-firstSet-09006");
        vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_2, 0, 2, indices_2,
                                             offsets_2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorBuffer, BindingAndOffsets) {
    TEST_DESCRIPTION("Test mapping from address to buffers when validating buffer offsets");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    m_command_buffer.Begin();

    VkDescriptorSetLayoutBinding binding1 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    vkt::DescriptorSetLayout dsl1(*m_device, binding1, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    VkDescriptorSetLayoutBinding binding2 = {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()};
    vkt::DescriptorSetLayout dsl2(*m_device, binding2,
                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                      VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT);

    const VkDescriptorSetLayout set_layouts[2] = {dsl1.handle(), dsl2.handle()};
    VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
    plci.setLayoutCount = 2;
    plci.pSetLayouts = set_layouts;
    vkt::PipelineLayout pipeline_layout(*m_device, plci);

    const VkDeviceSize large_buffer_size =
        std::max<VkDeviceSize>(256 * descriptor_buffer_properties.descriptorBufferOffsetAlignment, 8192);
    const VkDeviceSize small_buffer_size =
        std::max<VkDeviceSize>(4 * descriptor_buffer_properties.descriptorBufferOffsetAlignment, 4096);

    // Create a large and a small buffer
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = large_buffer_size;
    buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    vkt::Buffer large_buffer(*m_device, buffer_ci, vkt::no_mem);

    buffer_ci.size = small_buffer_size;
    vkt::Buffer small_buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), large_buffer.handle(), &buffer_mem_reqs);

    // Allocate common buffer memory
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->Physical().SetMemoryType(buffer_mem_reqs.memoryTypeBits, &alloc_info, 0);
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Bind those buffers to the same buffer memory
    vk::BindBufferMemory(device(), large_buffer.handle(), buffer_memory.handle(), 0);
    vk::BindBufferMemory(device(), small_buffer.handle(), buffer_memory.handle(), 0);

    // Check that internal mapping from address to buffers is correctly updated
    if (large_buffer.Address() != small_buffer.Address()) {
        GTEST_SKIP() << "Buffers address don't match";
    }
    // calling large_buffer->Address() twice should not result in this buffer being mapped twice.
    // If it is mapped twice, the error below will not be thrown.
    const VkDeviceAddress common_address = large_buffer.Address();

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper();
    dbbi.address = common_address;
    dbbi.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);

    constexpr uint32_t index = 0;

    // First call should succeed because offset is small enough to fit in large_buffer
    const VkDeviceSize offset = small_buffer_size;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                         &index, &offset);

    large_buffer.destroy();
    // Large buffer has been deleted, its entry in the address to buffers map must have been as well.
    // Since offset is too large to fit in small buffer, vkCmdSetDescriptorBufferOffsetsEXT should fail
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetDescriptorBufferOffsetsEXT-pOffsets-08063");
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                         &index, &offset);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDescriptorBuffer, InconsistentBuffer) {
    TEST_DESCRIPTION("Dispatch pipeline with descriptor set bound while descriptor buffer expected");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};

    VkDescriptorSetLayoutCreateInfo dslci = vku::InitStructHelper();
    dslci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    dslci.bindingCount = 1;
    dslci.pBindings = &binding;

    vkt::DescriptorSetLayout dsl(*m_device, dslci);

    VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &dsl.handle();

    vkt::PipelineLayout pipeline_layout(*m_device, plci);
    ASSERT_TRUE(pipeline_layout.initialized());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, vkt::device_address);

    VkDescriptorBufferBindingInfoEXT dbbi = vku::InitStructHelper();
    dbbi.address = buffer.Address();
    dbbi.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1, &dbbi);

    uint32_t index = 0;
    VkDeviceSize offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                                         &index, &offset);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08117");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDescriptorBuffer, InconsistentSet) {
    TEST_DESCRIPTION("Dispatch pipeline with descriptor buffer bound while of descriptor set expected");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    VkDescriptorSetLayoutCreateInfo dslci = vku::InitStructHelper();
    dslci.flags = 0;
    dslci.bindingCount = 1;
    dslci.pBindings = &binding;

    vkt::DescriptorSetLayout dsl(*m_device, dslci);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool pool(*m_device, ds_pool_ci);
    ASSERT_TRUE(pool.initialized());

    std::unique_ptr<vkt::DescriptorSet> ds(pool.AllocateSets(*m_device, dsl));
    ASSERT_TRUE(ds);

    VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &dsl.handle();

    vkt::PipelineLayout pipeline_layout(*m_device, plci);
    ASSERT_TRUE(pipeline_layout.initialized());

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    ASSERT_EQ(VK_SUCCESS, pipe.CreateComputePipeline());

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &ds->handle(), 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08115");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDescriptorBuffer, BindPoint) {
    TEST_DESCRIPTION("Descriptor buffer invalid bind point.");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::PipelineLayout pipeline_layout;
    {
        VkDescriptorSetLayoutBinding binding1 = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
        vkt::DescriptorSetLayout dsl1(*m_device, binding1, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

        vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
        VkDescriptorSetLayoutBinding binding2 = {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()};
        vkt::DescriptorSetLayout dsl2(*m_device, binding2,
                                      VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                          VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT);

        const VkDescriptorSetLayout set_layouts[2] = {dsl1.handle(), dsl2.handle()};
        VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
        plci.setLayoutCount = 2;
        plci.pSetLayouts = set_layouts;

        pipeline_layout.init(*m_device, plci);
    }

    {
        const std::optional<uint32_t> compute_qfi = m_device->ComputeOnlyQueueFamily();
        if (!compute_qfi) {
            GTEST_SKIP() << "No compute-only queue family, skipping bindpoint and queue tests.";
            return;
        }

        vkt::CommandPool command_pool(*m_device, compute_qfi.value());
        ASSERT_TRUE(command_pool.initialized());
        vkt::CommandBuffer command_buffer(*m_device, command_pool);

        command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-pipelineBindPoint-08069");
        vk::CmdBindDescriptorBufferEmbeddedSamplersEXT(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                       pipeline_layout.handle(), 1);
        m_errorMonitor->VerifyFound();
        command_buffer.End();
    }
}

TEST_F(NegativeDescriptorBuffer, DescriptorGetInfoBasic) {
    TEST_DESCRIPTION("Descriptor buffer vkDescriptorGetInfo().");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    uint8_t buffer[128];
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();
    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08018");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08018");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, DescriptorGetInfoValidPointer) {
    TEST_DESCRIPTION("If type is used, need valid pointer to corresponding data.");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    uint8_t buffer[16];
    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();

    dgi.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dgi.data.pCombinedImageSampler = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-pCombinedImageSampler-parameter");
    vk::GetDescriptorEXT(device(), &dgi, 4, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    dgi.data.pInputAttachmentImage = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-pInputAttachmentImage-parameter");
    vk::GetDescriptorEXT(device(), &dgi, 4, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    auto bad_struct = vku::InitStruct<VkBufferCopy2>();
    dgi.data.pUniformTexelBuffer = (VkDescriptorAddressInfoEXT*)&bad_struct;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-sType-sType");
    vk::GetDescriptorEXT(device(), &dgi, 4, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, DescriptorAddressInfoImplicit) {
    TEST_DESCRIPTION("Make sure VkDescriptorAddressInfoEXT are validated");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    auto bad_struct = vku::InitStruct<VkBufferCopy2>();
    VkDescriptorAddressInfoEXT dai = vku::InitStructHelper(&bad_struct);
    dai.address = 0;
    dai.range = 4;
    dai.format = VK_FORMAT_R8_UINT;

    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();
    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    dgi.data.pUniformTexelBuffer = &dai;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-pNext-pNext");
    uint8_t buffer[16];
    vk::GetDescriptorEXT(device(), &dgi, 4, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, DescriptorGetInfoSampler) {
    TEST_DESCRIPTION("Descriptor buffer vkDescriptorGetInfo() with a sampler backed VkDescriptorImageInfo.");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    uint8_t buffer[128];
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    const VkDescriptorImageInfo dii = {sampler.handle(), VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL};
    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();

    dgi.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dgi.data.pCombinedImageSampler = &dii;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08034");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.combinedImageSamplerDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    dgi.data.pSampledImage = &dii;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08035");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.sampledImageDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.data.pSampledImage = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08035");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.sampledImageDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    dgi.data.pStorageImage = &dii;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08036");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageImageDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.data.pStorageImage = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08036");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageImageDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    dgi.data.pUniformTexelBuffer = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08037");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformTexelBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    dgi.data.pStorageTexelBuffer = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08038");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dgi.data.pUniformBuffer = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08039");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dgi.data.pStorageBuffer = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08040");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, DescriptorGetInfoAS) {
    TEST_DESCRIPTION("Descriptor buffer vkDescriptorGetInfo() for Acceleration Structure.");
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    uint8_t buffer[128];
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();
    dgi.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    dgi.data.accelerationStructure = 0;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08041");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.accelerationStructureDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, DescriptorGetInfoAddressRange) {
    TEST_DESCRIPTION("Descriptor buffer vkDescriptorGetInfo() with VkDescriptorAddressInfoEXT.");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    uint8_t buffer[128];
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkt::Buffer d_buffer(*m_device, buffer_ci, vkt::no_mem);

    VkDescriptorAddressInfoEXT dai = vku::InitStructHelper();
    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();
    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dgi.data.pUniformBuffer = &dai;

    dai.address = 0;
    dai.range = 4;
    dai.format = VK_FORMAT_R8_UINT;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-address-08043");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), d_buffer.handle(), &mem_reqs);

    VkMemoryAllocateFlagsInfo memflagsinfo = vku::InitStructHelper();
    memflagsinfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    auto mem_alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, mem_reqs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    mem_alloc_info.pNext = &memflagsinfo;

    vkt::DeviceMemory mem(*m_device, mem_alloc_info);

    d_buffer.BindMemory(mem, 0);

    dai.address = d_buffer.Address();
    dai.range = 4096 * buffer_ci.size;
    dai.format = VK_FORMAT_R8_UINT;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-range-08045");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dai.range = 0;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-range-08940");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dai.range = VK_WHOLE_SIZE;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-range-08045");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-nullDescriptor-08939");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    {
        dai.range = 4;
        m_errorMonitor->SetDesiredError("VUID-vkGetDescriptorEXT-dataSize-08125");
        vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize - 1, &buffer);
        m_errorMonitor->VerifyFound();
    }

    mem.destroy();

    dai.range = 4;

    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dgi.data.pUniformBuffer = &dai;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08030");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dgi.data.pStorageBuffer = &dai;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08031");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    dgi.data.pUniformTexelBuffer = &dai;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08032");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformTexelBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    dgi.data.pStorageTexelBuffer = &dai;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorDataEXT-type-08033");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageTexelBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, LayoutFlags) {
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()};
    vkt::DescriptorSetLayout dsl(*m_device, binding);

    VkDeviceSize size;

    m_errorMonitor->SetDesiredError("VUID-vkGetDescriptorSetLayoutSizeEXT-layout-08012");
    vk::GetDescriptorSetLayoutSizeEXT(device(), dsl.handle(), &size);
    m_errorMonitor->VerifyFound();

    VkDeviceSize offset;

    m_errorMonitor->SetDesiredError("VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-layout-08014");
    vk::GetDescriptorSetLayoutBindingOffsetEXT(device(), dsl.handle(), 0, &offset);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, DescriptorBufferCaptureReplay) {
    AddRequiredFeature(vkt::Feature::descriptorBufferCaptureReplay);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    {
        VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
        buffer_ci.flags = VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        buffer_ci.size = 4096;
        buffer_ci.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        vkt::Buffer d_buffer(*m_device, buffer_ci, vkt::no_mem);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), d_buffer.handle(), &mem_reqs);

        auto mem_alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, mem_reqs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        // no alloc flags
        vkt::DeviceMemory mem(*m_device, mem_alloc_info);

        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-descriptorBufferCaptureReplay-08112");
        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-bufferDeviceAddressCaptureReplay-09200");
        m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-buffer-09201");
        vk::BindBufferMemory(device(), d_buffer.handle(), mem.handle(), 0);
        m_errorMonitor->VerifyFound();
    }

    {
        VkImageCreateInfo image_create_info = vku::InitStructHelper();
        image_create_info.flags = VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = 128;
        image_create_info.extent.height = 128;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.format = VK_FORMAT_D32_SFLOAT;
        image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        vkt::Image temp_image(*m_device, image_create_info, vkt::no_mem);

        VkMemoryRequirements mem_reqs;
        vk::GetImageMemoryRequirements(device(), temp_image.handle(), &mem_reqs);

        auto mem_alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // no allocate flags
        vkt::DeviceMemory mem(*m_device, mem_alloc_info);

        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-descriptorBufferCaptureReplay-08113");
        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-image-09202");
        vk::BindImageMemory(device(), temp_image.handle(), mem.handle(), 0);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorBuffer, DescriptorGetInfo) {
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    VkSampler invalid_sampler = CastToHandle<VkSampler, uintptr_t>(0xbaadbeef);
    VkImageView invalid_imageview = CastToHandle<VkImageView, uintptr_t>(0xbaadbeef);
    VkDeviceAddress invalid_buffer = CastToHandle<VkDeviceAddress, uintptr_t>(0xbaadbeef);

    uint8_t buffer[128];
    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();

    const VkDescriptorImageInfo dii = {invalid_sampler, invalid_imageview, VK_IMAGE_LAYOUT_GENERAL};

    dgi.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dgi.data.pCombinedImageSampler = &dii;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08019");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08020");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.combinedImageSamplerDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    dgi.data.pInputAttachmentImage = &dii;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08021");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.inputAttachmentDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    dgi.data.pSampledImage = &dii;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08022");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.sampledImageDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    dgi.data.pStorageImage = &dii;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08023");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageImageDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    VkDescriptorAddressInfoEXT dai = vku::InitStructHelper();
    dai.address = invalid_buffer;
    dai.range = 64;
    dai.format = VK_FORMAT_R8_UINT;

    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    dgi.data.pUniformTexelBuffer = &dai;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-None-08044");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08024");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformTexelBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    dgi.data.pStorageTexelBuffer = &dai;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-None-08044");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08025");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageTexelBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dgi.data.pUniformTexelBuffer = &dai;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-None-08044");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08026");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dgi.data.pStorageTexelBuffer = &dai;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-None-08044");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08027");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, SetBufferAddressSpaceLimits) {
    TEST_DESCRIPTION("Create VkBuffer with extension.");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);
    // After a few GB, can have memory issues running these tests
    // descriptorBufferAddressSpaceSize is always the largest of the 3 buffer address size limits
    constexpr VkDeviceSize max_limit = static_cast<VkDeviceSize>(1) << 31;
    if (descriptor_buffer_properties.descriptorBufferAddressSpaceSize > max_limit) {
        GTEST_SKIP() << "descriptorBufferAddressSpaceSize are too large";
    }

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = descriptor_buffer_properties.descriptorBufferAddressSpaceSize + 1;

    buffer_ci.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    CreateBufferTest(*this, &buffer_ci, "VUID-VkBufferCreateInfo-usage-08097");

    buffer_ci.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    CreateBufferTest(*this, &buffer_ci, "VUID-VkBufferCreateInfo-usage-08098");

    m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-usage-08097");
    buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                      VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    CreateBufferTest(*this, &buffer_ci, "VUID-VkBufferCreateInfo-usage-08098");
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5826
TEST_F(NegativeDescriptorBuffer, NullHandle) {
    TEST_DESCRIPTION("Descriptor buffer various tests.");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    const auto invalid_sampler = CastToHandle<VkSampler, uintptr_t>(0x0);
    const auto invalid_imageview = CastToHandle<VkImageView, uintptr_t>(0x0);

    std::array<std::byte, 128> buffer = {};
    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();

    const VkDescriptorImageInfo dii = {invalid_sampler, invalid_imageview, VK_IMAGE_LAYOUT_GENERAL};

    dgi.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    dgi.data.pInputAttachmentImage = &dii;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-08021");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.inputAttachmentDescriptorSize, buffer.data());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, NullCombinedImageSampler) {
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    uint8_t out;
    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();
    dgi.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dgi.data.pCombinedImageSampler = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-pCombinedImageSampler-parameter");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.combinedImageSamplerDescriptorSize / 2, &out);
    m_errorMonitor->VerifyFound();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    const VkDescriptorImageInfo dii = {sampler.handle(), VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL};
    dgi.data.pCombinedImageSampler = &dii;
    m_errorMonitor->SetDesiredError("VUID-vkGetDescriptorEXT-pDescriptorInfo-09507");
    m_errorMonitor->SetDesiredError("VUID-vkGetDescriptorEXT-dataSize-08125");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.combinedImageSamplerDescriptorSize / 2, &out);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, BufferUsage) {
    TEST_DESCRIPTION("Wrong Usage for buffer createion.");

    AddRequiredFeature(vkt::Feature::descriptorBufferPushDescriptors);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    if (descriptor_buffer_properties.bufferlessPushDescriptors) {
        GTEST_SKIP() << "bufferlessPushDescriptors is supported";
    }

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 64;
    buffer_create_info.usage = VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT;

    VkBuffer buffer;
    m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-usage-08103");
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, Binding) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance6);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    // TODO - try to get 08010 removed from spec as no possible way to create descriptor with the invalid flag
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-08009");
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-08010");
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->VerifyFound();

    VkBindDescriptorSetsInfo bind_ds_info = vku::InitStructHelper();
    bind_ds_info.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bind_ds_info.layout = pipeline_layout.handle();
    bind_ds_info.firstSet = 0;
    bind_ds_info.descriptorSetCount = 1;
    bind_ds_info.pDescriptorSets = &descriptor_set.set_;
    bind_ds_info.dynamicOffsetCount = 0;
    bind_ds_info.pDynamicOffsets = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkBindDescriptorSetsInfo-pDescriptorSets-08010");
    vk::CmdBindDescriptorSets2KHR(m_command_buffer.handle(), &bind_ds_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, InvalidDescriptorBufferUsage) {
    TEST_DESCRIPTION("Test vkCmdBindDescriptorBuffersEXT with invalid usage");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                      VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer d_buffer(*m_device, buffer_ci, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocate_flag_info);

    VkDescriptorBufferBindingInfoEXT binding_info = vku::InitStructHelper();
    binding_info.address = d_buffer.Address();
    binding_info.usage = buffer_ci.usage | 0x80000000;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingInfoEXT-None-09499");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1u, &binding_info);
    m_errorMonitor->VerifyFound();

    binding_info.usage = 0u;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorBufferBindingInfoEXT-None-09500");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 1u, &binding_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorBuffer, MaxTexelBufferElements) {
    TEST_DESCRIPTION("texel buffers must be less than maxTexelBufferElements.");
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), format, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Test requires support for VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT";
    }
    VkDeviceSize format_size = static_cast<VkDeviceSize>(vkuFormatTexelBlockSize(VK_FORMAT_R8G8B8A8_UNORM));

    VkDescriptorAddressInfoEXT dai = vku::InitStructHelper();
    dai.address = 0;
    dai.range = 2 * format_size * static_cast<VkDeviceSize>(m_device->Physical().limits_.maxTexelBufferElements);
    dai.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();
    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    dgi.data.pUniformTexelBuffer = &dai;

    uint8_t out;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-address-08043");  // null address
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorGetInfoEXT-type-09427");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformTexelBufferDescriptorSize, &out);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, TexelBufferFormat) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vkt::Buffer buffer(*m_device, 4096,
                       VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
                       vkt::device_address);

    VkDescriptorAddressInfoEXT dai = vku::InitStructHelper();
    dai.address = buffer.Address();
    dai.range = 4;
    dai.format = VK_FORMAT_UNDEFINED;

    VkDescriptorGetInfoEXT dgi = vku::InitStructHelper();
    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    dgi.data.pUniformTexelBuffer = &dai;

    uint8_t out;
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorAddressInfoEXT-None-09508");
    vk::GetDescriptorEXT(device(), &dgi, descriptor_buffer_properties.uniformTexelBufferDescriptorSize, &out);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, MaxResourceDescriptorBufferBindings) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorBuffer());

    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);
    if (descriptor_buffer_properties.maxResourceDescriptorBufferBindings != 1) {
        GTEST_SKIP() << "maxResourceDescriptorBufferBindings  is not 1";
    }

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    vkt::DescriptorSetLayout ds_layout(*m_device, binding, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT);

    VkDeviceSize ds_layout_size = 0;
    vk::GetDescriptorSetLayoutSizeEXT(device(), ds_layout.handle(), &ds_layout_size);

    vkt::Buffer descriptor_buffer(*m_device, ds_layout_size, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
                                  vkt::device_address);

    vkt::Buffer buffer_data(*m_device, 32, 0, vkt::device_address);
    VkDescriptorAddressInfoEXT addr_info = vku::InitStructHelper();
    addr_info.address = buffer_data.Address();
    addr_info.range = 32;
    addr_info.format = VK_FORMAT_UNDEFINED;

    VkDescriptorGetInfoEXT buffer_descriptor_info = vku::InitStructHelper();
    buffer_descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    buffer_descriptor_info.data.pStorageBuffer = &addr_info;

    void *mapped_descriptor_data = descriptor_buffer.Memory().Map();
    vk::GetDescriptorEXT(device(), &buffer_descriptor_info, descriptor_buffer_properties.storageBufferDescriptorSize,
                         mapped_descriptor_data);
    descriptor_buffer.Memory().Unmap();

    m_command_buffer.Begin();

    VkDescriptorBufferBindingInfoEXT descriptor_buffer_binding_infos[2];
    descriptor_buffer_binding_infos[0] = vku::InitStructHelper();
    descriptor_buffer_binding_infos[0].address = descriptor_buffer.Address();
    descriptor_buffer_binding_infos[0].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    descriptor_buffer_binding_infos[1] = vku::InitStructHelper();
    descriptor_buffer_binding_infos[1].address = descriptor_buffer.Address();
    descriptor_buffer_binding_infos[1].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    // VUID-vkCmdBindDescriptorBuffersEXT-maxResourceDescriptorBufferBindings-08049
    m_errorMonitor->SetDesiredError(
        "Addresses pointing to the same VkBuffer still count as multiple 'descriptor buffer bindings' towards the limits");
    vk::CmdBindDescriptorBuffersEXT(m_command_buffer.handle(), 2, descriptor_buffer_binding_infos);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}
