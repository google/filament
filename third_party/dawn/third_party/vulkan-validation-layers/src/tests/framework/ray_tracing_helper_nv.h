/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
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

namespace nv {
namespace rt {

// DEPRECATED - use vkt::rt::Pipeline instead
// ------------------------------------------
// Helper class for to create ray tracing pipeline tests
// Designed with minimal error checking to ensure easy error state creation
// See OneshotTest for typical usage
class RayTracingPipelineHelper {
  public:
    std::vector<VkDescriptorSetLayoutBinding> dsl_bindings_;
    std::unique_ptr<OneOffDescriptorSet> descriptor_set_;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
    VkPipelineLayoutCreateInfo pipeline_layout_ci_ = {};
    vkt::PipelineLayout pipeline_layout_;
    VkRayTracingPipelineCreateInfoNV rp_ci_ = {};
    VkRayTracingPipelineCreateInfoKHR rp_ci_KHR_ = {};
    VkPipelineCacheCreateInfo pc_ci_ = {};
    std::optional<VkRayTracingPipelineInterfaceCreateInfoKHR> rp_i_ci_;
    VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
    std::vector<VkRayTracingShaderGroupCreateInfoNV> groups_;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups_KHR_;
    std::unique_ptr<VkShaderObj> rgs_;
    std::unique_ptr<VkShaderObj> chs_;
    std::unique_ptr<VkShaderObj> mis_;
    std::vector<VkPipeline> libraries_;
    VkPipelineLibraryCreateInfoKHR rp_library_ci_;
    VkLayerTest& layer_test_;
    RayTracingPipelineHelper(VkLayerTest& test);
    ~RayTracingPipelineHelper();

    const VkPipeline& Handle() const { return pipeline_; }
    void InitShaderGroups();
    void InitDescriptorSetInfo();
    void InitDescriptorSetInfoKHR();
    void InitPipelineLayoutInfo();
    void InitShaderInfo();
    void InitNVRayTracingPipelineInfo();
    void AddLibrary(const RayTracingPipelineHelper& library);
    void InitPipelineCacheInfo();
    void InitInfo();
    void InitPipelineCache();
    void LateBindPipelineInfo(bool isKHR = false);
    VkResult CreateNVRayTracingPipeline(bool do_late_bind = true);
    // Helper function to create a simple test case (positive or negative)
    //
    // info_override can be any callable that takes a CreateNVRayTracingPipelineHelper &
    // flags, error can be any args accepted by "SetDesiredFailure".
    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test& test, const OverrideFunc& info_override, const std::vector<Error>& errors,
                            const VkFlags flags = kErrorBit) {
        RayTracingPipelineHelper helper(test);
        info_override(helper);

        for (const auto& error : errors) test.Monitor().SetDesiredFailureMsg(flags, error);
        helper.CreateNVRayTracingPipeline();
        test.Monitor().VerifyFound();
    }

    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test& test, const OverrideFunc& info_override, Error error, const VkFlags flags = kErrorBit) {
        OneshotTest(test, info_override, std::vector<Error>(1, error), flags);
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotPositiveTest(Test& test, const OverrideFunc& info_override, const VkFlags message_flag_mask = kErrorBit) {
        RayTracingPipelineHelper helper(test);
        info_override(helper);

        ASSERT_EQ(VK_SUCCESS, helper.CreateNVRayTracingPipeline());
    }

  private:
    VkPipeline pipeline_ = VK_NULL_HANDLE;
};

// DEPRECATED: This is part of the legacy ray tracing framework, now only used in the old nvidia ray tracing extension tests.
void GetSimpleGeometryForAccelerationStructureTests(const vkt::Device& device, vkt::Buffer* vbo, vkt::Buffer* ibo,
                                                    VkGeometryNV* geometry, VkDeviceSize offset = 0,
                                                    bool buffer_device_address = false);
}  // namespace rt
}  // namespace nv
