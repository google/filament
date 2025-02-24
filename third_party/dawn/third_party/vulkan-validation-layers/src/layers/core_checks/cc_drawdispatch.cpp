/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
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
 */

#include "drawdispatch/drawdispatch_vuids.h"
#include "core_validation.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/shader_module.h"

using vvl::DrawDispatchVuid;
using vvl::GetDrawDispatchVuid;

bool CoreChecks::ValidateGraphicsIndexedCmd(const vvl::CommandBuffer &cb_state, const Location &loc) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);
    const auto buffer_state = Get<vvl::Buffer>(cb_state.index_buffer_binding.buffer);
    if (!buffer_state && !enabled_features.maintenance6 && !enabled_features.nullDescriptor) {
        skip |= LogError(vuid.index_binding_07312, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), loc,
                         "Index buffer object has not been bound to this command buffer.");
    }
    return skip;
}

bool CoreChecks::ValidateCmdDrawInstance(const vvl::CommandBuffer &cb_state, uint32_t instanceCount, uint32_t firstInstance,
                                         const Location &loc) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);
    const auto *pipeline_state = cb_state.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);

    // Verify maxMultiviewInstanceIndex
    if (cb_state.active_render_pass && cb_state.active_render_pass->has_multiview_enabled &&
        ((static_cast<uint64_t>(instanceCount) + static_cast<uint64_t>(firstInstance)) >
         static_cast<uint64_t>(phys_dev_props_core11.maxMultiviewInstanceIndex))) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(cb_state.active_render_pass->Handle());
        skip |= LogError(vuid.max_multiview_instance_index_02688, objlist, loc,
                         "renderpass instance has multiview enabled, and maxMultiviewInstanceIndex: %" PRIu32
                         ", but instanceCount: %" PRIu32 " and firstInstance: %" PRIu32 ".",
                         phys_dev_props_core11.maxMultiviewInstanceIndex, instanceCount, firstInstance);
    }

    if (pipeline_state && pipeline_state->GraphicsCreateInfo().pVertexInputState) {
        const auto *vertex_input_divisor_state = vku::FindStructInPNextChain<VkPipelineVertexInputDivisorStateCreateInfo>(
            pipeline_state->GraphicsCreateInfo().pVertexInputState->pNext);
        if (vertex_input_divisor_state && phys_dev_props_core14.supportsNonZeroFirstInstance == VK_FALSE && firstInstance != 0u) {
            for (uint32_t i = 0; i < vertex_input_divisor_state->vertexBindingDivisorCount; ++i) {
                if (vertex_input_divisor_state->pVertexBindingDivisors[i].divisor != 1u) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline_state->Handle());
                    skip |= LogError(vuid.vertex_input_09461, objlist, loc,
                                     "VkPipelineVertexInputDivisorStateCreateInfo::pVertexBindingDivisors[%" PRIu32
                                     "].divisor is %" PRIu32 " and firstInstance is %" PRIu32
                                     ", but supportsNonZeroFirstInstance is VK_FALSE.",
                                     i, vertex_input_divisor_state->pVertexBindingDivisors[i].divisor, firstInstance);
                    break;  // only report first instance of the error
                }
            }
        }
    }

    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT) &&
            phys_dev_props_core14.supportsNonZeroFirstInstance == VK_FALSE && firstInstance != 0u) {
            for (const auto &binding_state : cb_state.dynamic_state_value.vertex_bindings) {
                const auto &desc = binding_state.second.desc;
                if (desc.divisor != 1u) {
                    LogObjectList objlist(cb_state.Handle());
                    if (pipeline_state) {
                        objlist.add(pipeline_state->Handle());
                    }
                    skip |= LogError(vuid.vertex_input_09462, objlist, loc,
                                     "vkCmdSetVertexInputEXT set pVertexBindingDivisors[%" PRIu32 "] (binding %" PRIu32
                                     ") divisor as %" PRIu32 ", but firstInstance is %" PRIu32
                                     " and supportsNonZeroFirstInstance is VK_FALSE.",
                                     binding_state.second.index, desc.binding, desc.divisor, firstInstance);
                    break;
                }
            }
        }
    }

    return skip;
}

