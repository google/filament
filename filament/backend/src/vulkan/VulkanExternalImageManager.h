/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_BACKEND_CACHING_VULKANEXTERNALIMAGEMANAGER_H
#define TNT_FILAMENT_BACKEND_CACHING_VULKANEXTERNALIMAGEMANAGER_H

#include "VulkanHandles.h"
#include "VulkanTexture.h"

#include "vulkan/memory/ResourcePointer.h"

#include <backend/DriverEnums.h>
#include <backend/platforms/VulkanPlatform.h>

#include <bluevk/BlueVK.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace filament::backend {

class VulkanYcbcrConversionCache;
class VulkanSamplerCache;
class VulkanDescriptorSetLayoutCache;
class VulkanDescriptorSetCache;

// Manages the logic of external images and their quirks wrt Vulkan.
class VulkanExternalImageManager {
public:
    // Key identifying a cached external image import and the parameters it was imported with.
    struct ImportKey {
        void const* bufferKey = nullptr;
        VkFormat format = VK_FORMAT_UNDEFINED;
        uint64_t externalFormat = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        TextureUsage usage = {};
        VkSamplerYcbcrConversion conversion = VK_NULL_HANDLE;

        bool operator==(ImportKey const& other) const noexcept = default;
    };

    VulkanExternalImageManager(
            VulkanSamplerCache* samplerCache,
            VulkanYcbcrConversionCache* ycbcrConversionCache,
            VulkanDescriptorSetCache* setCache,
            VulkanDescriptorSetLayoutCache* layoutCache);

    ~VulkanExternalImageManager();

    // Drops cached imports that have not been reused for a while.
    void gc();

    void terminate();

    using SetArray = std::array<fvkmemory::resource_ptr<VulkanDescriptorSet>,
            VulkanDescriptorSetLayout::UNIQUE_DESCRIPTOR_SET_COUNT>;

    using LayoutArray = std::array<fvkmemory::resource_ptr<VulkanDescriptorSetLayout>,
            VulkanDescriptorSetLayout::UNIQUE_DESCRIPTOR_SET_COUNT>;

    using VkLayoutArray = VulkanDescriptorSetLayout::DescriptorSetLayoutArray;

    void removeDescriptorSet(fvkmemory::resource_ptr<VulkanDescriptorSet> set);

    void bindExternallySampledTexture(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
            uint8_t bindingPoint, fvkmemory::resource_ptr<VulkanTexture> image,
            SamplerParams samplerParams);

    void clearTextureBinding(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
                             uint8_t bindingPoint);

    VkSamplerYcbcrConversion getVkSamplerYcbcrConversion(
            VulkanPlatform::ExternalImageMetadata const& metadata);

    // Returns a cached import of the external image's native buffer that is compatible with the
    // given key, or an empty pointer. Updates the cache's LRU order on hit, and drops any stale
    // import if the buffer's metadata changed.
    fvkmemory::resource_ptr<VulkanTexture> findImport(ImportKey const& key);

    // Caches an imported texture for the given key, evicting the least recently used import when
    // exceeding MAX_EXTERNAL_IMAGE_IMPORTS. The texture should own the import without holding an
    // ExternalImageHandle.
    void cacheImport(ImportKey const& key, fvkmemory::resource_ptr<VulkanTexture> texture);

    // - Update the descriptor set layout with the external samplers. This layout will be assign the
    // respective immutable samplers to the layout.
    // - Switch from binding the base layout to the new one.
    // - Create a new descriptor set based on the new layout and copy the old descriptor set
    // bindings that don't use external samplers.
    // - Update the bindings that use external samplers.
    void updateSetAndLayout(fvkmemory::resource_ptr<VulkanDescriptorSet> set);

private:

    VulkanSamplerCache* mSamplerCache;
    VulkanYcbcrConversionCache* mYcbcrConversionCache;
    VulkanDescriptorSetCache* mDescriptorSetCache;
    VulkanDescriptorSetLayoutCache* mDescriptorSetLayoutCache;

    using SetAndLayout = std::pair<fvkmemory::resource_ptr<VulkanDescriptorSet>,
            fvkmemory::resource_ptr<VulkanDescriptorSetLayout>>;

    struct SetBindingInfo {
        uint8_t binding = 0;
        fvkmemory::resource_ptr<VulkanTexture> image;
        fvkmemory::resource_ptr<VulkanDescriptorSet> set;
        VkSampler sampler = VK_NULL_HANDLE;
    };

    // Use vectors instead of hash maps because we only expect small number of entries.
    std::vector<SetBindingInfo> mSetBindings;

    // Maximum number of cached imports. This avoids infinite caching of imports.
    static constexpr size_t MAX_EXTERNAL_IMAGE_IMPORTS = 12;

    // Imports not reused for this many frames are dropped, so that the buffers of a destroyed
    // producer are not kept alive.
    static constexpr uint32_t EXTERNAL_IMAGE_IMPORT_MAX_IDLE_FRAMES = 120;

    // Imports of external images, keyed by their native buffer (e.g. AHardwareBuffer).
    //
    // Clients such as ImageReader-based video playback cycle through a small set of buffers, and
    // bind each of them again and again through a new ExternalImageHandle. Without this cache,
    // every bind would import the buffer again (new VkImage, imported memory, and a layout
    // transition that may discard the buffer contents). Instead, textures for a cached buffer
    // share the imported VkImage, while each one holds its own ExternalImageHandle.
    struct ExternalImageImport {
        ImportKey key;
        // Texture owning the import. It holds no ExternalImageHandle.
        fvkmemory::resource_ptr<VulkanTexture> texture;
        uint32_t lastUsedFrame = 0;
    };

    // Most recently used at the back.
    std::vector<ExternalImageImport> mImports;
    uint32_t mImportFrame = 0;
};

} // filament::backend

#endif // TNT_FILAMENT_BACKEND_CACHING_VULKANEXTERNALIMAGEMANAGER_H
