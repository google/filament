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

#include <gltfio/AssetPipeline.h>

#include <xatlas.h>

#define CGLTF_WRITE_IMPLEMENTATION
#include "cgltf_write.h"

#include <utils/Log.h>

#include <math/mat4.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <memory>
#include <vector>

using namespace filament::math;
using namespace utils;

using std::vector;
using filament::rays::SimpleMesh;

const char* const gltfio::AssetPipeline::BAKED_UV_ATTRIB = "TEXCOORD_4";
const int gltfio::AssetPipeline::BAKED_UV_ATTRIB_INDEX = 4;
static const char* const PREVIEW_MATERIAL_NAME = "preview";

namespace {

using AssetHandle = gltfio::AssetPipeline::AssetHandle;

static const char* const POSITION = "POSITION";
static const char* const NORMAL = "NORMAL";
static const char* const TANGENT = "TANGENT";
static const char* const GENERATOR_ID = "gltfio";

// Bookkeeping structure for baking a single primitive + node pair.
struct BakedPrim {
    const cgltf_node* sourceNode;
    const cgltf_mesh* sourceMesh;
    const cgltf_primitive* sourcePrimitive;
    const cgltf_attribute* sourcePositions;
    const cgltf_attribute* sourceNormals;
    const cgltf_attribute* sourceTangents;
    float3* bakedPositions;
    float3* bakedNormals;
    float4* bakedTangents;
    uint32_t* bakedIndices;
    float3 bakedMin;
    float3 bakedMax;
};

// Utility class to help populate cgltf arrays and ensure that the memory is freed.
template <typename T>
class ArrayHolder {
    vector<std::unique_ptr<T[]> > mItems;
public:
    T* alloc(size_t count) {
        mItems.push_back(std::make_unique<T[]>(count));
        return mItems.back().get();
    }
};

// Private implementation for AssetPipeline.
class Pipeline {
public:
    using RenderOptions = gltfio::AssetPipeline::RenderOptions;

    // Aggregate all buffers into a single buffer.
    const cgltf_data* flattenBuffers(const cgltf_data* sourceAsset);

    // Bake transforms and make each primitive correspond to a single node.
    const cgltf_data* flattenPrims(const cgltf_data* sourceAsset, uint32_t flags);

    // Use xatlas to generate a new UV set and modify topology appropriately.
    const cgltf_data* parameterize(const cgltf_data* sourceAsset, int maxIterations);

    // Strips materials from a flattened asset and replaces them a simple nonlit material.
    const cgltf_data* generatePreview(const cgltf_data* sourceAsset, const Path& texturePath);

    // Replaces or adds the given occlusion texture to all primitives that have BAKED_UV_ATTRIB.
    const cgltf_data* replaceOcclusion(const cgltf_data* sourceAsset, const Path& texturePath);

    // Replaces the texture URI for all primitives that have BAKED_UV_ATTRIB.
    void setOcclusionUri(cgltf_data* asset, const Path& texturePath);

    // Replaces the baseColor URI for all materials.
    void setBaseColorUri(cgltf_data* asset, const Path& texturePath);

    // Take ownership of the given asset and free it when the pipeline is destroyed.
    void retainSourceAsset(cgltf_data* asset);

    // Perform a two-pass baking operation (generate gbuffer, then use gbuffer).
    void bakeAmbientOcclusion(const cgltf_data* sourceAsset, image::LinearImage target,
            const RenderOptions& options);

    // Perform a non-bake render for diagnostic purposes.
    void renderAmbientOcclusion(const cgltf_data* sourceAsset,
            image::LinearImage target, const filament::rays::SimpleCamera& camera,
            const RenderOptions& options);

    // Perform a two-pass baking operation and populate all channels. Useful for diagnostics.
    void bakeAllOutputs(const cgltf_data* sourceAsset,
            image::LinearImage targets[filament::rays::OUTPUTPLANE_COUNT],
            const RenderOptions& options);

    ~Pipeline();

private:
    void bakeTransform(BakedPrim* prim, const mat4f& transform, const mat3f& normalMatrix);
    void populateResult(const BakedPrim* prims, size_t numPrims, size_t numVerts);
    bool filterPrim(const cgltf_primitive& prim);

    bool cgltfToXatlas(const cgltf_data* sourceAsset, xatlas::Atlas* atlas);
    cgltf_data* xatlasToCgltf(const cgltf_data* source, const xatlas::Atlas* atlas);

    // Convert a flattened cgltf asset into a PathTracer scene (i.e. an array of SimpleMesh).
    void cgltfToSimpleMesh(const cgltf_data* sourceAsset, SimpleMesh** meshes, size_t* numMeshes,
            bool provideDummyTexCoords = false);

    uint32_t mFlattenFlags;
    vector<cgltf_data*> mSourceAssets;

