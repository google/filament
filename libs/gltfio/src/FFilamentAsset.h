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

#ifndef GLTFIO_FFILAMENTASSET_H
#define GLTFIO_FFILAMENTASSET_H

#include <gltfio/FilamentAsset.h>

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>

#include <math/mat4.h>

#include <utils/Entity.h>

#include <cgltf.h>

#include "upcast.h"
#include "DependencyGraph.h"
#include "DracoCache.h"
#include "FFilamentInstance.h"

#include <tsl/robin_map.h>
#include <tsl/htrie_map.h>

#include <vector>

namespace utils {
    class NameComponentManager;
    class EntityManager;
}

namespace gltfio {

class Animator;
class Wireframe;

// Encapsulates VertexBuffer::setBufferAt() or IndexBuffer::setBuffer().
struct BufferSlot {
    const cgltf_accessor* accessor;
    cgltf_attribute_type attribute;
    int bufferIndex; // for vertex buffers only
    int morphTarget; // 0 if no morphing, otherwise 1-based index
    filament::VertexBuffer* vertexBuffer;
    filament::IndexBuffer* indexBuffer;
};

// Encapsulates a connection between Texture and MaterialInstance.
struct TextureSlot {
    const cgltf_texture* texture;
    filament::MaterialInstance* materialInstance;
    const char* materialParameter;
    filament::TextureSampler sampler;
    bool srgb;
};

struct FFilamentAsset : public FilamentAsset {
    FFilamentAsset(filament::Engine* engine, utils::NameComponentManager* names,
            utils::EntityManager* entityManager) :
            mEngine(engine), mNameManager(names), mEntityManager(entityManager) {}

    ~FFilamentAsset();

    size_t getEntityCount() const noexcept {
        return mEntities.size();
    }

    const utils::Entity* getEntities() const noexcept {
        return mEntities.empty() ? nullptr : mEntities.data();
    }

    const utils::Entity* getLightEntities() const noexcept {
        return mLightEntities.empty() ? nullptr : mLightEntities.data();
    }

    size_t getLightEntityCount() const noexcept {
        return mLightEntities.size();
    }

    const utils::Entity* getCameraEntities() const noexcept {
        return mCameraEntities.empty() ? nullptr : mCameraEntities.data();
    }

    size_t getCameraEntityCount() const noexcept {
        return mCameraEntities.size();
    }

    utils::Entity getRoot() const noexcept {
        return mRoot;
    }

    size_t popRenderables(utils::Entity* entities, size_t count) noexcept {
        return mDependencyGraph.popRenderables(entities, count);
    }

    size_t getMaterialInstanceCount() const noexcept {
        return mMaterialInstances.size();
    }

    const filament::MaterialInstance* const* getMaterialInstances() const noexcept {
        return mMaterialInstances.data();
    }

    filament::MaterialInstance* const* getMaterialInstances() noexcept {
        return mMaterialInstances.data();
    }

    size_t getResourceUriCount() const noexcept {
        return mResourceUris.size();
    }

    const char* const* getResourceUris() const noexcept {
        return mResourceUris.data();
    }

    filament::Aabb getBoundingBox() const noexcept {
        return mBoundingBox;
    }

    const char* getName(utils::Entity entity) const noexcept;

    utils::Entity getFirstEntityByName(const char* name) noexcept;

    size_t getEntitiesByName(const char* name, utils::Entity* entities,
            size_t maxCount) const noexcept;

    size_t getEntitiesByPrefix(const char* prefix, utils::Entity* entities,
            size_t maxCount) const noexcept;

    Animator* getAnimator() noexcept;

    utils::Entity getWireframe() noexcept;

    filament::Engine* getEngine() const noexcept {
        return mEngine;
    }

    void releaseSourceData() noexcept;

    const void* getSourceAsset() noexcept {
        return mSourceAsset;
    }

    FilamentInstance** getAssetInstances() noexcept {
        return (FilamentInstance**) mInstances.data();
    }

    size_t getAssetInstanceCount() const noexcept {
        return mInstances.size();
    }

    void acquireSourceAsset() {
        ++mSourceAssetRefCount;
    }

    void releaseSourceAsset();

    void takeOwnership(filament::Texture* texture) {
        mTextures.push_back(texture);
    }

    void bindTexture(const TextureSlot& tb, filament::Texture* texture) {
        tb.materialInstance->setParameter(tb.materialParameter, texture, tb.sampler);
        mDependencyGraph.addEdge(texture, tb.materialInstance, tb.materialParameter);
    }

    filament::Engine* mEngine;
    utils::NameComponentManager* mNameManager;
    utils::EntityManager* mEntityManager;
    std::vector<uint8_t> mGlbData;
    std::vector<utils::Entity> mEntities;
    std::vector<utils::Entity> mLightEntities;
    std::vector<utils::Entity> mCameraEntities;
    std::vector<filament::MaterialInstance*> mMaterialInstances;
    std::vector<filament::VertexBuffer*> mVertexBuffers;
    std::vector<filament::IndexBuffer*> mIndexBuffers;
    std::vector<filament::Texture*> mTextures;
    filament::Aabb mBoundingBox;
    utils::Entity mRoot;
    std::vector<FFilamentInstance*> mInstances;
    SkinVector mSkins; // unused for instanced assets
    Animator* mAnimator = nullptr;
    Wireframe* mWireframe = nullptr;
    int mSourceAssetRefCount = 0;
    bool mResourcesLoaded = false;
    bool mSharedSourceAsset = false;
    DependencyGraph mDependencyGraph;
    DracoCache mDracoCache;
    tsl::htrie_map<char, std::vector<utils::Entity>> mNameToEntity;

    // Sentinels for situations where ResourceLoader needs to generate data.
    const cgltf_accessor mGenerateNormals = {};
    const cgltf_accessor mGenerateTangents = {};

    // Transient source data that can freed via releaseSourceData:
    std::vector<BufferSlot> mBufferSlots;
    std::vector<TextureSlot> mTextureSlots;
    std::vector<const char*> mResourceUris;
    const cgltf_data* mSourceAsset = nullptr;
    NodeMap mNodeMap; // unused for instanced assets
    std::vector<std::pair<const cgltf_primitive*, filament::VertexBuffer*> > mPrimitives;
};

FILAMENT_UPCAST(FilamentAsset)

} // namespace gltfio

#endif // GLTFIO_FFILAMENTASSET_H