// VTG = Vertex Tessellation Geometry
bool CoreChecks::ValidateVTGShaderStages(const vvl::CommandBuffer &cb_state, const Location &loc) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);

    const auto *pipeline_state = cb_state.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (pipeline_state && pipeline_state->active_shaders & (VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)) {
        skip |= LogError(
            vuid.invalid_mesh_shader_stages_06481, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), loc,
            "The bound graphics pipeline must not have been created with "
            "VK_SHADER_STAGE_TASK_BIT_EXT or VK_SHADER_STAGE_MESH_BIT_EXT. Active shader stages on the bound pipeline are %s.",
            string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateMeshShaderStage(const vvl::CommandBuffer &cb_state, const Location &loc, bool is_NV) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);

    const auto *pipeline_state = cb_state.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (pipeline_state && !(pipeline_state->active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT)) {
        skip |= LogError(vuid.missing_mesh_shader_stages_07080, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), loc,
                         "The current pipeline bound to VK_PIPELINE_BIND_POINT_GRAPHICS must contain a shader stage using the "
                         "%s Execution Model. Active shader stages on the bound pipeline are %s.",
                         is_NV ? "MeshNV" : "MeshEXT", string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    if (pipeline_state &&
        (pipeline_state->active_shaders & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                           VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT))) {
        skip |= LogError(vuid.mesh_shader_stages_06480, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), loc,
                         "The bound graphics pipeline must not have been created with "
                         "VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "
                         "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT or VK_SHADER_STAGE_GEOMETRY_BIT. Active shader stages on the "
                         "bound pipeline are %s.",
                         string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    for (const auto &query : cb_state.activeQueries) {
        const auto query_pool_state = Get<vvl::QueryPool>(query.pool);
        if (!query_pool_state) continue;
        if (query_pool_state->create_info.queryType == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT) {
            skip |= LogError(vuid.xfb_queries_07074, cb_state.Handle(), loc, "Query with type %s is active.",
                             string_VkQueryType(query_pool_state->create_info.queryType));
        }
        if (query_pool_state->create_info.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
            skip |= LogError(vuid.pg_queries_07075, cb_state.Handle(), loc, "Query with type %s is active.",
                             string_VkQueryType(query_pool_state->create_info.queryType));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                        uint32_t firstVertex, uint32_t firstInstance, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateCmdDrawInstance(cb_state, instanceCount, firstInstance, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                                uint32_t firstInstance, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (!enabled_features.multiDraw) {
        skip |= LogError("VUID-vkCmdDrawMultiEXT-None-04933", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location, "The multiDraw feature was not enabled.");
    }
    if (drawCount > phys_dev_ext_props.multi_draw_props.maxMultiDrawCount) {
        skip |=
            LogError("VUID-vkCmdDrawMultiEXT-drawCount-04934", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                     error_obj.location.dot(Field::drawCount), "(%" PRIu32 ") must be less than maxMultiDrawCount (%" PRIu32 ").",
                     drawCount, phys_dev_ext_props.multi_draw_props.maxMultiDrawCount);
    }
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMultiEXT-drawCount-09628", stride,
                                                Struct::VkMultiDrawInfoEXT, sizeof(VkMultiDrawInfoEXT), error_obj.location);
    }
    if (drawCount != 0 && !pVertexInfo) {
        skip |= LogError("VUID-vkCmdDrawMultiEXT-drawCount-04935", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount), "is %" PRIu32 " but pVertexInfo is NULL.", drawCount);
    }

    skip |= ValidateCmdDrawInstance(cb_state, instanceCount, firstInstance, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::ValidateCmdDrawIndexedBufferSize(const vvl::CommandBuffer &cb_state, uint32_t indexCount, uint32_t firstIndex,
                                                  const Location &loc, const char *first_index_vuid) const {
    bool skip = false;
    if (enabled_features.robustBufferAccess2) {
        return skip;
    }
    const auto &index_buffer_binding = cb_state.index_buffer_binding;
    if (const auto buffer_state = Get<vvl::Buffer>(index_buffer_binding.buffer)) {
        const uint32_t index_size = GetIndexAlignment(index_buffer_binding.index_type);
        // This doesn't exactly match the pseudocode of the VUID, but the binding size is the *bound* size, such that the offset
        // has already been accounted for (subtracted from the buffer size), and is consistent with the use of
        // BufferBinding::size for vertex buffer bindings (which record the *bound* size, not the size of the bound buffer)
        VkDeviceSize end_offset = static_cast<VkDeviceSize>(index_size * (firstIndex + indexCount));
        if (end_offset > index_buffer_binding.size) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            objlist.add(buffer_state->Handle());
            skip |= LogError(first_index_vuid, objlist, loc,
                             "index size (%" PRIu32 ") * (firstIndex (%" PRIu32 ") + indexCount (%" PRIu32
                             ")) "
                             "+ binding offset (%" PRIuLEAST64 ") = an ending offset of %" PRIuLEAST64
                             " bytes, which is greater than the index buffer size (%" PRIuLEAST64 ").",
                             index_size, firstIndex, indexCount, index_buffer_binding.offset,
                             end_offset + index_buffer_binding.offset, index_buffer_binding.size + index_buffer_binding.offset);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                               uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateCmdDrawInstance(cb_state, instanceCount, firstInstance, error_obj.location);
    skip |= ValidateGraphicsIndexedCmd(cb_state, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateCmdDrawIndexedBufferSize(cb_state, indexCount, firstIndex, error_obj.location,
                                             "VUID-vkCmdDrawIndexed-robustBufferAccess2-08798");
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                       const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount,
                                                       uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (!enabled_features.multiDraw) {
        skip |= LogError("VUID-vkCmdDrawMultiIndexedEXT-None-04937", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location, "multiDraw feature was not enabled.");
    }
    if (drawCount > phys_dev_ext_props.multi_draw_props.maxMultiDrawCount) {
        skip |= LogError("VUID-vkCmdDrawMultiIndexedEXT-drawCount-04939", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be less than VkPhysicalDeviceMultiDrawPropertiesEXT::maxMultiDrawCount (%" PRIu32 ").",
                         drawCount, phys_dev_ext_props.multi_draw_props.maxMultiDrawCount);
    }

    skip |= ValidateCmdDrawInstance(cb_state, instanceCount, firstInstance, error_obj.location);
    skip |= ValidateGraphicsIndexedCmd(cb_state, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);

    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMultiIndexedEXT-drawCount-09629", stride,
                                                Struct::VkMultiDrawIndexedInfoEXT, sizeof(VkMultiDrawIndexedInfoEXT),
                                                error_obj.location);
    }

    // only index into pIndexInfo if we know parameters are sane
    if (drawCount != 0 && !pIndexInfo) {
        skip |= LogError("VUID-vkCmdDrawMultiIndexedEXT-drawCount-04940", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount), "is %" PRIu32 " but pIndexInfo is NULL.", drawCount);
    } else {
        const auto info_bytes = reinterpret_cast<const char *>(pIndexInfo);
        for (uint32_t i = 0; i < drawCount; i++) {
            const auto info_ptr = reinterpret_cast<const VkMultiDrawIndexedInfoEXT *>(info_bytes + i * stride);
            skip |= ValidateCmdDrawIndexedBufferSize(cb_state, info_ptr->indexCount, info_ptr->firstIndex,
                                                     error_obj.location.dot(Field::pIndexInfo, i),
                                                     "VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-08798");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<vvl::Buffer>(buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);

    if (!enabled_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError("VUID-vkCmdDrawIndirect-drawCount-02718", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawIndirect-drawCount-02719", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") is not less than or equal to the maximum allowed (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }
    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndirect-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 ") must be a multiple of 4.", offset);
    }
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndirect-drawCount-00476", stride,
                                                Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), error_obj.location);
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndirect-drawCount-00488", stride,
                                                Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), drawCount, offset,
                                                *buffer_state, error_obj.location);
    } else if ((drawCount == 1) && (offset + sizeof(VkDrawIndirectCommand)) > buffer_state->create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(buffer);
        skip |= LogError("VUID-vkCmdDrawIndirect-drawCount-00487", objlist, error_obj.location.dot(Field::drawCount),
                         "is 1 and (offset + sizeof(VkDrawIndirectCommand)) (%" PRIu64
                         ") is not less than "
                         "or equal to the size of buffer (%" PRIu64 ").",
                         (offset + sizeof(VkDrawIndirectCommand)), buffer_state->create_info.size);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndirectCommand structures accessed by this command must be 0, which will require access to the contents of 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateGraphicsIndexedCmd(cb_state, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<vvl::Buffer>(buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);

    if (!enabled_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect-drawCount-02718", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect-drawCount-02719", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") is not less than or equal to the maximum allowed (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }

    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndexedIndirect-drawCount-00528", stride,
                                                Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                                error_obj.location);
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndexedIndirect-drawCount-00540", stride,
                                                Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                                drawCount, offset, *buffer_state, error_obj.location);
    } else if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 ") must be a multiple of 4.", offset);
    } else if ((drawCount == 1) && (offset + sizeof(VkDrawIndexedIndirectCommand)) > buffer_state->create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(buffer);
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect-drawCount-00539", objlist, error_obj.location.dot(Field::drawCount),
                         "is 1 and (offset + sizeof(VkDrawIndexedIndirectCommand)) (%" PRIu64
                         ") is not less than "
                         "or equal to the size of buffer (%" PRIu64 ").",
                         (offset + sizeof(VkDrawIndexedIndirectCommand)), buffer_state->create_info.size);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndexedIndirectCommand structures accessed by this command must be 0, which will require access to the contents of
    // 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                            uint32_t groupCountZ, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);

    if (groupCountX > phys_dev_props.limits.maxComputeWorkGroupCount[0]) {
        skip |= LogError("VUID-vkCmdDispatch-groupCountX-00386", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location.dot(Field::groupCountX),
                         "(%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").", groupCountX,
                         phys_dev_props.limits.maxComputeWorkGroupCount[0]);
    }

    if (groupCountY > phys_dev_props.limits.maxComputeWorkGroupCount[1]) {
        skip |= LogError("VUID-vkCmdDispatch-groupCountY-00387", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location.dot(Field::groupCountY),
                         "(%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").", groupCountY,
                         phys_dev_props.limits.maxComputeWorkGroupCount[1]);
    }

    if (groupCountZ > phys_dev_props.limits.maxComputeWorkGroupCount[2]) {
        skip |= LogError("VUID-vkCmdDispatch-groupCountZ-00388", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location.dot(Field::groupCountZ),
                         "(%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").", groupCountZ,
                         phys_dev_props.limits.maxComputeWorkGroupCount[2]);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);

    // Paired if {} else if {} tests used to avoid any possible uint underflow
    uint32_t limit = phys_dev_props.limits.maxComputeWorkGroupCount[0];
    if (baseGroupX >= limit) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-baseGroupX-00421", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupX),
                     "(%" PRIu32 ") equals or exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").", baseGroupX, limit);
    } else if (groupCountX > (limit - baseGroupX)) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-groupCountX-00424", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupX),
                     "(%" PRIu32 ") + groupCountX (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                     baseGroupX, groupCountX, limit);
    }

    limit = phys_dev_props.limits.maxComputeWorkGroupCount[1];
    if (baseGroupY >= limit) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-baseGroupX-00422", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupY),
                     "(%" PRIu32 ") equals or exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").", baseGroupY, limit);
    } else if (groupCountY > (limit - baseGroupY)) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-groupCountY-00425", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupY),
                     "(%" PRIu32 ") + groupCountY (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                     baseGroupY, groupCountY, limit);
    }

    limit = phys_dev_props.limits.maxComputeWorkGroupCount[2];
    if (baseGroupZ >= limit) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-baseGroupZ-00423", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupZ),
                     "(%" PRIu32 ") equals or exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").", baseGroupZ, limit);
    } else if (groupCountZ > (limit - baseGroupZ)) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-groupCountZ-00426", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupZ),
                     "(%" PRIu32 ") + groupCountZ (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                     baseGroupZ, groupCountZ, limit);
    }

    if (baseGroupX || baseGroupY || baseGroupZ) {
        const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_COMPUTE);
        const auto &last_bound_state = cb_state.lastBound[lv_bind_point];
        const auto *pipeline_state = last_bound_state.pipeline_state;
        if (pipeline_state) {
            if (!(pipeline_state->create_flags & VK_PIPELINE_CREATE_DISPATCH_BASE)) {
                skip |= LogError("VUID-vkCmdDispatchBase-baseGroupX-00427", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                                 error_obj.location,
                                 "If any of baseGroupX (%" PRIu32 "), baseGroupY (%" PRIu32 "), or baseGroupZ (%" PRIu32
                                 ") are not zero, then the bound compute pipeline "
                                 "must have been created with the VK_PIPELINE_CREATE_DISPATCH_BASE flag",
                                 baseGroupX, baseGroupY, baseGroupZ);
            }
        } else {
            const auto *shader_object = last_bound_state.GetShaderState(ShaderObjectStage::COMPUTE);
            if (shader_object && ((shader_object->create_info.flags & VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT) == 0)) {
                skip |= LogError("VUID-vkCmdDispatchBase-baseGroupX-00427", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                                 error_obj.location,
                                 "If any of baseGroupX (%" PRIu32 "), baseGroupY (%" PRIu32 "), or baseGroupZ (%" PRIu32
                                 ") are not zero, then the bound compute shader object "
                                 "must have been created with the VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT flag",
                                 baseGroupX, baseGroupY, baseGroupZ);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                   uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                          error_obj);
}

