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

#include <gltfio/Animator.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/math.h>

#include "FFilamentAsset.h"
#include "FNodeManager.h"
#include "GltfEnums.h"

#include <filament/Box.h>
#include <filament/BufferObject.h>
#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MorphTargetBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>

#include <math/mat4.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/EntityManager.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/NameComponentManager.h>
#include <utils/Systrace.h>

#include <tsl/robin_map.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "downcast.h"

using namespace filament;
using namespace filament::math;
using namespace utils;

namespace filament::gltfio {

using SceneMask = NodeManager::SceneMask;

static const auto FREE_CALLBACK = [](void* mem, size_t, void*) { free(mem); };

// The default glTF material.
static constexpr cgltf_material kDefaultMat = {
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

// Sometimes a glTF bufferview includes unused data at the end (e.g. in skinning.gltf) so we need to
// compute the correct size of the vertex buffer. Filament automatically infers the size of
// driver-level vertex buffers from the attribute data (stride, count, offset) and clients are
// expected to avoid uploading data blobs that exceed this size. Since this information doesn't
// exist in the glTF we need to compute it manually. This is a bit of a cheat, cgltf_calc_size is
// private but its implementation file is available in this cpp file.
uint32_t computeBindingSize(const cgltf_accessor* accessor) {
    cgltf_size element_size = cgltf_calc_size(accessor->type, accessor->component_type);
    return uint32_t(accessor->stride * (accessor->count - 1) + element_size);
}

uint32_t computeBindingOffset(const cgltf_accessor* accessor) {
    return uint32_t(accessor->offset + accessor->buffer_view->offset);
}

static const char* getNodeName(const cgltf_node* node, const char* defaultNodeName) {
    if (node->name) return node->name;
    if (node->mesh && node->mesh->name) return node->mesh->name;
    if (node->light && node->light->name) return node->light->name;
    if (node->camera && node->camera->name) return node->camera->name;
    return defaultNodeName;
}

static bool primitiveHasVertexColor(const cgltf_primitive& inPrim) {
    for (int slot = 0; slot < inPrim.attributes_count; slot++) {
        const cgltf_attribute& inputAttribute = inPrim.attributes[slot];
        if (inputAttribute.type == cgltf_attribute_type_color) {
            return true;
        }
    }
    return false;
}

static LightManager::Type getLightType(const cgltf_light_type light) {
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

struct FAssetLoader : public AssetLoader {
    FAssetLoader(const AssetConfiguration& config) :
            mEntityManager(config.entities ? *config.entities : EntityManager::get()),
            mRenderableManager(config.engine->getRenderableManager()),
            mNameManager(config.names),
            mTransformManager(config.engine->getTransformManager()),
            mMaterials(*config.materials),
            mEngine(*config.engine),
            mDefaultNodeName(config.defaultNodeName) {}

    FFilamentAsset* createAsset(const uint8_t* bytes, uint32_t nbytes);
    FFilamentAsset* createInstancedAsset(const uint8_t* bytes, uint32_t numBytes,
            FilamentInstance** instances, size_t numInstances);
    FilamentInstance* createInstance(FFilamentAsset* primary);
    void importSkins(FFilamentAsset* primary, FFilamentInstance* instance,
            const cgltf_data* srcAsset);

    static void destroy(FAssetLoader** loader) noexcept {
        delete *loader;
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
    // Methods used during the first traveral (creation of VertexBuffer, IndexBuffer, etc)
    void createRootAsset(const cgltf_data* srcAsset);
    void recursePrimitives(const cgltf_data* srcAsset, const cgltf_node* rootNode);
    void createPrimitives(const cgltf_data* srcAsset, const cgltf_node* node, const char* name);
    bool createPrimitive(const cgltf_primitive& inPrim, Primitive* outPrim, const char* name);

    // Methods used during subsequent traverals (creation of entities, renderables, etc)
    void createInstances(const cgltf_data* srcAsset, size_t numInstances);
    FFilamentInstance* createInstance(const cgltf_data* srcAsset);
    void recurseEntities(const cgltf_data* srcAsset, const cgltf_node* node, SceneMask scenes,
            Entity parent, FFilamentInstance* instance);
    void createRenderable(const cgltf_data* srcAsset, const cgltf_node* node, Entity entity,
            const char* name, FFilamentInstance* instance);
    void createLight(const cgltf_light* light, Entity entity);
    void createCamera(const cgltf_camera* camera, Entity entity);
    void addTextureBinding(MaterialInstance* materialInstance, const char* parameterName,
            const cgltf_texture* srcTexture, bool srgb);
    void createMaterialVariants(const cgltf_data* srcAsset, const cgltf_mesh* mesh, Entity entity,
            FFilamentInstance* instance);

    // Utility methods that work with MaterialProvider.
    Material* getMaterial(const cgltf_data* srcAsset, const cgltf_material* inputMat, UvMap* uvmap,
            bool vertexColor);
    MaterialInstance* createMaterialInstance(const cgltf_data* srcAsset,
            const cgltf_material* inputMat, UvMap* uvmap, bool vertexColor);
    MaterialKey getMaterialKey(const cgltf_data* srcAsset,
            const cgltf_material* inputMat, UvMap* uvmap, bool vertexColor,
            cgltf_texture_view* baseColorTexture,
            cgltf_texture_view* metallicRoughnessTexture) const;

public:
    EntityManager& mEntityManager;
    RenderableManager& mRenderableManager;
    NameComponentManager* const mNameManager;
    TransformManager& mTransformManager;
    MaterialProvider& mMaterials;
    Engine& mEngine;
    FNodeManager mNodeManager;

    // Transient state used only for the asset currently being loaded:
    FFilamentAsset* mAsset;
    tsl::robin_map<cgltf_node*, SceneMask> mRootNodes;
    const char* mDefaultNodeName;
    bool mError = false;
    bool mDiagnosticsEnabled = false;
    MaterialInstanceCache mMaterialInstanceCache;

    // Weak reference to the largest dummy buffer so far in the current loading phase.
    BufferObject* mDummyBufferObject = nullptr;
};

FILAMENT_DOWNCAST(AssetLoader)

FFilamentAsset* FAssetLoader::createAsset(const uint8_t* bytes, uint32_t byteCount) {
    FilamentInstance* instances;
    return createInstancedAsset(bytes, byteCount, &instances, 1);
}

FFilamentAsset* FAssetLoader::createInstancedAsset(const uint8_t* bytes, uint32_t byteCount,
        FilamentInstance** instances, size_t numInstances) {
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
    utils::FixedCapacityVector<uint8_t> glbdata(byteCount);
    std::copy_n(bytes, byteCount, glbdata.data());

    cgltf_data* sourceAsset;
    cgltf_result result = cgltf_parse(&options, glbdata.data(), byteCount, &sourceAsset);
    if (result != cgltf_result_success) {
        slog.e << "Unable to parse glTF file." << io::endl;
        return nullptr;
    }

    createRootAsset(sourceAsset);
    if (mError) {
        delete mAsset;
        mAsset = nullptr;
        mError = false;
        return nullptr;
    }

    createInstances(sourceAsset, numInstances);
    if (mError) {
        delete mAsset;
        mAsset = nullptr;
        mError = false;
        return nullptr;
     }

    glbdata.swap(mAsset->mSourceAsset->glbData);
    std::copy_n(mAsset->mInstances.data(), numInstances, instances);
    return mAsset;
}

// note there a two overloads; this is the high-level one
FilamentInstance* FAssetLoader::createInstance(FFilamentAsset* primary) {
    if (!primary->mSourceAsset) {
        slog.e << "Source data has been released; asset is frozen." << io::endl;
        return nullptr;
    }
    const cgltf_data* srcAsset = primary->mSourceAsset->hierarchy;
    if (srcAsset->scenes == nullptr) {
        slog.e << "There is no scene in the asset." << io::endl;
        return nullptr;
    }

    FFilamentInstance* instance = createInstance(srcAsset);

    primary->mDependencyGraph.commitEdges();
    return instance;
}

void FAssetLoader::createRootAsset(const cgltf_data* srcAsset) {
    SYSTRACE_CALL();
    #if !GLTFIO_DRACO_SUPPORTED
    for (cgltf_size i = 0; i < srcAsset->extensions_required_count; i++) {
        if (!strcmp(srcAsset->extensions_required[i], "KHR_draco_mesh_compression")) {
            slog.e << "KHR_draco_mesh_compression is not supported." << io::endl;
            mAsset = nullptr;
            return;
        }
    }
    #endif

    mDummyBufferObject = nullptr;
    mAsset = new FFilamentAsset(&mEngine, mNameManager, &mEntityManager, &mNodeManager, srcAsset);

    // It is not an error for a glTF file to have zero scenes.
    mAsset->mScenes.clear();
    if (srcAsset->scenes == nullptr) {
        return;
    }

    // Create a single root node with an identity transform as a convenience to the client.
    mAsset->mRoot = mEntityManager.create();
    mTransformManager.create(mAsset->mRoot);

    // Check if the asset has an extras string.
    const cgltf_asset& asset = srcAsset->asset;
    const cgltf_size extras_size = asset.extras.end_offset - asset.extras.start_offset;
    if (extras_size > 1) {
        mAsset->mAssetExtras = CString(srcAsset->json + asset.extras.start_offset, extras_size);
    }

    // Build a mapping of root nodes to scene membership sets.
    assert_invariant(srcAsset->scenes_count <= NodeManager::MAX_SCENE_COUNT);
    mRootNodes.clear();
    const size_t sic = std::min(srcAsset->scenes_count, NodeManager::MAX_SCENE_COUNT);
    mAsset->mScenes.reserve(sic);
    for (size_t si = 0; si < sic; ++si) {
        const cgltf_scene& scene = srcAsset->scenes[si];
        mAsset->mScenes.emplace_back(scene.name);
        for (size_t ni = 0, nic = scene.nodes_count; ni < nic; ++ni) {
            mRootNodes[scene.nodes[ni]].set(si);
        }
    }

    // Some exporters (e.g. Cinema4D) produce assets with a separate animation hierarchy and
    // modeling hierarchy, where nodes in the former have no associated scene. We need to create
    // transformable entities for "un-scened" nodes in case they have bones.
    for (size_t i = 0, n = srcAsset->nodes_count; i < n; ++i) {
        cgltf_node* node = &srcAsset->nodes[i];
        if (node->parent == nullptr && mRootNodes.find(node) == mRootNodes.end()) {
            mRootNodes.insert({node, {}});
        }
    }

    for (const auto& [node, sceneMask] : mRootNodes) {
        recursePrimitives(srcAsset, node);
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
    mAsset->mResourceUris.reserve(resourceUris.size());
    for (std::string_view uri : resourceUris) {
        mAsset->mResourceUris.push_back(uri.data());
    }
}

void FAssetLoader::recursePrimitives(const cgltf_data* srcAsset, const cgltf_node* node) {
    const char* name = getNodeName(node, mDefaultNodeName);
    name = name ? name : "node";

    if (node->mesh) {
        createPrimitives(srcAsset, node, name);
        mAsset->mRenderableCount++;
    }

    for (cgltf_size i = 0, len = node->children_count; i < len; ++i) {
        recursePrimitives(srcAsset, node->children[i]);
    }
}

void FAssetLoader::createInstances(const cgltf_data* srcAsset, size_t numInstances) {
    // Create a separate entity hierarchy for each instance. Note that MeshCache (vertex
    // buffers and index buffers) and MaterialInstanceCache (materials and textures) help avoid
    // needless duplication of resources.
    for (size_t index = 0; index < numInstances; ++index) {
        if (createInstance(srcAsset) == nullptr) {
            mError = true;
            break;
        }
    }

    // Sort the entities so that the renderable ones come first. This allows us to expose
    // a "renderables only" pointer without storing a separate list.
    const auto& rm = mEngine.getRenderableManager();
    std::partition(mAsset->mEntities.begin(), mAsset->mEntities.end(), [&rm](Entity a) {
        return rm.hasComponent(a);
    });

    if (mError) {
        destroyAsset(mAsset);
        mAsset = nullptr;
        mError = false;
    }
}

// note there a two overloads; this is the low-level one
FFilamentInstance* FAssetLoader::createInstance(const cgltf_data* srcAsset) {
    auto rootTransform = mTransformManager.getInstance(mAsset->mRoot);
    Entity instanceRoot = mEntityManager.create();
    mTransformManager.create(instanceRoot, rootTransform);

    mMaterialInstanceCache = MaterialInstanceCache(srcAsset);

    // Create an instance object, which is a just a lightweight wrapper around a vector of
    // entities and an animator. The creation of animator is triggered from ResourceLoader
    // because it could require external bin data.
    FFilamentInstance* instance = new FFilamentInstance(instanceRoot, mAsset);

    // Check if the asset has variants.
    instance->mVariants.reserve(srcAsset->variants_count);
    for (cgltf_size i = 0, len = srcAsset->variants_count; i < len; ++i) {
        instance->mVariants.push_back({CString(srcAsset->variants[i].name)});
    }

    // For each scene root, recursively create all entities.
    for (const auto& pair : mRootNodes) {
        recurseEntities(srcAsset, pair.first, pair.second, instanceRoot, instance);
    }

    importSkins(mAsset, instance, srcAsset);

    // Now that all entities have been created, the instance can create the animator component.
    // Note that it may need to defer actual creation until external buffers are fully loaded.
    instance->createAnimator();

    mAsset->mInstances.push_back(instance);

    // Bounding boxes are not shared because users might call recomputeBoundingBoxes() which can
    // be affected by entity transforms. However, upon instance creation we can safely copy over
    // the asset's bounding box.
    instance->mBoundingBox = mAsset->mBoundingBox;

    mMaterialInstanceCache.flush(&instance->mMaterialInstances);

    return instance;
}

void FAssetLoader::recurseEntities(const cgltf_data* srcAsset, const cgltf_node* node,
        SceneMask scenes, Entity parent, FFilamentInstance* instance) {
    NodeManager& nm = mNodeManager;
    const Entity entity = mEntityManager.create();
    nm.create(entity);
    const auto nodeInstance = nm.getInstance(entity);
    nm.setSceneMembership(nodeInstance, scenes);

    // Always create a transform component to reflect the original hierarchy.
    mat4f localTransform;
    if (node->has_matrix) {
        memcpy(&localTransform[0][0], &node->matrix[0], 16 * sizeof(float));
    } else {
        quatf* rotation = (quatf*) &node->rotation[0];
        float3* scale = (float3*) &node->scale[0];
        float3* translation = (float3*) &node->translation[0];
        localTransform = composeMatrix(*translation, *rotation, *scale);
    }

    auto parentTransform = mTransformManager.getInstance(parent);
    mTransformManager.create(entity, parentTransform, localTransform);

    // Check if this node has an extras string.
    const cgltf_size extras_size = node->extras.end_offset - node->extras.start_offset;
    if (extras_size > 0) {
        const cgltf_data* srcAsset = mAsset->mSourceAsset->hierarchy;
        mNodeManager.setExtras(mNodeManager.getInstance(entity),
                {srcAsset->json + node->extras.start_offset, extras_size});
    }

    // Update the asset's entity list and private node mapping.
    mAsset->mEntities.push_back(entity);
    instance->mEntities.push_back(entity);
    instance->mNodeMap[node - srcAsset->nodes] = entity;

    const char* name = getNodeName(node, mDefaultNodeName);

    if (name) {
        mAsset->mNameToEntity[name].push_back(entity);
        if (mNameManager) {
            mNameManager->addComponent(entity);
            mNameManager->setName(mNameManager->getInstance(entity), name);
        }
    }

    // If no name is provided in the glTF or AssetConfiguration, use "node" for error messages.
    name = name ? name : "node";

    // If the node has a mesh, then create a renderable component.
    if (node->mesh) {
        createRenderable(srcAsset, node, entity, name, instance);
        if (srcAsset->variants_count > 0) {
            createMaterialVariants(srcAsset, node->mesh, entity, instance);
        }
    }

    if (node->light) {
        createLight(node->light, entity);
    }

    if (node->camera) {
        createCamera(node->camera, entity);
    }

    for (cgltf_size i = 0, len = node->children_count; i < len; ++i) {
        recurseEntities(srcAsset, node->children[i], scenes, entity, instance);
    }
}

void FAssetLoader::createPrimitives(const cgltf_data* srcAsset, const cgltf_node* node,
        const char* name) {
    const cgltf_mesh* mesh = node->mesh;
    assert_invariant(mesh != nullptr);

    // If the mesh is already loaded, obtain the list of Filament VertexBuffer / IndexBuffer objects
    // that were already generated (one for each primitive), otherwise allocate a new list of
    // pointers for the primitives.
    FixedCapacityVector<Primitive>& prims = mAsset->mMeshCache[mesh - srcAsset->meshes];
    if (prims.empty()) {
        prims.reserve(mesh->primitives_count);
        prims.resize(mesh->primitives_count);
    }

    Aabb aabb;

    for (cgltf_size index = 0, n = mesh->primitives_count; index < n; ++index) {
        Primitive& outputPrim = prims[index];
        const cgltf_primitive& inputPrim = mesh->primitives[index];

        // Create a Filament VertexBuffer and IndexBuffer for this prim if we haven't already.
        if (!outputPrim.vertices && !createPrimitive(inputPrim, &outputPrim, name)) {
            mError = true;
            return;
        }

        // Expand the object-space bounding box.
        aabb.min = min(outputPrim.aabb.min, aabb.min);
        aabb.max = max(outputPrim.aabb.max, aabb.max);
    }

    mat4f worldTransform;
    cgltf_node_transform_world(node, &worldTransform[0][0]);

    const Aabb transformed = aabb.transform(worldTransform);
    mAsset->mBoundingBox.min = min(mAsset->mBoundingBox.min, transformed.min);
    mAsset->mBoundingBox.max = max(mAsset->mBoundingBox.max, transformed.max);
 }

void FAssetLoader::createRenderable(const cgltf_data* srcAsset, const cgltf_node* node,
        Entity entity, const char* name, FFilamentInstance* instance) {
    const cgltf_mesh* mesh = node->mesh;
    const cgltf_size primitiveCount = mesh->primitives_count;

    // If the mesh is already loaded, obtain the list of Filament VertexBuffer / IndexBuffer objects
    // that were already generated (one for each primitive).
    FixedCapacityVector<Primitive>& prims = mAsset->mMeshCache[mesh - srcAsset->meshes];
    assert_invariant(prims.size() == primitiveCount);
    Primitive* outputPrim = prims.data();
    const cgltf_primitive* inputPrim = &mesh->primitives[0];

    Aabb aabb;

    // glTF spec says that all primitives must have the same number of morph targets.
    const cgltf_size numMorphTargets = inputPrim ? inputPrim->targets_count : 0;
    RenderableManager::Builder builder(primitiveCount);
    builder.morphing(numMorphTargets);

    // For each prim, create a Filament VertexBuffer, IndexBuffer, and MaterialInstance.
    // The VertexBuffer and IndexBuffer objects are cached for possible re-use, but MaterialInstance
    // is not.
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
        bool hasVertexColor = primitiveHasVertexColor(*inputPrim);
        MaterialInstance* mi = createMaterialInstance(srcAsset, inputPrim->material, &uvmap,
                hasVertexColor);
        assert_invariant(mi);
        if (!mi) {
            mError = true;
            continue;
        }

        mAsset->mDependencyGraph.addEdge(entity, mi);
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
            assert_invariant(outputPrim->targets);
            builder.morphing(0, index, outputPrim->targets);
        }
    }

    FixedCapacityVector<CString> morphTargetNames(numMorphTargets);
    for (cgltf_size i = 0, c = mesh->target_names_count; i < c; ++i) {
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
        RenderableManager::Instance renderable = mRenderableManager.getInstance(entity);
        const auto size = std::min(MAX_MORPH_TARGETS, numMorphTargets);
        FixedCapacityVector<float> weights(size, 0.0f);
        for (cgltf_size i = 0, c = std::min(size, mesh->weights_count); i < c; ++i) {
            weights[i] = mesh->weights[i];
        }
        for (cgltf_size i = 0, c = std::min(size, node->weights_count); i < c; ++i) {
            weights[i] = node->weights[i];
        }
        mRenderableManager.setMorphWeights(renderable, weights.data(), size);
    }
}

void FAssetLoader::createMaterialVariants(const cgltf_data* srcAsset, const cgltf_mesh* mesh,
        Entity entity, FFilamentInstance* instance) {
    UvMap uvmap {};
    for (cgltf_size prim = 0, n = mesh->primitives_count; prim < n; ++prim) {
        const cgltf_primitive& srcPrim = mesh->primitives[prim];
        for (size_t i = 0, m = srcPrim.mappings_count; i < m; i++) {
            const size_t variantIndex = srcPrim.mappings[i].variant;
            const cgltf_material* material = srcPrim.mappings[i].material;
            bool hasVertexColor = primitiveHasVertexColor(srcPrim);
            MaterialInstance* mi =
                    createMaterialInstance(srcAsset, material, &uvmap, hasVertexColor);
            assert_invariant(mi);
            if (!mi) {
                mError = true;
                break;
            }
            mAsset->mDependencyGraph.addEdge(entity, mi);
            instance->mVariants[variantIndex].mappings.push_back({entity, prim, mi});
        }
    }
}

bool FAssetLoader::createPrimitive(const cgltf_primitive& inPrim, Primitive* outPrim,
        const char* name) {
    Material* material = getMaterial(mAsset->mSourceAsset->hierarchy,
                inPrim.material, &outPrim->uvmap, primitiveHasVertexColor(inPrim));
    AttributeBitset requiredAttributes = material->getRequiredAttributes();

    // TODO: populate a mapping of Texture Index => [MaterialInstance, const char*] slots.
    // By creating this mapping during the "recursePrimitives" phase, we will can allow
    // zero-instance assets to exist. This will be useful for "preloading", which is a feature
    // request from Google.

    // Create a little lambda that appends to the asset's vertex buffer slots.
    auto slots = &mAsset->mBufferSlots;
    auto addBufferSlot = [slots](BufferSlot entry) {
        slots->push_back(entry);
    };

    // In glTF, each primitive may or may not have an index buffer.
    IndexBuffer* indices = nullptr;
    const cgltf_accessor* accessor = inPrim.indices;
    if (accessor) {
        IndexBuffer::IndexType indexType;
        if (!getIndexType(accessor->component_type, &indexType)) {
            utils::slog.e << "Unrecognized index type in " << name << utils::io::endl;
            return false;
        }

        indices = IndexBuffer::Builder()
            .indexCount(accessor->count)
            .bufferType(indexType)
            .build(mEngine);

        BufferSlot slot = { accessor };
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
        for (size_t i = 0; i < vertexCount; ++i) {
            indexData[i] = i;
        }
        IndexBuffer::BufferDescriptor bd(indexData, indexDataSize, FREE_CALLBACK);
        indices->setBuffer(mEngine, std::move(bd));
    }
    mAsset->mIndexBuffers.push_back(indices);

    VertexBuffer::Builder vbb;
    vbb.enableBufferObjects();

    bool hasUv0 = false, hasUv1 = false, hasVertexColor = false, hasNormals = false;
    uint32_t vertexCount = 0;

    const size_t firstSlot = mAsset->mBufferSlots.size();
    int slot = 0;

    for (cgltf_size aindex = 0; aindex < inPrim.attributes_count; aindex++) {
        const cgltf_attribute& attribute = inPrim.attributes[aindex];
        const int index = attribute.index;
        const cgltf_attribute_type atype = attribute.type;
        const cgltf_accessor* accessor = attribute.data;

        // The glTF tangent data is ignored here, but honored in ResourceLoader.
        if (atype == cgltf_attribute_type_tangent) {
            continue;
        }

        // At a minimum, surface orientation requires normals to be present in the source data.
        // Here we re-purpose the normals slot to point to the quats that get computed later.
        if (atype == cgltf_attribute_type_normal) {
            vbb.attribute(VertexAttribute::TANGENTS, slot, VertexBuffer::AttributeType::SHORT4);
            vbb.normalized(VertexAttribute::TANGENTS);
            hasNormals = true;
            addBufferSlot({&mAsset->mGenerateTangents, atype, slot++});
            continue;
        }

        if (atype == cgltf_attribute_type_color) {
            hasVertexColor = true;
        }

        // Translate the cgltf attribute enum into a Filament enum.
        VertexAttribute semantic;
        if (!getVertexAttrType(atype, &semantic)) {
            utils::slog.e << "Unrecognized vertex semantic in " << name << utils::io::endl;
            return false;
        }
        if (atype == cgltf_attribute_type_weights && index > 0) {
            utils::slog.e << "Too many bone weights in " << name << utils::io::endl;
            continue;
        }
        if (atype == cgltf_attribute_type_joints && index > 0) {
            utils::slog.e << "Too many joints in " << name << utils::io::endl;
            continue;
        }
        if (atype == cgltf_attribute_type_texcoord) {
            if (index >= UvMapSize) {
                utils::slog.e << "Too many texture coordinate sets in " << name << utils::io::endl;
                continue;
            }
            UvSet uvset = outPrim->uvmap[index];
            switch (uvset) {
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

        vertexCount = accessor->count;

        // The positions accessor is required to have min/max properties, use them to expand
        // the bounding box for this primitive.
        if (atype == cgltf_attribute_type_position) {
            const float* minp = &accessor->min[0];
            const float* maxp = &accessor->max[0];
            outPrim->aabb.min = min(outPrim->aabb.min, float3(minp[0], minp[1], minp[2]));
            outPrim->aabb.max = max(outPrim->aabb.max, float3(maxp[0], maxp[1], maxp[2]));
        }

        VertexBuffer::AttributeType fatype;
        VertexBuffer::AttributeType actualType;
        if (!getElementType(accessor->type, accessor->component_type, &fatype, &actualType)) {
            slog.e << "Unsupported accessor type in " << name << io::endl;
            return false;
        }
        const int stride = (fatype == actualType) ? accessor->stride : 0;

        // The cgltf library provides a stride value for all accessors, even though they do not
        // exist in the glTF file. It is computed from the type and the stride of the buffer view.
        // As a convenience, cgltf also replaces zero (default) stride with the actual stride.
        vbb.attribute(semantic, slot, fatype, 0, stride);
        vbb.normalized(semantic, accessor->normalized);
        addBufferSlot({accessor, atype, slot++});
    }

    // If the model is lit but does not have normals, we'll need to generate flat normals.
    if (requiredAttributes.test(VertexAttribute::TANGENTS) && !hasNormals) {
        vbb.attribute(VertexAttribute::TANGENTS, slot, VertexBuffer::AttributeType::SHORT4);
        vbb.normalized(VertexAttribute::TANGENTS);
        cgltf_attribute_type atype = cgltf_attribute_type_normal;
        addBufferSlot({&mAsset->mGenerateNormals, atype, slot++});
    }

    cgltf_size targetsCount = inPrim.targets_count;

    if (targetsCount > MAX_MORPH_TARGETS) {
        utils::slog.w << "WARNING: Exceeded max morph target count of "
                << MAX_MORPH_TARGETS << utils::io::endl;
        targetsCount = MAX_MORPH_TARGETS;
    }

    const Aabb baseAabb(outPrim->aabb);
    for (cgltf_size targetIndex = 0; targetIndex < targetsCount; targetIndex++) {
        const cgltf_morph_target& morphTarget = inPrim.targets[targetIndex];
        for (cgltf_size aindex = 0; aindex < morphTarget.attributes_count; aindex++) {
            const cgltf_attribute& attribute = morphTarget.attributes[aindex];
            const cgltf_accessor* accessor = attribute.data;
            const cgltf_attribute_type atype = attribute.type;

            // The glTF normal and tangent data are ignored here, but honored in ResourceLoader.
            if (atype == cgltf_attribute_type_normal || atype == cgltf_attribute_type_tangent) {
                continue;
            }

            if (atype != cgltf_attribute_type_position) {
                utils::slog.e << "Only positions, normals, and tangents can be morphed."
                        << utils::io::endl;
                return false;
            }

            if (!accessor->has_min || !accessor->has_max) {
                continue;
            }

            Aabb targetAabb(baseAabb);
            const float* minp = &accessor->min[0];
            const float* maxp = &accessor->max[0];

            // We assume that the range of morph target weight is [0, 1].
            targetAabb.min += float3(minp[0], minp[1], minp[2]);
            targetAabb.max += float3(maxp[0], maxp[1], maxp[2]);

            outPrim->aabb.min = min(outPrim->aabb.min, targetAabb.min);
            outPrim->aabb.max = max(outPrim->aabb.max, targetAabb.max);

            VertexBuffer::AttributeType fatype;
            VertexBuffer::AttributeType actualType;
            if (!getElementType(accessor->type, accessor->component_type, &fatype, &actualType)) {
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

    if (mMaterials.needsDummyData(VertexAttribute::COLOR) && !hasVertexColor) {
        needsDummyData = true;
        vbb.attribute(VertexAttribute::COLOR, slot, VertexBuffer::AttributeType::UBYTE4);
        vbb.normalized(VertexAttribute::COLOR);
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
    mAsset->mPrimitives.push_back({&inPrim, vertices});
    mAsset->mVertexBuffers.push_back(vertices);

    for (size_t i = firstSlot; i < mAsset->mBufferSlots.size(); ++i) {
        mAsset->mBufferSlots[i].vertexBuffer = vertices;
    }

    if (targetsCount > 0) {
        MorphTargetBuffer* targets = MorphTargetBuffer::Builder()
                .vertexCount(vertexCount)
                .count(targetsCount)
                .build(mEngine);
        outPrim->targets = targets;
        mAsset->mMorphTargetBuffers.push_back(targets);
        const cgltf_accessor* previous = nullptr;
        for (int tindex = 0; tindex < targetsCount; ++tindex) {
            const cgltf_morph_target& inTarget = inPrim.targets[tindex];
            for (cgltf_size aindex = 0; aindex < inTarget.attributes_count; ++aindex) {
                const cgltf_attribute& attribute = inTarget.attributes[aindex];
                const cgltf_accessor* accessor = attribute.data;
                const cgltf_attribute_type atype = attribute.type;
                if (atype == cgltf_attribute_type_position) {
                    // All position attributes must have the same number of components.
                    assert_invariant(!previous || previous->type == accessor->type);
                    previous = accessor;
                    BufferSlot slot = { accessor };
                    slot.morphTargetBuffer = targets;
                    slot.bufferIndex = tindex;
                    addBufferSlot(slot);
                    break;
                }
            }
        }
    }

    if (needsDummyData) {
        const uint32_t requiredSize = sizeof(ubyte4) * vertexCount;
        if (mDummyBufferObject == nullptr || requiredSize > mDummyBufferObject->getByteCount()) {
            mDummyBufferObject = BufferObject::Builder().size(requiredSize).build(mEngine);
            mAsset->mBufferObjects.push_back(mDummyBufferObject);
            uint32_t* dummyData = (uint32_t*) malloc(requiredSize);
            memset(dummyData, 0xff, requiredSize);
            VertexBuffer::BufferDescriptor bd(dummyData, requiredSize, FREE_CALLBACK);
            mDummyBufferObject->setBuffer(mEngine, std::move(bd));
        }
        vertices->setBufferObjectAt(mEngine, slot, mDummyBufferObject);
    }

    return true;
}

void FAssetLoader::createLight(const cgltf_light* light, Entity entity) {
    LightManager::Type type = getLightType(light->type);
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
        // Use 10.0f units as a resonable default falloff value.
        builder.falloff(10.0f);
    } else {
        builder.falloff(light->range);
    }

    builder.build(mEngine, entity);
    mAsset->mLightEntities.push_back(entity);
}

void FAssetLoader::createCamera(const cgltf_camera* camera, Entity entity) {
    Camera* filamentCamera = mEngine.createCamera(entity);

    if (camera->type == cgltf_camera_type_perspective) {
        auto& projection = camera->data.perspective;

        const cgltf_float yfovDegrees = 180.0 / F_PI * projection.yfov;

        // Use an "infinite" zfar plane if the provided one is missing (set to 0.0).
        const double far = projection.zfar > 0.0 ? projection.zfar : 100000000;

        filamentCamera->setProjection(yfovDegrees, 1.0,
                projection.znear, far,
                filament::Camera::Fov::VERTICAL);

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

    mAsset->mCameraEntities.push_back(entity);
}

MaterialKey FAssetLoader::getMaterialKey(const cgltf_data* srcAsset,
        const cgltf_material* inputMat, UvMap* uvmap, bool vertexColor,
        cgltf_texture_view* baseColorTexture, cgltf_texture_view* metallicRoughnessTexture) const {
    auto mrConfig = inputMat->pbr_metallic_roughness;
    auto sgConfig = inputMat->pbr_specular_glossiness;
    auto ccConfig = inputMat->clearcoat;
    auto trConfig = inputMat->transmission;
    auto shConfig = inputMat->sheen;
    auto vlConfig = inputMat->volume;
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
        trConfig.transmission_texture.has_transform;

    MaterialKey matkey {
        .doubleSided = !!inputMat->double_sided,
        .unlit = !!inputMat->unlit,
        .hasVertexColors = vertexColor,
        .hasBaseColorTexture = baseColorTexture->texture != nullptr,
        .hasNormalTexture = inputMat->normal_texture.texture != nullptr,
        .hasOcclusionTexture = inputMat->occlusion_texture.texture != nullptr,
        .hasEmissiveTexture = inputMat->emissive_texture.texture != nullptr,
        .enableDiagnostics = mDiagnosticsEnabled,
        .baseColorUV = (uint8_t) baseColorTexture->texcoord,
        .hasClearCoatTexture = ccConfig.clearcoat_texture.texture != nullptr,
        .clearCoatUV = (uint8_t) ccConfig.clearcoat_texture.texcoord,
        .hasClearCoatRoughnessTexture = ccConfig.clearcoat_roughness_texture.texture != nullptr,
        .clearCoatRoughnessUV = (uint8_t) ccConfig.clearcoat_roughness_texture.texcoord,
        .hasClearCoatNormalTexture = ccConfig.clearcoat_normal_texture.texture != nullptr,
        .clearCoatNormalUV = (uint8_t) ccConfig.clearcoat_normal_texture.texcoord,
        .hasClearCoat = !!inputMat->has_clearcoat,
        .hasTransmission = !!inputMat->has_transmission,
        .hasTextureTransforms = hasTextureTransforms,
        .emissiveUV = (uint8_t) inputMat->emissive_texture.texcoord,
        .aoUV = (uint8_t) inputMat->occlusion_texture.texcoord,
        .normalUV = (uint8_t) inputMat->normal_texture.texcoord,
        .hasTransmissionTexture = trConfig.transmission_texture.texture != nullptr,
        .transmissionUV = (uint8_t) trConfig.transmission_texture.texcoord,
        .hasSheenColorTexture = shConfig.sheen_color_texture.texture != nullptr,
        .sheenColorUV = (uint8_t) shConfig.sheen_color_texture.texcoord,
        .hasSheenRoughnessTexture = shConfig.sheen_roughness_texture.texture != nullptr,
        .sheenRoughnessUV = (uint8_t) shConfig.sheen_roughness_texture.texcoord,
        .hasVolumeThicknessTexture = vlConfig.thickness_texture.texture != nullptr,
        .volumeThicknessUV = (uint8_t) vlConfig.thickness_texture.texcoord,
        .hasSheen = !!inputMat->has_sheen,
        .hasIOR = !!inputMat->has_ior,
        .hasVolume = !!inputMat->has_volume,
    };

    if (inputMat->has_pbr_specular_glossiness) {
        matkey.useSpecularGlossiness = true;
        if (sgConfig.diffuse_texture.texture) {
            *baseColorTexture = sgConfig.diffuse_texture;
            matkey.hasBaseColorTexture = true;
            matkey.baseColorUV = (uint8_t) baseColorTexture->texcoord;
        }
        if (sgConfig.specular_glossiness_texture.texture) {
            *metallicRoughnessTexture = sgConfig.specular_glossiness_texture;
            matkey.hasSpecularGlossinessTexture = true;
            matkey.specularGlossinessUV = (uint8_t) metallicRoughnessTexture->texcoord;
        }
    } else {
        matkey.hasMetallicRoughnessTexture = metallicRoughnessTexture->texture != nullptr;
        matkey.metallicRoughnessUV = (uint8_t) metallicRoughnessTexture->texcoord;
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

Material* FAssetLoader::getMaterial(const cgltf_data* srcAsset,
        const cgltf_material* inputMat, UvMap* uvmap, bool vertexColor) {
    cgltf_texture_view baseColorTexture;
    cgltf_texture_view metallicRoughnessTexture;
    if (UTILS_UNLIKELY(inputMat == nullptr)) {
        inputMat = &kDefaultMat;
    }
    MaterialKey matkey = getMaterialKey(srcAsset, inputMat, uvmap, vertexColor,
            &baseColorTexture, &metallicRoughnessTexture);
    const char* label = inputMat->name ? inputMat->name : "material";
    Material* material = mMaterials.getMaterial(&matkey, uvmap, label);
    assert_invariant(material);
    return material;
}

MaterialInstance* FAssetLoader::createMaterialInstance(const cgltf_data* srcAsset,
        const cgltf_material* inputMat, UvMap* uvmap, bool vertexColor) {
    MaterialInstanceCache::Entry* const cacheEntry =
            mMaterialInstanceCache.getEntry(&inputMat, vertexColor);
    if (cacheEntry->instance) {
        *uvmap = cacheEntry->uvmap;
        return cacheEntry->instance;
    }

    cgltf_texture_view baseColorTexture;
    cgltf_texture_view metallicRoughnessTexture;
    MaterialKey matkey = getMaterialKey(srcAsset, inputMat, uvmap, vertexColor, &baseColorTexture,
            &metallicRoughnessTexture);

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
    auto shConfig = inputMat->sheen;
    auto vlConfig = inputMat->volume;

    // Check the material blending mode, not the cgltf blending mode, because the provider
    // might have selected an alternative blend mode (e.g. to support transmission).
    if (mi->getMaterial()->getBlendingMode() == filament::BlendingMode::MASKED) {
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

    const TextureProvider::TextureFlags sRGB = TextureProvider::TextureFlags::sRGB;
    const TextureProvider::TextureFlags LINEAR = TextureProvider::TextureFlags::NONE;

    if (matkey.hasBaseColorTexture) {
        mAsset->addTextureBinding(mi, "baseColorMap", baseColorTexture.texture, sRGB);
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
        mAsset->addTextureBinding(mi, "metallicRoughnessMap", metallicRoughnessTexture.texture, srgb);
        if (matkey.hasTextureTransforms) {
            const cgltf_texture_transform& uvt = metallicRoughnessTexture.transform;
            auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
            mi->setParameter("metallicRoughnessUvMatrix", uvmat);
        }
    }

    if (matkey.hasNormalTexture) {
        mAsset->addTextureBinding(mi, "normalMap", inputMat->normal_texture.texture, LINEAR);
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
        mAsset->addTextureBinding(mi, "occlusionMap", inputMat->occlusion_texture.texture, LINEAR);
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
        mAsset->addTextureBinding(mi, "emissiveMap", inputMat->emissive_texture.texture, sRGB);
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
            mAsset->addTextureBinding(mi, "clearCoatMap", ccConfig.clearcoat_texture.texture,
                    LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = ccConfig.clearcoat_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("clearCoatUvMatrix", uvmat);
            }
        }
        if (matkey.hasClearCoatRoughnessTexture) {
            mAsset->addTextureBinding(mi, "clearCoatRoughnessMap",
                    ccConfig.clearcoat_roughness_texture.texture, LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = ccConfig.clearcoat_roughness_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("clearCoatRoughnessUvMatrix", uvmat);
            }
        }
        if (matkey.hasClearCoatNormalTexture) {
            mAsset->addTextureBinding(mi, "clearCoatNormalMap",
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
            mAsset->addTextureBinding(mi, "sheenColorMap", shConfig.sheen_color_texture.texture,
                    sRGB);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = shConfig.sheen_color_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("sheenColorUvMatrix", uvmat);
            }
        }
        if (matkey.hasSheenRoughnessTexture) {
            mAsset->addTextureBinding(mi, "sheenRoughnessMap",
                    shConfig.sheen_roughness_texture.texture, LINEAR);
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
            mAsset->addTextureBinding(mi, "volumeThicknessMap", vlConfig.thickness_texture.texture,
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
            mAsset->addTextureBinding(mi, "transmissionMap", trConfig.transmission_texture.texture,
                    LINEAR);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = trConfig.transmission_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("transmissionUvMatrix", uvmat);
            }
        }
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

    *cacheEntry = { mi, *uvmap };
    return mi;
}

void FAssetLoader::importSkins(FFilamentAsset* primary,
        FFilamentInstance* instance, const cgltf_data* gltf) {
    instance->mSkins.reserve(gltf->skins_count);
    instance->mSkins.resize(gltf->skins_count);
    const auto& nodeMap = instance->mNodeMap;
    for (cgltf_size i = 0, len = gltf->nodes_count; i < len; ++i) {
        const cgltf_node& node = gltf->nodes[i];
        Entity entity = nodeMap[i];
        if (node.skin && entity) {
            int skinIndex = node.skin - &gltf->skins[0];
            instance->mSkins[skinIndex].targets.insert(entity);
        }
    }
    for (cgltf_size i = 0, len = gltf->skins_count; i < len; ++i) {
        FFilamentInstance::Skin& dstSkin = instance->mSkins[i];
        const cgltf_skin& srcSkin = gltf->skins[i];

        // Build a list of transformables for this skin, one for each joint.
        dstSkin.joints = FixedCapacityVector<Entity>(srcSkin.joints_count);
        for (cgltf_size i = 0, len = srcSkin.joints_count; i < len; ++i) {
            dstSkin.joints[i] = nodeMap[srcSkin.joints[i] - gltf->nodes];
        }
    }
}

AssetLoader* AssetLoader::create(const AssetConfiguration& config) {
    return new FAssetLoader(config);
}

void AssetLoader::destroy(AssetLoader** loader) {
    FAssetLoader* temp(downcast(*loader));
    FAssetLoader::destroy(&temp);
    *loader = temp;
}

FilamentAsset* AssetLoader::createAsset(uint8_t const* bytes, uint32_t nbytes) {
    return downcast(this)->createAsset(bytes, nbytes);
}

FilamentAsset* AssetLoader::createInstancedAsset(const uint8_t* bytes, uint32_t numBytes,
        FilamentInstance** instances, size_t numInstances) {
    return downcast(this)->createInstancedAsset(bytes, numBytes, instances, numInstances);
}

FilamentInstance* AssetLoader::createInstance(FilamentAsset* asset) {
    return downcast(this)->createInstance(downcast(asset));
}

void AssetLoader::enableDiagnostics(bool enable) {
    downcast(this)->mDiagnosticsEnabled = enable;
}

void AssetLoader::destroyAsset(const FilamentAsset* asset) {
    downcast(this)->destroyAsset(downcast(asset));
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
