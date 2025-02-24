/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <algorithm>
#include <assert.h>
#include <string>

#include <vulkan/vk_enum_string_helper.h>
#include "core_validation.h"
#include "cc_buffer_address.h"
#include "utils/ray_tracing_utils.h"
#include "state_tracker/ray_tracing_state.h"
#include "error_message/error_strings.h"

bool CoreChecks::ValidateInsertAccelerationStructureMemoryRange(VkAccelerationStructureNV as, const vvl::DeviceMemory &mem_info,
                                                                VkDeviceSize mem_offset, const Location &loc) const {
    return ValidateInsertMemoryRange(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureNV), mem_info, mem_offset, loc);
}

bool CoreChecks::PreCallValidateCreateAccelerationStructureNV(VkDevice device,
                                                              const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkAccelerationStructureNV *pAccelerationStructure,
                                                              const ErrorObject &error_obj) const {
    bool skip = false;
    if (pCreateInfo != nullptr && pCreateInfo->info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV) {
        for (uint32_t i = 0; i < pCreateInfo->info.geometryCount; i++) {
            skip |= ValidateGeometryNV(pCreateInfo->info.pGeometries[i],
                                       error_obj.location.dot(Field::pCreateInfo).dot(Field::info).dot(Field::pGeometries, i));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateAccelerationStructureKHR(VkDevice device,
                                                               const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkAccelerationStructureKHR *pAccelerationStructure,
                                                               const ErrorObject &error_obj) const {
    bool skip = false;
    auto buffer_state = Get<vvl::Buffer>(pCreateInfo->buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);

    if (!(buffer_state->usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-buffer-03614", buffer_state->Handle(),
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::buffer), "was created with %s.",
                         string_VkBufferUsageFlags2(buffer_state->usage).c_str());
    }
    if (buffer_state->create_info.flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-buffer-03615", buffer_state->Handle(),
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::buffer), "was created with %s.",
                         string_VkBufferCreateFlags(buffer_state->create_info.flags).c_str());
    }
    if (pCreateInfo->offset + pCreateInfo->size > buffer_state->create_info.size) {
        skip |= LogError("VUID-VkAccelerationStructureCreateInfoKHR-offset-03616", buffer_state->Handle(),
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::offset),
                         "(%" PRIu64 ") + size (%" PRIu64 ") must be less than the size of buffer (%" PRIu64 ").",
                         pCreateInfo->offset, pCreateInfo->size, buffer_state->create_info.size);
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                  const VkBindAccelerationStructureMemoryInfoNV *pBindInfos,
                                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const Location bind_info_loc = error_obj.location.dot(Field::pBindInfos, i);
        const VkBindAccelerationStructureMemoryInfoNV &info = pBindInfos[i];
        auto as_state = Get<vvl::AccelerationStructureNV>(info.accelerationStructure);
        ASSERT_AND_CONTINUE(as_state);

        if (as_state->HasFullRangeBound()) {
            skip |= LogError("VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-03620", info.accelerationStructure,
                             bind_info_loc.dot(Field::accelerationStructure), "must not already be backed by a memory object.");
        }

        // Validate bound memory range information
        auto mem_info = Get<vvl::DeviceMemory>(info.memory);
        if (mem_info) {
            skip |= ValidateInsertAccelerationStructureMemoryRange(info.accelerationStructure, *mem_info, info.memoryOffset,
                                                                   bind_info_loc.dot(Field::memoryOffset));
            skip |= ValidateMemoryTypes(*mem_info, as_state->memory_requirements.memoryTypeBits,
                                        bind_info_loc.dot(Field::accelerationStructure),
                                        "VUID-VkBindAccelerationStructureMemoryInfoNV-memory-03622");
        }

        // Validate memory requirements alignment
        if (SafeModulo(info.memoryOffset, as_state->memory_requirements.alignment) != 0) {
            skip |= LogError("VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03623", info.accelerationStructure,
                             bind_info_loc.dot(Field::memoryOffset),
                             "(%" PRIu64 ") must be a multiple of the alignment (%" PRIu64
                             ") member of the VkMemoryRequirements structure returned from "
                             "a call to vkGetAccelerationStructureMemoryRequirementsNV with accelerationStructure %s and type of "
                             "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV",
                             info.memoryOffset, as_state->memory_requirements.alignment,
                             FormatHandle(info.accelerationStructure).c_str());
        }

        if (mem_info) {
            // Validate memory requirements size
            if (as_state->memory_requirements.size > (mem_info->allocate_info.allocationSize - info.memoryOffset)) {
                skip |= LogError("VUID-VkBindAccelerationStructureMemoryInfoNV-size-03624", info.accelerationStructure,
                                 bind_info_loc.dot(Field::memory),
                                 "'s size (%" PRIu64 ") minus %s (%" PRIu64 ") is %" PRIu64
                                 ", but the size member of the VkMemoryRequirements structure returned from a call to "
                                 "vkGetAccelerationStructureMemoryRequirementsNV with accelerationStructure %s and type of "
                                 "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV is %" PRIu64 ".",
                                 as_state->memory_requirements.size, bind_info_loc.dot(Field::memoryOffset).Fields().c_str(),
                                 info.memoryOffset, mem_info->allocate_info.allocationSize - info.memoryOffset,
                                 FormatHandle(info.accelerationStructure).c_str(), as_state->memory_requirements.size);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                 size_t dataSize, void *pData, const ErrorObject &error_obj) const {
    bool skip = false;

    if (auto as_state = Get<vvl::AccelerationStructureNV>(accelerationStructure)) {
        skip |= VerifyBoundMemoryIsValid(as_state->MemoryState(), LogObjectList(accelerationStructure), as_state->Handle(),
                                         error_obj.location.dot(Field::accelerationStructure),
                                         "VUID-vkGetAccelerationStructureHandleNV-accelerationStructure-02787");
    }

    return skip;
}

bool CoreChecks::ValidateAccelerationStructuresMemoryAlisasing(const LogObjectList &objlist, uint32_t infoCount,
                                                               const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                               uint32_t info_i, const ErrorObject &error_obj) const {
    using sparse_container::range;

    bool skip = false;
    const VkAccelerationStructureBuildGeometryInfoKHR &info = pInfos[info_i];
    const Location info_i_loc = error_obj.location.dot(Field::pInfos, info_i);
    const auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure);
    const auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure);

    const bool info_in_mode_update = info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;

    if (info_in_mode_update && info.srcAccelerationStructure != info.dstAccelerationStructure && src_as_state && dst_as_state) {
        const char *vuid = error_obj.location.function == Func::vkCmdBuildAccelerationStructuresKHR
                               ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03668"
                           : error_obj.location.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                               ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03668"
                               : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03668";
        skip |= ValidateAccelStructsMemoryDoNotOverlap(error_obj.location, objlist, *src_as_state,
                                                       info_i_loc.dot(Field::srcAccelerationStructure), *dst_as_state,
                                                       info_i_loc.dot(Field::dstAccelerationStructure), vuid);
    }

    // Loop on other acceleration structure builds info.
    // Given that comparisons are commutative, only need to consider elements after info_i
    assert(infoCount > info_i);
    for (auto [other_info_j, other_info] : vvl::enumerate(pInfos + info_i + 1, infoCount - (info_i + 1))) {
        // Validate that scratch buffer's memory does not overlap destination acceleration structure's memory, or source
        // acceleration structure's memory if build mode is update, or other scratch buffers' memory.
        // Here validation is pessimistic: if one buffer associated to pInfos[other_info_j].scratchData.deviceAddress has an
        // overlap, an error will be logged.

        const Location other_info_j_loc = error_obj.location.dot(Field::pInfos, other_info_j);

        const auto other_dst_as_state = Get<vvl::AccelerationStructureKHR>(other_info.dstAccelerationStructure);
        const auto other_src_as_state = Get<vvl::AccelerationStructureKHR>(other_info.srcAccelerationStructure);

        // Validate destination acceleration structure's memory is not overlapped by another source acceleration structure's
        // memory that is going to be updated by this cmd
        if (dst_as_state && other_src_as_state) {
            if (other_info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                const char *vuid = error_obj.location.function == Func::vkCmdBuildAccelerationStructuresKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03701"
                                   : error_obj.location.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03701"
                                       : "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03701";

                skip |= ValidateAccelStructsMemoryDoNotOverlap(error_obj.location, objlist, *dst_as_state,
                                                               info_i_loc.dot(Field::dstAccelerationStructure), *other_src_as_state,
                                                               other_info_j_loc.dot(Field::srcAccelerationStructure), vuid);
            }
        }

        // Validate that there is no destination acceleration structures' memory overlaps
        if (dst_as_state && other_dst_as_state) {
            const char *vuid = error_obj.location.function == Func::vkCmdBuildAccelerationStructuresKHR
                                   ? "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03702"
                               : error_obj.location.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                   ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03702"
                                   : "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03702";

            skip |= ValidateAccelStructsMemoryDoNotOverlap(error_obj.location, objlist, *dst_as_state,
                                                           info_i_loc.dot(Field::dstAccelerationStructure), *other_dst_as_state,
                                                           other_info_j_loc.dot(Field::dstAccelerationStructure), vuid);
        }
    }

    return skip;
}

