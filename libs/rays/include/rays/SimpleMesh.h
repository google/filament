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

#ifndef RAYS_SIMPLEMESH_H
#define RAYS_SIMPLEMESH_H

#include <stddef.h>
#include <stdint.h>

namespace filament {
namespace rays {

/**
 * SimpleMesh is a description of triangle-based geometry intended for the path tracer.
 *
 * Holds weak references to client-side data that must stay alive for the duration of the render.
 * Indices and positions must always be present. Normals and uvs are only required when baking.
 *
 * Indices must be 32-bit. Positions, normals, and uvs must all be float3. Note that even the uv's
 * must be float3, so please pad with zeroes.
 */
struct SimpleMesh {
    size_t numVertices;
    size_t numIndices;
    const float* positions;
    size_t positionsStride;
    const uint32_t* indices;
    const float* normals;
    size_t normalsStride;
    const float* uvs;
    size_t uvsStride;
};

} // namespace rays
} // namespace filament

#endif // RAYS_SIMPLEMESH_H