bool CoreChecks::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    auto buffer_state = Get<vvl::Buffer>(buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDispatchIndirect-offset-02710", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 ") must be a multiple of 4.", offset);
    }
    if ((offset + sizeof(VkDispatchIndirectCommand)) > buffer_state->create_info.size) {
        skip |= LogError("VUID-vkCmdDispatchIndirect-offset-00407", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location,
                         "The (offset + sizeof(VkDrawIndexedIndirectCommand)) (%" PRIu64
                         ")  is greater than the "
                         "size of the buffer (%" PRIu64 ").",
                         offset + sizeof(VkDispatchIndirectCommand), buffer_state->create_info.size);
    }
    return skip;
}
bool CoreChecks::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndirectCount-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }

    if (countBufferOffset & 3) {
        skip |=
            LogError("VUID-vkCmdDrawIndirectCount-countBufferOffset-02716", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                     error_obj.location.dot(Field::countBufferOffset), "(%" PRIu64 "), is not a multiple of 4.", countBufferOffset);
    }

    if ((extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.drawIndirectCount == VK_FALSE))) {
        skip |= LogError("VUID-vkCmdDrawIndirectCount-None-04445", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location,
                         "Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.");
    }
    auto buffer_state = Get<vvl::Buffer>(buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);
    skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndirectCount-stride-03110", stride,
                                            Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), error_obj.location);
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndirectCount-maxDrawCount-03111", stride,
                                                Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), maxDrawCount, offset,
                                                *buffer_state, error_obj.location);
    }

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    auto count_buffer_state = Get<vvl::Buffer>(countBuffer);
    ASSERT_AND_RETURN_SKIP(count_buffer_state);
    skip |= ValidateIndirectCountCmd(cb_state, *count_buffer_state, countBufferOffset, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }
    if (countBufferOffset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-02716",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::countBufferOffset),
                         "(%" PRIu64 "), is not a multiple of 4.", countBufferOffset);
    }
    if ((extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.drawIndirectCount == VK_FALSE))) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount-None-04445", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location,
                         "Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.");
    }
    skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndexedIndirectCount-stride-03142", stride,
                                            Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                            error_obj.location);
    auto buffer_state = Get<vvl::Buffer>(buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndexedIndirectCount-maxDrawCount-03143", stride,
                                                Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                                maxDrawCount, offset, *buffer_state, error_obj.location);
    }

    skip |= ValidateGraphicsIndexedCmd(cb_state, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    auto count_buffer_state = Get<vvl::Buffer>(countBuffer);
    ASSERT_AND_RETURN_SKIP(count_buffer_state);
    skip |= ValidateIndirectCountCmd(cb_state, *count_buffer_state, countBufferOffset, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride,
                                                               const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

bool CoreChecks::PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                            uint32_t firstInstance, VkBuffer counterBuffer,
                                                            VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                            uint32_t vertexStride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (!enabled_features.transformFeedback) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-transformFeedback-02287",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location,
                         "transformFeedback feature is not enabled.");
    }
    if (IsExtEnabled(extensions.vk_ext_transform_feedback) && !phys_dev_ext_props.transform_feedback_props.transformFeedbackDraw) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-transformFeedbackDraw-02288",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location,
                         "VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackDraw is not supported");
    }
    if ((vertexStride <= 0) || (vertexStride > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride)) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-02289",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::vertexStride),
                         "(%" PRIu32 ") must be between 0 and maxTransformFeedbackBufferDataStride (%" PRIu32 ").", vertexStride,
                         phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride);
    }

    if (SafeModulo(counterBufferOffset, 4) != 0) {
        skip |= LogError(
            "VUID-vkCmdDrawIndirectByteCountEXT-counterBufferOffset-04568", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::counterBufferOffset), "(%" PRIu64 ") must be a multiple of 4.", counterBufferOffset);
    }
    // VUs being added in https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6310
    if (SafeModulo(counterOffset, 4) != 0) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-counterOffset-09474",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::counterOffset),
                         "(%" PRIu32 ") must be a multiple of 4.", counterOffset);
    }
    if (SafeModulo(vertexStride, 4) != 0) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-09475",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::vertexStride),
                         "(%" PRIu32 ") must be a multiple of 4.", vertexStride);
    }

    skip |= ValidateCmdDrawInstance(cb_state, instanceCount, firstInstance, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto counter_buffer_state = Get<vvl::Buffer>(counterBuffer);
    ASSERT_AND_RETURN_SKIP(counter_buffer_state);
    skip |= ValidateIndirectCmd(cb_state, *counter_buffer_state, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                               VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                               VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                               VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                               VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                               VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                               uint32_t width, uint32_t height, uint32_t depth,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (SafeModulo(callableShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02462",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::callableShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(callableShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02465",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::callableShaderBindingStride),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (callableShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02468",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::callableShaderBindingStride),
                         "must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride. ");
    }

    // hitShader
    if (SafeModulo(hitShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02460",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::hitShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(hitShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02464",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::hitShaderBindingStride),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (hitShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02467",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::hitShaderBindingStride),
                         "must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // missShader
    if (SafeModulo(missShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02458",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::missShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(missShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingStride-02463",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::missShaderBindingStride),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (missShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingStride-02466",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::missShaderBindingStride),
                         "must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // raygenShader
    if (SafeModulo(raygenShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02456",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::raygenShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (width > phys_dev_props.limits.maxComputeWorkGroupCount[0]) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-width-02469", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::width),
                         "must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[0].");
    }
    if (height > phys_dev_props.limits.maxComputeWorkGroupCount[1]) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-height-02470", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::height),
                         "must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1].");
    }
    if (depth > phys_dev_props.limits.maxComputeWorkGroupCount[2]) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-depth-02471", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::depth),
                         "must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2].");
    }

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, error_obj.location);
    auto callable_shader_buffer_state = Get<vvl::Buffer>(callableShaderBindingTableBuffer);
    if (callable_shader_buffer_state && callableShaderBindingOffset >= callable_shader_buffer_state->create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(callableShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02461", objlist,
                         error_obj.location.dot(Field::callableShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of callableShaderBindingTableBuffer %" PRIu64 " .",
                         callableShaderBindingOffset, callable_shader_buffer_state->create_info.size);
    }
    auto hit_shader_buffer_state = Get<vvl::Buffer>(hitShaderBindingTableBuffer);
    if (hit_shader_buffer_state && hitShaderBindingOffset >= hit_shader_buffer_state->create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(hitShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02459", objlist,
                         error_obj.location.dot(Field::hitShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of hitShaderBindingTableBuffer %" PRIu64 " .",
                         hitShaderBindingOffset, hit_shader_buffer_state->create_info.size);
    }
    auto miss_shader_buffer_state = Get<vvl::Buffer>(missShaderBindingTableBuffer);
    if (miss_shader_buffer_state && missShaderBindingOffset >= miss_shader_buffer_state->create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(missShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02457", objlist,
                         error_obj.location.dot(Field::missShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of missShaderBindingTableBuffer %" PRIu64 " .",
                         missShaderBindingOffset, miss_shader_buffer_state->create_info.size);
    }
    auto raygen_shader_buffer_state = Get<vvl::Buffer>(raygenShaderBindingTableBuffer);
    if (raygenShaderBindingOffset >= raygen_shader_buffer_state->create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(raygenShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02455", objlist,
                         error_obj.location.dot(Field::raygenShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of raygenShaderBindingTableBuffer %" PRIu64 " .",
                         raygenShaderBindingOffset, raygen_shader_buffer_state->create_info.size);
    }
    return skip;
}

bool CoreChecks::ValidateCmdTraceRaysKHR(const Location &loc, const vvl::CommandBuffer &cb_state,
                                         const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable) const {
    bool skip = false;
    const vvl::Pipeline *pipeline_state = cb_state.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    if (!pipeline_state) return skip;  // possible wasn't bound correctly, check caught elsewhere
    const bool is_indirect = loc.function == Func::vkCmdTraceRaysIndirectKHR;

    if (pHitShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pHitShaderBindingTable);
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) {
            if (pHitShaderBindingTable->deviceAddress == 0) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03697" : "VUID-vkCmdTraceRaysKHR-flags-03697";
                skip |= LogError(vuid, cb_state.Handle(), table_loc.dot(Field::deviceAddress), "is zero.");
            }
            if ((pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0)) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03514" : "VUID-vkCmdTraceRaysKHR-flags-03514";
                skip |= LogError(vuid, cb_state.Handle(), table_loc, "either size (%" PRIu64 ") and stride (%" PRIu64 ") is zero.",
                                 pHitShaderBindingTable->size, pHitShaderBindingTable->stride);
            }
        }
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) {
            if (pHitShaderBindingTable->deviceAddress == 0) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03696" : "VUID-vkCmdTraceRaysKHR-flags-03696";
                skip |= LogError(vuid, cb_state.Handle(), table_loc.dot(Field::deviceAddress), "is zero.");
            }
            if ((pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0)) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03513" : "VUID-vkCmdTraceRaysKHR-flags-03513";
                skip |= LogError(vuid, cb_state.Handle(), table_loc, "either size (%" PRIu64 ") and stride (%" PRIu64 ") is zero.",
                                 pHitShaderBindingTable->size, pHitShaderBindingTable->stride);
            }
        }
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) {
            // No vuid to check for pHitShaderBindingTable->deviceAddress == 0 with this flag

            if (pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03512" : "VUID-vkCmdTraceRaysKHR-flags-03512";
                skip |= LogError(vuid, cb_state.Handle(), table_loc, "either size (%" PRIu64 ") and stride (%" PRIu64 ") is zero.",
                                 pHitShaderBindingTable->size, pHitShaderBindingTable->stride);
            }
        }

        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03687"
                                                            : "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03687";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03688"
                                                          : "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03688";
        skip |= ValidateRaytracingShaderBindingTable(cb_state.VkHandle(), table_loc, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pHitShaderBindingTable);
    }

    if (pRaygenShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pRaygenShaderBindingTable);
        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03680"
                                                            : "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03680";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03681"
                                                          : "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03681";
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9368
        // TODO - waiting for https://gitlab.khronos.org/vulkan/vulkan/-/issues/4173
        if (const auto buffers = GetBuffersByAddress(pRaygenShaderBindingTable->deviceAddress); buffers.empty()) {
            skip |= LogError("UNASSIGNED-TraceRays-InvalidRayGenSBTAddress", cb_state.Handle(), table_loc.dot(Field::deviceAddress),
                             "(0x%" PRIx64 ") does not belong to a valid VkBuffer.",
                             pRaygenShaderBindingTable->deviceAddress);
        }
        skip |= ValidateRaytracingShaderBindingTable(cb_state.VkHandle(), table_loc, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pRaygenShaderBindingTable);
    }

    if (pMissShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pMissShaderBindingTable);
        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03683"
                                                            : "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03683";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03684"
                                                          : "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03684";
        skip |= ValidateRaytracingShaderBindingTable(cb_state.VkHandle(), table_loc, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pMissShaderBindingTable);
        if (pMissShaderBindingTable->deviceAddress == 0) {
            if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03511" : "VUID-vkCmdTraceRaysKHR-flags-03511";
                skip |= LogError(vuid, cb_state.Handle(), loc.dot(Field::pMissShaderBindingTable),
                                 "is 0 but last bound ray tracing pipeline (%s) was created with flags (%s).",
                                 FormatHandle(pipeline_state->Handle()).c_str(),
                                 string_VkPipelineCreateFlags2(pipeline_state->create_flags).c_str());
            }
        }
    }

    if (pCallableShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pCallableShaderBindingTable);
        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03691"
                                                            : "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03691";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03692"
                                                          : "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03692";
        skip |= ValidateRaytracingShaderBindingTable(cb_state.VkHandle(), table_loc, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pCallableShaderBindingTable);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                                uint32_t height, uint32_t depth, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    skip |= ValidateCmdTraceRaysKHR(error_obj.location, cb_state, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                    pHitShaderBindingTable, pCallableShaderBindingTable);
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                        const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                        VkDeviceAddress indirectDeviceAddress, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    skip |= ValidateCmdTraceRaysKHR(error_obj.location, cb_state, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                    pHitShaderBindingTable, pCallableShaderBindingTable);
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (taskCount > phys_dev_ext_props.mesh_shader_props_nv.maxDrawMeshTasksCount) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksNV-taskCount-02119", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::taskCount),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesNV::maxDrawMeshTasksCount (0x%" PRIxLEAST32 ").",
            taskCount, phys_dev_ext_props.mesh_shader_props_nv.maxDrawMeshTasksCount);
    }
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, true);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<vvl::Buffer>(buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);

    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02157", stride,
                                                Struct::VkDrawMeshTasksIndirectCommandNV, sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                drawCount, offset, *buffer_state, error_obj.location);
        if (!enabled_features.multiDrawIndirect) {
            skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02718",
                             cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                             "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
        }
        if ((stride & 3) || stride < sizeof(VkDrawMeshTasksIndirectCommandNV)) {
            skip |= LogError(
                "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02146", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                error_obj.location.dot(Field::stride),
                "(0x%" PRIxLEAST32 "), is not a multiple of 4 or smaller than sizeof (VkDrawMeshTasksIndirectCommandNV).", stride);
        }
    } else if (drawCount == 1 && ((offset + sizeof(VkDrawMeshTasksIndirectCommandNV)) > buffer_state.get()->create_info.size)) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(buffer);
        skip |=
            LogError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02156", objlist, error_obj.location,
                     "(offset + sizeof(VkDrawMeshTasksIndirectNV)) (%" PRIu64 ") is greater than the size of buffer (%" PRIu64 ").",
                     offset + sizeof(VkDrawMeshTasksIndirectCommandNV), buffer_state->create_info.size);
    }
    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectNV-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02719",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") is not less than or equal to maxDrawIndirectCount (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, true);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (offset & 3) {
        skip |=
            LogError("VUID-vkCmdDrawMeshTasksIndirectCountNV-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                     error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }
    if (countBufferOffset & 3) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectCountNV-countBufferOffset-02716",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::countBufferOffset),
                         "(%" PRIu64 "), is not a multiple of 4.", countBufferOffset);
    }

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<vvl::Buffer>(buffer);
    auto count_buffer_state = Get<vvl::Buffer>(countBuffer);
    ASSERT_AND_RETURN_SKIP(buffer_state && count_buffer_state);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    skip |= ValidateIndirectCountCmd(cb_state, *count_buffer_state, countBufferOffset, error_obj.location);
    skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountNV-stride-02182", stride,
                                            Struct::VkDrawMeshTasksIndirectCommandNV, sizeof(VkDrawMeshTasksIndirectCommandNV),
                                            error_obj.location);
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountNV-maxDrawCount-02183", stride,
                                                Struct::VkDrawMeshTasksIndirectCommandNV, sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                maxDrawCount, offset, *buffer_state, error_obj.location);
    }
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, true);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                    uint32_t groupCountZ, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, false);

    if (groupCountX > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07322", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::groupCountX),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[0] (0x%" PRIxLEAST32
            ").",
            groupCountX, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]);
    }
    if (groupCountY > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1]) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07323", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::groupCountY),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[1] (0x%" PRIxLEAST32
            ").",
            groupCountY, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1]);
    }
    if (groupCountZ > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2]) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07324", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::groupCountZ),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[2] (0x%" PRIxLEAST32
            ").",
            groupCountZ, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2]);
    }

    uint32_t maxTaskWorkGroupTotalCount = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount;
    uint64_t invocations = static_cast<uint64_t>(groupCountX) * static_cast<uint64_t>(groupCountY);
    // Prevent overflow.
    bool fail = false;
    if (invocations > vvl::MaxTypeValue(maxTaskWorkGroupTotalCount) || invocations > maxTaskWorkGroupTotalCount) {
        fail = true;
    }
    if (!fail) {
        invocations *= static_cast<uint64_t>(groupCountZ);
        if (invocations > vvl::MaxTypeValue(maxTaskWorkGroupTotalCount) || invocations > maxTaskWorkGroupTotalCount) {
            fail = true;
        }
    }
    if (fail) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07325", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location,
            "The product of groupCountX (0x%" PRIxLEAST32 "), groupCountY (0x%" PRIxLEAST32 ") and groupCountZ (0x%" PRIxLEAST32
            ") must be less than or equal to "
            "VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupTotalCount (0x%" PRIxLEAST32 ").",
            groupCountX, groupCountY, groupCountZ, maxTaskWorkGroupTotalCount);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            uint32_t drawCount, uint32_t stride,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<vvl::Buffer>(buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);

    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07088", stride,
                                                Struct::VkDrawMeshTasksIndirectCommandEXT,
                                                sizeof(VkDrawMeshTasksIndirectCommandEXT), error_obj.location);
        skip |= ValidateCmdDrawStrideWithBuffer(
            cb_state, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07090", stride, Struct::VkDrawMeshTasksIndirectCommandEXT,
            sizeof(VkDrawMeshTasksIndirectCommandEXT), drawCount, offset, *buffer_state, error_obj.location);
    }
    if ((drawCount == 1) && (offset + sizeof(VkDrawMeshTasksIndirectCommandEXT)) > buffer_state->create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(buffer);
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07089", objlist, error_obj.location.dot(Field::drawCount),
                         "is 1 and (offset + sizeof(vkCmdDrawMeshTasksIndirectEXT)) (%" PRIu64
                         ") is not less than "
                         "or equal to the size of buffer (%" PRIu64 ").",
                         (offset + sizeof(VkDrawMeshTasksIndirectCommandEXT)), buffer_state->create_info.size);
    }
    // TODO: vkMapMemory() and check the contents of buffer at offset
    // issue #4547 (https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4547)
    if (!enabled_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02718",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02719",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                         "%" PRIu32 ") is not less than or equal to maxDrawIndirectCount (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, false);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride, const ErrorObject &error_obj) const {
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    bool skip = false;
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<vvl::Buffer>(buffer);
    auto count_buffer_state = Get<vvl::Buffer>(countBuffer);
    ASSERT_AND_RETURN_SKIP(buffer_state && count_buffer_state);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *count_buffer_state, error_obj.location.dot(Field::countBuffer),
                                          vuid.indirect_count_contiguous_memory_02714);
    skip |= ValidateBufferUsageFlags(LogObjectList(commandBuffer, countBuffer), *count_buffer_state,
                                     VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true, vuid.indirect_count_buffer_bit_02715,
                                     error_obj.location.dot(Field::countBuffer));
    skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stride-07096", stride,
                                            Struct::VkDrawMeshTasksIndirectCommandEXT, sizeof(VkDrawMeshTasksIndirectCommandEXT),
                                            error_obj.location);
    if (maxDrawCount > 1) {
        skip |=
            ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-maxDrawCount-07097", stride,
                                            Struct::VkDrawMeshTasksIndirectCommandEXT, sizeof(VkDrawMeshTasksIndirectCommandEXT),
                                            maxDrawCount, offset, *buffer_state, error_obj.location);
    }
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, false);
    return skip;
}