bool CoreChecks::ValidateAccelerationStructuresDeviceScratchBufferMemoryAlisasing(
    const LogObjectList &objlist, uint32_t info_count, const VkAccelerationStructureBuildGeometryInfoKHR *p_infos, uint32_t info_i,
    const VkAccelerationStructureBuildRangeInfoKHR *const *pp_range_infos, const ErrorObject &error_obj) const {
    using sparse_container::range;

    bool skip = false;
    const VkAccelerationStructureBuildGeometryInfoKHR &info = p_infos[info_i];
    const Location info_i_loc = error_obj.location.dot(Field::pInfos, info_i);
    const auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure);
    const auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure);
    const rt::BuildType rt_build_type =
        error_obj.location.function == Func::vkBuildAccelerationStructuresKHR ? rt::BuildType::Host : rt::BuildType::Device;

    const bool info_in_mode_update = info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;

    // Cannot compute scratch buffer size from the CPU with indirect calls,
    // so cannot perform validation
    vvl::span<vvl::Buffer *const> info_scratches = GetBuffersByAddress(info.scratchData.deviceAddress);
    const VkDeviceSize assumed_scratch_size = rt::ComputeScratchSize(rt_build_type, device, info, pp_range_infos[info_i]);

    if (dst_as_state) {
        vvl::span<vvl::Buffer *const> dummy(nullptr, 0);
        skip |= ValidateScratchMemoryNoOverlap(error_obj.location, objlist, info_scratches, info.scratchData.deviceAddress,
                                               assumed_scratch_size, info_i_loc.dot(Field::scratchData).dot(Field::deviceAddress),
                                               info_in_mode_update ? src_as_state.get() : nullptr,
                                               info_i_loc.dot(Field::srcAccelerationStructure), *dst_as_state,
                                               info_i_loc.dot(Field::dstAccelerationStructure), dummy, 0, 0, nullptr);
    }

    // Loop on other acceleration structure builds info.
    // Given that comparisons are commutative, only need to consider elements after info_i
    assert(info_count > info_i);
    for (uint32_t other_info_j = info_i + 1; other_info_j < info_count; ++other_info_j) {
        // Validate that scratch buffer's memory does not overlap destination acceleration structure's memory, or source
        // acceleration structure's memory if build mode is update, or other scratch buffers' memory.
        // Here validation is pessimistic: if one buffer associated to pInfos[other_info_j].scratchData.deviceAddress has an
        // overlap, an error will be logged.

        const VkAccelerationStructureBuildGeometryInfoKHR &other_info = p_infos[other_info_j];
        const Location other_info_j_loc = error_obj.location.dot(Field::pInfos, other_info_j);

        const auto other_dst_as_state = Get<vvl::AccelerationStructureKHR>(other_info.dstAccelerationStructure);
        const auto other_src_as_state = Get<vvl::AccelerationStructureKHR>(other_info.srcAccelerationStructure);

        const bool other_info_in_update_mode = other_info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;

        if (other_dst_as_state) {
            auto other_info_scratches = GetBuffersByAddress(other_info.scratchData.deviceAddress);
            const VkDeviceSize assumed_other_scratch_size =
                rt::ComputeScratchSize(rt_build_type, device, other_info, pp_range_infos[other_info_j]);

            const Location other_scratch_loc = other_info_j_loc.dot(Field::scratchData);
            const Location other_scratch_address_loc = other_scratch_loc.dot(Field::deviceAddress);

            skip |= ValidateScratchMemoryNoOverlap(
                error_obj.location, objlist, info_scratches, info.scratchData.deviceAddress, assumed_scratch_size,
                info_i_loc.dot(Field::scratchData).dot(Field::deviceAddress),
                other_info_in_update_mode ? other_src_as_state.get() : nullptr,
                other_info_j_loc.dot(Field::srcAccelerationStructure), *other_dst_as_state,
                other_info_j_loc.dot(Field::dstAccelerationStructure), other_info_scratches, other_info.scratchData.deviceAddress,
                assumed_other_scratch_size, &other_scratch_address_loc);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                         const VkAccelerationStructureDeviceAddressInfoKHR *pInfo,
                                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    if (!enabled_features.accelerationStructure) {
        skip |= LogError("VUID-vkGetAccelerationStructureDeviceAddressKHR-accelerationStructure-08935", device, error_obj.location,
                         "accelerationStructure feature was not enabled.");
    }
    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetAccelerationStructureDeviceAddressKHR-device-03504", device, error_obj.location,
                         "bufferDeviceAddressMultiDevice feature was not enabled.");
    }

    if (const auto accel_struct = Get<vvl::AccelerationStructureKHR>(pInfo->accelerationStructure)) {
        const Location info_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateMemoryIsBoundToBuffer(device, *accel_struct->buffer_state,
                                              info_loc.dot(Field::accelerationStructure).dot(Field::buffer),
                                              "VUID-vkGetAccelerationStructureDeviceAddressKHR-pInfo-09541");

        if (!(accel_struct->buffer_state->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
            skip |= LogError("VUID-vkGetAccelerationStructureDeviceAddressKHR-pInfo-09542", LogObjectList(device),
                             info_loc.dot(Field::accelerationStructure).dot(Field::buffer), "was created with usage flag(s) %s.",
                             string_VkBufferUsageFlags2(accel_struct->buffer_state->usage).c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidateAccelerationBuffers(VkCommandBuffer cmd_buffer, uint32_t info_i,
                                             const VkAccelerationStructureBuildGeometryInfoKHR &info,
                                             const VkAccelerationStructureBuildRangeInfoKHR *geometry_build_ranges,
                                             const Location &info_loc) const {
    bool skip = false;

    const auto pick_vuid = [&info_loc](const char *direct_build_vu, const char *indirect_build_vu) -> const char * {
        return info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR ? direct_build_vu : indirect_build_vu;
    };

    auto buffer_check = [this, &pick_vuid](uint32_t gi, const VkDeviceOrHostAddressConstKHR address,
                                           const Location &geom_loc) -> bool {
        const auto buffer_states = GetBuffersByAddress(address.deviceAddress);
        const bool no_valid_buffer_found =
            !buffer_states.empty() && std::none_of(buffer_states.begin(), buffer_states.end(), [](const vvl::Buffer *buffer_state) {
                return buffer_state->usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
            });
        if (no_valid_buffer_found) {
            LogObjectList objlist(device);
            for (const auto &buffer_state : buffer_states) {
                objlist.add(buffer_state->Handle());
            }
            return LogError(
                pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673",
                          "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-geometry-03673"),
                objlist, geom_loc,
                "has no buffer which created with VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR.");
        }

        return false;
    };

    const Location pp_build_range_info_loc(info_loc.function, Field::ppBuildRangeInfos, info_i);
    for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
        const Location p_geom_loc = info_loc.dot(info.pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);
        const Location p_geom_geom_loc = p_geom_loc.dot(Field::geometry);
        const Location p_geom_geom_triangles_loc = p_geom_geom_loc.dot(Field::triangles);
        const VkAccelerationStructureGeometryKHR &geom_data = rt::GetGeometry(info, geom_i);

        switch (geom_data.geometryType) {
            case VK_GEOMETRY_TYPE_TRIANGLES_KHR:  // == VK_GEOMETRY_TYPE_TRIANGLES_NV
            {
                skip |=
                    buffer_check(geom_i, geom_data.geometry.triangles.vertexData, p_geom_geom_triangles_loc.dot(Field::vertexData));
                skip |=
                    buffer_check(geom_i, geom_data.geometry.triangles.indexData, p_geom_geom_triangles_loc.dot(Field::indexData));
                skip |= buffer_check(geom_i, geom_data.geometry.triangles.transformData,
                                     p_geom_geom_triangles_loc.dot(Field::transformData));

                auto vertex_buffer_states = GetBuffersByAddress(geom_data.geometry.triangles.vertexData.deviceAddress);
                if (vertex_buffer_states.empty() && geometry_build_ranges[geom_i].primitiveCount > 0) {
                    skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03804",
                                               "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03804"),
                                     cmd_buffer, p_geom_geom_triangles_loc.dot(Field::vertexData).dot(Field::deviceAddress),
                                     "(0x%" PRIx64 ") is not an address belonging to an existing buffer.",
                                     geom_data.geometry.triangles.vertexData.deviceAddress);
                } else {
                    BufferAddressValidation<1> buffer_address_validator = {
                        {{{pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03805",
                                     "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03805"),
                           [this](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                               return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state, out_error_msg);
                           },
                           []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }}}}};

                    skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
                        *this, vertex_buffer_states, p_geom_geom_triangles_loc.dot(Field::vertexData).dot(Field::deviceAddress),
                        LogObjectList(cmd_buffer), geom_data.geometry.triangles.vertexData.deviceAddress);
                }

                if (geom_data.geometry.triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
                    auto index_buffer_states = GetBuffersByAddress(geom_data.geometry.triangles.indexData.deviceAddress);
                    if (index_buffer_states.empty() && geometry_build_ranges[geom_i].primitiveCount > 0) {
                        skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03806",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03806"),
                                         cmd_buffer, p_geom_geom_triangles_loc.dot(Field::indexData).dot(Field::deviceAddress),
                                         "(0x%" PRIx64 ") is not an address belonging to an existing buffer. %s is %s.",
                                         geom_data.geometry.triangles.indexData.deviceAddress,
                                         p_geom_geom_triangles_loc.dot(Field::indexType).Fields().c_str(),
                                         string_VkIndexType(geom_data.geometry.triangles.indexType));
                    } else {
                        BufferAddressValidation<1> buffer_address_validator = {
                            {{{pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03807",
                                         "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03807"),
                               [this](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                                   return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state,
                                                                                                  out_error_msg);
                               },
                               []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }}}}};

                        skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
                            *this, index_buffer_states, p_geom_geom_triangles_loc.dot(Field::indexData).dot(Field::deviceAddress),
                            LogObjectList(cmd_buffer), geom_data.geometry.triangles.indexData.deviceAddress);
                    }
                    if (info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR &&
                        info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                        const auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure);
                        if (src_as_state && src_as_state->build_info_khr.has_value()) {
                            if (geom_i < src_as_state->build_range_infos.size()) {
                                if (const uint32_t recorded_primitive_count =
                                        src_as_state->build_range_infos[geom_i].primitiveCount;
                                    recorded_primitive_count != geometry_build_ranges[geom_i].primitiveCount) {
                                    const LogObjectList objlist(cmd_buffer, info.srcAccelerationStructure);
                                    skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-primitiveCount-03769", objlist,
                                                     p_geom_loc,
                                                     " has corresponding VkAccelerationStructureBuildRangeInfoKHR %s[%" PRIu32
                                                     "], but this build range has its primitiveCount member set to (%" PRIu32
                                                     ") when it was last specified as (%" PRIu32 ").",
                                                     pp_build_range_info_loc.Fields().c_str(), geom_i,
                                                     geometry_build_ranges[geom_i].primitiveCount, recorded_primitive_count);
                                }
                            }
                        }
                    }
                }

                const VkFormatProperties3KHR format_properties = GetPDFormatProperties(geom_data.geometry.triangles.vertexFormat);
                if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)) {
                    skip |= LogError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexFormat-03797", cmd_buffer,
                                     p_geom_geom_triangles_loc.dot(Field::vertexFormat),
                                     "is %s which doesn't support VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR.\n"
                                     "(supported bufferFeatures: %s)",
                                     string_VkFormat(geom_data.geometry.triangles.vertexFormat),
                                     string_VkFormatFeatureFlags2(format_properties.bufferFeatures).c_str());
                }
                // Only try to get format info if vertex format is valid
                else {
                    const VKU_FORMAT_INFO format_info = vkuGetFormatInfo(geom_data.geometry.triangles.vertexFormat);
                    uint32_t min_component_bits_size = format_info.components[0].size;
                    for (uint32_t component_i = 1; component_i < format_info.component_count; ++component_i) {
                        min_component_bits_size = std::min(format_info.components[component_i].size, min_component_bits_size);
                    }
                    const uint32_t min_component_byte_size = min_component_bits_size / 8;
                    if (SafeModulo(geom_data.geometry.triangles.vertexData.deviceAddress, min_component_byte_size) != 0) {
                        skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03711",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03711"),
                                         cmd_buffer, p_geom_geom_triangles_loc.dot(Field::vertexData).dot(Field::deviceAddress),
                                         "is 0x%" PRIx64 " and is not aligned to the minimum component byte size (%" PRIu32
                                         ") of its corresponding vertex "
                                         "format (%s).",
                                         geom_data.geometry.triangles.vertexData.deviceAddress, min_component_byte_size,
                                         string_VkFormat(geom_data.geometry.triangles.vertexFormat));
                    }
                    if (SafeModulo(geom_data.geometry.triangles.vertexStride, min_component_byte_size) != 0) {
                        skip |= LogError("VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03735", cmd_buffer,
                                         p_geom_geom_triangles_loc.dot(Field::vertexStride),
                                         "is %" PRIu64 " and is not aligned to the minimum component byte size (%" PRIu32
                                         ") of its corresponding vertex "
                                         "format (%s).",
                                         geom_data.geometry.triangles.vertexStride, min_component_byte_size,
                                         string_VkFormat(geom_data.geometry.triangles.vertexFormat));
                    }
                }
                if (geom_data.geometry.triangles.transformData.deviceAddress != 0) {
                    auto tranform_buffer_states = GetBuffersByAddress(geom_data.geometry.triangles.transformData.deviceAddress);
                    if (tranform_buffer_states.empty() && geometry_build_ranges[geom_i].primitiveCount > 0) {
                        skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03808",
                                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03808"),
                                         cmd_buffer, p_geom_geom_triangles_loc.dot(Field::transformData).dot(Field::deviceAddress),
                                         "(0x%" PRIx64 ") is not an address belonging to an existing buffer.",
                                         geom_data.geometry.triangles.transformData.deviceAddress);
                    } else {
                        BufferAddressValidation<1> buffer_address_validator = {
                            {{{pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03809",
                                         "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03809"),
                               [this](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                                   return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state,
                                                                                                  out_error_msg);
                               },
                               []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }}}}};

                        skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
                            *this, tranform_buffer_states,
                            p_geom_geom_triangles_loc.dot(Field::transformData).dot(Field::deviceAddress),
                            LogObjectList(cmd_buffer), geom_data.geometry.triangles.transformData.deviceAddress);
                    }
                }
                break;
            }
            case VK_GEOMETRY_TYPE_INSTANCES_KHR: {
                const Location instances_loc = p_geom_geom_loc.dot(Field::instances);
                const Location instances_data_loc = instances_loc.dot(Field::data);

                skip |= buffer_check(geom_i, geom_data.geometry.instances.data, instances_data_loc);

                auto buffer_states = GetBuffersByAddress(geom_data.geometry.instances.data.deviceAddress);
                if (buffer_states.empty() && geometry_build_ranges[geom_i].primitiveCount > 0) {
                    skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03813",
                                               "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03813"),
                                     cmd_buffer, instances_data_loc.dot(Field::deviceAddress),
                                     "(%" PRIu64 ") is not an address belonging to an existing buffer.",
                                     geom_data.geometry.instances.data.deviceAddress);
                } else {
                    BufferAddressValidation<1> buffer_address_validator = {
                        {{{pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03814",
                                     "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03814"),
                           [this](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                               return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state, out_error_msg);
                           },
                           []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }}}}};

                    skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
                        *this, buffer_states, instances_data_loc.dot(Field::deviceAddress), LogObjectList(cmd_buffer),
                        geom_data.geometry.instances.data.deviceAddress);
                }

                break;
            }
            case VK_GEOMETRY_TYPE_AABBS_KHR:  // == VK_GEOMETRY_TYPE_AABBS_NV
            {
                skip |= buffer_check(geom_i, geom_data.geometry.aabbs.data, p_geom_geom_loc.dot(Field::aabbs).dot(Field::data));

                auto aabb_buffer_states = GetBuffersByAddress(geom_data.geometry.aabbs.data.deviceAddress);
                if (aabb_buffer_states.empty() && geometry_build_ranges[geom_i].primitiveCount > 0) {
                    skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03811",
                                               "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03811"),
                                     cmd_buffer, p_geom_geom_loc.dot(Field::aabbs).dot(Field::data).dot(Field::deviceAddress),
                                     "(0x%" PRIx64 ") is not an address belonging to an existing buffer.",
                                     geom_data.geometry.aabbs.data.deviceAddress);
                } else {
                    BufferAddressValidation<1> buffer_address_validator = {
                        {{{pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03812",
                                     "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03812"),
                           [this](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                               return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state, out_error_msg);
                           },
                           []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }}}}};

                    skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
                        *this, aabb_buffer_states, p_geom_geom_loc.dot(Field::aabbs).dot(Field::data).dot(Field::deviceAddress),
                        LogObjectList(cmd_buffer), geom_data.geometry.aabbs.data.deviceAddress);
                }
                break;
            }
            default:
                // no-op
                break;
        }
    }

    if (info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR &&
        !(info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR &&
          (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR))) {
        if (const auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure)) {
            const VkDeviceSize as_minimum_size =
                rt::ComputeAccelerationStructureSize(rt::BuildType::Device, device, info, geometry_build_ranges);
            if (dst_as_state->create_info.size < as_minimum_size) {
                const LogObjectList objlist(cmd_buffer, info.dstAccelerationStructure);
                skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-10126", objlist,
                                 info_loc.dot(Field::dstAccelerationStructure),
                                 " was created with size (%" PRIu64
                                 "), but an acceleration structure build with corresponding ppBuildRangeInfos[%" PRIu32
                                 "] requires a minimum size of (%" PRIu64 ").",
                                 dst_as_state->create_info.size, info_i, as_minimum_size);
            }
        }
    }

    const auto buffer_states = GetBuffersByAddress(info.scratchData.deviceAddress);
    if (buffer_states.empty()) {
        skip |= LogError(pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802",
                                   "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03802"),
                         device, info_loc.dot(Field::scratchData).dot(Field::deviceAddress),
                         "(0x%" PRIx64 ") has no buffer is associated with it.", info.scratchData.deviceAddress);
    } else {
        // Hardcoded value of 1 for indirect calls because scratch size cannot be computed on the CPU in this case
        // (need to access build ranges). Easier to hardcode than to add the logic to not perform scratch buffer size
        // validation for indirect calls.
        const VkDeviceSize scratch_size = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                              ? rt::ComputeScratchSize(rt::BuildType::Device, device, info, geometry_build_ranges)
                                              : 1;
        const sparse_container::range<VkDeviceSize> scratch_address_range(info.scratchData.deviceAddress,
                                                                          info.scratchData.deviceAddress + scratch_size);
        const char *scratch_address_range_vuid = info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR
                                                     ? pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03671",
                                                                 "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03671")
                                                     : pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03672",
                                                                 "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03672");

        BufferAddressValidation<3> buffer_address_validator = {{{
            {pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03803",
                       "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03803"),
             [this](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                 return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state, out_error_msg);
             },
             []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }},

            {pick_vuid("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03674",
                       "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03674"),
             [](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                 if (!(buffer_state->usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) {
                     if (out_error_msg) {
                         *out_error_msg += "buffer usage is " + string_VkBufferUsageFlags2(buffer_state->usage) + '\n';
                     }
                     return false;
                 }
                 return true;
             },
             []() { return "The following buffers are missing VK_BUFFER_USAGE_STORAGE_BUFFER_BIT usage flag:"; }},

            {scratch_address_range_vuid,
             [scratch_address_range](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                 const sparse_container::range<VkDeviceSize> buffer_address_range = buffer_state->DeviceAddressRange();

                 if (!buffer_address_range.includes(scratch_address_range)) {
                     if (out_error_msg) {
                         *out_error_msg += "buffer address range is " + string_range_hex(buffer_address_range) + '\n';
                     }
                     return false;
                 }
                 return true;
             },
             [scratch_address_range]() {
                 return "The following buffers have an address range that does not include scratch address range " +
                        string_range_hex(scratch_address_range) + ":";
             }},

        }}};

        skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(*this, buffer_states,
                                                                  info_loc.dot(Field::scratchData).dot(Field::deviceAddress),
                                                                  LogObjectList(cmd_buffer), info.scratchData.deviceAddress);
    }

    return skip;
}

