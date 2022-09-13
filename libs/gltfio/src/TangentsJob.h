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

#include <cgltf.h>

#include <math/vec4.h>

namespace filament {

class VertexBuffer;
class MorphTargetBuffer;

}

namespace filament::gltfio {

/**
 * Internal helper that examines a cgltf primitive and generates data suitable for Filament's
 * TANGENTS attribute. This has been designed to be run as a JobSystem job, but clients are not
 * required to do so.
 */
struct TangentsJob {
    static constexpr int kMorphTargetUnused = -1;

    // The inputs to the procedure. The prim is owned by the client, which should ensure that it
    // stays alive for the duration of the procedure.
    struct InputParams {
        const cgltf_primitive* prim;
        const int morphTargetIndex = kMorphTargetUnused;
    };

    // The context of the procedure. These fields are not used by the procedure but are provided as
    // a convenience to clients. You can think of this as a scratch space for clients.
    struct Context {
        VertexBuffer* const vb;
        MorphTargetBuffer* const tb;
        const uint8_t slot;
    };

    // The outputs of the procedure. The results array gets malloc'd by the procedure, so clients
    // should remember to free it.
    struct OutputParams {
        cgltf_size vertexCount;
        math::short4* results;
    };

    // Clients might want to track the jobs in an array, so the arguments are bundled into a struct.
    struct Params {
        InputParams in;
        Context context;
        OutputParams out;
    };

    // Performs tangents generation synchronously. This can be invoked from inside a job if desired.
    // The parameters structure is owned by the client.
    static void run(Params* params);
};

} // namespace filament::gltfio
