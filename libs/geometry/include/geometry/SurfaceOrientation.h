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

namespace filament {
namespace geometry {

struct OrientationBuilderImpl;
struct OrientationImpl;

/**
 * The surface orientation helper can be used to populate Filament-style TANGENTS buffers.
 */
class SurfaceOrientation {
public:

    /**
     * Constructs an immutable surface orientation helper.
     *
     * At a minimum, clients must supply a vertex count and normals buffer. They can supply data in
     * any of the following three combinations:
     *
     *   1. vec3 normals only (not recommended)
     *   2. vec3 normals + vec4 tangents (sign of W determines bitangent orientation)
     *   3. vec3 normals + vec2 uvs + vec3 positions + uint3 indices
     */
    class Builder {
    public:
        Builder();
        ~Builder() noexcept;

        /**
         * These two attributes are required. They are not passed into the constructor to force
         * calling code to be self-documenting.
         * @{
         */
        Builder& vertexCount(size_t vertexCount) noexcept;
        Builder& normals(const filament::math::float3*) noexcept;
        /** @} */

        Builder& tangents(const filament::math::float4*) noexcept;
        Builder& uvs(const filament::math::float2*) noexcept;
        Builder& positions(const filament::math::float3*) noexcept;

        Builder& triangleCount(size_t triangleCount) noexcept;
        Builder& triangles(const filament::math::uint3*) noexcept;
        Builder& triangles(const filament::math::ushort3*) noexcept;

        /**
         * Supplying explicit stride values is optional, they each default to the size of their
         * respective input attributes, e.g. sizeof(float3) for normals.
         * @{
         */
        Builder& normalStride(size_t numBytes) noexcept;
        Builder& tangentStride(size_t numBytes) noexcept;
        Builder& uvStride(size_t numBytes) noexcept;
        Builder& positionStride(size_t numBytes) noexcept;
        /** @} */

        /**
         * Generates quats or panics if the submitted data is an incomplete combination.
         */
        SurfaceOrientation* build();

    private:
        OrientationBuilderImpl* mImpl;
        Builder(const Builder&);
        Builder& operator=(const Builder&);
    };

    static void destroy(SurfaceOrientation* so);

    /**
     * Returns the vertex count.
     */
    size_t getVertexCount() const noexcept;

    /**
     * Converts quaternions into the desired output format and writes up to "nquats"
     * to the given output pointer. Normally nquats should be equal to the vertex count.
     * The optional stride is the desired quat-to-quat stride in bytes.
     * @{
     */
    void getQuats(filament::math::quatf* out, size_t nquats, size_t stride = 0) const noexcept;
    void getQuats(filament::math::short4* out, size_t nquats, size_t stride = 0) const noexcept;
    void getQuats(filament::math::quath* out, size_t nquats, size_t stride = 0) const noexcept;
    /** @} */

private:
    SurfaceOrientation(OrientationImpl*);
    ~SurfaceOrientation();
    OrientationImpl* mImpl;
    friend struct OrientationBuilderImpl;
};

} // namespace geometry
} // namespace filament

#endif // TNT_GEOMETRY_SURFACEORIENTATION_H