bool CoreChecks::CommonBuildAccelerationStructureValidation(const VkAccelerationStructureBuildGeometryInfoKHR &info,
                                                            const Location &info_loc, LogObjectList object_list) const {
    bool skip = false;

    const auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure);
    if (!src_as_state) return skip;

    if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
        if (!src_as_state->is_built) {
            const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                   ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03667"
                               : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                   ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03667"
                                   : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03667";
            LogObjectList objlist = object_list;
            objlist.add(info.srcAccelerationStructure);
            skip |= LogError(
                vuid, objlist, info_loc.dot(Field::mode),
                "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, srcAccelerationStructure must have been previously built");
        } else if (src_as_state->build_info_khr.has_value()) {
            if (!(src_as_state->build_info_khr->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR)) {
                const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03667"
                                   : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03667"
                                       : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03667";
                LogObjectList objlist = object_list;
                objlist.add(info.srcAccelerationStructure);
                skip |= LogError(vuid, objlist, info_loc.dot(Field::mode),
                                 "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, srcAccelerationStructure has been previously "
                                 "constructed with flags %s.",
                                 string_VkBuildAccelerationStructureFlagsKHR(src_as_state->build_info_khr->flags).c_str());
            }

            if (info.flags != src_as_state->build_info_khr->flags) {
                const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03759"
                                   : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03759"
                                       : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03759";
                LogObjectList objlist = object_list;
                objlist.add(info.srcAccelerationStructure);
                skip |= LogError(vuid, objlist, info_loc.dot(Field::mode),
                                 "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but %s (%s) must have the same value as "
                                 "specified when srcAccelerationStructure was last built (%s).",
                                 info_loc.dot(Field::flags).Fields().c_str(),
                                 string_VkBuildAccelerationStructureFlagsKHR(info.flags).c_str(),
                                 string_VkBuildAccelerationStructureFlagsKHR(src_as_state->build_info_khr->flags).c_str());
            }

            if (info.type != src_as_state->build_info_khr->type) {
                const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03760"
                                   : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03760"
                                       : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03760";

                LogObjectList objlist = object_list;
                objlist.add(info.srcAccelerationStructure);
                skip |= LogError(vuid, objlist, info_loc.dot(Field::mode),
                                 "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR, but type (%s) must have the same value as "
                                 "specified when srcAccelerationStructure was last built (%s).",
                                 string_VkAccelerationStructureTypeKHR(info.type),
                                 string_VkAccelerationStructureTypeKHR(src_as_state->build_info_khr->type));
            }

            if (info.geometryCount != src_as_state->build_info_khr->geometryCount) {
                const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03758"
                                   : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                       ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03758"
                                       : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03758";

                LogObjectList objlist = object_list;
                objlist.add(info.srcAccelerationStructure);
                skip |= LogError(vuid, objlist, info_loc.dot(Field::mode),
                                 "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR,"
                                 " but geometryCount (%" PRIu32
                                 ") must have the same value as specified when "
                                 "srcAccelerationStructure was last built (%" PRIu32 ").",
                                 info.geometryCount, src_as_state->build_info_khr->geometryCount);
            } else if (info.pGeometries || info.ppGeometries) {
                for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
                    const VkAccelerationStructureGeometryKHR &updated_geometry =
                        info.pGeometries ? info.pGeometries[geom_i] : *info.ppGeometries[geom_i];

                    const vku::safe_VkAccelerationStructureGeometryKHR &last_geometry =
                        src_as_state->build_info_khr->pGeometries ? src_as_state->build_info_khr->pGeometries[geom_i]
                                                                  : *src_as_state->build_info_khr->ppGeometries[geom_i];

                    const Location geom_loc = info_loc.dot(info.pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);

                    if (updated_geometry.geometryType != last_geometry.geometryType) {
                        const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                               ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03761"
                                           : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                               ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03761"
                                               : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03761";

                        LogObjectList objlist = object_list;
                        objlist.add(info.srcAccelerationStructure);
                        skip |= LogError(vuid, objlist, geom_loc.dot(Field::geometryType), "is %s but was last specified as %s.",
                                         string_VkGeometryTypeKHR(updated_geometry.geometryType),
                                         string_VkGeometryTypeKHR(last_geometry.geometryType));
                    }

                    if (updated_geometry.flags != last_geometry.flags) {
                        const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                               ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03762"
                                           : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                               ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03762"
                                               : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03762";

                        LogObjectList objlist = object_list;
                        objlist.add(info.srcAccelerationStructure);
                        skip |= LogError(vuid, objlist, geom_loc.dot(Field::flags), "is %s but was last specified as %s.",
                                         string_VkGeometryFlagsKHR(updated_geometry.flags).c_str(),
                                         string_VkGeometryFlagsKHR(last_geometry.flags).c_str());
                    }

                    if (updated_geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
                        if (updated_geometry.geometry.triangles.vertexFormat != last_geometry.geometry.triangles.vertexFormat) {
                            const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                                   ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03763"
                                               : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                                   ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03763"
                                                   : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03763";

                            LogObjectList objlist = object_list;
                            objlist.add(info.srcAccelerationStructure);
                            skip |= LogError(vuid, objlist,
                                             geom_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::vertexFormat),
                                             "is %s but was last specified as %s.",
                                             string_VkFormat(updated_geometry.geometry.triangles.vertexFormat),
                                             string_VkFormat(last_geometry.geometry.triangles.vertexFormat));
                        }

                        if (updated_geometry.geometry.triangles.maxVertex != last_geometry.geometry.triangles.maxVertex) {
                            const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                                   ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03764"
                                               : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                                   ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03764"
                                                   : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03764";

                            LogObjectList objlist = object_list;
                            objlist.add(info.srcAccelerationStructure);
                            skip |=
                                LogError(vuid, objlist, geom_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::maxVertex),
                                         "is %" PRIu32 " but was last specified as %" PRIu32 ".",
                                         updated_geometry.geometry.triangles.maxVertex, last_geometry.geometry.triangles.maxVertex);
                        }

                        if (updated_geometry.geometry.triangles.indexType != last_geometry.geometry.triangles.indexType) {
                            const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                                   ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03765"
                                               : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                                   ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03765"
                                                   : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03765";

                            LogObjectList objlist = object_list;
                            objlist.add(info.srcAccelerationStructure);
                            skip |=
                                LogError(vuid, objlist, geom_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::indexType),
                                         "is %s but was last specified as %s.",
                                         string_VkIndexType(updated_geometry.geometry.triangles.indexType),
                                         string_VkIndexType(last_geometry.geometry.triangles.indexType));
                        }

                        if (last_geometry.geometry.triangles.transformData.deviceAddress == 0 &&
                            updated_geometry.geometry.triangles.transformData.deviceAddress != 0) {
                            const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                                   ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03766"
                                               : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                                   ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03766"
                                                   : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03766";

                            LogObjectList objlist = object_list;
                            objlist.add(info.srcAccelerationStructure);
                            skip |= LogError(vuid, objlist,
                                             geom_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::transformData),
                                             "is 0x%" PRIx64 " but was last specified as NULL.",
                                             updated_geometry.geometry.triangles.transformData.deviceAddress);
                        }

                        if (last_geometry.geometry.triangles.transformData.deviceAddress != 0 &&
                            updated_geometry.geometry.triangles.transformData.deviceAddress == 0) {
                            const char *vuid = info_loc.function == Func::vkCmdBuildAccelerationStructuresKHR
                                                   ? "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03767"
                                               : info_loc.function == Func::vkCmdBuildAccelerationStructuresIndirectKHR
                                                   ? "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03767"
                                                   : "VUID-vkBuildAccelerationStructuresKHR-pInfos-03767";

                            LogObjectList objlist = object_list;
                            objlist.add(info.srcAccelerationStructure);
                            skip |= LogError(vuid, objlist,
                                             geom_loc.dot(Field::geometry).dot(Field::triangles).dot(Field::transformData),
                                             "is NULL but was last specified as 0x%" PRIx64 ".",
                                             last_geometry.geometry.triangles.transformData.deviceAddress);
                        }
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const {
    using sparse_container::range;
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (!pInfos || !ppBuildRangeInfos) {
        return skip;
    }

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        if (const auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure)) {
            if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                if (!src_as_state->buffer_state) {
                    const LogObjectList objlist(device, commandBuffer, info.srcAccelerationStructure);
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03708", objlist, info_loc.dot(Field::mode),
                                     "is VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR but the buffer associated with "
                                     "srcAccelerationStructure is not valid.");
                } else {
                    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_as_state->buffer_state,
                                                          info_loc.dot(Field::srcAccelerationStructure),
                                                          "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03708");
                }
            }
        }

        if (const auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure)) {
            skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_as_state->buffer_state,
                                                  info_loc.dot(Field::dstAccelerationStructure),
                                                  "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03707");
            if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
                if (dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
                    dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
                    const LogObjectList objlist(device, commandBuffer);
                    skip |= LogError(
                        "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03700", objlist, info_loc.dot(Field::type),
                        "is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, but its dstAccelerationStructure was created with %s.",
                        string_VkAccelerationStructureTypeKHR(dst_as_state->create_info.type));
                }
            }
            if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
                if (dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                    dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
                    const LogObjectList objlist(device, commandBuffer);
                    skip |= LogError(
                        "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03699", objlist, info_loc.dot(Field::type),
                        "is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, but its dstAccelerationStructure was created with %s.",
                        string_VkAccelerationStructureTypeKHR(dst_as_state->create_info.type));
                }
            }
        }

        skip |= CommonBuildAccelerationStructureValidation(info, info_loc, commandBuffer);

        skip |= ValidateAccelerationBuffers(commandBuffer, info_i, info, ppBuildRangeInfos[info_i], info_loc);

        skip |= ValidateAccelerationStructuresMemoryAlisasing(commandBuffer, infoCount, pInfos, info_i, error_obj);

        skip |= ValidateAccelerationStructuresDeviceScratchBufferMemoryAlisasing(commandBuffer, infoCount, pInfos, info_i,
                                                                                 ppBuildRangeInfos, error_obj);
    }

    return skip;
}

