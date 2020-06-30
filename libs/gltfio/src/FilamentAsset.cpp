/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <gltfio/Animator.h>

#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>

#include "Wireframe.h"

using namespace filament;
using namespace utils;

namespace gltfio {

FFilamentAsset::~FFilamentAsset() {
    releaseSourceData();

    // The only things we need to free in the instances are their animators.
    // The union of all instance entities will be destroyed below.
    for (FFilamentInstance* instance : mInstances) {
        delete instance->animator;
        delete instance;
    }

    delete mAnimator;
    delete mWireframe;
    mEngine->destroy(mRoot);
    for (auto entity : mEntities) {
        // Destroys the entity's renderable, light, transform, and camera components.
        mEngine->destroy(entity);
        // Destroys the actual entity.
        mEntityManager->destroy(entity);
    }
    for (auto mi : mMaterialInstances) {
        mEngine->destroy(mi);
    }
    for (auto vb : mVertexBuffers) {
        mEngine->destroy(vb);
    }
    for (auto ib : mIndexBuffers) {
        mEngine->destroy(ib);
    }
    for (auto tx : mTextures) {
        mEngine->destroy(tx);
    }
}

Animator* FFilamentAsset::getAnimator() noexcept {
    if (!mAnimator) {
        mAnimator = new Animator(this, nullptr);
    }
    return mAnimator;
}

Entity FFilamentAsset::getWireframe() noexcept {
    if (!mWireframe) {
        mWireframe = new Wireframe(this);
    }
    return mWireframe->mEntity;
}

void FFilamentAsset::releaseSourceData() noexcept {
    // To ensure that all possible memory is freed, we reassign to new containers rather than
    // calling clear(). With many container types (such as robin_map), clearing is a fast
    // operation that merely frees the storage for the items.
    mResourceUris = {};
    mNodeMap = {};
    mPrimitives = {};
    mBufferSlots = {};
    mTextureSlots = {};
    releaseSourceAsset();
    for (FFilamentInstance* instance : mInstances) {
        instance->nodeMap = {};
    }
}

void FFilamentAsset::releaseSourceAsset() {
    if (--mSourceAssetRefCount == 0) {
        // At this point, all vertex buffers have been uploaded to the GPU and we can finally
        // release all remaining CPU-side source data, such as aggregated GLB buffers and Draco
        // meshes. Note that sidecar bin data is already released, because external resources
        // are released eagerly via BufferDescriptor callbacks.
        mDracoCache = {};
        mGlbData = {};
        if (!mSharedSourceAsset) {
            cgltf_free((cgltf_data*) mSourceAsset);
        }
        mSourceAsset = nullptr;
    }
}

const char* FFilamentAsset::getName(utils::Entity entity) const noexcept {
    if (mNameManager == nullptr) {
        return nullptr;
    }
    auto nameInstance = mNameManager->getInstance(entity);
    return nameInstance ? mNameManager->getName(nameInstance) : nullptr;
}

Entity FFilamentAsset::getFirstEntityByName(const char* name) noexcept {
    const auto iter = mNameToEntity.find(name);
    if (iter == mNameToEntity.end()) {
        return {};
    }
    return iter->front();
}

size_t FFilamentAsset::getEntitiesByName(const char* name, Entity* entities,
        size_t maxCount) const noexcept {
    const auto iter = mNameToEntity.find(name);
    if (iter == mNameToEntity.end()) {
        return 0;
    }
    const auto& source = *iter;
    if (entities == nullptr) {
        return source.size();
    }
    maxCount = std::min(maxCount, source.size());
    if (maxCount == 0) {
        return 0;
    }
    size_t count = 0;
    for (Entity entity : source) {
        entities[count] = entity;
        if (++count >= maxCount) {
            return count;
        }
    }
    return count;
}

size_t FFilamentAsset::getEntitiesByPrefix(const char* prefix, Entity* entities,
        size_t maxCount) const noexcept {
    const auto range = mNameToEntity.equal_prefix_range(prefix);
    size_t count = 0;
    for (auto iter = range.first; iter != range.second; ++iter) {
        count += iter->size();
    }
    if (entities == nullptr) {
        return count;
    }
    maxCount = std::min(maxCount, count);
    if (maxCount == 0) {
        return 0;
    }
    count = 0;
    for (auto iter = range.first; iter != range.second; ++iter) {
        const auto& source = *iter;
        for (Entity entity : source) {
            entities[count] = entity;
            if (++count >= maxCount) {
                return count;
            }
        }
    }
    return count;
}

size_t FilamentAsset::getEntityCount() const noexcept {
    return upcast(this)->getEntityCount();
}

const Entity* FilamentAsset::getEntities() const noexcept {
    return upcast(this)->getEntities();
}

const Entity* FilamentAsset::getLightEntities() const noexcept {
    return upcast(this)->getLightEntities();
}

size_t FilamentAsset::getLightEntityCount() const noexcept {
    return upcast(this)->getLightEntityCount();
}

const utils::Entity* FilamentAsset::getCameraEntities() const noexcept {
    return upcast(this)->getCameraEntities();
}

size_t FilamentAsset::getCameraEntityCount() const noexcept {
    return upcast(this)->getCameraEntityCount();
}

Entity FilamentAsset::getRoot() const noexcept {
    return upcast(this)->getRoot();
}

Entity FilamentAsset::popRenderable() noexcept {
    Entity result[1];
    const bool empty = !popRenderables(result, 1);
    return empty ? Entity() : result[0];
}

size_t FilamentAsset::popRenderables(Entity* result, size_t count) noexcept {
    return upcast(this)->popRenderables(result, count);
}

size_t FilamentAsset::getMaterialInstanceCount() const noexcept {
    return upcast(this)->getMaterialInstanceCount();
}

const MaterialInstance* const* FilamentAsset::getMaterialInstances() const noexcept {
    return upcast(this)->getMaterialInstances();
}

MaterialInstance* const* FilamentAsset::getMaterialInstances() noexcept {
    return upcast(this)->getMaterialInstances();
}

size_t FilamentAsset::getResourceUriCount() const noexcept {
    return upcast(this)->getResourceUriCount();
}

const char* const* FilamentAsset::getResourceUris() const noexcept {
    return upcast(this)->getResourceUris();
}

filament::Aabb FilamentAsset::getBoundingBox() const noexcept {
    return upcast(this)->getBoundingBox();
}

const char* FilamentAsset::getName(Entity entity) const noexcept {
    return upcast(this)->getName(entity);
}

Entity FilamentAsset::getFirstEntityByName(const char* name) noexcept {
    return upcast(this)->getFirstEntityByName(name);
}

size_t FilamentAsset::getEntitiesByName(const char* name, Entity* entities,
        size_t maxCount) const noexcept {
    return upcast(this)->getEntitiesByName(name, entities, maxCount);
}

size_t FilamentAsset::getEntitiesByPrefix(const char* prefix, Entity* entities,
        size_t maxCount) const noexcept {
    return upcast(this)->getEntitiesByPrefix(prefix, entities, maxCount);
}

Animator* FilamentAsset::getAnimator() noexcept {
    return upcast(this)->getAnimator();
}

Entity FilamentAsset::getWireframe() noexcept {
    return upcast(this)->getWireframe();
}

Engine* FilamentAsset::getEngine() const noexcept {
    return upcast(this)->getEngine();
}

void FilamentAsset::releaseSourceData() noexcept {
    return upcast(this)->releaseSourceData();
}

const void* FilamentAsset::getSourceAsset() noexcept {
    return upcast(this)->getSourceAsset();
}

FilamentInstance** FilamentAsset::getAssetInstances() noexcept {
    return upcast(this)->getAssetInstances();
}

size_t FilamentAsset::getAssetInstanceCount() const noexcept {
    return upcast(this)->getAssetInstanceCount();
}

} // namespace gltfio
