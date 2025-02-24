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

#pragma once

#include "layer_validation_tests.h"
#include "descriptor_helper.h"
#include "shader_helper.h"

// Helper class for tersely creating create pipeline tests
//
// Designed with minimal error checking to ensure easy error state creation
// See OneshotTest for typical usage
class CreatePipelineHelper {
  public:
    std::vector<VkDescriptorSetLayoutBinding> dsl_bindings_;
    std::unique_ptr<OneOffDescriptorSet> descriptor_set_;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
    VkPipelineVertexInputStateCreateInfo vi_ci_ = {};
    VkPipelineInputAssemblyStateCreateInfo ia_ci_ = {};
    VkPipelineTessellationStateCreateInfo tess_ci_ = {};
    VkPipelineViewportStateCreateInfo vp_state_ci_ = {};
    VkPipelineMultisampleStateCreateInfo ms_ci_ = {};
    VkPipelineLayoutCreateInfo pipeline_layout_ci_ = {};
    vkt::PipelineLayout pipeline_layout_;
    VkPipelineDynamicStateCreateInfo dyn_state_ci_ = {};
    VkPipelineRasterizationStateCreateInfo rs_state_ci_ = {};
    VkPipelineRasterizationLineStateCreateInfo line_state_ci_ = {};
    VkPipelineColorBlendAttachmentState cb_attachments_ = {};
    VkPipelineColorBlendStateCreateInfo cb_ci_ = {};
    VkPipelineDepthStencilStateCreateInfo ds_ci_ = {};
    VkGraphicsPipelineCreateInfo gp_ci_ = {};
    VkPipelineCacheCreateInfo pc_ci_ = {};
    VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
    std::unique_ptr<VkShaderObj> vs_;
    std::unique_ptr<VkShaderObj> fs_;
    VkLayerTest &layer_test_;
    vkt::Device *device_;
    std::optional<VkGraphicsPipelineLibraryCreateInfoEXT> gpl_info;
    // advantage of taking a VkLayerTest over vkt::Device is we can get the default renderpass from InitRenderTarget
    CreatePipelineHelper(VkLayerTest &test, void *pNext = nullptr);
    ~CreatePipelineHelper();

    const VkPipeline &Handle() const { return pipeline_; }
    void InitShaderInfo();
    void ResetShaderInfo(const char *vertex_shader_text, const char *fragment_shader_text);
    void VertexShaderOnly();
    void Destroy();

    void LateBindPipelineInfo();
    VkResult CreateGraphicsPipeline(bool do_late_bind = true, bool no_cache = false);

    void InitVertexInputLibInfo(void *p_next = nullptr);

    template <typename StageContainer>
    void InitPreRasterLibInfoFromContainer(const StageContainer &stages, void *p_next = nullptr) {
        InitPreRasterLibInfo(stages.data(), p_next);
        gp_ci_.stageCount = static_cast<uint32_t>(stages.size());
    }
    void InitPreRasterLibInfo(const VkPipelineShaderStageCreateInfo *info, void *p_next = nullptr);

    template <typename StageContainer>
    void InitFragmentLibInfoFromContainer(const StageContainer &stages, void *p_next = nullptr) {
        InitFragmentLibInfo(stages.data(), p_next);
        gp_ci_.stageCount = static_cast<uint32_t>(stages.size());
    }
    void InitFragmentLibInfo(const VkPipelineShaderStageCreateInfo *info, void *p_next = nullptr);

    void InitFragmentOutputLibInfo(void *p_next = nullptr);

    // Both Pre-Rasterization and Fragment Shader
    void InitShaderLibInfo(std::vector<VkPipelineShaderStageCreateInfo> &info, void *p_next = nullptr);

