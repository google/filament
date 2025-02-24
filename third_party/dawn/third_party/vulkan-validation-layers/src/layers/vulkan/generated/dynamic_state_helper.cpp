// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dynamic_state_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/

// NOLINTBEGIN

#include "state_tracker/pipeline_state.h"

VkDynamicState ConvertToDynamicState(CBDynamicState dynamic_state) {
    switch (dynamic_state) {
        case CB_DYNAMIC_STATE_VIEWPORT:
            return VK_DYNAMIC_STATE_VIEWPORT;
        case CB_DYNAMIC_STATE_SCISSOR:
            return VK_DYNAMIC_STATE_SCISSOR;
        case CB_DYNAMIC_STATE_LINE_WIDTH:
            return VK_DYNAMIC_STATE_LINE_WIDTH;
        case CB_DYNAMIC_STATE_DEPTH_BIAS:
            return VK_DYNAMIC_STATE_DEPTH_BIAS;
        case CB_DYNAMIC_STATE_BLEND_CONSTANTS:
            return VK_DYNAMIC_STATE_BLEND_CONSTANTS;
        case CB_DYNAMIC_STATE_DEPTH_BOUNDS:
            return VK_DYNAMIC_STATE_DEPTH_BOUNDS;
        case CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
            return VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
        case CB_DYNAMIC_STATE_STENCIL_WRITE_MASK:
            return VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
        case CB_DYNAMIC_STATE_STENCIL_REFERENCE:
            return VK_DYNAMIC_STATE_STENCIL_REFERENCE;
        case CB_DYNAMIC_STATE_CULL_MODE:
            return VK_DYNAMIC_STATE_CULL_MODE;
        case CB_DYNAMIC_STATE_FRONT_FACE:
            return VK_DYNAMIC_STATE_FRONT_FACE;
        case CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
            return VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY;
        case CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
            return VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT;
        case CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
            return VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT;
        case CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
            return VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE;
        case CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
            return VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE;
        case CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
            return VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE;
        case CB_DYNAMIC_STATE_DEPTH_COMPARE_OP:
            return VK_DYNAMIC_STATE_DEPTH_COMPARE_OP;
        case CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
            return VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE;
        case CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
            return VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE;
        case CB_DYNAMIC_STATE_STENCIL_OP:
            return VK_DYNAMIC_STATE_STENCIL_OP;
        case CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
            return VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE;
        case CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
            return VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE;
        case CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE:
            return VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE;
        case CB_DYNAMIC_STATE_LINE_STIPPLE:
            return VK_DYNAMIC_STATE_LINE_STIPPLE;
        case CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
            return VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV;
        case CB_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
            return VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT;
        case CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
            return VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT;
        case CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT:
            return VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT;
        case CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
            return VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT;
        case CB_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR:
            return VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR;
        case CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
            return VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;
        case CB_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
            return VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV;
        case CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV:
            return VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV;
        case CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
            return VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV;
        case CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR:
            return VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR;
        case CB_DYNAMIC_STATE_VERTEX_INPUT_EXT:
            return VK_DYNAMIC_STATE_VERTEX_INPUT_EXT;
        case CB_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
            return VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT;
        case CB_DYNAMIC_STATE_LOGIC_OP_EXT:
            return VK_DYNAMIC_STATE_LOGIC_OP_EXT;
        case CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
            return VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT;
        case CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
            return VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT;
        case CB_DYNAMIC_STATE_POLYGON_MODE_EXT:
            return VK_DYNAMIC_STATE_POLYGON_MODE_EXT;
        case CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
            return VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT;
        case CB_DYNAMIC_STATE_SAMPLE_MASK_EXT:
            return VK_DYNAMIC_STATE_SAMPLE_MASK_EXT;
        case CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
            return VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT;
        case CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
            return VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT;
        case CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
            return VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT;
        case CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
            return VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT;
        case CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
            return VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT;
        case CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT:
            return VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT;
        case CB_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
            return VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT;
        case CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT:
            return VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT;
        case CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
            return VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT;
        case CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
            return VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT;
        case CB_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
            return VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT;
        case CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
            return VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT;
        case CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT:
            return VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT;
        case CB_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
            return VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT;
        case CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
            return VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT;
        case CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
            return VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT;
        case CB_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
            return VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT;
        case CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV:
            return VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV;
        case CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV:
            return VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV;
        case CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
            return VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV;
        case CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
            return VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV;
        case CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
            return VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV;
        case CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
            return VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV;
        case CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
            return VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV;
        case CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV:
            return VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV;
        case CB_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV:
            return VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV;
        case CB_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV:
            return VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV;
        case CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT:
            return VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT;
        case CB_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT:
            return VK_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT;

        default:
            return VK_DYNAMIC_STATE_MAX_ENUM;
    }
}

