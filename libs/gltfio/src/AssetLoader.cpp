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

#include "FFilamentAsset.h"
#include "GltfEnums.h"

#include <filament/Box.h>
#include <filament/BufferObject.h>
#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>

#include <math/mat4.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/EntityManager.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/NameComponentManager.h>
#include <utils/Systrace.h>

#include <tsl/robin_map.h>

#include <vector>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "math.h"
#include "upcast.h"

using namespace filament;
using namespace filament::math;
using namespace utils;

namespace gltfio {

void importSkins(const cgltf_data* gltf, const NodeMap& nodeMap, SkinVector& dstSkins);

static const auto FREE_CALLBACK = [](void* mem, size_t, void*) { free(mem); };

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

struct FAssetLoader : public AssetLoader {
    FAssetLoader(const AssetConfiguration& config) :
            mEntityManager(config.entities ? *config.entities : EntityManager::get()),
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

    NameComponentManager* getNames() const noexcept {
        return mNameManager;
    }

    const Material* const* getMaterials() const noexcept {
        return mMaterials->getMaterials();
    }

    void createAsset(const cgltf_data* srcAsset, size_t numInstances);
    FFilamentInstance* createInstance(FFilamentAsset* primary, const cgltf_scene* scene);
    void createEntity(const cgltf_node* node, Entity parent, bool enableLight,
            FFilamentInstance* instance);
    void createRenderable(const cgltf_node* node, Entity entity, const char* name);
    bool createPrimitive(const cgltf_primitive* inPrim, Primitive* outPrim, const UvMap& uvmap,
            const char* name);
    void createLight(const cgltf_light* light, Entity entity);
    void createCamera(const cgltf_camera* camera, Entity entity);
    MaterialInstance* createMaterialInstance(const cgltf_material* inputMat, UvMap* uvmap,
            bool vertexColor);
    void addTextureBinding(MaterialInstance* materialInstance, const char* parameterName,
            const cgltf_texture* srcTexture, bool srgb);
    bool primitiveHasVertexColor(const cgltf_primitive* inPrim) const;

    static LightManager::Type getLightType(const cgltf_light_type type);

    EntityManager& mEntityManager;
    RenderableManager& mRenderableManager;
    NameComponentManager* mNameManager;
    TransformManager& mTransformManager;
    MaterialProvider* mMaterials;
    Engine* mEngine;

