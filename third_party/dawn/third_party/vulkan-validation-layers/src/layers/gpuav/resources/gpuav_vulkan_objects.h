/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
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

#pragma once

#include "containers/custom_containers.h"
#include "vma/vma.h"

#include <typeinfo>
#include <unordered_map>
#include <vector>

struct Location;
namespace gpuav {
class Validator;

namespace vko {

class DescriptorSetManager {
  public:
    DescriptorSetManager(VkDevice device, uint32_t num_bindings_in_set);
    ~DescriptorSetManager();

    VkResult GetDescriptorSet(VkDescriptorPool *out_desc_pool, VkDescriptorSetLayout ds_layout, VkDescriptorSet *out_desc_sets);
    VkResult GetDescriptorSets(uint32_t count, VkDescriptorPool *out_pool, VkDescriptorSetLayout ds_layout,
                               std::vector<VkDescriptorSet> *out_desc_sets);
    void PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set);

  private:
    std::unique_lock<std::mutex> Lock() const { return std::unique_lock<std::mutex>(lock_); }

    struct PoolTracker {
        uint32_t size;
        uint32_t used;
    };
    VkDevice device;
    uint32_t num_bindings_in_set;
    vvl::unordered_map<VkDescriptorPool, PoolTracker> desc_pool_map_;
    mutable std::mutex lock_;
};

class Buffer {
  public:
    explicit Buffer(Validator &gpuav) : gpuav(gpuav) {}

    // Warps VMA calls to simplify error reporting.
    // No error propagation, but if hitting a VMA error, GPU-AV is likely not going to recover anyway.

    [[nodiscard]] void *MapMemory(const Location &loc) const;
    void UnmapMemory() const;
    void FlushAllocation(const Location &loc, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
    void InvalidateAllocation(const Location &loc, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

    [[nodiscard]] bool Create(const Location &loc, const VkBufferCreateInfo *buffer_create_info,
                              const VmaAllocationCreateInfo *allocation_create_info);
    void Destroy();

    bool IsDestroyed() const { return buffer == VK_NULL_HANDLE; }
    const VkBuffer &VkHandle() const { return buffer; }
    const VmaAllocation &Allocation() const { return allocation; }
    VkDeviceAddress Address() const { return device_address; };

  private:
    const Validator &gpuav;
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    // If buffer was not created with VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT then this will not be zero
    VkDeviceAddress device_address = 0;
};

// Register/Create and register GPU resources, all to be destroyed upon a call to DestroyResources
class GpuResourcesManager {
  public:
    explicit GpuResourcesManager(DescriptorSetManager &descriptor_set_manager) : descriptor_set_manager_(descriptor_set_manager) {}

    VkDescriptorSet GetManagedDescriptorSet(VkDescriptorSetLayout desc_set_layout);
    vko::Buffer GetManagedBuffer(Validator &gpuav, const Location &loc, const VkBufferCreateInfo &ci,
                                 const VmaAllocationCreateInfo &vma_ci);

    void DestroyResources();

  private:
    DescriptorSetManager &descriptor_set_manager_;
    std::vector<std::pair<VkDescriptorPool, VkDescriptorSet>> descriptors_;
    std::vector<vko::Buffer> buffers_;
};

// Cache a single object of type T. Key is *only* based on typeid(T)
class SharedResourcesCache {
  public:
    template <typename T>
    T *TryGet() {
        auto entry = shared_validation_resources_map_.find(typeid(T));
        if (entry == shared_validation_resources_map_.cend()) {
            return nullptr;
        }
        T *t = reinterpret_cast<T *>(entry->second.first);
        return t;
    }

    // First call to Get<T> will create the object, subsequent calls will retrieve the cached entry.
    // /!\ The cache key is only based on the type T, not on the passed parameters
    // => Successive calls to Get<T> with different parameters will NOT give different objects,
    // only the entry cached upon the first call to Get<T> will be retrieved
    template <typename T, class... ConstructorTypes>
    T &Get(ConstructorTypes &&...args) {
        T *t = TryGet<T>();
        if (t) return *t;

        auto entry =
            shared_validation_resources_map_.insert({typeid(T), {new T(std::forward<ConstructorTypes>(args)...), [](void *ptr) {
                                                                     auto obj = static_cast<T *>(ptr);
                                                                     delete obj;
                                                                 }}});
        return *static_cast<T *>(entry.first->second.first);
    }

    void Clear();

  private:
    using TypeInfoRef = std::reference_wrapper<const std::type_info>;
    struct Hasher {
        std::size_t operator()(TypeInfoRef code) const { return code.get().hash_code(); }
    };
    struct EqualTo {
        bool operator()(TypeInfoRef lhs, TypeInfoRef rhs) const { return lhs.get() == rhs.get(); }
    };

    // Tried to use vvl::unordered_map, but fails to compile on Windows currently
    std::unordered_map<TypeInfoRef, std::pair<void * /*object*/, void (*)(void *) /*object destructor*/>, Hasher, EqualTo>
        shared_validation_resources_map_;
};

}  // namespace vko

}  // namespace gpuav