// Action command == vkCmdDraw*, vkCmdDispatch*, vkCmdTraceRays*
// This is the main logic shared by all action commands
bool CoreChecks::ValidateActionState(const vvl::CommandBuffer &cb_state, const VkPipelineBindPoint bind_point,
                                     const Location &loc) const {
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const auto &last_bound_state = cb_state.lastBound[lv_bind_point];
    const vvl::Pipeline *pipeline = last_bound_state.pipeline_state;

    bool skip = false;

    // Quick verify that if there is no pipeine, the shade object is being used
    if (!pipeline && !enabled_features.shaderObject) {
        return LogError(vuid.pipeline_bound_08606, cb_state.GetObjectList(bind_point), loc,
                        "A valid %s pipeline must be bound with vkCmdBindPipeline before calling this command.",
                        string_VkPipelineBindPoint(bind_point));
    }

    if (!pipeline) {
        skip |= ValidateShaderObjectBoundShader(last_bound_state, bind_point, vuid);
        if (skip) return skip;  // if shaders are bound wrong, likely to give false positives after
    }

    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        skip |= ValidateDrawDynamicState(last_bound_state, vuid);
        skip |= ValidateDrawPrimitivesGeneratedQuery(last_bound_state, vuid);
        skip |= ValidateDrawProtectedMemory(last_bound_state, vuid);
        skip |= ValidateDrawDualSourceBlend(last_bound_state, vuid);

        if (pipeline) {
            skip |= ValidateDrawPipeline(last_bound_state, *pipeline, vuid);
        } else {
            skip |= ValidateDrawShaderObject(last_bound_state, vuid);
        }

    } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        if (pipeline) {
            skip |= ValidateTraceRaysDynamicStateSetStatus(last_bound_state, *pipeline, vuid);
        }
        if (!cb_state.unprotected) {
            skip |= LogError(vuid.ray_query_protected_cb_03635, cb_state.GetObjectList(bind_point), loc,
                             "called in a protected command buffer.");
        }
    }

    if (pipeline) {
        skip |= ValidateActionStateDescriptorsPipeline(last_bound_state, bind_point, *pipeline, vuid);
    } else if (last_bound_state.cb_state.descriptor_buffer_binding_info.empty()) {
        // TODO - VkPipeline have VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT (descriptor_buffer_mode) to know if using descriptor
        // buffers, but VK_EXT_shader_object has no flag. For now, if the command buffer ever calls vkCmdBindDescriptorBuffersEXT,
        // we just assume things are bound until we add some form of GPU side tracking for descriptor buffers
        skip |= ValidateActionStateDescriptorsShaderObject(last_bound_state, bind_point, vuid);
    }

    skip |= ValidateActionStatePushConstant(last_bound_state, pipeline, vuid);

    if (!cb_state.unprotected) {
        skip |= ValidateActionStateProtectedMemory(last_bound_state, bind_point, pipeline, vuid);
    }

    return skip;
}