    struct {
        ArrayHolder<cgltf_data> resultAssets;
        ArrayHolder<cgltf_scene> scenes;
        ArrayHolder<cgltf_node*> nodePointers;
        ArrayHolder<cgltf_node> nodes;
        ArrayHolder<cgltf_mesh> meshes;
        ArrayHolder<cgltf_primitive> prims;
        ArrayHolder<cgltf_attribute> attributes;
        ArrayHolder<cgltf_accessor> accessors;
        ArrayHolder<cgltf_buffer_view> views;
        ArrayHolder<cgltf_buffer> buffers;
        ArrayHolder<cgltf_image> images;
        ArrayHolder<cgltf_texture> textures;
        ArrayHolder<cgltf_material> materials;
        ArrayHolder<uint8_t> bufferData;
        ArrayHolder<SimpleMesh> simpleMeshes;
    } mStorage;
};

cgltf_size getNumFloats(cgltf_type type) {
    switch (type) {
        case cgltf_type_vec2: return 2;
        case cgltf_type_vec3: return 3;
        case cgltf_type_vec4: return 4;
        case cgltf_type_mat2: return 4;
        case cgltf_type_mat3: return 9;
        case cgltf_type_mat4: return 16;
        case cgltf_type_invalid:
        case cgltf_type_scalar:
        default: return 1;
    }
}

// Returns true if the given cgltf asset has been flattened by the mesh pipeline and is therefore
// amenable to subsequent pipeline operations like baking and exporting.
bool isFlattened(const cgltf_data* asset) {
    return asset && asset->buffers_count == 1 && asset->nodes_count == asset->meshes_count &&
            !strcmp(asset->asset.generator, GENERATOR_ID);
}

// Returns true if the given primitive should be baked out, false if it should be culled away.
bool Pipeline::filterPrim(const cgltf_primitive& prim) {
    const bool filterTriangles = mFlattenFlags & gltfio::AssetPipeline::FILTER_TRIANGLES;
    if (filterTriangles && prim.type != cgltf_primitive_type_triangles) {
        return false;
    }
    for (cgltf_size k = 0; k < prim.attributes_count; ++k) {
        const cgltf_attribute& attr = prim.attributes[k];
        if (!attr.data || !attr.data->count || attr.data->is_sparse) {
            return false;
        }
    }
    if (!prim.indices || prim.indices->is_sparse) {
        // TODO: generate trivial indices
        return false;
    }
    return true;
}

const cgltf_data* Pipeline::flattenBuffers(const cgltf_data* sourceAsset) {
    // Determine the total required size for the aggregated buffer.
    size_t totalSize = 0;
    for (size_t i = 0, len = sourceAsset->buffers_count; i < len; ++i) {
        totalSize += sourceAsset->buffers[i].size;
    }

    // Count the total number of attributes and primitives.
    size_t attribsCount = 0;
    size_t primsCount = 0;
    for (size_t i = 0, len = sourceAsset->meshes_count; i < len; ++i) {
        const auto& mesh = sourceAsset->meshes[i];
        primsCount += mesh.primitives_count;
        for (size_t j = 0; j < mesh.primitives_count; ++j) {
            attribsCount += mesh.primitives[j].attributes_count;
        }
    }

    // Count the total number of referenced nodes.
    size_t nodePointersCount = 0;
    for (size_t i = 0, len = sourceAsset->scenes_count; i < len; ++i) {
        const auto& scene = sourceAsset->scenes[i];
        nodePointersCount += scene.nodes_count;
    }

    // Allocate inner lists.
    uint8_t* bufferData = mStorage.bufferData.alloc(totalSize);
    cgltf_primitive* primitives = mStorage.prims.alloc(primsCount);
    cgltf_attribute* attributes = mStorage.attributes.alloc(attribsCount);
    cgltf_node** nodePointers = mStorage.nodePointers.alloc(nodePointersCount);

    // Allocate top-level structs.
    cgltf_buffer* buffer = mStorage.buffers.alloc(1);
    cgltf_buffer_view* views = mStorage.views.alloc(sourceAsset->buffer_views_count);
    cgltf_accessor* accessors = mStorage.accessors.alloc(sourceAsset->accessors_count);
    cgltf_image* images = mStorage.images.alloc(sourceAsset->images_count);
    cgltf_texture* textures = mStorage.textures.alloc(sourceAsset->textures_count);
    cgltf_material* materials = mStorage.materials.alloc(sourceAsset->materials_count);
    cgltf_mesh* meshes = mStorage.meshes.alloc(sourceAsset->meshes_count);
    cgltf_node* nodes = mStorage.nodes.alloc(sourceAsset->nodes_count);
    cgltf_scene* scenes = mStorage.scenes.alloc(sourceAsset->scenes_count);
    cgltf_data* resultAsset = mStorage.resultAssets.alloc(1);

    // Populate the new buffer object.
    size_t offset = 0;
    vector<size_t> offsets(sourceAsset->buffers_count);
    for (size_t i = 0, len = sourceAsset->buffers_count; i < len; ++i) {
        size_t size = sourceAsset->buffers[i].size;
        memcpy(bufferData + offset, sourceAsset->buffers[i].data, size);
        offsets[i] = offset;
        offset += size;
    }
    buffer->size = totalSize;
    buffer->data = bufferData;

    // Populate the buffer views.
    for (size_t i = 0, len = sourceAsset->buffer_views_count; i < len; ++i) {
        auto& view = views[i] = sourceAsset->buffer_views[i];
        size_t bufferIndex = view.buffer - sourceAsset->buffers;
        view.buffer = buffer;
        view.offset += offsets[bufferIndex];
    }

    // Clone the accessors.
    for (size_t i = 0, len = sourceAsset->accessors_count; i < len; ++i) {
        auto& accessor = accessors[i] = sourceAsset->accessors[i];
        accessor.buffer_view = views + (accessor.buffer_view - sourceAsset->buffer_views);
    }

    // Clone the images.
    for (size_t i = 0, len = sourceAsset->images_count; i < len; ++i) {
        auto& image = images[i] = sourceAsset->images[i];
        if (image.buffer_view) {
            image.buffer_view = views + (image.buffer_view - sourceAsset->buffer_views);
        }
    }

    // Clone the textures.
    for (size_t i = 0, len = sourceAsset->textures_count; i < len; ++i) {
        auto& texture = textures[i] = sourceAsset->textures[i];
        if (texture.image) {
            texture.image = images + (texture.image - sourceAsset->images);
        }
    }

    // Clone the nodes.
    for (size_t i = 0, len = sourceAsset->nodes_count; i < len; ++i) {
        auto& node = nodes[i] = sourceAsset->nodes[i];
        if (node.mesh) {
            node.mesh = meshes + (node.mesh - sourceAsset->meshes);
        }
    }

    // Clone the scenes.
    for (size_t i = 0, len = sourceAsset->scenes_count; i < len; ++i) {
        const auto& sourceScene = sourceAsset->scenes[i];
        auto& resultScene = scenes[i] = sourceScene;
        resultScene.nodes = nodePointers;
        for (size_t j = 0; j < sourceScene.nodes_count; ++j) {
            resultScene.nodes[j] = nodes + (sourceScene.nodes[j] - sourceAsset->nodes);
        }
        nodePointers += sourceScene.nodes_count;
    }

    // Clone the materials.
    for (size_t i = 0, len = sourceAsset->materials_count; i < len; ++i) {
        auto& material = materials[i] = sourceAsset->materials[i];
        auto& t0 = material.pbr_metallic_roughness.base_color_texture.texture;
        auto& t1 = material.pbr_metallic_roughness.metallic_roughness_texture.texture;
        auto& t2 = material.pbr_specular_glossiness.diffuse_texture.texture;
        auto& t3 = material.pbr_specular_glossiness.specular_glossiness_texture.texture;
        auto& t4 = material.normal_texture.texture;
        auto& t5 = material.occlusion_texture.texture;
        auto& t6 = material.emissive_texture.texture;
        t0 = t0 ? textures + (t0 - sourceAsset->textures) : nullptr;
        t1 = t1 ? textures + (t1 - sourceAsset->textures) : nullptr;
        t2 = t2 ? textures + (t2 - sourceAsset->textures) : nullptr;
        t3 = t3 ? textures + (t3 - sourceAsset->textures) : nullptr;
        t4 = t4 ? textures + (t4 - sourceAsset->textures) : nullptr;
        t5 = t5 ? textures + (t5 - sourceAsset->textures) : nullptr;
        t6 = t6 ? textures + (t6 - sourceAsset->textures) : nullptr;
    }

    // Clone the meshes, primitives, and attributes.
    for (size_t i = 0, len = sourceAsset->meshes_count; i < len; ++i) {
        const auto& sourceMesh = sourceAsset->meshes[i];
        auto& resultMesh = meshes[i] = sourceMesh;
        resultMesh.primitives = primitives;
        for (size_t j = 0; j < sourceMesh.primitives_count; ++j) {
            const auto& sourcePrim = sourceMesh.primitives[j];
            auto& resultPrim = resultMesh.primitives[j] = sourcePrim;
            resultPrim.material = materials + (sourcePrim.material - sourceAsset->materials);
            resultPrim.attributes = attributes;
            resultPrim.indices = accessors + (sourcePrim.indices - sourceAsset->accessors);
            for (size_t k = 0; k < sourcePrim.attributes_count; ++k) {
                const auto& sourceAttr = sourcePrim.attributes[k];
                auto& resultAttr = resultPrim.attributes[k] = sourceAttr;
                resultAttr.data = accessors + (sourceAttr.data - sourceAsset->accessors);
            }
            attributes += sourcePrim.attributes_count;
        }
        primitives += sourceMesh.primitives_count;
    }

    // Clone the high-level asset structure, then substitute some of the top-level lists.
    *resultAsset = *sourceAsset;
    resultAsset->buffers = buffer;
    resultAsset->buffers_count = 1;
    resultAsset->buffer_views = views;
    resultAsset->accessors = accessors;
    resultAsset->images = images;
    resultAsset->textures = textures;
    resultAsset->materials = materials;
    resultAsset->meshes = meshes;
    resultAsset->nodes = nodes;
    resultAsset->scenes = scenes;
    resultAsset->scene = scenes + (sourceAsset->scene - sourceAsset->scenes);
    return resultAsset;
}

const cgltf_data* Pipeline::flattenPrims(const cgltf_data* sourceAsset, uint32_t flags) {
    mFlattenFlags = flags;

    // This must be called after flattenBuffers.
    assert(sourceAsset->buffers_count == 1);
    assert(sourceAsset->buffers[0].data != nullptr);

    // Determine the number of primitives and attributes that will be baked.
    size_t numPrims = 0;
    size_t numAttributes = 0;
    for (cgltf_size i = 0; i < sourceAsset->nodes_count; ++i) {
        const cgltf_node& node = sourceAsset->nodes[i];
        if (node.mesh) {
            for (cgltf_size j = 0; j < node.mesh->primitives_count; ++j) {
                cgltf_primitive& sourcePrim = node.mesh->primitives[j];
                if (filterPrim(sourcePrim)) {
                    numPrims++;
                    numAttributes += sourcePrim.attributes_count;
                }
            }
        }
    }
    vector<BakedPrim> bakedPrims;
    bakedPrims.reserve(numPrims);

    // Count the total number of vertices and start filling in the BakedPrim structs.
    int numPositions = 0, numNormals = 0, numTangents = 0, numIndices = 0;
    int numPrimsWithNormals = 0, numPrimsWithTangents = 0;
    for (cgltf_size i = 0; i < sourceAsset->nodes_count; ++i) {
        const cgltf_node& node = sourceAsset->nodes[i];
        if (node.mesh) {
            for (cgltf_size j = 0; j < node.mesh->primitives_count; ++j) {
                const cgltf_primitive& sourcePrim = node.mesh->primitives[j];
                if (filterPrim(sourcePrim)) {
                    BakedPrim bakedPrim {
                        .sourceNode = &node,
                        .sourceMesh = node.mesh,
                        .sourcePrimitive = &sourcePrim,
                    };
                    for (cgltf_size k = 0; k < sourcePrim.attributes_count; ++k) {
                        const cgltf_attribute& attr = sourcePrim.attributes[k];
                        switch (attr.type) {
                            case cgltf_attribute_type_position:
                                numPositions += attr.data->count;
                                bakedPrim.sourcePositions = &attr;
                                break;
                            case cgltf_attribute_type_tangent:
                                numPrimsWithTangents++;
                                numTangents += attr.data->count;
                                bakedPrim.sourceTangents = &attr;
                                break;
                            case cgltf_attribute_type_normal:
                                numPrimsWithNormals++;
                                numNormals += attr.data->count;
                                bakedPrim.sourceNormals = &attr;
                                break;
                            default:
                                break;
                        }
                    }
                    numIndices += sourcePrim.indices->count;
                    bakedPrims.push_back(bakedPrim);
                }
            }
        }
    }

    // Allocate a buffer large enough to hold vertex positions and indices.
    const size_t positionsDataSize = sizeof(float3) * numPositions;
    const size_t normalsDataSize = sizeof(float3) * numNormals;
    const size_t tangentsDataSize = sizeof(float4) * numTangents;
    const size_t vertexDataSize = positionsDataSize + normalsDataSize + tangentsDataSize;
    const size_t indexDataSize = sizeof(uint32_t) * numIndices;
    uint8_t* bufferData = mStorage.bufferData.alloc(vertexDataSize + indexDataSize);
    float3* bakedPositions = (float3*) bufferData;
    float3* bakedNormals = (float3*) (bufferData + positionsDataSize);
    float4* bakedTangents = (float4*) (bufferData + positionsDataSize + normalsDataSize);
    uint32_t* bakedIndices = (uint32_t*) (bufferData + vertexDataSize);

    // Find the overall bounds of the scene and compute a scaling factor. In some cases this
    // provides a huge boost to the quality of the segmentation results.
    mat4f scale;
    if (flags & gltfio::AssetPipeline::SCALE_TO_UNIT) {
        const cgltf_node* node = nullptr;
        mat4f worldTransform;
        float3 minpt = std::numeric_limits<float>::max();
        float3 maxpt = std::numeric_limits<float>::lowest();
        for (size_t i = 0; i < numPrims; ++i) {
            BakedPrim& bakedPrim = bakedPrims[i];
            if (bakedPrim.sourceNode != node) {
                node = bakedPrim.sourceNode;
                cgltf_node_transform_world(node, &worldTransform[0][0]);
            }
            float* pmin = bakedPrim.sourcePositions->data->min;
            float* pmax = bakedPrim.sourcePositions->data->max;
            float3 a = (worldTransform * float4(pmin[0], pmin[1], pmin[2], 1.0)).xyz;
            float3 b = (worldTransform * float4(pmin[0], pmin[1], pmax[2], 1.0)).xyz;
            float3 c = (worldTransform * float4(pmin[0], pmax[1], pmin[2], 1.0)).xyz;
            float3 d = (worldTransform * float4(pmin[0], pmax[1], pmax[2], 1.0)).xyz;
            float3 e = (worldTransform * float4(pmax[0], pmin[1], pmin[2], 1.0)).xyz;
            float3 f = (worldTransform * float4(pmax[0], pmin[1], pmax[2], 1.0)).xyz;
            float3 g = (worldTransform * float4(pmax[0], pmax[1], pmin[2], 1.0)).xyz;
            float3 h = (worldTransform * float4(pmax[0], pmax[1], pmax[2], 1.0)).xyz;
            minpt = min(minpt, min(min(min(min(min(min(min(a, b), c), d), e), f), g), h));
            maxpt = max(maxpt, max(max(max(max(max(max(max(a, b), c), d), e), f), g), h));
        }
        const float3 extent = maxpt - minpt;
        const float maxExtent = std::max(std::max(extent.x, extent.y), extent.z);
        const float scaleFactor = 2.0f / maxExtent;
        const float3 center = (minpt + maxpt) / 2.0f;
        scale = mat4f::scaling(float3(scaleFactor)) * mat4f::translation(-center);
    }

    // Next, perform the actual baking: convert all vertex positions to fp32, transform them by
    // their respective node matrix, etc.
    const cgltf_node* node = nullptr;
    mat4f matrix;
    mat3f normalMatrix;
    for (size_t i = 0; i < numPrims; ++i) {
        BakedPrim& bakedPrim = bakedPrims[i];
        if (bakedPrim.sourceNode != node) {
            node = bakedPrim.sourceNode;
            cgltf_node_transform_world(node, &matrix[0][0]);
            normalMatrix = transpose(inverse(matrix.upperLeft()));
            matrix = scale * matrix;
        }
        const cgltf_primitive* sourcePrim = bakedPrim.sourcePrimitive;

        bakedPrim.bakedPositions = bakedPositions;
        bakedPositions += bakedPrim.sourcePositions->data->count;

        bakedPrim.bakedIndices = bakedIndices;
        bakedIndices += sourcePrim->indices->count;

        if (bakedPrim.sourceNormals) {
            bakedPrim.bakedNormals = bakedNormals;
            bakedNormals += bakedPrim.sourceNormals->data->count;
        }

        if (bakedPrim.sourceTangents) {
            bakedPrim.bakedTangents = bakedTangents;
            bakedTangents += bakedPrim.sourceTangents->data->count;
        }

        bakeTransform(&bakedPrim, matrix, normalMatrix);
    }

    // Keep all buffer views + accessors from the source asset (they can be culled later) and add
    // new buffer views + accessors for indices and baked attributes.
    size_t numAttributesBaked = numPrims + numPrimsWithNormals + numPrimsWithTangents;
    size_t numBufferViews = sourceAsset->buffer_views_count + numPrims + numAttributesBaked;
    size_t numAccessors = sourceAsset->accessors_count + numPrims + numAttributesBaked;

    // Allocate memory for the various cgltf structures.
    cgltf_data* resultAsset = mStorage.resultAssets.alloc(1);
    cgltf_scene* scene = mStorage.scenes.alloc(1);
    cgltf_node** nodePointers = mStorage.nodePointers.alloc(numPrims);
    cgltf_node* nodes = mStorage.nodes.alloc(numPrims);
    cgltf_mesh* meshes = mStorage.meshes.alloc(numPrims);
    cgltf_primitive* prims = mStorage.prims.alloc(numPrims);
    cgltf_buffer_view* views = mStorage.views.alloc(numBufferViews);
    cgltf_accessor* accessors = mStorage.accessors.alloc(numAccessors);
    cgltf_attribute* attributes = mStorage.attributes.alloc(numAttributes);
    cgltf_buffer* buffers = mStorage.buffers.alloc(2);
    cgltf_image* images = mStorage.images.alloc(sourceAsset->images_count);

    // Initialize iterators for various cgtlf lists.
    cgltf_buffer_view* indicesViews = views;
    cgltf_buffer_view* positionsViews = indicesViews + numPrims;
    cgltf_buffer_view* normalsViews = positionsViews + numPrims;
    cgltf_buffer_view* tangentsViews = normalsViews + numPrimsWithNormals;
    cgltf_accessor* indicesAccessors = accessors;
    cgltf_accessor* positionsAccessors = indicesAccessors + numPrims;
    cgltf_accessor* normalsAccessors = positionsAccessors + numPrims;
    cgltf_accessor* tangentsAccessors = normalsAccessors + numPrimsWithNormals;
    cgltf_size positionsOffset = 0;
    cgltf_size normalsOffset = positionsDataSize;
    cgltf_size tangentsOffset = positionsDataSize + normalsDataSize;
    cgltf_size indicesOffset = vertexDataSize;

    // Populate the fields of the cgltf structures.
    size_t attrIndex = 0;
    for (size_t primIndex = 0; primIndex < numPrims; ++primIndex) {
        BakedPrim& bakedPrim = bakedPrims[primIndex];

        nodePointers[primIndex] = nodes + primIndex;

        nodes[primIndex] = {
            .name = bakedPrim.sourceNode->name,
            .mesh = meshes + primIndex,
            .scale = {1.0f, 1.0f, 1.0f},
        };

        meshes[primIndex] = {
            .name = bakedPrim.sourceMesh->name,
            .primitives = prims + primIndex,
            .primitives_count = 1,
        };

        cgltf_accessor& indicesAccessor = *indicesAccessors++ = {
            .component_type = cgltf_component_type_r_32u,
            .type = cgltf_type_scalar,
            .count = bakedPrim.sourcePrimitive->indices->count,
            .stride = sizeof(uint32_t),
            .buffer_view = indicesViews,
        };
        cgltf_buffer_view& indicesBufferView = *indicesViews++ = {
            .buffer = buffers,
            .offset = indicesOffset,
            .size = indicesAccessor.count * sizeof(uint32_t)
        };
        indicesOffset += indicesBufferView.size;

        cgltf_accessor& positionsAccessor = *positionsAccessors++ = {
            .component_type = cgltf_component_type_r_32f,
            .type = cgltf_type_vec3,
            .count = bakedPrim.sourcePositions->data->count,
            .stride = sizeof(float3),
            .buffer_view = positionsViews,
            .has_min = true,
            .has_max = true,
        };
        cgltf_buffer_view& positionsBufferView = *positionsViews++ = {
            .buffer = buffers,
            .offset = positionsOffset,
            .size = positionsAccessor.count * sizeof(float3)
        };
        positionsOffset += positionsBufferView.size;
        *((float3*) positionsAccessor.min) = bakedPrim.bakedMin;
        *((float3*) positionsAccessor.max) = bakedPrim.bakedMax;
        cgltf_attribute& positionsAttribute = attributes[attrIndex++] = {
            .name = (char*) POSITION,
            .type = cgltf_attribute_type_position,
            .data = &positionsAccessor
        };

        if (bakedPrim.bakedNormals) {
            cgltf_accessor& normalsAccessor = *normalsAccessors++ = {
                .component_type = cgltf_component_type_r_32f,
                .type = cgltf_type_vec3,
                .count = bakedPrim.sourceNormals->data->count,
                .stride = sizeof(float3),
                .buffer_view = normalsViews
            };
            cgltf_buffer_view& normalsBufferView = *normalsViews++ = {
                .buffer = buffers,
                .offset = normalsOffset,
                .size = normalsAccessor.count * sizeof(float3)
            };
            normalsOffset += normalsBufferView.size;
            attributes[attrIndex++] = {
                .name = (char*) NORMAL,
                .type = cgltf_attribute_type_normal,
                .data = &normalsAccessor
            };
        }

        if (bakedPrim.bakedTangents) {
            cgltf_accessor& tangentsAccessor = *tangentsAccessors++ = {
                .component_type = cgltf_component_type_r_32f,
                .type = cgltf_type_vec4,
                .count = bakedPrim.sourceTangents->data->count,
                .stride = sizeof(float4),
                .buffer_view = tangentsViews
            };
            cgltf_buffer_view& tangentsBufferView = *tangentsViews++ = {
                .buffer = buffers,
                .offset = tangentsOffset,
                .size = tangentsAccessor.count * sizeof(float4)
            };
            tangentsOffset += tangentsBufferView.size;
            attributes[attrIndex++] = {
                .name = (char*) TANGENT,
                .type = cgltf_attribute_type_tangent,
                .data = &tangentsAccessor
            };
        }

        size_t attrCount = bakedPrim.sourcePrimitive->attributes_count;
        for (size_t j = 0; j < attrCount; ++j) {
            const auto& srcAttr = bakedPrim.sourcePrimitive->attributes[j];
            if (srcAttr.type != cgltf_attribute_type_position &&
                    srcAttr.type != cgltf_attribute_type_normal &&
                    srcAttr.type != cgltf_attribute_type_tangent) {
                auto& attr = attributes[attrIndex++] = srcAttr;
                size_t accessorIndex = attr.data - sourceAsset->accessors;
                attr.data = accessors + numPrims + numAttributesBaked + accessorIndex;
            }
        }

        prims[primIndex] = {
            .type = cgltf_primitive_type_triangles,
            .indices = &indicesAccessor,
            .material = bakedPrim.sourcePrimitive->material,
            .attributes = &positionsAttribute,
            .attributes_count = attrCount,
        };
    }

    assert(attrIndex == numAttributes);

    scene->name = sourceAsset->scene->name;
    scene->nodes = nodePointers;
    scene->nodes_count = numPrims;

    buffers[0].size = vertexDataSize + indexDataSize;
    buffers[0].data = bufferData;

    buffers[1] = sourceAsset->buffers[0];

    resultAsset->file_type = sourceAsset->file_type;
    resultAsset->file_data = sourceAsset->file_data;
    resultAsset->asset = sourceAsset->asset;
    resultAsset->asset.generator = (char*) GENERATOR_ID;
    resultAsset->meshes = meshes;
    resultAsset->meshes_count = numPrims;
    resultAsset->accessors = accessors;
    resultAsset->accessors_count = numAccessors;
    resultAsset->buffer_views = views;
    resultAsset->buffer_views_count = numBufferViews;
    resultAsset->buffers = buffers;
    resultAsset->buffers_count = 2;
    resultAsset->nodes = nodes;
    resultAsset->nodes_count = numPrims;
    resultAsset->scenes = scene;
    resultAsset->scenes_count = 1;
    resultAsset->scene = scene;
    resultAsset->images = images;
    resultAsset->images_count = sourceAsset->images_count;
    resultAsset->textures = sourceAsset->textures;
    resultAsset->textures_count = sourceAsset->textures_count;
    resultAsset->materials = sourceAsset->materials;
    resultAsset->materials_count = sourceAsset->materials_count;
    resultAsset->samplers = sourceAsset->samplers;
    resultAsset->samplers_count = sourceAsset->samplers_count;

    // Copy over the buffer views, accessors, and textures, then fix up the pointers.
    const size_t offset = numPrims + numAttributesBaked;
    for (size_t i = offset; i < numBufferViews; ++i) {
        auto& view = views[i] = sourceAsset->buffer_views[i - offset];
        view.buffer = &buffers[1];
    }
    for (size_t i = offset; i < numAccessors; ++i) {
        auto& accessor = accessors[i] = sourceAsset->accessors[i - offset];
        size_t viewIndex = accessor.buffer_view - sourceAsset->buffer_views;
        accessor.buffer_view = views + offset + viewIndex;
    }

    for (size_t i = 0; i < sourceAsset->images_count; ++i) {
        auto& image = images[i] = sourceAsset->images[i];
        if (image.buffer_view) {
            size_t viewIndex = image.buffer_view - sourceAsset->buffer_views;
            image.buffer_view = views + offset + viewIndex;
        }
    }
    for (size_t i = 0; i < resultAsset->textures_count; ++i) {
        auto& image = resultAsset->textures[i].image;
        if (image) {
            size_t imageIndex = image - sourceAsset->images;
            image = images + imageIndex;
        }
    }

    return resultAsset;
}

void Pipeline::bakeAmbientOcclusion(const cgltf_data* sourceAsset, image::LinearImage target,
        const RenderOptions& options) {
    SimpleMesh* meshes;
    size_t numMeshes;
    cgltfToSimpleMesh(sourceAsset, &meshes, &numMeshes);

    filament::rays::PathTracer::Builder builder;
    builder
        .meshes(meshes, numMeshes)
        .outputPlane(filament::rays::AMBIENT_OCCLUSION, target)
        .uvCamera(options.enableDilation)
        .denoise(options.enableDilation)
        .samplesPerPixel(options.samplesPerPixel)
        .occlusionRayBounds(options.aoRayNear, std::numeric_limits<float>::infinity())
        .tileCallback(options.progress, options.userData)
        .doneCallback(options.done, options.userData);

    builder.build().render();
}

void Pipeline::renderAmbientOcclusion(const cgltf_data* sourceAsset, image::LinearImage target,
        const filament::rays::SimpleCamera& camera, const RenderOptions& options) {
    SimpleMesh* meshes;
    size_t numMeshes;
    cgltfToSimpleMesh(sourceAsset, &meshes, &numMeshes, true);

    filament::rays::PathTracer::Builder builder;
    builder
        .meshes(meshes, numMeshes)
        .outputPlane(filament::rays::AMBIENT_OCCLUSION, target)
        .filmCamera(camera)
        .denoise(options.enableDenoise)
        .samplesPerPixel(options.samplesPerPixel)
        .occlusionRayBounds(options.aoRayNear, std::numeric_limits<float>::infinity())
        .tileCallback(options.progress, options.userData)
        .doneCallback(options.done, options.userData);

    builder.build().render();
}

void Pipeline::bakeAllOutputs(const cgltf_data* sourceAsset,
        image::LinearImage targets[filament::rays::OUTPUTPLANE_COUNT],
        const RenderOptions& options) {
    SimpleMesh* meshes;
    size_t numMeshes;
    cgltfToSimpleMesh(sourceAsset, &meshes, &numMeshes);

    using namespace filament::rays;
    PathTracer::Builder builder;
    builder
        .meshes(meshes, numMeshes)
        .outputPlane(AMBIENT_OCCLUSION, targets[(int) AMBIENT_OCCLUSION])
        .outputPlane(BENT_NORMALS, targets[(int) BENT_NORMALS])
        .outputPlane(MESH_NORMALS, targets[(int) MESH_NORMALS])
        .outputPlane(MESH_POSITIONS, targets[(int) MESH_POSITIONS])
        .uvCamera(options.enableDilation)
        .denoise()
        .samplesPerPixel(options.samplesPerPixel)
        .occlusionRayBounds(options.aoRayNear, std::numeric_limits<float>::infinity())
        .tileCallback(options.progress, options.userData)
        .doneCallback(options.done, options.userData);

    builder.build().render();
}

void Pipeline::bakeTransform(BakedPrim* prim, const mat4f& transform, const mat3f& normalMatrix) {
    const cgltf_primitive* source = prim->sourcePrimitive;
    const cgltf_attribute* sourcePositions = prim->sourcePositions;
    const size_t numPositions = sourcePositions->data->count;

    // Read position data, converting to float if necessary.
    cgltf_float* writePtr = &prim->bakedPositions->x;
    for (cgltf_size index = 0; index < numPositions; ++index, writePtr += 3) {
        cgltf_accessor_read_float(sourcePositions->data, index, writePtr, 3);
    }

    // Prepare for computing the post-transformed bounding box.
    float3& minpt = prim->bakedMin = std::numeric_limits<float>::max();
    float3& maxpt = prim->bakedMax = std::numeric_limits<float>::lowest();

    // Transform the positions and compute the new bounding box.
    float3* bakedPositions = prim->bakedPositions;
    for (cgltf_size index = 0; index < numPositions; ++index) {
        float3& pt = bakedPositions[index];
        pt = (transform * float4(pt, 1.0f)).xyz;
        minpt = min(minpt, pt);
        maxpt = max(maxpt, pt);
    }

    // Read index data, converting to uint32 if necessary.
    uint32_t* bakedIndices = prim->bakedIndices;
    for (cgltf_size index = 0, len = source->indices->count; index < len; ++index) {
        bakedIndices[index] = cgltf_accessor_read_index(source->indices, index);
    }

    // Transform normals if available.
    if (prim->bakedNormals) {
        const cgltf_attribute* sourceNormals = prim->sourceNormals;
        const size_t numNormals = sourceNormals->data->count;
        cgltf_float* writePtr = &prim->bakedNormals->x;
        for (cgltf_size index = 0; index < numNormals; ++index, writePtr += 3) {
            cgltf_accessor_read_float(sourceNormals->data, index, writePtr, 3);
        }
        float3* bakedNormals = prim->bakedNormals;
        for (cgltf_size index = 0; index < numNormals; ++index) {
            float3& n = bakedNormals[index];
            n = normalize(normalMatrix * n);
        }
    }

    // Transform tangents if available.
    if (prim->bakedTangents) {
        const cgltf_attribute* sourceTangents = prim->sourceTangents;
        const size_t numTangents = sourceTangents->data->count;
        cgltf_float* writePtr = &prim->bakedTangents->x;
        for (cgltf_size index = 0; index < numTangents; ++index, writePtr += 4) {
            cgltf_accessor_read_float(sourceTangents->data, index, writePtr, 4);
        }
        float4* bakedTangents = prim->bakedTangents;
        for (cgltf_size index = 0; index < numTangents; ++index) {
            float3& t = bakedTangents[index].xyz;
            t = normalize(normalMatrix * t);
        }
    }
}

bool Pipeline::cgltfToXatlas(const cgltf_data* sourceAsset, xatlas::Atlas* atlas) {
    for (size_t meshIndex = 0; meshIndex < sourceAsset->meshes_count; ++meshIndex) {
        const cgltf_primitive& prim = sourceAsset->meshes[meshIndex].primitives[0];
        const cgltf_attribute* positions = nullptr;
        const cgltf_attribute* texcoords = nullptr;
        const cgltf_attribute* normals = nullptr;

        // Gather all vertex attributes of interest.
        for (cgltf_size k = 0; k < prim.attributes_count; ++k) {
            const cgltf_attribute& attr = prim.attributes[k];
            if (attr.index != 0 || attr.data == nullptr || attr.data->buffer_view == nullptr) {
                continue;
            }
            if (attr.data->component_type != cgltf_component_type_r_32f) {
                continue;
            }
            switch (attr.type) {
                case cgltf_attribute_type_position: positions = &attr; break;
                case cgltf_attribute_type_normal:   normals = &attr;   break;
                case cgltf_attribute_type_texcoord: texcoords = &attr; break;
                default: break;
            }
        }

        xatlas::MeshDecl decl {};

        // xatlas can produce higher-quality results if it has normals, but they are optional.
        if (normals) {
            const auto& accessor = normals->data;
            cgltf_size offset = accessor->offset + accessor->buffer_view->offset;
            const uint8_t* data = (const uint8_t*) accessor->buffer_view->buffer->data;
            decl.vertexNormalData = data + offset;
            decl.vertexNormalStride = accessor->stride ? accessor->stride : sizeof(float3);
        }

        // Again, xatlas can produce higher-quality results if it has UVs, but they are optional.
        if (texcoords) {
            const auto& accessor = texcoords->data;
            cgltf_size offset = accessor->offset + accessor->buffer_view->offset;
            const uint8_t* data = (const uint8_t*) accessor->buffer_view->buffer->data;
            decl.vertexUvData = data + offset;
            decl.vertexUvStride = accessor->stride ? accessor->stride : sizeof(float2);
        }

        // The flattening process guarantees packed fp32 position data.
        {
            const auto& accessor = positions->data;
            cgltf_size offset = accessor->offset + accessor->buffer_view->offset;
            const uint8_t* data = (const uint8_t*) accessor->buffer_view->buffer->data;
            decl.vertexCount = uint32_t(positions->data->count);
            decl.vertexPositionData = data + offset;
            decl.vertexPositionStride = sizeof(float3);
        }

        // The flattening process guarantees packed uint32 indices.
        {
            const auto& accessor = prim.indices;
            cgltf_size offset = accessor->offset + accessor->buffer_view->offset;
            const uint8_t* data = (const uint8_t*) accessor->buffer_view->buffer->data;
            decl.indexFormat = xatlas::IndexFormat::UInt32;
            decl.indexData = data + offset;
            decl.indexCount = accessor->count;
        }

        xatlas::AddMeshError::Enum error = xatlas::AddMesh(atlas, decl);
        if (error != xatlas::AddMeshError::Success) {
            utils::slog.e << "Error parameterizing " << sourceAsset->meshes[meshIndex].name <<
                    " -- " << xatlas::StringForEnum(error) << utils::io::endl;
            return false;
        }
    }

    return true;
}

cgltf_data* Pipeline::xatlasToCgltf(const cgltf_data* sourceAsset, const xatlas::Atlas* atlas) {
    if (sourceAsset->buffers_count != 1 || sourceAsset->buffers[0].data == nullptr) {
        utils::slog.e << "Parameterization requires a valid flattened asset." << utils::io::endl;
        return nullptr;
    }
    if (atlas->meshCount != sourceAsset->meshes_count) {
        utils::slog.e << "Unexpected mesh count." << utils::io::endl;
        return nullptr;
    }

    // Check if baked UVs already exist, in which case they are replaced rather than added.
    auto skipAttribute = [](const cgltf_attribute& attr) {
        const cgltf_int uvindex = gltfio::AssetPipeline::BAKED_UV_ATTRIB_INDEX;
        return attr.type == cgltf_attribute_type_texcoord && attr.index == uvindex;
    };

    // Determine the number of attributes that will be required, which is the same as the old number
    // of attributes plus an extra UV set per prim.
    size_t numAttributes = 0;
    const size_t numPrims = atlas->meshCount;
    for (cgltf_size i = 0; i < numPrims; ++i) {
        const cgltf_mesh& mesh = sourceAsset->meshes[i];
        const cgltf_primitive& sourcePrim = mesh.primitives[0];
        numAttributes += sourcePrim.attributes_count + 1;
        for (size_t ai = 0; ai < sourcePrim.attributes_count; ++ai) {
            if (skipAttribute(sourcePrim.attributes[ai])) {
                numAttributes--;
                break;
            }
        }
    }

    // The number of required accessors will be the same as the number of vertex attributes, plus
    // an additional one for for the index buffer.
    const size_t numAccessors = numAttributes + numPrims;

    // We need two buffer views per primitive: one for vertex attributes and one for indices.
    // We need a unique buffer view per primitive because the strides might be unique.
    const size_t numBufferViews = numPrims * 2;

    // Determine the number of node references.
    size_t numNodePointers = 0;
    for (cgltf_size i = 0; i < sourceAsset->scenes_count; ++i) {
        const cgltf_scene& scene = sourceAsset->scenes[i];
        numNodePointers += scene.nodes_count;
    }

    // Compute the size of the new vertex and index buffers (which are consolidated).
    cgltf_size numIndices = 0;
    cgltf_size numFloats = 0;
    for (cgltf_size i = 0; i < numPrims; i++) {
        const auto& atlasMesh = atlas->meshes[i];
        const auto& sourceMesh = sourceAsset->meshes[i];
        const cgltf_primitive& sourcePrim = sourceMesh.primitives[0];
        numIndices += atlasMesh.indexCount;
        cgltf_size floatsPerVert = 0;
        for (size_t ai = 0; ai < sourcePrim.attributes_count; ++ai) {
            const cgltf_attribute& attr = sourcePrim.attributes[ai];
            if (skipAttribute(attr)) {
                continue;
            }
            floatsPerVert += getNumFloats(attr.data->type);
        }
        floatsPerVert += 2;
        numFloats += atlasMesh.vertexCount * floatsPerVert;
    }
    cgltf_size resultBufferSize = 4 * (numIndices + numFloats);

    // Allocate top-level structs.
    cgltf_data* resultAsset = mStorage.resultAssets.alloc(1);
    cgltf_scene* scenes = mStorage.scenes.alloc(sourceAsset->scenes_count);
    cgltf_node** nodePointers = mStorage.nodePointers.alloc(numNodePointers);
    cgltf_node* nodes = mStorage.nodes.alloc(numPrims);
    cgltf_mesh* meshes = mStorage.meshes.alloc(numPrims);
    cgltf_primitive* prims = mStorage.prims.alloc(numPrims);
    cgltf_buffer_view* views = mStorage.views.alloc(numBufferViews);
    cgltf_accessor* accessors = mStorage.accessors.alloc(numAccessors);
    cgltf_attribute* attributes = mStorage.attributes.alloc(numAttributes);
    cgltf_buffer* buffers = mStorage.buffers.alloc(1);
    uint8_t* resultData = mStorage.bufferData.alloc(resultBufferSize);

    buffers[0] = {
        .size = resultBufferSize,
        .data = resultData
    };

    // Clone the scenes and nodes. This is easy because the source asset has been flattened.
    assert(numPrims == sourceAsset->nodes_count);
    for (size_t i = 0, len = sourceAsset->scenes_count; i < len; ++i) {
        cgltf_scene& scene = scenes[i] = sourceAsset->scenes[i];
        for (size_t j = 0; j < scene.nodes_count; ++j) {
            size_t nodeIndex = scene.nodes[j] - sourceAsset->nodes;
            nodePointers[j] = nodes + nodeIndex;
        }
        scene.nodes = nodePointers;
    }
    for (cgltf_size i = 0; i < numPrims; i++) {
        auto& node = nodes[i] = sourceAsset->nodes[i];
        node.mesh = meshes + i;
    }

    // Convert all vertices from the source asset to fp32 and create a new interleaved buffer.
    float* vertexWritePtr = (float*) resultData;
    for (cgltf_size i = 0; i < numPrims; i++) {
        const auto& atlasMesh = atlas->meshes[i];
        const cgltf_mesh& sourceMesh = sourceAsset->meshes[i];
        const cgltf_primitive& sourcePrim = sourceMesh.primitives[0];
        for (uint32_t j = 0; j < atlasMesh.vertexCount; ++j) {
            const xatlas::Vertex& atlasVertex = atlasMesh.vertexArray[j];
            uint32_t sourceIndex = atlasVertex.xref;
            for (size_t ai = 0; ai < sourcePrim.attributes_count; ++ai) {
                const cgltf_attribute& attr = sourcePrim.attributes[ai];
                if (skipAttribute(attr)) {
                    continue;
                }
                const cgltf_accessor* accessor = attr.data;
                cgltf_type attribType = accessor->type;
                cgltf_size elementSize = getNumFloats(attribType);
                cgltf_accessor_read_float(accessor, sourceIndex, vertexWritePtr, elementSize);
                vertexWritePtr += elementSize;
            }
            vertexWritePtr[0] = atlasVertex.uv[0] / atlas->width;
            vertexWritePtr[1] = atlasVertex.uv[1] / atlas->height;
            vertexWritePtr += 2;
        }
    }

    // Populate the index buffer.
    uint32_t* indexWritePtr = (uint32_t*) vertexWritePtr;
    for (cgltf_size i = 0; i < numPrims; i++) {
        const auto& atlasMesh = atlas->meshes[i];
        memcpy(indexWritePtr, atlasMesh.indexArray, atlasMesh.indexCount * sizeof(uint32_t));
        indexWritePtr += atlasMesh.indexCount;
    }

    // Populate the two buffer views for each prim.
    cgltf_size vertexBufferOffset = 0;
    cgltf_size indexBufferOffset = numFloats * sizeof(float);
    cgltf_buffer_view* resultBufferView = views;
    for (cgltf_size i = 0; i < numPrims; i++) {
        const auto& atlasMesh = atlas->meshes[i];
        const cgltf_mesh& sourceMesh = sourceAsset->meshes[i];
        const cgltf_primitive& sourcePrim = sourceMesh.primitives[0];

        // Determine the vertex stride.
        cgltf_size floatsPerVert = 0;
        for (size_t ai = 0; ai < sourcePrim.attributes_count; ++ai) {
            const cgltf_attribute& attr = sourcePrim.attributes[ai];
            if (skipAttribute(attr)) {
                continue;
            }
            floatsPerVert += getNumFloats(attr.data->type);
        }
        floatsPerVert += 2;
        const cgltf_size stride = floatsPerVert * sizeof(float);

        // Populate the buffer view for the vertex attributes.
        resultBufferView[0] = {
            .buffer = buffers,
            .offset = vertexBufferOffset,
            .size = atlasMesh.vertexCount * stride,
            .stride = stride,
            .type = cgltf_buffer_view_type_vertices
        };

        // Populate the buffer view for the triangle indices.
        resultBufferView[1] = {
            .buffer = buffers,
            .offset = indexBufferOffset,
            .size = atlasMesh.indexCount * sizeof(uint32_t),
            .type = cgltf_buffer_view_type_indices
        };

        vertexBufferOffset += resultBufferView[0].size;
        indexBufferOffset += resultBufferView[1].size;
        resultBufferView += 2;
    }

    // Populate the accessors and attributes for each prim.
    cgltf_accessor* resultAccessor = accessors;
    cgltf_attribute* resultAttribute = attributes;
    for (cgltf_size i = 0; i < numPrims; i++) {
        const auto& atlasMesh = atlas->meshes[i];
        const cgltf_mesh& sourceMesh = sourceAsset->meshes[i];
        const cgltf_primitive& sourcePrim = sourceMesh.primitives[0];
        cgltf_mesh& resultMesh = meshes[i];
        cgltf_primitive& resultPrim = prims[i];
        resultMesh = sourceMesh;
        resultMesh.primitives = &resultPrim;
        resultPrim = sourcePrim;

        resultPrim.attributes = resultAttribute;
        resultPrim.attributes_count = sourcePrim.attributes_count + 1;
        for (size_t ai = 0; ai < sourcePrim.attributes_count; ++ai) {
            if (skipAttribute(sourcePrim.attributes[ai])) {
                resultPrim.attributes_count--;
                break;
            }
        }

        resultPrim.indices = resultAccessor + resultPrim.attributes_count;
        cgltf_buffer_view* vertexBufferView = views + i * 2 + 0;
        cgltf_buffer_view* indexBufferView = views + i * 2 + 1;
        cgltf_size offset = 0;
        for (size_t ai = 0; ai < sourcePrim.attributes_count; ++ai) {
            const cgltf_attribute& attr = sourcePrim.attributes[ai];
            if (skipAttribute(attr)) {
                continue;
            }
            const cgltf_accessor* sourceAccessor = attr.data;
            *resultAttribute = {
                .name = attr.name,
                .type = attr.type,
                .index = attr.index,
                .data = resultAccessor
            };
            *resultAccessor = {
                .component_type = cgltf_component_type_r_32f,
                .type = sourceAccessor->type,
                .offset = offset,
                .count = atlasMesh.vertexCount,
                .stride = vertexBufferView->stride,
                .buffer_view = vertexBufferView,
                .has_min = sourceAccessor->has_min,
                .has_max = sourceAccessor->has_max,
            };
            if (resultAccessor->has_min) {
                memcpy(resultAccessor->min, sourceAccessor->min, sizeof(sourceAccessor->min));
            }
            if (resultAccessor->has_max) {
                memcpy(resultAccessor->max, sourceAccessor->max, sizeof(sourceAccessor->max));
            }
            offset += sizeof(float) * getNumFloats(sourceAccessor->type);
            ++resultAccessor;
            ++resultAttribute;
        }

        // Create the new attribute for the baked UV's and point it to its corresponding accessor.
        *resultAttribute++ = {
            .name = (char*) gltfio::AssetPipeline::BAKED_UV_ATTRIB,
            .type = cgltf_attribute_type_texcoord,
            .index = gltfio::AssetPipeline::BAKED_UV_ATTRIB_INDEX,
            .data = resultAccessor
        };
        *resultAccessor++ = {
            .component_type = cgltf_component_type_r_32f,
            .type = cgltf_type_vec2,
            .offset = offset,
            .count = atlasMesh.vertexCount,
            .stride = vertexBufferView->stride,
            .buffer_view = vertexBufferView,
        };

        // Accessor for index buffer.
        *resultAccessor++ = {
            .component_type = cgltf_component_type_r_32u,
            .type = cgltf_type_scalar,
            .count = atlasMesh.indexCount,
            .stride = sizeof(uint32_t),
            .buffer_view = indexBufferView,
        };
    }

    // Clone the high-level asset structure, then substitute some of the top-level lists.
    *resultAsset = *sourceAsset;
    resultAsset->buffers = buffers;
    resultAsset->buffer_views = views;
    resultAsset->buffer_views_count = numBufferViews;
    resultAsset->accessors = accessors;
    resultAsset->accessors_count = numAccessors;
    resultAsset->meshes = meshes;
    resultAsset->nodes = nodes;
    resultAsset->scenes = scenes;
    resultAsset->scene = scenes + (sourceAsset->scene - sourceAsset->scenes);
    return resultAsset;
}

void Pipeline::cgltfToSimpleMesh(const cgltf_data* sourceAsset, SimpleMesh** meshes,
        size_t* numMeshes, bool provideDummyTexCoords) {
    *numMeshes = sourceAsset->scene->nodes_count;
    *meshes = mStorage.simpleMeshes.alloc(sourceAsset->scene->nodes_count);

    const cgltf_node** nodes = (const cgltf_node**) sourceAsset->scene->nodes;
    for (cgltf_size i = 0, len = sourceAsset->scene->nodes_count; i < len; ++i) {
        const cgltf_primitive& prim = nodes[i]->mesh->primitives[0];

        // Collect the attributes for positions, uvs, and normals.
        const cgltf_accessor* positions = nullptr;
        const cgltf_accessor* texcoords = nullptr;
        const cgltf_accessor* normals = nullptr;
        for (cgltf_size j = 0, len = prim.attributes_count; j < len; ++j) {
            const cgltf_attribute& attr = prim.attributes[j];
            if (attr.type == cgltf_attribute_type_position && attr.index == 0) {
                positions = attr.data;
            } else if (!strcmp(attr.name, gltfio::AssetPipeline::BAKED_UV_ATTRIB)) {
                texcoords = attr.data;
            } else if (attr.type == cgltf_attribute_type_normal && attr.index == 0) {
                normals = attr.data;
            }
        }

        if (!texcoords && provideDummyTexCoords) {
            texcoords = positions;
        }

        // Allocate space for embree vertex data, then populate it.
        if (positions && texcoords && normals) {
            const size_t numBytes = sizeof(float) * 3 * positions->count;
            float* pTexcoords = (float*) mStorage.bufferData.alloc(numBytes);
            float* pPositions = (float*) mStorage.bufferData.alloc(numBytes);
            float* pNormals = (float*) mStorage.bufferData.alloc(numBytes);
            float* uvs = pTexcoords;
            float* pos = pPositions;
            float* nrm = pNormals;

            // Note that SimpleMesh requires all vertex attributes to be float3, including texture
            // coordinates. This is because baking uses UV as position data, and embree always
            // requires 3D position data.
            for (size_t i = 0; i < positions->count; ++i) {
                cgltf_accessor_read_float(texcoords, i, uvs, 2);
                cgltf_accessor_read_float(positions, i, pos, 3);
                cgltf_accessor_read_float(normals, i, nrm, 3);
                uvs[2] = 0.0f;
                pos += 3;
                uvs += 3;
                nrm += 3;
            }

            // For indices, we simply reference the data directly, no need for conversion.
            uint8_t* pIndices = ((uint8_t*) prim.indices->buffer_view->buffer->data) +
                    prim.indices->offset + prim.indices->buffer_view->offset;

            (*meshes)[i] = SimpleMesh {
                .numVertices = positions->count,
                .numIndices = prim.indices->count,
                .positions = pPositions,
                .positionsStride = sizeof(float) * 3,
                .indices = (uint32_t*) pIndices,
                .normals = pNormals,
                .normalsStride = sizeof(float) * 3,
                .uvs = pTexcoords,
                .uvsStride = sizeof(float) * 3
            };

        } else {
            utils::slog.w << "Complete attributes not found for mesh " << i << " of " << len
                    << utils::io::endl;
        }
    }
}

const cgltf_data* Pipeline::parameterize(const cgltf_data* sourceAsset, int maxIterations) {
    if (!isFlattened(sourceAsset)) {
        utils::slog.e << "Only flattened assets can be parameterized." << utils::io::endl;
        return nullptr;
    }

    xatlas::Atlas* atlas = xatlas::Create();
    if (!cgltfToXatlas(sourceAsset, atlas)) {
        return nullptr;
    }

    utils::slog.i << "Computing charts..." << utils::io::endl;
    xatlas::ChartOptions coptions;
    coptions.maxIterations = maxIterations;
    xatlas::ComputeCharts(atlas, coptions);

    utils::slog.i << "Parameterizing charts..." << utils::io::endl;
    xatlas::ParameterizeCharts(atlas);

    utils::slog.i << "Packing charts..." << utils::io::endl;
    xatlas::PackCharts(atlas);

    if (atlas->atlasCount == 0) {
        // If this occurs, try building the debug configuration (xatlas has many asserts).
        utils::slog.e << "Error: unable to generate atlas, mesh is not clean." << utils::io::endl;
        xatlas::Destroy(atlas);
        return nullptr;
    }

    utils::slog.i << "Produced "
            << atlas->atlasCount << " atlases, "
            << atlas->chartCount << " charts, "
            << atlas->meshCount  << " meshes." << utils::io::endl;

    const cgltf_data* result = xatlasToCgltf(sourceAsset, atlas);
    xatlas::Destroy(atlas);
    return result;
}

const cgltf_data* Pipeline::generatePreview(const cgltf_data* sourceAsset, const Path& texture) {
    if (!isFlattened(sourceAsset)) {
        utils::slog.e << "Only flattened assets can be modified." << utils::io::endl;
        return nullptr;
    }
    const std::string texturePath = texture;

    // The only attributes we need are positions and UVs.
    const size_t numPrims = sourceAsset->meshes_count;
    const size_t numAttributes = numPrims * 2;

    // The number of required accessors will be the same as the number of vertex attributes, plus
    // an additional one for the index buffer.
    const size_t numAccessors = numAttributes + numPrims;

    // Determine the number of node references.
    size_t numNodePointers = 0;
    for (cgltf_size i = 0; i < sourceAsset->scenes_count; ++i) {
        const cgltf_scene& scene = sourceAsset->scenes[i];
        numNodePointers += scene.nodes_count;
    }

    // Allocate memory for new asset structures that will be retained by the pipeline.
    cgltf_data* resultAsset = mStorage.resultAssets.alloc(1);
    cgltf_scene* scenes = mStorage.scenes.alloc(sourceAsset->scenes_count);
    cgltf_node** nodePointers = mStorage.nodePointers.alloc(numNodePointers);
    cgltf_node* nodes = mStorage.nodes.alloc(numPrims);
    cgltf_mesh* meshes = mStorage.meshes.alloc(numPrims);
    cgltf_primitive* prims = mStorage.prims.alloc(numPrims);
    cgltf_accessor* accessors = mStorage.accessors.alloc(numAccessors);
    cgltf_attribute* attributes = mStorage.attributes.alloc(numAttributes);
    cgltf_image* images = mStorage.images.alloc(1);
    cgltf_texture* textures = mStorage.textures.alloc(1);
    cgltf_material* materials = mStorage.materials.alloc(1);
    char* pathString = (char*) mStorage.bufferData.alloc(texturePath.size() + 1);

    // Clone the scenes and nodes. This is easy because the source asset has been flattened.
    assert(numPrims == sourceAsset->nodes_count);
    for (size_t i = 0, len = sourceAsset->scenes_count; i < len; ++i) {
        cgltf_scene& scene = scenes[i] = sourceAsset->scenes[i];
        for (size_t j = 0; j < scene.nodes_count; ++j) {
            size_t nodeIndex = scene.nodes[j] - sourceAsset->nodes;
            nodePointers[j] = nodes + nodeIndex;
        }
        scene.nodes = nodePointers;
    }
    for (cgltf_size i = 0; i < numPrims; i++) {
        auto& node = nodes[i] = sourceAsset->nodes[i];
        node.mesh = meshes + i;
    }

    // Populate the accessors and attributes for each prim by filtering out position and UV's.
    cgltf_accessor* resultAccessor = accessors;
    cgltf_attribute* resultAttribute = attributes;
    for (cgltf_size i = 0; i < numPrims; i++) {
        const cgltf_mesh& sourceMesh = sourceAsset->meshes[i];
        const cgltf_primitive& sourcePrim = sourceMesh.primitives[0];
        cgltf_mesh& resultMesh = meshes[i];
        cgltf_primitive& resultPrim = prims[i];
        resultMesh = sourceMesh;
        resultMesh.primitives = &resultPrim;
        resultPrim = sourcePrim;
        resultPrim.attributes = resultAttribute;
        resultPrim.attributes_count = 0;
        resultPrim.material = materials;
        for (size_t ai = 0; ai < sourcePrim.attributes_count; ++ai) {
            const cgltf_attribute& sourceAttrib = sourcePrim.attributes[ai];
            const cgltf_accessor* sourceAccessor = sourceAttrib.data;
            if (sourceAttrib.type == cgltf_attribute_type_texcoord &&
                    sourceAttrib.index == gltfio::AssetPipeline::BAKED_UV_ATTRIB_INDEX) {
                *resultAttribute = sourceAttrib;
                resultAttribute->data = resultAccessor;
                *resultAccessor = *sourceAccessor;
                if (resultAccessor->has_min) {
                    memcpy(resultAccessor->min, sourceAccessor->min, sizeof(sourceAccessor->min));
                }
                if (resultAccessor->has_max) {
                    memcpy(resultAccessor->max, sourceAccessor->max, sizeof(sourceAccessor->max));
                }
                resultAccessor++;
                resultAttribute++;
                resultPrim.attributes_count++;
                continue;
            }
            if (sourceAttrib.type == cgltf_attribute_type_position && sourceAttrib.index == 0) {
                *resultAttribute = sourceAttrib;
                resultAttribute->data = resultAccessor;
                *resultAccessor = *sourceAccessor;
                if (resultAccessor->has_min) {
                    memcpy(resultAccessor->min, sourceAccessor->min, sizeof(sourceAccessor->min));
                }
                if (resultAccessor->has_max) {
                    memcpy(resultAccessor->max, sourceAccessor->max, sizeof(sourceAccessor->max));
                }
                resultAccessor++;
                resultAttribute++;
                resultPrim.attributes_count++;
                continue;
            }
        }
        assert(resultPrim.attributes_count == 2);
        resultPrim.indices = resultAccessor;
        *resultAccessor++ = *sourcePrim.indices;
    }

    // Create the new material, texture, and image.
    materials[0] = {
        .name = (char*) PREVIEW_MATERIAL_NAME,
        .has_pbr_metallic_roughness = true,
        .has_pbr_specular_glossiness = false,
        .pbr_metallic_roughness = {
            .base_color_texture = {
                .texture = textures,
                .texcoord = gltfio::AssetPipeline::BAKED_UV_ATTRIB_INDEX
            },
            .base_color_factor = {1, 1, 1, 1},
            .metallic_factor = 0.0f,
            .roughness_factor = 1.0f
        },
        .pbr_specular_glossiness = {},
	    .normal_texture = {},
	    .occlusion_texture = {},
	    .emissive_texture = {},
	    .emissive_factor = {},
	    .alpha_mode = cgltf_alpha_mode_opaque,
	    .alpha_cutoff = 0.5f,
	    .double_sided = false,
	    .unlit = true
    };
    textures[0] = { .image = images };
    images[0] = { .uri = pathString };
    strncpy(pathString, texturePath.c_str(), texturePath.size() + 1);

    // Clone the high-level asset structure, then substitute some of the top-level lists.
    *resultAsset = *sourceAsset;
    resultAsset->accessors = accessors;
    resultAsset->accessors_count = numAccessors;
    resultAsset->images = images;
    resultAsset->images_count = 1;
    resultAsset->textures = textures;
    resultAsset->textures_count = 1;
    resultAsset->materials = materials;
    resultAsset->materials_count = 1;
    resultAsset->meshes = meshes;
    resultAsset->nodes = nodes;
    resultAsset->scenes = scenes;
    resultAsset->scene = scenes + (sourceAsset->scene - sourceAsset->scenes);
    return resultAsset;
}

const cgltf_data* Pipeline::replaceOcclusion(const cgltf_data* sourceAsset, const Path& texture) {
    if (!isFlattened(sourceAsset)) {
        utils::slog.e << "Only flattened assets can be modified." << utils::io::endl;
        return nullptr;
    }
    const std::string texturePath = texture;
    const size_t numPrims = sourceAsset->meshes_count;

    // Count the total number of referenced nodes.
    size_t nodePointersCount = 0;
    for (size_t i = 0, len = sourceAsset->scenes_count; i < len; ++i) {
        const auto& scene = sourceAsset->scenes[i];
        nodePointersCount += scene.nodes_count;
    }

    // Allocate memory for new asset structures that will be retained by the pipeline.
    cgltf_data* resultAsset = mStorage.resultAssets.alloc(1);
    cgltf_image* images = mStorage.images.alloc(sourceAsset->images_count + 1);
    cgltf_texture* textures = mStorage.textures.alloc(sourceAsset->textures_count + 1);
    cgltf_material* materials = mStorage.materials.alloc(sourceAsset->materials_count);
    cgltf_primitive* prims = mStorage.prims.alloc(numPrims);
    cgltf_mesh* meshes = mStorage.meshes.alloc(sourceAsset->meshes_count);
    cgltf_node* nodes = mStorage.nodes.alloc(sourceAsset->nodes_count);
    cgltf_scene* scenes = mStorage.scenes.alloc(sourceAsset->scenes_count);
    cgltf_node** nodePointers = mStorage.nodePointers.alloc(nodePointersCount);
    char* pathString = (char*) mStorage.bufferData.alloc(texturePath.size() + 1);

    // Clone the nodes.
    for (size_t i = 0, len = sourceAsset->nodes_count; i < len; ++i) {
        auto& node = nodes[i] = sourceAsset->nodes[i];
        if (node.mesh) {
            node.mesh = meshes + (node.mesh - sourceAsset->meshes);
        }
    }

    // Clone the scenes.
    for (size_t i = 0, len = sourceAsset->scenes_count; i < len; ++i) {
        const auto& sourceScene = sourceAsset->scenes[i];
        auto& resultScene = scenes[i] = sourceScene;
        resultScene.nodes = nodePointers;
        for (size_t j = 0; j < sourceScene.nodes_count; ++j) {
            resultScene.nodes[j] = nodes + (sourceScene.nodes[j] - sourceAsset->nodes);
        }
        nodePointers += sourceScene.nodes_count;
    }

    // Clone the meshes and primitives; update the material pointers.
    for (size_t i = 0, len = sourceAsset->meshes_count; i < len; ++i) {
        const cgltf_mesh& sourceMesh = sourceAsset->meshes[i];
        cgltf_mesh& resultMesh = meshes[i] = sourceMesh;
        const cgltf_primitive& sourcePrim = sourceMesh.primitives[0];
        cgltf_primitive& resultPrim = prims[i] = sourcePrim;
        resultMesh.primitives = &resultPrim;
        auto& mat = resultPrim.material;
        mat = mat ? materials + (mat - sourceAsset->materials) : nullptr;
    }

    // Copy over existing textures and images.
    for (size_t tindex = 0, len = sourceAsset->textures_count; tindex < len; ++tindex) {
        const cgltf_texture& sourceTexture = sourceAsset->textures[tindex];
        cgltf_texture& resultTexture = textures[tindex];
        resultTexture = sourceTexture;
        resultTexture.image = images + (resultTexture.image - sourceAsset->images);
    }
    for (size_t iindex = 0, len = sourceAsset->images_count; iindex < len; ++iindex) {
        const cgltf_image& sourceImage = sourceAsset->images[iindex];
        cgltf_image& resultImage = images[iindex];
        resultImage = sourceImage;
    }

    // Populate the newly added texture and image.
    cgltf_texture& newTexture = textures[sourceAsset->textures_count];
    cgltf_image& newImage = images[sourceAsset->images_count];
    newTexture = { .image = &newImage };
    newImage = { .uri = pathString };
    strncpy(pathString, texturePath.c_str(), texturePath.size() + 1);

    // Clone the materials and update the texture pointers.
    for (size_t mindex = 0, len = sourceAsset->materials_count; mindex < len; ++mindex) {
        const cgltf_material& sourceMaterial = sourceAsset->materials[mindex];
        cgltf_material& resultMaterial = materials[mindex];
        resultMaterial = sourceMaterial;
        if (!resultMaterial.unlit) {
            resultMaterial.occlusion_texture = {
                .texture = &newTexture,
                .texcoord = gltfio::AssetPipeline::BAKED_UV_ATTRIB_INDEX,
                .scale = 1.0f,
                .has_transform = false
            };
        }
        auto& t0 = resultMaterial.pbr_metallic_roughness.base_color_texture.texture;
        auto& t1 = resultMaterial.pbr_metallic_roughness.metallic_roughness_texture.texture;
        auto& t2 = resultMaterial.pbr_specular_glossiness.diffuse_texture.texture;
        auto& t3 = resultMaterial.pbr_specular_glossiness.specular_glossiness_texture.texture;
        auto& t4 = resultMaterial.normal_texture.texture;
        auto& t5 = resultMaterial.emissive_texture.texture;
        t0 = t0 ? textures + (t0 - sourceAsset->textures) : nullptr;
        t1 = t1 ? textures + (t1 - sourceAsset->textures) : nullptr;
        t2 = t2 ? textures + (t2 - sourceAsset->textures) : nullptr;
        t3 = t3 ? textures + (t3 - sourceAsset->textures) : nullptr;
        t4 = t4 ? textures + (t4 - sourceAsset->textures) : nullptr;
        t5 = t5 ? textures + (t5 - sourceAsset->textures) : nullptr;
    }

    // Clone the high-level asset structure, then substitute some of the top-level lists.
    *resultAsset = *sourceAsset;
    resultAsset->images = images;
    resultAsset->textures = textures;
    resultAsset->materials = materials;
    resultAsset->meshes = meshes;
    resultAsset->nodes = nodes;
    resultAsset->scenes = scenes;
    resultAsset->scene = scenes + (sourceAsset->scene - sourceAsset->scenes);
    resultAsset->images_count++;
    resultAsset->textures_count++;
    return resultAsset;
}

void Pipeline::setOcclusionUri(cgltf_data* asset, const Path& texturePath) {
    if (!isFlattened(asset)) {
        utils::slog.e << "Only flattened assets can be modified." << utils::io::endl;
    }
    std::string uri = texturePath;
    char* pathString = (char*) mStorage.bufferData.alloc(uri.size() + 1);
    strncpy(pathString, uri.c_str(), uri.size() + 1);
    const cgltf_int kAttribIndex = gltfio::AssetPipeline::BAKED_UV_ATTRIB_INDEX;
    for (size_t mindex = 0, len = asset->materials_count; mindex < len; ++mindex) {
        auto& occlusion = asset->materials[mindex].occlusion_texture;
        if (occlusion.texture && occlusion.texcoord == kAttribIndex) {
            occlusion.texture->image->uri = pathString;
        }
    }
}

void Pipeline::setBaseColorUri(cgltf_data* asset, const Path& texturePath) {
    if (!isFlattened(asset)) {
        utils::slog.e << "Only flattened assets can be modified." << utils::io::endl;
    }
    std::string uri = texturePath;
    char* pathString = (char*) mStorage.bufferData.alloc(uri.size() + 1);
    strncpy(pathString, uri.c_str(), uri.size() + 1);
    for (size_t mindex = 0, len = asset->materials_count; mindex < len; ++mindex) {
        auto& texview = asset->materials[mindex].pbr_metallic_roughness.base_color_texture;
        if (texview.texture && texview.texture->image) {
            texview.texture->image->uri = pathString;
        }
    }
}

void Pipeline::retainSourceAsset(cgltf_data* asset) {
    mSourceAssets.push_back(asset);
}

Pipeline::~Pipeline() {
    for (auto asset : mSourceAssets) {
        cgltf_free(asset);
    }
}

} // anonymous namespace