    // Transient state used only for the asset currently being loaded:
    FFilamentAsset* mResult;
    const char* mDefaultNodeName;
    bool mError = false;
    bool mDiagnosticsEnabled = false;
};

FILAMENT_UPCAST(AssetLoader)

FFilamentAsset* FAssetLoader::createAssetFromJson(const uint8_t* bytes, uint32_t nbytes) {
    cgltf_options options { cgltf_file_type_invalid };
    cgltf_data* sourceAsset;
    cgltf_result result = cgltf_parse(&options, bytes, nbytes, &sourceAsset);
    if (result != cgltf_result_success) {
        slog.e << "Unable to parse JSON file." << io::endl;
        return nullptr;
    }
    createAsset(sourceAsset, 0);
    return mResult;
}

FFilamentAsset* FAssetLoader::createAssetFromBinary(const uint8_t* bytes, uint32_t nbytes) {

    // The cgltf library handles GLB efficiently by pointing all buffer views into the source data.
    // However, we wish our API to be simple and safe, allowing clients to free up their source blob
    // immediately, without worrying about when all the data has finished uploading asynchronously
    // to the GPU. To achieve this we create a copy of the source blob and stash it inside the
    // asset, asking cgltf to parse the copy. This allows us to free it at the correct time (i.e.
    // after all GPU uploads have completed). Although it incurs a copy, the added safety of this
    // API seems worthwhile.
    std::vector<uint8_t> glbdata(bytes, bytes + nbytes);

    cgltf_options options { cgltf_file_type_glb };
    cgltf_data* sourceAsset;
    cgltf_result result = cgltf_parse(&options, glbdata.data(), nbytes, &sourceAsset);
    if (result != cgltf_result_success) {
        slog.e << "Unable to parse glb file." << io::endl;
        return nullptr;
    }
    createAsset(sourceAsset, 0);
    if (mResult) {
        glbdata.swap(mResult->mSourceAsset->glbData);
    }
    return mResult;
}

FFilamentAsset* FAssetLoader::createInstancedAsset(const uint8_t* bytes, uint32_t numBytes,
        FilamentInstance** instances, size_t numInstances) {
    ASSERT_PRECONDITION(numInstances > 0, "Instance count must be 1 or more.");

    // This method can be used to load JSON or GLB. By using a default options struct, we are asking
    // cgltf to examine the magic identifier to determine which type of file is being loaded.
    cgltf_options options {};

    // Clients can free up their source blob immediately, but cgltf has pointers into the data that
    // need to stay valid. Therefore we create a copy of the source blob and stash it inside the
    // asset.
    std::vector<uint8_t> glbdata(bytes, bytes + numBytes);

    cgltf_data* sourceAsset;
    cgltf_result result = cgltf_parse(&options, glbdata.data(), numBytes, &sourceAsset);
    if (result != cgltf_result_success) {
        slog.e << "Unable to parse glTF file." << io::endl;
        return nullptr;
    }
    createAsset(sourceAsset, numInstances);
    if (mResult) {
        glbdata.swap(mResult->mSourceAsset->glbData);
        std::copy_n(mResult->mInstances.data(), numInstances, instances);
    }
    return mResult;
}

FilamentInstance* FAssetLoader::createInstance(FFilamentAsset* primary) {
    if (!primary->mSourceAsset) {
        slog.e << "Source data has been released; asset is frozen." << io::endl;
        return nullptr;
    }
    if (!primary->isInstanced()) {
        slog.e << "Cannot add an instance to a non-instanced asset." << io::endl;
        return nullptr;
    }
    const cgltf_data* srcAsset = primary->mSourceAsset->hierarchy;
    const cgltf_scene* scene = srcAsset->scene ? srcAsset->scene : srcAsset->scenes;
    if (!scene) {
        slog.e << "There is no scene in the asset." << io::endl;
        return nullptr;
    }
    FFilamentInstance* instance = createInstance(primary, scene);

    // Import the skin data. This is normally done by ResourceLoader but dynamically created
    // instances are a bit special.
    importSkins(primary->mSourceAsset->hierarchy, instance->nodeMap, instance->skins);
    if (primary->mAnimator) {
        primary->mAnimator->addInstance(instance);
    }

    primary->mDependencyGraph.refinalize();
    return instance;
}

void FAssetLoader::createAsset(const cgltf_data* srcAsset, size_t numInstances) {
    SYSTRACE_CALL();
    #if !GLTFIO_DRACO_SUPPORTED
    for (cgltf_size i = 0; i < srcAsset->extensions_required_count; i++) {
        if (!strcmp(srcAsset->extensions_required[i], "KHR_draco_mesh_compression")) {
            slog.e << "KHR_draco_mesh_compression is not supported." << io::endl;
            mResult = nullptr;
            return;
        }
    }
    #endif

    mResult = new FFilamentAsset(mEngine, mNameManager, &mEntityManager, srcAsset);

    // If there is no default scene specified, then the default is the first one.
    // It is not an error for a glTF file to have zero scenes.
    const cgltf_scene* scene = srcAsset->scene ? srcAsset->scene : srcAsset->scenes;
    if (!scene) {
        return;
    }

    // Create a single root node with an identity transform as a convenience to the client.
    mResult->mRoot = mEntityManager.create();
    mTransformManager.create(mResult->mRoot);

    if (numInstances == 0) {
        // For each scene root, recursively create all entities.
        for (cgltf_size i = 0, len = scene->nodes_count; i < len; ++i) {
            cgltf_node** nodes = scene->nodes;
            createEntity(nodes[i], mResult->mRoot, true, nullptr);
        }
    } else {
        // Create a separate entity hierarchy for each instance. Note that MeshCache (vertex
        // buffers and index buffers) and MatInstanceCache (materials and textures) help avoid
        // needless duplication of resources.
        for (size_t index = 0; index < numInstances; ++index) {
            if (createInstance(mResult, scene) == nullptr) {
                mError = true;
                break;
            }
        }
    }

    // Find every unique resource URI and store a pointer to any of the cgltf-owned cstrings
    // that match the URI. These strings get freed during releaseSourceData().
    tsl::robin_map<std::string, const char*> resourceUris;
    auto addResourceUri = [&resourceUris](const char* uri) {
        if (uri) {
            resourceUris[uri] = uri;
        }
    };
    for (cgltf_size i = 0, len = srcAsset->buffers_count; i < len; ++i) {
        addResourceUri(srcAsset->buffers[i].uri);
    }
    for (cgltf_size i = 0, len = srcAsset->images_count; i < len; ++i) {
        addResourceUri(srcAsset->images[i].uri);
    }
    mResult->mResourceUris.reserve(resourceUris.size());
    for (auto pair : resourceUris) {
        mResult->mResourceUris.push_back(pair.second);
    }

    if (mError) {
        destroyAsset(mResult);
        mResult = nullptr;
        mError = false;
    }
}

FFilamentInstance* FAssetLoader::createInstance(FFilamentAsset* primary, const cgltf_scene* scene) {
    auto rootTransform = mTransformManager.getInstance(primary->mRoot);
    Entity instanceRoot = mEntityManager.create();
    mTransformManager.create(instanceRoot, rootTransform);

    // Create an instance object, which is a just a lightweight wrapper around a vector of
    // entities and a lazily created animator.
    FFilamentInstance* instance = new FFilamentInstance;
    instance->root = instanceRoot;
    instance->animator = nullptr;
    instance->owner = primary;
    primary->mInstances.push_back(instance);

    // For each scene root, recursively create all entities.
    for (cgltf_size i = 0, len = scene->nodes_count; i < len; ++i) {
        cgltf_node** nodes = scene->nodes;
        createEntity(nodes[i], instanceRoot, false, instance);
    }
    return instance;
}

void FAssetLoader::createEntity(const cgltf_node* node, Entity parent, bool enableLight,
        FFilamentInstance* instance) {
    Entity entity = mEntityManager.create();

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

    // Update the asset's entity list and private node mapping.
    mResult->mEntities.push_back(entity);
    if (instance) {
        instance->entities.push_back(entity);
        instance->nodeMap[node] = entity;
    } else {
        mResult->mNodeMap[node] = entity;
    }

    const char* name = getNodeName(node, mDefaultNodeName);

    if (name) {
        mResult->mNameToEntity[name].push_back(entity);
        if (mNameManager) {
            mNameManager->addComponent(entity);
            mNameManager->setName(mNameManager->getInstance(entity), name);
        }
    }

    // If no name is provided in the glTF or AssetConfiguration, use "node" for error messages.
    name = name ? name : "node";

    // If the node has a mesh, then create a renderable component.
    if (node->mesh) {
        createRenderable(node, entity, name);
    }

    if (node->light && enableLight) {
        createLight(node->light, entity);
    }

    if (node->camera) {
        createCamera(node->camera, entity);
    }

    for (cgltf_size i = 0, len = node->children_count; i < len; ++i) {
        createEntity(node->children[i], entity, enableLight, instance);
    }
}

void FAssetLoader::createRenderable(const cgltf_node* node, Entity entity, const char* name) {
    const cgltf_mesh* mesh = node->mesh;

    // Compute the transform relative to the root.
    auto thisTransform = mTransformManager.getInstance(entity);
    mat4f worldTransform = mTransformManager.getWorldTransform(thisTransform);

    cgltf_size nprims = mesh->primitives_count;
    RenderableManager::Builder builder(nprims);

    // If the mesh is already loaded, obtain the list of Filament VertexBuffer / IndexBuffer objects
    // that were already generated (one for each primitive), otherwise allocate a new list of
    // pointers for the primitives.
    auto iter = mResult->mMeshCache.find(mesh);
    if (iter == mResult->mMeshCache.end()) {
        mResult->mMeshCache[mesh].resize(nprims);
    }
    Primitive* outputPrim = mResult->mMeshCache[mesh].data();
    const cgltf_primitive* inputPrim = &mesh->primitives[0];

    Aabb aabb;

    cgltf_size numMorphTargets = 0;

    // For each prim, create a Filament VertexBuffer, IndexBuffer, and MaterialInstance.
    for (cgltf_size index = 0; index < nprims; ++index, ++outputPrim, ++inputPrim) {
        RenderableManager::PrimitiveType primType;
        if (!getPrimitiveType(inputPrim->type, &primType)) {
            slog.e << "Unsupported primitive type in " << name << io::endl;
        }

        if (inputPrim->targets_count > 0) {
            if (numMorphTargets > 0 && inputPrim->targets_count != numMorphTargets) {
                slog.e << "Sister primitives must all have the same number of morph targets."
                        << io::endl;
            }
            numMorphTargets = inputPrim->targets_count;
        }

        // Create a material instance for this primitive or fetch one from the cache.
        UvMap uvmap {};
        bool hasVertexColor = primitiveHasVertexColor(inputPrim);
        MaterialInstance* mi = createMaterialInstance(inputPrim->material, &uvmap, hasVertexColor);
        if (!mi) {
            mError = true;
            continue;
        }

        mResult->mDependencyGraph.addEdge(entity, mi);
        builder.material(index, mi);

        // Create a Filament VertexBuffer and IndexBuffer for this prim if we haven't already.
        if (!outputPrim->vertices && !createPrimitive(inputPrim, outputPrim, uvmap, name)) {
            mError = true;
            continue;
        }

        // Expand the object-space bounding box.
        aabb.min = min(outputPrim->aabb.min, aabb.min);
        aabb.max = max(outputPrim->aabb.max, aabb.max);

        // We are not using the optional offset, minIndex, maxIndex, and count arguments when
        // calling geometry() on the builder. It appears that the glTF spec does not have
        // facilities for these parameters, which is not a huge loss since some of the buffer
        // view and accessor features already have this functionality.
        builder.geometry(index, primType, outputPrim->vertices, outputPrim->indices);
    }

    if (numMorphTargets > 0) {
        builder.morphing(true);
    }

    const Aabb transformed = aabb.transform(worldTransform);

    // Expand the world-space bounding box.
    mResult->mBoundingBox.min = min(mResult->mBoundingBox.min, transformed.min);
    mResult->mBoundingBox.max = max(mResult->mBoundingBox.max, transformed.max);

    if (node->skin) {
       builder.skinning(node->skin->joints_count);
    }

    // Per the spec, glTF models must have valid mix / max annotations for position attributes.
    // However in practice these can be missing and we should be as robust as other glTF viewers.
    // If desired, clients can enable the "recomputeBoundingBoxes" feature in ResourceLoader.
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
        .build(*mEngine, entity);

    // According to the spec, the mesh may or may not specify default weights, regardless of whether
    // it actually has morph targets. If it has morphing enabled then the default weights are 0. If
    // node weights are provided, they override the ones specified on the mesh.
    if (numMorphTargets > 0) {
        RenderableManager::Instance renderable = mRenderableManager.getInstance(entity);
        float4 weights(0, 0, 0, 0);
        for (cgltf_size i = 0; i < std::min(MAX_MORPH_TARGETS, mesh->weights_count); ++i) {
            weights[i] = mesh->weights[i];
        }
        for (cgltf_size i = 0; i < std::min(MAX_MORPH_TARGETS, node->weights_count); ++i) {
            weights[i] = node->weights[i];
        }
        mRenderableManager.setMorphWeights(renderable, weights);
    }
}

bool FAssetLoader::createPrimitive(const cgltf_primitive* inPrim, Primitive* outPrim,
        const UvMap& uvmap, const char* name) {
    outPrim->uvmap = uvmap;

    // Create a little lambda that appends to the asset's vertex buffer slots.
    auto slots = &mResult->mBufferSlots;
    auto addBufferSlot = [slots](BufferSlot entry) {
        slots->push_back(entry);
    };

    // In glTF, each primitive may or may not have an index buffer.
    IndexBuffer* indices = nullptr;
    const cgltf_accessor* accessor = inPrim->indices;
    if (accessor) {
        IndexBuffer::IndexType indexType;
        if (!getIndexType(accessor->component_type, &indexType)) {
            utils::slog.e << "Unrecognized index type in " << name << utils::io::endl;
            return false;
        }

        indices = IndexBuffer::Builder()
            .indexCount(accessor->count)
            .bufferType(indexType)
            .build(*mEngine);

        BufferSlot slot = { accessor };
        slot.indexBuffer = indices;
        addBufferSlot(slot);
    } else if (inPrim->attributes_count > 0) {
        // If a primitive does not have an index buffer, generate a trivial one now.
        const uint32_t vertexCount = inPrim->attributes[0].data->count;

        indices = IndexBuffer::Builder()
            .indexCount(vertexCount)
            .bufferType(IndexBuffer::IndexType::UINT)
            .build(*mEngine);

        const size_t indexDataSize = vertexCount * sizeof(uint32_t);
        uint32_t* indexData = (uint32_t*) malloc(indexDataSize);
        for (size_t i = 0; i < vertexCount; ++i) {
            indexData[i] = i;
        }
        IndexBuffer::BufferDescriptor bd(indexData, indexDataSize, FREE_CALLBACK);
        indices->setBuffer(*mEngine, std::move(bd));
    }
    mResult->mIndexBuffers.push_back(indices);

    VertexBuffer::Builder vbb;
    vbb.enableBufferObjects();

    bool hasUv0 = false, hasUv1 = false, hasVertexColor = false, hasNormals = false;
    uint32_t vertexCount = 0;

    const size_t firstSlot = mResult->mBufferSlots.size();
    int slot = 0;

    for (cgltf_size aindex = 0; aindex < inPrim->attributes_count; aindex++) {
        const cgltf_attribute& attribute = inPrim->attributes[aindex];
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
            addBufferSlot({&mResult->mGenerateTangents, atype, slot++});
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
        if (atype == cgltf_attribute_type_texcoord) {
            if (index >= UvMapSize) {
                utils::slog.e << "Too many texture coordinate sets in " << name << utils::io::endl;
                continue;
            }
            UvSet uvset = uvmap[index];
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
                    if (!hasUv0 && getNumUvSets(uvmap) == 0) {
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
    if (inPrim->material && !inPrim->material->unlit && !hasNormals) {
        vbb.attribute(VertexAttribute::TANGENTS, slot, VertexBuffer::AttributeType::SHORT4);
        vbb.normalized(VertexAttribute::TANGENTS);
        cgltf_attribute_type atype = cgltf_attribute_type_normal;
        addBufferSlot({&mResult->mGenerateNormals, atype, slot++});
    }

    cgltf_size targetsCount = inPrim->targets_count;

    // There is no need to emit a warning if there are more than 4 targets. This is only the base
    // VertexBuffer and more might be created by MorphHelper.
    if (targetsCount > MAX_MORPH_TARGETS) {
        targetsCount = MAX_MORPH_TARGETS;
    }

    constexpr int baseTangentsAttr = (int) VertexAttribute::MORPH_TANGENTS_0;
    constexpr int basePositionAttr = (int) VertexAttribute::MORPH_POSITION_0;

    for (cgltf_size targetIndex = 0; targetIndex < targetsCount; targetIndex++) {
        const cgltf_morph_target& morphTarget = inPrim->targets[targetIndex];
        for (cgltf_size aindex = 0; aindex < morphTarget.attributes_count; aindex++) {
            const cgltf_attribute& attribute = morphTarget.attributes[aindex];
            const cgltf_accessor* accessor = attribute.data;
            const cgltf_attribute_type atype = attribute.type;
            const int morphId = targetIndex + 1;

            // The glTF tangent data is ignored here, but honored in ResourceLoader.
            if (atype == cgltf_attribute_type_tangent) {
                continue;
            }

            if (atype == cgltf_attribute_type_normal) {
                VertexAttribute attr = (VertexAttribute) (baseTangentsAttr + targetIndex);
                vbb.attribute(attr, slot, VertexBuffer::AttributeType::SHORT4);
                vbb.normalized(attr);
                outPrim->morphTangents[targetIndex] = slot;
                addBufferSlot({&mResult->mGenerateTangents, atype, slot++, morphId});
                continue;
            }

            if (atype != cgltf_attribute_type_position) {
                utils::slog.e << "Only positions, normals, and tangents can be morphed."
                        << utils::io::endl;
                return false;
            }

            const float* minp = &accessor->min[0];
            const float* maxp = &accessor->max[0];
            outPrim->aabb.min = min(outPrim->aabb.min, float3(minp[0], minp[1], minp[2]));
            outPrim->aabb.max = max(outPrim->aabb.max, float3(maxp[0], maxp[1], maxp[2]));

            VertexBuffer::AttributeType fatype;
            VertexBuffer::AttributeType actualType;
            if (!getElementType(accessor->type, accessor->component_type, &fatype, &actualType)) {
                slog.e << "Unsupported accessor type in " << name << io::endl;
                return false;
            }
            const int stride = (fatype == actualType) ? accessor->stride : 0;

            VertexAttribute attr = (VertexAttribute) (basePositionAttr + targetIndex);
            vbb.attribute(attr, slot, fatype, 0, stride);
            vbb.normalized(attr, accessor->normalized);
            outPrim->morphPositions[targetIndex] = slot;
            addBufferSlot({accessor, atype, slot++, morphId});
        }
    }

    if (vertexCount == 0) {
        slog.e << "Empty vertex buffer in " << name << io::endl;
        return false;
    }

    vbb.vertexCount(vertexCount);

    // If an ubershader is used, then we provide a single dummy buffer for all unfulfilled vertex
    // requirements. The color data should be a sequence of normalized UBYTE4, so dummy UVs are
    // USHORT2 to make the sizes match.
    bool needsDummyData = false;
    if (mMaterials->getSource() == LOAD_UBERSHADERS) {
        if (!hasUv0) {
            needsDummyData = true;
            vbb.attribute(VertexAttribute::UV0, slot, VertexBuffer::AttributeType::USHORT2);
            vbb.normalized(VertexAttribute::UV0);
        }
        if (!hasUv1) {
            needsDummyData = true;
            vbb.attribute(VertexAttribute::UV1, slot, VertexBuffer::AttributeType::USHORT2);
            vbb.normalized(VertexAttribute::UV1);
        }
        if (!hasVertexColor) {
            needsDummyData = true;
            vbb.attribute(VertexAttribute::COLOR, slot, VertexBuffer::AttributeType::UBYTE4);
            vbb.normalized(VertexAttribute::COLOR);
        }
    } else {
        int numUvSets = getNumUvSets(uvmap);
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
    }

    vbb.bufferCount(needsDummyData ? slot + 1 : slot);

    VertexBuffer* vertices = vbb.build(*mEngine);

    outPrim->indices = indices;
    outPrim->vertices = vertices;
    mResult->mPrimitives.push_back({inPrim, vertices});
    mResult->mVertexBuffers.push_back(vertices);

    for (size_t i = firstSlot; i < mResult->mBufferSlots.size(); ++i) {
        mResult->mBufferSlots[i].vertexBuffer = vertices;
    }

    if (needsDummyData) {
        uint32_t size = sizeof(ubyte4) * vertexCount;
        BufferObject* bufferObject = BufferObject::Builder().size(size).build(*mEngine);
        mResult->mBufferObjects.push_back(bufferObject);
        uint32_t* dummyData = (uint32_t*) malloc(size);
        memset(dummyData, 0xff, size);
        VertexBuffer::BufferDescriptor bd(dummyData, size, FREE_CALLBACK);
        bufferObject->setBuffer(*mEngine, std::move(bd));
        vertices->setBufferObjectAt(*mEngine, slot, bufferObject);
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

    builder.build(*mEngine, entity);
    mResult->mLightEntities.push_back(entity);
}

void FAssetLoader::createCamera(const cgltf_camera* camera, Entity entity) {
    Camera* filamentCamera = mEngine->createCamera(entity);

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

    mResult->mCameraEntities.push_back(entity);
}

MaterialInstance* FAssetLoader::createMaterialInstance(const cgltf_material* inputMat,
        UvMap* uvmap, bool vertexColor) {
    intptr_t key = ((intptr_t) inputMat) ^ (vertexColor ? 1 : 0);
    auto iter = mResult->mMatInstanceCache.find(key);
    if (iter != mResult->mMatInstanceCache.end()) {
        *uvmap = iter->second.uvmap;
        return iter->second.instance;
    }

    // The default glTF material.
    static const cgltf_material kDefaultMat = {
        .name = (char*) "Default GLTF material",
        .has_pbr_metallic_roughness = true,
        .has_pbr_specular_glossiness = false,
        .has_clearcoat = false,
        .has_transmission = false,
        .has_ior = false,
        .has_specular = false,
        .has_sheen = false,
        .pbr_metallic_roughness = {
	        .base_color_factor = {1.0, 1.0, 1.0, 1.0},
	        .metallic_factor = 1.0,
	        .roughness_factor = 1.0,
        },
    };
    inputMat = inputMat ? inputMat : &kDefaultMat;

    auto mrConfig = inputMat->pbr_metallic_roughness;
    auto sgConfig = inputMat->pbr_specular_glossiness;
    auto ccConfig = inputMat->clearcoat;
    auto trConfig = inputMat->transmission;
    auto shConfig = inputMat->sheen;

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

    cgltf_texture_view baseColorTexture = mrConfig.base_color_texture;
    cgltf_texture_view metallicRoughnessTexture = mrConfig.metallic_roughness_texture;

    MaterialKey matkey {
        .doubleSided = !!inputMat->double_sided,
        .unlit = !!inputMat->unlit,
        .hasVertexColors = vertexColor,
        .hasBaseColorTexture = baseColorTexture.texture != nullptr,
        .hasNormalTexture = inputMat->normal_texture.texture != nullptr,
        .hasOcclusionTexture = inputMat->occlusion_texture.texture != nullptr,
        .hasEmissiveTexture = inputMat->emissive_texture.texture != nullptr,
        .enableDiagnostics = mDiagnosticsEnabled,
        .baseColorUV = (uint8_t) baseColorTexture.texcoord,
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
        .hasSheen = !!inputMat->has_sheen,
        .hasIOR = !!inputMat->has_ior,
    };

    if (inputMat->has_pbr_specular_glossiness) {
        matkey.useSpecularGlossiness = true;
        if (sgConfig.diffuse_texture.texture) {
            baseColorTexture = sgConfig.diffuse_texture;
            matkey.hasBaseColorTexture = true;
            matkey.baseColorUV = (uint8_t) baseColorTexture.texcoord;
        }
        if (sgConfig.specular_glossiness_texture.texture) {
            metallicRoughnessTexture = sgConfig.specular_glossiness_texture;
            matkey.hasSpecularGlossinessTexture = true;
            matkey.specularGlossinessUV = (uint8_t) metallicRoughnessTexture.texcoord;
        }
    } else {
        matkey.hasMetallicRoughnessTexture = metallicRoughnessTexture.texture != nullptr;
        matkey.metallicRoughnessUV = (uint8_t) metallicRoughnessTexture.texcoord;
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
    }

    // This not only creates a material instance, it modifies the material key according to our
    // rendering constraints. For example, Filament only supports 2 sets of texture coordinates.
    MaterialInstance* mi = mMaterials->createMaterialInstance(&matkey, uvmap, inputMat->name);
    if (!mi) {
        slog.e << "No material with the specified requirements exists." << io::endl;
        return nullptr;
    }

    mResult->mMaterialInstances.push_back(mi);

    // Check the material blending mode, not the cgltf blending mode, because the provider
    // might have selected an alternative blend mode (e.g. to support transmission).
    if (mi->getMaterial()->getBlendingMode() == filament::BlendingMode::MASKED) {
        mi->setMaskThreshold(inputMat->alpha_cutoff);
    }

    const float* e = inputMat->emissive_factor;
    mi->setParameter("emissiveFactor", float3(e[0], e[1], e[2]));

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

    if (matkey.hasBaseColorTexture) {
        addTextureBinding(mi, "baseColorMap", baseColorTexture.texture, true);
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
        bool srgb = inputMat->has_pbr_specular_glossiness;
        addTextureBinding(mi, "metallicRoughnessMap", metallicRoughnessTexture.texture, srgb);
        if (matkey.hasTextureTransforms) {
            const cgltf_texture_transform& uvt = metallicRoughnessTexture.transform;
            auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
            mi->setParameter("metallicRoughnessUvMatrix", uvmat);
        }
    }

    if (matkey.hasNormalTexture) {
        addTextureBinding(mi, "normalMap", inputMat->normal_texture.texture, false);
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
        addTextureBinding(mi, "occlusionMap", inputMat->occlusion_texture.texture, false);
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
        addTextureBinding(mi, "emissiveMap", inputMat->emissive_texture.texture, true);
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
            addTextureBinding(mi, "clearCoatMap", ccConfig.clearcoat_texture.texture, false);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = ccConfig.clearcoat_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("clearCoatUvMatrix", uvmat);
            }
        }
        if (matkey.hasClearCoatRoughnessTexture) {
            addTextureBinding(mi, "clearCoatRoughnessMap", ccConfig.clearcoat_roughness_texture.texture, false);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = ccConfig.clearcoat_roughness_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("clearCoatRoughnessUvMatrix", uvmat);
            }
        }
        if (matkey.hasClearCoatNormalTexture) {
            addTextureBinding(mi, "clearCoatNormalMap", ccConfig.clearcoat_normal_texture.texture, false);
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
            addTextureBinding(mi, "sheenColorMap", shConfig.sheen_color_texture.texture, true);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = shConfig.sheen_color_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("sheenColorUvMatrix", uvmat);
            }
        }
        if (matkey.hasSheenRoughnessTexture) {
            addTextureBinding(mi, "sheenRoughnessMap", shConfig.sheen_roughness_texture.texture, false);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = shConfig.sheen_roughness_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("sheenRoughnessUvMatrix", uvmat);
            }
        }
    }

