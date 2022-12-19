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

FFilamentInstance::FFilamentInstance(Entity root, FFilamentAsset const* owner) :
    mRoot(root),
    mOwner(owner),
    mNodeMap(owner->mSourceAsset->hierarchy->nodes_count, Entity()) {}

FFilamentInstance::~FFilamentInstance() {
    delete mAnimator;
    for (auto mi : mMaterialInstances) {
        mOwner->mEngine->destroy(mi);
    }
}

Animator* FFilamentInstance::getAnimator() const noexcept {
    assert_invariant(mAnimator);
    return mAnimator;
}

void FFilamentInstance::createAnimator() {
    if (mAnimator == nullptr && mOwner->mResourcesLoaded) {
        mAnimator = new Animator(mOwner, this);
    }
}

size_t FFilamentInstance::getSkinCount() const noexcept {
    return mSkins.size();
}

const char* FFilamentInstance::getSkinNameAt(size_t skinIndex) const noexcept {
    if (mSkins.size() <= skinIndex) {
        return nullptr;
    }
    return mOwner->mSkins[skinIndex].name.c_str();
}

size_t FFilamentInstance::getJointCountAt(size_t skinIndex) const noexcept {
    if (mSkins.size() <= skinIndex) {
        return 0;
    }
    return mSkins[skinIndex].joints.size();
}

const utils::Entity* FFilamentInstance::getJointsAt(size_t skinIndex) const noexcept {
    if (mSkins.size() <= skinIndex) {
        return nullptr;
    }
    return mSkins[skinIndex].joints.data();
}

void FFilamentInstance::attachSkin(size_t skinIndex, Entity target) noexcept {
    if (UTILS_UNLIKELY(mSkins.size() <= skinIndex || target.isNull())) {
        return;
    }
    mSkins[skinIndex].targets.insert(target);
}

void FFilamentInstance::detachSkin(size_t skinIndex, Entity target) noexcept {
    if (UTILS_UNLIKELY(mSkins.size() <= skinIndex || target.isNull())) {
        return;
    }
    mSkins[skinIndex].targets.erase(target);
}

mat4f const* FFilamentInstance::getInverseBindMatricesAt(size_t skinIndex) const {
    assert_invariant(mOwner);
    ASSERT_PRECONDITION(skinIndex < mOwner->mSkins.size(), "skinIndex must be less than the number of skins in this instance.");
    return mOwner->mSkins[skinIndex].inverseBindMatrices.data();
}