namespace gltfio {

AssetPipeline::AssetPipeline() {
    mImpl = new Pipeline();
}

AssetPipeline::~AssetPipeline() {
    Pipeline* impl = (Pipeline*) mImpl;
    delete impl;
}

AssetHandle AssetPipeline::flatten(AssetHandle source, uint32_t flags) {
    Pipeline* impl = (Pipeline*) mImpl;
    const cgltf_data* asset = (const cgltf_data*) source;
    if (asset->animations_count > 0 || asset->skins_count > 0) {
        utils::slog.e << "Cannot flatten assets with animation or skinning." << utils::io::endl;
        return nullptr;
    }
    if (asset->buffers_count > 1) {
        asset = impl->flattenBuffers(asset);
    }
    asset = impl->flattenPrims(asset, flags);
    asset = impl->flattenBuffers(asset);
    return asset;
}

AssetHandle AssetPipeline::load(const utils::Path& fileOrDirectory) {
    utils::Path filename = fileOrDirectory;
    if (!filename.exists()) {
        utils::slog.e << "file " << filename << " not found!" << utils::io::endl;
        return nullptr;
    }
    if (filename.isDirectory()) {
        auto files = filename.listContents();
        for (auto file : files) {
            if (file.getExtension() == "gltf") {
                filename = file;
                break;
            }
        }
        if (filename.isDirectory()) {
            utils::slog.e << "no glTF file found in " << filename << utils::io::endl;
            return nullptr;
        }
    }

    // Parse the glTF file.
    cgltf_options options { cgltf_file_type_gltf };
    cgltf_data* sourceAsset;
    cgltf_result result = cgltf_parse_file(&options, filename.c_str(), &sourceAsset);
    if (result != cgltf_result_success) {
        return nullptr;
    }
    Pipeline* impl = (Pipeline*) mImpl;
    impl->retainSourceAsset(sourceAsset);

    // Load external resources.
    utils::Path abspath = filename.getAbsolutePath();
    if (cgltf_load_buffers(&options, sourceAsset, abspath.c_str()) != cgltf_result_success) {
        utils::slog.e << "Unable to load external buffers." << utils::io::endl;
        return nullptr;
    }

    return sourceAsset;
}

bool AssetPipeline::save(AssetHandle handle, const utils::Path& jsonPath,
        const utils::Path& binPath) {
    cgltf_data* asset = (cgltf_data*) handle;

    if (!isFlattened(asset)) {
        utils::slog.e << "Only flattened assets can be exported to disk." << utils::io::endl;
        return false;
    }

    std::string binName = binPath.getName();
    asset->buffers[0].uri = (char*) (binName.c_str());
    cgltf_options options { cgltf_file_type_gltf };
    if (cgltf_write_file(&options, jsonPath.c_str(), asset) != cgltf_result_success) {
        utils::slog.e << "Unable to write to " << jsonPath << utils::io::endl;
        asset->buffers[0].uri = nullptr;
        return false;
    }
    asset->buffers[0].uri = nullptr;

    FILE* binFile = fopen(binPath.c_str(), "wb");
    if (!binFile) {
        utils::slog.e << "Unable to write to " << binPath << utils::io::endl;
        return false;
    }
    fwrite((char*) asset->buffers[0].data, asset->buffers[0].size, 1, binFile);
    fclose(binFile);
    return true;
}

AssetHandle AssetPipeline::parameterize(AssetHandle source, int maxIterations) {
    Pipeline* impl = (Pipeline*) mImpl;
    return impl->parameterize((const cgltf_data*) source, maxIterations);
}

AssetHandle AssetPipeline::generatePreview(AssetHandle source, const Path& texture) {
    Pipeline* impl = (Pipeline*) mImpl;
    return impl->generatePreview((const cgltf_data*) source, texture);
}

AssetHandle AssetPipeline::replaceOcclusion(AssetHandle source, const Path& texture) {
    Pipeline* impl = (Pipeline*) mImpl;
    return impl->replaceOcclusion((const cgltf_data*) source, texture);
}

void AssetPipeline::setOcclusionUri(AssetHandle asset, const Path& texture) {
    Pipeline* impl = (Pipeline*) mImpl;
    impl->setOcclusionUri((cgltf_data*) asset, texture);
}

void AssetPipeline::setBaseColorUri(AssetHandle asset, const Path& texture) {
    Pipeline* impl = (Pipeline*) mImpl;
    impl->setBaseColorUri((cgltf_data*) asset, texture);
}

void AssetPipeline::bakeAmbientOcclusion(AssetHandle source, image::LinearImage target,
        const RenderOptions& options) {
    Pipeline* impl = (Pipeline*) mImpl;
    auto sourceAsset = (const cgltf_data*) source;
    if (!isFlattened(sourceAsset)) {
        utils::slog.e << "Only flattened assets can be baked." << utils::io::endl;
        return;
    }
    impl->bakeAmbientOcclusion(sourceAsset, target, options);
}

void AssetPipeline::renderAmbientOcclusion(AssetHandle source, image::LinearImage target,
        const filament::rays::SimpleCamera& camera, const RenderOptions& options) {
    Pipeline* impl = (Pipeline*) mImpl;
    auto sourceAsset = (const cgltf_data*) source;
    if (!isFlattened(sourceAsset)) {
        utils::slog.e << "Only flattened assets can be rendered." << utils::io::endl;
        return;
    }
    impl->renderAmbientOcclusion(sourceAsset, target, camera, options);
}

void AssetPipeline::bakeAllOutputs(AssetHandle source,
        image::LinearImage targets[filament::rays::OUTPUTPLANE_COUNT],
        const RenderOptions& options) {
    Pipeline* impl = (Pipeline*) mImpl;
    auto sourceAsset = (const cgltf_data*) source;
    if (!isFlattened(sourceAsset)) {
        utils::slog.e << "Only flattened assets can be baked." << utils::io::endl;
        return;
    }
    impl->bakeAllOutputs(sourceAsset, targets, options);
}

bool AssetPipeline::isFlattened(AssetHandle source) {
    return ::isFlattened((const cgltf_data*) source);
}

}  // namespace gltfio
