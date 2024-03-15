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

#ifndef GLTFIO_TANGENT_SPACE_MESH_WRAPPER_H
#define GLTFIO_TANGENT_SPACE_MESH_WRAPPER_H

#include <geometry/TangentSpaceMesh.h>

#include <math/vec4.h>

namespace filament::gltfio {

using namespace math;

// Wrapper around TangentSpaceMesh because in the case of unlit material, we do not need to go
// through TSM transformation, and we simply passthrough any given input as output.
struct TangentSpaceMeshWrapper {
    using AuxType = geometry::TangentSpaceMesh::AuxAttribute;

    struct Builder {
        struct Impl;

        Builder(bool isUnlit);

        Builder& vertexCount(size_t count) noexcept;
        Builder& normals(float3 const* normals) noexcept;
        Builder& tangents(float4 const* tangents) noexcept;
        Builder& uvs(float2 const* uvs) noexcept;
        Builder& positions(float3 const* positions) noexcept;
        Builder& triangleCount(size_t triangleCount) noexcept;
        Builder& triangles(uint3 const* triangles) noexcept;
        template<typename T>
        Builder& aux(AuxType type, T data);
        TangentSpaceMeshWrapper* build();

    private:
        Impl* mImpl;
    };

    explicit TangentSpaceMeshWrapper() = default;
    
    static void destroy(TangentSpaceMeshWrapper* mesh);

    float3* getPositions() noexcept;
    float2* getUVs() noexcept;
    short4* getQuats() noexcept;
    uint3* getTriangles();

    template<typename T>
    using is_supported_aux_t = typename std::enable_if<
            std::is_same<float2*, T>::value || std::is_same<float3*, T>::value ||
            std::is_same<float4*, T>::value || std::is_same<ushort3*, T>::value ||
            std::is_same<ushort4*, T>::value>::type;
    template<typename T, typename = is_supported_aux_t<T>>
    T getAux(AuxType attribute) noexcept;

    size_t getVertexCount() const noexcept;
    size_t getTriangleCount() const noexcept;

private:
    struct Impl;
    Impl* mImpl;

    friend struct Builder::Impl;
};

} // namespace filament

#endif // GLTFIO_TANGENTS_JOB_EXTENDED_H