void FFilamentInstance::recomputeBoundingBoxes() {
    ASSERT_PRECONDITION(mOwner->mSourceAsset,
            "Do not call releaseSourceData before recomputeBoundingBoxes");

    ASSERT_PRECONDITION(mOwner->mResourcesLoaded,
            "Do not call recomputeBoundingBoxes before loadResources or asyncBeginLoad");

    auto& rm = mOwner->mEngine->getRenderableManager();
    auto& tm = mOwner->mEngine->getTransformManager();

    // The purpose of the root node is to give the client a place for custom transforms.
    // Since it is not part of the source model, it should be ignored when computing the
    // bounding box.
    TransformManager::Instance root = tm.getInstance(mOwner->getRoot());
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

                if (!prim->targets_count) {
                    break;
                }

                Aabb baseAabb(aabb);
                for (cgltf_size targetIndex = 0; targetIndex < prim->targets_count; ++targetIndex) {
                    const cgltf_morph_target& target = prim->targets[targetIndex];
                    for (cgltf_size attribIndex = 0; attribIndex < target.attributes_count; ++attribIndex) {
                        const cgltf_attribute& targetAttribute = target.attributes[attribIndex];
                        if (targetAttribute.type != cgltf_attribute_type_position) {
                            continue;
                        }

                        const cgltf_accessor* targetAccessor = targetAttribute.data;

                        assert_invariant(targetAccessor);
                        assert_invariant(targetAccessor->count == accessor->count);
                        assert_invariant(cgltf_num_components(targetAccessor->type) == dim);

                        cgltf_accessor_unpack_floats(targetAccessor, unpacked.data(), unpacked.size());

                        Aabb targetAabb;
                        for (cgltf_size i = 0, j = 0, n = accessor->count; i < n; ++i, j += dim) {
                            float3 delta(unpacked[j + 0], unpacked[j + 1], unpacked[j + 2]);
                            targetAabb.min = min(targetAabb.min, delta);
                            targetAabb.max = max(targetAabb.max, delta);
                        }

                        targetAabb.min += baseAabb.min;
                        targetAabb.max += baseAabb.max;

                        aabb.min = min(aabb.min, targetAabb.min);
                        aabb.max = max(aabb.max, targetAabb.max);

                        break;
                    }
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
        const Skin& instanceSkin = mSkins[prim.skinIndex];
        const FFilamentAsset::Skin& assetSkin = mOwner->mSkins[prim.skinIndex];
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
    const cgltf_data* hierarchy = mOwner->mSourceAsset->hierarchy;
    const cgltf_node* nodes = hierarchy->nodes;
    for (size_t i = 0, n = hierarchy->nodes_count; i < n; ++i) {
        if (const cgltf_mesh* mesh = nodes[i].mesh; mesh) {
            primCount += mesh->primitives_count;
        }
    }
    auto primitives = FixedCapacityVector<Prim>::with_capacity(primCount);
    const cgltf_skin* baseSkin = &hierarchy->skins[0];
    for (size_t i = 0, n = hierarchy->nodes_count; i < n; ++i) {
        const cgltf_node& node = nodes[i];
        const Entity entity = mNodeMap[i];
        if (entity.isNull()) {
            continue;
        }
        if (const cgltf_mesh* mesh = node.mesh; mesh) {
            for (cgltf_size j = 0, nprims = mesh->primitives_count; j < nprims; ++j) {
                primitives.push_back({
                    .prim = &mesh->primitives[j],
                    .node = entity,
                    .skinIndex = node.skin ? (node.skin - baseSkin) : -1
                });
            }
        }
    }

    // Kick off a bounding box job for every primitive.
    FixedCapacityVector<Aabb> bounds(primitives.size());
    JobSystem& js = mOwner->mEngine->getJobSystem();
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
    for (size_t i = 0, n = mOwner->mSourceAsset->hierarchy->nodes_count; i < n; ++i) {
        const cgltf_node& node = nodes[i];
        const Entity entity = mNodeMap[i];
        if (const cgltf_mesh* mesh = node.mesh; mesh) {
            // Find the object-space bounds for the renderable by unioning the bounds of each prim.
            Aabb aabb;
            for (cgltf_size j = 0, nprims = mesh->primitives_count; j < nprims; ++j) {
                Aabb primBounds = bounds[primIndex++];
                aabb.min = min(aabb.min, primBounds.min);
                aabb.max = max(aabb.max, primBounds.max);
            }
            auto renderable = rm.getInstance(entity);
            rm.setAxisAlignedBoundingBox(renderable, Box().set(aabb.min, aabb.max));

            // Transform this bounding box, then update the asset-level bounding box.
            auto transformable = tm.getInstance(entity);
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

    mBoundingBox = assetBounds;
}

size_t FFilamentInstance::getMaterialVariantCount() const noexcept {
    return mVariants.size();
}

const char* FFilamentInstance::getMaterialVariantName(size_t variantIndex) const noexcept {
    if (variantIndex >= mVariants.size()) {
        return nullptr;
    }
    return mVariants[variantIndex].name.c_str();
}

void FFilamentInstance::applyMaterialVariant(size_t variantIndex) noexcept {
    if (variantIndex >= mVariants.size()) {
        return;
    }
    const auto& mappings = mVariants[variantIndex].mappings;
    RenderableManager& rm = mOwner->mEngine->getRenderableManager();
    for (const auto& mapping : mappings) {
        auto renderable = rm.getInstance(mapping.renderable);
        rm.setMaterialInstanceAt(renderable, mapping.primitiveIndex, mapping.material);
    }
}

void FilamentInstance::detachMaterialInstances() {
    downcast(this)->detachMaterialInstances();
}

size_t FilamentInstance::getMaterialInstanceCount() const noexcept {
    return downcast(this)->getMaterialInstanceCount();
}

const MaterialInstance* const* FilamentInstance::getMaterialInstances() const noexcept {
    return downcast(this)->getMaterialInstances();
}

MaterialInstance* const* FilamentInstance::getMaterialInstances() noexcept {
    return downcast(this)->getMaterialInstances();
}

const char* FilamentInstance::getMaterialVariantName(size_t variantIndex) const noexcept {
    return downcast(this)->getMaterialVariantName(variantIndex);
}

void FilamentInstance::applyMaterialVariant(size_t variantIndex) noexcept {
    return downcast(this)->applyMaterialVariant(variantIndex);
}

size_t FilamentInstance::getMaterialVariantCount() const noexcept {
    return downcast(this)->getMaterialVariantCount();
}

FilamentAsset const* FilamentInstance::getAsset() const noexcept {
    return downcast(this)->mOwner;
}

size_t FilamentInstance::getEntityCount() const noexcept {
    return downcast(this)->mEntities.size();
}

const Entity* FilamentInstance::getEntities() const noexcept {
    const auto& entities = downcast(this)->mEntities;
    return entities.empty() ? nullptr : entities.data();
}

Entity FilamentInstance::getRoot() const noexcept {
    return downcast(this)->mRoot;
}

Animator* FilamentInstance::getAnimator() noexcept {
    return downcast(this)->getAnimator();
}

size_t FilamentInstance::getSkinCount() const noexcept {
    return downcast(this)->getSkinCount();
}

const char* FilamentInstance::getSkinNameAt(size_t skinIndex) const noexcept {
    return downcast(this)->getSkinNameAt(skinIndex);
}

size_t FilamentInstance::getJointCountAt(size_t skinIndex) const noexcept {
    return downcast(this)->getJointCountAt(skinIndex);
}

const Entity* FilamentInstance::getJointsAt(size_t skinIndex) const noexcept {
    return downcast(this)->getJointsAt(skinIndex);
}

void FilamentInstance::attachSkin(size_t skinIndex, Entity target) noexcept {
    return downcast(this)->attachSkin(skinIndex, target);
}

void FilamentInstance::detachSkin(size_t skinIndex, Entity target) noexcept {
    return downcast(this)->detachSkin(skinIndex, target);
}

math::mat4f const* FilamentInstance::getInverseBindMatricesAt(size_t skinIndex) const {
    return downcast(this)->getInverseBindMatricesAt(skinIndex);
}

void FilamentInstance::recomputeBoundingBoxes() {
    return downcast(this)->recomputeBoundingBoxes();
}

Aabb FilamentInstance::getBoundingBox() const noexcept {
    return downcast(this)->mBoundingBox;
}

} // namespace filament::gltfio