CBDynamicState ConvertToCBDynamicState(VkDynamicState dynamic_state) {
    switch (dynamic_state) {
        case VK_DYNAMIC_STATE_VIEWPORT:
            return CB_DYNAMIC_STATE_VIEWPORT;
        case VK_DYNAMIC_STATE_SCISSOR:
            return CB_DYNAMIC_STATE_SCISSOR;
        case VK_DYNAMIC_STATE_LINE_WIDTH:
            return CB_DYNAMIC_STATE_LINE_WIDTH;
        case VK_DYNAMIC_STATE_DEPTH_BIAS:
            return CB_DYNAMIC_STATE_DEPTH_BIAS;
        case VK_DYNAMIC_STATE_BLEND_CONSTANTS:
            return CB_DYNAMIC_STATE_BLEND_CONSTANTS;
        case VK_DYNAMIC_STATE_DEPTH_BOUNDS:
            return CB_DYNAMIC_STATE_DEPTH_BOUNDS;
        case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
            return CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
        case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK:
            return CB_DYNAMIC_STATE_STENCIL_WRITE_MASK;
        case VK_DYNAMIC_STATE_STENCIL_REFERENCE:
            return CB_DYNAMIC_STATE_STENCIL_REFERENCE;
        case VK_DYNAMIC_STATE_CULL_MODE:
            return CB_DYNAMIC_STATE_CULL_MODE;
        case VK_DYNAMIC_STATE_FRONT_FACE:
            return CB_DYNAMIC_STATE_FRONT_FACE;
        case VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
            return CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY;
        case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
            return CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT;
        case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
            return CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT;
        case VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
            return CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE;
        case VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
            return CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE;
        case VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
            return CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE;
        case VK_DYNAMIC_STATE_DEPTH_COMPARE_OP:
            return CB_DYNAMIC_STATE_DEPTH_COMPARE_OP;
        case VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
            return CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE;
        case VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
            return CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE;
        case VK_DYNAMIC_STATE_STENCIL_OP:
            return CB_DYNAMIC_STATE_STENCIL_OP;
        case VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
            return CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE;
        case VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
            return CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE;
        case VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE:
            return CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE;
        case VK_DYNAMIC_STATE_LINE_STIPPLE:
            return CB_DYNAMIC_STATE_LINE_STIPPLE;
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
            return CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV;
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
            return CB_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT;
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
            return CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT;
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT:
            return CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT;
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
            return CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT;
        case VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR:
            return CB_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR;
        case VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
            return CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;
        case VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
            return CB_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV;
        case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV:
            return CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV;
        case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
            return CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV;
        case VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR:
            return CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR;
        case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
            return CB_DYNAMIC_STATE_VERTEX_INPUT_EXT;
        case VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
            return CB_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT;
        case VK_DYNAMIC_STATE_LOGIC_OP_EXT:
            return CB_DYNAMIC_STATE_LOGIC_OP_EXT;
        case VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
            return CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT;
        case VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
            return CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT;
        case VK_DYNAMIC_STATE_POLYGON_MODE_EXT:
            return CB_DYNAMIC_STATE_POLYGON_MODE_EXT;
        case VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
            return CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT;
        case VK_DYNAMIC_STATE_SAMPLE_MASK_EXT:
            return CB_DYNAMIC_STATE_SAMPLE_MASK_EXT;
        case VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
            return CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT;
        case VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
            return CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT;
        case VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
            return CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT;
        case VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
            return CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT;
        case VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
            return CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT;
        case VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT:
            return CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT;
        case VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
            return CB_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT;
        case VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT:
            return CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT;
        case VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
            return CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT;
        case VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
            return CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT;
        case VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
            return CB_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT;
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
            return CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT;
        case VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT:
            return CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT;
        case VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
            return CB_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT;
        case VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
            return CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT;
        case VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
            return CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT;
        case VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
            return CB_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT;
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV:
            return CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV;
        case VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV:
            return CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV;
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
            return CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV;
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
            return CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV;
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
            return CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV;
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
            return CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV;
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
            return CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV;
        case VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV:
            return CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV;
        case VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV:
            return CB_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV;
        case VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV:
            return CB_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV;
        case VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT:
            return CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT;
        case VK_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT:
            return CB_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT;

        default:
            return CB_DYNAMIC_STATE_STATUS_NUM;
    }
}

