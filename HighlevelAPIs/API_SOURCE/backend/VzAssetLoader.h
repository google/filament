#pragma once
#include "../VzNameComponents.hpp"
#include "../FIncludes.h"

namespace filament::gltfio {
    using Entity = utils::Entity;
    using FFilamentAsset = filament::gltfio::FFilamentAsset;
    using SceneMask = NodeManager::SceneMask;
    using VID = uint32_t;
    using GeometryVID = VID;
    using MaterialVID = VID;
    using TextureVID = VID;
    using MInstanceVID = VID;

    // The default glTF material.
    static constexpr cgltf_material kDefaultMat = {
        .name = (char*)"Default GLTF material",
        .has_pbr_metallic_roughness = true,
        .has_pbr_specular_glossiness = false,
        .has_clearcoat = false,
        .has_transmission = false,
        .has_volume = false,
        .has_ior = false,
        .has_specular = false,
        .has_sheen = false,
        .pbr_metallic_roughness = {
            .base_color_factor = {1.0, 1.0, 1.0, 1.0},
            .metallic_factor = 1.0,
            .roughness_factor = 1.0,
        },
    };

    class MaterialInstanceCache {
    public:
        struct Entry {
            MaterialInstance* instance;
            UvMap uvmap;
        };

        MaterialInstanceCache() {}

        MaterialInstanceCache(const cgltf_data* hierarchy) :
            mHierarchy(hierarchy),
            mMaterialInstances(hierarchy->materials_count, Entry{}),
            mMaterialInstancesWithVertexColor(hierarchy->materials_count, Entry{}) {}

        void flush(utils::FixedCapacityVector<MaterialInstance*>* dest) {
            size_t count = 0;
            for (const Entry& entry : mMaterialInstances) {
                if (entry.instance) {
                    ++count;
                }
            }
            for (const Entry& entry : mMaterialInstancesWithVertexColor) {
                if (entry.instance) {
                    ++count;
                }
            }
            if (mDefaultMaterialInstance.instance) {
                ++count;
            }
            if (mDefaultMaterialInstanceWithVertexColor.instance) {
                ++count;
            }
            assert_invariant(dest->size() == 0);
            dest->reserve(count);
            for (const Entry& entry : mMaterialInstances) {
                if (entry.instance) {
                    dest->push_back(entry.instance);
                }
            }
            for (const Entry& entry : mMaterialInstancesWithVertexColor) {
                if (entry.instance) {
                    dest->push_back(entry.instance);
                }
            }
            if (mDefaultMaterialInstance.instance) {
                dest->push_back(mDefaultMaterialInstance.instance);
            }
            if (mDefaultMaterialInstanceWithVertexColor.instance) {
                dest->push_back(mDefaultMaterialInstanceWithVertexColor.instance);
            }
        }

        Entry* getEntry(const cgltf_material** mat, bool vertexColor) {
            if (*mat) {
                EntryVector& entries = vertexColor ?
                    mMaterialInstancesWithVertexColor : mMaterialInstances;
                const cgltf_material* basePointer = mHierarchy->materials;
                return &entries[*mat - basePointer];
            }
            *mat = &kDefaultMat;
            return vertexColor ? &mDefaultMaterialInstanceWithVertexColor : &mDefaultMaterialInstance;
        }

    private:
        using EntryVector = utils::FixedCapacityVector<Entry>;
        const cgltf_data* mHierarchy = {};
        EntryVector mMaterialInstances;
        EntryVector mMaterialInstancesWithVertexColor;
        Entry mDefaultMaterialInstance = {};
        Entry mDefaultMaterialInstanceWithVertexColor = {};
    };

    struct VzAssetLoader : public AssetLoader {
        VzAssetLoader(AssetConfiguration const& config) :
            mEntityManager(config.entities ? *config.entities : EntityManager::get()),
            mRenderableManager(config.engine->getRenderableManager()),
            mNameManager((vzm::VzNameCompManager*)config.names),
            mTransformManager(config.engine->getTransformManager()),
            mMaterials(*config.materials),
            mEngine(*config.engine),
            mDefaultNodeName(config.defaultNodeName) {
            if (config.ext) {
                FILAMENT_CHECK_PRECONDITION(AssetConfigurationExtended::isSupported())
                    << "Extend asset loading is not supported on this platform";
                mLoaderExtended = std::make_unique<AssetLoaderExtended>(
                    *config.ext, config.engine, mMaterials);
            }
        }

