/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "FFilamentAsset.h"
#include "FFilamentInstance.h"

#include <math/vec4.h>

#include <tsl/robin_map.h>

#include <vector>

struct cgltf_node;
struct cgltf_mesh;
struct cgltf_primitive;

namespace gltfio {

/**
 * Internal class that partitions lists of morph weights and maintains a cache of BufferObject
 * instances. This allows gltfio to support up to 255 morph targets.
 *
 * Each partition is associated with an unordered set of 4 (or fewer) morph target indices, which
 * we call the "primary indices" for that time slice.
 *
 * Animator has ownership over a single instance of MorphHelper, thus it is 1:1 with FilamentAsset.
 */
class MorphHelper {
public:
    using Entity = utils::Entity;
    MorphHelper(FFilamentAsset* asset, FFilamentInstance* inst);
    ~MorphHelper();

    /**
     * Picks the 4 most influential weights and applies them to the target entity.
     */
    void applyWeights(Entity targetEntity, float const* weights, size_t count) noexcept;

private:
    struct GltfTarget {
        filament::BufferObject* bufferObject;
        int morphTargetIndex;
        cgltf_attribute_type type;
    };

    struct GltfPrimitive {
        filament::VertexBuffer* vertexBuffer;
        int baseSlot;
        std::vector<GltfTarget> targets; // TODO: flatten this?
    };

    struct TableEntry {
        std::vector<GltfPrimitive> primitives; // TODO: flatten this?
    };

    void addPrimitive(cgltf_mesh const* mesh, int primitiveIndex, TableEntry* entry);
    int determineBaseSlot(const cgltf_primitive& prim) const;

    std::vector<float> mPartiallySortedWeights;
    tsl::robin_map<Entity, TableEntry> mMorphTable;
    const FFilamentAsset* mAsset;
    const FFilamentInstance* mInstance;
};

} // namespace gltfio
