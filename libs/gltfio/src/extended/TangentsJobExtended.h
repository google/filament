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

#ifndef GLTFIO_TANGENTS_JOB_EXTENDED_H
#define GLTFIO_TANGENTS_JOB_EXTENDED_H

#include <gltfio/MaterialProvider.h> // for UvMap
#include <math/vec4.h>

#include <cgltf.h>

namespace filament::gltfio {

// Encapsulates a tangent-space transformation, which computes tangents (and maybe transform the
// mesh vertices/indices depending on the algorithm used). Input to this job can be the base mesh
// and attributes, or it could be a specific morph target, where offsets to the base mesh will be
// computed with respect to the morph target index.
struct TangentsJobExtended {
    static constexpr int kMorphTargetUnused = -1;

    // The inputs to the procedure. The prim is owned by the client, which should ensure that it
    // stays alive for the duration of the procedure.
    struct InputParams {
        cgltf_primitive const* prim;
        int morphTargetIndex = kMorphTargetUnused;
        UvMap uvmap;
    };

    // The outputs of the procedure. The results array gets malloc'd by the procedure, so clients
    // should remember to free it.
    struct OutputParams {
        size_t triangleCount = 0;
        math::uint3* triangles = nullptr;

        size_t vertexCount = 0;
        math::short4* tbn = nullptr;
        math::float2* uv0 = nullptr;
        math::float2* uv1 = nullptr;
        math::float3* positions = nullptr;
        math::ushort4* joints = nullptr;
        math::float4* weights = nullptr;
        math::float4* colors = nullptr;

        bool isEmpty() const {
            return !tbn && !uv0 && !uv1 && !positions && !joints && !weights && !colors;
        }
    };

    // Clients might want to track the jobs in an array, so the arguments are bundled into a struct.
    struct Params {
        InputParams in;
        OutputParams out;
        uint8_t jobType = 0;        
    };

    // Performs tangents generation synchronously. This can be invoked from inside a job if desired.
    // The parameters structure is owned by the client.
    static void run(Params* params);
};

} // namespace filament::gltfio

#endif // GLTFIO_TANGENTS_JOB_EXTENDED_H
