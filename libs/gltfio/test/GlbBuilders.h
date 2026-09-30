/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_GLTFIO_TEST_GLBBUILDERS_H
#define TNT_GLTFIO_TEST_GLBBUILDERS_H

#include <math/vec3.h>

#include <cstddef>
#include <cstdint>
#include <vector>

// Builders for small, synthetic GLB assets that are used by the gltfio tests.
namespace test {

// Builds a GLB with a single EXT_meshopt_compression bufferView whose decoded layout is described
// by `meshoptCount`, `stride`, `mode` and `filter`. The compressed payload is a valid encoding of
// zeroed vertices when `stride` is a non-zero multiple of 4, and zero bytes otherwise.
std::vector<uint8_t> makeMeshoptGlb(size_t meshoptCount, size_t stride,
        char const* mode = "ATTRIBUTES", char const* filter = "NONE");

// Builds a single-primitive GLB with an unlit material whose 8-bit index accessor declares
// `indexCount` indices, while its bufferView only holds a single byte.
std::vector<uint8_t> makeMalformedEightBitIndexGlb(uint32_t indexCount);

// Builds a minimal single-primitive GLB whose index accessor can be given an arbitrary type,
// component type, count and bufferView byteStride. This is what lets a test express the case where
// the IndexBuffer capacity (count * componentSize) and the size computed from the accessor's
// stride and type disagree.
std::vector<uint8_t> makeIndexAccessorGlb(char const* indexType, int indexComponentType,
        uint32_t indexCount, uint32_t indexByteStride);

// Builds a single-triangle GLB with `morphTargetCount` morph targets, each of which only carries a
// (zero) TANGENT delta.
std::vector<uint8_t> makeMorphTargetGlb(int morphTargetCount);

// The material that the morph target builders below assign to their primitive.
enum class MorphMaterial { NONE, UNLIT, LIT };

// The morph position delta stored for vertex `v` by makeSparseTangentMorphGlb().
filament::math::float3 sparseMorphPositionDelta(int v);

// Builds a single-triangle GLB with `morphTargetCount` morph targets. Every target carries the
// same POSITION delta (see sparseMorphPositionDelta), and only the target at `tangentTargetIndex`
// additionally carries a TANGENT delta. This reproduces the layouts of issues #10180 (no material)
// and #10500 (unlit material), where the extended loader used to size slotIndices by a filtered
// target count but index it by the raw target index. When `withColors` is set, the base primitive
// also has a COLOR_0 attribute. When `withUvs` is set, it also has a TEXCOORD_0 attribute, which
// makes lit primitives use mikktspace instead of the provided tangents.
std::vector<uint8_t> makeSparseTangentMorphGlb(int morphTargetCount, int tangentTargetIndex,
        MorphMaterial material, bool withColors = false, bool withUvs = false);

// Builds a non-indexed GLB with two triangles forming a unit quad, and a single morph target. The
// two vertices on the shared edge are duplicated, and the copies differ only in COLOR_0 (red for
// the first triangle, blue for the second). This is how hard color edges are usually authored. The
// morph delta depends only on the position, so the copies also receive identical deltas.
//
// With a lit material and TEXCOORD_0, the base tangent-space job keeps the copies apart (6
// vertices) because their colors differ, while the morph target job welds them (4 vertices)
// because it does not carry the base colors. The morph target data therefore does not line up
// with the base vertex buffer.
std::vector<uint8_t> makeColorSeamMorphGlb(MorphMaterial material);

} // namespace test

#endif // TNT_GLTFIO_TEST_GLBBUILDERS_H
