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

#ifndef TNT_GEOMETRY_SURFACEORIENTATION_H
#define TNT_GEOMETRY_SURFACEORIENTATION_H

#include <math/quat.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/compiler.h>

namespace filament {

/**
 * Mesh-related utilities.
 */
namespace geometry {

struct OrientationBuilderImpl;
struct OrientationImpl;

/**
 * The surface orientation helper can be used to populate Filament-style TANGENTS buffers.
 */
class UTILS_PUBLIC SurfaceOrientation {
public:

    /**
     * The Builder is used to construct an immutable surface orientation helper.
     *
     * Clients provide pointers into their own data, which is synchronously consumed during build().
     * At a minimum, clients must supply a vertex count. They can supply data in any of the
     * following combinations:
     *
     *   1. normals only ........................... not recommended, selects arbitrary orientation
     *   2. normals + tangents ..................... sign of W determines bitangent orientation
     *   3. normals + uvs + positions + indices .... selects Lengyel’s Method
     *   4. positions + indices .................... generates normals for flat shading only
     *
     * Additionally, the client-side data has the following type constraints:
     *
     *  - Normals must be float3
     *  - Tangents must be float4
     *  - UVs must be float2
     *  - Positions must be float3
     *  - Triangles must be uint3 or ushort3
     *
     * Currently, mikktspace is not supported because it requires re-indexing the mesh. Instead
     * we use the method described by Eric Lengyel in "Foundations of Game Engine Development"
     * (Volume 2, Chapter 7).
     */
    class Builder {
    public:
        Builder() noexcept;
        ~Builder() noexcept;
        Builder(Builder&& that) noexcept;
        Builder& operator=(Builder&& that) noexcept;

        /**
         * This attribute is required.
         */
        Builder& vertexCount(size_t vertexCount) noexcept;

        Builder& normals(const filament::math::float3*, size_t stride = 0) noexcept;
        Builder& tangents(const filament::math::float4*, size_t stride = 0) noexcept;
        Builder& uvs(const filament::math::float2*, size_t stride = 0) noexcept;
        Builder& positions(const filament::math::float3*, size_t stride = 0) noexcept;

        Builder& triangleCount(size_t triangleCount) noexcept;
        Builder& triangles(const filament::math::uint3*) noexcept;
        Builder& triangles(const filament::math::ushort3*) noexcept;

        /**
         * Generates quats or returns null if the submitted data is an incomplete combination.
         */
        SurfaceOrientation* build();

    private:
        OrientationBuilderImpl* mImpl;
        Builder(const Builder&) = delete;
        Builder& operator=(const Builder&) = delete;
    };

    ~SurfaceOrientation() noexcept;
    SurfaceOrientation(SurfaceOrientation&& that) noexcept;
    SurfaceOrientation& operator=(SurfaceOrientation&& that) noexcept;

    /**
     * Returns the vertex count.
     */
    size_t getVertexCount() const noexcept;

    /**
     * Converts quaternions into the desired output format and writes up to "quatCount"
     * to the given output pointer. Normally quatCount should be equal to the vertex count.
     * The optional stride is the desired quat-to-quat stride in bytes.
     * @{
     */
    void getQuats(filament::math::quatf* out, size_t quatCount, size_t stride = 0) const noexcept;
    void getQuats(filament::math::short4* out, size_t quatCount, size_t stride = 0) const noexcept;
    void getQuats(filament::math::quath* out, size_t quatCount, size_t stride = 0) const noexcept;
    /**
     * @}
     */

private:
    SurfaceOrientation(OrientationImpl*) noexcept;
    SurfaceOrientation(const SurfaceOrientation&) = delete;
    SurfaceOrientation& operator=(const SurfaceOrientation&) = delete;
    OrientationImpl* mImpl;
    friend struct OrientationBuilderImpl;
};

} // namespace geometry
} // namespace filament

#endif // TNT_GEOMETRY_SURFACEORIENTATION_H
