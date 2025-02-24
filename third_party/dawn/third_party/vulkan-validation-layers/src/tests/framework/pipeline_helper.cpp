/*
 * Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "pipeline_helper.h"

CreatePipelineHelper::CreatePipelineHelper(VkLayerTest &test, void *pNext) : layer_test_(test) {
    // default VkDevice, can be overwritten if multi-device tests
    device_ = layer_test_.DeviceObj();

    gp_ci_ = vku::InitStructHelper();
    gp_ci_.pNext = pNext;

    // InitDescriptorSetInfo
    dsl_bindings_.emplace_back(VkDescriptorSetLayoutBinding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr});

    // InitInputAndVertexInfo
    vi_ci_ = vku::InitStructHelper();

    ia_ci_ = vku::InitStructHelper();
    ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    // InitMultisampleInfo
    ms_ci_ = vku::InitStructHelper();
    ms_ci_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_ci_.sampleShadingEnable = VK_FALSE;
    ms_ci_.minSampleShading = 1.0;
    ms_ci_.pSampleMask = nullptr;

    // InitPipelineLayoutInfo
    pipeline_layout_ci_ = vku::InitStructHelper();
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created

    // InitViewportInfo
    viewport_ = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    scissor_ = {{0, 0}, {64, 64}};
    vp_state_ci_ = vku::InitStructHelper();
    vp_state_ci_.viewportCount = 1;
    vp_state_ci_.pViewports = &viewport_;
    vp_state_ci_.scissorCount = 1;
    vp_state_ci_.pScissors = &scissor_;

    // InitRasterizationInfo
    rs_state_ci_ = vku::InitStructHelper();
    rs_state_ci_.flags = 0;
    rs_state_ci_.depthClampEnable = VK_FALSE;
    rs_state_ci_.rasterizerDiscardEnable = VK_FALSE;
    rs_state_ci_.polygonMode = VK_POLYGON_MODE_FILL;
    rs_state_ci_.cullMode = VK_CULL_MODE_BACK_BIT;
    rs_state_ci_.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs_state_ci_.depthBiasEnable = VK_FALSE;
    rs_state_ci_.lineWidth = 1.0F;

    // InitLineRasterizationInfo
    line_state_ci_ = vku::InitStructHelper();
    line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT;
    line_state_ci_.stippledLineEnable = VK_FALSE;
    line_state_ci_.lineStippleFactor = 0;
    line_state_ci_.lineStipplePattern = 0;
    if (test.IsExtensionsEnabled(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME) ||
        test.IsExtensionsEnabled(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME)) {
        rs_state_ci_.pNext = &line_state_ci_;
    }

    // InitBlendStateInfo
    cb_ci_ = vku::InitStructHelper();
    cb_ci_.logicOpEnable = VK_FALSE;
    cb_ci_.logicOp = VK_LOGIC_OP_COPY;  // ignored if enable is VK_FALSE above
    cb_ci_.attachmentCount = 1;
    cb_ci_.pAttachments = &cb_attachments_;
    for (int i = 0; i < 4; i++) {
        cb_ci_.blendConstants[0] = 1.0F;
    }

    pc_ci_ = vku::InitStructHelper();
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;

    InitShaderInfo();

    // Color-only rendering in a subpass with no depth/stencil attachment
    // Active Pipeline Shader Stages
    //    Vertex Shader
    //    Fragment Shader
    // Required: Fixed-Function Pipeline Stages
    //    VkPipelineVertexInputStateCreateInfo
    //    VkPipelineInputAssemblyStateCreateInfo
    //    VkPipelineViewportStateCreateInfo
    //    VkPipelineRasterizationStateCreateInfo
    //    VkPipelineMultisampleStateCreateInfo
    //    VkPipelineColorBlendStateCreateInfo
    gp_ci_.layout = VK_NULL_HANDLE;
    gp_ci_.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
    gp_ci_.pVertexInputState = &vi_ci_;
    gp_ci_.pInputAssemblyState = &ia_ci_;
    gp_ci_.pTessellationState = nullptr;
    gp_ci_.pViewportState = &vp_state_ci_;
    gp_ci_.pMultisampleState = &ms_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;
    gp_ci_.pDepthStencilState = nullptr;
    gp_ci_.pColorBlendState = &cb_ci_;
    gp_ci_.pDynamicState = nullptr;
    gp_ci_.renderPass = layer_test_.RenderPass();
}

CreatePipelineHelper::~CreatePipelineHelper() { Destroy(); }

void CreatePipelineHelper::InitShaderInfo() { ResetShaderInfo(kVertexMinimalGlsl, kFragmentMinimalGlsl); }

void CreatePipelineHelper::ResetShaderInfo(const char *vertex_shader_text, const char *fragment_shader_text) {
    vs_ = std::make_unique<VkShaderObj>(&layer_test_, vertex_shader_text, VK_SHADER_STAGE_VERTEX_BIT);
    fs_ = std::make_unique<VkShaderObj>(&layer_test_, fragment_shader_text, VK_SHADER_STAGE_FRAGMENT_BIT);
    // We shouldn't need a fragment shader but add it to be able to run on more devices
    shader_stages_ = {vs_->GetStageCreateInfo(), fs_->GetStageCreateInfo()};
}

void CreatePipelineHelper::VertexShaderOnly() {
    shader_stages_.clear();
    shader_stages_.push_back(vs_->GetStageCreateInfo());
}

void CreatePipelineHelper::Destroy() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(device_->handle(), pipeline_cache_, nullptr);
        pipeline_cache_ = VK_NULL_HANDLE;
    }
    if (pipeline_ != VK_NULL_HANDLE) {
        vk::DestroyPipeline(device_->handle(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
}

// Designed for majority of cases that just need to simply add dynamic state
void CreatePipelineHelper::AddDynamicState(VkDynamicState dynamic_state) {
    dynamic_states_.push_back(dynamic_state);
    dyn_state_ci_ = vku::InitStructHelper();
    dyn_state_ci_.pDynamicStates = dynamic_states_.data();
    dyn_state_ci_.dynamicStateCount = dynamic_states_.size();
    // Set here and don't have have to worry about late bind setting it
    gp_ci_.pDynamicState = &dyn_state_ci_;
}

void CreatePipelineHelper::InitVertexInputLibInfo(void *p_next) {
    gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT;

    gp_ci_ = vku::InitStructHelper(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    gp_ci_.pVertexInputState = &vi_ci_;
    gp_ci_.pInputAssemblyState = &ia_ci_;

    gp_ci_.stageCount = 0;
    shader_stages_.clear();
}

void CreatePipelineHelper::InitPreRasterLibInfo(const VkPipelineShaderStageCreateInfo *info, void *p_next) {
    gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT;

    gp_ci_ = vku::InitStructHelper(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    gp_ci_.pViewportState = &vp_state_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.RenderPass();
    gp_ci_.subpass = 0;

    gp_ci_.stageCount = 1;  // default is just the Vertex shader
    gp_ci_.pStages = info;
}

void CreatePipelineHelper::InitFragmentLibInfo(const VkPipelineShaderStageCreateInfo *info, void *p_next) {
    gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;

    gp_ci_ = vku::InitStructHelper(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    //  gp_ci_.pTessellationState = nullptr; // TODO
    gp_ci_.pViewportState = &vp_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.RenderPass();
    gp_ci_.subpass = 0;

    // TODO if renderPass is null, MS info is not needed
    gp_ci_.pMultisampleState = &ms_ci_;

    gp_ci_.stageCount = 1;  // default is just the Fragment shader
    gp_ci_.pStages = info;
}

void CreatePipelineHelper::InitFragmentOutputLibInfo(void *p_next) {
    gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;

    gp_ci_ = vku::InitStructHelper(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;
    gp_ci_.pColorBlendState = &cb_ci_;
    gp_ci_.pMultisampleState = &ms_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.RenderPass();
    gp_ci_.subpass = 0;

    gp_ci_.stageCount = 0;
    shader_stages_.clear();
}

void CreatePipelineHelper::InitShaderLibInfo(std::vector<VkPipelineShaderStageCreateInfo> &info, void *p_next) {
    gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags =
        VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT | VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;

    gp_ci_ = vku::InitStructHelper(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    gp_ci_.pViewportState = &vp_state_ci_;
    gp_ci_.pViewportState = &vp_state_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.RenderPass();
    gp_ci_.subpass = 0;

    gp_ci_.pMultisampleState = &ms_ci_;

    gp_ci_.stageCount = info.size();
    gp_ci_.pStages = info.data();
}

void CreatePipelineHelper::InitPipelineCache() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(device_->handle(), pipeline_cache_, nullptr);
    }
    VkResult err = vk::CreatePipelineCache(device_->handle(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_EQ(VK_SUCCESS, err);
}

void CreatePipelineHelper::LateBindPipelineInfo() {
    // By value or dynamically located items must be late bound
    if (gp_ci_.layout == VK_NULL_HANDLE) {
        // Create a default descriptor and pipeline layout
        if (pipeline_layout_.handle() == VK_NULL_HANDLE) {
            if (!descriptor_set_) {
                // User can pass in own bindings
                descriptor_set_.reset(new OneOffDescriptorSet(device_, dsl_bindings_));
                ASSERT_TRUE(descriptor_set_->Initialized());
            }

            const std::vector<VkPushConstantRange> push_ranges(
                pipeline_layout_ci_.pPushConstantRanges,
                pipeline_layout_ci_.pPushConstantRanges + pipeline_layout_ci_.pushConstantRangeCount);
            pipeline_layout_ = vkt::PipelineLayout(*device_, {&descriptor_set_->layout_}, push_ranges, pipeline_layout_ci_.flags);
        }
        gp_ci_.layout = pipeline_layout_.handle();
    }
    if (gp_ci_.stageCount == 0) {
        gp_ci_.stageCount = shader_stages_.size();
        gp_ci_.pStages = shader_stages_.data();
    }
    if ((gp_ci_.pTessellationState == nullptr) && IsValidVkStruct(tess_ci_)) {
        gp_ci_.pTessellationState = &tess_ci_;
    }
    if ((gp_ci_.pDynamicState == nullptr) && IsValidVkStruct(dyn_state_ci_)) {
        gp_ci_.pDynamicState = &dyn_state_ci_;
    }
    if ((gp_ci_.pDepthStencilState == nullptr) && IsValidVkStruct(ds_ci_)) {
        gp_ci_.pDepthStencilState = &ds_ci_;
    }
}

VkResult CreatePipelineHelper::CreateGraphicsPipeline(bool do_late_bind, bool no_cache) {
    InitPipelineCache();
    if (do_late_bind) {
        LateBindPipelineInfo();
    }
    return vk::CreateGraphicsPipelines(device_->handle(), no_cache ? VK_NULL_HANDLE : pipeline_cache_, 1, &gp_ci_, NULL,
                                       &pipeline_);
}

CreateComputePipelineHelper::CreateComputePipelineHelper(VkLayerTest &test, void *pNext) : layer_test_(test) {
    // default VkDevice, can be overwritten if multi-device tests
    device_ = layer_test_.DeviceObj();

    cp_ci_ = vku::InitStructHelper();
    cp_ci_.pNext = pNext;

    // InitDescriptorSetInfo
    dsl_bindings_.emplace_back(VkDescriptorSetLayoutBinding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr});

    // InitPipelineLayoutInfo
    pipeline_layout_ci_ = vku::InitStructHelper();
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created

    // InitPipelineCacheInfo
    pc_ci_ = vku::InitStructHelper();
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;

    InitShaderInfo();

    cp_ci_.flags = 0;
    cp_ci_.layout = VK_NULL_HANDLE;
}

CreateComputePipelineHelper::~CreateComputePipelineHelper() { Destroy(); }

void CreateComputePipelineHelper::InitShaderInfo() {
    cs_ = std::make_unique<VkShaderObj>(&layer_test_, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);
    // We shouldn't need a fragment shader but add it to be able to run on more devices
}

void CreateComputePipelineHelper::InitPipelineCache() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(device_->handle(), pipeline_cache_, nullptr);
    }
    VkResult err = vk::CreatePipelineCache(device_->handle(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_EQ(VK_SUCCESS, err);
}

void CreateComputePipelineHelper::Destroy() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(device_->handle(), pipeline_cache_, nullptr);
        pipeline_cache_ = VK_NULL_HANDLE;
    }
    if (pipeline_ != VK_NULL_HANDLE) {
        vk::DestroyPipeline(device_->handle(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
}

void CreateComputePipelineHelper::LateBindPipelineInfo() {
    // By value or dynamically located items must be late bound
    if (cp_ci_.layout == VK_NULL_HANDLE) {
        // Create a default descriptor and pipeline layout
        if (pipeline_layout_.handle() == VK_NULL_HANDLE) {
            if (!descriptor_set_) {
                // User can pass in own bindings
                descriptor_set_.reset(new OneOffDescriptorSet(device_, dsl_bindings_));
                ASSERT_TRUE(descriptor_set_->Initialized());
            }

            const std::vector<VkPushConstantRange> push_ranges(
                pipeline_layout_ci_.pPushConstantRanges,
                pipeline_layout_ci_.pPushConstantRanges + pipeline_layout_ci_.pushConstantRangeCount);
            pipeline_layout_ = vkt::PipelineLayout(*device_, {&descriptor_set_->layout_}, push_ranges, pipeline_layout_ci_.flags);
        }

        cp_ci_.layout = pipeline_layout_.handle();
    }
    cp_ci_.stage = cs_.get()->GetStageCreateInfo();
}

VkResult CreateComputePipelineHelper::CreateComputePipeline(bool do_late_bind, bool no_cache) {
    InitPipelineCache();
    if (do_late_bind) {
        LateBindPipelineInfo();
    }
    return vk::CreateComputePipelines(device_->handle(), no_cache ? VK_NULL_HANDLE : pipeline_cache_, 1, &cp_ci_, NULL, &pipeline_);
}

namespace vkt {

GraphicsPipelineLibraryStage::GraphicsPipelineLibraryStage(vvl::span<const uint32_t> spv, VkShaderStageFlagBits stage) : spv(spv) {
    shader_ci = vku::InitStructHelper();
    shader_ci.codeSize = spv.size() * sizeof(uint32_t);
    shader_ci.pCode = spv.data();

    stage_ci = vku::InitStructHelper(&shader_ci);
    stage_ci.stage = stage;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";
}

SimpleGPL::SimpleGPL(VkLayerTest &test, VkPipelineLayout layout, const char *vertex_shader, const char *fragment_shader)
    : vertex_input_lib_(test), pre_raster_lib_(test), frag_shader_lib_(test), frag_out_lib_(test) {
    auto device = test.DeviceObj();

    vertex_input_lib_.InitVertexInputLibInfo();
    vertex_input_lib_.CreateGraphicsPipeline(false);

    // For GPU-AV tests this shrinks things so only a single fragment is executed
    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};

    const auto vs_spv = test.GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader ? vertex_shader : kVertexMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);
    pre_raster_lib_.InitPreRasterLibInfo(&vs_stage.stage_ci);
    pre_raster_lib_.vp_state_ci_.pViewports = &viewport;
    pre_raster_lib_.vp_state_ci_.pScissors = &scissor;
    pre_raster_lib_.gp_ci_.layout = layout;
    pre_raster_lib_.CreateGraphicsPipeline();

    const auto fs_spv = test.GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader ? fragment_shader : kFragmentMinimalGlsl);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);
    frag_shader_lib_.InitFragmentLibInfo(&fs_stage.stage_ci);
    frag_shader_lib_.gp_ci_.layout = layout;
    frag_shader_lib_.CreateGraphicsPipeline(false);

    frag_out_lib_.InitFragmentOutputLibInfo();
    frag_out_lib_.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib_.Handle(),
        pre_raster_lib_.Handle(),
        frag_shader_lib_.Handle(),
        frag_out_lib_.Handle(),
    };

    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = layout;
    pipe_.init(*device, exe_pipe_ci);
}

}  // namespace vkt
