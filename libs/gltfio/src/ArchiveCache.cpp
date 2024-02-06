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

#include <filament/Material.h>

#include <uberz/ArchiveEnums.h>
#include <uberz/ReadableArchive.h>

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/debug.h>
#include <utils/memalign.h>
#include <utils/ostream.h>

#include <zstd.h>

#include <stdint.h>
#include <stddef.h>
#include <string.h>

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

static bool strIsEqual(const CString& a, const char* b) {
    return strncmp(a.c_str(), b, a.size()) == 0;
}

void ArchiveCache::load(const void* archiveData, uint64_t archiveByteCount) {
    assert_invariant(mArchive == nullptr && "Do not call load() twice");
    const uint64_t decompSize = ZSTD_getFrameContentSize(archiveData, archiveByteCount);
    if (decompSize == ZSTD_CONTENTSIZE_UNKNOWN || decompSize == ZSTD_CONTENTSIZE_ERROR) {
        PANIC_POSTCONDITION("Decompression error.");
    }
    uint64_t* basePointer = (uint64_t*) utils::aligned_alloc(decompSize, 8);
    ZSTD_decompress(basePointer, decompSize, archiveData, archiveByteCount);
    mArchive = (ReadableArchive*) basePointer;
    convertOffsetsToPointers(mArchive);
    mMaterials = FixedCapacityVector<Material*>(mArchive->specsCount, nullptr);
}

// This loops though all ubershaders and returns the first one that meets the given requirements.
Material* ArchiveCache::getMaterial(const ArchiveRequirements& reqs) {
    assert_invariant(mArchive && "Please call load() before requesting any materials.");
    if (mArchive == nullptr) {
        return nullptr;
    }

    for (uint64_t i = 0; i < mArchive->specsCount; ++i) {
        const ArchiveSpec& spec = mArchive->specs[i];
        if (spec.blendingMode != INVALID_BLENDING && spec.blendingMode != reqs.blendingMode) {
            debugSuitability(i, "blend mode mismatch.");
            continue;
        }
        if (spec.shadingModel != INVALID_SHADING_MODEL && spec.shadingModel != reqs.shadingModel) {
            debugSuitability(i, "material model.");
            continue;
        }
        bool specIsSuitable = true;

        // For each feature required by the mesh, this ubershader is suitable only if it includes a
        // feature flag for it and the feature flag is either OPTIONAL or REQUIRED.
        for (const auto& req : reqs.features) {
            const CString& meshRequirement = req.first;
            if (req.second == false) {
                continue;
            }
            bool found = false;
            for (uint64_t j = 0; j < spec.flagsCount && !found; ++j) {
                const ArchiveFlag& flag = spec.flags[j];
                if (strIsEqual(meshRequirement, flag.name)) {
                    if (flag.value != ArchiveFeature::UNSUPPORTED) {
                        found = true;
                    }
                    break;
                }
            }
            if (!found) {
                debugSuitability(i, meshRequirement.c_str());
                specIsSuitable = false;
                break;
            }
        }

        // If this ubershader requires a certain feature to be enabled in the glTF, but the glTF
        // mesh doesn't have it, then this ubershader is not suitable. This occurs very rarely, so
        // it intentionally comes after the other suitability check.
        for (uint64_t j = 0; j < spec.flagsCount && specIsSuitable; ++j) {
            ArchiveFlag const& flag = spec.flags[j];
            if (UTILS_UNLIKELY(flag.value == ArchiveFeature::REQUIRED)) {
                // This allocates a new CString just to make a robin_map lookup, but this is rare
                // because almost none of our feature flags are REQUIRED.
                auto iter = reqs.features.find(CString(flag.name));
                if (iter == reqs.features.end() || iter.value() == false) {
                    debugSuitability(i, flag.name);
                    specIsSuitable = false;
                }
            }
        }

        if (specIsSuitable) {
            if (mMaterials[i] == nullptr) {
                mMaterials[i] = Material::Builder()
                    .package(spec.package, spec.packageByteCount)
                    .build(mEngine);
            }

            return mMaterials[i];
        }
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
    for (auto mat : mMaterials) mEngine.destroy(mat);
    mMaterials.clear();
}

FeatureMap ArchiveCache::getFeatureMap(Material* material) const {
    FeatureMap features;
    for (size_t specIndex = 0; specIndex < mMaterials.size(); ++specIndex) {
        if (material == mMaterials[specIndex]) {
            const ArchiveSpec& spec = mArchive->specs[specIndex];
            for (uint64_t j = 0; j < spec.flagsCount; ++j) {
                const ArchiveFlag& flag = spec.flags[j];
                features[flag.name] = flag.value;
            }
            break;
        }
    }
    return features;
}

ArchiveCache::~ArchiveCache() {
    assert_invariant(mMaterials.empty() &&
        "Please call destroyMaterials explicitly to ensure correct destruction order");
    utils::aligned_free(mArchive);
}

} // namespace filament::gltfio
