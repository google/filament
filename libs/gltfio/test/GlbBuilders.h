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

// Builds a single-triangle GLB with `morphTargetCount` morph targets and two weight animations
// (one LINEAR and one CUBICSPLINE), reproducing issue #10409 when `morphTargetCount` exceeds
// MAX_MORPH_TARGETS.
std::vector<uint8_t> makeAnimatedMorphTargetGlb(int morphTargetCount);

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

// Builds a GLB whose first primitive has only a TEXCOORD_0 attribute (no POSITION) and uses the
// default lit material, reproducing issue #10638. When `includeValidSiblingPrimitive` is true, the
// mesh also contains a second primitive with a valid POSITION attribute.
std::vector<uint8_t> makePositionlessPrimitiveGlb(bool includeValidSiblingPrimitive = false);

// Builds a single-triangle GLB using KHR_draco_mesh_compression whose compressed bufferView holds
// invalid Draco bytes, so that Draco decoding fails and the primitive's accessors are left with a
// null buffer_view.
std::vector<uint8_t> makeInvalidDracoGlb();

// Builds a single-primitive GLB whose POSITION accessor declares `vertexCount` vertices and omits
// TEXCOORD_0 and COLOR_0 so that AssetLoader's dummy buffer allocation path is exercised.
std::vector<uint8_t> makeDummyBufferOverflowGlb(uint64_t vertexCount);

// Builds a single-triangle GLB whose scene root is the head of a chain of `depth` nested nodes
// (node 0 -> node 1 -> ... -> node depth - 1), with the mesh attached to the leaf node and a
// translation of (1, 0, 0) on every node, reproducing issue #10624 when `depth` is large.
std::vector<uint8_t> makeDeeplyNestedNodesGlb(size_t depth);

// Builds a single-triangle GLB whose NORMAL accessor is sparse and has its sparse indices
// bufferView compressed with EXT_meshopt_compression (mode = INDICES). The uncompressed fallback
// bytes in the buffer contain an in-bounds index (0), while the meshopt stream decodes to an
// out-of-bounds index (>= vertexCount), reproducing issue #10201.
std::vector<uint8_t> makeMeshoptSparseAccessorOobGlb();

// The translation that makeTranslationAnimationGlb() stores for its second keyframe.
constexpr filament::math::float3 kTranslationAnimationEnd = { 1.0f, 2.0f, 3.0f };

// The float that makeTranslationAnimationGlb() writes into the gap between strided timestamps. It
// is deliberately larger than any real timestamp so that a reader which ignores the stride produces
// a visibly wrong duration.
constexpr float kTranslationAnimationStridePadding = 7.0f;

// Options for makeTranslationAnimationGlb(). The defaults produce a valid asset.
struct TranslationAnimationOptions {
    // The byteLength of the output (translation) bufferView and the byteOffset of the output
    // accessor within it. Making these reach past the end of the buffer produces an asset that
    // cgltf_validate() rejects, so that loadResources() fails.
    uint32_t outputViewByteLength = 24;
    uint32_t outputAccessorByteOffset = 0;

    // When non-zero, the timeline bufferView declares this byteStride (which must be a multiple of
    // 4 that is at least 8) and the bytes between consecutive timestamps are filled with
    // kTranslationAnimationStridePadding.
    uint32_t timelineByteStride = 0;

    // The value of the second (last) timestamp. The first timestamp is always 0. Non-finite values
    // are stored verbatim, since the timestamps live in the binary chunk rather than in JSON.
    float lastTimestamp = 1.0f;

    // When set, the timeline is stored in a second buffer that has no URI and is not the GLB BIN
    // chunk. cgltf_load_buffers() leaves such a buffer with null data but still reports success,
    // so loadResources() succeeds while the timeline bytes are unreadable. This mirrors the
    // EXT_meshopt_compression fallback buffer case and the non-filesystem builds, where a missing
    // external resource is tolerated.
    bool timelineInUnloadedBuffer = false;
};

// Builds a GLB with a single node named "animated" and no meshes. The node is animated by one
// LINEAR translation channel with keyframes at t = 0 (origin) and t = `lastTimestamp`
// (kTranslationAnimationEnd). See TranslationAnimationOptions for the ways in which the asset can
// be made invalid or unreadable.
std::vector<uint8_t> makeTranslationAnimationGlb(TranslationAnimationOptions const& options = {});

} // namespace test

#endif // TNT_GLTFIO_TEST_GLBBUILDERS_H
