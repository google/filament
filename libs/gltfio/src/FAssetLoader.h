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

#ifndef GLTFIO_FASSETLOADER_H
#define GLTFIO_FASSETLOADER_H

#include <gltfio/AssetLoader.h>
#include <gltfio/MaterialProvider.h>

#include <filament/Engine.h>
#include <filament/LightManager.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include <cgltf.h>

#include "upcast.h"

namespace filament {
    class MaterialInstance;
    class RenderableManager;
    class TransformManager;
}

namespace utils {
    class NameComponentManager;
}

namespace gltfio {
class FilamentInstance;

struct FFilamentAsset;
struct FFilamentInstance;
struct Primitive;

struct FAssetLoader : public AssetLoader {
    FAssetLoader(const AssetConfiguration& config) :
        mEntityManager(config.entities ? *config.entities : utils::EntityManager::get()),
        mRenderableManager(config.engine->getRenderableManager()),
        mNameManager(config.names),
        mTransformManager(config.engine->getTransformManager()),
        mMaterials(config.materials),
        mEngine(config.engine),
        mDefaultNodeName(config.defaultNodeName) {}

    FFilamentAsset* createAssetFromJson(const uint8_t* bytes, uint32_t nbytes);
    FFilamentAsset* createAssetFromBinary(const uint8_t* bytes, uint32_t nbytes);
    FFilamentAsset* createInstancedAsset(const uint8_t* bytes, uint32_t numBytes,
        FilamentInstance** instances, size_t numInstances);
    FilamentInstance* createInstance(FFilamentAsset* primary);

    bool createAssets(const uint8_t* bytes, uint32_t numBytes, FilamentAsset** assets,
        size_t numAssets);

    ~FAssetLoader() {
        delete mMaterials;
    }

    void destroyAsset(const FFilamentAsset* asset) {
        delete asset;
    }

    size_t getMaterialsCount() const noexcept {
        return mMaterials->getMaterialsCount();
    }

    utils::NameComponentManager* getNames() const noexcept {
        return mNameManager;
    }

    const filament::Material* const* getMaterials() const noexcept {
        return mMaterials->getMaterials();
    }

    void createAsset(const cgltf_data* srcAsset, size_t numInstances);
    FFilamentInstance* createInstance(FFilamentAsset* primary, const cgltf_scene* scene);
    void createEntity(const cgltf_node* node, utils::Entity parent, bool enableLight,
        FFilamentInstance* instance);
    void createRenderable(const cgltf_node* node, utils::Entity entity, const char* name);
    bool createPrimitive(const cgltf_primitive* inPrim, Primitive* outPrim, const UvMap& uvmap,
        const char* name);
    void createLight(const cgltf_light* light, utils::Entity entity);
    void createCamera(const cgltf_camera* camera, utils::Entity entity);
    filament::MaterialInstance* createMaterialInstance(const cgltf_material* inputMat, UvMap* uvmap,
        bool vertexColor);
    void addTextureBinding(filament::MaterialInstance* materialInstance, const char* parameterName,
        const cgltf_texture* srcTexture, bool srgb);
    bool primitiveHasVertexColor(const cgltf_primitive* inPrim) const;

    static filament::LightManager::Type getLightType(const cgltf_light_type type);

    utils::EntityManager& mEntityManager;
    filament::RenderableManager& mRenderableManager;
    utils::NameComponentManager* mNameManager;
    filament::TransformManager& mTransformManager;
    MaterialProvider* mMaterials;
    filament::Engine* mEngine;

    // Transient state used only for the asset currently being loaded:
    FFilamentAsset* mResult;
    const char* mDefaultNodeName;
    bool mError = false;
    bool mDiagnosticsEnabled = false;
};

FILAMENT_UPCAST(AssetLoader)

} // namespace gltfio

#endif // GLTFIO_FASSETLOADER_H
