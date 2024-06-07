/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "AssetLoaderExtended.h"

#include "../DracoCache.h"
#include "../FFilamentAsset.h"
#include "../GltfEnums.h"
#include "../Utility.h"
#include "TangentsJobExtended.h"

#include <filament/BufferObject.h>

#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/Panic.h>

#include <cgltf.h>

#include <unordered_map>
#include <variant>

namespace filament::gltfio {

namespace {

constexpr uint8_t const VERTEX_JOB = 0x1;
constexpr uint8_t const INDEX_JOB = 0x2;
constexpr uint8_t const MORPH_TARGET_JOB = 0x4;

constexpr int const GENERATED_0 = FFilamentAsset::ResourceInfoExtended::GENERATED_0_INDEX;
constexpr int const GENERATED_1 = FFilamentAsset::ResourceInfoExtended::GENERATED_1_INDEX;

using BufferSlot = AssetLoaderExtended::BufferSlot;
using BufferType = std::variant<short4*, ushort4*, float2*, float3*, float4*>;

struct AttributeHash {
    size_t operator()(Attribute const& key) const {
        size_t h1 = std::hash<uint64_t>{}((uint64_t) key.type);
        size_t h2 = std::hash<uint64_t>{}((uint64_t) key.index);
        return h1 ^ (h2 << 1);
    }
};

struct AttributeEqual {
    bool operator()(Attribute const& lhs, Attribute const& rhs) const {
        return lhs.type == rhs.type && lhs.index == rhs.index;
    }
};

using AttributesMap =
        std::unordered_map<Attribute, FilamentAttribute, AttributeHash, AttributeEqual>;

inline std::tuple<VertexBuffer::AttributeType, size_t, void*> getVertexBundle(
        VertexAttribute attrib, TangentsJobExtended::OutputParams& out) {
    VertexBuffer::AttributeType type;
    size_t byteCount = 0;
    void* data = nullptr;
    switch (attrib) {
        case VertexAttribute::POSITION:
            type = VertexBuffer::AttributeType::FLOAT3;
            byteCount = sizeof(float3);
            data = out.positions;
            out.positions = nullptr;
            break;
        case VertexAttribute::TANGENTS:
            type = VertexBuffer::AttributeType::SHORT4;
            byteCount = sizeof(short4);
            data = out.tbn;
            out.tbn = nullptr;
            break;
        case VertexAttribute::COLOR:
            type = VertexBuffer::AttributeType::FLOAT4;
            byteCount = sizeof(float4);
            data = out.colors;
            out.colors = nullptr;
            break;
        case VertexAttribute::UV0:
            type = VertexBuffer::AttributeType::FLOAT2;
            byteCount = sizeof(float2);
            data = out.uv0;
            out.uv0 = nullptr;
            break;
        case VertexAttribute::UV1:
            type = VertexBuffer::AttributeType::FLOAT2;
            byteCount = sizeof(float2);
            data = out.uv1;
            out.uv1 = nullptr;
            break;
        case VertexAttribute::BONE_INDICES:
            type = VertexBuffer::AttributeType::USHORT4;
            byteCount = sizeof(ushort4);
            data = out.joints;
            out.joints = nullptr;
            break;
        case VertexAttribute::BONE_WEIGHTS:
            type = VertexBuffer::AttributeType::FLOAT4;
            byteCount = sizeof(float4);
            data = out.weights;
            out.weights = nullptr;
            break;
        default:
            PANIC_POSTCONDITION("Unexpected vertex attribute %d", static_cast<int>(attrib));
    }
    return {type, byteCount, data};
}

// This will run the jobs to create tangent spaces if necessary, or simply forward the data if the
// input does not require processing. The output is a list of buffers that will be uploaded in the
// ResourceLoader.
std::vector<BufferSlot> computeGeometries(cgltf_primitive const* prim, uint8_t const jobType,
        AttributesMap const& attributesMap, std::vector<int> const& morphTargets, UvMap const& uvmap,
        filament::Engine* engine) {

    bool const isUnlit = prim->material ? prim->material->unlit : false;

    using Params = TangentsJobExtended::Params;

    std::unordered_map<int, Params> jobs;
    auto getJob = [&jobs](int key) -> Params& {
        return jobs.try_emplace(key).first->second;
    };

    // Create a job description for each triangle-based primitive.
    // Collect all TANGENT vertex attribute slots that need to be populated.
    if ((jobType & VERTEX_JOB) != 0) {
        auto& job = getJob(TangentsJobExtended::kMorphTargetUnused);
        job.in = {
                .prim = prim,
                .uvmap = uvmap,
        };
        job.jobType |= VERTEX_JOB;
    }
    if ((jobType & INDEX_JOB) != 0) {
        auto& job = getJob(TangentsJobExtended::kMorphTargetUnused);
        job.in = {
                .prim = prim,
                .uvmap = uvmap,
        };
        job.jobType |= INDEX_JOB;
    }
    for (auto const target: morphTargets) {
        auto& job = getJob(target);
        job.jobType = MORPH_TARGET_JOB;
        job.in = {
                .prim = prim,
                .morphTargetIndex = target,
        };
    }

    utils::JobSystem& js = engine->getJobSystem();
    utils::JobSystem::Job* parent = js.createJob();
    for (auto& [key, params]: jobs) {
        js.run(utils::jobs::createJob(js, parent,
                [pptr = &params] { TangentsJobExtended::run(pptr); }));
    }
    js.runAndWait(parent);

    std::vector<BufferSlot> slots;

    struct MorphTargetOut {
        int morphTarget;
        float3* positions;
        short4* tbn;
        size_t vertexCount;
    };
    std::vector<MorphTargetOut> morphTargetOuts;

    for (auto& [key, params]: jobs) {
        uint8_t const jobType = params.jobType;
        TangentsJobExtended::OutputParams& out = params.out;
        size_t const vertexCount = out.vertexCount;

        if ((jobType & VERTEX_JOB) != 0) {
            auto vertexBufferBuilder =
                    VertexBuffer::Builder().enableBufferObjects().vertexCount(vertexCount);

            std::vector<BufferSlot> vslots;
            bool slottedTangent = false;
            int maxSlot = 0;
            for (auto [cgltfAttr, filamentAttr]: attributesMap) {
                auto const [cattr, expectedIndex] = cgltfAttr;
                auto const [vattr, slot] = filamentAttr;
                auto const [type, byteCount, data] = getVertexBundle(vattr, out);

                vertexBufferBuilder.attribute(vattr, slot, type);

                // Here we generate data if needed.
                if (expectedIndex == GENERATED_0 || expectedIndex == GENERATED_1) {
                    // We should free `data` here because it's not being passed on to ResourceLoader.
                    if (data) {
                        free(data);
                    }

                    size_t const requiredSize = byteCount * vertexCount;
                    auto gendata = (uint8_t*) malloc(requiredSize);

                    if (vattr == filament::VertexAttribute::COLOR) {
                        // Assume white as the default if colors need to be generated.
                        float4* dataf = (float4*) gendata;
                        for (size_t i = 0; i < vertexCount; ++i) {
                            dataf[i] = float4(1.0, 1.0, 1.0, 1.0f);
                        }
                    } else {
                        memset(gendata, 0xff, requiredSize);
                    }

                    vslots.push_back({
                        .slot = slot,
                        .sizeInBytes = requiredSize,
                        .data = gendata,
                    });
                } else {
                    // Note that normalization is not necessary because we always convert the input.
                    if (vattr == filament::VertexAttribute::TANGENTS) {
                        vertexBufferBuilder.normalized(vattr);
                        slottedTangent = true;
                    }
                    vslots.push_back({
                        .slot = slot,
                        .sizeInBytes = byteCount * vertexCount,
                        .data = data,
                    });
                }
                maxSlot = std::max(maxSlot, slot);
            }

            // Tangent is always computed for lit.
            if (!slottedTangent && !isUnlit) {
                auto const slot = maxSlot + 1;
                auto const vattr = filament::VertexAttribute::TANGENTS;
                auto const [type, byteCount, data] = getVertexBundle(vattr, out);
                vertexBufferBuilder.attribute(vattr, slot, type);
                vertexBufferBuilder.normalized(vattr);
                vslots.push_back({
                        .slot = slot,
                        .sizeInBytes = byteCount * vertexCount,
                        .data = data,
                });
            }

            assert_invariant(!vslots.empty());
            vertexBufferBuilder.bufferCount(vslots.size());
            auto vertexBuffer = vertexBufferBuilder.build(*engine);
            std::for_each(vslots.begin(), vslots.end(),
                    [vertexBuffer](BufferSlot& slot) { slot.vertices = vertexBuffer; });
            slots.insert(slots.end(), vslots.begin(), vslots.end());
        }
        if ((jobType & INDEX_JOB) != 0) {
            auto indexBuffer = IndexBuffer::Builder()
                                       .indexCount(out.triangleCount * 3)
                                       .bufferType(IndexBuffer::IndexType::UINT)
                                       .build(*engine);

            slots.push_back({
                    .indices = indexBuffer,
                    .sizeInBytes = out.triangleCount * 3 * 4,
                    .data = out.triangles,
            });
            out.triangles = nullptr;
        }
        if ((jobType & MORPH_TARGET_JOB) != 0) {
            morphTargetOuts.push_back({
                    .morphTarget = params.in.morphTargetIndex,
                    .positions = out.positions,
                    .tbn = out.tbn,
                    .vertexCount = vertexCount,
            });
            out.positions = nullptr;
            out.tbn = nullptr;
        }

        // We should have passed ownership of all allocation to other parties.
        assert_invariant(out.isEmpty());
    }

    if (!morphTargets.empty()) {
        auto const vertexCount = morphTargetOuts[0].vertexCount;
        MorphTargetBuffer* buffer = MorphTargetBuffer::Builder()
                                            .count(morphTargets.size())
                                            .vertexCount(vertexCount)
                                            .build(*engine);
        for (auto target: morphTargetOuts) {
            assert_invariant(target.vertexCount == vertexCount);
            slots.push_back({.target = buffer,
                    .slot = target.morphTarget,
                    .targetData = {
                            .tbn = target.tbn,
                            .positions = target.positions,
                    }});
        }
    }
    return slots;
}

} // anonymous namespace

// The first portion of this function prepares the computation of geometries associated with one
// cgltf primitive by transforming types into Filament associated (or gltfio internal) types. If the
// input mesh is meshopt compressed or is in the Draco format, then it will be first transformed
// into the uncompressed version and then the geometries (tangents etc) will be computed.
bool AssetLoaderExtended::createPrimitive(Input* input, Output* out,
        std::vector<BufferSlot>& outSlots) {
    auto gltf = input->gltf;
    auto prim = input->prim;
    auto name = input->name;

    bool const isUnlit = prim->material ? prim->material->unlit : false;
    uint8_t jobType = 0;

    // In glTF, each primitive may or may not have an index buffer.
    const cgltf_accessor* indexAccessor = prim->indices;
    if (indexAccessor || prim->attributes_count > 0) {
        IndexBuffer::IndexType indexType;
        if (indexAccessor && !getIndexType(indexAccessor->component_type, &indexType)) {
            utils::slog.e << "Unrecognized index type in " << name << utils::io::endl;
            return false;
        }
        jobType |= INDEX_JOB;
    }

    jobType |= VERTEX_JOB;

    AttributesMap attributesMap;
    bool hasUv0 = false, hasUv1 = false, hasVertexColor = false, hasNormals = false;
    int slotCount = 0;

    for (cgltf_size aindex = 0; aindex < prim->attributes_count; aindex++) {
        cgltf_attribute const attribute = prim->attributes[aindex];
        int const index = attribute.index;
        cgltf_attribute_type const atype = attribute.type;
        cgltf_accessor const* accessor = attribute.data;

        Attribute const cattr{atype, index};

        // At a minimum, surface orientation requires normals to be present in the source data.
        // Here we re-purpose the normals slot to point to the quats that get computed later.
        if (atype == cgltf_attribute_type_normal) {
            if (isUnlit) continue;
            if (!hasNormals) {
                FilamentAttribute const fattr { VertexAttribute::TANGENTS, slotCount++ };
                hasNormals = true;
                attributesMap[cattr] = fattr;
            }
            continue;
        }

        if (atype == cgltf_attribute_type_tangent) {
            if (isUnlit) continue;
            if (!hasNormals) {
                FilamentAttribute const fattr { VertexAttribute::TANGENTS, slotCount++ };
                hasNormals = true;
                attributesMap[cattr] = fattr;
            }
            continue;
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
            UvSet uvset = out->uvmap[index];
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
                    if (!hasUv0 && getNumUvSets(out->uvmap) == 0) {
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

        if (atype == cgltf_attribute_type_color) {
            hasVertexColor = true;
        }

        // The positions accessor is required to have min/max properties, use them to expand
        // the bounding box for this primitive.
        if (atype == cgltf_attribute_type_position) {
            const float* minp = &accessor->min[0];
            const float* maxp = &accessor->max[0];
            out->aabb.min = min(out->aabb.min, float3(minp[0], minp[1], minp[2]));
            out->aabb.max = max(out->aabb.max, float3(maxp[0], maxp[1], maxp[2]));
        }

        if (VertexBuffer::AttributeType fatype, actualType;
                !getElementType(accessor->type, accessor->component_type, &fatype, &actualType)) {
            utils::slog.e << "Unsupported accessor type in " << name << utils::io::endl;
            return false;
        }

        attributesMap[cattr] = { semantic, slotCount++ };

        if (accessor->count == 0) {
            utils::slog.e << "Empty vertex buffer in " << name << utils::io::endl;
            return false;
        }
    }

    cgltf_size targetsCount = prim->targets_count;
    if (targetsCount > MAX_MORPH_TARGETS) {
        utils::slog.w << "WARNING: Exceeded max morph target count of " << MAX_MORPH_TARGETS
                      << utils::io::endl;
        targetsCount = MAX_MORPH_TARGETS;
    }

    // A set of morph targets to generate tangents for.
    std::vector<int> morphTargets;

    Aabb const baseAabb(out->aabb);
    for (cgltf_size targetIndex = 0; targetIndex < targetsCount; targetIndex++) {
        bool morphTargetHasNormals = false;
        cgltf_morph_target const& target = prim->targets[targetIndex];
        for (cgltf_size aindex = 0; aindex < target.attributes_count; aindex++) {
            cgltf_attribute const& attribute = target.attributes[aindex];
            cgltf_accessor const* accessor = attribute.data;
            cgltf_attribute_type const atype = attribute.type;

            if (atype != cgltf_attribute_type_position && atype != cgltf_attribute_type_normal &&
                    atype != cgltf_attribute_type_tangent) {
                utils::slog.e << "Only positions, normals, and tangents can be morphed."
                              << " type=" << static_cast<int>(atype) << utils::io::endl;
                return false;
            }

            if (VertexBuffer::AttributeType fatype, actualType; !getElementType(accessor->type,
                        accessor->component_type, &fatype, &actualType)) {
                utils::slog.e << "Unsupported accessor type in " << name << utils::io::endl;
                return false;
            }

            if (atype == cgltf_attribute_type_position && accessor->has_min && accessor->has_max) {
                Aabb targetAabb(baseAabb);
                float const* minp = &accessor->min[0];
                float const* maxp = &accessor->max[0];

                // We assume that the range of morph target weight is [0, 1].
                targetAabb.min += float3(minp[0], minp[1], minp[2]);
                targetAabb.max += float3(maxp[0], maxp[1], maxp[2]);

                out->aabb.min = min(out->aabb.min, targetAabb.min);
                out->aabb.max = max(out->aabb.max, targetAabb.max);
            }

            if (atype == cgltf_attribute_type_tangent) {
                morphTargetHasNormals = true;
                morphTargets.push_back(targetIndex);
            }
        }
        // Generate flat normals if necessary.
        if (!morphTargetHasNormals && prim->material && !prim->material->unlit) {
            morphTargets.push_back(targetIndex);
        }
    }

    // We provide a single dummy buffer (filled with 0xff) for all unfulfilled vertex requirements.
    // The color data should be a sequence of normalized UBYTE4, so dummy UVs are USHORT2 to make
    // the sizes match.
    if (mMaterials.needsDummyData(VertexAttribute::UV0) && !hasUv0) {
        attributesMap[{cgltf_attribute_type_texcoord, GENERATED_0}] = {VertexAttribute::UV0,
                slotCount++};
    }

    if (mMaterials.needsDummyData(VertexAttribute::UV1) && !hasUv1) {
        attributesMap[{cgltf_attribute_type_texcoord, GENERATED_1}] = {VertexAttribute::UV1,
                slotCount++};
    }

    if (mMaterials.needsDummyData(VertexAttribute::COLOR) && !hasVertexColor) {
        attributesMap[{cgltf_attribute_type_color, GENERATED_0}] = {VertexAttribute::COLOR,
                slotCount++};
    }

    int numUvSets = getNumUvSets(out->uvmap);
    if (!hasUv0 && numUvSets > 0) {
        attributesMap[{cgltf_attribute_type_texcoord, GENERATED_0}] = {VertexAttribute::UV0,
                slotCount++};
    }

    if (!hasUv1 && numUvSets > 1) {
        utils::slog.w << "Missing UV1 data in " << name << utils::io::endl;
        attributesMap[{cgltf_attribute_type_texcoord, GENERATED_1}] = {VertexAttribute::UV1,
                slotCount++};
    }

    if (!utility::loadCgltfBuffers(gltf, mGltfPath.c_str(), mUriDataCache)) {
        return false;
    }

    utility::decodeDracoMeshes(gltf, prim, input->dracoCache);
    utility::decodeMeshoptCompression(gltf);

    auto slots = computeGeometries(prim, jobType, attributesMap, morphTargets, out->uvmap, mEngine);

    for (auto slot: slots) {
        if (slot.vertices) {
            assert_invariant(!out->vertices || out->vertices == slot.vertices);
            out->vertices = slot.vertices;
        }
        if (slot.indices) {
            assert_invariant(!out->indices || out->indices == slot.indices);
            out->indices = slot.indices;
        }
        // FIXME: repair morphing
        assert_invariant(!slot.target);
//        if (slot.target) {
//            assert_invariant(!out->targets || out->targets == slot.target);
//            out->targets = slot.target;
//        }
    }

    outSlots.insert(outSlots.end(), slots.begin(), slots.end());
    return true;
}

}// namespace filament::gltfio