    if (matkey.hasTransmission) {
        mi->setParameter("transmissionFactor", trConfig.transmission_factor);
        if (matkey.hasTransmissionTexture) {
            addTextureBinding(mi, "transmissionMap", trConfig.transmission_texture.texture, false);
            if (matkey.hasTextureTransforms) {
                const cgltf_texture_transform& uvt = trConfig.transmission_texture.transform;
                auto uvmat = matrixFromUvTransform(uvt.offset, uvt.rotation, uvt.scale);
                mi->setParameter("transmissionUvMatrix", uvmat);
            }
        }
    }

    if (matkey.hasIOR) {
        mi->setParameter("ior", inputMat->ior.ior);
    }

    mResult->mMatInstanceCache[key] = {mi, *uvmap};
    return mi;
}

void FAssetLoader::addTextureBinding(MaterialInstance* materialInstance, const char* parameterName,
        const cgltf_texture* srcTexture, bool srgb) {
    if (!srcTexture->image) {
        slog.w << "Texture is missing image (" << srcTexture->name << ")." << io::endl;
        return;
    }
    TextureSampler dstSampler;
    auto srcSampler = srcTexture->sampler;
    if (srcSampler) {
        dstSampler.setWrapModeS(getWrapMode(srcSampler->wrap_s));
        dstSampler.setWrapModeT(getWrapMode(srcSampler->wrap_t));
        dstSampler.setMagFilter(getMagFilter(srcSampler->mag_filter));
        dstSampler.setMinFilter(getMinFilter(srcSampler->min_filter));
    } else {
        // These defaults are stipulated by the spec:
        dstSampler.setWrapModeS(TextureSampler::WrapMode::REPEAT);
        dstSampler.setWrapModeT(TextureSampler::WrapMode::REPEAT);

        // These defaults are up the implementation but since we generate mipmaps unconditionally,
        // we might as well use them. In practice the conformance models look awful without
        // using mipmapping by default.
        dstSampler.setMagFilter(TextureSampler::MagFilter::LINEAR);
        dstSampler.setMinFilter(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR);
    }

    mResult->mTextureSlots.push_back({
        .texture = srcTexture,
        .materialInstance = materialInstance,
        .materialParameter = parameterName,
        .sampler = dstSampler,
        .srgb = srgb
    });
    mResult->mDependencyGraph.addEdge(materialInstance, parameterName);
}

