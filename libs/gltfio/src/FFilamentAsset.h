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

#include <cgltf.h>

#include "upcast.h"
#include "Wireframe.h"

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
    FFilamentAsset(filament::Engine* engine) : mEngine(engine) {}

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
        return mEntities.data();
    }

    utils::Entity getRoot() const noexcept {
        return mRoot;
    }

    size_t getMaterialInstanceCount() const noexcept {
        return mMaterialInstances.size();
    }

    const filament::MaterialInstance* const* getMaterialInstances() const noexcept {
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

    filament::Aabb getBoundingBox() const noexcept {
        return mBoundingBox;
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
        mBufferBindings.clear();
        mBufferBindings.shrink_to_fit();
        mTextureBindings.clear();
        mTextureBindings.shrink_to_fit();
        mNodeMap.clear();
        mPrimMap.clear();
        releaseSourceAsset();
    }

    void acquireSourceAsset() {
        ++mSourceAssetRefCount;
    }

    void releaseSourceAsset() {
        if (--mSourceAssetRefCount == 0) {
            mGlbData.clear();
            mGlbData.shrink_to_fit();
            cgltf_free((cgltf_data*) mSourceAsset);
            mSourceAsset = nullptr;
        }
    }

    filament::Engine* mEngine;
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

    /** @{
     * Transient source data that can freed via releaseSourceData(). */
    std::vector<BufferBinding> mBufferBindings;
    std::vector<TextureBinding> mTextureBindings;
    const cgltf_data* mSourceAsset = nullptr;
    tsl::robin_map<const cgltf_node*, utils::Entity> mNodeMap;
    tsl::robin_map<const cgltf_primitive*, filament::VertexBuffer*> mPrimMap;
    /** @} */
};

FILAMENT_UPCAST(FilamentAsset)

} // namespace details
} // namespace gltfio

#endif // GLTFIO_FFILAMENTASSET_H
