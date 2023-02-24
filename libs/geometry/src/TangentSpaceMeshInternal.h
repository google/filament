/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef TNT_GEOMETRY_TANGENTSPACEMESHIMPL_H
#define TNT_GEOMETRY_TANGENTSPACEMESHIMPL_H

#include <geometry/TangentSpaceMesh.h>

#include <math/mat3.h>
#include <math/norm.h>

namespace filament::geometry {

using namespace filament::math;
using Algorithm = TangentSpaceMesh::Algorithm;

struct TangentSpaceMeshInput {
    size_t vertexCount = 0;
    float3 const* normals = nullptr;
    float2 const* uvs = nullptr;
    float3 const* positions = nullptr;
    ushort3 const* triangles16 = nullptr;
    uint3 const* triangles32 = nullptr;

    size_t normalStride = 0;
    size_t uvStride = 0;
    size_t positionStride = 0;
    size_t triangleCount = 0;

    Algorithm algorithm;
};

struct TangentSpaceMeshOutput {
    Algorithm algorithm;

    size_t triangleCount = 0;
    size_t vertexCount = 0;

    quatf const* tangentSpace = nullptr;
    float2 const* uvs = nullptr;
    float3 const* positions = nullptr;
    uint3 const* triangles32 = nullptr;
    ushort3 const* triangles16 = nullptr;
};

template<typename InputType>
inline const InputType* pointerAdd(InputType const* ptr, size_t index, size_t stride) noexcept {
    return (InputType*) (((uint8_t const*) ptr) + (index * stride));
}

template<typename InputType>
inline InputType* pointerAdd(InputType* ptr, size_t index, size_t stride) noexcept {
    return (InputType*) (((uint8_t*) ptr) + (index * stride));
}

}// namespace filament::geometry

#endif//TNT_GEOMETRY_TANGENTSPACEMESHIMPL_H
