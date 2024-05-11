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

#include <variant>

namespace filament {
namespace geometry {

struct TangentSpaceMeshInput;
struct TangentSpaceMeshOutput;

 /**
 * This class builds Filament-style TANGENTS buffers given an input mesh.
 *
 * This class enables the client to chose between several algorithms. The client can retrieve the
 * result through the `get` methods on the class. If the chosen algorithm did not remesh the input,
 * the client is advised to just use the data they provided instead of querying.  For example, if
 * the chosen method is Algorithm::FRISVAD, then the client should not need to call getPositions().
 * We will simply copy from the input `positions` in that case.
 *
 * If the client calls getPositions() and positions were not provided as input, we will throw
 * and exception. Similar behavior will apply to UVs.
 *
 * This class supersedes the implementation in SurfaceOrientation.h
 */
class TangentSpaceMesh {
public:
    enum class Algorithm : uint8_t {
        /**
         * default
         *
         * Tries to select the best possible algorithm given the input. The corresponding algorithms
         * are detailed in the corresponding enums.
         * <pre>
         *   INPUT                                  ALGORITHM
         *   -----------------------------------------------------------
         *   normals                                FRISVAD
         *   positions + indices                    FLAT_SHADING
         *   normals + uvs + positions + indices    MIKKTSPACE
         * </pre>
         */
        DEFAULT = 0,

        /**
         * mikktspace
         *
         * **Requires**: `normals + uvs + positions + indices` <br/>
         * **Reference**:
         *    - Mikkelsen, M., 2008. Simulation of wrinkled surfaces revisited.
         *    - https://github.com/mmikk/MikkTSpace
         *    - https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#meshes-overview
         *
         * **Note**: Will remesh
         */
        MIKKTSPACE = 1,

        /**
         * Lengyel's method
         *
         * **Requires**: `normals + uvs + positions + indices` <br/>
         * **Reference**: Lengyel, E., 2019. Foundations of Game Engine Development: Rendering. Terathon
         *     Software LLC.. (Chapter 7)
         */
        LENGYEL = 2,

        /**
         * Hughes-Moller method
         *
         * **Requires**: `normals` <br/>
         * **Reference**:
         *     - Hughes, J.F. and Moller, T., 1999. Building an orthonormal basis from a unit
         *       vector. journal of graphics tools, 4(4), pp.33-35.
         *     - Parker, S.G., Bigler, J., Dietrich, A., Friedrich, H., Hoberock, J., Luebke, D.,
         *       McAllister, D., McGuire, M., Morley, K., Robison, A. and Stich, M., 2010.
         *       Optix: a general purpose ray tracing engine. Acm transactions on graphics (tog),
         *       29(4), pp.1-13.
         * **Note**: We implement the Optix variant, which is documented in the second reference above.
         */
        HUGHES_MOLLER = 3,

        /**
         * Frisvad's method
         *
         * **Requires**: `normals` <br/>
         * **Reference**:
         *     - Frisvad, J.R., 2012. Building an orthonormal basis from a 3D unit vector without
         *       normalization. Journal of Graphics Tools, 16(3), pp.151-159.
         *     - http://people.compute.dtu.dk/jerf/code/hairy/
         */
        FRISVAD = 4,
    };

    /**
     * This enum specifies the auxiliary attributes of each vertex that can be provided as input.
     * These attributes do not affect the computation of the tangent space, but they will be
     * properly mapped when a remeshing is carried out.
     */
    enum class AuxAttribute : uint8_t {
        UV1 = 0x0,
        COLORS = 0x1,
        JOINTS = 0x2,
        WEIGHTS = 0x3,
    };

    using InData = std::variant<filament::math::float2 const*, filament::math::float3 const*,
            filament::math::float4 const*, filament::math::ushort3 const*,
            filament::math::ushort4 const*>;

    /**
     * Use this class to provide input to the TangentSpaceMesh computation. **Important**:
     * Computation of the tangent space is intended to be synchronous (working on the same thread).
     * Client is expected to keep the input immutable and in a good state for the duration of both
     * computation *and* query. That is, when querying the result of the tangent spaces, part of the
     * result might depend on the input data.
     */
    class Builder {
    public:
        Builder() noexcept;
        ~Builder() noexcept;