    // Helper function to create a simple test case (positive or negative)
    //
    // info_override can be any callable that takes a CreatePipelineHeper &
    // flags, error can be any args accepted by "SetDesiredFailure".
    template <typename Test, typename OverrideFunc, typename ErrorContainer>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, const ErrorContainer &errors) {
        CreatePipelineHelper helper(test);
        info_override(helper);

        for (const auto &error : errors) test.Monitor().SetDesiredFailureMsg(flags, error);
        helper.CreateGraphicsPipeline();

        if (!errors.empty()) {
            test.Monitor().VerifyFound();
        }
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, const char *error) {
        std::array errors = {error};
        OneshotTest(test, info_override, flags, errors);
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, const std::string &error) {
        std::array errors = {error};
        OneshotTest(test, info_override, flags, errors);
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags) {
        std::array<const char *, 0> errors;
        OneshotTest(test, info_override, flags, errors);
    }

    void AddDynamicState(VkDynamicState dynamic_state);

  private:
    void InitPipelineCache();
    VkPipeline pipeline_ = VK_NULL_HANDLE;
    // Hold some state for making certain pipeline creations easier
    std::vector<VkDynamicState> dynamic_states_;

    VkViewport viewport_ = {};
    VkRect2D scissor_ = {};
};

class CreateComputePipelineHelper {
  public:
    std::vector<VkDescriptorSetLayoutBinding> dsl_bindings_;
    std::unique_ptr<OneOffDescriptorSet> descriptor_set_;
    VkPipelineLayoutCreateInfo pipeline_layout_ci_ = {};
    vkt::PipelineLayout pipeline_layout_;
    VkComputePipelineCreateInfo cp_ci_ = {};
    VkPipelineCacheCreateInfo pc_ci_ = {};
    VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
    std::unique_ptr<VkShaderObj> cs_;
    bool override_skip_ = false;
    VkLayerTest &layer_test_;
    vkt::Device *device_;
    CreateComputePipelineHelper(VkLayerTest &test, void *pNext = nullptr);
    ~CreateComputePipelineHelper();

    const VkPipeline &Handle() const { return pipeline_; }
    void InitShaderInfo();
    void Destroy();

    void LateBindPipelineInfo();
    VkResult CreateComputePipeline(bool do_late_bind = true, bool no_cache = false);

    // Helper function to create a simple test case (positive or negative)
    //
    // info_override can be any callable that takes a CreatePipelineHeper &
    // flags, error can be any args accepted by "SetDesiredFailure".
    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, const std::vector<Error> &errors,
                            bool positive_test = false) {
        CreateComputePipelineHelper helper(test);
        info_override(helper);
        // Allow lambda to decide if to skip trying to compile pipeline to prevent crashing
        if (helper.override_skip_) {
            helper.override_skip_ = false;  // reset
            return;
        }

        for (const auto &error : errors) test.Monitor().SetDesiredFailureMsg(flags, error);
        helper.CreateComputePipeline();

        if (!errors.empty()) {
            test.Monitor().VerifyFound();
        }
    }

    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, Error error) {
        OneshotTest(test, info_override, flags, std::vector<Error>(1, error));
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags) {
        OneshotTest(test, info_override, flags, std::vector<std::string>{});
    }

  private:
    void InitPipelineCache();
    VkPipeline pipeline_ = VK_NULL_HANDLE;
};

namespace vkt {

struct GraphicsPipelineLibraryStage {
    vvl::span<const uint32_t> spv;
    VkShaderModuleCreateInfo shader_ci;
    VkPipelineShaderStageCreateInfo stage_ci;

    GraphicsPipelineLibraryStage(vvl::span<const uint32_t> spv, VkShaderStageFlagBits stage);
};

// Used when need a Graphics Pipeline Library with the most basic components
// For GPU-AV tests, this will only run a single fragment pixel
class SimpleGPL {
  public:
    SimpleGPL(VkLayerTest &test, VkPipelineLayout layout, const char *vertex_shader = nullptr,
              const char *fragment_shader = nullptr);

    const VkPipeline &Handle() const { return pipe_.handle(); }

  private:
    CreatePipelineHelper vertex_input_lib_;
    CreatePipelineHelper pre_raster_lib_;
    CreatePipelineHelper frag_shader_lib_;
    CreatePipelineHelper frag_out_lib_;
    vkt::Pipeline pipe_;
};

}  // namespace vkt

static inline VkPipelineColorBlendAttachmentState DefaultColorBlendAttachmentState() {
    VkPipelineColorBlendAttachmentState state = {};
    state.blendEnable = VK_TRUE;
    state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    state.colorBlendOp = VK_BLEND_OP_ADD;
    state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    state.alphaBlendOp = VK_BLEND_OP_ADD;
    return state;
}