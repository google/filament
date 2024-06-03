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

#include <filament/RenderableManager.h>
#include <filament/Scene.h>

#include <utils/EntityManager.h>
#include <utils/Log.h>
#include <utils/NameComponentManager.h>

#include "GltfEnums.h"
#include "Wireframe.h"

using namespace filament;
using namespace utils;

namespace filament::gltfio {

FFilamentAsset::~FFilamentAsset() {
    // Free transient load-time data if they haven't been freed yet.
    releaseSourceData();

    // Destroy all instance objects. Instance entities / components are
    // destroyed later in this method because they are owned by the asset
    // (except for the root of the instance).
    for (FFilamentInstance* instance : mInstances) {
        mEntityManager->destroy(instance->mRoot);
        delete instance;
    }

    delete mWireframe;

    // Destroy name components.
    if (mNameManager) {
        for (auto entity : mEntities) {
            mNameManager->removeComponent(entity);
        }
    }

    // Destroy gltfio node components.
    for (auto entity : mEntities) {
        mNodeManager->destroy(entity);

    }

    // Destroy gltfio trs transform components.
    for (auto entity : mEntities) {
        mTrsTransformManager->destroy(entity);
    }

    // Destroy all renderable, light, transform, and camera components,
    // then destroy the actual entities. This includes instances.
    if (!mDetachedFilamentComponents) {
        mEngine->destroy(mRoot);
        mEntityManager->destroy(mRoot);
        for (auto entity : mEntities) {
            mEngine->destroy(entity);
            mEntityManager->destroy(entity);
        }
    }

    for (auto vb : mVertexBuffers) {
        mEngine->destroy(vb);
    }
    for (auto bo : mBufferObjects) {
        mEngine->destroy(bo);
    }
    for (auto ib : mIndexBuffers) {
        mEngine->destroy(ib);
    }
    for (auto tx : mTextures) {
        if (UTILS_LIKELY(tx.isOwner)) {
            mEngine->destroy(tx.texture);
        }
    }
    for (auto tb : mMorphTargetBuffers) {
        mEngine->destroy(tb);
    }
}

const char* FFilamentAsset::getExtras(utils::Entity entity) const noexcept {
    if (entity.isNull()) {
        return mAssetExtras.c_str();
    }
    return mNodeManager->getExtras(mNodeManager->getInstance(entity)).c_str();
}

void FFilamentAsset::addTextureBinding(MaterialInstance* materialInstance,
        const char* parameterName, const cgltf_texture* srcTexture,
        TextureProvider::TextureFlags flags) {
    if (!srcTexture->image && !srcTexture->basisu_image) {
#ifndef NDEBUG
        slog.w << "Texture is missing image (" << srcTexture->name << ")." << io::endl;
#endif
        return;
    }

    const size_t textureIndex = (size_t) (srcTexture - mSourceAsset->hierarchy->textures);
    TextureInfo& info = mTextures[textureIndex];

    // All bindings for a particular glTF texture must have the same transform function.
    assert_invariant(info.bindings.size() == 0 || info.flags == flags);
    info.flags = flags;

    const TextureSlot slot = { materialInstance, parameterName };
    if (info.texture) {
        applyTextureBinding(textureIndex, slot, false);
    } else {
        mDependencyGraph.addEdge(materialInstance, parameterName);
        info.bindings.push_back(slot);
    }
}

void FFilamentAsset::applyTextureBinding(size_t textureIndex, const TextureSlot& tb,
        bool addDependency) {
    const TextureInfo& info = mTextures[textureIndex];
    assert_invariant(info.texture);
    const cgltf_sampler* srcSampler = mSourceAsset->hierarchy->textures[textureIndex].sampler;
    TextureSampler sampler;
    if (srcSampler) {
        sampler.setWrapModeS(getWrapMode(srcSampler->wrap_s));
        sampler.setWrapModeT(getWrapMode(srcSampler->wrap_t));
        sampler.setMagFilter(getMagFilter(srcSampler->mag_filter));
        sampler.setMinFilter(getMinFilter(srcSampler->min_filter));
    } else {
        // These defaults are stipulated by the spec:
        sampler.setWrapModeS(TextureSampler::WrapMode::REPEAT);
        sampler.setWrapModeT(TextureSampler::WrapMode::REPEAT);

        // These defaults are up to the implementation but since we try to provide mipmaps,
        // we might as well use them. In practice the conformance models look awful without
        // using mipmapping by default.
        sampler.setMagFilter(TextureSampler::MagFilter::LINEAR);
        sampler.setMinFilter(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR);
    }
    tb.materialInstance->setParameter(tb.materialParameter, info.texture, sampler);
    if (addDependency) {
        mDependencyGraph.addEdge(info.texture, tb.materialInstance, tb.materialParameter);
    }
}

const char* FFilamentAsset::getMorphTargetNameAt(utils::Entity entity,
        size_t targetIndex) const noexcept {
    if (!mResourcesLoaded) {
        return nullptr;
    }

    const auto& names =  mNodeManager->getMorphTargetNames(mNodeManager->getInstance(entity));
    if (targetIndex >= names.size()) {
        return nullptr;
    }

    return names[targetIndex].c_str();
}

size_t FFilamentAsset::getMorphTargetCountAt(utils::Entity entity) const noexcept {
    if (!mResourcesLoaded) {
        return 0;
    }

    const auto& names = mNodeManager->getMorphTargetNames(mNodeManager->getInstance(entity));
    return names.size();
}

Entity FFilamentAsset::getWireframe() noexcept {
    if (!mWireframe) {
        mWireframe = new Wireframe(this);
    }
    return mWireframe->mEntity;
}

void FFilamentAsset::releaseSourceData() noexcept {
    // To ensure that all possible memory is freed, we reassign to new containers rather than
    // calling clear(). With many container types, clearing is a fast operation that merely frees
    // the storage for the items but not the actual container.
    for (auto& info : mTextures) {
        info.bindings = {};
    }
    mMeshCache = {};
    mResourceUris = {};
    mSourceAsset.reset();
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
    return iter->second.front();
}

size_t FFilamentAsset::getEntitiesByName(const char* name, Entity* entities,
        size_t maxCount) const noexcept {
    const auto iter = mNameToEntity.find(name);
    if (iter == mNameToEntity.end()) {
        return 0;
    }
    const auto& source = iter->second;
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
    if (maxCount == 0) {
        return 0;
    }
    std::string_view prefixString(prefix);
    size_t count = 0;
    for (auto& [k, v] : mNameToEntity) {
        if (k.compare(0, prefixString.size(), prefixString) == 0) {
            for (Entity entity : v) {
                if (entities) {
                    entities[count] = entity;
                }
                if (++count >= maxCount) {
                    return count;
                }
            }
        }
    }
    return count;
}

void FFilamentAsset::addEntitiesToScene(Scene& targetScene, const Entity* entities,
        size_t count, SceneMask sceneFilter) const {
    auto& nm = *mNodeManager;
    for (size_t ei = 0; ei < count; ++ei) {
        const Entity entity = entities[ei];
        NodeManager::SceneMask mask = nm.getSceneMembership(nm.getInstance(entity));
        if ((mask & sceneFilter).any()) {
            targetScene.addEntity(entity);
        }
    }
}

void FilamentAsset::detachFilamentComponents() noexcept {
    downcast(this)->detachFilamentComponents();
}

bool FilamentAsset::areFilamentComponentsDetached() const noexcept {
    return downcast(this)->mDetachedFilamentComponents;
}

size_t FilamentAsset::getEntityCount() const noexcept {
    return downcast(this)->getEntityCount();
}

const Entity* FilamentAsset::getEntities() const noexcept {
    return downcast(this)->getEntities();
}

const Entity* FilamentAsset::getLightEntities() const noexcept {
    return downcast(this)->getLightEntities();
}

size_t FilamentAsset::getLightEntityCount() const noexcept {
    return downcast(this)->getLightEntityCount();
}

const Entity* FilamentAsset::getRenderableEntities() const noexcept {
    return downcast(this)->getRenderableEntities();
}

size_t FilamentAsset::getRenderableEntityCount() const noexcept {
    return downcast(this)->getRenderableEntityCount();
}

const utils::Entity* FilamentAsset::getCameraEntities() const noexcept {
    return downcast(this)->getCameraEntities();
}

size_t FilamentAsset::getCameraEntityCount() const noexcept {
    return downcast(this)->getCameraEntityCount();
}

Entity FilamentAsset::getRoot() const noexcept {
    return downcast(this)->getRoot();
}

Entity FilamentAsset::popRenderable() noexcept {
    Entity result[1];
    const bool empty = !popRenderables(result, 1);
    return empty ? Entity() : result[0];
}

size_t FilamentAsset::popRenderables(Entity* result, size_t count) noexcept {
    return downcast(this)->popRenderables(result, count);
}

size_t FilamentAsset::getResourceUriCount() const noexcept {
    return downcast(this)->getResourceUriCount();
}

const char* const* FilamentAsset::getResourceUris() const noexcept {
    return downcast(this)->getResourceUris();
}

filament::Aabb FilamentAsset::getBoundingBox() const noexcept {
    return downcast(this)->getBoundingBox();
}

const char* FilamentAsset::getName(Entity entity) const noexcept {
    return downcast(this)->getName(entity);
}

Entity FilamentAsset::getFirstEntityByName(const char* name) noexcept {
    return downcast(this)->getFirstEntityByName(name);
}

size_t FilamentAsset::getEntitiesByName(const char* name, Entity* entities,
        size_t maxCount) const noexcept {
    return downcast(this)->getEntitiesByName(name, entities, maxCount);
}

size_t FilamentAsset::getEntitiesByPrefix(const char* prefix, Entity* entities,
        size_t maxCount) const noexcept {
    return downcast(this)->getEntitiesByPrefix(prefix, entities, maxCount);
}

const char* FilamentAsset::getExtras(Entity entity) const noexcept {
    return downcast(this)->getExtras(entity);
}

const char* FilamentAsset::getMorphTargetNameAt(utils::Entity entity,
        size_t targetIndex) const noexcept {
    return downcast(this)->getMorphTargetNameAt(entity, targetIndex);
}

size_t FilamentAsset::getMorphTargetCountAt(utils::Entity entity) const noexcept {
    return downcast(this)->getMorphTargetCountAt(entity);
}

Entity FilamentAsset::getWireframe() noexcept {
    return downcast(this)->getWireframe();
}

Engine* FilamentAsset::getEngine() const noexcept {
    return downcast(this)->getEngine();
}

void FilamentAsset::releaseSourceData() noexcept {
    return downcast(this)->releaseSourceData();
}

const void* FilamentAsset::getSourceAsset() noexcept {
    return downcast(this)->getSourceAsset();
}

FilamentInstance** FilamentAsset::getAssetInstances() noexcept {
    return downcast(this)->getAssetInstances();
}

size_t FilamentAsset::getAssetInstanceCount() const noexcept {
    return downcast(this)->getAssetInstanceCount();
}

size_t FilamentAsset::getSceneCount() const noexcept {
    return downcast(this)->getSceneCount();
}

const char* FilamentAsset::getSceneName(size_t sceneIndex) const noexcept {
    return downcast(this)->getSceneName(sceneIndex);
}

void FilamentAsset::addEntitiesToScene(Scene& targetScene, const Entity* entities, size_t count,
        SceneMask sceneFilter) const {
    downcast(this)->addEntitiesToScene(targetScene, entities, count, sceneFilter);
}

} // namespace filament::gltfio