bool CoreChecks::PreCallValidateBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkBuildAccelerationStructuresKHR-deferredOperation-03678");

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);
        auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure);
        auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure);

        if (src_as_state) {
            if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                skip |= ValidateAccelStructBufferMemoryIsHostVisible(*src_as_state, info_loc.dot(Field::srcAccelerationStructure),
                                                                     "VUID-vkBuildAccelerationStructuresKHR-pInfos-03723");
                skip |=
                    ValidateAccelStructBufferMemoryIsNotMultiInstance(*src_as_state, info_loc.dot(Field::srcAccelerationStructure),
                                                                      "VUID-vkBuildAccelerationStructuresKHR-pInfos-03776");
            }
        }

        if (dst_as_state) {
            skip |= ValidateAccelStructBufferMemoryIsHostVisible(*dst_as_state, info_loc.dot(Field::dstAccelerationStructure),
                                                                 "VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");

            skip |= ValidateAccelStructBufferMemoryIsNotMultiInstance(*dst_as_state, info_loc.dot(Field::dstAccelerationStructure),
                                                                      "VUID-vkBuildAccelerationStructuresKHR-pInfos-03775");

            if (!(info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR &&
                  (info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR))) {
                const VkDeviceSize as_minimum_size =
                    rt::ComputeAccelerationStructureSize(rt::BuildType::Host, device, info, ppBuildRangeInfos[info_i]);
                if (dst_as_state->create_info.size < as_minimum_size) {
                    const LogObjectList objlist(info.dstAccelerationStructure);
                    skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-10126", objlist,
                                     info_loc.dot(Field::dstAccelerationStructure),
                                     " was created with size (%" PRIu64
                                     "), but an acceleration structure build with corresponding ppBuildRangeInfos[%" PRIu32
                                     "] requires a minimum size of (%" PRIu64 ").",
                                     dst_as_state->create_info.size, info_i, as_minimum_size);
                }
            }

            if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
                if (dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
                    dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
                    skip |= LogError("VUID-vkBuildAccelerationStructuresKHR-pInfos-03700", info.dstAccelerationStructure,
                                     info_loc.dot(Field::type),
                                     "is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, but its dstAccelerationStructure was "
                                     "built with type %s.",
                                     string_VkAccelerationStructureTypeKHR(dst_as_state->create_info.type));
                }
            }

            if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
                if (dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                    dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
                    skip |= LogError(
                        "VUID-vkBuildAccelerationStructuresKHR-pInfos-03699", info.dstAccelerationStructure,
                        info_loc.dot(Field::type),
                        "is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, but its dstAccelerationStructure was built with type %s.",
                        string_VkAccelerationStructureTypeKHR(dst_as_state->create_info.type));
                }
            }
        }

        skip |= ValidateAccelerationStructuresMemoryAlisasing(LogObjectList(), infoCount, pInfos, info_i, error_obj);
        const VkDeviceSize scratch_i_size = rt::ComputeScratchSize(rt::BuildType::Host, device, info, ppBuildRangeInfos[info_i]);
        auto scratch_i_host_addr = reinterpret_cast<uint64_t>(info.scratchData.hostAddress);
        const sparse_container::range<uint64_t> scratch_addr_range(scratch_i_host_addr, scratch_i_host_addr + scratch_i_size);

        assert(infoCount > info_i);
        for (auto [other_info_j, other_info] : vvl::enumerate(pInfos + info_i + 1, infoCount - (info_i + 1))) {
            const Location other_info_j_loc = error_obj.location.dot(Field::pInfos, other_info_j + info_i + 1);
            const VkDeviceSize other_scratch_size =
                rt::ComputeScratchSize(rt::BuildType::Host, device, other_info, ppBuildRangeInfos[other_info_j]);
            auto other_scratch_host_addr = reinterpret_cast<uint64_t>(other_info.scratchData.hostAddress);
            const sparse_container::range<uint64_t> other_scratch_addr_range(other_scratch_host_addr,
                                                                             other_scratch_host_addr + other_scratch_size);

            if (scratch_addr_range.intersects(other_scratch_addr_range)) {
                std::string info_i_scratch_str = info_loc.dot(Field::scratchData).Fields();
                std::string other_info_j_scratch_str = other_info_j_loc.dot(Field::scratchData).Fields();
                skip |= LogError(
                    "VUID-vkBuildAccelerationStructuresKHR-scratchData-03704", device, info_loc.dot(Field::scratchData),
                    "overlaps with %s on host address range %s.\n"
                    "%s.hostAddress is %p and assumed scratch size is %" PRIu64
                    ".\n"
                    "%s.hostAddress is %p and assumed scratch size is %" PRIu64 ".",
                    other_info_j_scratch_str.c_str(), string_range_hex(scratch_addr_range & other_scratch_addr_range).c_str(),
                    info_i_scratch_str.c_str(), info.scratchData.hostAddress, scratch_i_size, other_info_j_scratch_str.c_str(),
                    other_info.scratchData.hostAddress, other_scratch_size);
            }
        }

        skip |= CommonBuildAccelerationStructureValidation(info, info_loc, LogObjectList());

        for (uint32_t geom_i = 0; geom_i < info.geometryCount; ++geom_i) {
            const VkAccelerationStructureGeometryKHR &geom = rt::GetGeometry(info, geom_i);

            if (geom.geometryType != VK_GEOMETRY_TYPE_INSTANCES_KHR) {
                continue;
            }

            const Location geometry_loc = info_loc.dot(info.pGeometries ? Field::pGeometries : Field::ppGeometries, geom_i);

            if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR && src_as_state &&
                src_as_state->build_info_khr.has_value()) {
                if (geom_i < src_as_state->build_range_infos.size()) {
                    if (const uint32_t recorded_primitive_count = src_as_state->build_range_infos[geom_i].primitiveCount;
                        recorded_primitive_count != ppBuildRangeInfos[info_i][geom_i].primitiveCount) {
                        const LogObjectList objlist(info.srcAccelerationStructure);
                        skip |= LogError("VUID-vkCmdBuildAccelerationStructuresKHR-primitiveCount-03769", objlist, geometry_loc,
                                         " has corresponding VkAccelerationStructureBuildRangeInfoKHR %s[%" PRIu32
                                         "], but this build range has its primitiveCount member set to (%" PRIu32
                                         ") when it was last specified as (%" PRIu32 ").",
                                         error_obj.location.dot(Field::ppBuildRangeInfos, info_i).Fields().c_str(), geom_i,
                                         ppBuildRangeInfos[info_i][geom_i].primitiveCount, recorded_primitive_count);
                    }
                }
            }

            for (uint32_t instance_i = 0; instance_i < ppBuildRangeInfos[info_i][geom_i].primitiveCount; ++instance_i) {
                const VkAccelerationStructureInstanceKHR *instance = nullptr;
                if (geom.geometry.instances.data.hostAddress) {
                    if (geom.geometry.instances.arrayOfPointers) {
                        auto instance_pointers_array = reinterpret_cast<VkAccelerationStructureInstanceKHR const *const *>(
                            geom.geometry.instances.data.hostAddress);
                        instance = instance_pointers_array[instance_i];
                    } else {
                        auto instances_array =
                            reinterpret_cast<VkAccelerationStructureInstanceKHR const *>(geom.geometry.instances.data.hostAddress);
                        instance = instances_array + instance_i;
                    }

                    // Can only get here if geom.geometry.instances.arrayOfPointers is true
                    if (!instance) {
                        skip |= LogError(
                            "VUID-vkBuildAccelerationStructuresKHR-pInfos-03779", device,
                            geometry_loc.dot(Field::geometry)
                                .dot(Field::instances)
                                .dot(Field::data)
                                .dot(Field::hostAddress, instance_i),
                            "(0x%p) does not reference a valid VkAccelerationStructureKHR object. %s is %s.", instance,
                            geometry_loc.dot(Field::geometry).dot(Field::instances).dot(Field::arrayOfPointers).Fields().c_str(),
                            string_VkBool32(geom.geometry.instances.arrayOfPointers).c_str());

                        // Following checks rely on instance not being null
                        continue;
                    }

                    const VkAccelerationStructureKHR accel_struct =
                        CastFromUint64<VkAccelerationStructureKHR>(instance->accelerationStructureReference);
                    auto accel_struct_state = Get<vvl::AccelerationStructureKHR>(accel_struct);

                    if (!accel_struct_state) {
                        skip |= LogError(
                            "VUID-vkBuildAccelerationStructuresKHR-pInfos-03779", device,
                            geometry_loc.dot(Field::geometry)
                                .dot(Field::instances)
                                .dot(Field::data)
                                .dot(Field::hostAddress, instance_i)
                                .dot(Field::accelerationStructureReference),
                            "(%" PRIu64 ") does not reference a valid VkAccelerationStructureKHR object. %s is %s.",
                            instance->accelerationStructureReference,
                            geometry_loc.dot(Field::geometry).dot(Field::instances).dot(Field::arrayOfPointers).Fields().c_str(),
                            string_VkBool32(geom.geometry.instances.arrayOfPointers).c_str());
                    }
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                          const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                                          const VkDeviceAddress *pIndirectDeviceAddresses,
                                                                          const uint32_t *pIndirectStrides,
                                                                          const uint32_t *const *ppMaxPrimitiveCounts,
                                                                          const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    if (!cb_state->unprotected) {
        skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-commandBuffer-09547", commandBuffer, error_obj.location,
                         "called in a protected command buffer.");
    }

    for (const auto [info_i, info] : vvl::enumerate(pInfos, infoCount)) {
        const Location info_loc = error_obj.location.dot(Field::pInfos, info_i);

        if (auto src_as_state = Get<vvl::AccelerationStructureKHR>(info.srcAccelerationStructure)) {
            if (info.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
                skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_as_state->buffer_state,
                                                      info_loc.dot(Field::srcAccelerationStructure),
                                                      "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03708");
            }
        }

        if (auto dst_as_state = Get<vvl::AccelerationStructureKHR>(info.dstAccelerationStructure)) {
            skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_as_state->buffer_state,
                                                  info_loc.dot(Field::dstAccelerationStructure),
                                                  "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03707");
            if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
                if (dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR &&
                    dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03700", info.dstAccelerationStructure,
                                     info_loc.dot(Field::type),
                                     "is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, but its dstAccelerationStructure was "
                                     "built with type %s.",
                                     string_VkAccelerationStructureTypeKHR(dst_as_state->create_info.type));
                }
            }

            if (info.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
                if (dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                    dst_as_state->create_info.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR) {
                    skip |= LogError(
                        "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-pInfos-03699", info.dstAccelerationStructure,
                        info_loc.dot(Field::type),
                        "is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, but its dstAccelerationStructure was built with type %s.",
                        string_VkAccelerationStructureTypeKHR(dst_as_state->create_info.type));
                }
            }
        }

        skip |= ValidateAccelerationStructuresMemoryAlisasing(commandBuffer, infoCount, pInfos, info_i, error_obj);

        skip |= CommonBuildAccelerationStructureValidation(info, info_loc, commandBuffer);

        skip |= ValidateAccelerationBuffers(commandBuffer, info_i, info, nullptr, info_loc);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData,
                                                                VkDeviceSize instanceOffset, VkBool32 update,
                                                                VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                                VkBuffer scratch, VkDeviceSize scratchOffset,
                                                                const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (pInfo != nullptr && pInfo->type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV) {
        for (uint32_t i = 0; i < pInfo->geometryCount; i++) {
            skip |= ValidateGeometryNV(pInfo->pGeometries[i], error_obj.location.dot(Field::pInfo).dot(Field::pGeometries, i));
        }
    }

    if (pInfo != nullptr && pInfo->geometryCount > phys_dev_ext_props.ray_tracing_props_nv.maxGeometryCount) {
        skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-geometryCount-02241", commandBuffer, error_obj.location,
                         "geometryCount [%" PRIu32
                         "] must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxGeometryCount.",
                         pInfo->geometryCount);
    }

    auto dst_as_state = Get<vvl::AccelerationStructureNV>(dst);
    auto src_as_state = Get<vvl::AccelerationStructureNV>(src);

    if (dst_as_state && pInfo) {
        if (dst_as_state->create_info.info.type != pInfo->type) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                             "create info VkAccelerationStructureInfoNV::type"
                             "[%s] must be identical to build info VkAccelerationStructureInfoNV::type [%s].",
                             string_VkAccelerationStructureTypeKHR(dst_as_state->create_info.info.type),
                             string_VkAccelerationStructureTypeKHR(pInfo->type));
        }
        if (dst_as_state->create_info.info.flags != pInfo->flags) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                             "create info VkAccelerationStructureInfoNV::flags"
                             "[%s] must be identical to build info VkAccelerationStructureInfoNV::flags [%s].",
                             string_VkBuildAccelerationStructureFlagsKHR(dst_as_state->create_info.info.flags).c_str(),
                             string_VkBuildAccelerationStructureFlagsKHR(pInfo->flags).c_str());
        }
        if (dst_as_state->create_info.info.instanceCount < pInfo->instanceCount) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                             "create info VkAccelerationStructureInfoNV::instanceCount "
                             "[%" PRIu32
                             "] must be greater than or equal to build info VkAccelerationStructureInfoNV::instanceCount [%" PRIu32
                             "].",
                             dst_as_state->create_info.info.instanceCount, pInfo->instanceCount);
        }
        if (dst_as_state->create_info.info.geometryCount < pInfo->geometryCount) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                             "create info VkAccelerationStructureInfoNV::geometryCount"
                             "[%" PRIu32
                             "] must be greater than or equal to build info VkAccelerationStructureInfoNV::geometryCount [%" PRIu32
                             "].",
                             dst_as_state->create_info.info.geometryCount, pInfo->geometryCount);
        } else {
            for (uint32_t i = 0; i < pInfo->geometryCount; i++) {
                const VkGeometryDataNV &create_geometry_data = dst_as_state->create_info.info.pGeometries[i].geometry;
                const VkGeometryDataNV &build_geometry_data = pInfo->pGeometries[i].geometry;
                if (create_geometry_data.triangles.vertexCount < build_geometry_data.triangles.vertexCount) {
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                                     "create info pGeometries[%" PRIu32 "].geometry.triangles.vertexCount [%" PRIu32
                                     "]"
                                     "must be greater than or equal to build info pGeometries[%" PRIu32
                                     "].geometry.triangles.vertexCount [%" PRIu32 "].",
                                     i, create_geometry_data.triangles.vertexCount, i, build_geometry_data.triangles.vertexCount);
                    break;
                }
                if (create_geometry_data.triangles.indexCount < build_geometry_data.triangles.indexCount) {
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                                     "create info pGeometries[%" PRIu32 "].geometry.triangles.indexCount [%" PRIu32
                                     "]"
                                     "must be greater than or equal to build info pGeometries[%" PRIu32
                                     "].geometry.triangles.indexCount [%" PRIu32 "].",
                                     i, create_geometry_data.triangles.indexCount, i, build_geometry_data.triangles.indexCount);
                    break;
                }
                if (create_geometry_data.aabbs.numAABBs < build_geometry_data.aabbs.numAABBs) {
                    skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-dst-02488", commandBuffer, error_obj.location,
                                     "create info pGeometries[%" PRIu32 "].geometry.aabbs.numAABBs [%" PRIu32
                                     "]"
                                     "must be greater than or equal to build info pGeometries[%" PRIu32
                                     "].geometry.aabbs.numAABBs [%" PRIu32 "].",
                                     i, create_geometry_data.aabbs.numAABBs, i, build_geometry_data.aabbs.numAABBs);
                    break;
                }
            }
        }
    }

    if (dst_as_state) {
        skip |= VerifyBoundMemoryIsValid(dst_as_state->MemoryState(), LogObjectList(commandBuffer, dst), dst_as_state->Handle(),
                                         error_obj.location.dot(Field::dst), "VUID-vkCmdBuildAccelerationStructureNV-dst-07787");
    }

    auto scratch_buffer_state = Get<vvl::Buffer>(scratch);
    if (update == VK_TRUE) {
        if (src == VK_NULL_HANDLE) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-update-02489", commandBuffer, error_obj.location,
                             "If update is VK_TRUE, src must not be VK_NULL_HANDLE.");
        } else {
            if (!src_as_state || !src_as_state->built ||
                !(src_as_state->build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV)) {
                skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-update-02490", commandBuffer, error_obj.location,
                                 "If update is VK_TRUE, src must have been built before "
                                 "with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV set in "
                                 "VkAccelerationStructureInfoNV::flags.");
            }
        }
        if (dst_as_state && !dst_as_state->update_scratch_memory_requirements_checked) {
            skip |= LogWarning("WARNING-vkCmdBuildAccelerationStructureNV-update-requirements", dst, error_obj.location,
                               "Updating %s but vkGetAccelerationStructureMemoryRequirementsNV() "
                               "has not been called for update scratch memory.",
                               FormatHandle(dst_as_state->Handle()).c_str());
            // Use requirements fetched at create time
        }
        if (scratch_buffer_state && dst_as_state &&
            dst_as_state->update_scratch_memory_requirements.size > (scratch_buffer_state->create_info.size - scratchOffset)) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-update-02492", commandBuffer, error_obj.location,
                             "If update is VK_TRUE, The size member of the "
                             "VkMemoryRequirements structure returned from a call to "
                             "vkGetAccelerationStructureMemoryRequirementsNV with "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::accelerationStructure set to dst and "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::type set to "
                             "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV must be less than "
                             "or equal to the size of scratch minus scratchOffset");
        }
    } else {
        if (dst_as_state && !dst_as_state->build_scratch_memory_requirements_checked) {
            skip |= LogWarning("WARNING-vkCmdBuildAccelerationStructureNV-scratch-requirements", dst, error_obj.location,
                               "Assigning scratch buffer to %s but "
                               "vkGetAccelerationStructureMemoryRequirementsNV() has not been called for scratch memory.",
                               FormatHandle(dst_as_state->Handle()).c_str());
            // Use requirements fetched at create time
        }
        if (scratch_buffer_state && dst_as_state &&
            dst_as_state->build_scratch_memory_requirements.size > (scratch_buffer_state->create_info.size - scratchOffset)) {
            skip |= LogError("VUID-vkCmdBuildAccelerationStructureNV-update-02491", commandBuffer, error_obj.location,
                             "If update is VK_FALSE, The size member of the "
                             "VkMemoryRequirements structure returned from a call to "
                             "vkGetAccelerationStructureMemoryRequirementsNV with "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::accelerationStructure set to dst and "
                             "VkAccelerationStructureMemoryRequirementsInfoNV::type set to "
                             "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV must be less than "
                             "or equal to the size of scratch minus scratchOffset");
        }
    }
    if (instanceData != VK_NULL_HANDLE) {
        if (auto buffer_state = Get<vvl::Buffer>(instanceData)) {
            skip |= ValidateBufferUsageFlags(
                LogObjectList(commandBuffer, instanceData), *buffer_state, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, true,
                "VUID-VkAccelerationStructureInfoNV-instanceData-02782", error_obj.location.dot(Field::instanceData));
        }
    }
    if (scratch_buffer_state) {
        skip |= ValidateBufferUsageFlags(
            LogObjectList(commandBuffer, scratch), *scratch_buffer_state, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, true,
            "VUID-VkAccelerationStructureInfoNV-scratch-02781", error_obj.location.dot(Field::scratch));
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                               VkAccelerationStructureNV src,
                                                               VkCopyAccelerationStructureModeNV mode,
                                                               const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    skip |= ValidateCmd(*cb_state, error_obj.location);
    auto dst_as_state = Get<vvl::AccelerationStructureNV>(dst);
    auto src_as_state = Get<vvl::AccelerationStructureNV>(src);

    if (dst_as_state) {
        const LogObjectList objlist(commandBuffer, dst);
        skip |= VerifyBoundMemoryIsValid(dst_as_state->MemoryState(), objlist, dst_as_state->Handle(),
                                         error_obj.location.dot(Field::dst), "VUID-vkCmdCopyAccelerationStructureNV-dst-07792");
        skip |= VerifyBoundMemoryIsDeviceVisible(dst_as_state->MemoryState(), objlist, dst_as_state->Handle(),
                                                 error_obj.location.dot(Field::dst),
                                                 "VUID-vkCmdCopyAccelerationStructureNV-buffer-03719");
    }
    if (src_as_state) {
        const LogObjectList objlist(commandBuffer, src);
        skip |= VerifyBoundMemoryIsDeviceVisible(src_as_state->MemoryState(), objlist, src_as_state->Handle(),
                                                 error_obj.location.dot(Field::src),
                                                 "VUID-vkCmdCopyAccelerationStructureNV-buffer-03718");
        if (!src_as_state->built) {
            skip |= LogError("VUID-vkCmdCopyAccelerationStructureNV-src-04963", commandBuffer, error_obj.location,
                             "The source acceleration structure src has not yet been built.");
        }
    }

    if (mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV) {
        if (src_as_state &&
            (!src_as_state->built || !(src_as_state->build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV))) {
            skip |= LogError("VUID-vkCmdCopyAccelerationStructureNV-src-03411", commandBuffer, error_obj.location,
                             "src must have been built with "
                             "VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV if mode is "
                             "VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV.");
        }
    }
    if (!(mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV || mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR)) {
        skip |= LogError("VUID-vkCmdCopyAccelerationStructureNV-mode-03410", commandBuffer, error_obj.location,
                         "mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR"
                         "or VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto as_state = Get<vvl::AccelerationStructureNV>(accelerationStructure)) {
        skip |= ValidateObjectNotInUse(as_state.get(), error_obj.location,
                                       "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-03752");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto as_state = Get<vvl::AccelerationStructureKHR>(accelerationStructure)) {
        skip |= ValidateObjectNotInUse(as_state.get(), error_obj.location,
                                       "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-02442");
    }
    return skip;
}

void CoreChecks::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const RecordObject &record_obj) {
    if (disabled[query_validation]) return;
    // Enqueue the submit time validation check here, before the submit time state update in BaseClass::PostCall...
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    cb_state->query_updates.emplace_back([accelerationStructureCount, firstQuery, queryPool](
                                             vvl::CommandBuffer &cb_state_arg, bool do_validate, VkQueryPool &firstPerfQueryPool,
                                             uint32_t perfPass, QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        bool skip = false;
        for (uint32_t i = 0; i < accelerationStructureCount; i++) {
            QueryObject query_obj = {queryPool, firstQuery + i, perfPass};
            skip |= VerifyQueryIsReset(cb_state_arg, query_obj, Func::vkCmdWriteAccelerationStructuresPropertiesKHR,
                                       firstPerfQueryPool, perfPass, localQueryToStateMap);
            (*localQueryToStateMap)[query_obj] = QUERYSTATE_ENDED;
        }
        return skip;
    });
}

