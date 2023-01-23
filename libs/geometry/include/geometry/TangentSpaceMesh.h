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

#ifndef TNT_GEOMETRY_TANGENTSPACEMESH_H
#define TNT_GEOMETRY_TANGENTSPACEMESH_H

#include <math/quat.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <utils/compiler.h>

namespace filament {
namespace geometry {

struct TangentSpaceMeshInput;
struct TangentSpaceMeshOutput;

/**
 * For building Filament-style TANGENTS buffers given an input mesh.
 * Supersedes the implementation in SurfaceOrientation.h
 */
 /* WARNING: WORK-IN-PROGRESS, PLEASE DO NOT USE */
class UTILS_PUBLIC TangentSpaceMesh {
public:
    enum class AlgorithmHint : uint8_t {
        /*
         * Tries to select the best possible algorithm given the input. The corresponding algorithms
         * are detailed in the corresponding enums.
         *   INPUT                                  ALGORITHM
         *   -----------------------------------------------------------
         *   normals                                FRISVAD
         *   positions + indices                    FLAT_SHADING
         *   normals + tangents                     SIGN_OF_W
         *   normals + uvs + positions + indices    MIKKTSPACE
         */
        DEFAULT = 0,

        /*
         * --- mikktspace ---
         * Requires: normals + uvs + positions + indices
         * Reference: Mikkelsen, M., 2008. Simulation of wrinkled surfaces revisited.
         *            https://github.com/mmikk/MikkTSpace
         *            https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#meshes-overview
         * Note: Will remesh
         */
        MIKKTSPACE = 1,

        /*
         * --- Lengyel's method ---
         * Requires: normals + uvs + positions + indices
         * Reference: Lengyel, E., 2019. Foundations of Game Engine Development: Rendering. Terathon
         *     Software LLC.. (Chapter 7)
         */
        LENGYEL = 2,

        /*
         * --- Hughes-Moller method ---
         * Requires: normals
         * Reference: Möller, T. and Hughes, J.F., 1999. Efficiently building a matrix to rotate one
         *     vector to another. Journal of graphics tools, 4(4), pp.1-4.
         */
        HUGHES_MOLLER = 3,

        /*
         * --- Frisvad's method ---
         * Requires: normals
         * Reference: Frisvad, J.R., 2012. Building an orthonormal basis from a 3D unit vector
         *     without normalization. Journal of Graphics Tools, 16(3), pp.151-159.
         *     http://people.compute.dtu.dk/jerf/code/hairy/
         */
        FRISVAD = 4,

        /*
         * --- Flat Shading ---
         * Requires: positions + indices
         * Note: Will remesh
         */
        FLAT_SHADING = 5,

        /*
         * --- Sign of W ---
         * Requires: normals + tangents
         * Note: The sign of W determines the orientation of the bitangent.
         */
        SIGN_OF_W = 6
    };

    /*
     * Computation of the tangent space is intended to be synchronous (worked on the same thread).
     * Client is expected to keep the input immutable and in a good state for the duration of both
     * computation *and* query. That is, when querying the result of the tangent spaces, part of the
     * result might depend on the input data.
     */
    class Builder {
    public:
        Builder() noexcept;
        ~Builder() noexcept;
        Builder(Builder&& that) noexcept;
        Builder& operator=(Builder&& that) noexcept;

        Builder& vertexCount(size_t vertexCount) noexcept;

        Builder& normals(const filament::math::float3*, size_t stride = 0) noexcept;
        Builder& tangents(const filament::math::float4*, size_t stride = 0) noexcept;
        Builder& uvs(const filament::math::float2*, size_t stride = 0) noexcept;
        Builder& positions(const filament::math::float3*, size_t stride = 0) noexcept;

        Builder& triangleCount(size_t triangleCount) noexcept;
        Builder& triangles(const filament::math::uint3*) noexcept;
        Builder& triangles(const filament::math::ushort3*) noexcept;

        Builder& algorithmHint(AlgorithmHint) noexcept;

        TangentSpaceMesh* build();

    private:
        Builder(const Builder&) = delete;
        Builder& operator=(const Builder&) = delete;

        TangentSpaceMesh* mMesh;
    };

    ~TangentSpaceMesh() noexcept;
    TangentSpaceMesh(TangentSpaceMesh&& that) noexcept;
    TangentSpaceMesh& operator=(TangentSpaceMesh&& that) noexcept;

    size_t getVertexCount() const noexcept;

    /*
     * The following methods for retrieving vertex-correlated data assumes that the *out* param
     * provided by the client is at least of *getVertexCount()* length (accounting for stride).
     *
     * If the chosen algorithm did not remesh the input, the client is advised to just use the
     * data they provided instead of querying.  For example, if the chosen method is FRISVAD, then
     * the client should not need to call "getPositions". We will simply copy from the input
     * *positions* in that case.
     *
     * If the client calls getPositions and positions were not provided as input, we will throw
     * and exception. Similar behavior will apply to UVs.
     */
    void getPositions(filament::math::float3* out, size_t stride = 0) const;
    void getUVs(filament::math::float2* out, size_t stride = 0) const;
    void getQuats(filament::math::quatf* out, size_t stride = 0) const noexcept;
    void getQuats(filament::math::short4* out, size_t stride = 0) const noexcept;
    void getQuats(filament::math::quath* out, size_t stride = 0) const noexcept;

    size_t getTriangleCount() const;

    /*
     * The following methods for retrieving triangles assumes that the *out* param provided by the
     * client is at least of *getTriangleCount()* length.
     *
     * If the chosen algorithm did not remesh the input, the client is advised to just use the
     * indices they provided instead of querying.  For example, if the chosen method is FRISVAD,
     * then calling *getTriangles()* will be simply copying from the input *triangles*.
     *
     * If the client calls getTriangles() and triangles were not provided as input, we will throw
     * and exception.
     */
    void getTriangles(filament::math::uint3* out) const;
    void getTriangles(filament::math::ushort3* out) const;

private:
    TangentSpaceMesh() noexcept;
    TangentSpaceMesh(const TangentSpaceMesh&) = delete;
    TangentSpaceMesh& operator=(const TangentSpaceMesh&) = delete;
    TangentSpaceMeshInput* mInput;
    TangentSpaceMeshOutput* mOutput;

    friend class Builder;
};

} // namespace geometry
} // namespace filament

#endif //TNT_GEOMETRY_TANGENTSPACEMESH_H
