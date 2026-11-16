/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "TangentsJob.h"

#include <geometry/SurfaceOrientation.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <memory>

using namespace filament::gltfio;
using namespace filament;
using namespace filament::math;

namespace {

const void* getDirectFloatPtr(const cgltf_accessor* accessor, cgltf_type expectedType,
        bool allowArbitraryStride, size_t* outStride) noexcept {
    if (!accessor || accessor->is_sparse || !accessor->buffer_view ||
            accessor->component_type != cgltf_component_type_r_32f ||
            accessor->type != expectedType) {
        return nullptr;
    }
    const uint8_t* data = cgltf_buffer_view_data(accessor->buffer_view);
    if (!data) {
        return nullptr;
    }
    const uint8_t* ptr = data + accessor->offset;
    if (reinterpret_cast<uintptr_t>(ptr) % alignof(float) != 0) {
        return nullptr;
    }
    const size_t tightStride = cgltf_num_components(expectedType) * sizeof(float);
    if (accessor->stride != tightStride) {
        if (!allowArbitraryStride || (accessor->stride % alignof(float) != 0)) {
            return nullptr;
        }
        *outStride = accessor->stride;
    } else {
        *outStride = 0;
    }
    return ptr;
}

} // anonymous namespace

// This procedure is designed to run in an isolated job.
void TangentsJob::run(Params* params) {
    const cgltf_primitive& prim = *params->in.prim;
    const int morphTargetIndex = params->in.morphTargetIndex;
    const bool isMorphTarget = morphTargetIndex != kMorphTargetUnused;

    // Extract the vertex count from the first attribute. All attributes must have the same count.
    assert(prim.attributes_count > 0);
    const cgltf_size vertexCount = prim.attributes[0].data->count;
    params->out.vertexCount = vertexCount;
    if (vertexCount == 0) {
        return;
    }

    // Declare storage for data that has been unpacked and converted from the source buffers.
    // This data needs to be held until after the SurfaceOrientation helper consumes it.
    // Not all of these will be required, so they get allocated lazily.
    std::unique_ptr<float3[]> unpackedNormals;
    std::unique_ptr<float4[]> unpackedTangents;
    std::unique_ptr<float3[]> unpackedPositions;
    std::unique_ptr<float2[]> unpackedTexCoords;
    std::unique_ptr<uint3[]> unpackedTriangles;
    std::unique_ptr<ushort3[]> unpackedTriangles16;
    std::unique_ptr<float[]> morphDeltas;

    // Build a mapping from cgltf_attribute_type to cgltf_accessor.
    const int NUM_ATTRIBUTES = cgltf_attribute_type_max_enum;
    const cgltf_accessor* baseAccessors[NUM_ATTRIBUTES] = {};
    const cgltf_accessor* morphTargetAccessors[NUM_ATTRIBUTES] = {};

    // Collect accessors for normals, tangents, etc. Note that we skip over attributes with
    // non-zero set indices likes TEXCOORD_1, TEXCOORD_2 to avoid overflowing the tiny arrays.
    // The SurfaceOrientation helper does not need them anyway.
    for (cgltf_size aindex = 0; aindex < prim.attributes_count; aindex++) {
        const cgltf_attribute& attr = prim.attributes[aindex];
        if (attr.index == 0) {
            baseAccessors[attr.type] = attr.data;
        }
    }
    if (isMorphTarget) {
        const cgltf_morph_target& morphTarget = prim.targets[morphTargetIndex];
        for (cgltf_size aindex = 0; aindex < morphTarget.attributes_count; aindex++) {
            const cgltf_attribute& attr = morphTarget.attributes[aindex];
            if (attr.index == 0) {
                assert(baseAccessors[attr.type] &&
                        "Morph target data has no corresponding base vertex data.");
                morphTargetAccessors[attr.type] = attr.data;
            }
        }
    }

    const cgltf_accessor* const baseNormalsInfo = baseAccessors[cgltf_attribute_type_normal];
    const cgltf_accessor* const baseTangentsInfo = baseAccessors[cgltf_attribute_type_tangent];
    const cgltf_accessor* const basePosInfo = baseAccessors[cgltf_attribute_type_position];
    const cgltf_accessor* const uvInfo = baseAccessors[cgltf_attribute_type_texcoord];

    const bool hasNormals = baseNormalsInfo != nullptr;
    const bool hasTangents = baseTangentsInfo != nullptr;
    const bool hasPositions = basePosInfo != nullptr;
    const bool hasUvs = uvInfo && uvInfo->count == vertexCount && uvInfo->type == cgltf_type_vec2;

    // SurfaceOrientation::Builder selects one of four mutually exclusive algorithms:
    //   1. normals + tangents -> buildWithSuppliedTangents (needs normals, tangents)
    //   2. normals + uvs + positions + triangles -> buildWithUvs (needs normals, uvs, positions, triangles)
    //   3. normals only -> buildWithNormalsOnly (needs normals)
    //   4. positions + triangles (no normals) -> buildWithFlatNormals (needs positions, triangles)
    // Only unpack and supply the attributes required by the selected path.
    const bool needsNormals = hasNormals;
    const bool needsTangents = hasNormals && hasTangents;
    const bool needsPositions = hasPositions && (!hasNormals || (!hasTangents && hasUvs));
    const bool needsUvs = needsPositions && hasNormals;
    const bool allowNormalTangentStride = !needsUvs;

    geometry::SurfaceOrientation::Builder sob;
    sob.vertexCount(vertexCount);

    // Allocate scratch space to store morph deltas when bulk unpacking is used.
    if (isMorphTarget) {
        auto const* mtNorm = needsNormals ? morphTargetAccessors[cgltf_attribute_type_normal] : nullptr;
        auto const* mtTan = needsTangents ? morphTargetAccessors[cgltf_attribute_type_tangent] : nullptr;
        auto const* mtPos = needsPositions ? morphTargetAccessors[cgltf_attribute_type_position] : nullptr;
        bool const needsVec3Scratch =
                (mtNorm && mtNorm->type == cgltf_type_vec3) ||
                (mtTan && mtTan->type == cgltf_type_vec3) ||
                (mtPos && mtPos->type == cgltf_type_vec3);
        bool const needsVec4Scratch = mtPos && mtPos->type == cgltf_type_vec4 && mtPos->is_sparse;
        if (needsVec4Scratch || needsVec3Scratch) {
            morphDeltas.reset(new float[vertexCount * (needsVec4Scratch ? 4 : 3)]());
        }
    }

    // Provide normals (in-place when possible, otherwise unpacked).
    if (needsNormals) {
        assert(baseNormalsInfo->count == vertexCount);
        assert(baseNormalsInfo->type == cgltf_type_vec3);
        auto const* mtNormalsInfo = morphTargetAccessors[cgltf_attribute_type_normal];
        const bool hasMorphNormals = mtNormalsInfo && mtNormalsInfo->type == cgltf_type_vec3;
        size_t normalStride = 0;
        const void* directNormals = !hasMorphNormals ? getDirectFloatPtr(
                baseNormalsInfo, cgltf_type_vec3, allowNormalTangentStride, &normalStride) : nullptr;
        if (directNormals) {
            sob.normals(static_cast<const float3*>(directNormals), normalStride);
        } else {
            unpackedNormals.reset(new float3[vertexCount]());
            cgltf_accessor_unpack_floats(baseNormalsInfo, &unpackedNormals[0].x, vertexCount * 3);
            if (hasMorphNormals) {
                cgltf_accessor_unpack_floats(mtNormalsInfo, morphDeltas.get(), vertexCount * 3);
                for (cgltf_size i = 0; i < vertexCount; i++) {
                    float const* d = morphDeltas.get() + i * 3;
                    unpackedNormals[i] += float3{ d[0], d[1], d[2] };
                }
            }
            sob.normals(unpackedNormals.get());
        }
    }

    // Provide tangents (in-place when possible, otherwise unpacked).
    if (needsTangents) {
        assert(baseTangentsInfo->count == vertexCount);
        auto const* mtTangentsInfo = morphTargetAccessors[cgltf_attribute_type_tangent];
        const bool hasMorphTangents = mtTangentsInfo && mtTangentsInfo->type == cgltf_type_vec3;
        size_t tangentStride = 0;
        const void* directTangents = !hasMorphTangents ? getDirectFloatPtr(
                baseTangentsInfo, cgltf_type_vec4, allowNormalTangentStride, &tangentStride) : nullptr;
        if (directTangents) {
            sob.tangents(static_cast<const float4*>(directTangents), tangentStride);
        } else {
            unpackedTangents.reset(new float4[vertexCount]());
            cgltf_accessor_unpack_floats(baseTangentsInfo, &unpackedTangents[0].x, vertexCount * 4);
            if (hasMorphTangents) {
                cgltf_accessor_unpack_floats(mtTangentsInfo, morphDeltas.get(), vertexCount * 3);
                for (cgltf_size i = 0; i < vertexCount; i++) {
                    float const* d = morphDeltas.get() + i * 3;
                    unpackedTangents[i].xyz += float3{ d[0], d[1], d[2] };
                }
            }
            sob.tangents(unpackedTangents.get());
        }
    }

    if (needsPositions) {
        assert(basePosInfo->count == vertexCount && basePosInfo->type == cgltf_type_vec3);
        auto const* mtPositionsInfo = morphTargetAccessors[cgltf_attribute_type_position];
        const bool hasMorphPositions = mtPositionsInfo &&
                (mtPositionsInfo->type == cgltf_type_vec3 || mtPositionsInfo->type == cgltf_type_vec4);
        size_t posStride = 0;
        const void* directPositions = !hasMorphPositions ? getDirectFloatPtr(
                basePosInfo, cgltf_type_vec3, false, &posStride) : nullptr;
        if (directPositions) {
            sob.positions(static_cast<const float3*>(directPositions));
        } else {
            unpackedPositions.reset(new float3[vertexCount]());
            cgltf_accessor_unpack_floats(basePosInfo, &unpackedPositions[0].x, vertexCount * 3);
            if (mtPositionsInfo) {
                if (mtPositionsInfo->type == cgltf_type_vec3) {
                    cgltf_accessor_unpack_floats(mtPositionsInfo, morphDeltas.get(), vertexCount * 3);
                    for (cgltf_size i = 0; i < vertexCount; i++) {
                        float const* d = morphDeltas.get() + i * 3;
                        unpackedPositions[i] += float3{ d[0], d[1], d[2] };
                    }
                } else if (mtPositionsInfo->type == cgltf_type_vec4) {
                    if (!mtPositionsInfo->is_sparse) {
                        float4 delta{};
                        cgltf_size const count = std::min(vertexCount, mtPositionsInfo->count);
                        for (cgltf_size i = 0; i < count; i++) {
                            if (cgltf_accessor_read_float(mtPositionsInfo, i, &delta.x, 4)) {
                                unpackedPositions[i] += delta.xyz;
                            }
                        }
                    } else {
                        cgltf_accessor_unpack_floats(mtPositionsInfo, morphDeltas.get(),
                                vertexCount * 4);
                        for (cgltf_size i = 0; i < vertexCount; i++) {
                            float const* d = morphDeltas.get() + i * 4;
                            unpackedPositions[i] += float3{ d[0], d[1], d[2] };
                        }
                    }
                }
            }
            sob.positions(unpackedPositions.get());
        }

        const size_t triangleCount = prim.indices ? (prim.indices->count / 3) : (vertexCount / 3);
        sob.triangleCount(triangleCount);

        if (prim.indices) {
            const cgltf_accessor* indices = prim.indices;
            const uint8_t* indexData = (!indices->is_sparse && indices->buffer_view) ?
                    cgltf_buffer_view_data(indices->buffer_view) : nullptr;
            const uint8_t* ptr = indexData ? (indexData + indices->offset) : nullptr;
            if (ptr && indices->component_type == cgltf_component_type_r_16u &&
                    indices->stride == sizeof(uint16_t) &&
                    (reinterpret_cast<uintptr_t>(ptr) % alignof(uint16_t) == 0)) {
                sob.triangles(reinterpret_cast<const ushort3*>(ptr));
            } else if (ptr && indices->component_type == cgltf_component_type_r_32u &&
                    indices->stride == sizeof(uint32_t) &&
                    (reinterpret_cast<uintptr_t>(ptr) % alignof(uint32_t) == 0)) {
                sob.triangles(reinterpret_cast<const uint3*>(ptr));
            } else if (ptr && indices->component_type == cgltf_component_type_r_8u) {
                unpackedTriangles16.reset(new ushort3[triangleCount]);
                cgltf_accessor_unpack_indices(indices, unpackedTriangles16.get(),
                        sizeof(uint16_t), triangleCount * 3);
                sob.triangles(unpackedTriangles16.get());
            } else {
                unpackedTriangles.reset(new uint3[triangleCount]);
                if (!cgltf_accessor_unpack_indices(indices, unpackedTriangles.get(),
                        sizeof(uint32_t), triangleCount * 3)) {
                    for (size_t tri = 0, j = 0; tri < triangleCount; ++tri) {
                        auto& triangle = unpackedTriangles[tri];
                        triangle.x = cgltf_accessor_read_index(indices, j++);
                        triangle.y = cgltf_accessor_read_index(indices, j++);
                        triangle.z = cgltf_accessor_read_index(indices, j++);
                    }
                }
                sob.triangles(unpackedTriangles.get());
            }
        } else {
            unpackedTriangles.reset(new uint3[triangleCount]);
            for (size_t tri = 0, j = 0; tri < triangleCount; ++tri) {
                auto& triangle = unpackedTriangles[tri];
                triangle.x = j++;
                triangle.y = j++;
                triangle.z = j++;
            }
            sob.triangles(unpackedTriangles.get());
        }

        if (needsUvs) {
            size_t uvStride = 0;
            const void* directUvs = getDirectFloatPtr(uvInfo, cgltf_type_vec2, false, &uvStride);
            if (directUvs) {
                sob.uvs(static_cast<const float2*>(directUvs));
            } else {
                unpackedTexCoords.reset(new float2[vertexCount]);
                cgltf_accessor_unpack_floats(uvInfo, &unpackedTexCoords[0].x, vertexCount * 2);
                sob.uvs(unpackedTexCoords.get());
            }
        }
    }

    // Compute surface orientation quaternions.
    params->out.results = (short4*) calloc(vertexCount, sizeof(short4));
    if (needsPositions || needsNormals) {
        if (geometry::SurfaceOrientation* helper = sob.build(); helper != nullptr) {
            helper->getQuats(params->out.results, vertexCount);
            delete helper;
        }
    }
}
