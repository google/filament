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

#include <tsl/robin_map.h>

#include <set>
#include <string>
#include <vector>

namespace gltfio {
namespace details {

struct Skin {
    std::string name;
    std::vector<filament::math::mat4f> inverseBindMatrices;
    std::vector<utils::Entity> joints;
    std::vector<utils::Entity> targets;
};

struct FFilamentAsset : public FilamentAsset {
    FFilamentAsset(filament::Engine* engine, utils::NameComponentManager* names) :
            mEngine(engine), mNameManager(names) {}

    ~FFilamentAsset() {
        releaseSourceData();
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

    size_t getBufferBindingCount() const noexcept {
        return mBufferBindings.size();
    }

    const BufferBinding* getBufferBindings() const noexcept {
        return mBufferBindings.data();
    }

    size_t getTextureBindingCount() const noexcept {
        return mTextureBindings.size();
    }

    const TextureBinding* getTextureBindings() const noexcept {
        return mTextureBindings.data();
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
            mAnimator = new Animator(this);
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
        // TODO: bundle all these transient items into a "SourceData" struct.
        mBufferBindings = {};
        mTextureBindings = {};
        mResourceUris = {};
        mNodeMap = {};
        mPrimMap = {};
        mAccessorMap = {};
        releaseSourceAsset();
    }

    const void* getSourceAsset() noexcept {
        return mSourceAsset;
    }

    void acquireSourceAsset() {
        ++mSourceAssetRefCount;
    }

    void releaseSourceAsset() {
        if (--mSourceAssetRefCount == 0) {
            mGlbData.clear();
            mGlbData.shrink_to_fit();
            if (!mSharedSourceAsset) {
                cgltf_free((cgltf_data*) mSourceAsset);
            }
            mSourceAsset = nullptr;
        }
    }

    void takeOwnership(filament::Texture* texture) {
        mTextures.push_back(texture);
    }

    void bindTexture(const TextureBinding& tb, filament::Texture* texture) {
        tb.materialInstance->setParameter(tb.materialParameter, texture, tb.sampler);
        mDependencyGraph.addEdge(texture, tb.materialInstance, tb.materialParameter);
    }

    filament::Engine* mEngine;
    utils::NameComponentManager* mNameManager;
    std::vector<uint8_t> mGlbData;
    std::vector<utils::Entity> mEntities;
    std::vector<filament::MaterialInstance*> mMaterialInstances;
    std::vector<filament::VertexBuffer*> mVertexBuffers;
    std::vector<filament::IndexBuffer*> mIndexBuffers;
    std::vector<filament::Texture*> mTextures;
    filament::Aabb mBoundingBox;
    utils::Entity mRoot;
    std::vector<Skin> mSkins;
    Animator* mAnimator = nullptr;
    Wireframe* mWireframe = nullptr;
    int mSourceAssetRefCount = 0;
    bool mResourcesLoaded = false;
    bool mSharedSourceAsset = false;
    DependencyGraph mDependencyGraph;

    // Transient source data that can freed via releaseSourceData:
    std::vector<BufferBinding> mBufferBindings;
    std::vector<TextureBinding> mTextureBindings;
    std::vector<const char*> mResourceUris;
    const cgltf_data* mSourceAsset = nullptr;
    tsl::robin_map<const cgltf_node*, utils::Entity> mNodeMap;
    tsl::robin_map<const cgltf_primitive*, filament::VertexBuffer*> mPrimMap;
    tsl::robin_map<const cgltf_accessor*, std::vector<filament::VertexBuffer*>> mAccessorMap;
};

FILAMENT_UPCAST(FilamentAsset)

} // namespace details
} // namespace gltfio

#endif // GLTFIO_FFILAMENTASSET_H