bool FAssetLoader::primitiveHasVertexColor(const cgltf_primitive* inPrim) const {
    for (int slot = 0; slot < inPrim->attributes_count; slot++) {
        const cgltf_attribute& inputAttribute = inPrim->attributes[slot];
        if (inputAttribute.type == cgltf_attribute_type_color) {
            return true;
        }
    }
    return false;
}

LightManager::Type FAssetLoader::getLightType(const cgltf_light_type light) {
    switch (light) {
        case cgltf_light_type_invalid:
        case cgltf_light_type_directional:
            return LightManager::Type::DIRECTIONAL;
        case cgltf_light_type_point:
            return LightManager::Type::POINT;
        case cgltf_light_type_spot:
            return LightManager::Type::FOCUSED_SPOT;
    }
}

AssetLoader* AssetLoader::create(const AssetConfiguration& config) {
    return new FAssetLoader(config);
}

void AssetLoader::destroy(AssetLoader** loader) {
    delete *loader;
    *loader = nullptr;
}

FilamentAsset* AssetLoader::createAssetFromJson(uint8_t const* bytes, uint32_t nbytes) {
    return upcast(this)->createAssetFromJson(bytes, nbytes);
}

FilamentAsset* AssetLoader::createAssetFromBinary(uint8_t const* bytes, uint32_t nbytes) {
    return upcast(this)->createAssetFromBinary(bytes, nbytes);
}

FilamentAsset* AssetLoader::createInstancedAsset(const uint8_t* bytes, uint32_t numBytes,
        FilamentInstance** instances, size_t numInstances) {
    return upcast(this)->createInstancedAsset(bytes, numBytes, instances, numInstances);
}

FilamentInstance* AssetLoader::createInstance(FilamentAsset* asset) {
    return upcast(this)->createInstance(upcast(asset));
}

void AssetLoader::enableDiagnostics(bool enable) {
    upcast(this)->mDiagnosticsEnabled = enable;
}

void AssetLoader::destroyAsset(const FilamentAsset* asset) {
    upcast(this)->destroyAsset(upcast(asset));
}

size_t AssetLoader::getMaterialsCount() const noexcept {
    return upcast(this)->getMaterialsCount();
}

NameComponentManager* AssetLoader::getNames() const noexcept {
    return upcast(this)->getNames();
}

const Material* const* AssetLoader::getMaterials() const noexcept {
    return upcast(this)->getMaterials();
}

} // namespace gltfio
