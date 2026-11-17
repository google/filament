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

#include "ArchiveCache.h"

#include <uberz/ArchiveEnums.h>
#include <uberz/ReadableArchive.h>

#include <filament/Material.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>
#include <utils/memalign.h>
#include <utils/ostream.h>
#include <utils/Panic.h>

#include <zstd.h>

#include <cstddef>
#include <cstdint>
#include <string_view>

using namespace utils;
using namespace filament::uberz;

namespace filament::gltfio {

// Set this to a certain spec index to find out why it was deemed unsuitable.
// To find the spec index of interest, try invoking uberz with the verbose flag.
constexpr static int DEBUG_SPEC_INDEX = -1;

constexpr static void debugSuitability(int index, const char* msg) {
    if constexpr(DEBUG_SPEC_INDEX > 0) {
        if (index == DEBUG_SPEC_INDEX) {
            slog.d << "Spec " << DEBUG_SPEC_INDEX
                << " is unsuitable due to " << msg << io::endl;
        }
    }
}

std::string_view archiveFeatureName(ArchiveFeatureId id) noexcept {
    using namespace std::string_view_literals;
    switch (id) {
        case ArchiveFeatureId::Sheen: return "Sheen"sv;
        case ArchiveFeatureId::Transmission: return "Transmission"sv;
        case ArchiveFeatureId::Volume: return "Volume"sv;
        case ArchiveFeatureId::Ior: return "Ior"sv;
        case ArchiveFeatureId::VertexColors: return "VertexColors"sv;
        case ArchiveFeatureId::BaseColorTexture: return "BaseColorTexture"sv;
        case ArchiveFeatureId::NormalTexture: return "NormalTexture"sv;
        case ArchiveFeatureId::OcclusionTexture: return "OcclusionTexture"sv;
        case ArchiveFeatureId::EmissiveTexture: return "EmissiveTexture"sv;
        case ArchiveFeatureId::MetallicRoughnessTexture: return "MetallicRoughnessTexture"sv;
        case ArchiveFeatureId::ClearCoatTexture: return "ClearCoatTexture"sv;
        case ArchiveFeatureId::ClearCoatRoughnessTexture: return "ClearCoatRoughnessTexture"sv;
        case ArchiveFeatureId::ClearCoatNormalTexture: return "ClearCoatNormalTexture"sv;
        case ArchiveFeatureId::ClearCoat: return "ClearCoat"sv;
        case ArchiveFeatureId::TextureTransforms: return "TextureTransforms"sv;
        case ArchiveFeatureId::TransmissionTexture: return "TransmissionTexture"sv;
        case ArchiveFeatureId::SheenColorTexture: return "SheenColorTexture"sv;
        case ArchiveFeatureId::SheenRoughnessTexture: return "SheenRoughnessTexture"sv;
        case ArchiveFeatureId::VolumeThicknessTexture: return "VolumeThicknessTexture"sv;
        case ArchiveFeatureId::Specular: return "Specular"sv;
        case ArchiveFeatureId::SpecularTexture: return "SpecularTexture"sv;
        case ArchiveFeatureId::SpecularColorTexture: return "SpecularColorTexture"sv;
        case ArchiveFeatureId::Dispersion: return "Dispersion"sv;
        case ArchiveFeatureId::Count: break;
    }
    return ""sv;
}

void ArchiveCache::load(const void* archiveData, uint64_t archiveByteCount) {
    assert_invariant(mArchive == nullptr && "Do not call load() twice");
    const uint64_t decompSize = ZSTD_getFrameContentSize(archiveData, archiveByteCount);
    if (decompSize == ZSTD_CONTENTSIZE_UNKNOWN || decompSize == ZSTD_CONTENTSIZE_ERROR) {
        PANIC_POSTCONDITION("Decompression error.");
    }
    // Reject implausibly large declared sizes to prevent unbounded allocation
    // from attacker-controlled archive files (decompression bomb).
    static constexpr uint64_t MAX_ARCHIVE_DECOMPRESSED_SIZE = 256ull * 1024ull * 1024ull; // 256 MiB
    if (UTILS_UNLIKELY(decompSize > MAX_ARCHIVE_DECOMPRESSED_SIZE)) {
        slog.e << "ArchiveCache: decompressed size exceeds limit (" << decompSize
                << " bytes)" << io::endl;
        return;
    }
    auto* basePointer = (uint64_t*) utils::aligned_alloc(decompSize, 8);
    if (UTILS_UNLIKELY(basePointer == nullptr)) {
        slog.e << "ArchiveCache: failed to allocate decompression buffer ("
                << decompSize << " bytes)" << io::endl;
        return;
    }
    const size_t decompressed = ZSTD_decompress(basePointer, decompSize, archiveData,
            archiveByteCount);
    if (UTILS_UNLIKELY(ZSTD_isError(decompressed))) {
        slog.e << "ArchiveCache: decompression failed: " << ZSTD_getErrorName(decompressed)
                << io::endl;
        aligned_free(basePointer);
        return;
    }
    mArchive = reinterpret_cast<ReadableArchive*>(basePointer);
    if (!convertOffsetsToPointers(mArchive, decompSize)) {
        aligned_free(basePointer);
        mArchive = nullptr;
        return;
    }
    mMaterials = FixedCapacityVector<Material*>(mArchive->specsCount, nullptr);
    mSpecMasks = FixedCapacityVector<SpecFeatureMasks>(mArchive->specsCount);
    for (uint64_t i = 0; i < mArchive->specsCount; ++i) {
        const ArchiveSpec& spec = mArchive->specs[i];
        SpecFeatureMasks masks{};
        for (uint64_t j = 0; j < spec.flagsCount; ++j) {
            const ArchiveFlag& flag = spec.flags[j];
            const std::string_view flagName(flag.name);
            bool matched = false;
            for (uint8_t k = 0; k < static_cast<uint8_t>(ArchiveFeatureId::Count); ++k) {
                const auto id = static_cast<ArchiveFeatureId>(k);
                if (flagName == archiveFeatureName(id)) {
                    if (flag.value != ArchiveFeature::UNSUPPORTED) {
                        masks.supportedMask |= archiveFeatureBit(id);
                    }
                    if (flag.value == ArchiveFeature::REQUIRED) {
                        masks.requiredMask |= archiveFeatureBit(id);
                    }
                    matched = true;
                    break;
                }
            }
            if (!matched && flag.value == ArchiveFeature::REQUIRED) {
                masks.hasUnknownRequiredFlag = true;
            }
        }
        mSpecMasks[i] = masks;
    }
}

// This loops though all ubershaders and returns the cheapest one that meets the given requirements.
Material* ArchiveCache::getMaterial(const ArchiveRequirements& reqs) {
    assert_invariant(mArchive && "Please call load() before requesting any materials.");
    if (mArchive == nullptr) {
        return nullptr;
    }

    uint64_t bestIndex = UINT64_MAX;
    uint16_t minSupportedFlags = UINT16_MAX;

    for (uint64_t i = 0; i < mArchive->specsCount; ++i) {
        const ArchiveSpec& spec = mArchive->specs[i];
        if (spec.blendingMode != INVALID_BLENDING && spec.blendingMode != reqs.blendingMode) {
            debugSuitability(int(i), "blend mode mismatch.");
            continue;
        }

        if (spec.shadingModel != INVALID_SHADING_MODEL && spec.shadingModel != reqs.shadingModel) {
            debugSuitability(int(i), "material model.");
            continue;
        }

        const SpecFeatureMasks& masks = mSpecMasks[i];

        // For each feature required by the mesh, this ubershader is suitable only if it includes a
        // feature flag for it and the feature flag is either OPTIONAL or REQUIRED.
        if ((reqs.featuresMask & ~masks.supportedMask) != 0) {
            if constexpr (DEBUG_SPEC_INDEX > 0) {
                const uint32_t missing = reqs.featuresMask & ~masks.supportedMask;
                for (uint8_t k = 0; k < static_cast<uint8_t>(ArchiveFeatureId::Count); ++k) {
                    const auto id = static_cast<ArchiveFeatureId>(k);
                    if (missing & archiveFeatureBit(id)) {
                        debugSuitability(int(i), archiveFeatureName(id).data());
                        break;
                    }
                }
            }
            continue;
        }

        // If this ubershader requires a certain feature to be enabled in the glTF, but the glTF
        // mesh doesn't have it, then this ubershader is not suitable.
        if (UTILS_UNLIKELY(masks.hasUnknownRequiredFlag ||
                (masks.requiredMask & ~reqs.featuresMask) != 0)) {
            debugSuitability(int(i), "required flag missing.");
            continue;
        }

        if (bestIndex == UINT64_MAX || spec.supportedFlagsCount < minSupportedFlags) {
            bestIndex = i;
            minSupportedFlags = spec.supportedFlagsCount;
        }
    }

    if (bestIndex != UINT64_MAX) {
        if (mMaterials[bestIndex] == nullptr) {
            const ArchiveSpec& spec = mArchive->specs[bestIndex];
            mMaterials[bestIndex] = Material::Builder()
                .package(spec.package, spec.packageByteCount)
                .build(mEngine);
        }
        return mMaterials[bestIndex];
    }

    return nullptr;
}

Material* ArchiveCache::getDefaultMaterial() {
    assert_invariant(mArchive && "Please call load() before requesting any materials.");
    assert_invariant(!mMaterials.empty() && "Archive must have at least one material.");
    if (!mArchive) return nullptr;
    if (mMaterials[0] == nullptr) {
        mMaterials[0] = Material::Builder()
            .package(mArchive->specs[0].package, mArchive->specs[0].packageByteCount)
            .build(mEngine);
    }
    return mMaterials[0];
}

void ArchiveCache::destroyMaterials() {
    for (const auto mat : mMaterials) mEngine.destroy(mat);
    mMaterials.clear();
    mSpecMasks.clear();
}

uint32_t ArchiveCache::getSupportedFeatures(const Material* material) const noexcept {
    for (size_t specIndex = 0; specIndex < mMaterials.size(); ++specIndex) {
        if (material == mMaterials[specIndex]) {
            return mSpecMasks[specIndex].supportedMask;
        }
    }
    return 0;
}

ArchiveCache::~ArchiveCache() {
    assert_invariant(mMaterials.empty() &&
        "Please call destroyMaterials explicitly to ensure correct destruction order");
    aligned_free(mArchive);
}

} // namespace filament::gltfio