// Validate the draw-time state for this descriptor set
// We can skip validating the descriptor set if "nothing" has changed since the last validation.
// Same set, no image layout changes, and same "pipeline state" (binding_req_map). If there are
// any dynamic descriptors, always revalidate rather than caching the values. We currently only
// apply this optimization if IsManyDescriptors is true, to avoid the overhead of copying the
// binding_req_map which could potentially be expensive.
static bool NeedDrawStateValidated(const vvl::CommandBuffer &cb_state, const vvl::DescriptorSet *descriptor_set,
                                   const LastBound::DescriptorSetSlot &ds_slot, bool disabled_image_layout_validation) {
    return ds_slot.dynamic_offsets.size() > 0 ||
           // Revalidate if descriptor set (or contents) has changed
           ds_slot.validated_set != descriptor_set || ds_slot.validated_set_change_count != descriptor_set->GetChangeCount() ||
           (!disabled_image_layout_validation &&
            ds_slot.validated_set_image_layout_change_count != cb_state.image_layout_change_count);
}

bool CoreChecks::ValidateActionStateDescriptorsPipeline(const LastBound &last_bound_state, const VkPipelineBindPoint bind_point,
                                                        const vvl::Pipeline &pipeline, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    for (const auto &ds_slot : last_bound_state.ds_slots) {
        // TODO - This currently implicitly is checking for VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT being set
        if (pipeline.descriptor_buffer_mode) {
            if (ds_slot.ds_state && !ds_slot.ds_state->IsPushDescriptor()) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle(), ds_slot.ds_state->Handle());
                skip |= LogError(vuid.descriptor_buffer_bit_not_set_08115, objlist, vuid.loc(),
                                 "pipeline bound to %s requires a descriptor buffer (because it was created with "
                                 "VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT), but has a bound VkDescriptorSet (%s)",
                                 string_VkPipelineBindPoint(bind_point), FormatHandle(ds_slot.ds_state->Handle()).c_str());
                break;
            }

        } else if (ds_slot.descriptor_buffer_binding.has_value()) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(vuid.descriptor_buffer_set_offset_missing_08117, objlist, vuid.loc(),
                             "pipeline bound to %s requires a VkDescriptorSet (because it was not created with "
                             "VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT), but has a bound descriptor buffer"
                             " (index=%" PRIu32 " offset=%" PRIu64 ")",
                             string_VkPipelineBindPoint(bind_point), ds_slot.descriptor_buffer_binding->index,
                             ds_slot.descriptor_buffer_binding->offset);
            break;
        }
    }

    // Check if the current pipeline is compatible for the maximum used set with the bound sets.
    if (pipeline.descriptor_buffer_mode) return skip;

    const auto pipeline_layout = pipeline.PipelineLayoutState();
    if (!pipeline.active_slots.empty() && !last_bound_state.IsBoundSetCompatible(pipeline.max_active_slot, *pipeline_layout)) {
        LogObjectList objlist(pipeline.Handle());
        const auto layouts = pipeline.PipelineLayoutStateUnion();
        std::ostringstream pipe_layouts_log;
        if (layouts.size() > 1) {
            pipe_layouts_log << "a union of layouts [ ";
            for (const auto &layout : layouts) {
                objlist.add(layout->Handle());
                pipe_layouts_log << FormatHandle(*layout) << " ";
            }
            pipe_layouts_log << "]";
        } else {
            pipe_layouts_log << FormatHandle(*layouts.front());
        }
        objlist.add(last_bound_state.desc_set_pipeline_layout);
        std::string range =
            pipeline.max_active_slot == 0 ? "set 0 is" : "all sets 0 to " + std::to_string(pipeline.max_active_slot) + " are";
        skip |= LogError(vuid.compatible_pipeline_08600, objlist, vuid.loc(),
                         "The %s (created with %s) statically uses descriptor set %" PRIu32
                         ", but %s not compatible with the pipeline layout bound with %s (%s)\n%s",
                         FormatHandle(pipeline).c_str(), pipe_layouts_log.str().c_str(), pipeline.max_active_slot, range.c_str(),
                         String(last_bound_state.desc_set_bound_command),
                         FormatHandle(last_bound_state.desc_set_pipeline_layout).c_str(),
                         last_bound_state.DescribeNonCompatibleSet(pipeline.max_active_slot, *pipeline_layout).c_str());
    } else {
        // if the bound set is not compatible, the rest will just be extra redundant errors
        for (const auto &[set_index, binding_req_map] : pipeline.active_slots) {
            std::string error_string;
            const auto ds_slot = last_bound_state.ds_slots[set_index];
            if (!ds_slot.ds_state) {
                skip |= LogError(vuid.compatible_pipeline_08600, cb_state.GetObjectList(bind_point), vuid.loc(),
                                 "%s uses set #%" PRIu32
                                 " but that set is not bound. (Need to use a command like vkCmdBindDescriptorSets to bind the set)",
                                 FormatHandle(pipeline).c_str(), set_index);
            } else if (!VerifySetLayoutCompatibility(*ds_slot.ds_state, pipeline_layout->set_layouts, pipeline_layout->Handle(),
                                                     set_index, error_string)) {
                // Set is bound but not compatible w/ overlapping pipeline_layout from PSO
                VkDescriptorSet set_handle = ds_slot.ds_state->VkHandle();
                LogObjectList objlist = cb_state.GetObjectList(bind_point);
                objlist.add(set_handle);
                objlist.add(pipeline_layout->Handle());
                skip |= LogError(vuid.compatible_pipeline_08600, objlist, vuid.loc(),
                                 "%s bound as set #%" PRIu32 " is not compatible with overlapping %s due to: %s",
                                 FormatHandle(set_handle).c_str(), set_index, FormatHandle(*pipeline_layout).c_str(),
                                 error_string.c_str());
            } else {  // Valid set is bound and layout compatible, validate that it's updated
                // Pull the set node
                const auto *descriptor_set = ds_slot.ds_state.get();
                ASSERT_AND_CONTINUE(descriptor_set);

                const bool need_validate =
                    NeedDrawStateValidated(cb_state, descriptor_set, ds_slot, disabled[image_layout_validation]);
                if (need_validate) {
                    skip |= ValidateDrawState(*descriptor_set, set_index, binding_req_map, cb_state, vuid.loc(), vuid);
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateActionStateDescriptorsShaderObject(const LastBound &last_bound_state, const VkPipelineBindPoint bind_point,
                                                            const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    // Check if the current shader objects are compatible for the maximum used set with the bound sets.
    for (const auto &shader_state : last_bound_state.shader_object_states) {
        if (!shader_state) continue;

        if (shader_state && !shader_state->active_slots.empty() &&
            !last_bound_state.IsBoundSetCompatible(shader_state->max_active_slot, *shader_state)) {
            LogObjectList objlist(cb_state.Handle(), shader_state->Handle());
            skip |= LogError(vuid.compatible_pipeline_08600, objlist, vuid.loc(),
                             "The %s statically uses descriptor set (index #%" PRIu32
                             ") which is not compatible with the currently bound descriptor set's layout\n%s",
                             FormatHandle(shader_state->Handle()).c_str(), shader_state->max_active_slot,
                             last_bound_state.DescribeNonCompatibleSet(shader_state->max_active_slot, *shader_state).c_str());
        } else {
            // if the bound set is not copmatible, the rest will just be extra redundant errors
            for (const auto &[set_index, binding_req_map] : shader_state->active_slots) {
                std::string error_string;
                const auto ds_slot = last_bound_state.ds_slots[set_index];
                if (!ds_slot.ds_state) {
                    const LogObjectList objlist(cb_state.Handle(), shader_state->Handle());
                    skip |= LogError(vuid.compatible_pipeline_08600, objlist, vuid.loc(),
                                     "%s uses set #%" PRIu32 " but that set is not bound.",
                                     FormatHandle(shader_state->Handle()).c_str(), set_index);
                } else if (!VerifySetLayoutCompatibility(*ds_slot.ds_state, shader_state->set_layouts, shader_state->Handle(),
                                                         set_index, error_string)) {
                    // Set is bound but not compatible w/ overlapping pipeline_layout from PSO
                    VkDescriptorSet set_handle = ds_slot.ds_state->VkHandle();
                    const LogObjectList objlist(cb_state.Handle(), set_handle, shader_state->Handle());
                    skip |= LogError(vuid.compatible_pipeline_08600, objlist, vuid.loc(),
                                     "%s bound as set #%" PRIu32 " is not compatible with overlapping %s due to: %s",
                                     FormatHandle(set_handle).c_str(), set_index, FormatHandle(shader_state->Handle()).c_str(),
                                     error_string.c_str());
                } else {  // Valid set is bound and layout compatible, validate that it's updated
                    // Pull the set node
                    const auto *descriptor_set = ds_slot.ds_state.get();
                    ASSERT_AND_CONTINUE(descriptor_set);

                    const bool need_validate =
                        NeedDrawStateValidated(cb_state, descriptor_set, ds_slot, disabled[image_layout_validation]);
                    if (need_validate) {
                        skip |= ValidateDrawState(*descriptor_set, set_index, binding_req_map, cb_state, vuid.loc(), vuid);
                    }
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateActionStatePushConstant(const LastBound &last_bound_state, const vvl::Pipeline *pipeline,
                                                 const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    // Verify if push constants have been set
    // NOTE: Currently not checking whether active push constants are compatible with the active pipeline, nor whether the
    //       "life times" of push constants are correct.
    //       Discussion on validity of these checks can be found at https://gitlab.khronos.org/vulkan/vulkan/-/issues/2602.
    if (pipeline) {
        auto const &pipeline_layout = pipeline->PipelineLayoutState();
        if (!cb_state.push_constant_ranges_layout ||
            (pipeline_layout->push_constant_ranges_layout == cb_state.push_constant_ranges_layout)) {
            for (const auto &stage : pipeline->stage_states) {
                if (!stage.entrypoint || !stage.entrypoint->push_constant_variable) {
                    continue;  // no static push constant in shader
                }

                // Edge case where if the shader is using push constants statically and there never was a vkCmdPushConstants
                if (!cb_state.push_constant_ranges_layout && !enabled_features.maintenance4) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline_layout->Handle(), pipeline->Handle());
                    skip |= LogError(vuid.push_constants_set_08602, objlist, vuid.loc(),
                                     "Shader in %s uses push-constant statically but vkCmdPushConstants was not called yet for "
                                     "pipeline layout %s.",
                                     string_VkShaderStageFlags(stage.GetStage()).c_str(),
                                     FormatHandle(pipeline_layout->Handle()).c_str());
                }
            }
        }
    } else {
        if (!cb_state.push_constant_ranges_layout) {
            for (const auto &stage : last_bound_state.shader_object_states) {
                if (!stage || !stage->entrypoint || !stage->entrypoint->push_constant_variable) {
                    continue;
                }
                // Edge case where if the shader is using push constants statically and there never was a vkCmdPushConstants
                if (!cb_state.push_constant_ranges_layout && !enabled_features.maintenance4) {
                    const LogObjectList objlist(cb_state.Handle(), stage->Handle());
                    skip |= LogError(vuid.push_constants_set_08602, objlist, vuid.loc(),
                                     "Shader in %s uses push-constant statically but vkCmdPushConstants was not called yet.",
                                     string_VkShaderStageFlags(stage->create_info.stage).c_str());
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateActionStateProtectedMemory(const LastBound &last_bound_state, const VkPipelineBindPoint bind_point,
                                                    const vvl::Pipeline *pipeline, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;

    if (pipeline) {
        for (const auto &stage : pipeline->stage_states) {
            // Stage may not have SPIR-V data (e.g. due to the use of shader module identifier or in Vulkan SC)
            if (!stage.spirv_state) continue;

            if (stage.spirv_state->HasCapability(spv::CapabilityRayQueryKHR)) {
                skip |= LogError(vuid.ray_query_04617, last_bound_state.cb_state.GetObjectList(bind_point), vuid.loc(),
                                 "Shader in %s uses OpCapability RayQueryKHR but the command buffer is protected.",
                                 string_VkShaderStageFlags(stage.GetStage()).c_str());
            }
        }
    } else {
        for (const auto &stage : last_bound_state.shader_object_states) {
            if (stage && stage->spirv->HasCapability(spv::CapabilityRayQueryKHR)) {
                skip |= LogError(vuid.ray_query_04617, last_bound_state.cb_state.GetObjectList(bind_point), vuid.loc(),
                                 "Shader in %s uses OpCapability RayQueryKHR but the command buffer is protected.",
                                 string_VkShaderStageFlags(stage->create_info.stage).c_str());
            }
        }
    }
    return skip;
}

bool CoreChecks::MatchSampleLocationsInfo(const VkSampleLocationsInfoEXT &info_1, const VkSampleLocationsInfoEXT &info_2) const {
    if (info_1.sampleLocationsPerPixel != info_2.sampleLocationsPerPixel ||
        info_1.sampleLocationGridSize.width != info_2.sampleLocationGridSize.width ||
        info_1.sampleLocationGridSize.height != info_2.sampleLocationGridSize.height ||
        info_1.sampleLocationsCount != info_2.sampleLocationsCount) {
        return false;
    }
    for (uint32_t i = 0; i < info_1.sampleLocationsCount; ++i) {
        if (info_1.pSampleLocations[i].x != info_2.pSampleLocations[i].x ||
            info_1.pSampleLocations[i].y != info_2.pSampleLocations[i].y) {
            return false;
        }
    }
    return true;
}

bool CoreChecks::ValidateIndirectCmd(const vvl::CommandBuffer &cb_state, const vvl::Buffer &buffer_state,
                                     const Location &loc) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);
    LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
    objlist.add(buffer_state.Handle());

    skip |= ValidateMemoryIsBoundToBuffer(cb_state.VkHandle(), buffer_state, loc.dot(Field::buffer),
                                          vuid.indirect_contiguous_memory_02708);
    skip |= ValidateBufferUsageFlags(objlist, buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_buffer_bit_02290, loc.dot(Field::buffer));
    if (cb_state.unprotected == false) {
        skip |= LogError(vuid.indirect_protected_cb_02711, objlist, loc,
                         "Indirect commands can't be used in protected command buffers.");
    }
    return skip;
}

bool CoreChecks::ValidateIndirectCountCmd(const vvl::CommandBuffer &cb_state, const vvl::Buffer &count_buffer_state,
                                          VkDeviceSize count_buffer_offset, const Location &loc) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);
    LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
    objlist.add(count_buffer_state.Handle());

    skip |= ValidateMemoryIsBoundToBuffer(cb_state.VkHandle(), count_buffer_state, loc.dot(Field::countBuffer),
                                          vuid.indirect_count_contiguous_memory_02714);
    skip |= ValidateBufferUsageFlags(objlist, count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_count_buffer_bit_02715, loc.dot(Field::countBuffer));
    if (count_buffer_offset + sizeof(uint32_t) > count_buffer_state.create_info.size) {
        skip |= LogError(vuid.indirect_count_offset_04129, objlist, loc,
                         "countBufferOffset (%" PRIu64 ") + sizeof(uint32_t) is greater than the buffer size of %" PRIu64 ".",
                         count_buffer_offset, count_buffer_state.create_info.size);
    }
    return skip;
}

bool CoreChecks::ValidateDrawPrimitivesGeneratedQuery(const LastBound &last_bound_state, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    const bool with_rasterizer_discard = enabled_features.primitivesGeneratedQueryWithRasterizerDiscard == VK_TRUE;
    const bool with_non_zero_streams = enabled_features.primitivesGeneratedQueryWithNonZeroStreams == VK_TRUE;

    if (with_rasterizer_discard && with_non_zero_streams) {
        return skip;
    }

    bool primitives_generated_query = false;
    for (const auto &query : cb_state.activeQueries) {
        auto query_pool_state = Get<vvl::QueryPool>(query.pool);
        if (query_pool_state && query_pool_state->create_info.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
            primitives_generated_query = true;
            break;
        }
    }

    if (primitives_generated_query) {
        if (!with_rasterizer_discard && last_bound_state.IsRasterizationDisabled()) {
            skip |= LogError(vuid.primitives_generated_06708, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), vuid.loc(),
                             "a VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT query is active and pipeline was created with "
                             "VkPipelineRasterizationStateCreateInfo::rasterizerDiscardEnable set to VK_TRUE, but "
                             "primitivesGeneratedQueryWithRasterizerDiscard feature is not enabled.");
        }
        const vvl::Pipeline *pipeline = last_bound_state.pipeline_state;
        if (!with_non_zero_streams && pipeline) {
            const auto rasterization_state_stream_ci =
                vku::FindStructInPNextChain<VkPipelineRasterizationStateStreamCreateInfoEXT>(pipeline->RasterizationStatePNext());
            if (rasterization_state_stream_ci && rasterization_state_stream_ci->rasterizationStream != 0) {
                skip |= LogError(vuid.primitives_generated_streams_06709, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                                 vuid.loc(),
                                 "a VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT query is active and pipeline was created with "
                                 "VkPipelineRasterizationStateStreamCreateInfoEXT::rasterizationStream set to %" PRIu32
                                 ", but primitivesGeneratedQueryWithNonZeroStreams feature is not enabled.",
                                 rasterization_state_stream_ci->rasterizationStream);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawProtectedMemory(const LastBound &last_bound_state, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;

    if (!enabled_features.protectedMemory) {
        return skip;
    }

    // Verify vertex & index buffer for unprotected command buffer.
    // Because vertex & index buffer is read only, it doesn't need to care protected command buffer case.
    for (const auto &vertex_buffer_binding : cb_state.current_vertex_buffer_binding_info) {
        if (const auto buffer_state = Get<vvl::Buffer>(vertex_buffer_binding.second.buffer)) {
            skip |= ValidateProtectedBuffer(cb_state, *buffer_state, vuid.loc(), vuid.unprotected_command_buffer_02707,
                                            " (Buffer is the vertex buffer)");
        }
    }

    if (const auto buffer_state = Get<vvl::Buffer>(cb_state.index_buffer_binding.buffer)) {
        skip |= ValidateProtectedBuffer(cb_state, *buffer_state, vuid.loc(), vuid.unprotected_command_buffer_02707,
                                        " (Buffer is the index buffer)");
    }

    return skip;
}

bool CoreChecks::ValidateDrawDualSourceBlend(const LastBound &last_bound_state, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    const auto *pipeline = last_bound_state.pipeline_state;
    if (pipeline && !pipeline->ColorBlendState()) return skip;

    const spirv::EntryPoint *fragment_entry_point = last_bound_state.GetFragmentEntryPoint();
    if (!fragment_entry_point) return skip;

    uint32_t max_fragment_location = 0;
    for (const auto *variable : fragment_entry_point->user_defined_interface_variables) {
        if (variable->storage_class != spv::StorageClassOutput) continue;
        if (variable->decorations.location != spirv::kInvalidValue) {
            max_fragment_location = std::max(max_fragment_location, variable->decorations.location);
        }
    }
    if (max_fragment_location < phys_dev_props.limits.maxFragmentDualSrcAttachments) return skip;

    // If color blend is disabled, the blend equation doesn't matter
    const bool dynamic_blend_enable = !pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    const bool dynamic_blend_equation = !pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    const uint32_t attachment_count = pipeline ? pipeline->ColorBlendState()->attachmentCount
                                               : (uint32_t)cb_state.dynamic_state_value.color_blend_equations.size();
    for (uint32_t i = 0; i < attachment_count; ++i) {
        const bool blend_enable = dynamic_blend_enable ? cb_state.dynamic_state_value.color_blend_enabled[i]
                                                       : pipeline->ColorBlendState()->pAttachments[i].blendEnable;
        if (!blend_enable) continue;
        if (dynamic_blend_equation) {
            const VkColorBlendEquationEXT &color_blend_equation = cb_state.dynamic_state_value.color_blend_equations[i];
            if (IsSecondaryColorInputBlendFactor(color_blend_equation.srcColorBlendFactor) ||
                IsSecondaryColorInputBlendFactor(color_blend_equation.dstColorBlendFactor) ||
                IsSecondaryColorInputBlendFactor(color_blend_equation.srcAlphaBlendFactor) ||
                IsSecondaryColorInputBlendFactor(color_blend_equation.dstAlphaBlendFactor)) {
                const LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
                skip |= LogError(vuid.blend_dual_source_09239, objlist, vuid.loc(),
                                 "Fragment output attachment %" PRIu32
                                 " is using Dual-Source Blending, but the largest output fragment Location (%" PRIu32
                                 ") is not less than maxFragmentDualSrcAttachments (%" PRIu32
                                 "). The following are set by vkCmdSetColorBlendEquationEXT:\n\tsrcColorBlendFactor = "
                                 "%s\n\tdstColorBlendFactor = %s\n\tsrcAlphaBlendFactor = "
                                 "%s\n\tdstAlphaBlendFactor = %s\n",
                                 i, max_fragment_location, phys_dev_props.limits.maxFragmentDualSrcAttachments,
                                 string_VkBlendFactor(color_blend_equation.srcColorBlendFactor),
                                 string_VkBlendFactor(color_blend_equation.dstColorBlendFactor),
                                 string_VkBlendFactor(color_blend_equation.srcAlphaBlendFactor),
                                 string_VkBlendFactor(color_blend_equation.dstAlphaBlendFactor));
                break;
            }
        } else {
            const VkPipelineColorBlendAttachmentState &attachment = pipeline->ColorBlendState()->pAttachments[i];
            if (IsSecondaryColorInputBlendFactor(attachment.srcColorBlendFactor) ||
                IsSecondaryColorInputBlendFactor(attachment.dstColorBlendFactor) ||
                IsSecondaryColorInputBlendFactor(attachment.srcAlphaBlendFactor) ||
                IsSecondaryColorInputBlendFactor(attachment.dstAlphaBlendFactor)) {
                const LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
                skip |= LogError(
                    vuid.blend_dual_source_09239, objlist, vuid.loc(),
                    "Fragment output attachment %" PRIu32
                    " is using Dual-Source Blending, but the largest output fragment Location (%" PRIu32
                    ") is not less than maxFragmentDualSrcAttachments (%" PRIu32
                    "). The following are set by VkPipelineColorBlendAttachmentState:\n\tsrcColorBlendFactor = "
                    "%s\n\tdstColorBlendFactor = %s\n\tsrcAlphaBlendFactor = %s\n\tdstAlphaBlendFactor "
                    "= %s\n",
                    i, max_fragment_location, phys_dev_props.limits.maxFragmentDualSrcAttachments,
                    string_VkBlendFactor(attachment.srcColorBlendFactor), string_VkBlendFactor(attachment.dstColorBlendFactor),
                    string_VkBlendFactor(attachment.srcAlphaBlendFactor), string_VkBlendFactor(attachment.dstAlphaBlendFactor));
                break;
            }
        }
    }

    return skip;
}
