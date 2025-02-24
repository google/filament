/*
 * Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

class NegativePortabilitySubset : public VkLayerTest {};

TEST_F(NegativePortabilitySubset, Device) {
    TEST_DESCRIPTION("Portability: CreateDevice called and VK_KHR_portability_subset not enabled");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    vkt::PhysicalDevice phys_device(Gpu());

    // request all queues
    const std::vector<VkQueueFamilyProperties> queue_props = phys_device.queue_properties_;
    vkt::QueueCreateInfoArray queue_info(phys_device.queue_properties_);

    // Only request creation with queuefamilies that have at least one queue
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.Data();
    for (uint32_t j = 0; j < queue_info.Size(); ++j) {
        if (qci[j].queueCount) {
            create_queue_infos.push_back(qci[j]);
        }
    }

    VkDeviceCreateInfo dev_info = vku::InitStructHelper();
    dev_info.queueCreateInfoCount = create_queue_infos.size();
    dev_info.pQueueCreateInfos = create_queue_infos.data();
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = 0;
    dev_info.ppEnabledExtensionNames =
        nullptr;  // VK_KHR_portability_subset not included in enabled extensions should trigger 04451

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pProperties-04451");
    VkDevice device;
    vk::CreateDevice(Gpu(), &dev_info, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePortabilitySubset, Event) {
    TEST_DESCRIPTION("Portability: CreateEvent when not supported");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_feature);
    portability_feature.events = VK_FALSE;  // Make sure events are disabled

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    m_errorMonitor->SetDesiredError("VUID-vkCreateEvent-events-04468");
    VkEventCreateInfo eci = vku::InitStructHelper();
    VkEvent event;
    vk::CreateEvent(device(), &eci, nullptr, &event);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePortabilitySubset, Image) {
    TEST_DESCRIPTION("Portability: CreateImage - VUIDs 04459, 04460");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_feature);
    // Make sure image features are disabled via portability extension
    portability_feature.imageView2DOn3DImage = VK_FALSE;
    portability_feature.multisampleArrayImage = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent.width = 512;
    ci.extent.height = 64;
    ci.extent.depth = 1;
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-imageView2DOn3DImage-04459");

    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.flags = 0;
    ci.samples = VK_SAMPLE_COUNT_2_BIT;
    ci.arrayLayers = 2;
    CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-multisampleArrayImage-04460");
}

TEST_F(NegativePortabilitySubset, ImageViewFormatSwizzle) {
    TEST_DESCRIPTION("Portability: If imageViewFormatSwizzle is not enabled");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(portability_feature);
    portability_feature.imageViewFormatSwizzle = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &portability_feature));

    VkImageCreateInfo imageCI = vku::InitStructHelper();
    imageCI.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCI.extent.width = 512;
    imageCI.extent.height = 64;
    imageCI.extent.depth = 1;
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCI.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    vkt::Image image(*m_device, imageCI, vkt::set_layout);

    VkImageViewCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.image = image.handle();
    ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    // Incorrect swizzling due to portability
    ci.components.r = VK_COMPONENT_SWIZZLE_G;
    ci.components.g = VK_COMPONENT_SWIZZLE_G;
    ci.components.b = VK_COMPONENT_SWIZZLE_R;
    ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    CreateImageViewTest(*this, &ci, "VUID-VkImageViewCreateInfo-imageViewFormatSwizzle-04465");

    // Verify using VK_COMPONENT_SWIZZLE_R/G/B/A works when imageViewFormatSwizzle == VK_FALSE
    ci.components.r = VK_COMPONENT_SWIZZLE_R;
    ci.components.g = VK_COMPONENT_SWIZZLE_G;
    ci.components.b = VK_COMPONENT_SWIZZLE_B;
    ci.components.a = VK_COMPONENT_SWIZZLE_A;
    CreateImageViewTest(*this, &ci);
}

TEST_F(NegativePortabilitySubset, ImageViewFormatReinterpretationComponentCount) {
    TEST_DESCRIPTION("Portability: If ImageViewFormatReinterpretation is not enabled");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    portability_feature.imageViewFormatReinterpretation = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &portability_feature));

    VkImageCreateInfo imageCI = vku::InitStructHelper();
    imageCI.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCI.extent.width = 512;
    imageCI.extent.height = 64;
    imageCI.extent.depth = 1;
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCI.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    vkt::Image image(*m_device, imageCI, vkt::set_layout);

    VkImageViewCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.image = image.handle();
    ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ci.format = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    // Incorrect swizzling due to portability
    ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    // Format might not be supported
    // TODO - Need to figure out which format is supported that hits 04466
    m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-None-02273");
    CreateImageViewTest(*this, &ci, "VUID-VkImageViewCreateInfo-imageViewFormatReinterpretation-04466");
}

TEST_F(NegativePortabilitySubset, Sampler) {
    TEST_DESCRIPTION("Portability: CreateSampler - VUID 04467");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_feature);
    // Make sure image features are disabled via portability extension
    portability_feature.samplerMipLodBias = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.mipLodBias = 1.0f;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-samplerMipLodBias-04467");
}

TEST_F(NegativePortabilitySubset, TriangleFans) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUID 04452");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_feature);
    // Make sure image features are disabled via portability extension
    portability_feature.triangleFans = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(Gpu());
    m_depthStencil->Init(*m_device, m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view.handle());

    CreatePipelineHelper pipe(*this);
    pipe.ia_ci_ = VkPipelineInputAssemblyStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_FALSE};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.VertexShaderOnly();

    m_errorMonitor->SetDesiredError("VUID-VkPipelineInputAssemblyStateCreateInfo-triangleFans-04452");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePortabilitySubset, VertexInputStride) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUID 04456");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_feature);
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    // Get the current vertex stride to ensure we pass an incorrect value when creating the graphics pipeline
    VkPhysicalDevicePortabilitySubsetPropertiesKHR portability_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(portability_properties);

    ASSERT_TRUE(portability_properties.minVertexInputBindingStrideAlignment > 0);
    auto vertex_stride = portability_properties.minVertexInputBindingStrideAlignment - 1;

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(Gpu());
    m_depthStencil->Init(*m_device, m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view.handle());

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription vertex_desc{
        0,                            // binding
        vertex_stride,                // stride
        VK_VERTEX_INPUT_RATE_VERTEX,  // inputRate
    };
    pipe.vi_ci_ = VkPipelineVertexInputStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &vertex_desc, 0, nullptr};
    pipe.ia_ci_ = VkPipelineInputAssemblyStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.VertexShaderOnly();

    m_errorMonitor->SetDesiredError("VUID-VkVertexInputBindingDescription-stride-04456");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePortabilitySubset, VertexAttributes) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUID 04457");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_feature);
    // Make sure image features are disabled via portability extension
    portability_feature.vertexAttributeAccessBeyondStride = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(Gpu());
    m_depthStencil->Init(*m_device, m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view.handle());

    CreatePipelineHelper pipe(*this);
    VkVertexInputBindingDescription vertex_desc{
        0,                            // binding
        4,                            // stride
        VK_VERTEX_INPUT_RATE_VERTEX,  // inputRate
    };
    VkVertexInputAttributeDescription vertex_attrib{
        0,                     // location
        0,                     // binding
        VK_FORMAT_R32_SFLOAT,  // format; size == 4
        4,                     // offset; size(format) + offset > description.stride, so this should trigger 04457
    };
    pipe.vi_ci_ = VkPipelineVertexInputStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &vertex_desc, 1, &vertex_attrib};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.VertexShaderOnly();

    m_errorMonitor->SetDesiredError("VUID-VkVertexInputAttributeDescription-vertexAttributeAccessBeyondStride-04457");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePortabilitySubset, RasterizationState) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUID 04458");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_feature);
    // Make sure point polygons are disabled
    portability_feature.pointPolygons = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.format = VK_FORMAT_B8G8R8A8_SRGB;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference color_ref{};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;

    vk::CreateRenderPass(device(), &rp_info, nullptr, &m_renderPass);

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.polygonMode = VK_POLYGON_MODE_POINT;

    m_errorMonitor->SetDesiredError("VUID-VkPipelineRasterizationStateCreateInfo-pointPolygons-04458");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativePortabilitySubset, DepthStencilState) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUID 04453");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_feature);
    portability_feature.separateStencilMaskRef = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(Gpu());
    m_depthStencil->Init(*m_device, m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthStencil->SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView depth_image_view = m_depthStencil->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view.handle());

    VkPipelineDepthStencilStateCreateInfo depth_stencil_ci = vku::InitStructHelper();
    depth_stencil_ci.stencilTestEnable = VK_TRUE;
    depth_stencil_ci.front.reference = 1;
    depth_stencil_ci.back.reference = depth_stencil_ci.front.reference + 1;
    //  front.reference != back.reference should trigger 04453

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pDepthStencilState = &depth_stencil_ci;
    pipe.rs_state_ci_.cullMode = VK_CULL_MODE_NONE;
    pipe.VertexShaderOnly();

    m_errorMonitor->SetDesiredError("VUID-VkPipelineDepthStencilStateCreateInfo-separateStencilMaskRef-04453");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Ensure using without depth-stencil works
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    // pDepthStencilState should be ignored if rasterization is disabled or if the referenced subpass does not use a depth/stencil
    // attachment
    pipe.gp_ci_.pDepthStencilState = nullptr;
    pipe.CreateGraphicsPipeline();
}

TEST_F(NegativePortabilitySubset, ColorBlendAttachmentState) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUIDs 04454, 04455");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_feature);
    portability_feature.constantAlphaColorBlendFactors = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.cb_attachments_.srcColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;

    m_errorMonitor->SetDesiredError("VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04454");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.cb_attachments_.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    m_errorMonitor->SetDesiredError("VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04454");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.cb_attachments_.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    pipe.cb_attachments_.dstColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
    m_errorMonitor->SetDesiredError("VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04455");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.cb_attachments_.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    m_errorMonitor->SetDesiredError("VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04455");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

class VkPortabilitySubsetTest : public VkLayerTest {};
TEST_F(VkPortabilitySubsetTest, UpdateDescriptorSets) {
    TEST_DESCRIPTION("Portability: UpdateDescriptorSets - VUID 04450");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_feature);
    // Make sure image features are disabled via portability extension
    portability_feature.mutableComparisonSamplers = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.compareEnable = VK_TRUE;  // Incompatible with portability setting
    vkt::Sampler sampler(*m_device, sampler_info);

    constexpr VkFormat img_format = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image(*m_device, 32, 32, 1, img_format, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.Layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    auto image_view_create_info = image.BasicViewCreatInfo();
    vkt::ImageView view(*m_device, image_view_create_info);

    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler.handle();
    img_info.imageView = view.handle();
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[0].pImageInfo = &img_info;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorImageInfo-mutableComparisonSamplers-04450");
    vk::UpdateDescriptorSets(device(), 1, descriptor_writes, 0, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkPortabilitySubsetTest, ShaderValidation) {
    TEST_DESCRIPTION("Attempt to use shader features that are not supported via portability");
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(portability_feature);
    portability_feature.tessellationIsolines = VK_FALSE;                    // Make sure IsoLines are disabled
    portability_feature.tessellationPointMode = VK_FALSE;                   // Make sure PointMode is disabled
    portability_feature.shaderSampleRateInterpolationFunctions = VK_FALSE;  // Make sure interpolation functions are disabled

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    VkShaderObj tsc_obj(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};
    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.ia_ci_ = iasci;
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.tess_ci_ = tsci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tsc_obj.GetStageCreateInfo()};

    // Attempt to use isolines in the TES shader when not available
    {
        static const char *tes_source = R"glsl(
            #version 450
            layout(isolines, equal_spacing, cw) in;
            void main() {
                gl_Position = vec4(1);
            }
        )glsl";
        VkShaderObj tes_obj(this, tes_source, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
        pipe.shader_stages_.emplace_back(tes_obj.GetStageCreateInfo());
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-tessellationShader-06326");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    // Attempt to use point_mode in the TES shader when not available
    {
        static const char *tes_source = R"glsl(
            #version 450
            layout(triangles, point_mode) in;
            void main() {
                gl_PointSize = 1.0;
                gl_Position = vec4(1);
            }
        )glsl";

        // Reset TES shader stage
        pipe.InitShaderInfo();
        VkShaderObj tes_obj(this, tes_source, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), tsc_obj.GetStageCreateInfo(), tes_obj.GetStageCreateInfo()};

        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-tessellationShader-06327");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    // Attempt to use interpolation functions when not supported
    {
        static const char *vs_source = R"glsl(
            #version 450
            layout(location = 0) out vec4 c;
            void main() {
                c = vec4(1);
                gl_Position = vec4(1);
            }
        )glsl";
        VkShaderObj vs_obj(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

        static const char *fs_source = R"glsl(
            #version 450
            layout(location = 0) in vec4 c;
            layout(location = 0) out vec4 frag_out;
            void main() {
                frag_out = interpolateAtCentroid(c);
            }
        )glsl";
        VkShaderObj fs_obj(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper raster_pipe(*this);
        iasci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        raster_pipe.ia_ci_ = iasci;
        raster_pipe.ia_ci_ = iasci;
        raster_pipe.tess_ci_ = tsci;
        raster_pipe.shader_stages_ = {vs_obj.GetStageCreateInfo(), fs_obj.GetStageCreateInfo()};
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-shaderSampleRateInterpolationFunctions-06325");
        raster_pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkPortabilitySubsetTest, PortabilitySubsetColorBlendFactor) {
    TEST_DESCRIPTION("Test invalid color blend factor with portability subset");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::extendedDynamicState3ColorBlendEquation);
    RETURN_IF_SKIP(Init());

    VkColorBlendEquationEXT color_blend_equation = {
        VK_BLEND_FACTOR_CONSTANT_ALPHA, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,           VK_BLEND_OP_ADD,
    };

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkColorBlendEquationEXT-constantAlphaColorBlendFactors-07362");
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 0u, 1u, &color_blend_equation);
    m_errorMonitor->VerifyFound();

    color_blend_equation.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_equation.dstColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
    m_errorMonitor->SetDesiredError("VUID-VkColorBlendEquationEXT-constantAlphaColorBlendFactors-07363");
    vk::CmdSetColorBlendEquationEXT(m_command_buffer.handle(), 0u, 1u, &color_blend_equation);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(VkPortabilitySubsetTest, InstanceCreateEnumerate) {
    TEST_DESCRIPTION("Validate creating instances with VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR.");
    std::vector<const char *> enabled_extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                                    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME};

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    GTEST_SKIP() << "Android doesn't support Debug Utils";
#endif

    auto ici = GetInstanceCreateInfo();
    ici.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    ici.enabledExtensionCount = 1;
    ici.ppEnabledExtensionNames = enabled_extensions.data();

    VkInstance local_instance;

    m_errorMonitor->SetDesiredError("VUID-VkInstanceCreateInfo-flags-06559");
    vk::CreateInstance(&ici, nullptr, &local_instance);
    m_errorMonitor->VerifyFound();

    if (InstanceExtensionSupported(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        ici.enabledExtensionCount = 2;
        ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &local_instance));
        vk::DestroyInstance(local_instance, nullptr);
    }
}

TEST_F(VkPortabilitySubsetTest, FeatureWithoutExtension) {
    TEST_DESCRIPTION("Make sure can't use portability without VK_KHR_portability_subset");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    if (DeviceExtensionSupported(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GTEST_SKIP() << "Test needs no VK_KHR_portability_subset support";
    }

    VkPhysicalDevicePortabilitySubsetFeaturesKHR feature = vku::InitStructHelper();

    vkt::PhysicalDevice physical_device(Gpu());
    vkt::QueueCreateInfoArray queue_info(physical_device.queue_properties_);
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.Data();
    for (uint32_t i = 0; i < queue_info.Size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&feature);
    device_create_info.queueCreateInfoCount = queue_info.Size();
    device_create_info.pQueueCreateInfos = queue_info.Data();
    VkDevice testDevice;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
    vk::CreateDevice(Gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}