        /**
         * Move constructor
         */
        Builder(Builder&& that) noexcept;

        /**
         * Move constructor
         */
        Builder& operator=(Builder&& that) noexcept;

        Builder(Builder const&) = delete;
        Builder& operator=(Builder const&) = delete;

        /**
         * Client must provide this parameter
         *
         * @param vertexCount The input number of vertcies
         */
        Builder& vertexCount(size_t vertexCount) noexcept;

        /**
         * @param normals The input normals
         * @param stride The stride for iterating through `normals`
         * @return Builder
         */
        Builder& normals(filament::math::float3 const* normals, size_t stride = 0) noexcept;

        /**
         * @param tangents The input tangents. The `w` component is for use with
         *     Algorithm::SIGN_OF_W.
         * @param stride The stride for iterating through `tangents`
         * @return Builder
         */
        Builder& tangents(filament::math::float4 const* tangents, size_t stride = 0) noexcept;

        /**
         * @param uvs The input uvs
         * @param stride The stride for iterating through `uvs`
         * @return Builder
         */
        Builder& uvs(filament::math::float2 const* uvs, size_t stride = 0) noexcept;

        /**
         * Sets "auxiliary" attributes that will be properly mapped when remeshed.
         *
         * @param attribute The attribute of the data to be stored
         * @param data The data to be store
         * @param stride The stride for iterating through `attribute`
         * @return Builder
         */
        Builder& aux(AuxAttribute attribute, InData data, size_t stride = 0) noexcept;

        /**
         * @param positions The input positions
         * @param stride The stride for iterating through `positions`
         * @return Builder
         */
        Builder& positions(filament::math::float3 const* positions, size_t stride = 0) noexcept;

        /**
         * @param triangleCount The input number of triangles
         * @return Builder
         */
        Builder& triangleCount(size_t triangleCount) noexcept;

        /**
         * @param triangles The triangles in 32-bit indices
         * @return Builder
         */
        Builder& triangles(filament::math::uint3 const* triangles) noexcept;

        /**
         * @param triangles The triangles in 16-bit indices
         * @return Builder
         */
        Builder& triangles(filament::math::ushort3 const* triangles) noexcept;

        /**
         * The Client can provide an algorithm hint to produce the tangents.
         *
         * @param algorithm The algorithm hint.
         * @return Builder
         */
        Builder& algorithm(Algorithm algorithm) noexcept;

        /**
         * Computes the tangent space mesh. The resulting mesh object is owned by the callee. The
         * callee must call TangentSpaceMesh::destroy on the object once they are finished with it.
         *
         * The state of the Builder will be reset after each call to build(). The client needs to
         * populate the builder with parameters again if they choose to re-use it.
         *
         * @return A TangentSpaceMesh
         */
        TangentSpaceMesh* build();

    private:
        TangentSpaceMesh* mMesh = nullptr;
    };

    /**
     * Destroy the mesh object
     * @param mesh A pointer to a TangentSpaceMesh ready to be destroyed
     */
     static void destroy(TangentSpaceMesh* mesh) noexcept;

    /**
     * Move constructor
     */
    TangentSpaceMesh(TangentSpaceMesh&& that) noexcept;

    /**
     * Move constructor
     */
    TangentSpaceMesh& operator=(TangentSpaceMesh&& that) noexcept;

    TangentSpaceMesh(TangentSpaceMesh const&) = delete;
    TangentSpaceMesh& operator=(TangentSpaceMesh const&) = delete;

    /**
     * Number of output vertices
     *
     * The number of output vertices can be the same as the input if the selected algorithm did not
     * "remesh" the input.
     *
     * @return The number of vertices
     */
    size_t getVertexCount() const noexcept;

    /**
     * Get output vertex positions.
     * Assumes the `out` param is at least of getVertexCount() length (while accounting for
     * `stride`). The output vertices can be the same as the input if the selected algorithm did
     * not "remesh" the input. The remeshed vertices are not guarranteed to have correlation in
     * order with the input mesh.
     *
     * @param  out    Client-allocated array that will be used for copying out positions.
     * @param  stride Stride for iterating through `out`
     */
    void getPositions(filament::math::float3* out, size_t stride = 0) const;

