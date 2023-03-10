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

#include <memory>

#include <geometry/SurfaceOrientation.h>

using namespace filament::gltfio;
using namespace filament;
using namespace filament::math;

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
    std::unique_ptr<float3[]> morphDeltas;

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

    geometry::SurfaceOrientation::Builder sob;
    sob.vertexCount(vertexCount);

    // Allocate scratch space to store morph deltas.
    if (isMorphTarget) {
        morphDeltas.reset(new float3[vertexCount]);
    }

    // Convert normals into packed floats.
    if (auto baseNormalsInfo = baseAccessors[cgltf_attribute_type_normal]; baseNormalsInfo) {
        assert(baseNormalsInfo->count == vertexCount);
        assert(baseNormalsInfo->type == cgltf_type_vec3);
        unpackedNormals.reset(new float3[vertexCount]);
        cgltf_accessor_unpack_floats(baseNormalsInfo, &unpackedNormals[0].x, vertexCount * 3);
        if (auto mtNormalsInfo = morphTargetAccessors[cgltf_attribute_type_normal]) {
            cgltf_accessor_unpack_floats(mtNormalsInfo, &morphDeltas[0].x, vertexCount * 3);
            for (cgltf_size i = 0; i < vertexCount; i++) {
                unpackedNormals[i] += morphDeltas[i];
            }
        }
        sob.normals(unpackedNormals.get());
    }

    // Convert tangents into packed floats.
    if (auto baseTangentsInfo = baseAccessors[cgltf_attribute_type_tangent]; baseTangentsInfo) {
        assert(baseTangentsInfo->count == vertexCount);
        unpackedTangents.reset(new float4[vertexCount]);
        cgltf_accessor_unpack_floats(baseTangentsInfo, &unpackedTangents[0].x, vertexCount * 4);
        if (auto mtTangentsInfo = morphTargetAccessors[cgltf_attribute_type_tangent]) {
            cgltf_accessor_unpack_floats(mtTangentsInfo, &morphDeltas[0].x, vertexCount * 3);
            for (cgltf_size i = 0; i < vertexCount; i++) {
                unpackedTangents[i].xyz += morphDeltas[i];
            }
        }
        sob.tangents(unpackedTangents.get());
    }

    if (auto basePosInfo = baseAccessors[cgltf_attribute_type_position]; basePosInfo) {
        assert(basePosInfo->count == vertexCount && basePosInfo->type == cgltf_type_vec3);
        unpackedPositions.reset(new float3[vertexCount]);
        cgltf_accessor_unpack_floats(basePosInfo, &unpackedPositions[0].x, vertexCount * 3);
        sob.positions(unpackedPositions.get());
        if (auto mtPositionsInfo = morphTargetAccessors[cgltf_attribute_type_position]) {
            cgltf_accessor_unpack_floats(mtPositionsInfo, &morphDeltas[0].x, vertexCount * 3);
            for (cgltf_size i = 0; i < vertexCount; i++) {
                unpackedPositions[i] += morphDeltas[i];
            }
        }
    }

    const size_t triangleCount = prim.indices ? (prim.indices->count / 3) : (vertexCount / 3);
    unpackedTriangles.reset(new uint3[triangleCount]);

    if (prim.indices) {
        for (size_t tri = 0, j = 0; tri < triangleCount; ++tri) {
            auto& triangle = unpackedTriangles[tri];
            triangle.x = cgltf_accessor_read_index(prim.indices, j++);
            triangle.y = cgltf_accessor_read_index(prim.indices, j++);
            triangle.z = cgltf_accessor_read_index(prim.indices, j++);
        }
    } else {
        for (size_t tri = 0, j = 0; tri < triangleCount; ++tri) {
            auto& triangle = unpackedTriangles[tri];
            triangle.x = j++;
            triangle.y = j++;
            triangle.z = j++;
        }
    }

    sob.triangleCount(triangleCount);
    sob.triangles(unpackedTriangles.get());

    auto uvInfo = baseAccessors[cgltf_attribute_type_texcoord];
    if (uvInfo && uvInfo->count == vertexCount && uvInfo->type == cgltf_type_vec2) {
        unpackedTexCoords.reset(new float2[vertexCount]);
        cgltf_accessor_unpack_floats(uvInfo, &unpackedTexCoords[0].x, vertexCount * 2);
        sob.uvs(unpackedTexCoords.get());
    }

    // Compute surface orientation quaternions.
    params->out.results = (short4*) malloc(sizeof(short4) * vertexCount);
    geometry::SurfaceOrientation* helper = sob.build();
    helper->getQuats(params->out.results, vertexCount);
    delete helper;
}