bool CoreChecks::PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                         const VkAccelerationStructureKHR *pAccelerationStructures,
                                                                         VkQueryType queryType, size_t dataSize, void *pData,
                                                                         size_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
        const Location as_loc = error_obj.location.dot(Field::pAccelerationStructures, i);
        auto as_state = Get<vvl::AccelerationStructureKHR>(pAccelerationStructures[i]);
        ASSERT_AND_CONTINUE(as_state);

        skip |= ValidateAccelStructBufferMemoryIsHostVisible(*as_state, as_loc,
                                                             "VUID-vkWriteAccelerationStructuresPropertiesKHR-buffer-03733");

        skip |= ValidateAccelStructBufferMemoryIsNotMultiInstance(*as_state, as_loc,
                                                                  "VUID-vkWriteAccelerationStructuresPropertiesKHR-buffer-03784");

        if (!as_state->is_built) {
            skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-04964", device, as_loc,
                             "has not been built.");
        } else if (as_state->build_info_khr.has_value()) {
            if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR) {
                if (!(as_state->build_info_khr->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
                    const LogObjectList objlist(device, pAccelerationStructures[i]);
                    skip |= LogError("VUID-vkWriteAccelerationStructuresPropertiesKHR-accelerationStructures-03431", objlist,
                                     as_loc, "has flags %s.",
                                     string_VkBuildAccelerationStructureFlagsKHR(as_state->build_info_khr->flags).c_str());
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);
    auto query_pool_state = Get<vvl::QueryPool>(queryPool);
    ASSERT_AND_RETURN_SKIP(query_pool_state);
    const auto &query_pool_ci = query_pool_state->create_info;
    if (query_pool_ci.queryType != queryType) {
        skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-queryPool-02493", commandBuffer,
                         error_obj.location.dot(Field::queryType),
                         "was created with %s which is different from the type queryPool was created with (%s).",
                         string_VkQueryType(queryType), string_VkQueryType(query_pool_ci.queryType));
    }
    for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
        const Location as_loc = error_obj.location.dot(Field::pAccelerationStructures, i);
        auto as_state = Get<vvl::AccelerationStructureKHR>(pAccelerationStructures[i]);
        ASSERT_AND_CONTINUE(as_state);

        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *as_state->buffer_state, as_loc.dot(Field::buffer),
                                              "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-buffer-03736");

        if (!as_state->is_built) {
            skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-pAccelerationStructures-04964", commandBuffer,
                             as_loc, "has not been built.");
        } else {
            if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR && as_state->build_info_khr.has_value()) {
                if (!(as_state->build_info_khr->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
                    skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-accelerationStructures-03431",
                                     commandBuffer, as_loc,
                                     "was built with %s, but queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR.",
                                     string_VkBuildAccelerationStructureFlagsKHR(as_state->build_info_khr->flags).c_str());
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);
    auto query_pool_state = Get<vvl::QueryPool>(queryPool);
    ASSERT_AND_RETURN_SKIP(query_pool_state);
    const auto &query_pool_ci = query_pool_state->create_info;
    if (query_pool_ci.queryType != queryType) {
        skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesNV-queryPool-03755", commandBuffer,
                         error_obj.location.dot(Field::queryType),
                         "was created with %s which is differnent from the type queryPool was created with %s.",
                         string_VkQueryType(queryType), string_VkQueryType(query_pool_ci.queryType));
    }
    for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
        if (queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV) {
            auto as_state = Get<vvl::AccelerationStructureNV>(pAccelerationStructures[i]);
            ASSERT_AND_CONTINUE(as_state);

            if (!(as_state->build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
                skip |= LogError("VUID-vkCmdWriteAccelerationStructuresPropertiesNV-pAccelerationStructures-06215", commandBuffer,
                                 error_obj.location.dot(Field::pAccelerationStructures, i),
                                 "was built with %s, but queryType is VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR.",
                                 string_VkBuildAccelerationStructureFlagsKHR(as_state->build_info.flags).c_str());
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR &as_info,
                                                          const VulkanTypedHandle &handle, const Location &info_loc) const {
    bool skip = false;

    auto src_as_state = Get<vvl::AccelerationStructureKHR>(as_info.src);
    if (src_as_state) {
        if (!src_as_state->is_built) {
            skip |= LogError("VUID-VkCopyAccelerationStructureInfoKHR-src-04963", device, info_loc.dot(Field::src),
                             "has not been built.");
        }

        if (auto buffer_state = Get<vvl::Buffer>(src_as_state->create_info.buffer)) {
            skip |= ValidateMemoryIsBoundToBuffer(device, *buffer_state, info_loc.dot(Field::src),
                                                  "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
        }

        if (as_info.mode == VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR && src_as_state->build_info_khr.has_value()) {
            if (!(src_as_state->build_info_khr->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)) {
                const LogObjectList objlist(handle, as_info.src);
                skip |= LogError("VUID-VkCopyAccelerationStructureInfoKHR-src-03411", objlist, info_loc.dot(Field::src),
                                 "(%s) must have been built with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR"
                                 "if mode is VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR.",
                                 FormatHandle(as_info.src).c_str());
            }
        }
    }
    auto dst_as_state = Get<vvl::AccelerationStructureKHR>(as_info.dst);
    if (dst_as_state) {
        if (auto buffer_state = Get<vvl::Buffer>(dst_as_state->create_info.buffer)) {
            skip |= ValidateMemoryIsBoundToBuffer(device, *buffer_state, info_loc.dot(Field::dst),
                                                  "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
        }
    }

    if (src_as_state && dst_as_state) {
        skip |= ValidateAccelStructsMemoryDoNotOverlap(info_loc.function, LogObjectList(), *src_as_state, info_loc.dot(Field::src),
                                                       *dst_as_state, info_loc.dot(Field::dst),
                                                       "VUID-VkCopyAccelerationStructureInfoKHR-dst-07791");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    skip |= ValidateCopyAccelerationStructureInfoKHR(*pInfo, error_obj.handle, info_loc);
    if (auto src_accel_state = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_accel_state->buffer_state, info_loc.dot(Field::src),
                                              "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
    }
    if (auto dst_accel_state = Get<vvl::AccelerationStructureKHR>(pInfo->dst)) {
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_accel_state->buffer_state, info_loc.dot(Field::dst),
                                              "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, operation, error_obj.location.dot(Field::operation),
                                      "VUID-vkDestroyDeferredOperationKHR-operation-03436");
    return skip;
}

bool CoreChecks::PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyAccelerationStructureKHR-deferredOperation-03678");

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    skip |= ValidateCopyAccelerationStructureInfoKHR(*pInfo, error_obj.handle, error_obj.location.dot(Field::pInfo));

    if (auto src_accel_state = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        skip |= ValidateAccelStructBufferMemoryIsHostVisible(*src_accel_state, info_loc.dot(Field::src),
                                                             "VUID-vkCopyAccelerationStructureKHR-buffer-03727");

        skip |= ValidateAccelStructBufferMemoryIsNotMultiInstance(*src_accel_state, info_loc.dot(Field::src),
                                                                  "VUID-vkCopyAccelerationStructureKHR-buffer-03780");
    }

    if (auto dst_accel_state = Get<vvl::AccelerationStructureKHR>(pInfo->dst)) {
        skip |= ValidateAccelStructBufferMemoryIsHostVisible(*dst_accel_state, info_loc.dot(Field::dst),
                                                             "VUID-vkCopyAccelerationStructureKHR-buffer-03728");

        skip |= ValidateAccelStructBufferMemoryIsNotMultiInstance(*dst_accel_state, info_loc.dot(Field::dst),
                                                                  "VUID-vkCopyAccelerationStructureKHR-buffer-03781");
    }
    return skip;
}

bool CoreChecks::ValidateVkCopyAccelerationStructureToMemoryInfoKHR(const vvl::AccelerationStructureKHR &src_accel_struct,
                                                                    LogObjectList objlist, const Location &loc) const {
    bool skip = false;
    if (!src_accel_struct.is_built) {
        objlist.add(src_accel_struct.Handle());
        skip |= LogError("VUID-VkCopyAccelerationStructureToMemoryInfoKHR-src-04959", objlist, loc.dot(Field::src),
                         "has not been built.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                     const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
                                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyAccelerationStructureToMemoryKHR-deferredOperation-03678");

    if (const auto src_accel_struct = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        const Location info_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateVkCopyAccelerationStructureToMemoryInfoKHR(*src_accel_struct, LogObjectList(device), info_loc);

        if (auto buffer_state = Get<vvl::Buffer>(src_accel_struct->create_info.buffer)) {
            skip |= ValidateAccelStructBufferMemoryIsHostVisible(*src_accel_struct, info_loc.dot(Field::src),
                                                                 "VUID-vkCopyAccelerationStructureToMemoryKHR-buffer-03731");

            skip |= ValidateAccelStructBufferMemoryIsNotMultiInstance(*src_accel_struct, info_loc.dot(Field::src),
                                                                      "VUID-vkCopyAccelerationStructureToMemoryKHR-buffer-03783");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                        const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
                                                                        const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (auto src_accel_struct = Get<vvl::AccelerationStructureKHR>(pInfo->src)) {
        skip |= ValidateVkCopyAccelerationStructureToMemoryInfoKHR(*src_accel_struct, LogObjectList(commandBuffer),
                                                                   error_obj.location.dot(Field::pInfo));

        if (auto buffer_state = Get<vvl::Buffer>(src_accel_struct->create_info.buffer)) {
            skip |=
                ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, error_obj.location.dot(Field::pInfo).dot(Field::src),
                                              "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-None-03559");
        }
    }

    const VkDeviceAddress dst_address = pInfo->dst.deviceAddress;

    const auto buffer_states = GetBuffersByAddress(dst_address);
    if (buffer_states.empty()) {
        skip |= LogError("VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pInfo-03739", commandBuffer,
                         error_obj.location.dot(Field::pInfo).dot(Field::dst).dot(Field::deviceAddress),
                         "(0x%" PRIx64 ") is not a valid buffer address.", dst_address);
    } else {
        BufferAddressValidation<1> buffer_address_validator = {
            {{{"VUID-vkCmdCopyAccelerationStructureToMemoryKHR-pInfo-03741",
               [this](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                   return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state, out_error_msg);
               },
               []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }}}}};

        skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
            *this, buffer_states, error_obj.location.dot(Field::pInfo).dot(Field::dst).dot(Field::deviceAddress),
            LogObjectList(commandBuffer), dst_address);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                     const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCopyMemoryToAccelerationStructureKHR-deferredOperation-03678");

    if (auto accel_state = Get<vvl::AccelerationStructureKHR>(pInfo->dst)) {
        skip |= ValidateAccelStructBufferMemoryIsHostVisible(*accel_state, error_obj.location.dot(Field::pInfo).dot(Field::dst),
                                                             "VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03730");

        skip |=
            ValidateAccelStructBufferMemoryIsNotMultiInstance(*accel_state, error_obj.location.dot(Field::pInfo).dot(Field::dst),
                                                              "VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03782");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                        const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                        const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (auto accel_state = Get<vvl::AccelerationStructureKHR>(pInfo->dst)) {
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *accel_state->buffer_state,
                                              error_obj.location.dot(Field::pInfo).dot(Field::dst),
                                              "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-buffer-03745");
    }

    const VkDeviceAddress src_address = pInfo->src.deviceAddress;

    const auto buffer_states = GetBuffersByAddress(src_address);
    if (buffer_states.empty()) {
        skip |= LogError("VUID-vkCmdCopyMemoryToAccelerationStructureKHR-pInfo-03742", commandBuffer,
                         error_obj.location.dot(Field::pInfo).dot(Field::src).dot(Field::deviceAddress),
                         "(0x%" PRIx64 ") is not a valid buffer address.", src_address);
    } else {
        BufferAddressValidation<1> buffer_address_validator = {
            {{{"VUID-vkCmdCopyMemoryToAccelerationStructureKHR-pInfo-03744",
               [this](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                   return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state, out_error_msg);
               },
               []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }}}}};

        skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
            *this, buffer_states, error_obj.location.dot(Field::pInfo).dot(Field::src).dot(Field::deviceAddress),
            LogObjectList(commandBuffer), src_address);
    }

    return skip;
}

uint32_t CoreChecks::CalcTotalShaderGroupCount(const vvl::Pipeline &pipeline) const {
    uint32_t total = 0;
    const auto &create_info = pipeline.RayTracingCreateInfo();
    if (create_info.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR) {
        total = create_info.groupCount;

        if (create_info.pLibraryInfo) {
            for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
                auto library_pipeline_state = Get<vvl::Pipeline>(create_info.pLibraryInfo->pLibraries[i]);
                if (!library_pipeline_state) continue;
                total += CalcTotalShaderGroupCount(*library_pipeline_state.get());
            }
        }
    } else if (create_info.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV) {
        total = create_info.groupCount;

        if (create_info.pLibraryInfo) {
            for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
                auto library_pipeline_state = Get<vvl::Pipeline>(create_info.pLibraryInfo->pLibraries[i]);
                if (!library_pipeline_state) continue;
                total += CalcTotalShaderGroupCount(*library_pipeline_state.get());
            }
        }
    }

    return total;
}

