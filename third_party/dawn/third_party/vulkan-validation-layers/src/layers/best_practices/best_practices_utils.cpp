/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/bp_state.h"

struct VendorSpecificInfo {
    EnableFlags vendor_id;
    std::string name;
};

const auto& GetVendorInfo() {
    static const std::map<BPVendorFlagBits, VendorSpecificInfo> kVendorInfo = {
        {kBPVendorArm, {vendor_specific_arm, "Arm"}},
        {kBPVendorAMD, {vendor_specific_amd, "AMD"}},
        {kBPVendorIMG, {vendor_specific_img, "IMG"}},
        {kBPVendorNVIDIA, {vendor_specific_nvidia, "NVIDIA"}}};

    return kVendorInfo;
}

ReadLockGuard BestPractices::ReadLock() const {
    if (global_settings.fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard BestPractices::WriteLock() {
    if (global_settings.fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
}

std::shared_ptr<vvl::CommandBuffer> BestPractices::CreateCmdBufferState(VkCommandBuffer handle,
                                                                        const VkCommandBufferAllocateInfo* allocate_info,
                                                                        const vvl::CommandPool* pool) {
    return std::static_pointer_cast<vvl::CommandBuffer>(
        std::make_shared<bp_state::CommandBuffer>(*this, handle, allocate_info, pool));
}

bp_state::CommandBuffer::CommandBuffer(BestPractices& bp, VkCommandBuffer handle, const VkCommandBufferAllocateInfo* allocate_info,
                                       const vvl::CommandPool* pool)
    : vvl::CommandBuffer(bp, handle, allocate_info, pool) {}

bool bp_state::VendorCheckEnabled(const CHECK_ENABLED& enabled, BPVendorFlags vendors) {
    for (const auto& vendor : GetVendorInfo()) {
        if (vendors & vendor.first && enabled[vendor.second.vendor_id]) {
            return true;
        }
    }
    return false;
}

const char* bp_state::VendorSpecificTag(BPVendorFlags vendors) {
    // Cache built vendor tags in a map
    static vvl::unordered_map<BPVendorFlags, std::string> tag_map;

    auto res = tag_map.find(vendors);
    if (res == tag_map.end()) {
        // Build the vendor tag string
        std::stringstream vendor_tag;

        vendor_tag << "[";
        bool first_vendor = true;
        for (const auto& vendor : GetVendorInfo()) {
            if (vendors & vendor.first) {
                if (!first_vendor) {
                    vendor_tag << ", ";
                }
                vendor_tag << vendor.second.name;
                first_vendor = false;
            }
        }
        vendor_tag << "]";

        tag_map[vendors] = vendor_tag.str();
        res = tag_map.find(vendors);
    }

    return res->second.c_str();
}
