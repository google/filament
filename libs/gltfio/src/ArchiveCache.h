/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef GLTFIO_ARCHIVE_CACHE_H
#define GLTFIO_ARCHIVE_CACHE_H

#include <uberz/ReadableArchive.h>

#include <filament/Engine.h>
#include <filament/Material.h>

#include <utils/FixedCapacityVector.h>

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace filament::gltfio {

    enum class ArchiveFeatureId : uint8_t {
        Sheen = 0,
        Transmission,
        Volume,
        Ior,
        VertexColors,
        BaseColorTexture,
        NormalTexture,
        OcclusionTexture,
        EmissiveTexture,
        MetallicRoughnessTexture,
        ClearCoatTexture,
        ClearCoatRoughnessTexture,
        ClearCoatNormalTexture,
        ClearCoat,
        TextureTransforms,
        TransmissionTexture,
        SheenColorTexture,
        SheenRoughnessTexture,
        VolumeThicknessTexture,
        Specular,
        SpecularTexture,
        SpecularColorTexture,
        Dispersion,
        Count
    };

    inline constexpr uint32_t archiveFeatureBit(ArchiveFeatureId id) noexcept {
        return 1u << static_cast<uint8_t>(id);
    }

    std::string_view archiveFeatureName(ArchiveFeatureId id) noexcept;

    struct ArchiveRequirements {
        Shading shadingModel;
        BlendingMode blendingMode;
        uint32_t featuresMask = 0;

        void setFeature(ArchiveFeatureId id, bool enabled) noexcept {
            const uint32_t bit = archiveFeatureBit(id);
            featuresMask = enabled ? (featuresMask | bit) : (featuresMask & ~bit);
        }

        bool hasFeature(ArchiveFeatureId id) const noexcept {
            return (featuresMask & archiveFeatureBit(id)) != 0;
        }
    };

    // Stores a set of Filament materials and knows how to choose a suitable material when given a
    // set of requirements. Used by gltfio; users do not need to access this class directly.
    class ArchiveCache {
    public:
        ArchiveCache(Engine& engine) : mEngine(engine) {}
        ~ArchiveCache();

        void load(const void* archiveData, uint64_t archiveByteCount);
        Material* getMaterial(const ArchiveRequirements& requirements);
        Material* getDefaultMaterial();
        const Material* const* getMaterials() const noexcept { return mMaterials.data(); }
        size_t getMaterialsCount() const noexcept { return mMaterials.size(); }
        void destroyMaterials();
        uint32_t getSupportedFeatures(const Material* material) const noexcept;

    private:
        struct SpecFeatureMasks {
            uint32_t supportedMask = 0;
            uint32_t requiredMask = 0;
            bool hasUnknownRequiredFlag = false;
        };

        Engine& mEngine;
        utils::FixedCapacityVector<Material*> mMaterials;
        utils::FixedCapacityVector<SpecFeatureMasks> mSpecMasks;
        uberz::ReadableArchive* mArchive = nullptr;
    };

} // namespace filament::gltfio

#endif // GLTFIO_ARCHIVE_CACHE_H
