/* Copyright (c) 2021-2025 The Khronos Group Inc.
 * Copyright (c) 2021-2025 Valve Corporation
 * Copyright (c) 2021-2025 LunarG, Inc.
 * Copyright (C) 2021-2025 Google Inc.
 * Modifications Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
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
#include "sync/sync_vuid_maps.h"
#include "error_message/error_location.h"

#include <cassert>

namespace sync_vuid_maps {
using vvl::Entry;
using vvl::Field;
using vvl::FindVUID;
using vvl::Func;
using vvl::Key;
using vvl::Struct;

const vvl::unordered_map<VkPipelineStageFlags2, std::string> &GetFeatureNameMap() {
    static const vvl::unordered_map<VkPipelineStageFlags2, std::string> feature_name_map{
        {VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, "geometryShader"},
        {VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, "tessellationShader"},
        {VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, "tessellationShader"},
        {VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT, "conditionalRendering"},
        {VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT, "fragmentDensity"},
        {VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, "transformFeedback"},
        {VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT, "meshShader"},
        {VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT, "taskShader"},
        {VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, "shadingRate"},
        {VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, "rayTracing"},
        {VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, "rayTracing"},
        {VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI, "subpassShading"},
        {VK_PIPELINE_STAGE_2_INVOCATION_MASK_BIT_HUAWEI, "invocationMask"},
    };
    return feature_name_map;
}
// commonvalidity/pipeline_stage_common.txt
// commonvalidity/stage_mask_2_common.txt
// commonvalidity/stage_mask_common.txt
static const vvl::unordered_map<VkPipelineStageFlags2, std::vector<Entry>> &GetStageMaskErrorsMap() {
    static const vvl::unordered_map<VkPipelineStageFlags2, std::vector<Entry>> stage_mask_errors{
        {VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT,
         std::vector<Entry>{
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03931"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03931"},
             {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04092"},
             {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04092"},
             {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03931"},
             {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04092"},
             {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04092"},
             {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04092"},
             {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04092"},
             {Key(Func::vkCmdWriteBufferMarkerAMD, Field::stage), "VUID-vkCmdWriteBufferMarkerAMD-pipelineStage-04077"},
             {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-03931"},
             {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03931"},
             {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04077"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03931"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03931"},
             {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03931"},
             {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03931"},
             {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03931"},
             {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04092"},
             {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04092"},
             {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04092"},
             {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04092"},
             {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04092"},
         }},
        {VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
         std::vector<Entry>{
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03932"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03932"},
             {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04093"},
             {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04093"},
             {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03932"},
             {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04093"},
             {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04093"},
             {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04093"},
             {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04093"},
             {Key(Func::vkCmdWriteBufferMarkerAMD, Field::stage), "VUID-vkCmdWriteBufferMarkerAMD-pipelineStage-04078"},
             {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-03932"},
             {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03932"},
             {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04078"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03932"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03932"},
             {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03932"},
             {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03932"},
             {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03932"},
             {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04093"},
             {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04093"},
             {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04093"},
             {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04093"},
             {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04093"},
         }},
        {VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,
         std::vector<Entry>{
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03929"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03929"},
             {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04090"},
             {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04090"},
             {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03929"},
             {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04090"},
             {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04090"},
             {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04090"},
             {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04090"},
             {Key(Func::vkCmdWriteBufferMarkerAMD, Field::stage), "VUID-vkCmdWriteBufferMarkerAMD-pipelineStage-04075"},
             {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-03929"},
             {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03929"},
             {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04075"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03929"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03929"},
             {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03929"},
             {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03929"},
             {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03929"},
             {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04090"},
             {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04090"},
             {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04090"},
             {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04090"},
             {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04090"},
         }},
        {VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT,
         std::vector<Entry>{
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03934"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03934"},
             {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04095"},
             {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04095"},
             {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03934"},
             {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04095"},
             {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04095"},
             {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04095"},
             {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04095"},
             {Key(Func::vkCmdWriteBufferMarkerAMD, Field::stage), "VUID-vkCmdWriteBufferMarkerAMD-pipelineStage-04080"},
             {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-03934"},
             {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03934"},
             {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04080"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03934"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03934"},
             {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03934"},
             {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03934"},
             {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03934"},
             {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04095"},
             {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04095"},
             {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04095"},
             {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04095"},
             {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04095"},
         }},
        {VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT,
         std::vector<Entry>{
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03935"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03935"},
             {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04096"},
             {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04096"},
             {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03935"},
             {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04096"},
             {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04096"},
             {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04096"},
             {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04096"},
             {Key(Func::vkCmdWriteBufferMarkerAMD, Field::stage), "VUID-vkCmdWriteBufferMarkerAMD-pipelineStage-07077"},
             {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-03935"},
             {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03935"},
             {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-07077"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03935"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03935"},
             {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03935"},
             {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03935"},
             {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03935"},
             {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04096"},
             {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04096"},
             {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04096"},
             {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04096"},
             {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04096"},
         }},
        {VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT,
         std::vector<Entry>{
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03930"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03930"},
             {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04091"},
             {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04091"},
             {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03930"},
             {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04091"},
             {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04091"},
             {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04091"},
             {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04091"},
             {Key(Func::vkCmdWriteBufferMarkerAMD, Field::stage), "VUID-vkCmdWriteBufferMarkerAMD-pipelineStage-04076"},
             {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-03930"},
             {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03930"},
             {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04076"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03930"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03930"},
             {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03930"},
             {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03930"},
             {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03930"},
             {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04091"},
             {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04091"},
             {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04091"},
             {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04091"},
             {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04091"},
         }},
        {VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT,
         std::vector<Entry>{
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03930"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03930"},
             {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04091"},
             {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04091"},
             {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03930"},
             {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04091"},
             {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04091"},
             {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04091"},
             {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04091"},
             {Key(Func::vkCmdWriteBufferMarkerAMD, Field::stage), "VUID-vkCmdWriteBufferMarkerAMD-pipelineStage-04076"},
             {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-03930"},
             {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03930"},
             {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04076"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03930"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03930"},
             {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03930"},
             {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03930"},
             {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03930"},
             {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04091"},
             {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04091"},
             {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04091"},
             {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04091"},
             {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04091"},
         }},
        {VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT,
         std::vector<Entry>{
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03933"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03933"},
             {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04094"},
             {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04094"},
             {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03933"},
             {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04094"},
             {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04094"},
             {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04094"},
             {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04094"},
             {Key(Func::vkCmdWriteBufferMarkerAMD, Field::stage), "VUID-vkCmdWriteBufferMarkerAMD-pipelineStage-04079"},
             {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-03933"},
             {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03933"},
             {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04079"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03933"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03933"},
             {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03933"},
             {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03933"},
             {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03933"},
             {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04094"},
             {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04094"},
             {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04094"},
             {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04094"},
             {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04094"},
         }},
        {VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
         std::vector<Entry>{
             {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-07949"},
             {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-07949"},
             {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-07949"},
             {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-07949"},
             {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-07949"},
             {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-07949"},
             {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-07949"},
             {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-07949"},
             {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-07949"},
             {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-07949"},
             {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-07949"},
             {Key(Func::vkCmdWriteTimestamp, Field::rayTracingPipeline), "VUID-vkCmdWriteTimestamp-rayTracingPipeline-07943"},
             {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-07946"},
             {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-07946"},
             {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-07946"},
             {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-07946"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-07946"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-07946"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-07946"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-07946"},
             {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-07946"},
             {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-07946"},
         }},
    };
    return stage_mask_errors;
}

const auto &GetStageMaskErrorsNone() {
    static const std::array<Entry, 12> kStageMaskErrorsNone{{
        {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-03937"},
        {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-03937"},
        {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-03937"},
        {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-03937"},
        {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-03937"},
        {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-03937"},
        {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-synchronization2-06489"},
        {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-03937"},
        {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-03937"},
        {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-03937"},
        {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-03937"},
        {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-03937"},
    }};
    return kStageMaskErrorsNone;
}

const auto &GetStageMaskErrorsShadingRate() {
    static const std::array<Entry, 22> kStageMaskErrorsShadingRate{{
        {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-07316"},
        {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-07316"},
        {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-07318"},
        {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-07318"},
        {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-07316"},
        {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-07318"},
        {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-07318"},
        {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-07318"},
        {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-07318"},
        {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-07316"},
        {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-07316"},
        {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-shadingRateImage-07314"},
        {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-07316"},
        {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-07316"},
        {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-07316"},
        {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-07316"},
        {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-07316"},
        {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-07318"},
        {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-07318"},
        {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-07318"},
        {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-07318"},
        {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-07318"},
    }};
    return kStageMaskErrorsShadingRate;
}

const auto &GetStageMaskErrorsSubpassShader() {
    static const std::array<Entry, 10> kStageMaskErrorsSubpassShader{{
        {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-04957"},
        {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-04957"},
        {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-04957"},
        {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-04957"},
        {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-04957"},
        {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-04957"},
        {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-04957"},
        {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-04957"},
        {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-04957"},
        {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-04957"},
    }};
    return kStageMaskErrorsSubpassShader;
}

const auto &GetStageMaskErrorsInvocationMask() {
    static const std::array<Entry, 10> kStageMaskErrorsInvocationMask{{
        {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-04995"},
        {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-04995"},
        {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-04995"},
        {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-04995"},
        {Key(Func::vkCmdWriteBufferMarker2AMD, Field::stage), "VUID-vkCmdWriteBufferMarker2AMD-stage-04995"},
        {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-04995"},
        {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-04995"},
        {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-04995"},
        {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-04995"},
        {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-04995"},
    }};
    return kStageMaskErrorsInvocationMask;
}

const std::string &GetBadFeatureVUID(const Location &loc, VkPipelineStageFlags2 bit, const DeviceExtensions &device_extensions) {
    // special case for stages that require an extension or feature bit,
    // this is checked in DisabledPipelineStages
    if (bit == VK_PIPELINE_STAGE_2_NONE) {
        return FindVUID(loc, GetStageMaskErrorsNone());
    } else if (bit == VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) {
        return FindVUID(loc, GetStageMaskErrorsShadingRate());
    } else if (bit == VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI) {
        return FindVUID(loc, GetStageMaskErrorsSubpassShader());
    } else if (bit == VK_PIPELINE_STAGE_2_INVOCATION_MASK_BIT_HUAWEI) {
        return FindVUID(loc, GetStageMaskErrorsInvocationMask());
    }

    const auto &result = FindVUID(bit, loc, GetStageMaskErrorsMap());
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandle-pipeline-stage-feature");
        return unhandled;
    }
    return result;
}

// commonvalidity/access_mask_2_common.txt
static const vvl::unordered_map<VkAccessFlags2, std::array<Entry, 6>> &GetAccessMask2CommonMap() {
    using ValueType = std::array<Entry, 6>;
    static const vvl::unordered_map<VkAccessFlags2, ValueType> access_mask2_common{
        {VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03900"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03900"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03900"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03900"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03900"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03900"},
         }}},
        {VK_ACCESS_2_INDEX_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03901"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03901"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03901"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03901"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03901"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03901"},
         }}},
        {VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03902"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03902"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03902"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03902"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03902"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03902"},
         }}},
        {VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03903"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03903"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03903"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03903"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03903"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03903"},
         }}},
        {VK_ACCESS_2_UNIFORM_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03904"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03904"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03904"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03904"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03904"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03904"},
         }}},
        {VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03905"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03905"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03905"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03905"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03905"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03905"},
         }}},
        {VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03906"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03906"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03906"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03906"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03906"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03906"},
         }}},
        {VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-07272"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-07272"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-07272"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-07272"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-07272"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-07272"},
         }}},
        {VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03907"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03907"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03907"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03907"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03907"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03907"},
         }}},
        {VK_ACCESS_2_SHADER_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-07454"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-07454"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-07454"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-07454"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-07454"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-07454"},
         }}},
        {VK_ACCESS_2_SHADER_WRITE_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03909"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03909"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03909"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03909"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03909"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03909"},
         }}},
        {VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03910"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03910"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03910"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03910"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03910"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03910"},
         }}},
        {VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03911"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03911"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03911"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03911"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03911"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03911"},
         }}},
        {VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03912"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03912"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03912"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03912"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03912"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03912"},
         }}},
        {VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03913"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03913"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03913"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03913"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03913"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03913"},
         }}},
        {VK_ACCESS_2_TRANSFER_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03914"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03914"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03914"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03914"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03914"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03914"},
         }}},
        {VK_ACCESS_2_TRANSFER_WRITE_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03915"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03915"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03915"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03915"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03915"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03915"},
         }}},
        {VK_ACCESS_2_HOST_READ_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03916"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03916"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03916"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03916"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03916"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03916"},
         }}},
        {VK_ACCESS_2_HOST_WRITE_BIT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03917"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03917"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03917"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03917"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03917"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03917"},
         }}},
        {VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03918"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03918"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03918"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03918"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03918"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03918"},
         }}},
        {VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03919"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03919"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03919"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03919"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03919"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03919"},
         }}},
        {VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03920"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03920"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03920"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03920"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03920"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03920"},
         }}},
        {VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04747"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04747"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04747"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04747"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04747"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04747"},
         }}},
        {VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03920"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03920"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03920"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03920"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03920"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03920"},
         }}},
        {VK_ACCESS_2_SHADING_RATE_IMAGE_READ_BIT_NV,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03922"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03922"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03922"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03922"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03922"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03922"},
         }}},
        {VK_ACCESS_2_COMMAND_PREPROCESS_READ_BIT_NV,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03923"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03923"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03923"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03923"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03923"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03923"},
         }}},
        {VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_NV,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03924"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03924"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03924"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03924"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03924"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03924"},
         }}},
        {VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03925"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03925"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03925"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03925"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03925"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03925"},
         }}},
        {VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03926"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03926"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03926"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03926"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03926"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03926"},
         }}},
        {VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03927"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03927"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03927"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03927"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03927"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03927"},
         }}},
        {VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03928"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03928"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03928"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03928"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03928"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03928"},
         }}},
        {VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04858"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04858"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04858"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04858"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04858"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04858"},
         }}},
        {VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04859"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04859"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04859"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04859"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04859"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04859"},
         }}},
        {VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04860"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04860"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04860"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04860"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04860"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04860"},
         }}},
        {VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04861"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04861"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04861"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04861"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04861"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04861"},
         }}},
        {VK_ACCESS_2_OPTICAL_FLOW_READ_BIT_NV,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-07455"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-07455"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-07455"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-07455"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-07455"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-07455"},
         }}},
        {VK_ACCESS_2_OPTICAL_FLOW_WRITE_BIT_NV,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-07456"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-07456"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-07456"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-07456"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-07456"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-07456"},
         }}},
        {VK_ACCESS_2_MICROMAP_WRITE_BIT_EXT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-07457"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-07457"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-07457"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-07457"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-07457"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-07457"},
         }}},
        {VK_ACCESS_2_MICROMAP_READ_BIT_EXT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-07458"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-07458"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-07458"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-07458"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-07458"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-07458"},
         }}},
        {VK_ACCESS_2_INVOCATION_MASK_READ_BIT_HUAWEI,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04994"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04994"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04994"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04994"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04994"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04994"},
         }}},
        {VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT,
         ValueType{{
             {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-08118"},
             {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-08118"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-08118"},
             {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-08118"},
             {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-08118"},
             {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-08118"},
         }}},
    };
    return access_mask2_common;
}
// commonvalidity/fine_sync_commands_common.txt
const std::vector<Entry> &GetFineSyncCommon() {
    static const std::vector<Entry> kFineSyncCommon = {
        {Key(Func::vkCmdPipelineBarrier, Struct::VkMemoryBarrier, Field::srcAccessMask),
         "VUID-vkCmdPipelineBarrier-srcAccessMask-02815"},
        {Key(Func::vkCmdPipelineBarrier, Struct::VkMemoryBarrier, Field::dstAccessMask),
         "VUID-vkCmdPipelineBarrier-dstAccessMask-02816"},
        {Key(Func::vkCmdPipelineBarrier, Struct::VkBufferMemoryBarrier, Field::srcAccessMask),
         "VUID-vkCmdPipelineBarrier-pBufferMemoryBarriers-02817"},
        {Key(Func::vkCmdPipelineBarrier, Struct::VkBufferMemoryBarrier, Field::dstAccessMask),
         "VUID-vkCmdPipelineBarrier-pBufferMemoryBarriers-02818"},
        {Key(Func::vkCmdPipelineBarrier, Struct::VkImageMemoryBarrier, Field::srcAccessMask),
         "VUID-vkCmdPipelineBarrier-pImageMemoryBarriers-02819"},
        {Key(Func::vkCmdPipelineBarrier, Struct::VkImageMemoryBarrier, Field::dstAccessMask),
         "VUID-vkCmdPipelineBarrier-pImageMemoryBarriers-02820"},
        {Key(Func::vkCmdWaitEvents, Struct::VkMemoryBarrier, Field::srcAccessMask), "VUID-vkCmdWaitEvents-srcAccessMask-02815"},
        {Key(Func::vkCmdWaitEvents, Struct::VkMemoryBarrier, Field::dstAccessMask), "VUID-vkCmdWaitEvents-dstAccessMask-02816"},
        {Key(Func::vkCmdWaitEvents, Struct::VkBufferMemoryBarrier, Field::srcAccessMask),
         "VUID-vkCmdWaitEvents-pBufferMemoryBarriers-02817"},
        {Key(Func::vkCmdWaitEvents, Struct::VkBufferMemoryBarrier, Field::dstAccessMask),
         "VUID-vkCmdWaitEvents-pBufferMemoryBarriers-02818"},
        {Key(Func::vkCmdWaitEvents, Struct::VkImageMemoryBarrier, Field::srcAccessMask),
         "VUID-vkCmdWaitEvents-pImageMemoryBarriers-02819"},
        {Key(Func::vkCmdWaitEvents, Struct::VkImageMemoryBarrier, Field::dstAccessMask),
         "VUID-vkCmdWaitEvents-pImageMemoryBarriers-02820"},
        {Key(Struct::VkSubpassDependency, Field::srcAccessMask), "VUID-VkSubpassDependency-srcAccessMask-00868"},
        {Key(Struct::VkSubpassDependency, Field::dstAccessMask), "VUID-VkSubpassDependency-dstAccessMask-00869"},
        {Key(Struct::VkSubpassDependency2, Field::srcAccessMask), "VUID-VkSubpassDependency2-srcAccessMask-03088"},
        {Key(Struct::VkSubpassDependency2, Field::dstAccessMask), "VUID-VkSubpassDependency2-dstAccessMask-03089"},
    };
    return kFineSyncCommon;
}
const std::string &GetBadAccessFlagsVUID(const Location &loc, VkAccessFlags2 bit) {
    const auto &result = FindVUID(bit, loc, GetAccessMask2CommonMap());
    if (!result.empty()) {
        return result;
    }
    const auto &result2 = FindVUID(loc, GetFineSyncCommon());
    assert(!result2.empty());
    if (result2.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-bad-access-flags");
        return unhandled;
    }
    return result2;
}

// commonvalidity/access_mask_common.adoc/access_mask_2_common.adoc
static const auto &GetLocation2VUIDMap() {
    static const vvl::unordered_map<Key, std::string, Key::hash> Location2VUID{
        // Sync2 barriers. This can match different functions that work with VkDependencyInfo
        {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-06256"},
        {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-06256"},
        {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-06256"},
        {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-06256"},
        {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-06256"},
        {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-06256"},

        // Sync1 barrier. This matches only vkCmdPipelineBarrier.
        {Key(Func::vkCmdPipelineBarrier, Struct::VkMemoryBarrier, Field::srcAccessMask),
         "VUID-vkCmdPipelineBarrier-srcAccessMask-06257"},
        {Key(Func::vkCmdPipelineBarrier, Struct::VkMemoryBarrier, Field::dstAccessMask),
         "VUID-vkCmdPipelineBarrier-dstAccessMask-06257"},
        {Key(Func::vkCmdPipelineBarrier, Struct::VkBufferMemoryBarrier, Field::srcAccessMask),
         "VUID-vkCmdPipelineBarrier-srcAccessMask-06257"},
        {Key(Func::vkCmdPipelineBarrier, Struct::VkBufferMemoryBarrier, Field::dstAccessMask),
         "VUID-vkCmdPipelineBarrier-dstAccessMask-06257"},
        {Key(Func::vkCmdPipelineBarrier, Struct::VkImageMemoryBarrier, Field::srcAccessMask),
         "VUID-vkCmdPipelineBarrier-srcAccessMask-06257"},
        {Key(Func::vkCmdPipelineBarrier, Struct::VkImageMemoryBarrier, Field::dstAccessMask),
         "VUID-vkCmdPipelineBarrier-dstAccessMask-06257"},

        // Sync1 event wait. This matches only vkCmdWaitEvents.
        {Key(Func::vkCmdWaitEvents, Struct::VkMemoryBarrier, Field::srcAccessMask), "VUID-vkCmdWaitEvents-srcAccessMask-06257"},
        {Key(Func::vkCmdWaitEvents, Struct::VkMemoryBarrier, Field::dstAccessMask), "VUID-vkCmdWaitEvents-dstAccessMask-06257"},
        {Key(Func::vkCmdWaitEvents, Struct::VkBufferMemoryBarrier, Field::srcAccessMask),
         "VUID-vkCmdWaitEvents-srcAccessMask-06257"},
        {Key(Func::vkCmdWaitEvents, Struct::VkBufferMemoryBarrier, Field::dstAccessMask),
         "VUID-vkCmdWaitEvents-dstAccessMask-06257"},
        {Key(Func::vkCmdWaitEvents, Struct::VkImageMemoryBarrier, Field::srcAccessMask),
         "VUID-vkCmdWaitEvents-srcAccessMask-06257"},
        {Key(Func::vkCmdWaitEvents, Struct::VkImageMemoryBarrier, Field::dstAccessMask),
         "VUID-vkCmdWaitEvents-dstAccessMask-06257"},
    };
    return Location2VUID;
}

const std::string &GetAccessMaskRayQueryVUIDSelector(const Location &loc, const DeviceExtensions &device_extensions) {
    // At first try exact match: VUID for specific parameter (struct + field) of specific function
    const Key key_full(loc.function, loc.structure, loc.field);
    if (auto it = GetLocation2VUIDMap().find(key_full); it != GetLocation2VUIDMap().end()) {
        return it->second;
    }
    // Try to match VUID based on parameter (so can be used by multiple functions)
    const Key key_struct_field(loc.structure, loc.field);
    if (auto it = GetLocation2VUIDMap().find(key_struct_field); it != GetLocation2VUIDMap().end()) {
        return it->second;
    }
    static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-bad-access-flags");
    return unhandled;
}

const std::vector<Entry> &GetQueueCapErrors() {
    static const std::vector<Entry> kQueueCapErrors{
        {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-vkQueueSubmit-pWaitDstStageMask-00066"},
        {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-vkCmdBeginRenderPass-srcStageMask-06451"},
        {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-vkCmdBeginRenderPass-dstStageMask-06452"},
        {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-06457"},
        {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-06458"},
        {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-06459"},
        {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-06460"},
        {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-06461"},
        {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-06462"},
        {Key(Func::vkCmdWriteBufferMarkerAMD, Field::pipelineStage), "VUID-vkCmdWriteBufferMarkerAMD-pipelineStage-04074"},
        {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04074"},
        {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-vkCmdBeginRenderPass2-srcStageMask-06453"},
        {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-vkCmdBeginRenderPass2-dstStageMask-06454"},
        {Key(Func::vkCmdSetEvent, Field::srcStageMask), "VUID-vkCmdSetEvent2-srcStageMask-03827"},
        {Key(Func::vkCmdSetEvent, Field::dstStageMask), "VUID-vkCmdSetEvent2-dstStageMask-03828"},
        {Key(Func::vkCmdPipelineBarrier2, Struct::VkMemoryBarrier2, Field::srcStageMask),
         "VUID-vkCmdPipelineBarrier2-srcStageMask-09673"},
        {Key(Func::vkCmdPipelineBarrier2, Struct::VkMemoryBarrier2, Field::dstStageMask),
         "VUID-vkCmdPipelineBarrier2-dstStageMask-09674"},
        {Key(Func::vkCmdPipelineBarrier2, Struct::VkBufferMemoryBarrier2, Field::srcStageMask),
         "VUID-vkCmdPipelineBarrier2-srcStageMask-09675"},
        {Key(Func::vkCmdPipelineBarrier2, Struct::VkBufferMemoryBarrier2, Field::dstStageMask),
         "VUID-vkCmdPipelineBarrier2-dstStageMask-09676"},
        {Key(Func::vkCmdPipelineBarrier2, Struct::VkImageMemoryBarrier2, Field::srcStageMask),
         "VUID-vkCmdPipelineBarrier2-srcStageMask-09675"},
        {Key(Func::vkCmdPipelineBarrier2, Struct::VkImageMemoryBarrier2, Field::dstStageMask),
         "VUID-vkCmdPipelineBarrier2-dstStageMask-09676"},
        {Key(Func::vkCmdWaitEvents2, Field::srcStageMask), "VUID-vkCmdWaitEvents2-srcStageMask-03842"},
        {Key(Func::vkCmdWaitEvents2, Field::dstStageMask), "VUID-vkCmdWaitEvents2-dstStageMask-03843"},
        {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03860"},
        {Key(Func::vkQueueSubmit2, Field::pSignalSemaphoreInfos, true), "VUID-vkQueueSubmit2-stageMask-03869"},
        {Key(Func::vkQueueSubmit2, Field::pWaitSemaphoreInfos, true), "VUID-vkQueueSubmit2-stageMask-03870"},
    };
    return kQueueCapErrors;
}

const std::string &GetStageQueueCapVUID(const Location &loc, VkPipelineStageFlags2 bit) {
    // no per-bit lookups needed
    const auto &result = FindVUID(loc, GetQueueCapErrors());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-queue-capabilities");
        return unhandled;
    }
    return result;
}

static const vvl::unordered_map<QueueError, std::vector<Entry>> &GetBarrierQueueErrors() {
    static const vvl::unordered_map<QueueError, std::vector<Entry>> kBarrierQueueErrors{
        {QueueError::kSrcNoExternalExt,
         {
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-None-09097"},
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-None-09097"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-None-09119"},
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-None-09119"},
         }},
        {QueueError::kDstNoExternalExt,
         {
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-None-09098"},
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-None-09098"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-None-09120"},
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-None-09120"},
         }},
        {QueueError::kSrcNoForeignExt,
         {
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-srcQueueFamilyIndex-09099"},
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-srcQueueFamilyIndex-09099"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-09121"},
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-09121"},
         }},
        {QueueError::kDstNoForeignExt,
         {
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-dstQueueFamilyIndex-09100"},
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-dstQueueFamilyIndex-09100"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-dstQueueFamilyIndex-09122"},
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-dstQueueFamilyIndex-09122"},
         }},
        {QueueError::kSync1ConcurrentNoIgnored,
         {
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-None-09049"},
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-None-09052"},
         }},
        {QueueError::kSync1ConcurrentSrc,
         {
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-None-09050"},
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-None-09053"},
         }},
        {QueueError::kSync1ConcurrentDst,
         {
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-None-09051"},
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-None-09054"},
         }},
        {QueueError::kExclusiveSrc,
         {
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-buffer-09095"},
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-buffer-09095"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-09117"},
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-09117"},
         }},
        {QueueError::kExclusiveDst,
         {
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-buffer-09096"},
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-buffer-09096"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-09118"},
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-09118"},
         }},
        {QueueError::kHostStage,
         {
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-srcStageMask-03851"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcStageMask-03854"},
             {Key(Struct::VkBufferMemoryBarrier), "VUID-vkCmdPipelineBarrier-srcStageMask-09634"},
             {Key(Struct::VkImageMemoryBarrier), "VUID-vkCmdPipelineBarrier-srcStageMask-09633"},
         }},
        {QueueError::kSubmitQueueMustMatchSrcOrDst,
         {
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-vkCmdPipelineBarrier2-srcQueueFamilyIndex-10387"},
             {Key(Struct::VkBufferMemoryBarrier), "VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-10388"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-vkCmdPipelineBarrier2-srcQueueFamilyIndex-10387"},
             {Key(Struct::VkImageMemoryBarrier), "VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-10388"},
         }},
    };
    return kBarrierQueueErrors;
}

const vvl::unordered_map<QueueError, std::string> &GetQueueErrorSummaryMap() {
    static const vvl::unordered_map<QueueError, std::string> kQueueErrorSummary{
        {QueueError::kSrcNoExternalExt, "Source queue family must not be VK_QUEUE_FAMILY_EXTERNAL."},
        {QueueError::kDstNoExternalExt, "Destination queue family must not be VK_QUEUE_FAMILY_EXTERNAL."},
        {QueueError::kSrcNoForeignExt, "Source queue family must not be VK_QUEUE_FAMILY_FOREIGN_EXT."},
        {QueueError::kDstNoForeignExt, "Destination queue family must not be VK_QUEUE_FAMILY_FOREIGN_EXT."},
        {QueueError::kSync1ConcurrentSrc, "Source queue family must be VK_QUEUE_FAMILY_IGNORED or VK_QUEUE_FAMILY_EXTERNAL."},
        {QueueError::kSync1ConcurrentDst, "Destination queue family must be VK_QUEUE_FAMILY_IGNORED or VK_QUEUE_FAMILY_EXTERNAL."},
        {QueueError::kExclusiveSrc, "Source queue family must be valid when using VK_SHARING_MODE_EXCLUSIVE."},
        {QueueError::kExclusiveDst, "Destination queue family must be valid when using VK_SHARING_MODE_EXCLUSIVE."},
    };
    return kQueueErrorSummary;
}

const std::string &GetBarrierQueueVUID(const Location &loc, QueueError error) {
    const auto &result = FindVUID(error, loc, GetBarrierQueueErrors());
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-queue-error");
        return unhandled;
    }
    return result;
}

const vvl::unordered_map<VkImageLayout, std::array<Entry, 2>> &GetImageLayoutErrorsMap() {
    using ValueType = std::array<Entry, 2>;
    static const vvl::unordered_map<VkImageLayout, std::array<Entry, 2>> kImageLayoutErrors{
        {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01208"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
         }}},
        {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01209"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
         }}},
        {VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01210"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
         }}},
        {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01211"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
         }}},
        {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01212"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
         }}},
        {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01213"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
         }}},
        {VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,  // alias VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-02088"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-02088"},
         }}},
        {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01658"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
         }}},
        {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01659"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
         }}},
        {VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07006"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07006"},
         }}},
        {VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-09550"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-09550"},
         }}},
        {VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"},
         }}},
        {VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"},
         }}},
        {VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"},
         }}},
        {VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"},
         }}},
        {VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"},
         }}},
        {VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"},
         }}},
        {VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR,
         ValueType{{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-10287"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-10287"},
         }}},
    };
    return kImageLayoutErrors;
}

const std::string &GetBadImageLayoutVUID(const Location &loc, VkImageLayout layout) {
    const auto &result = FindVUID(layout, loc, GetImageLayoutErrorsMap());
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-bad-image-layout");
        return unhandled;
    }
    return result;
}

static const vvl::unordered_map<BufferError, std::array<Entry, 2>> &GetBufferErrorsMap() {
    static const vvl::unordered_map<BufferError, std::array<Entry, 2>> kBufferErrors{
        {BufferError::kNoMemory,
         {{
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-buffer-01931"},
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-buffer-01931"},
         }}},
        {BufferError::kOffsetTooBig,
         {{
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-offset-01187"},
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-offset-01187"},
         }}},
        {BufferError::kSizeOutOfRange,
         {{
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-size-01189"},
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-size-01189"},
         }}},
        {BufferError::kSizeZero,
         {{
             {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-size-01188"},
             {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-size-01188"},
         }}},
    };
    return kBufferErrors;
}

const std::string &GetBufferBarrierVUID(const Location &loc, BufferError error) {
    const auto &result = FindVUID(error, loc, GetBufferErrorsMap());
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-buffer-barrier-error");
        return unhandled;
    }
    return result;
}

const vvl::unordered_map<ImageError, std::vector<Entry>> &GetImageErrorsMap() {
    static const vvl::unordered_map<ImageError, std::vector<Entry>> kImageErrors{
        {ImageError::kNoMemory,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-01932"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-01932"},
         }},
        {ImageError::kConflictingLayout,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01197"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01197"},
         }},
        {ImageError::kBadLayout,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-newLayout-01198"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-newLayout-01198"},
         }},
        {ImageError::kBadAttFeedbackLoopLayout,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-attachmentFeedbackLoopLayout-07313"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-attachmentFeedbackLoopLayout-07313"},
         }},
        {ImageError::kBadSync2OldLayout,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-synchronization2-07793"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-synchronization2-07793"},
         }},
        {ImageError::kBadSync2NewLayout,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-synchronization2-07794"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-synchronization2-07794"},
         }},
        {ImageError::kNotColorAspectSinglePlane,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-09241"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-09241"},
         }},
        {ImageError::kNotColorAspectNonDisjoint,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-09242"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-09242"},
         }},
        {ImageError::kBadMultiplanarAspect,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-01672"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-01672"},
         }},
        {ImageError::kNotDepthOrStencilAspect,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-03319"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-03319"},
         }},
        {ImageError::kNotDepthAndStencilAspect,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-03320"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-03320"},
         }},
        {ImageError::kSeparateDepthWithStencilLayout,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-aspectMask-08702"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-aspectMask-08702"},
         }},
        {ImageError::kSeparateStencilhWithDepthLayout,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-aspectMask-08703"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-aspectMask-08703"},
         }},
        {ImageError::kRenderPassMismatch,
         {
             {Key(Func::vkCmdPipelineBarrier), "VUID-vkCmdPipelineBarrier-image-04073"},
             {Key(Func::vkCmdPipelineBarrier2), "VUID-vkCmdPipelineBarrier2-image-04073"},
         }},
        {ImageError::kRenderPassMismatchColorUnused,
         {
             {Key(Func::vkCmdPipelineBarrier), "VUID-vkCmdPipelineBarrier-image-09373"},
             {Key(Func::vkCmdPipelineBarrier2), "VUID-vkCmdPipelineBarrier2-image-09373"},
         }},
        {ImageError::kRenderPassMismatchAhbZero,
         {
             {Key(Func::vkCmdPipelineBarrier), "VUID-vkCmdPipelineBarrier-image-09374"},
             {Key(Func::vkCmdPipelineBarrier2), "VUID-vkCmdPipelineBarrier2-image-09374"},
         }},
        {ImageError::kRenderPassLayoutChange,
         {
             {Key(Func::vkCmdPipelineBarrier), "VUID-vkCmdPipelineBarrier-oldLayout-01181"},
             {Key(Func::vkCmdPipelineBarrier2), "VUID-vkCmdPipelineBarrier2-oldLayout-01181"},
         }},
        {ImageError::kDynamicRenderingLocalReadNew,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-dynamicRenderingLocalRead-09552"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-dynamicRenderingLocalRead-09552"},
         }},
        {ImageError::kDynamicRenderingLocalReadOld,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-dynamicRenderingLocalRead-09551"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-dynamicRenderingLocalRead-09551"},
         }},
        {ImageError::kAspectMask,
         {
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-subresourceRange-09601"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-subresourceRange-09601"},
         }},
    };
    return kImageErrors;
}

const std::string &GetImageBarrierVUID(const Location &loc, ImageError error) {
    const auto &result = FindVUID(error, loc, GetImageErrorsMap());
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-image-barrier-error");
        return unhandled;
    }
    return result;
}

static const vvl::unordered_map<SubmitError, std::vector<Entry>> &GetSubmitErrorsMap() {
    static const vvl::unordered_map<SubmitError, std::vector<Entry>> kSubmitErrors{
        {SubmitError::kTimelineSemSmallValue,
         {
             {Key(Struct::VkSemaphoreSignalInfo), "VUID-VkSemaphoreSignalInfo-value-03258"},
             {Key(Struct::VkBindSparseInfo), "VUID-VkBindSparseInfo-pSignalSemaphores-03249"},
             {Key(Struct::VkSubmitInfo), "VUID-VkSubmitInfo-pSignalSemaphores-03242"},
             {Key(Struct::VkSubmitInfo2), "VUID-VkSubmitInfo2-semaphore-03882"},
         }},
        {SubmitError::kSemAlreadySignalled,
         {
             {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pSignalSemaphores-00067"},
             {Key(Func::vkQueueBindSparse), "VUID-vkQueueBindSparse-pSignalSemaphores-01115"},
             {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-semaphore-03868"},
         }},
        {SubmitError::kBinaryCannotBeSignalled,
         {
             {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pWaitSemaphores-03238"},
             {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-semaphore-03873"},
             {Key(Func::vkQueueBindSparse), "VUID-vkQueueBindSparse-pWaitSemaphores-03245"},
             {Key(Func::vkQueuePresentKHR), "VUID-vkQueuePresentKHR-pWaitSemaphores-03268"},
         }},
        {SubmitError::kTimelineSemMaxDiff,
         {
             {Key(Struct::VkBindSparseInfo, Field::pWaitSemaphores), "VUID-VkBindSparseInfo-pWaitSemaphores-03250"},
             {Key(Struct::VkBindSparseInfo, Field::pSignalSemaphores), "VUID-VkBindSparseInfo-pSignalSemaphores-03251"},
             {Key(Struct::VkSubmitInfo, Field::pWaitSemaphores), "VUID-VkSubmitInfo-pWaitSemaphores-03243"},
             {Key(Struct::VkSubmitInfo, Field::pSignalSemaphores), "VUID-VkSubmitInfo-pSignalSemaphores-03244"},
             {Key(Struct::VkSemaphoreSignalInfo), "VUID-VkSemaphoreSignalInfo-value-03260"},
             {Key(Struct::VkSubmitInfo2, Field::pWaitSemaphoreInfos, true), "VUID-VkSubmitInfo2-semaphore-03884"},
             {Key(Struct::VkSubmitInfo2, Field::pSignalSemaphoreInfos, true), "VUID-VkSubmitInfo2-semaphore-03883"},
         }},
        {SubmitError::kProtectedFeatureDisabled,
         {
             {Key(Struct::VkProtectedSubmitInfo), "VUID-vkQueueSubmit-queue-06448"},
             {Key(Struct::VkSubmitInfo2), "VUID-vkQueueSubmit2-queue-06447"},
         }},
        {SubmitError::kBadUnprotectedSubmit,
         {
             {Key(Struct::VkSubmitInfo), "VUID-VkSubmitInfo-pNext-04148"},
             {Key(Struct::VkSubmitInfo2), "VUID-VkSubmitInfo2-flags-03886"},
         }},
        {SubmitError::kBadProtectedSubmit,
         {
             {Key(Struct::VkSubmitInfo), "VUID-VkSubmitInfo-pNext-04120"},
             {Key(Struct::VkSubmitInfo2), "VUID-VkSubmitInfo2-flags-03887"},
         }},
        {SubmitError::kCmdNotSimultaneous,
         {
             {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pCommandBuffers-00071"},
             {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-commandBuffer-03875"},
         }},
        {SubmitError::kReusedOneTimeCmd,
         {
             {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pCommandBuffers-00072"},
             {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-commandBuffer-03876"},
         }},
        {SubmitError::kSecondaryCmdNotSimultaneous,
         {
             {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pCommandBuffers-00073"},
             {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-commandBuffer-03877"},
         }},
        {SubmitError::kCmdWrongQueueFamily,
         {
             {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pCommandBuffers-00074"},
             {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-commandBuffer-03878"},
         }},
        {SubmitError::kSecondaryCmdInSubmit,
         {
             {Key(Func::vkQueueSubmit), "VUID-VkSubmitInfo-pCommandBuffers-00075"},
             {Key(Func::vkQueueSubmit2), "VUID-VkCommandBufferSubmitInfo-commandBuffer-03890"},
         }},
        {SubmitError::kHostStageMask,
         {
             {Key(Struct::VkSubmitInfo), "VUID-VkSubmitInfo-pWaitDstStageMask-00078"},
             {Key(Func::vkCmdSetEvent), "VUID-vkCmdSetEvent-stageMask-01149"},
             {Key(Func::vkCmdResetEvent), "VUID-vkCmdResetEvent-stageMask-01153"},
             {Key(Func::vkCmdResetEvent2), "VUID-vkCmdResetEvent2-stageMask-03830"},
         }},
        {SubmitError::kOtherQueueWaiting,
         {
             {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pWaitSemaphores-00068"},
             {Key(Func::vkQueueBindSparse), "VUID-vkQueueBindSparse-pWaitSemaphores-01116"},
             {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-semaphore-03871"},
             {Key(Func::vkQueuePresentKHR), "VUID-vkQueuePresentKHR-pWaitSemaphores-01294"},
         }},
    };
    return kSubmitErrors;
}

const std::string &GetQueueSubmitVUID(const Location &loc, SubmitError error) {
    const auto &result = FindVUID(error, loc, GetSubmitErrorsMap());
    if (result.empty()) {
        // TODO - Handle better way then Key::recursive to find certain VUs
        // Can reproduce with NegativeSyncObject.Sync2QueueSubmitTimelineSemaphoreValue
        if (loc.structure == Struct::VkSubmitInfo2) {
            if (loc.prev->field == Field::pWaitSemaphoreInfos || loc.prev->field == Field::pSignalSemaphoreInfos) {
                return FindVUID(error, *loc.prev, GetSubmitErrorsMap());
            }
        }

        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-submit-error");
        return unhandled;
    }
    return result;
}

const std::string &GetShaderTileImageVUID(const Location &loc, ShaderTileImageError error) {
    static const vvl::unordered_map<ShaderTileImageError, std::vector<Entry>> kShaderTileImageErrors{
        {ShaderTileImageError::kShaderTileImageFeatureError,
         {
             {Key(Func::vkCmdPipelineBarrier), "VUID-vkCmdPipelineBarrier-None-09553"},
             {Key(Func::vkCmdPipelineBarrier2), "VUID-vkCmdPipelineBarrier2-None-09553"},
         }},
        {ShaderTileImageError::kShaderTileImageFramebufferSpace,
         {
             {Key(Func::vkCmdPipelineBarrier), "VUID-vkCmdPipelineBarrier-srcStageMask-09556"},
             {Key(Func::vkCmdPipelineBarrier2), "VUID-vkCmdPipelineBarrier2-srcStageMask-09556"},
         }},
        {ShaderTileImageError::kShaderTileImageNoBuffersOrImages,
         {
             {Key(Func::vkCmdPipelineBarrier), "VUID-vkCmdPipelineBarrier-None-09554"},
             {Key(Func::vkCmdPipelineBarrier2), "VUID-vkCmdPipelineBarrier2-None-09554"},
         }},
        {ShaderTileImageError::kShaderTileImageLayout,
         {
             {Key(Func::vkCmdPipelineBarrier), "VUID-vkCmdPipelineBarrier-image-09555"},
             {Key(Func::vkCmdPipelineBarrier2), "VUID-vkCmdPipelineBarrier2-image-09555"},
         }},
        {ShaderTileImageError::kShaderTileImageDependencyFlags,
         {
             {Key(Func::vkCmdPipelineBarrier), "VUID-vkCmdPipelineBarrier-dependencyFlags-07891"},
             {Key(Func::vkCmdPipelineBarrier2), "VUID-vkCmdPipelineBarrier2-dependencyFlags-07891"},
         }},
    };

    const auto &result = FindVUID(error, loc, kShaderTileImageErrors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-barrier-error");
        return unhandled;
    }
    return result;
}

}  // namespace sync_vuid_maps