    /**
     * Get output UVs.
     * Assumes the `out` param is at least of getVertexCount() length (while accounting for
     * `stride`). The output uvs can be the same as the input if the selected algorithm did
     * not "remesh" the input. The remeshed UVs are not guarranteed to have correlation in order
     * with the input mesh.
     *
     * @param  out    Client-allocated array that will be used for copying out UVs.
     * @param  stride Stride for iterating through `out`
     */
    void getUVs(filament::math::float2* out, size_t stride = 0) const;

    /**
     * Get output tangent space.
     * Assumes the `out` param is at least of getVertexCount() length (while accounting for
     * `stride`).
     *
     * @param  out    Client-allocated array that will be used for copying out tangent space in
     *                32-bit floating points.
     * @param  stride Stride for iterating through `out`
     */
    void getQuats(filament::math::quatf* out, size_t stride = 0) const noexcept;

    /**
     * Get output tangent space.
     * Assumes the `out` param is at least of getVertexCount() length (while accounting for
     * `stride`).
     *
     * @param  out    Client-allocated array that will be used for copying out tangent space in
     *                16-bit signed integers.
     * @param  stride Stride for iterating through `out`
     */
    void getQuats(filament::math::short4* out, size_t stride = 0) const noexcept;

    /**
     * Get output tangent space.
     * Assumes the `out` param is at least of getVertexCount() length (while accounting for
     * `stride`).
     *
     * @param  out    Client-allocated array that will be used for copying out tangent space in
     *                16-bit floating points.
     * @param  stride Stride for iterating through `out`
     */
    void getQuats(filament::math::quath* out, size_t stride = 0) const noexcept;

    /**
     * Get output auxiliary attributes.
     * Assumes the `out` param is at least of getVertexCount() length (while accounting for
     * `stride`).
     *
     * @param  out    Client-allocated array that will be used for copying out attribute as T
     * @param  stride Stride for iterating through `out`
     */
    template<typename T>
    using is_supported_aux_t =
            typename std::enable_if<std::is_same<filament::math::float2, T>::value ||
                                    std::is_same<filament::math::float3, T>::value ||
                                    std::is_same<filament::math::float4, T>::value ||
                                    std::is_same<filament::math::ushort3, T>::value ||
                                    std::is_same<filament::math::ushort4, T>::value>::type;
    template<typename T, typename = is_supported_aux_t<T>>
    void getAux(AuxAttribute attribute, T* out, size_t stride = 0) const noexcept;

    /**
     * Get number of output triangles.
     * The number of output triangles is the same as the number of input triangles. However, when a
     * "remesh" is carried out the output triangles are not guarranteed to have any correlation with
     * the input.
     *
     * @return The number of vertices
     */
    size_t getTriangleCount() const noexcept;

    /**
     * Get output triangles.
     * This method assumes that the `out` param provided by the client is at least of
     * getTriangleCount() length. If the client calls getTriangles() and triangles were not
     * provided as input, we will throw and exception.
     *
     * @param out Client's array for the output triangles in unsigned 32-bit indices.
     */
    void getTriangles(filament::math::uint3* out) const;

    /**
     * Get output triangles.
     * This method assumes that the `out` param provided by the client is at least of
     * getTriangleCount() length. If the client calls getTriangles() and triangles were not
     * provided as input, we will throw and exception.
     *
     * @param out Client's array for the output triangles in unsigned 16-bit indices.
     */
    void getTriangles(filament::math::ushort3* out) const;

    /**
     * @return Whether the TBN algorithm remeshed the input.
     */
    bool remeshed() const noexcept;

private:
    ~TangentSpaceMesh() noexcept;
    TangentSpaceMesh() noexcept;
    TangentSpaceMeshInput* mInput;
    TangentSpaceMeshOutput* mOutput;

    friend class Builder;
};

} // namespace geometry
} // namespace filament

#endif //TNT_GEOMETRY_TANGENTSPACEMESH_H
