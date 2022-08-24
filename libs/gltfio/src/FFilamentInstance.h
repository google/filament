/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef GLTFIO_FFILAMENTINSTANCE_H
#define GLTFIO_FFILAMENTINSTANCE_H

#include <gltfio/FilamentInstance.h>

#include <utils/CString.h>
#include <utils/Entity.h>
#include <utils/FixedCapacityVector.h>

#include <math/mat4.h>

#include <tsl/robin_map.h>
#include <tsl/robin_set.h>

#include <vector>

#include "upcast.h"

struct cgltf_node;

namespace filament {
    class MaterialInstance;
}

namespace filament::gltfio {

struct FFilamentAsset;
class Animator;

struct Skin {
    utils::CString name;

    // The inverse bind matrices and joints arrays must have the same count. Each element
    // corresponds to a single bone. We considered using the ECS to store these, but this would be
    // complicated because a single node might be used as a bone in more than one skin, and its
    // inverse bind matrix might be unique in each of these skins.
    utils::FixedCapacityVector<filament::math::mat4f> inverseBindMatrices;
    utils::FixedCapacityVector<utils::Entity> joints;

    // The set of all nodes that are influenced by this skin.
    // This is initially gleaned from the glTF file using the "skin" attribute of each node.
    tsl::robin_set<utils::Entity, utils::Entity::Hasher> targets;
};

struct VariantMapping {
    utils::Entity renderable;
    size_t primitiveIndex;
    filament::MaterialInstance* material;
};

struct Variant {
    utils::CString name;
    std::vector<VariantMapping> mappings;
};

using SkinVector = std::vector<Skin>;
using NodeMap = tsl::robin_map<const cgltf_node*, utils::Entity>;

struct FFilamentInstance : public FilamentInstance {
    std::vector<utils::Entity> entities;
    utils::FixedCapacityVector<Variant> variants;
    utils::Entity root;
    Animator* animator;
    FFilamentAsset* owner;
    SkinVector skins;
    NodeMap nodeMap;
    filament::Aabb boundingBox;
    void createAnimator();
    Animator* getAnimator() const noexcept;
    size_t getSkinCount() const noexcept;
    const char* getSkinNameAt(size_t skinIndex) const noexcept;
    size_t getJointCountAt(size_t skinIndex) const noexcept;
    const utils::Entity* getJointsAt(size_t skinIndex) const noexcept;
    void attachSkin(size_t skinIndex, utils::Entity target) noexcept;
    void detachSkin(size_t skinIndex, utils::Entity target) noexcept;
    void applyMaterialVariant(size_t variantIndex) noexcept;
    void recomputeBoundingBoxes();
};

FILAMENT_UPCAST(FilamentInstance)

} // namespace filament::gltfio

#endif // GLTFIO_FFILAMENTINSTANCE_H
