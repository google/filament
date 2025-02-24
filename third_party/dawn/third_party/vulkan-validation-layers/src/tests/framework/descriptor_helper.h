/*
 * Copyright (c) 2023-2024 The Khronos Group Inc.
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

class OneOffDescriptorSet {
  public:
    vkt::Device *device_;
    VkDescriptorPool pool_{};
    vkt::DescriptorSetLayout layout_;
    VkDescriptorSet set_{VK_NULL_HANDLE};

    // Only one member of ResourceInfo object contains a value.
    // The pointers to Image/Buffer/BufferView info structures can't be stored in 'descriptor_writes'
    // during WriteDescriptor call, because subsequent calls can reallocate which invalidates stored pointers.
    // When UpdateDescriptorSets is called it's safe to initialize the pointers.
    struct ResourceInfo {
        std::optional<VkDescriptorImageInfo> image_info;
        std::optional<VkDescriptorBufferInfo> buffer_info;
        std::optional<VkBufferView> buffer_view;
        std::optional<VkWriteDescriptorSetAccelerationStructureKHR> accel_struct_info;
    };
    std::vector<ResourceInfo> resource_infos;
    std::vector<VkWriteDescriptorSet> descriptor_writes;

    OneOffDescriptorSet(vkt::Device *device, const std::vector<VkDescriptorSetLayoutBinding> &bindings,
                        VkDescriptorSetLayoutCreateFlags layout_flags = 0, void *layout_pnext = nullptr,
                        VkDescriptorPoolCreateFlags pool_flags = 0, void *allocate_pnext = nullptr,
                        void *create_pool_pnext = nullptr);
    OneOffDescriptorSet(){};
    ~OneOffDescriptorSet();
    bool Initialized();
    void Clear();
    void WriteDescriptorBufferInfo(int binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range,
                                   VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uint32_t arrayElement = 0);
    void WriteDescriptorBufferView(int binding, VkBufferView buffer_view,
                                   VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                                   uint32_t arrayElement = 0);
    void WriteDescriptorImageInfo(int binding, VkImageView image_view, VkSampler sampler,
                                  VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                  VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t arrayElement = 0);
    void WriteDescriptorAccelStruct(int binding, uint32_t accelerationStructureCount,
                                    const VkAccelerationStructureKHR *pAccelerationStructures, uint32_t arrayElement = 0);
    void UpdateDescriptorSets();

  private:
    void AddDescriptorWrite(uint32_t binding, uint32_t array_element, VkDescriptorType descriptor_type,
                            uint32_t descriptor_count = 1);
};

// Descriptor Indexing focused variation
class OneOffDescriptorIndexingSet : public OneOffDescriptorSet {
  public:
    // Same as VkDescriptorSetLayoutBinding but ties the flags into it
    struct Binding {
        uint32_t binding;
        VkDescriptorType descriptorType;
        uint32_t descriptorCount;
        VkShaderStageFlags stageFlags;
        const VkSampler *pImmutableSamplers;
        VkDescriptorBindingFlags flag;
    };
    typedef std::vector<Binding> Bindings;

    OneOffDescriptorIndexingSet(vkt::Device *device, const Bindings &bindings, void *allocate_pnext = nullptr,
                                void *create_pool_pnext = nullptr);
};