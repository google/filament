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

#include "FFilamentInstance.h"
#include "FFilamentAsset.h"

#include <gltfio/Animator.h>

#include <utils/JobSystem.h>
#include <utils/Log.h>

using namespace filament;
using namespace filament::math;
using namespace utils;

namespace filament::gltfio {

Animator* FFilamentInstance::getAnimator() const noexcept {
    assert_invariant(animator);
    return animator;
}

void FFilamentInstance::createAnimator() {
    assert_invariant(animator == nullptr);
    animator = new Animator(owner, this);
}

size_t FFilamentInstance::getSkinCount() const noexcept {
    return skins.size();
}

const char* FFilamentInstance::getSkinNameAt(size_t skinIndex) const noexcept {
    if (skins.size() <= skinIndex) {
        return nullptr;
    }
    return owner->mSkins[skinIndex].name.c_str();
}

size_t FFilamentInstance::getJointCountAt(size_t skinIndex) const noexcept {
    if (skins.size() <= skinIndex) {
        return 0;
    }
    return skins[skinIndex].joints.size();
}

const utils::Entity* FFilamentInstance::getJointsAt(size_t skinIndex) const noexcept {
    if (skins.size() <= skinIndex) {
        return nullptr;
    }
    return skins[skinIndex].joints.data();
}

void FFilamentInstance::attachSkin(size_t skinIndex, Entity target) noexcept {
    if (UTILS_UNLIKELY(skins.size() <= skinIndex || target.isNull())) {
        return;
    }
    skins[skinIndex].targets.insert(target);
}

void FFilamentInstance::detachSkin(size_t skinIndex, Entity target) noexcept {
    if (UTILS_UNLIKELY(skins.size() <= skinIndex || target.isNull())) {
        return;
    }
    skins[skinIndex].targets.erase(target);
}

void FFilamentInstance::applyMaterialVariant(size_t variantIndex) noexcept {
    if (variantIndex >= variants.size()) {
        return;
    }
    const auto& mappings = variants[variantIndex].mappings;
    RenderableManager& rm = owner->mEngine->getRenderableManager();
    for (const auto& mapping : mappings) {
        auto renderable = rm.getInstance(mapping.renderable);
        rm.setMaterialInstanceAt(renderable, mapping.primitiveIndex, mapping.material);
    }
}

void FFilamentInstance::recomputeBoundingBoxes() {
    ASSERT_PRECONDITION(owner->mSourceAsset,
            "Do not call releaseSourceData before recomputeBoundingBoxes");

    ASSERT_PRECONDITION(owner->mResourcesLoaded,
            "Do not call recomputeBoundingBoxes before loadResources or asyncBeginLoad");

    auto& rm = owner->mEngine->getRenderableManager();
    auto& tm = owner->mEngine->getTransformManager();

    // The purpose of the root node is to give the client a place for custom transforms.
    // Since it is not part of the source model, it should be ignored when computing the
    // bounding box.
    TransformManager::Instance root = tm.getInstance(owner->getRoot());
    utils::FixedCapacityVector<Entity> modelRoots(tm.getChildCount(root));
    tm.getChildren(root, modelRoots.data(), modelRoots.size());
    for (auto e : modelRoots) {
        tm.setParent(tm.getInstance(e), 0);
    }

    struct Prim {
        cgltf_primitive const* prim;
        Entity node;
        ssize_t skinIndex;
    };

    auto computeBoundingBox = [](const cgltf_primitive* prim) -> Aabb {
        Aabb aabb;
        for (cgltf_size slot = 0; slot < prim->attributes_count; slot++) {
            const cgltf_attribute& attr = prim->attributes[slot];
            const cgltf_accessor* accessor = attr.data;
            const size_t dim = cgltf_num_components(accessor->type);
            if (attr.type == cgltf_attribute_type_position && dim >= 3) {
                utils::FixedCapacityVector<float> unpacked(accessor->count * dim);
                cgltf_accessor_unpack_floats(accessor, unpacked.data(), unpacked.size());
                for (cgltf_size i = 0, j = 0, n = accessor->count; i < n; ++i, j += dim) {
                    float3 pt(unpacked[j + 0], unpacked[j + 1], unpacked[j + 2]);
                    aabb.min = min(aabb.min, pt);
                    aabb.max = max(aabb.max, pt);
                }
                break;
            }
        }
        return aabb;
    };

    auto computeBoundingBoxSkinned = [&tm, this](const Prim& prim) -> Aabb {
        FixedCapacityVector<float3> verts;
        FixedCapacityVector<uint4> joints;
        FixedCapacityVector<float4> weights;
        for (cgltf_size slot = 0, n = prim.prim->attributes_count; slot < n; ++slot) {
            const cgltf_attribute& attr = prim.prim->attributes[slot];
            const cgltf_accessor& accessor = *attr.data;
            switch (attr.type) {
            case cgltf_attribute_type_position:
                verts = FixedCapacityVector<float3>(accessor.count);
                cgltf_accessor_unpack_floats(&accessor, &verts.data()->x, accessor.count * 3);
                break;
            case cgltf_attribute_type_joints: {
                FixedCapacityVector<float4> tmp(accessor.count);
                cgltf_accessor_unpack_floats(&accessor, &tmp.data()->x, accessor.count * 4);
                joints = FixedCapacityVector<uint4>(accessor.count);
                for (size_t i = 0, n = accessor.count; i < n; ++i) {
                    joints[i] = uint4(tmp[i]);
                }
                break;
            }
            case cgltf_attribute_type_weights:
                weights = FixedCapacityVector<float4>(accessor.count);
                cgltf_accessor_unpack_floats(&accessor, &weights.data()->x, accessor.count * 4);
                break;
            default:
                break;
            }
        }

        Aabb aabb;
        TransformManager::Instance transformable = tm.getInstance(prim.node);
        const mat4f inverseGlobalTransform = inverse(tm.getWorldTransform(transformable));
        const Skin& instanceSkin = skins[prim.skinIndex];
        const FFilamentAsset::Skin& assetSkin = owner->mSkins[prim.skinIndex];
        for (size_t i = 0, n = verts.size(); i < n; i++) {
            mat4f tmp = mat4f(0.0f);
            for (size_t j = 0; j < 4; j++) {
                size_t jointIndex = joints[i][j];
                Entity jointEntity = instanceSkin.joints[jointIndex];
                mat4f globalJointTransform = tm.getWorldTransform(tm.getInstance(jointEntity));
                mat4f inverseBindMatrix = assetSkin.inverseBindMatrices[jointIndex];
                tmp += weights[i][j] *  globalJointTransform * inverseBindMatrix;
            }
            const mat4f skinMatrix = inverseGlobalTransform * tmp;

            const float3 point = verts[i];

            // NOTE: Filament's vertex shader assumes that last row is [0,0,0,1]
            // so we make the same assumption in the following transformation.

            const float3 skinnedPoint =
                    point.x * skinMatrix[0].xyz +
                    point.y * skinMatrix[1].xyz +
                    point.z * skinMatrix[2].xyz +
                    skinMatrix[3].xyz;

            aabb.min = min(aabb.min, skinnedPoint);
            aabb.max = max(aabb.max, skinnedPoint);
        }
        return aabb;
    };

    // Collect all mesh primitives that we wish to find bounds for. For each mesh primitive, we also
    // collect the skin it is bound to (nullptr if not skinned) for bounds computation.
    size_t primCount = 0;
    for (auto iter : nodeMap) {
        const cgltf_mesh* mesh = iter.first->mesh;
        if (mesh) {
            primCount += mesh->primitives_count;
        }
    }
    auto primitives = FixedCapacityVector<Prim>::with_capacity(primCount);
    const cgltf_skin* baseSkin = &owner->mSourceAsset->hierarchy->skins[0];
    for (auto [node, entity] : nodeMap) {
        if (const cgltf_mesh* mesh = node->mesh; mesh) {
            for (cgltf_size index = 0, nprims = mesh->primitives_count; index < nprims; ++index) {
                primitives.push_back({
                    .prim = &mesh->primitives[index],
                    .node = entity,
                    .skinIndex = node->skin ? (node->skin - baseSkin) : -1
                });
            }
        }
    }

    // Kick off a bounding box job for every primitive.
    FixedCapacityVector<Aabb> bounds(primitives.size());
    JobSystem& js = owner->mEngine->getJobSystem();
    JobSystem::Job* parent = js.createJob();
    for (size_t i = 0; i < primitives.size(); ++i) {
        Aabb& result = bounds[i];
        const Prim& prim = primitives[i];
        if (primitives[i].skinIndex < 0) {
            js.run(jobs::createJob(js, parent, [&prim, &result, computeBoundingBox] {
                result = computeBoundingBox(prim.prim);
            }));
        } else {
            js.run(jobs::createJob(js, parent, [&prim, &result, computeBoundingBoxSkinned] {
                result = computeBoundingBoxSkinned(prim);
            }));
        }
    }
    js.runAndWait(parent);

    // Compute the asset-level bounding box.
    size_t primIndex = 0;
    Aabb assetBounds;
    for (auto iter : nodeMap) {
        const cgltf_mesh* mesh = iter.first->mesh;
        if (mesh) {
            // Find the object-space bounds for the renderable by unioning the bounds of each prim.
            Aabb aabb;
            for (cgltf_size index = 0, nprims = mesh->primitives_count; index < nprims; ++index) {
                Aabb primBounds = bounds[primIndex++];
                aabb.min = min(aabb.min, primBounds.min);
                aabb.max = max(aabb.max, primBounds.max);
            }
            auto renderable = rm.getInstance(iter.second);
            rm.setAxisAlignedBoundingBox(renderable, Box().set(aabb.min, aabb.max));

            // Transform this bounding box, then update the asset-level bounding box.
            auto transformable = tm.getInstance(iter.second);
            const mat4f worldTransform = tm.getWorldTransform(transformable);
            const Aabb transformed = aabb.transform(worldTransform);
            assetBounds.min = min(assetBounds.min, transformed.min);
            assetBounds.max = max(assetBounds.max, transformed.max);
        }
    }

    // Restore the root node.
    for (auto e : modelRoots) {
        tm.setParent(tm.getInstance(e), root);
    }

    boundingBox = assetBounds;
}

FilamentAsset* FilamentInstance::getAsset() const noexcept {
    return upcast(this)->owner;
}

size_t FilamentInstance::getEntityCount() const noexcept {
    return upcast(this)->entities.size();
}

const Entity* FilamentInstance::getEntities() const noexcept {
    const auto& entities = upcast(this)->entities;
    return entities.empty() ? nullptr : entities.data();
}

Entity FilamentInstance::getRoot() const noexcept {
    return upcast(this)->root;
}

void FilamentInstance::applyMaterialVariant(size_t variantIndex) noexcept {
    return upcast(this)->applyMaterialVariant(variantIndex);
}

Animator* FilamentInstance::getAnimator() noexcept {
    return upcast(this)->getAnimator();
}

size_t FilamentInstance::getSkinCount() const noexcept {
    return upcast(this)->getSkinCount();
}

const char* FilamentInstance::getSkinNameAt(size_t skinIndex) const noexcept {
    return upcast(this)->getSkinNameAt(skinIndex);
}

size_t FilamentInstance::getJointCountAt(size_t skinIndex) const noexcept {
    return upcast(this)->getJointCountAt(skinIndex);
}

const Entity* FilamentInstance::getJointsAt(size_t skinIndex) const noexcept {
    return upcast(this)->getJointsAt(skinIndex);
}

void FilamentInstance::attachSkin(size_t skinIndex, Entity target) noexcept {
    return upcast(this)->attachSkin(skinIndex, target);
}

void FilamentInstance::detachSkin(size_t skinIndex, Entity target) noexcept {
    return upcast(this)->detachSkin(skinIndex, target);
}

void FilamentInstance::recomputeBoundingBoxes() {
    return upcast(this)->recomputeBoundingBoxes();
}

Aabb FilamentInstance::getBoundingBox() const noexcept {
    return upcast(this)->boundingBox;
}

} // namespace filament::gltfio