        FFilamentAsset* createAsset(const uint8_t* bytes, uint32_t nbytes);
        FFilamentAsset* createInstancedAsset(const uint8_t* bytes, uint32_t numBytes,
            FilamentInstance** instances, size_t numInstances);
        FilamentInstance* createInstance(FFilamentAsset* fAsset);

        static void destroy(VzAssetLoader** loader) noexcept {
            delete* loader;
            *loader = nullptr;
        }

        void destroyAsset(const FFilamentAsset* asset) {
            delete asset;
        }

        size_t getMaterialsCount() const noexcept {
            return mMaterials.getMaterialsCount();
        }

        NameComponentManager* getNames() const noexcept {
            return mNameManager;
        }

        NodeManager& getNodeManager() noexcept {
            return mNodeManager;
        }

        const Material* const* getMaterials() const noexcept {
            return mMaterials.getMaterials();
        }

    private:
        void importSkins(FFilamentInstance* instance, const cgltf_data* srcAsset);

        // Methods used during the first traveral (creation of VertexBuffer, IndexBuffer, etc)
        FFilamentAsset* createRootAsset(const cgltf_data* srcAsset);
        void recursePrimitives(const cgltf_node* rootNode, FFilamentAsset* fAsset);
        void createPrimitives(const cgltf_node* node, const char* name, FFilamentAsset* fAsset);
        bool createPrimitive(const cgltf_primitive& inPrim, const char* name, Primitive* outPrim,
            FFilamentAsset* fAsset);

        // Methods used during subsequent traverals (creation of entities, renderables, etc)
        void createInstances(size_t numInstances, FFilamentAsset* fAsset);
        void recurseEntities(const cgltf_node* node, SceneMask scenes, Entity parent,
            FFilamentAsset* fAsset, FFilamentInstance* instance);
        void createRenderable(const cgltf_node* node, Entity entity, const char* name,
            FFilamentAsset* fAsset);
        void createLight(const cgltf_light* light, Entity entity, FFilamentAsset* fAsset);
        void createCamera(const cgltf_camera* camera, Entity entity, FFilamentAsset* fAsset);
        void addTextureBinding(MaterialInstance* materialInstance, const char* parameterName,
            const cgltf_texture* srcTexture, bool srgb);
        void createMaterialVariants(const cgltf_mesh* mesh, Entity entity, FFilamentAsset* fAsset,
            FFilamentInstance* instance);

        // Utility methods that work with MaterialProvider.
        Material* getMaterial(const cgltf_data* srcAsset, const cgltf_material* inputMat, UvMap* uvmap,
            bool vertexColor);
        MaterialInstance* createMaterialInstance(const cgltf_material* inputMat, UvMap* uvmap,
            bool vertexColor, FFilamentAsset* fAsset);
        MaterialKey getMaterialKey(const cgltf_data* srcAsset,
            const cgltf_material* inputMat, UvMap* uvmap, bool vertexColor,
            cgltf_texture_view* baseColorTexture,
            cgltf_texture_view* metallicRoughnessTexture) const;

    public:
        EntityManager& mEntityManager;
        RenderableManager& mRenderableManager;
        vzm::VzNameCompManager* const mNameManager;
        TransformManager& mTransformManager;
        MaterialProvider& mMaterials;
        Engine& mEngine;
        FNodeManager mNodeManager;
        FTrsTransformManager mTrsTransformManager;

        // Transient state used only for the asset currently being loaded:
        const char* mDefaultNodeName;
        bool mError = false;
        bool mDiagnosticsEnabled = false;
        MaterialInstanceCache mMaterialInstanceCache;

        // Weak reference to the largest dummy buffer so far in the current loading phase.
        BufferObject* mDummyBufferObject = nullptr;

        std::unordered_map<const cgltf_mesh*, GeometryVID> mGeometryMap;
        std::unordered_map<const Material*, MaterialVID> mMaterialMap;
        std::unordered_map<const MaterialInstance*, MInstanceVID> mMIMap;
        std::unordered_map<VID, std::string> mLightMap;
        std::unordered_map<VID, std::string> mCameraMap;
        std::unordered_map<VID, std::string> mRenderableActorMap;
        std::unordered_map<VID, std::string> mNodeActorMap;
        std::unordered_map<VID, std::string> mSkeltonRootMap;

    public:
        std::unique_ptr<AssetLoaderExtended> mLoaderExtended;
    };
}