const char* DynamicStateToString(CBDynamicState dynamic_state) {
    return string_VkDynamicState(ConvertToDynamicState(dynamic_state));
}

std::string DynamicStatesToString(CBDynamicFlags const& dynamic_states) {
    std::string ret;
    // enum is not zero based
    for (int index = 1; index < CB_DYNAMIC_STATE_STATUS_NUM; ++index) {
        CBDynamicState status = static_cast<CBDynamicState>(index);
        if (dynamic_states[status]) {
            if (!ret.empty()) ret.append("|");
            ret.append(string_VkDynamicState(ConvertToDynamicState(status)));
        }
    }
    if (ret.empty()) ret.append("(Unknown Dynamic State)");
    return ret;
}

std::string DynamicStatesCommandsToString(CBDynamicFlags const& dynamic_states) {
    std::string ret;
    // enum is not zero based
    for (int index = 1; index < CB_DYNAMIC_STATE_STATUS_NUM; ++index) {
        CBDynamicState status = static_cast<CBDynamicState>(index);
        if (dynamic_states[status]) {
            if (!ret.empty()) ret.append(", ");
            ret.append(DescribeDynamicStateCommand(status));
        }
    }
    if (ret.empty()) ret.append("(Unknown Dynamic State)");
    return ret;
}