bool CoreChecks::PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                   uint32_t groupCount, size_t dataSize, void *pData,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    auto pipeline_ptr = Get<vvl::Pipeline>(pipeline);
    ASSERT_AND_RETURN_SKIP(pipeline_ptr);
    if (pipeline_ptr->pipeline_type != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        skip |=
            LogError("VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-04619", pipeline, error_obj.location.dot(Field::pipeline),
                     "is a %s pipeline.", string_VkPipelineBindPoint(pipeline_ptr->pipeline_type));
        return skip;
    }

    const vvl::Pipeline &pipeline_state = *pipeline_ptr;
    if (pipeline_state.create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
        if (!enabled_features.pipelineLibraryGroupHandles) {
            skip |= LogError("VUID-vkGetRayTracingShaderGroupHandlesKHR-pipeline-07828", pipeline,
                             error_obj.location.dot(Field::pipeline),
                             "was created with %s, but the pipelineLibraryGroupHandles feature was not enabled.",
                             string_VkPipelineCreateFlags2(pipeline_state.create_flags).c_str());
        }
    }
    if (dataSize < (phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleSize * groupCount)) {
        skip |=
            LogError("VUID-vkGetRayTracingShaderGroupHandlesKHR-dataSize-02420", device, error_obj.location.dot(Field::dataSize),
                     "(%zu) must be at least "
                     "shaderGroupHandleSize (%" PRIu32 ") * groupCount (%" PRIu32 ").",
                     dataSize, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleSize, groupCount);
    }

    const uint32_t total_group_count = CalcTotalShaderGroupCount(pipeline_state);

    if (firstGroup >= total_group_count) {
        skip |= LogError("VUID-vkGetRayTracingShaderGroupHandlesKHR-firstGroup-04050", device,
                         error_obj.location.dot(Field::firstGroup),
                         "(%" PRIu32 ") must be less than the number of shader groups in pipeline (%" PRIu32 ").", firstGroup,
                         total_group_count);
    }
    if ((firstGroup + groupCount) > total_group_count) {
        skip |= LogError("VUID-vkGetRayTracingShaderGroupHandlesKHR-firstGroup-02419", device,
                         error_obj.location.dot(Field::firstGroup),
                         "(%" PRIu32 ") + groupCount (%" PRIu32
                         ") must be less than or equal to the number "
                         "of shader groups in pipeline (%" PRIu32 ").",
                         firstGroup, groupCount, total_group_count);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                uint32_t firstGroup, uint32_t groupCount,
                                                                                size_t dataSize, void *pData,
                                                                                const ErrorObject &error_obj) const {
    bool skip = false;
    if (dataSize < (phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleCaptureReplaySize * groupCount)) {
        skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-dataSize-03484", device,
                         error_obj.location.dot(Field::dataSize),
                         "(%zu) must be at least "
                         "shaderGroupHandleCaptureReplaySize (%" PRIu32 ") * groupCount (%" PRIu32 ").",
                         dataSize, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleCaptureReplaySize, groupCount);
    }
    auto pipeline_state = Get<vvl::Pipeline>(pipeline);
    ASSERT_AND_RETURN_SKIP(pipeline_state);
    if (pipeline_state->pipeline_type != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-04620", pipeline,
                         error_obj.location.dot(Field::pipeline), "is a %s pipeline.",
                         string_VkPipelineBindPoint(pipeline_state->pipeline_type));
        return skip;
    }

    const auto &create_info = pipeline_state->RayTracingCreateInfo();
    if (create_info.flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
        if (!enabled_features.pipelineLibraryGroupHandles) {
            skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-07829", pipeline,
                             error_obj.location.dot(Field::pipeline),
                             "was created with %s, but the pipelineLibraryGroupHandles feature was not enabled.",
                             string_VkPipelineCreateFlags(create_info.flags).c_str());
        }
    }

    const uint32_t total_group_count = CalcTotalShaderGroupCount(*pipeline_state);

    if (firstGroup >= total_group_count) {
        skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-04051", device,
                         error_obj.location.dot(Field::firstGroup),
                         "(%" PRIu32
                         ") must be less than the number of shader "
                         "groups in pipeline (%" PRIu32 ").",
                         firstGroup, total_group_count);
    }
    if ((firstGroup + groupCount) > total_group_count) {
        skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-firstGroup-03483", device,
                         error_obj.location.dot(Field::firstGroup),
                         "(%" PRIu32 ") + groupCount (%" PRIu32
                         ") must be less than or equal to the number of shader groups in pipeline (%" PRIu32 ").",
                         firstGroup, groupCount, total_group_count);
    }
    if (!(create_info.flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
        skip |= LogError("VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-pipeline-03607", pipeline,
                         error_obj.location.dot(Field::pipeline), "was created with %s.",
                         string_VkPipelineCreateFlags(create_info.flags).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                                     const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                     VkShaderGroupShaderKHR groupShader,
                                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    auto pipeline_state = Get<vvl::Pipeline>(pipeline);
    ASSERT_AND_RETURN_SKIP(pipeline_state);

    if (pipeline_state->pipeline_type != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        skip |= LogError("VUID-vkGetRayTracingShaderGroupStackSizeKHR-pipeline-04622", pipeline,
                         error_obj.location.dot(Field::pipeline), "is a %s pipeline.",
                         string_VkPipelineBindPoint(pipeline_state->pipeline_type));
    } else {
        const auto &create_info = pipeline_state->RayTracingCreateInfo();
        if (group >= create_info.groupCount) {
            skip |=
                LogError("VUID-vkGetRayTracingShaderGroupStackSizeKHR-group-03608", pipeline, error_obj.location.dot(Field::group),
                         "(%" PRIu32 ") must be less than the number of shader groups in pipeline (%" PRIu32 ").", group,
                         create_info.groupCount);
        } else {
            const auto &group_info = create_info.pGroups[group];
            bool unused_group = false;
            switch (groupShader) {
                case VK_SHADER_GROUP_SHADER_GENERAL_KHR:
                    if (group_info.generalShader == VK_SHADER_UNUSED_KHR) {
                        unused_group = true;
                    }
                    break;
                case VK_SHADER_GROUP_SHADER_CLOSEST_HIT_KHR:
                    if (group_info.closestHitShader == VK_SHADER_UNUSED_KHR) {
                        unused_group = true;
                    }
                    break;
                case VK_SHADER_GROUP_SHADER_ANY_HIT_KHR:
                    if (group_info.anyHitShader == VK_SHADER_UNUSED_KHR) {
                        unused_group = true;
                    }
                    break;
                case VK_SHADER_GROUP_SHADER_INTERSECTION_KHR:
                    if (group_info.intersectionShader == VK_SHADER_UNUSED_KHR) {
                        unused_group = true;
                    }
                    break;

                default:
                    break;
            }
            if (unused_group) {
                const LogObjectList objlist(device, pipeline);
                skip |= LogError("VUID-vkGetRayTracingShaderGroupStackSizeKHR-groupShader-03609", objlist,
                                 error_obj.location.dot(Field::groupShader),
                                 "is %s but the corresponding shader in shader group (%" PRIu32 ") is VK_SHADER_UNUSED_KHR.",
                                 string_VkShaderGroupShaderKHR(groupShader), group);
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles, const Location &loc) const {
    bool skip = false;

    auto vb_state = Get<vvl::Buffer>(triangles.vertexData);
    if (vb_state && vb_state->create_info.size <= triangles.vertexOffset) {
        skip |= LogError("VUID-VkGeometryTrianglesNV-vertexOffset-02428", device, loc, "is invalid.");
    }

    auto ib_state = Get<vvl::Buffer>(triangles.indexData);
    if (ib_state && ib_state->create_info.size <= triangles.indexOffset) {
        skip |= LogError("VUID-VkGeometryTrianglesNV-indexOffset-02431", device, loc, "is invalid.");
    }

    auto td_state = Get<vvl::Buffer>(triangles.transformData);
    if (td_state && td_state->create_info.size <= triangles.transformOffset) {
        skip |= LogError("VUID-VkGeometryTrianglesNV-transformOffset-02437", device, loc, "is invalid.");
    }

    return skip;
}

bool CoreChecks::ValidateGeometryAABBNV(const VkGeometryAABBNV &aabbs, const Location &loc) const {
    bool skip = false;

    auto aabb_state = Get<vvl::Buffer>(aabbs.aabbData);
    if (aabb_state && aabb_state->create_info.size > 0 && aabb_state->create_info.size <= aabbs.offset) {
        skip |= LogError("VUID-VkGeometryAABBNV-offset-02439", device, loc, "is invalid.");
    }

    return skip;
}

bool CoreChecks::ValidateGeometryNV(const VkGeometryNV &geometry, const Location &loc) const {
    bool skip = false;
    if (geometry.geometryType == VK_GEOMETRY_TYPE_TRIANGLES_NV) {
        skip |= ValidateGeometryTrianglesNV(geometry.geometry.triangles, loc);
    } else if (geometry.geometryType == VK_GEOMETRY_TYPE_AABBS_NV) {
        skip |= ValidateGeometryAABBNV(geometry.geometry.aabbs, loc);
    }
    return skip;
}

bool CoreChecks::ValidateRaytracingShaderBindingTable(VkCommandBuffer commandBuffer, const Location &table_loc,
                                                      const char *vuid_single_device_memory, const char *vuid_binding_table_flag,
                                                      const VkStridedDeviceAddressRegionKHR &binding_table) const {
    bool skip = false;

    if (binding_table.deviceAddress == 0 || binding_table.size == 0) {
        return skip;
    }

    const auto buffer_states = GetBuffersByAddress(binding_table.deviceAddress);
    if (buffer_states.empty()) {
        skip |= LogError("VUID-VkStridedDeviceAddressRegionKHR-size-04631", commandBuffer, table_loc.dot(Field::deviceAddress),
                         "(0x%" PRIx64 ") has no buffer associated with it.", binding_table.deviceAddress);
    } else {
        const sparse_container::range<VkDeviceSize> requested_range(binding_table.deviceAddress,
                                                                    binding_table.deviceAddress + binding_table.size - 1);

        BufferAddressValidation<4> buffer_address_validator = {{{
            {vuid_single_device_memory,
             [this](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                 return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state, out_error_msg);
             },
             []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }},

            {vuid_binding_table_flag,
             [](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                 if (!(static_cast<uint32_t>(buffer_state->usage) & VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)) {
                     if (out_error_msg) {
                         *out_error_msg += "buffer has usage " + string_VkBufferUsageFlags2(buffer_state->usage);
                     }
                     return false;
                 }
                 return true;
             },
             []() {
                 return "The following buffers have not been created with the VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR "
                        "usage flag:";
             }},

            {"VUID-VkStridedDeviceAddressRegionKHR-size-04631",
             [&requested_range](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                 const auto buffer_address_range = buffer_state->DeviceAddressRange();
                 if (!buffer_address_range.includes(requested_range)) {
                     if (out_error_msg) {
                         const std::string buffer_address_range_string = string_range_hex(buffer_address_range);
                         *out_error_msg += "buffer device address range is " + buffer_address_range_string;
                     }
                     return false;
                 }

                 return true;
             },
             [table_loc, requested_range_string = string_range_hex(requested_range)]() {
                 return "The following buffers do not include " + table_loc.Fields() + " buffer device address range " +
                        requested_range_string + ':';
             }},

            {"VUID-VkStridedDeviceAddressRegionKHR-size-04632",
             [&binding_table](vvl::Buffer *const buffer_state, std::string *out_error_msg) {
                 if (binding_table.stride > buffer_state->create_info.size) {
                     if (out_error_msg) {
                         *out_error_msg += "buffer size is " + std::to_string(buffer_state->create_info.size);
                     }
                     return false;
                 }
                 return true;
             },
             [table_loc, &binding_table]() {
                 return "The following buffers have a size inferior to " + table_loc.Fields() + "->stride (" +
                        std::to_string(binding_table.stride) + "):";
             }},
        }}};

        skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(*this, buffer_states, table_loc.dot(Field::deviceAddress),
                                                                  LogObjectList(commandBuffer), binding_table.deviceAddress);
    }

    return skip;
}

bool CoreChecks::ValidateDeferredOperation(VkDevice device, VkDeferredOperationKHR deferred_operation, const Location &loc,
                                           const char *vuid) const {
    // validate in core check because need to make sure it is a valid VkDeferredOperationKHR object
    bool skip = false;
    if (deferred_operation != VK_NULL_HANDLE) {
        VkResult result = DispatchGetDeferredOperationResultKHR(device, deferred_operation);
        if (result == VK_NOT_READY) {
            skip |= LogError(vuid, deferred_operation, loc.dot(Field::deferredOperation), "%s is not completed.",
                             FormatHandle(deferred_operation).c_str());
        }
    }
    return skip;
}
