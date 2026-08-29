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

#include "downcast.h"
#include "FFilamentAsset.h"
#include "FNodeManager.h"
#include "FTrsTransformManager.h"
#include "GltfEnums.h"
#include "Utility.h"

#include "extended/AssetLoaderExtended.h"

#include <gltfio/Animator.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/math.h>

#include <filament/Box.h>
#include <filament/BufferObject.h>
#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MorphTargetBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>

#include <private/utils/Tracing.h>

#include <utils/compiler.h>
#include <utils/EntityManager.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>
#include <utils/NameComponentManager.h>
#include <utils/Panic.h>

#include <math/mat4.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <cgltf.h>
#include <tsl/robin_map.h>

#include <codecvt>
#include <locale>
#include <memory>

using namespace filament;
using namespace filament::math;
using namespace utils;

namespace filament::gltfio {

namespace {

using SceneMask = NodeManager::SceneMask;

const auto FREE_CALLBACK = [](void* mem, size_t, void*) { free(mem); };

// The default glTF material.
constexpr cgltf_material kDefaultMat = {
    .name = (char*) "Default GLTF material",
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

std::string getNodeName(cgltf_node const* node, char const* defaultNodeName) {
    auto const getNameImpl = [node, defaultNodeName]() -> char const* {
        if (node->name) return node->name;
        if (node->mesh && node->mesh->name) return node->mesh->name;
        if (node->light && node->light->name) return node->light->name;
        if (node->camera && node->camera->name) return node->camera->name;
        if (defaultNodeName) return defaultNodeName;
        return "<unknown>";
    };

    std::string strOrig(getNameImpl());

    // We handle the potential case of escaped characters in the JSON which should be properly
    // interpreted as unicode. See spec:
    // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#json-encoding
    // Also see spec for escaped strings in JSON (Section 2.5) https://www.ietf.org/rfc/rfc4627.txt

    std::string strEscaped;
    size_t cur = 0, idx = 0;
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;

    auto const addUnencodedSubstr = [&](size_t const cursor, size_t const nextPoint) {
        assert_invariant(nextPoint >= cursor);
        if (cursor == nextPoint) {
            return;
        }
        strEscaped += strOrig.substr(cursor, nextPoint - cursor);
    };

    while ((idx = strOrig.find("\\u", cur)) != std::string::npos) {
        if (idx + 6 > strOrig.length()) {
            slog.w << "gltfio: Unable to interpret node name=" << strOrig
                          << " as proper unicode encoding." << io::endl;
            return strOrig;
        }

        // Turns string of the form \u0062 to 0x0062
        std::string const hexStr = strOrig.substr(idx + 2, 4);
        if (hexStr.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) {
            slog.w << "gltfio: Unable to interpret node name=" << strOrig
                          << " as proper unicode encoding." << io::endl;
            return strOrig;
        }

        addUnencodedSubstr(cur, idx);

        strEscaped += conv.to_bytes((char32_t) std::stoul(hexStr, nullptr, 16));
        cur = idx + 6;
    }
    addUnencodedSubstr(cur, strOrig.length());

    return strEscaped;
}

bool primitiveHasVertexColor(const cgltf_primitive& inPrim) {
    for (int slot = 0; slot < inPrim.attributes_count; slot++) {
        const cgltf_attribute& inputAttribute = inPrim.attributes[slot];
        if (inputAttribute.type == cgltf_attribute_type_color) {
            return true;
        }
    }
    return false;
}

LightManager::Type getLightType(const cgltf_light_type light) {
    switch (light) {
        case cgltf_light_type_max_enum:
        case cgltf_light_type_invalid:
            assert_invariant(false && "Invalid light type");
            return LightManager::Type::DIRECTIONAL;
        case cgltf_light_type_directional:
            return LightManager::Type::DIRECTIONAL;
        case cgltf_light_type_point:
            return LightManager::Type::POINT;
        case cgltf_light_type_spot:
            return LightManager::Type::FOCUSED_SPOT;
    }
    assert_invariant(false && "Invalid light type");
    return LightManager::Type::DIRECTIONAL;
}

// MaterialInstanceCache
// ---------------------
// Each glTF material definition corresponds to a single MaterialInstance, which are temporarily
// cached when loading a FilamentInstance. If a given glTF material is referenced by multiple
// glTF meshes, then their corresponding Filament primitives will share the same Filament
// MaterialInstance and UvMap. The UvMap is a mapping from each texcoord slot in glTF to one of
// Filament's 2 texcoord sets.
//
// Notes:
// - The Material objects (used to create instances) are cached in MaterialProvider, not here.
// - The cache is not responsible for destroying material instances.
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

    void flush(FixedCapacityVector<MaterialInstance*>* dest) {
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

    Entry* getEntry(const cgltf_material** mat, bool const vertexColor) {
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
    using EntryVector = FixedCapacityVector<Entry>;
    const cgltf_data* mHierarchy = {};
    EntryVector mMaterialInstances;
    EntryVector mMaterialInstancesWithVertexColor;
    Entry mDefaultMaterialInstance = {};
    Entry mDefaultMaterialInstanceWithVertexColor = {};
};

struct FAssetLoader : AssetLoader {
    explicit FAssetLoader(AssetConfiguration const& config) :
            mEntityManager(config.entities ? *config.entities : EntityManager::get()),
            mRenderableManager(config.engine->getRenderableManager()),
            mNameManager(config.names),
            mTransformManager(config.engine->getTransformManager()),
            mMaterials(*config.materials),
            mEngine(*config.engine),
            mNodeManager(mEntityManager),
            mTrsTransformManager(mEntityManager),
            mDefaultNodeName(config.defaultNodeName) {
        if (config.ext) {
            FILAMENT_CHECK_POSTCONDITION(AssetConfigurationExtended::isSupported())
                    << "Extend asset loading is not supported on this platform";
            mLoaderExtended = std::make_unique<AssetLoaderExtended>(
                    *config.ext, config.engine, mMaterials);
        }
    }

    FFilamentAsset* createAsset(const uint8_t* bytes, uint32_t byteCount);
    FFilamentAsset* createInstancedAsset(const uint8_t* bytes, uint32_t byteCount,
            FilamentInstance** instances, size_t numInstances);
    FilamentInstance* createInstance(FFilamentAsset* fAsset);

    static void destroy(FAssetLoader** loader) noexcept {
        delete *loader;
        *loader = nullptr;
    }

    static void destroyAsset(const FFilamentAsset* asset) {
        delete asset;
    }

    void gc() noexcept {
        mNodeManager.gc();
        mTrsTransformManager.gc();
        if (mNameManager) {
            mNameManager->gc();
        }
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
    static void importSkins(FFilamentInstance* instance, const cgltf_data* srcAsset);

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
    void createLight(const cgltf_light* light, Entity entity, FFilamentAsset* fAsset) const;
    void createCamera(const cgltf_camera* camera, Entity entity, FFilamentAsset* fAsset) const;
    void createMaterialVariants(const cgltf_mesh* mesh, Entity entity, FFilamentAsset* fAsset,
            FFilamentInstance* instance);

    // Utility methods that work with MaterialProvider.
    Material* getMaterial(const cgltf_material* inputMat, UvMap* uvmap, bool vertexColor) const;
    MaterialInstance* createMaterialInstance(const cgltf_material* inputMat, UvMap* uvmap,
            bool vertexColor, FFilamentAsset* fAsset);
    MaterialKey getMaterialKey(const cgltf_material* inputMat, bool vertexColor,
            cgltf_texture_view* baseColorTexture,
            cgltf_texture_view* metallicRoughnessTexture) const;

    FFilamentAsset* preresolveTextures(FFilamentAsset* fAsset, const cgltf_data* srcAsset) const;

public:
    EntityManager& mEntityManager;
    RenderableManager& mRenderableManager;
    NameComponentManager* const mNameManager;
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

    std::unique_ptr<AssetLoaderExtended> mLoaderExtended;
};

} // anonymous

FILAMENT_DOWNCAST(AssetLoader)

FFilamentAsset* FAssetLoader::createAsset(const uint8_t* bytes, uint32_t const byteCount) {
    FilamentInstance* instances;
    return createInstancedAsset(bytes, byteCount, &instances, 1);
}

FFilamentAsset* FAssetLoader::createInstancedAsset(const uint8_t* bytes, uint32_t const byteCount,
        FilamentInstance** instances, size_t const numInstances) {
    // This method can be used to load JSON or GLB. By using a default options struct, we are asking
    // cgltf to examine the magic identifier to determine which type of file is being loaded.
    cgltf_options options {};

    if constexpr (!GLTFIO_USE_FILESYSTEM) {
        // Provide a custom free callback for each buffer that was loaded from a "file", as opposed
        // to a data:// URL.
        //
        // Since GLTFIO_USE_FILESYSTEM is false, ResourceLoader requires the app provide the file
        // content from outside, so we need to do nothing here, as opposed to the default, which is
        // to call "free".
        //
        // This callback also gets called for the root-level file_data, but since we use
        // `cgltf_parse`, the file_data field is always null.
        options.file.release = [](const cgltf_memory_options*, const cgltf_file_options*, void*) {};
    }

    // Clients can free up their source blob immediately, but cgltf has pointers into the data that
    // need to stay valid. Therefore we create a copy of the source blob and stash it inside the
    // asset.
    FixedCapacityVector<uint8_t> glbdata(byteCount);
    std::copy_n(bytes, byteCount, glbdata.data());

    // The ownership of an allocated `sourceAsset` will be moved to FFilamentAsset::mSourceAsset.
    cgltf_data* sourceAsset = nullptr;
    cgltf_result const result = cgltf_parse(&options, glbdata.data(), byteCount, &sourceAsset);
    if (result != cgltf_result_success) {
        slog.e << "Unable to parse glTF file." << io::endl;
        return nullptr;
    }

    FFilamentAsset* fAsset = createRootAsset(sourceAsset);
    if (mError) {
        delete fAsset;
        fAsset = nullptr;
        mError = false;
        return nullptr;
    }
    glbdata.swap(fAsset->mSourceAsset->glbData);

    createInstances(numInstances, fAsset);
    if (mError) {
        delete fAsset;
        fAsset = nullptr;
        mError = false;
        return nullptr;
    }

    std::copy_n(fAsset->mInstances.data(), numInstances, instances);
    return fAsset;
}

FilamentInstance* FAssetLoader::createInstance(FFilamentAsset* fAsset) {
    if (!fAsset->mSourceAsset) {
        slog.e << "Source data has been released; asset is frozen." << io::endl;
        return nullptr;
    }
    const cgltf_data* srcAsset = fAsset->mSourceAsset->hierarchy;
    if (srcAsset->scenes == nullptr) {
        slog.e << "There is no scene in the asset." << io::endl;
        return nullptr;
    }

    auto rootTransform = mTransformManager.getInstance(fAsset->mRoot);
    Entity instanceRoot = mEntityManager.create();
    mTransformManager.create(instanceRoot, rootTransform);

    mMaterialInstanceCache = MaterialInstanceCache(srcAsset);

    // Create an instance object, which is a just a lightweight wrapper around a vector of
    // entities and an animator. The creation of animator is triggered from ResourceLoader
    // because it could require external bin data.
    FFilamentInstance* instance = new FFilamentInstance(instanceRoot, fAsset);

    // Check if the asset has variants.
    instance->mVariants.reserve(srcAsset->variants_count);
    for (cgltf_size i = 0, len = srcAsset->variants_count; i < len; ++i) {
        instance->mVariants.push_back({ .name = CString(srcAsset->variants[i].name) });
    }

    // For each scene root, recursively create all entities.
    for (const auto& pair : fAsset->mRootNodes) {
        recurseEntities(pair.first, pair.second, instanceRoot, fAsset, instance);
    }

    importSkins(instance, srcAsset);

    // Now that all entities have been created, the instance can create the animator component.
    // Note that it may need to defer actual creation until external buffers are fully loaded.
    instance->createAnimator();

    fAsset->mInstances.push_back(instance);

    // Bounding boxes are not shared because users might call recomputeBoundingBoxes() which can
    // be affected by entity transforms. However, upon instance creation we can safely copy over
    // the asset's bounding box.
    instance->mBoundingBox = fAsset->mBoundingBox;

    mMaterialInstanceCache.flush(&instance->mMaterialInstances);

    fAsset->mDependencyGraph.commitEdges();

    return instance;
}

FFilamentAsset* FAssetLoader::createRootAsset(const cgltf_data* srcAsset) {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_GLTFIO);
    #if !GLTFIO_DRACO_SUPPORTED
    for (cgltf_size i = 0; i < srcAsset->extensions_required_count; i++) {
        if (!strcmp(srcAsset->extensions_required[i], "KHR_draco_mesh_compression")) {
            slog.e << "KHR_draco_mesh_compression is not supported." << io::endl;
            return nullptr;
        }
    }
    #endif

    mDummyBufferObject = nullptr;
    FFilamentAsset* fAsset = new FFilamentAsset(&mEngine, mNameManager, &mEntityManager,
            &mNodeManager, &mTrsTransformManager, srcAsset, static_cast<bool>(mLoaderExtended));

    // It is not an error for a glTF file to have zero scenes.
    fAsset->mScenes.clear();
    if (srcAsset->scenes == nullptr) {
        return fAsset;
    }

    // Create a single root node with an identity transform as a convenience to the client.
    fAsset->mRoot = mEntityManager.create();
    mTransformManager.create(fAsset->mRoot);

    // Check if the asset has an extras string.
    const cgltf_asset& asset = srcAsset->asset;
    const cgltf_size extras_size = asset.extras.end_offset - asset.extras.start_offset;
    if (extras_size > 1) {
        fAsset->mAssetExtras = CString(srcAsset->json + asset.extras.start_offset, extras_size);
    }

    // Build a mapping of root nodes to scene membership sets.
    assert_invariant(srcAsset->scenes_count <= NodeManager::MAX_SCENE_COUNT);
    fAsset->mRootNodes.clear();
    const size_t sic = std::min(srcAsset->scenes_count, NodeManager::MAX_SCENE_COUNT);
    fAsset->mScenes.reserve(sic);
    for (size_t si = 0; si < sic; ++si) {
        const cgltf_scene& scene = srcAsset->scenes[si];
        fAsset->mScenes.emplace_back(scene.name);
        for (size_t ni = 0, nic = scene.nodes_count; ni < nic; ++ni) {
            fAsset->mRootNodes[scene.nodes[ni]].set(si);
        }
    }

    // Some exporters (e.g. Cinema4D) produce assets with a separate animation hierarchy and
    // modeling hierarchy, where nodes in the former have no associated scene. We need to create
    // transformable entities for "un-scened" nodes in case they have bones.
    for (size_t i = 0, n = srcAsset->nodes_count; i < n; ++i) {
        cgltf_node* node = &srcAsset->nodes[i];
        if (node->parent == nullptr && fAsset->mRootNodes.find(node) == fAsset->mRootNodes.end()) {
            fAsset->mRootNodes.insert({node, {}});
        }
    }

    for (const auto& [node, sceneMask] : fAsset->mRootNodes) {
        recursePrimitives(node, fAsset);
    }

    // Find every unique resource URI and store a pointer to any of the cgltf-owned cstrings
    // that match the URI. These strings get freed during releaseSourceData().
    tsl::robin_set<std::string_view> resourceUris;
    auto addResourceUri = [&resourceUris](const char* uri) {
        if (uri) {
            resourceUris.insert(uri);
        }
    };
    for (cgltf_size i = 0, len = srcAsset->buffers_count; i < len; ++i) {
        addResourceUri(srcAsset->buffers[i].uri);
    }
    for (cgltf_size i = 0, len = srcAsset->images_count; i < len; ++i) {
        addResourceUri(srcAsset->images[i].uri);
    }
    fAsset->mResourceUris.reserve(resourceUris.size());
    for (std::string_view uri : resourceUris) {
        fAsset->mResourceUris.push_back(uri.data());
    }

    return preresolveTextures(fAsset, srcAsset);
}

void FAssetLoader::recursePrimitives(const cgltf_node* rootNode, FFilamentAsset* fAsset) {
    auto const nameStr = getNodeName(rootNode, mDefaultNodeName);
    const char* name = nameStr.c_str();
    name = name ? name : "node";

    if (rootNode->mesh) {
        createPrimitives(rootNode, name, fAsset);
        fAsset->mRenderableCount++;
    }

    for (cgltf_size i = 0, len = rootNode->children_count; i < len; ++i) {
        recursePrimitives(rootNode->children[i], fAsset);
    }
}

void FAssetLoader::createInstances(size_t const numInstances, FFilamentAsset* fAsset) {
    // Create a separate entity hierarchy for each instance. Note that MeshCache (vertex
    // buffers and index buffers) and MaterialInstanceCache (materials and textures) help avoid
    // needless duplication of resources.
    for (size_t index = 0; index < numInstances; ++index) {
        if (createInstance(fAsset) == nullptr) {
            mError = true;
            break;
        }
    }

    // Sort the entities so that the renderable ones come first. This allows us to expose
    // a "renderables only" pointer without storing a separate list.
    const auto& rm = mEngine.getRenderableManager();
    std::ranges::partition(fAsset->mEntities, [&rm](Entity const a) {
        return rm.hasComponent(a);
    });
}

void FAssetLoader::recurseEntities(const cgltf_node* node, SceneMask const scenes,
        Entity const parent, FFilamentAsset* fAsset, FFilamentInstance* instance) {
    NodeManager& nm = mNodeManager;
    const cgltf_data* srcAsset = fAsset->mSourceAsset->hierarchy;
    const Entity entity = mEntityManager.create();
    nm.create(entity);
    const auto nodeInstance = nm.getInstance(entity);
    nm.setSceneMembership(nodeInstance, scenes);

    // Always create a transform component to reflect the original hierarchy.
    mat4f localTransform;
    if (node->has_matrix) {
        memcpy(&localTransform[0][0], &node->matrix[0], 16 * sizeof(float));
    } else {
        quatf const* rotation = reinterpret_cast<quatf const*>(&node->rotation[0]);
        float3 const* scale = reinterpret_cast<float3 const*>(&node->scale[0]);
        float3 const* translation = reinterpret_cast<float3 const*>(&node->translation[0]);
        mTrsTransformManager.create(entity, *translation, *rotation, *scale);
        localTransform = mTrsTransformManager.getTransform(
                mTrsTransformManager.getInstance(entity));
    }

    auto const parentTransform = mTransformManager.getInstance(parent);
    mTransformManager.create(entity, parentTransform, localTransform);

    // Check if this node has an extras string.
    const cgltf_size extras_size = node->extras.end_offset - node->extras.start_offset;
    if (extras_size > 0) {
        mNodeManager.setExtras(mNodeManager.getInstance(entity),
                {srcAsset->json + node->extras.start_offset, extras_size});
    }

    // Update the asset's entity list and private node mapping.
    fAsset->mEntities.push_back(entity);
    instance->mEntities.push_back(entity);
    instance->mNodeMap[node - srcAsset->nodes] = entity;

    auto const nameStr = getNodeName(node, mDefaultNodeName);
    const char* name = nameStr.c_str();

    if (name) {
        fAsset->mNameToEntity[name].push_back(entity);
        if (mNameManager) {
            mNameManager->addComponent(entity);
            mNameManager->setName(mNameManager->getInstance(entity), name);
        }
    }

    // If no name is provided in the glTF or AssetConfiguration, use "node" for error messages.
    name = name ? name : "node";

    // If the node has a mesh, then create a renderable component.
    if (node->mesh) {
        createRenderable(node, entity, name, fAsset);
        if (srcAsset->variants_count > 0) {
            createMaterialVariants(node->mesh, entity, fAsset, instance);
        }
    }

    if (node->light) {
        createLight(node->light, entity, fAsset);
    }

    if (node->camera) {
        createCamera(node->camera, entity, fAsset);
    }

    for (cgltf_size i = 0, len = node->children_count; i < len; ++i) {
        recurseEntities(node->children[i], scenes, entity, fAsset, instance);
    }
}

void FAssetLoader::createPrimitives(const cgltf_node* node, const char* name,
        FFilamentAsset* fAsset) {
    cgltf_data* gltf = fAsset->mSourceAsset->hierarchy;
    const cgltf_mesh* mesh = node->mesh;
    assert_invariant(gltf != nullptr);
    assert_invariant(mesh != nullptr);

    // If the mesh is already loaded, obtain the list of Filament VertexBuffer / IndexBuffer objects
    // that were already generated (one for each primitive), otherwise allocate a new list of
    // pointers for the primitives.
    FixedCapacityVector<Primitive>& prims = fAsset->mMeshCache[mesh - gltf->meshes];
    if (prims.empty()) {
        prims.reserve(mesh->primitives_count);
        prims.resize(mesh->primitives_count);
    }

    Aabb aabb;

    for (cgltf_size index = 0, n = mesh->primitives_count; index < n; ++index) {
        Primitive& outputPrim = prims[index];
        cgltf_primitive& inputPrim = mesh->primitives[index];

        if (!outputPrim.vertices) {
            if (mLoaderExtended) {
                auto& resourceInfo = std::get<FFilamentAsset::ResourceInfoExtended>(fAsset->mResourceInfo);
                resourceInfo.uriDataCache = mLoaderExtended->getUriDataCache();
                AssetLoaderExtended::Input input{
                        .gltf = gltf,
                        .prim = &inputPrim,
                        .name = name,
                        .dracoCache = &fAsset->mSourceAsset->dracoCache,
                        .material = getMaterial(inputPrim.material, &outputPrim.uvmap,
                            utility::primitiveHasVertexColor(&inputPrim)),
                };

                mError = !mLoaderExtended->createPrimitive(&input, &outputPrim, resourceInfo.slots);
                if (!mError) {
                    if (outputPrim.vertices) {
                        fAsset->mVertexBuffers.push_back(outputPrim.vertices);
                    }
                    if (outputPrim.indices) {
                        fAsset->mIndexBuffers.push_back(outputPrim.indices);
                    }
                }
            } else {
                // Create a Filament VertexBuffer and IndexBuffer for this prim if we haven't
                // already.
                mError = !createPrimitive(inputPrim, name, &outputPrim, fAsset);
            }
            if (mError) {
                return;
            }
        }

        // Expand the object-space bounding box.
        aabb.min = min(outputPrim.aabb.min, aabb.min);
        aabb.max = max(outputPrim.aabb.max, aabb.max);
    }

    mat4f worldTransform;
    cgltf_node_transform_world(node, &worldTransform[0][0]);

    const Aabb transformed = aabb.transform(worldTransform);
    fAsset->mBoundingBox.min = min(fAsset->mBoundingBox.min, transformed.min);
    fAsset->mBoundingBox.max = max(fAsset->mBoundingBox.max, transformed.max);
 }

void FAssetLoader::createRenderable(const cgltf_node* node, Entity const entity, const char* name,
        FFilamentAsset* fAsset) {
    const cgltf_data* srcAsset = fAsset->mSourceAsset->hierarchy;
    const cgltf_mesh* mesh = node->mesh;
    const cgltf_size primitiveCount = mesh->primitives_count;

    // If the mesh is already loaded, obtain the list of Filament VertexBuffer / IndexBuffer objects
    // that were already generated (one for each primitive).
    FixedCapacityVector<Primitive>& prims = fAsset->mMeshCache[mesh - srcAsset->meshes];
    assert_invariant(prims.size() == primitiveCount);
    Primitive* outputPrim = prims.data();
    const cgltf_primitive* inputPrim = &mesh->primitives[0];

    Aabb aabb;

    // glTF spec says that all primitives must have the same number of morph targets.
    const cgltf_size numMorphTargets = inputPrim ? inputPrim->targets_count : 0;
    RenderableManager::Builder builder(primitiveCount);

    // For each prim, create a Filament VertexBuffer, IndexBuffer, and MaterialInstance.
    // The VertexBuffer and IndexBuffer objects are cached for possible re-use, but MaterialInstance
    // is not.
    size_t morphingVertexCount = 0;
    for (cgltf_size index = 0; index < primitiveCount; ++index, ++outputPrim, ++inputPrim) {
        RenderableManager::PrimitiveType primType;
        if (!getPrimitiveType(inputPrim->type, &primType)) {
            slog.e << "Unsupported primitive type in " << name << io::endl;
        }

        if (numMorphTargets != inputPrim->targets_count) {
            slog.e << "Sister primitives must all have the same number of morph targets."
                   << io::endl;
            mError = true;
            continue;
        }

        // Create a material instance for this primitive or fetch one from the cache.
        UvMap uvmap {};
        bool const hasVertexColor = primitiveHasVertexColor(*inputPrim);
        MaterialInstance* mi = createMaterialInstance(inputPrim->material, &uvmap, hasVertexColor,
                fAsset);
        assert_invariant(mi);
        if (!mi) {
            mError = true;
            continue;
        }

        fAsset->mDependencyGraph.addEdge(entity, mi);
        builder.material(index, mi);

        assert_invariant(outputPrim->vertices);

        // Expand the object-space bounding box.
        aabb.min = min(outputPrim->aabb.min, aabb.min);
        aabb.max = max(outputPrim->aabb.max, aabb.max);

        // We are not using the optional offset, minIndex, maxIndex, and count arguments when
        // calling geometry() on the builder. It appears that the glTF spec does not have
        // facilities for these parameters, which is not a huge loss since some of the buffer
        // view and accessor features already have this functionality.
        builder.geometry(index, primType, outputPrim->vertices, outputPrim->indices);

        if (numMorphTargets) {
            outputPrim->morphTargetOffset = morphingVertexCount;    // FIXME: can I do that here?
            builder.morphing(0, index, morphingVertexCount);
            morphingVertexCount += outputPrim->vertices->getVertexCount();
        }
    }

    if (numMorphTargets) {
        MorphTargetBuffer* morphTargetBuffer = MorphTargetBuffer::Builder()
                .count(numMorphTargets)
                .vertexCount(morphingVertexCount)
                .build(mEngine);

        fAsset->mMorphTargetBuffers.push_back(morphTargetBuffer);

        builder.morphing(morphTargetBuffer);

        // The Primitive structs in mMeshCache are shared across all instances of the same
        // mesh. Only the first call should populate morphTargetBuffer in the shared Primitive
        // and write into the existing BufferSlots (which were allocated by createPrimitive).
        // Subsequent instances must append NEW BufferSlots so that ResourceLoader uploads
        // morph data to every instance's MorphTargetBuffer independently.
        //
        // NOTE: This fix covers morph position data only. Morph tangent recomputation in
        // ResourceLoader::computeTangents() reads MorphTargetBuffer from the shared Primitive,
        // so additional instances with lit morph targets may have incorrect tangent frames.
        const bool isFirstMorphSetup = (prims.data()->morphTargetBuffer == nullptr);

        outputPrim = prims.data();
        inputPrim = &mesh->primitives[0];
        for (cgltf_size index = 0; index < primitiveCount; ++index, ++outputPrim, ++inputPrim) {
            if (isFirstMorphSetup) {
                outputPrim->morphTargetBuffer = morphTargetBuffer;
            }

            UTILS_UNUSED_IN_RELEASE cgltf_accessor const* previous = nullptr;
            // createPrimitives() caps the per-primitive morph-target count at MAX_MORPH_TARGETS and
            // sizes slotIndices accordingly; iterate only over the slots that exist so a mesh with
            // more morph targets than the cap does not index slotIndices out of bounds.
            const size_t numSlots = outputPrim->slotIndices.size();
            for (int tindex = 0; tindex < numMorphTargets && static_cast<size_t>(tindex) < numSlots; ++tindex) {
                const cgltf_morph_target& inTarget = inputPrim->targets[tindex];
                for (cgltf_size aindex = 0; aindex < inTarget.attributes_count; ++aindex) {
                    const cgltf_attribute& attribute = inTarget.attributes[aindex];
                    const cgltf_accessor* accessor = attribute.data;
                    const cgltf_attribute_type atype = attribute.type;
                    if (atype == cgltf_attribute_type_position) {
                        // All position attributes must have the same number of components.
                        assert_invariant(!previous || previous->type == accessor->type);
                        previous = accessor;

                        if (std::holds_alternative<FFilamentAsset::ResourceInfo>(
                                fAsset->mResourceInfo)) {
                            using BufferSlot = FFilamentAsset::ResourceInfo::BufferSlot;
                            auto& slots = std::get<FFilamentAsset::ResourceInfo>(
                                    fAsset->mResourceInfo).mBufferSlots;

                            if (isFirstMorphSetup) {
                                // First instance: write into the pre-allocated slot.
                                BufferSlot& slot = slots[outputPrim->slotIndices[tindex]];

                                assert_invariant(!slot.vertexBuffer);
                                assert_invariant(!slot.indexBuffer);

                                slot.morphTargetBuffer = morphTargetBuffer;
                                slot.morphTargetOffset = outputPrim->morphTargetOffset;
                                slot.morphTargetCount = outputPrim->vertices->getVertexCount();
                                slot.bufferIndex = tindex;
                            } else {
                                // Additional instances: append a new slot so that
                                // ResourceLoader uploads morph data to this instance's
                                // MorphTargetBuffer without overwriting the original.
                                BufferSlot newSlot = {};
                                newSlot.accessor = accessor;
                                newSlot.morphTargetBuffer = morphTargetBuffer;
                                newSlot.morphTargetOffset = outputPrim->morphTargetOffset;
                                newSlot.morphTargetCount = outputPrim->vertices->getVertexCount();
                                newSlot.bufferIndex = tindex;
                                slots.push_back(newSlot);
                            }
                        }
                        // NOTE: The ResourceInfoExtended path is not handled here for
                        // additional instances because it requires CPU-side geometry
                        // processing (targetData.positions, targetData.tbn) that is only
                        // performed in AssetLoaderExtended::createPrimitive(). The extended
                        // path is currently limited to desktop platforms and does not support
                        // multi-instance morph targets.
                        else if (std::holds_alternative<FFilamentAsset::ResourceInfoExtended>(
                                fAsset->mResourceInfo))
                        {
                            using BufferSlot = FFilamentAsset::ResourceInfoExtended::BufferSlot;
                            auto& slots = std::get<FFilamentAsset::ResourceInfoExtended>(
                                    fAsset->mResourceInfo).slots;

                            if (isFirstMorphSetup) {
                                BufferSlot& slot = slots[outputPrim->slotIndices[tindex]];

                                assert_invariant(slot.slot == tindex);
                                assert_invariant(!slot.vertices);
                                assert_invariant(!slot.indices);

                                slot.target = morphTargetBuffer;
                                slot.offset = outputPrim->morphTargetOffset;
                                slot.count = outputPrim->vertices->getVertexCount();
                            }
                            // Additional instances: skip. The extended loader requires
                            // targetData that is not available at createRenderable() time.
                        }

                        break;
                    }
                }
            }
        }
    }

    FixedCapacityVector<CString> morphTargetNames(numMorphTargets);
    // A mesh's morph-target name count (mesh.extras.targetNames) is parsed independently of its
    // morph-target count, and is not constrained when the mesh has no primitives (numMorphTargets
    // is then 0). Bound the copy by the destination capacity so that a mesh declaring more target
    // names than morph targets does not write past morphTargetNames.
    for (cgltf_size i = 0, c = std::min(numMorphTargets, mesh->target_names_count); i < c; ++i) {
        morphTargetNames[i] = CString(mesh->target_names[i]);
    }
    auto& nm = mNodeManager;
    nm.setMorphTargetNames(nm.getInstance(entity), std::move(morphTargetNames));

    if (node->skin) {
        builder.skinning(node->skin->joints_count);
    }

    // Per the spec, glTF models must have valid mix / max annotations for position attributes.
    // If desired, clients can call "recomputeBoundingBoxes()" in FilamentInstance.
    Box box = Box().set(aabb.min, aabb.max);
    if (box.isEmpty()) {
        slog.w << "Missing bounding box in " << name << io::endl;
        box = Box().set(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
    }

    builder
        .boundingBox(box)
        .culling(true)
        .castShadows(true)
        .receiveShadows(true)
        .build(mEngine, entity);

    // According to the spec, the mesh may or may not specify default weights, regardless of whether
    // it actually has morph targets. If it has morphing enabled then the default weights are 0. If
    // node weights are provided, they override the ones specified on the mesh.
    if (numMorphTargets > 0) {
        RenderableManager::Instance const renderable = mRenderableManager.getInstance(entity);
        const auto size = std::min(MAX_MORPH_TARGETS, numMorphTargets);
        FixedCapacityVector weights(size, 0.0f);
        for (cgltf_size i = 0, c = std::min(size, mesh->weights_count); i < c; ++i) {
            weights[i] = mesh->weights[i];
        }
        for (cgltf_size i = 0, c = std::min(size, node->weights_count); i < c; ++i) {
            weights[i] = node->weights[i];
        }
        mRenderableManager.setMorphWeights(renderable, weights.data(), size);
    }
}

void FAssetLoader::createMaterialVariants(const cgltf_mesh* mesh, Entity const entity,
        FFilamentAsset* fAsset, FFilamentInstance* instance) {
    UvMap uvmap {};
    for (cgltf_size prim = 0, n = mesh->primitives_count; prim < n; ++prim) {
        const cgltf_primitive& srcPrim = mesh->primitives[prim];
        for (size_t i = 0, m = srcPrim.mappings_count; i < m; i++) {
            const size_t variantIndex = srcPrim.mappings[i].variant;
            const cgltf_material* material = srcPrim.mappings[i].material;
            const bool hasVertexColor = primitiveHasVertexColor(srcPrim);
            MaterialInstance* mi =
                    createMaterialInstance(material, &uvmap, hasVertexColor, fAsset);
            assert_invariant(mi);
            if (!mi) {
                mError = true;
                break;
            }
            fAsset->mDependencyGraph.addEdge(entity, mi);
            instance->mVariants[variantIndex].mappings.push_back({
                .renderable = entity,
                .primitiveIndex = prim,
                .material = mi
            });
        }
    }
}

bool FAssetLoader::createPrimitive(const cgltf_primitive& inPrim, const char* name,
        Primitive* outPrim, FFilamentAsset* fAsset) {

    using BufferSlot = FFilamentAsset::ResourceInfo::BufferSlot;

    Material* material = getMaterial(inPrim.material,
            &outPrim->uvmap, primitiveHasVertexColor(inPrim));
    AttributeBitset requiredAttributes = material->getRequiredAttributes();

    // TODO: populate a mapping of Texture Index => [MaterialInstance, const char*] slots.
    // By creating this mapping during the "recursePrimitives" phase, we will can allow
    // zero-instance assets to exist. This will be useful for "preloading", which is a feature
    // request from Google.

    // Create a little lambda that appends to the asset's vertex buffer slots.
    auto* const slots = &std::get<FFilamentAsset::ResourceInfo>(fAsset->mResourceInfo).mBufferSlots;
    auto addBufferSlot = [slots](FFilamentAsset::ResourceInfo::BufferSlot const& entry) {
        slots->push_back(entry);
    };

    // In glTF, each primitive may or may not have an index buffer.
    IndexBuffer* indices = nullptr;
    const cgltf_accessor* accessor = inPrim.indices;
    if (accessor) {
        IndexBuffer::IndexType indexType;
        if (!getIndexType(accessor->component_type, &indexType)) {
            slog.e << "Unrecognized index type in " << name << io::endl;
            return false;
        }

        indices = IndexBuffer::Builder()
            .indexCount(accessor->count)
            .bufferType(indexType)
            .build(mEngine);

        FFilamentAsset::ResourceInfo::BufferSlot slot = { .accessor = accessor };
        slot.indexBuffer = indices;
        addBufferSlot(slot);
    } else if (inPrim.attributes_count > 0) {
        // If a primitive does not have an index buffer, generate a trivial one now.
        const uint32_t vertexCount = inPrim.attributes[0].data->count;

        indices = IndexBuffer::Builder()
            .indexCount(vertexCount)
            .bufferType(IndexBuffer::IndexType::UINT)
            .build(mEngine);

        const size_t indexDataSize = vertexCount * sizeof(uint32_t);
        uint32_t* indexData = (uint32_t*) malloc(indexDataSize);
        if (!indexData) {
            slog.e << "Out of memory allocating generated index buffer." << io::endl;
            return false;
        }
        for (size_t i = 0; i < vertexCount; ++i) {
            indexData[i] = i;
        }
        IndexBuffer::BufferDescriptor bd(indexData, indexDataSize, FREE_CALLBACK);
        indices->setBuffer(mEngine, std::move(bd));
    }
    fAsset->mIndexBuffers.push_back(indices);

    VertexBuffer::Builder vbb;
    vbb.enableBufferObjects();

    bool hasUv0 = false, hasUv1 = false, hasVertexColor = false, hasNormals = false;
    int8_t currentCustomIndex = -1;
    uint32_t vertexCount = 0;

    const size_t firstSlot = slots->size();
    int slot = 0;

    for (cgltf_size aindex = 0; aindex < inPrim.attributes_count; aindex++) {
        const cgltf_attribute& attribute = inPrim.attributes[aindex];
        const int index = attribute.index;
        const cgltf_attribute_type atype = attribute.type;
        const cgltf_accessor* innerAccessor = attribute.data;
        int8_t customIndex = -1;

        // The glTF tangent data is ignored here, but honored in ResourceLoader.
        if (atype == cgltf_attribute_type_tangent) {
            continue;
        }

        // At a minimum, surface orientation requires normals to be present in the source data.
        // Here we re-purpose the normals slot to point to the quats that get computed later.
        if (atype == cgltf_attribute_type_normal) {
            vbb.attribute(TANGENTS, slot, VertexBuffer::AttributeType::SHORT4);
            vbb.normalized(TANGENTS);
            hasNormals = true;
            addBufferSlot({
                .accessor = &fAsset->mGenerateTangents,
                .attribute = atype,
                .bufferIndex = slot++
            });
            continue;
        }

        if (atype == cgltf_attribute_type_color) {
            if (hasVertexColor) {
                // We already had a vertex color before, we need to store this is as a custom
                // attribute.
                customIndex = ++currentCustomIndex;
            } else {
                hasVertexColor = true;
            }
        }

        // Translate the cgltf attribute enum into a Filament enum.
        VertexAttribute semantic;
        if (!getCustomVertexAttrType(customIndex, &semantic) &&
                !getVertexAttrType(atype, &semantic)) {
            slog.e << "Unrecognized vertex semantic in " << name << io::endl;
            return false;
        }
        if (atype == cgltf_attribute_type_weights && index > 0) {
            slog.e << "Too many bone weights in " << name << io::endl;
            continue;
        }
        if (atype == cgltf_attribute_type_joints && index > 0) {
            slog.e << "Too many joints in " << name << io::endl;
            continue;
        }

        if (atype == cgltf_attribute_type_texcoord) {
            if (index >= UvMapSize) {
                slog.e << "Too many texture coordinate sets in " << name << io::endl;
                continue;
            }
            switch (UvSet uvset = outPrim->uvmap[index]) {
                case UV0:
                    semantic = VertexAttribute::UV0;
                    hasUv0 = true;
                    break;
                case UV1:
                    semantic = VertexAttribute::UV1;
                    hasUv1 = true;
                    break;
                case UNUSED:
                    // If we have a free slot, then include this unused UV set in the VertexBuffer.
                    // This allows clients to swap the glTF material with a custom material.
                    if (!hasUv0 && getNumUvSets(outPrim->uvmap) == 0) {
                        semantic = VertexAttribute::UV0;
                        hasUv0 = true;
                        break;
                    }

                    // If there are no free slots then drop this unused texture coordinate set.
                    // This should not print an error or warning because the glTF spec stipulates an
                    // order of degradation for gracefully dropping UV sets. We implement this in
                    // constrainMaterial in MaterialProvider.
                    continue;
            }
        }

        vertexCount = innerAccessor->count;

        // The positions accessor is required to have min/max properties, use them to expand
        // the bounding box for this primitive.
        if (atype == cgltf_attribute_type_position) {
            const float* minp = &innerAccessor->min[0];
            const float* maxp = &innerAccessor->max[0];
            outPrim->aabb.min = min(outPrim->aabb.min, float3(minp[0], minp[1], minp[2]));
            outPrim->aabb.max = max(outPrim->aabb.max, float3(maxp[0], maxp[1], maxp[2]));
        }

        VertexBuffer::AttributeType fatype;
        VertexBuffer::AttributeType actualType;
        if (!getElementType(innerAccessor->type, innerAccessor->component_type, &fatype, &actualType)) {
            slog.e << "Unsupported accessor type in " << name << io::endl;
            return false;
        }
        const int stride = (fatype == actualType) ? innerAccessor->stride : 0;

        // The cgltf library provides a stride value for all accessors, even though they do not
        // exist in the glTF file. It is computed from the type and the stride of the buffer view.
        // As a convenience, cgltf also replaces zero (default) stride with the actual stride.
        vbb.attribute(semantic, slot, fatype, 0, stride);
        vbb.normalized(semantic, innerAccessor->normalized);
        addBufferSlot({ .accessor = innerAccessor, .attribute = atype, .bufferIndex = slot++ });
    }

    // If the model is lit but does not have normals, we'll need to generate flat normals.
    if (requiredAttributes.test(TANGENTS) && !hasNormals) {
        vbb.attribute(TANGENTS, slot, VertexBuffer::AttributeType::SHORT4);
        vbb.normalized(TANGENTS);
        cgltf_attribute_type atype = cgltf_attribute_type_normal;
        addBufferSlot({
            .accessor = &fAsset->mGenerateNormals,
            .attribute = atype,
            .bufferIndex = slot++
        });
    }

    cgltf_size targetsCount = inPrim.targets_count;

    if (targetsCount > MAX_MORPH_TARGETS) {
        slog.w << "WARNING: Exceeded max morph target count of "
                << MAX_MORPH_TARGETS << io::endl;
        targetsCount = MAX_MORPH_TARGETS;
    }

    const Aabb baseAabb(outPrim->aabb);
    for (cgltf_size targetIndex = 0; targetIndex < targetsCount; targetIndex++) {
        const cgltf_morph_target& morphTarget = inPrim.targets[targetIndex];
        for (cgltf_size aindex = 0; aindex < morphTarget.attributes_count; aindex++) {
            const cgltf_attribute& attribute = morphTarget.attributes[aindex];
            const cgltf_accessor* innerAccessor = attribute.data;
            const cgltf_attribute_type atype = attribute.type;

            // The glTF normal and tangent data are ignored here, but honored in ResourceLoader.
            if (atype == cgltf_attribute_type_normal || atype == cgltf_attribute_type_tangent) {
                continue;
            }

            if (atype != cgltf_attribute_type_position) {
                slog.e << "Only positions, normals, and tangents can be morphed."
                        << io::endl;
                return false;
            }

            if (!innerAccessor->has_min || !innerAccessor->has_max) {
                continue;
            }

            Aabb targetAabb(baseAabb);
            const float* minp = &innerAccessor->min[0];
            const float* maxp = &innerAccessor->max[0];

            // We assume that the range of morph target weight is [0, 1].
            targetAabb.min += float3(minp[0], minp[1], minp[2]);
            targetAabb.max += float3(maxp[0], maxp[1], maxp[2]);

            outPrim->aabb.min = min(outPrim->aabb.min, targetAabb.min);
            outPrim->aabb.max = max(outPrim->aabb.max, targetAabb.max);

            VertexBuffer::AttributeType fatype;
            VertexBuffer::AttributeType actualType;
            if (!getElementType(innerAccessor->type, innerAccessor->component_type, &fatype, &actualType)) {
                slog.e << "Unsupported accessor type in " << name << io::endl;
                return false;
            }
        }
    }

    if (vertexCount == 0) {
        slog.e << "Empty vertex buffer in " << name << io::endl;
        return false;
    }

    vbb.vertexCount(vertexCount);

    // We provide a single dummy buffer (filled with 0xff) for all unfulfilled vertex requirements.
    // The color data should be a sequence of normalized UBYTE4, so dummy UVs are USHORT2 to make
    // the sizes match.
    bool needsDummyData = false;

    if (mMaterials.needsDummyData(VertexAttribute::UV0) && !hasUv0) {
        needsDummyData = true;
        hasUv0 = true;
        vbb.attribute(VertexAttribute::UV0, slot, VertexBuffer::AttributeType::USHORT2);
        vbb.normalized(VertexAttribute::UV0);
    }

    if (mMaterials.needsDummyData(VertexAttribute::UV1) && !hasUv1) {
        hasUv1 = true;
        needsDummyData = true;
        vbb.attribute(VertexAttribute::UV1, slot, VertexBuffer::AttributeType::USHORT2);
        vbb.normalized(VertexAttribute::UV1);
    }

    if (mMaterials.needsDummyData(COLOR) && !hasVertexColor) {
        needsDummyData = true;
        vbb.attribute(COLOR, slot, VertexBuffer::AttributeType::UBYTE4);
        vbb.normalized(COLOR);
    }

    int numUvSets = getNumUvSets(outPrim->uvmap);
    if (!hasUv0 && numUvSets > 0) {
        needsDummyData = true;
        vbb.attribute(VertexAttribute::UV0, slot, VertexBuffer::AttributeType::USHORT2);
        vbb.normalized(VertexAttribute::UV0);
        slog.w << "Missing UV0 data in " << name << io::endl;
    }

    if (!hasUv1 && numUvSets > 1) {
        needsDummyData = true;
        vbb.attribute(VertexAttribute::UV1, slot, VertexBuffer::AttributeType::USHORT2);
        vbb.normalized(VertexAttribute::UV1);
        slog.w << "Missing UV1 data in " << name << io::endl;
    }

    vbb.bufferCount(needsDummyData ? slot + 1 : slot);

    VertexBuffer* vertices = vbb.build(mEngine);

    outPrim->indices = indices;
    outPrim->vertices = vertices;
    auto& primitives = std::get<FFilamentAsset::ResourceInfo>(fAsset->mResourceInfo).mPrimitives;
    primitives.push_back({&inPrim, vertices});
    fAsset->mVertexBuffers.push_back(vertices);

    for (size_t i = firstSlot; i < slots->size(); ++i) {
        (*slots)[i].vertexBuffer = vertices;
    }

    if (targetsCount > 0) {
        UTILS_UNUSED_IN_RELEASE cgltf_accessor const* previous = nullptr;
        outPrim->slotIndices.resize(targetsCount);
        for (int tindex = 0; tindex < targetsCount; ++tindex) {
            const cgltf_morph_target& inTarget = inPrim.targets[tindex];
            for (cgltf_size aindex = 0; aindex < inTarget.attributes_count; ++aindex) {
                const cgltf_attribute& attribute = inTarget.attributes[aindex];
                const cgltf_accessor* innerAccessor = attribute.data;
                const cgltf_attribute_type atype = attribute.type;
                if (atype == cgltf_attribute_type_position) {
                    // All position attributes must have the same number of components.
                    assert_invariant(!previous || previous->type == innerAccessor->type);
                    previous = innerAccessor;
                    BufferSlot innerSlot = { .accessor = innerAccessor };
                    outPrim->slotIndices[tindex] = slots->size();
                    addBufferSlot(innerSlot);
                    break;
                }
            }
        }
    }

    if (needsDummyData) {
        const uint32_t requiredSize = sizeof(ubyte4) * vertexCount;
        if (mDummyBufferObject == nullptr || requiredSize > mDummyBufferObject->getByteCount()) {
            mDummyBufferObject = BufferObject::Builder().size(requiredSize).build(mEngine);
            fAsset->mBufferObjects.push_back(mDummyBufferObject);
            uint32_t* dummyData = static_cast<uint32_t*>(malloc(requiredSize));
            if (!dummyData) {
                slog.e << "Out of memory allocating dummy vertex data." << io::endl;
                return false;
            }
            memset(dummyData, 0xff, requiredSize);
            VertexBuffer::BufferDescriptor bd(dummyData, requiredSize, FREE_CALLBACK);
            mDummyBufferObject->setBuffer(mEngine, std::move(bd));
        }
        vertices->setBufferObjectAt(mEngine, slot, mDummyBufferObject);
    }

    return true;
}

void FAssetLoader::createLight(const cgltf_light* light, Entity const entity,
        FFilamentAsset* fAsset) const {
    LightManager::Type const type = getLightType(light->type);
    LightManager::Builder builder(type);

    builder.direction({0.0f, 0.0f, -1.0f});
    builder.color({light->color[0], light->color[1], light->color[2]});

    switch (type) {
        case LightManager::Type::SUN:
        case LightManager::Type::DIRECTIONAL:
            builder.intensity(light->intensity);
            break;
        case LightManager::Type::POINT:
            builder.intensityCandela(light->intensity);
            break;
        case LightManager::Type::FOCUSED_SPOT:
        case LightManager::Type::SPOT:
            // glTF specifies half angles, so does Filament
            builder.spotLightCone(
                    light->spot_inner_cone_angle,
                    light->spot_outer_cone_angle);
            builder.intensityCandela(light->intensity);
            break;
    }

    if (light->range == 0.0f) {
        // The spec calls for an infinite range in this case, but it's extremely inefficient
        // for our clustered forward rendering system. Instead, attempt to compute a reasonable
        // range that aims fo 0.05 candela at a certain distance.
        //
        // The light formula is:
        //   E = I / distance^2 * saturate(1 - distance^4 / range^4)^2
        //
        // Ignoring the windowing function:
        //   E = I / distance^2
        //
        // Solving for a known intensity:
        //   0.05 = I / distance^2
        //   distance = sqrt(I / 0.05)
        //
        // The resulting range is however way too large so we aggressively tune it down by
        // using ^(1/4) instead of a square root. This still gives good results and compares
        // favorably to hand-picked ranges.
        //
        // Note: this is a best effort guess since we don't take the camera's exposure into
        // account. A target of 0.05 candela is ~0.6 lumen for a point light (4pi steradians),
        // so it's extremely dim.
        float const range = std::pow(light->intensity / 0.05f, 0.25f);
        builder.falloff(range);
    } else {
        builder.falloff(light->range);
    }

    builder.build(mEngine, entity);
    fAsset->mLightEntities.push_back(entity);
}

void FAssetLoader::createCamera(const cgltf_camera* camera, Entity const entity,
        FFilamentAsset* fAsset) const {
    Camera* filamentCamera = mEngine.createCamera(entity);

    if (camera->type == cgltf_camera_type_perspective) {
        auto& projection = camera->data.perspective;

        const cgltf_float yfovDegrees = 180.0 / F_PI * projection.yfov;

        // Use an "infinite" zfar plane if the provided one is missing (set to 0.0).
        const double far = projection.zfar > 0.0 ? projection.zfar : 100000000;

        filamentCamera->setProjection(yfovDegrees, 1.0,
                projection.znear, far,
                Camera::Fov::VERTICAL);

        // Use a default aspect ratio of 1.0 if the provided one is missing.
        const double aspect = projection.aspect_ratio > 0.0 ? projection.aspect_ratio : 1.0;

        // Use the scaling matrix to set the aspect ratio, so clients can easily change it.
        filamentCamera->setScaling({1.0 / aspect, 1.0 });
    } else if (camera->type == cgltf_camera_type_orthographic) {
        auto& projection = camera->data.orthographic;

        const double left   = -projection.xmag * 0.5;
        const double right  =  projection.xmag * 0.5;
        const double bottom = -projection.ymag * 0.5;
        const double top    =  projection.ymag * 0.5;

        filamentCamera->setProjection(Camera::Projection::ORTHO,
                left, right, bottom, top, projection.znear, projection.zfar);
    } else {
        slog.e << "Invalid GLTF camera type." << io::endl;
        return;
    }

    fAsset->mCameraEntities.push_back(entity);
}

MaterialKey FAssetLoader::getMaterialKey(const cgltf_material* inputMat, bool vertexColor,
        cgltf_texture_view* baseColorTexture, cgltf_texture_view* metallicRoughnessTexture) const {
    auto mrConfig = inputMat->pbr_metallic_roughness;
    auto sgConfig = inputMat->pbr_specular_glossiness;
    auto ccConfig = inputMat->clearcoat;
    auto trConfig = inputMat->transmission;
    auto shConfig = inputMat->sheen;
    auto vlConfig = inputMat->volume;
    auto spConfig = inputMat->specular;
    auto irConfig = inputMat->iridescence;
    *baseColorTexture = mrConfig.base_color_texture;
    *metallicRoughnessTexture = mrConfig.metallic_roughness_texture;

    bool hasTextureTransforms =
        sgConfig.diffuse_texture.has_transform ||
        sgConfig.specular_glossiness_texture.has_transform ||
        mrConfig.base_color_texture.has_transform ||
        mrConfig.metallic_roughness_texture.has_transform ||
        inputMat->normal_texture.has_transform ||
        inputMat->occlusion_texture.has_transform ||
        inputMat->emissive_texture.has_transform ||
        ccConfig.clearcoat_texture.has_transform ||
        ccConfig.clearcoat_roughness_texture.has_transform ||
        ccConfig.clearcoat_normal_texture.has_transform ||
        shConfig.sheen_color_texture.has_transform ||
        shConfig.sheen_roughness_texture.has_transform ||
        trConfig.transmission_texture.has_transform ||
        spConfig.specular_color_texture.has_transform ||
        spConfig.specular_texture.has_transform ||
        irConfig.iridescence_texture.has_transform ||
        irConfig.iridescence_thickness_texture.has_transform;

    MaterialKey matkey {
        .doubleSided = !!inputMat->double_sided,
        .unlit = !!inputMat->unlit,
        .hasVertexColors = vertexColor,
        .hasBaseColorTexture = baseColorTexture->texture != nullptr,
        .hasNormalTexture = inputMat->normal_texture.texture != nullptr,
        .hasOcclusionTexture = inputMat->occlusion_texture.texture != nullptr,
        .hasEmissiveTexture = inputMat->emissive_texture.texture != nullptr,
        .enableDiagnostics = mDiagnosticsEnabled,
        .baseColorUV = static_cast<uint8_t>(baseColorTexture->texcoord),
        .hasClearCoatTexture = ccConfig.clearcoat_texture.texture != nullptr,
        .clearCoatUV = static_cast<uint8_t>(ccConfig.clearcoat_texture.texcoord),
        .hasClearCoatRoughnessTexture = ccConfig.clearcoat_roughness_texture.texture != nullptr,
        .clearCoatRoughnessUV = static_cast<uint8_t>(ccConfig.clearcoat_roughness_texture.texcoord),
        .hasClearCoatNormalTexture = ccConfig.clearcoat_normal_texture.texture != nullptr,
        .clearCoatNormalUV = static_cast<uint8_t>(ccConfig.clearcoat_normal_texture.texcoord),
        .hasClearCoat = !!inputMat->has_clearcoat,
        .hasTransmission = !!inputMat->has_transmission,
        .hasTextureTransforms = hasTextureTransforms,
        .emissiveUV = static_cast<uint8_t>(inputMat->emissive_texture.texcoord),
        .aoUV = static_cast<uint8_t>(inputMat->occlusion_texture.texcoord),
        .normalUV = static_cast<uint8_t>(inputMat->normal_texture.texcoord),
        .hasTransmissionTexture = trConfig.transmission_texture.texture != nullptr,
        .transmissionUV = static_cast<uint8_t>(trConfig.transmission_texture.texcoord),
        .hasSheenColorTexture = shConfig.sheen_color_texture.texture != nullptr,
        .sheenColorUV = static_cast<uint8_t>(shConfig.sheen_color_texture.texcoord),
        .hasSheenRoughnessTexture = shConfig.sheen_roughness_texture.texture != nullptr,
        .sheenRoughnessUV = static_cast<uint8_t>(shConfig.sheen_roughness_texture.texcoord),
        .hasVolumeThicknessTexture = vlConfig.thickness_texture.texture != nullptr,
        .volumeThicknessUV = static_cast<uint8_t>(vlConfig.thickness_texture.texcoord),
        .hasSheen = !!inputMat->has_sheen,
        .hasIOR = !!inputMat->has_ior,
        .hasVolume = !!inputMat->has_volume,
        .hasDispersion = !!inputMat->has_dispersion,
        .hasSpecular = !!inputMat->has_specular,
        .hasSpecularTexture = spConfig.specular_texture.texture != nullptr,
        .hasSpecularColorTexture = spConfig.specular_color_texture.texture != nullptr,
        .hasIridescence = !!inputMat->has_iridescence,
        .specularTextureUV = static_cast<uint8_t>(spConfig.specular_texture.texcoord),
        .specularColorTextureUV = static_cast<uint8_t>(spConfig.specular_color_texture.texcoord),
        .hasIridescenceTexture = irConfig.iridescence_texture.texture != nullptr,
        .iridescenceUV = static_cast<uint8_t>(irConfig.iridescence_texture.texcoord),
        .hasIridescenceThicknessTexture =
                irConfig.iridescence_thickness_texture.texture != nullptr,
        .iridescenceThicknessUV =
                static_cast<uint8_t>(irConfig.iridescence_thickness_texture.texcoord),
    };

    if (inputMat->has_pbr_specular_glossiness) {
        matkey.useSpecularGlossiness = true;
        if (sgConfig.diffuse_texture.texture) {
            *baseColorTexture = sgConfig.diffuse_texture;
            matkey.hasBaseColorTexture = true;
            matkey.baseColorUV = static_cast<uint8_t>(baseColorTexture->texcoord);
        }
        if (sgConfig.specular_glossiness_texture.texture) {
            *metallicRoughnessTexture = sgConfig.specular_glossiness_texture;
            matkey.hasSpecularGlossinessTexture = true;
            matkey.specularGlossinessUV = static_cast<uint8_t>(metallicRoughnessTexture->texcoord);
        }
    } else {
        matkey.hasMetallicRoughnessTexture = metallicRoughnessTexture->texture != nullptr;
        matkey.metallicRoughnessUV = static_cast<uint8_t>(metallicRoughnessTexture->texcoord);
    }

    switch (inputMat->alpha_mode) {
        case cgltf_alpha_mode_opaque:
            matkey.alphaMode = AlphaMode::OPAQUE;
            break;
        case cgltf_alpha_mode_mask:
            matkey.alphaMode = AlphaMode::MASK;
            break;
        case cgltf_alpha_mode_blend:
            matkey.alphaMode = AlphaMode::BLEND;
            break;
        case cgltf_alpha_mode_max_enum:
            break;
    }

    return matkey;
}

Material* FAssetLoader::getMaterial(const cgltf_material* inputMat,
        UvMap* uvmap, bool const vertexColor) const {
    cgltf_texture_view baseColorTexture;
    cgltf_texture_view metallicRoughnessTexture;
    if (UTILS_UNLIKELY(inputMat == nullptr)) {
        inputMat = &kDefaultMat;
    }
    MaterialKey matkey = getMaterialKey(inputMat, vertexColor, &baseColorTexture, &metallicRoughnessTexture);
    const char* label = inputMat->name ? inputMat->name : "material";
    Material* material = mMaterials.getMaterial(&matkey, uvmap, label);
    assert_invariant(material);
    return material;
}

MaterialInstance* FAssetLoader::createMaterialInstance(const cgltf_material* inputMat, UvMap* uvmap,
    bool vertexColor, FFilamentAsset* fAsset) {
    const cgltf_data* srcAsset = fAsset->mSourceAsset->hierarchy;
    MaterialInstanceCache::Entry* const cacheEntry =
            mMaterialInstanceCache.getEntry(&inputMat, vertexColor);
    if (cacheEntry->instance) {
        *uvmap = cacheEntry->uvmap;
        return cacheEntry->instance;
    }

    cgltf_texture_view baseColorTexture;
    cgltf_texture_view metallicRoughnessTexture;
    MaterialKey matkey = getMaterialKey(inputMat, vertexColor, &baseColorTexture, &metallicRoughnessTexture);

    // Check if this material has an extras string.
    CString extras;
    const cgltf_size extras_size = inputMat->extras.end_offset - inputMat->extras.start_offset;
    if (extras_size > 0) {
        extras = CString(srcAsset->json + inputMat->extras.start_offset, extras_size);
    }

    // This not only creates a material instance, it modifies the material key according to our
    // rendering constraints. For example, Filament only supports 2 sets of texture coordinates.
    MaterialInstance* mi = mMaterials.createMaterialInstance(&matkey, uvmap, inputMat->name,
            extras.c_str());
    if (!mi) {
        slog.e << "No material with the specified requirements exists." << io::endl;
        return nullptr;
    }

    auto mrConfig = inputMat->pbr_metallic_roughness;
    auto sgConfig = inputMat->pbr_specular_glossiness;
    auto ccConfig = inputMat->clearcoat;
    auto trConfig = inputMat->transmission;
    auto dpConfig = inputMat->dispersion;
    auto shConfig = inputMat->sheen;
    auto vlConfig = inputMat->volume;
    auto spConfig = inputMat->specular;
    auto irConfig = inputMat->iridescence;

    // Check the material blending mode, not the cgltf blending mode, because the provider
    // might have selected an alternative blend mode (e.g. to support transmission).
    if (mi->getMaterial()->getBlendingMode() == BlendingMode::MASKED) {
        mi->setMaskThreshold(inputMat->alpha_cutoff);
    }

    const float* emissive = &inputMat->emissive_factor[0];
    float3 emissiveFactor(emissive[0], emissive[1], emissive[2]);
    if (inputMat->has_emissive_strength) {
        emissiveFactor *= inputMat->emissive_strength.emissive_strength;
    }
    mi->setParameter("emissiveFactor", emissiveFactor);

    const float* c = mrConfig.base_color_factor;
    mi->setParameter("baseColorFactor", float4(c[0], c[1], c[2], c[3]));
    mi->setParameter("metallicFactor", mrConfig.metallic_factor);
    mi->setParameter("roughnessFactor", mrConfig.roughness_factor);

    if (matkey.useSpecularGlossiness) {
        const float* df = sgConfig.diffuse_factor;
        const float* sf = sgConfig.specular_factor;
        mi->setParameter("baseColorFactor", float4(df[0], df[1], df[2], df[3]));
        mi->setParameter("specularFactor", float3(sf[0], sf[1], sf[2]));
        mi->setParameter("glossinessFactor", sgConfig.glossiness_factor);
    }

    constexpr TextureProvider::TextureFlags sRGB = TextureProvider::TextureFlags::sRGB;
    constexpr TextureProvider::TextureFlags LINEAR = TextureProvider::TextureFlags::NONE;

    if (matkey.hasBaseColorTexture) {
        fAsset->addTextureBinding(mi, "baseColorMap", baseColorTexture.texture, sRGB);
        if (matkey.hasTextureTransforms) {
            const cgltf_texture_transform& uvt = baseColorTexture.transform;
            auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
            mi->setParameter("baseColorUvMatrix", uvmat);
        }
    }

    if (matkey.hasMetallicRoughnessTexture) {
        // The "metallicRoughnessMap" is actually a specular-glossiness map when the extension is
        // enabled. Note that KHR_materials_pbrSpecularGlossiness specifies that diffuseTexture and
        // specularGlossinessTexture are both sRGB, whereas the core glTF spec stipulates that
        // metallicRoughness is not sRGB.
        TextureProvider::TextureFlags srgb = inputMat->has_pbr_specular_glossiness ? sRGB : LINEAR;
        fAsset->addTextureBinding(mi, "metallicRoughnessMap", metallicRoughnessTexture.texture, srgb);
        if (matkey.hasTextureTransforms) {
            const cgltf_texture_transform& uvt = metallicRoughnessTexture.transform;
            auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
            mi->setParameter("metallicRoughnessUvMatrix", uvmat);
        }
    }

    if (matkey.hasNormalTexture) {
        fAsset->addTextureBinding(mi, "normalMap", inputMat->normal_texture.texture, LINEAR);
        if (matkey.hasTextureTransforms) {
            const cgltf_texture_transform& uvt = inputMat->normal_texture.transform;
            auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
            mi->setParameter("normalUvMatrix", uvmat);
        }
        mi->setParameter("normalScale", inputMat->normal_texture.scale);
    } else {
        mi->setParameter("normalScale", 1.0f);
    }

    if (matkey.hasOcclusionTexture) {
        fAsset->addTextureBinding(mi, "occlusionMap", inputMat->occlusion_texture.texture, LINEAR);
        if (matkey.hasTextureTransforms) {
            const cgltf_texture_transform& uvt = inputMat->occlusion_texture.transform;
            auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
            mi->setParameter("occlusionUvMatrix", uvmat);
        }
        mi->setParameter("aoStrength", inputMat->occlusion_texture.scale);
    } else {
        mi->setParameter("aoStrength", 1.0f);
    }

    if (matkey.hasEmissiveTexture) {
        fAsset->addTextureBinding(mi, "emissiveMap", inputMat->emissive_texture.texture, sRGB);
        if (matkey.hasTextureTransforms) {
            const cgltf_texture_transform& uvt = inputMat->emissive_texture.transform;
            auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
            mi->setParameter("emissiveUvMatrix", uvmat);
        }
    }

    if (matkey.hasClearCoat) {
        mi->setParameter("clearCoatFactor", ccConfig.clearcoat_factor);
        mi->setParameter("clearCoatRoughnessFactor", ccConfig.clearcoat_roughness_factor);

        if (matkey.hasClearCoatTexture) {
            fAsset->addTextureBinding(mi, "clearCoatMap", ccConfig.clearcoat_texture.texture,
                    LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = ccConfig.clearcoat_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("clearCoatUvMatrix", uvmat);
            }
        }
        if (matkey.hasClearCoatRoughnessTexture) {
            fAsset->addTextureBinding(mi, "clearCoatRoughnessMap",
                    ccConfig.clearcoat_roughness_texture.texture, LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = ccConfig.clearcoat_roughness_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("clearCoatRoughnessUvMatrix", uvmat);
            }
        }
        if (matkey.hasClearCoatNormalTexture) {
            fAsset->addTextureBinding(mi, "clearCoatNormalMap",
                    ccConfig.clearcoat_normal_texture.texture, LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = ccConfig.clearcoat_normal_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("clearCoatNormalUvMatrix", uvmat);
            }
            mi->setParameter("clearCoatNormalScale", ccConfig.clearcoat_normal_texture.scale);
        }
    }

    if (matkey.hasSheen) {
        const float* s = shConfig.sheen_color_factor;
        mi->setParameter("sheenColorFactor", float3{s[0], s[1], s[2]});
        mi->setParameter("sheenRoughnessFactor", shConfig.sheen_roughness_factor);

        if (matkey.hasSheenColorTexture) {
            fAsset->addTextureBinding(mi, "sheenColorMap", shConfig.sheen_color_texture.texture,
                    sRGB);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = shConfig.sheen_color_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("sheenColorUvMatrix", uvmat);
            }
        }
        if (matkey.hasSheenRoughnessTexture) {
            bool sameTexture = shConfig.sheen_color_texture.texture == shConfig.sheen_roughness_texture.texture;
            fAsset->addTextureBinding(mi, "sheenRoughnessMap",
                    shConfig.sheen_roughness_texture.texture, sameTexture ? sRGB : LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = shConfig.sheen_roughness_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("sheenRoughnessUvMatrix", uvmat);
            }
        }
    }

    if (matkey.hasVolume) {
        mi->setParameter("volumeThicknessFactor", vlConfig.thickness_factor);

        float attenuationDistance = vlConfig.attenuation_distance;
        // TODO: We assume a color in linear sRGB, is this correct? The spec doesn't say anything
        const float* attenuationColor = vlConfig.attenuation_color;
        LinearColor absorption = Color::absorptionAtDistance(
                *reinterpret_cast<const LinearColor*>(attenuationColor), attenuationDistance);
        mi->setParameter("volumeAbsorption", RgbType::LINEAR, absorption);

        if (matkey.hasVolumeThicknessTexture) {
            fAsset->addTextureBinding(mi, "volumeThicknessMap", vlConfig.thickness_texture.texture,
                    LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = vlConfig.thickness_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("volumeThicknessUvMatrix", uvmat);
            }
        }
    }

    if (matkey.hasTransmission) {
        mi->setParameter("transmissionFactor", trConfig.transmission_factor);
        if (matkey.hasTransmissionTexture) {
            fAsset->addTextureBinding(mi, "transmissionMap", trConfig.transmission_texture.texture,
                    LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = trConfig.transmission_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("transmissionUvMatrix", uvmat);
            }
        }
    }

    if (matkey.hasDispersion) {
        mi->setParameter("dispersion", dpConfig.dispersion);
    }

    // IOR can be implemented as either IOR or reflectance because of ubershaders
    if (matkey.hasIOR) {
        if (mi->getMaterial()->hasParameter("ior")) {
            mi->setParameter("ior", inputMat->ior.ior);
        }
        if (mi->getMaterial()->hasParameter("reflectance")) {
            float ior = inputMat->ior.ior;
            float f0 = (ior - 1.0f) / (ior + 1.0f);
            f0 *= f0;
            float reflectance = std::sqrt(f0 / 0.16f);
            mi->setParameter("reflectance", reflectance);
        }
    }

    if (mi->getMaterial()->hasParameter("emissiveStrength")) {
        mi->setParameter("emissiveStrength", inputMat->has_emissive_strength ?
                inputMat->emissive_strength.emissive_strength : 1.0f);
    }

    if (matkey.hasSpecular) {
        const float* s = spConfig.specular_color_factor;
        mi->setParameter("specularColorFactor", float3{s[0], s[1], s[2]});
        mi->setParameter("specularStrength", spConfig.specular_factor);

        if (matkey.hasSpecularColorTexture) {
            fAsset->addTextureBinding(mi, "specularColorMap", spConfig.specular_color_texture.texture, sRGB);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform uvt = spConfig.specular_color_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("specularColorUvMatrix", uvmat);
            }
        }
        if (matkey.hasSpecularTexture) {
            bool sameTexture = spConfig.specular_color_texture.texture == spConfig.specular_texture.texture;
            fAsset->addTextureBinding(mi, "specularMap", spConfig.specular_texture.texture, sameTexture ? sRGB : LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform uvt = spConfig.specular_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("specularUvMatrix", uvmat);
            }
        }
    }

    if (matkey.hasIridescence) {
        mi->setParameter("iridescenceFactor", irConfig.iridescence_factor);
        mi->setParameter("iridescenceIor", irConfig.iridescence_ior);
        mi->setParameter("iridescenceThicknessMaximum",
                irConfig.iridescence_thickness_max);

        if (matkey.hasIridescenceTexture) {
            fAsset->addTextureBinding(mi, "iridescenceMap",
                    irConfig.iridescence_texture.texture, LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform uvt = irConfig.iridescence_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("iridescenceUvMatrix", uvmat);
            }
        }
        if (matkey.hasIridescenceThicknessTexture) {
            mi->setParameter("iridescenceThicknessMinimum",
                    irConfig.iridescence_thickness_min);
            fAsset->addTextureBinding(mi, "iridescenceThicknessMap",
                    irConfig.iridescence_thickness_texture.texture, LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform uvt =
                        irConfig.iridescence_thickness_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("iridescenceThicknessUvMatrix", uvmat);
            }
        }
    }

    *cacheEntry = { .instance = mi, .uvmap = *uvmap };
    return mi;
}

void FAssetLoader::importSkins(FFilamentInstance* instance, const cgltf_data* srcAsset) {
    instance->mSkins.reserve(srcAsset->skins_count);
    instance->mSkins.resize(srcAsset->skins_count);
    const auto& nodeMap = instance->mNodeMap;
    for (cgltf_size i = 0, len = srcAsset->nodes_count; i < len; ++i) {
        const cgltf_node& node = srcAsset->nodes[i];
        Entity entity = nodeMap[i];
        if (node.skin && entity) {
            int const skinIndex = node.skin - &srcAsset->skins[0];
            instance->mSkins[skinIndex].targets.insert(entity);
        }
    }
    for (cgltf_size i = 0, len = srcAsset->skins_count; i < len; ++i) {
        FFilamentInstance::Skin& dstSkin = instance->mSkins[i];
        const cgltf_skin& srcSkin = srcAsset->skins[i];

        // Build a list of transformables for this skin, one for each joint.
        dstSkin.joints = FixedCapacityVector<Entity>(srcSkin.joints_count);
        for (cgltf_size j = 0, jointsLen = srcSkin.joints_count; j < jointsLen; ++j) {
            dstSkin.joints[j] = nodeMap[srcSkin.joints[j] - srcAsset->nodes];
        }
    }
}

FFilamentAsset* FAssetLoader::preresolveTextures(FFilamentAsset* fAsset,
        const cgltf_data* srcAsset) const {
    // This part fills out FFilamentAsset::mTextures so that even if we started with num instance=0
    // for createInstancedAsset, later calls to createInstance will still succeed. (mTextures needs
    // to have the proper flags set before ResourceLoader creates the actual Filament textures).

    // Pre-resolve textures for all materials so that they can be loaded even if no instances are
    // created.
    auto resolveTexture = [&](const cgltf_texture* texture, TextureProvider::TextureFlags const flags) {
        if (texture) {
            const size_t gltfTextureIndex = static_cast<size_t>(texture - srcAsset->textures);
            fAsset->obtainAssetTextureIndex(gltfTextureIndex, flags);
        }
    };
    for (size_t i = 0; i < srcAsset->materials_count; ++i) {
        const cgltf_material* inputMat = &srcAsset->materials[i];
        cgltf_texture_view baseColorTexture;
        cgltf_texture_view metallicRoughnessTexture;
        MaterialKey matkey = getMaterialKey(inputMat, false, &baseColorTexture, &metallicRoughnessTexture);

        constexpr TextureProvider::TextureFlags sRGB = TextureProvider::TextureFlags::sRGB;
        constexpr TextureProvider::TextureFlags LINEAR = TextureProvider::TextureFlags::NONE;

        // This section has the exact same checks and flags as createMaterialInstance().
        if (matkey.hasBaseColorTexture) {
            resolveTexture(baseColorTexture.texture, sRGB);
        }
        if (matkey.hasMetallicRoughnessTexture) {
            TextureProvider::TextureFlags srgb =
                    inputMat->has_pbr_specular_glossiness ? sRGB : LINEAR;
            resolveTexture(metallicRoughnessTexture.texture, srgb);
        }
        if (matkey.hasNormalTexture) {
            resolveTexture(inputMat->normal_texture.texture, LINEAR);
        }
        if (matkey.hasOcclusionTexture) {
            resolveTexture(inputMat->occlusion_texture.texture, LINEAR);
        }
        if (matkey.hasEmissiveTexture) {
            resolveTexture(inputMat->emissive_texture.texture, sRGB);
        }

        if (matkey.hasClearCoat) {
            auto ccConfig = inputMat->clearcoat;
            if (matkey.hasClearCoatTexture) {
                resolveTexture(ccConfig.clearcoat_texture.texture, LINEAR);
            }
            if (matkey.hasClearCoatRoughnessTexture) {
                resolveTexture(ccConfig.clearcoat_roughness_texture.texture, LINEAR);
            }
            if (matkey.hasClearCoatNormalTexture) {
                resolveTexture(ccConfig.clearcoat_normal_texture.texture, LINEAR);
            }
        }
        if (matkey.hasSheen) {
            auto shConfig = inputMat->sheen;
            if (matkey.hasSheenColorTexture) {
                resolveTexture(shConfig.sheen_color_texture.texture, sRGB);
            }
            if (matkey.hasSheenRoughnessTexture) {
                bool sameTexture = shConfig.sheen_color_texture.texture ==
                                   shConfig.sheen_roughness_texture.texture;
                resolveTexture(shConfig.sheen_roughness_texture.texture,
                        sameTexture ? sRGB : LINEAR);
            }
        }
        if (matkey.hasVolume) {
            auto vlConfig = inputMat->volume;
            if (matkey.hasVolumeThicknessTexture) {
                resolveTexture(vlConfig.thickness_texture.texture, LINEAR);
            }
        }
        if (matkey.hasTransmission) {
            auto trConfig = inputMat->transmission;
            if (matkey.hasTransmissionTexture) {
                resolveTexture(trConfig.transmission_texture.texture, LINEAR);
            }
        }
        if (matkey.hasSpecular) {
            auto spConfig = inputMat->specular;
            if (matkey.hasSpecularColorTexture) {
                resolveTexture(spConfig.specular_color_texture.texture, sRGB);
            }
            if (matkey.hasSpecularTexture) {
                bool sameTexture = spConfig.specular_color_texture.texture ==
                                   spConfig.specular_texture.texture;
                resolveTexture(spConfig.specular_texture.texture, sameTexture ? sRGB : LINEAR);
            }
        }
        if (matkey.hasIridescence) {
            auto irConfig = inputMat->iridescence;
            if (matkey.hasIridescenceTexture) {
                resolveTexture(irConfig.iridescence_texture.texture, LINEAR);
            }
            if (matkey.hasIridescenceThicknessTexture) {
                resolveTexture(irConfig.iridescence_thickness_texture.texture, LINEAR);
            }
        }
    }
    return fAsset;
}

bool AssetConfigurationExtended::isSupported() {
#if defined(__ANDROID__) || defined(FILAMENT_IOS) || defined(__EMSCRIPTEN__)
    return false;
#else
    return true;
#endif
}

AssetLoader* AssetLoader::create(const AssetConfiguration& config) {
    return new FAssetLoader(config);
}

void AssetLoader::destroy(AssetLoader** loader) {
    FAssetLoader* temp(downcast(*loader));
    FAssetLoader::destroy(&temp);
    *loader = temp;
}

FilamentAsset* AssetLoader::createAsset(uint8_t const* bytes, uint32_t const numBytes) {
    return downcast(this)->createAsset(bytes, numBytes);
}

FilamentAsset* AssetLoader::createInstancedAsset(const uint8_t* bytes, uint32_t const numBytes,
        FilamentInstance** instances, size_t const numInstances) {
    return downcast(this)->createInstancedAsset(bytes, numBytes, instances, numInstances);
}

FilamentInstance* AssetLoader::createInstance(FilamentAsset* asset) {
    return downcast(this)->createInstance(downcast(asset));
}

void AssetLoader::enableDiagnostics(bool const enable) {
    downcast(this)->mDiagnosticsEnabled = enable;
}

void AssetLoader::destroyAsset(const FilamentAsset* asset) {
    downcast(this)->destroyAsset(downcast(asset));
}

void AssetLoader::gc() noexcept {
    downcast(this)->gc();
}

size_t AssetLoader::getMaterialsCount() const noexcept {
    return downcast(this)->getMaterialsCount();
}

NameComponentManager* AssetLoader::getNames() const noexcept {
    return downcast(this)->getNames();
}

const Material* const* AssetLoader::getMaterials() const noexcept {
    return downcast(this)->getMaterials();
}

MaterialProvider& AssetLoader::getMaterialProvider() noexcept {
    return downcast(this)->mMaterials;
}

} // namespace filament::gltfio