std::string DescribeDynamicStateCommand(CBDynamicState dynamic_state) {
    std::stringstream ss;
    vvl::Func func = vvl::Func::Empty;
    switch (dynamic_state) {
        case CB_DYNAMIC_STATE_VIEWPORT:
            func = vvl::Func::vkCmdSetViewport;
            break;
        case CB_DYNAMIC_STATE_SCISSOR:
            func = vvl::Func::vkCmdSetScissor;
            break;
        case CB_DYNAMIC_STATE_LINE_WIDTH:
            func = vvl::Func::vkCmdSetLineWidth;
            break;
        case CB_DYNAMIC_STATE_DEPTH_BIAS:
            func = vvl::Func::vkCmdSetDepthBias;
            break;
        case CB_DYNAMIC_STATE_BLEND_CONSTANTS:
            func = vvl::Func::vkCmdSetBlendConstants;
            break;
        case CB_DYNAMIC_STATE_DEPTH_BOUNDS:
            func = vvl::Func::vkCmdSetDepthBounds;
            break;
        case CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
            func = vvl::Func::vkCmdSetStencilCompareMask;
            break;
        case CB_DYNAMIC_STATE_STENCIL_WRITE_MASK:
            func = vvl::Func::vkCmdSetStencilWriteMask;
            break;
        case CB_DYNAMIC_STATE_STENCIL_REFERENCE:
            func = vvl::Func::vkCmdSetStencilReference;
            break;
        case CB_DYNAMIC_STATE_CULL_MODE:
            func = vvl::Func::vkCmdSetCullMode;
            break;
        case CB_DYNAMIC_STATE_FRONT_FACE:
            func = vvl::Func::vkCmdSetFrontFace;
            break;
        case CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
            func = vvl::Func::vkCmdSetPrimitiveTopology;
            break;
        case CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
            func = vvl::Func::vkCmdSetViewportWithCount;
            break;
        case CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
            func = vvl::Func::vkCmdSetScissorWithCount;
            break;
        case CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
            func = vvl::Func::vkCmdBindVertexBuffers2;
            break;
        case CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
            func = vvl::Func::vkCmdSetDepthTestEnable;
            break;
        case CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
            func = vvl::Func::vkCmdSetDepthWriteEnable;
            break;
        case CB_DYNAMIC_STATE_DEPTH_COMPARE_OP:
            func = vvl::Func::vkCmdSetDepthCompareOp;
            break;
        case CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
            func = vvl::Func::vkCmdSetDepthBoundsTestEnable;
            break;
        case CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
            func = vvl::Func::vkCmdSetStencilTestEnable;
            break;
        case CB_DYNAMIC_STATE_STENCIL_OP:
            func = vvl::Func::vkCmdSetStencilOp;
            break;
        case CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
            func = vvl::Func::vkCmdSetRasterizerDiscardEnable;
            break;
        case CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
            func = vvl::Func::vkCmdSetDepthBiasEnable;
            break;
        case CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE:
            func = vvl::Func::vkCmdSetPrimitiveRestartEnable;
            break;
        case CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
            func = vvl::Func::vkCmdSetViewportWScalingNV;
            break;
        case CB_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
            func = vvl::Func::vkCmdSetDiscardRectangleEXT;
            break;
        case CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
            func = vvl::Func::vkCmdSetDiscardRectangleEnableEXT;
            break;
        case CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT:
            func = vvl::Func::vkCmdSetDiscardRectangleModeEXT;
            break;
        case CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
            func = vvl::Func::vkCmdSetSampleLocationsEXT;
            break;
        case CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
            func = vvl::Func::vkCmdSetViewportShadingRatePaletteNV;
            break;
        case CB_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
            func = vvl::Func::vkCmdSetCoarseSampleOrderNV;
            break;
        case CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV:
            func = vvl::Func::vkCmdSetExclusiveScissorEnableNV;
            break;
        case CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
            func = vvl::Func::vkCmdSetExclusiveScissorNV;
            break;
        case CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR:
            func = vvl::Func::vkCmdSetFragmentShadingRateKHR;
            break;
        case CB_DYNAMIC_STATE_LINE_STIPPLE:
            func = vvl::Func::vkCmdSetLineStipple;
            break;
        case CB_DYNAMIC_STATE_VERTEX_INPUT_EXT:
            func = vvl::Func::vkCmdSetVertexInputEXT;
            break;
        case CB_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
            func = vvl::Func::vkCmdSetPatchControlPointsEXT;
            break;
        case CB_DYNAMIC_STATE_LOGIC_OP_EXT:
            func = vvl::Func::vkCmdSetLogicOpEXT;
            break;
        case CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
            func = vvl::Func::vkCmdSetColorWriteEnableEXT;
            break;
        case CB_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
            func = vvl::Func::vkCmdSetTessellationDomainOriginEXT;
            break;
        case CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
            func = vvl::Func::vkCmdSetDepthClampEnableEXT;
            break;
        case CB_DYNAMIC_STATE_POLYGON_MODE_EXT:
            func = vvl::Func::vkCmdSetPolygonModeEXT;
            break;
        case CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
            func = vvl::Func::vkCmdSetRasterizationSamplesEXT;
            break;
        case CB_DYNAMIC_STATE_SAMPLE_MASK_EXT:
            func = vvl::Func::vkCmdSetSampleMaskEXT;
            break;
        case CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
            func = vvl::Func::vkCmdSetAlphaToCoverageEnableEXT;
            break;
        case CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
            func = vvl::Func::vkCmdSetAlphaToOneEnableEXT;
            break;
        case CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
            func = vvl::Func::vkCmdSetLogicOpEnableEXT;
            break;
        case CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
            func = vvl::Func::vkCmdSetColorBlendEnableEXT;
            break;
        case CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
            func = vvl::Func::vkCmdSetColorBlendEquationEXT;
            break;
        case CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT:
            func = vvl::Func::vkCmdSetColorWriteMaskEXT;
            break;
        case CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT:
            func = vvl::Func::vkCmdSetRasterizationStreamEXT;
            break;
        case CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
            func = vvl::Func::vkCmdSetConservativeRasterizationModeEXT;
            break;
        case CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
            func = vvl::Func::vkCmdSetExtraPrimitiveOverestimationSizeEXT;
            break;
        case CB_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
            func = vvl::Func::vkCmdSetDepthClipEnableEXT;
            break;
        case CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
            func = vvl::Func::vkCmdSetSampleLocationsEnableEXT;
            break;
        case CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT:
            func = vvl::Func::vkCmdSetColorBlendAdvancedEXT;
            break;
        case CB_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
            func = vvl::Func::vkCmdSetProvokingVertexModeEXT;
            break;
        case CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
            func = vvl::Func::vkCmdSetLineRasterizationModeEXT;
            break;
        case CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
            func = vvl::Func::vkCmdSetLineStippleEnableEXT;
            break;
        case CB_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
            func = vvl::Func::vkCmdSetDepthClipNegativeOneToOneEXT;
            break;
        case CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV:
            func = vvl::Func::vkCmdSetViewportWScalingEnableNV;
            break;
        case CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV:
            func = vvl::Func::vkCmdSetViewportSwizzleNV;
            break;
        case CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
            func = vvl::Func::vkCmdSetCoverageToColorEnableNV;
            break;
        case CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
            func = vvl::Func::vkCmdSetCoverageToColorLocationNV;
            break;
        case CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
            func = vvl::Func::vkCmdSetCoverageModulationModeNV;
            break;
        case CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
            func = vvl::Func::vkCmdSetCoverageModulationTableEnableNV;
            break;
        case CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
            func = vvl::Func::vkCmdSetCoverageModulationTableNV;
            break;
        case CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV:
            func = vvl::Func::vkCmdSetShadingRateImageEnableNV;
            break;
        case CB_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV:
            func = vvl::Func::vkCmdSetRepresentativeFragmentTestEnableNV;
            break;
        case CB_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV:
            func = vvl::Func::vkCmdSetCoverageReductionModeNV;
            break;
        case CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT:
            func = vvl::Func::vkCmdSetAttachmentFeedbackLoopEnableEXT;
            break;
        case CB_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR:
            func = vvl::Func::vkCmdSetRayTracingPipelineStackSizeKHR;
            break;
        case CB_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT:
            func = vvl::Func::vkCmdSetDepthClampRangeEXT;
            break;
        default:
            ss << "(Unknown Dynamic State) ";
    }

    ss << String(func);

    // Currently only exception that has 2 commands that can set it
    if (dynamic_state == CB_DYNAMIC_STATE_DEPTH_BIAS) {
        ss << " or " << String(vvl::Func::vkCmdSetDepthBias2EXT);
    }

    return ss.str();
}

