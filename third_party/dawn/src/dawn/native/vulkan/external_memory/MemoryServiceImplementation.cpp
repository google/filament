// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/vulkan/external_memory/MemoryServiceImplementation.h"
#include "dawn/native/vulkan/DeviceVk.h"

namespace dawn::native::vulkan::external_memory {

ServiceImplementation::ServiceImplementation(Device* device) : mDevice(device) {}
ServiceImplementation::~ServiceImplementation() = default;

bool ServiceImplementation::RequiresDedicatedAllocation(const ExternalImageDescriptorVk* descriptor,
                                                        VkImage image) const {
    switch (descriptor->dedicatedAllocation) {
        case NeedsDedicatedAllocation::Yes:
            return true;

        case NeedsDedicatedAllocation::No:
            return false;

        case NeedsDedicatedAllocation::Detect:
            if (!mDevice->GetDeviceInfo().HasExt(DeviceExt::DedicatedAllocation)) {
                return false;
            }

            VkMemoryDedicatedRequirements dedicatedRequirements;
            dedicatedRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
            dedicatedRequirements.pNext = nullptr;

            VkMemoryRequirements2 baseRequirements;
            baseRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
            baseRequirements.pNext = &dedicatedRequirements;

            VkImageMemoryRequirementsInfo2 imageInfo;
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
            imageInfo.pNext = nullptr;
            imageInfo.image = image;

            mDevice->fn.GetImageMemoryRequirements2(mDevice->GetVkDevice(), &imageInfo,
                                                    &baseRequirements);

            // The Vulkan spec requires that prefersDA is set if requiresDA is, so we can just check
            // for prefersDA.
            return dedicatedRequirements.prefersDedicatedAllocation;
    }
    DAWN_UNREACHABLE();
}

}  // namespace dawn::native::vulkan::external_memory
