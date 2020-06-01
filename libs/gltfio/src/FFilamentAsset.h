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
#include <gltfio/Animator.h>

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
#include <utils/NameComponentManager.h>

#include <cgltf.h>

#include "upcast.h"
#include "Wireframe.h"
#include "DependencyGraph.h"
#include "DracoCache.h"
#include "FFilamentInstance.h"

#include <tsl/robin_map.h>

#include <string>
#include <vector>

namespace gltfio {

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
    FFilamentAsset(filament::Engine* engine, utils::NameComponentManager* names) :
            mEngine(engine), mNameManager(names) {}

    ~FFilamentAsset() {
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
            mEngine->destroy(entity);
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

    const char* getName(utils::Entity entity) const noexcept {
        if (mNameManager == nullptr) {
            return nullptr;
        }
        auto nameInstance = mNameManager->getInstance(entity);
        return nameInstance ? mNameManager->getName(nameInstance) : nullptr;
    }

    Animator* getAnimator() noexcept {
        if (!mAnimator) {
            mAnimator = new Animator(this, nullptr);
        }
        return mAnimator;
    }

    utils::Entity getWireframe() noexcept {
        if (!mWireframe) {
            mWireframe = new Wireframe(this);
        }
        return mWireframe->mEntity;
    }

    filament::Engine* getEngine() const noexcept {
        return mEngine;
    }

    void releaseSourceData() noexcept {
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

    void releaseSourceAsset() {
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

    void takeOwnership(filament::Texture* texture) {
        mTextures.push_back(texture);
    }

    void bindTexture(const TextureSlot& tb, filament::Texture* texture) {
        tb.materialInstance->setParameter(tb.materialParameter, texture, tb.sampler);
        mDependencyGraph.addEdge(texture, tb.materialInstance, tb.materialParameter);
    }

    filament::Engine* mEngine;
    utils::NameComponentManager* mNameManager;
    std::vector<uint8_t> mGlbData;
    std::vector<utils::Entity> mEntities;
    std::vector<utils::Entity> mLightEntities;
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
