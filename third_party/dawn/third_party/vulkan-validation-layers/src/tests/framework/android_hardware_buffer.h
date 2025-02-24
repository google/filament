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

#include "utils/android_ndk_types.h"
#include "../framework/layer_validation_tests.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)

// Not in public NDK headers, only AOSP headers, but NDK will allow it for usage of camera apps and we use for AHB tests
constexpr uint32_t AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED = 0x22;
constexpr uint64_t AHARDWAREBUFFER_USAGE_CAMERA_WRITE = 0x20000;
constexpr uint64_t AHARDWAREBUFFER_USAGE_CAMERA_READ = 0x40000;

// Helper to get the memory type index for AHB object that are being imported
// returns false if can't set the values correctly
inline bool SetAllocationInfoImportAHB(vkt::Device *device, VkAndroidHardwareBufferPropertiesANDROID ahb_props,
                                       VkMemoryAllocateInfo &info) {
    // Set index to match one of the bits in ahb_props that is also only Device Local
    // Android implemenetations "should have" a DEVICE_LOCAL only index designed for AHB
    VkMemoryPropertyFlagBits property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkPhysicalDeviceMemoryProperties mem_props = device->Physical().memory_properties_;
    // AHB object hold the real allocationSize needed
    info.allocationSize = ahb_props.allocationSize;
    info.memoryTypeIndex = mem_props.memoryTypeCount + 1;
    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
        if ((ahb_props.memoryTypeBits & (1 << i)) && ((mem_props.memoryTypes[i].propertyFlags & property) == property)) {
            info.memoryTypeIndex = i;
            break;
        }
    }
    return info.memoryTypeIndex < mem_props.memoryTypeCount;
}

namespace vkt {
class AHB {
  public:
    AHB(const AHardwareBuffer_Desc *ahb_desc) { init(ahb_desc); }
    AHB(uint32_t format, uint64_t usage, uint32_t width, uint32_t height = 1, uint32_t layers = 1, uint32_t stride = 1) {
        AHardwareBuffer_Desc ahb_desc = {};
        ahb_desc.format = format;
        ahb_desc.usage = usage;
        ahb_desc.width = width;
        ahb_desc.height = height;
        ahb_desc.layers = layers;
        ahb_desc.stride = stride;
        init(&ahb_desc);
    }

    void init(const AHardwareBuffer_Desc *ahb_desc) { AHardwareBuffer_allocate(ahb_desc, &ahb); }

    uint64_t GetExternalFormat(const Device &dev, void *pNext = nullptr) const {
        VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = vku::InitStructHelper(pNext);
        VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper(&ahb_fmt_props);
        vk::GetAndroidHardwareBufferPropertiesANDROID(dev.handle(), ahb, &ahb_props);
        return ahb_fmt_props.externalFormat;
    }

    ~AHB() {
        if (ahb) {
            AHardwareBuffer_release(ahb);
            ahb = nullptr;
        }
    }

    struct AHardwareBuffer *handle() {
        return ahb;
    }

  private:
    AHardwareBuffer *ahb = nullptr;
};
}  // namespace vkt
#endif  // VK_USE_PLATFORM_ANDROID_KHR