// For anything with multple uses
static std::string_view rasterizer_discard_enable_dynamic{
    "vkCmdSetRasterizerDiscardEnable last set rasterizerDiscardEnable to VK_FALSE.\n"};
static std::string_view rasterizer_discard_enable_static{
    "VkPipelineRasterizationStateCreateInfo::rasterizerDiscardEnable was VK_FALSE in the last bound graphics pipeline.\n"};
static std::string_view stencil_test_enable_dynamic{"vkCmdSetStencilTestEnable last set stencilTestEnable to VK_TRUE.\n"};
static std::string_view stencil_test_enable_static{
    "VkPipelineDepthStencilStateCreateInfo::stencilTestEnable was VK_TRUE in the last bound graphics pipeline.\n"};

std::string DescribeDynamicStateDependency(CBDynamicState dynamic_state, const vvl::Pipeline* pipeline) {
    std::stringstream ss;
    switch (dynamic_state) {
        case CB_DYNAMIC_STATE_DEPTH_BIAS:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE)) {
                ss << "vkCmdSetDepthBiasEnable last set depthTestEnable to VK_TRUE.\n";
            } else {
                ss << "VkPipelineRasterizationStateCreateInfo::depthTestEnable was VK_TRUE in the last bound graphics pipeline.\n";
            }
            break;
        case CB_DYNAMIC_STATE_DEPTH_BOUNDS:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE)) {
                ss << "vkCmdSetDepthBoundsTestEnable last set depthBoundsTestEnable to VK_TRUE.\n";
            } else {
                ss << "VkPipelineDepthStencilStateCreateInfo::depthBoundsTestEnable was VK_TRUE in the last bound graphics "
                      "pipeline.\n";
            }
            break;
        case CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE)) {
                ss << stencil_test_enable_dynamic;
            } else {
                ss << stencil_test_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_STENCIL_WRITE_MASK:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE)) {
                ss << stencil_test_enable_dynamic;
            } else {
                ss << stencil_test_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_STENCIL_REFERENCE:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE)) {
                ss << stencil_test_enable_dynamic;
            } else {
                ss << stencil_test_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_CULL_MODE:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_FRONT_FACE:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_DEPTH_COMPARE_OP:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE)) {
                ss << "vkCmdSetDepthTestEnable last set depthTestEnable to VK_TRUE.\n";
            } else {
                ss << "VkPipelineDepthStencilStateCreateInfo::depthTestEnable was VK_TRUE in the last bound graphics pipeline.\n";
            }
            break;
        case CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_STENCIL_OP:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE)) {
                ss << stencil_test_enable_dynamic;
            } else {
                ss << stencil_test_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV)) {
                ss << "vkCmdSetViewportWScalingEnableNV last set viewportWScalingEnable to VK_TRUE.\n";
            } else {
                ss << "VkPipelineViewportStateCreateInfo::pNext->VkPipelineViewportWScalingStateCreateInfoNV::"
                      "viewportWScalingEnable was VK_TRUE in the last bound graphics pipeline.\n";
            }
            break;
        case CB_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT)) {
                ss << "vkCmdSetDiscardRectangleEnableEXT last set discardRectangleEnable to VK_TRUE.\n";
            } else {
                ss << "VkGraphicsPipelineCreateInfo::pNext->VkPipelineDiscardRectangleStateCreateInfoEXT::discardRectangleCount "
                      "was greater than zero in the last bound graphics pipeline.\n";
            }
            break;
        case CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT)) {
                ss << "vkCmdSetDiscardRectangleEnableEXT last set discardRectangleEnable to VK_TRUE.\n";
            } else {
                ss << "VkGraphicsPipelineCreateInfo::pNext->VkPipelineDiscardRectangleStateCreateInfoEXT::discardRectangleCount "
                      "was greater than zero in the last bound graphics pipeline.\n";
            }
            break;
        case CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)) {
                ss << "vkCmdSetSampleLocationsEnableEXT last set logicOpEnable to VK_TRUE.\n";
            } else {
                ss << "VkPipelineMultisampleStateCreateInfo::pNext->VkPipelineSampleLocationsStateCreateInfoEXT::"
                      "sampleLocationsEnable was VK_TRUE in the last bound graphics pipeline.\n";
            }
            break;
        case CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV)) {
                ss << "vkCmdSetShadingRateImageEnableNV last set shadingRateImageEnable to VK_TRUE.\n";
            } else {
                ss << "VkPipelineViewportStateCreateInfo::pNext->VkPipelineViewportShadingRateImageStateCreateInfoNV::"
                      "shadingRateImageEnable was VK_TRUE in the last bound graphics pipeline.\n";
            }
            break;
        case CB_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
            break;
        case CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_LINE_STIPPLE:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT)) {
                ss << "vkCmdSetLineStippleEnableEXT last set stippledLineEnable to VK_TRUE.\n";
            } else {
                ss << "VkPipelineRasterizationLineStateCreateInfo::stippledLineEnable was VK_TRUE in the last bound graphics "
                      "pipeline.\n";
            }
            break;
        case CB_DYNAMIC_STATE_LOGIC_OP_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)) {
                ss << "vkCmdSetLogicOpEnableEXT last set logicOpEnable to VK_TRUE.\n";
            } else {
                ss << "VkPipelineColorBlendStateCreateInfo::logicOpEnable was VK_TRUE in the last bound graphics pipeline.\n";
            }
            break;
        case CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            break;
        case CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                ss << rasterizer_discard_enable_dynamic;
            } else {
                ss << rasterizer_discard_enable_static;
            }
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV)) {
                ss << "vkCmdSetCoverageModulationTableEnableNV last set coverageModulationTableEnable to VK_TRUE.\n";
            } else {
                ss << "VkPipelineMultisampleStateCreateInfo::pNext->VkPipelineCoverageModulationStateCreateInfoNV::"
                      "coverageModulationTableEnable was VK_TRUE in the last bound graphics pipeline.\n";
            }
            break;
        case CB_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT:
            if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT)) {
                ss << "vkCmdSetDepthClampEnableEXT last set depthClampEnable to VK_TRUE.\n";
            } else {
                ss << "VkPipelineRasterizationStateCreateInfo::depthClampEnable was VK_TRUE in the last bound graphics pipeline.\n";
            }
            break;
        default:
            break;  // not all state will be dependent on other state
    }

    return ss.str();
}

// NOLINTEND
