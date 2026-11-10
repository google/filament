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

#include "GlbBuilders.h"

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <gtest/gtest.h>
#include <meshoptimizer.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

using filament::math::float2;
using filament::math::float3;
using filament::math::float4;

namespace test {

namespace {

// The base vertex color stored by makeSparseTangentMorphGlb() when `withColors` is set.
constexpr float4 kSparseMorphBaseColor = { 1.0f, 0.0f, 1.0f, 1.0f };

void appendU32LE(std::vector<uint8_t>& dst, uint32_t value) {
    dst.push_back(uint8_t(value & 0xffu));
    dst.push_back(uint8_t((value >> 8u) & 0xffu));
    dst.push_back(uint8_t((value >> 16u) & 0xffu));
    dst.push_back(uint8_t((value >> 24u) & 0xffu));
}

// Pads `json` and `bin` to 4-byte boundaries and wraps them into a GLB container.
std::vector<uint8_t> assembleGlb(std::string json, std::vector<uint8_t> bin) {
    while ((json.size() % 4u) != 0u) {
        json.push_back(' ');
    }
    while ((bin.size() % 4u) != 0u) {
        bin.push_back(0);
    }

    const uint32_t jsonSize = uint32_t(json.size());
    const uint32_t binSize = uint32_t(bin.size());
    const uint32_t totalSize = 12u + 8u + jsonSize + 8u + binSize;

    std::vector<uint8_t> glb;
    glb.reserve(totalSize);

    appendU32LE(glb, 0x46546c67u);
    appendU32LE(glb, 2u);
    appendU32LE(glb, totalSize);
    appendU32LE(glb, jsonSize);
    appendU32LE(glb, 0x4e4f534au);
    glb.insert(glb.end(), json.begin(), json.end());
    appendU32LE(glb, binSize);
    appendU32LE(glb, 0x004e4942u);
    glb.insert(glb.end(), bin.begin(), bin.end());

    return glb;
}

std::vector<uint8_t> makeMeshoptPayload(size_t vertexCount, size_t stride) {
    std::vector<uint8_t> vertices(vertexCount * stride, 0);
    std::vector<uint8_t> encoded(meshopt_encodeVertexBufferBound(vertexCount, stride));
    const size_t encodedSize = meshopt_encodeVertexBuffer(encoded.data(), encoded.size(),
            vertices.data(), vertexCount, stride);
    EXPECT_GT(encodedSize, 0u);
    encoded.resize(encodedSize);
    return encoded;
}

std::string makeMeshoptGlbJson(size_t meshoptCount, size_t meshoptEncodedSize,
        size_t bufferByteLength, size_t stride, const char* mode, const char* filter) {
    const size_t decodedSize = meshoptCount * stride;
    return std::string(R"({
  "asset": { "version": "2.0" },
  "extensionsUsed": ["EXT_meshopt_compression"],
  "buffers": [
    { "byteLength": )") + std::to_string(bufferByteLength) + R"( }
  ],
  "bufferViews": [
    {
      "buffer": 0,
      "byteOffset": 0,
      "byteLength": )" + std::to_string(decodedSize) + R"(,
      "extensions": {
        "EXT_meshopt_compression": {
          "buffer": 0,
          "byteOffset": 0,
          "byteLength": )" + std::to_string(meshoptEncodedSize) + R"(,
          "byteStride": )" + std::to_string(stride) + R"(,
          "count": )" + std::to_string(meshoptCount) + R"(,
          "mode": ")" + mode + R"(",
          "filter": ")" + filter + R"("
        }
      }
    }
  ],
  "nodes": [],
  "scenes": [
    { "nodes": [] }
  ],
  "scene": 0
})";
}

// Fills in the JSON fragments that declare `material` and reference it from the first primitive.
void morphMaterialJson(MorphMaterial material, std::string* extensionsUsed,
        std::string* materials, std::string* materialRef) {
    switch (material) {
        case MorphMaterial::NONE:
            break;
        case MorphMaterial::UNLIT:
            *extensionsUsed = R"("extensionsUsed":["KHR_materials_unlit"],)";
            *materials = R"("materials":[{"extensions":{"KHR_materials_unlit":{}}}],)";
            *materialRef = R"(,"material":0)";
            break;
        case MorphMaterial::LIT:
            *materials = R"("materials":[{}],)";
            *materialRef = R"(,"material":0)";
            break;
    }
}

} // namespace

std::vector<uint8_t> makeMeshoptGlb(size_t meshoptCount, size_t stride, char const* mode,
        char const* filter) {
    static constexpr size_t kEncodedVertexCount = 256;
    std::vector<uint8_t> meshopt;
    if (stride > 0 && stride % 4 == 0) {
        meshopt = makeMeshoptPayload(kEncodedVertexCount, stride);
    } else {
        meshopt.assign(32, 0);
    }

    std::string json = makeMeshoptGlbJson(meshoptCount, meshopt.size(), meshopt.size(), stride,
            mode, filter);
    return assembleGlb(std::move(json), std::move(meshopt));
}

std::vector<uint8_t> makeMalformedEightBitIndexGlb(uint32_t indexCount) {
    std::string json = std::string(R"({
  "asset": { "version": "2.0" },
  "extensionsUsed": ["KHR_materials_unlit"],
  "buffers": [
    { "byteLength": 13 }
  ],
  "bufferViews": [
    { "buffer": 0, "byteOffset": 0, "byteLength": 12 },
    { "buffer": 0, "byteOffset": 12, "byteLength": 1 }
  ],
  "accessors": [
    {
      "bufferView": 0,
      "componentType": 5126,
      "count": 1,
      "type": "VEC3",
      "min": [0, 0, 0],
      "max": [0, 0, 0]
    },
    {
      "bufferView": 1,
      "componentType": 5121,
      "count": )") +
            std::to_string(indexCount) +
            R"(,
      "type": "SCALAR"
    }
  ],
  "materials": [
    {
      "extensions": {
        "KHR_materials_unlit": {}
      }
    }
  ],
  "meshes": [
    {
      "primitives": [
        {
          "attributes": { "POSITION": 0 },
          "indices": 1,
          "material": 0
        }
      ]
    }
  ],
  "nodes": [
    { "mesh": 0 }
  ],
  "scenes": [
    { "nodes": [0] }
  ],
  "scene": 0
})";

    return assembleGlb(std::move(json), std::vector<uint8_t>(13, 0));
}

std::vector<uint8_t> makeIndexAccessorGlb(char const* indexType, int indexComponentType,
        uint32_t indexCount, uint32_t indexByteStride) {
    constexpr uint32_t kBinSize = 256u;
    // Three vertices, so that the all-zero index data in the bin chunk stays in bounds and the
    // only thing under test is the index accessor's own layout.
    constexpr uint32_t kPositionCount = 3u;
    constexpr uint32_t kPositionBytes = kPositionCount * 12u;

    std::string strideJson;
    if (indexByteStride > 0) {
        strideJson = ", \"byteStride\": " + std::to_string(indexByteStride);
    }

    std::string json = std::string(R"({
  "asset": { "version": "2.0" },
  "buffers": [
    { "byteLength": )") + std::to_string(kBinSize) + R"( }
  ],
  "bufferViews": [
    { "buffer": 0, "byteOffset": 0, "byteLength": )" + std::to_string(kPositionBytes) + R"( },
    { "buffer": 0, "byteOffset": )" + std::to_string(kPositionBytes) + R"(, "byteLength": )" +
            std::to_string(kBinSize - kPositionBytes) + strideJson + R"( }
  ],
  "accessors": [
    {
      "bufferView": 0,
      "componentType": 5126,
      "count": )" + std::to_string(kPositionCount) + R"(,
      "type": "VEC3",
      "min": [0, 0, 0],
      "max": [0, 0, 0]
    },
    {
      "bufferView": 1,
      "componentType": )" + std::to_string(indexComponentType) + R"(,
      "count": )" + std::to_string(indexCount) + R"(,
      "type": ")" + indexType + R"("
    }
  ],
  "meshes": [
    {
      "primitives": [
        { "attributes": { "POSITION": 0 }, "indices": 1 }
      ]
    }
  ],
  "nodes": [
    { "mesh": 0 }
  ],
  "scenes": [
    { "nodes": [0] }
  ],
  "scene": 0
})";

    return assembleGlb(std::move(json), std::vector<uint8_t>(kBinSize, 0));
}

std::vector<uint8_t> makeMorphTargetGlb(int morphTargetCount) {
    std::string targets;
    std::string weights = "[";
    for (int i = 0; i < morphTargetCount; ++i) {
        targets += (i == 0) ? "{\"TANGENT\":2}" : ",{\"TANGENT\":2}";
        weights += (i == 0) ? "0.0" : ",0.0";
    }
    weights += "]";

    std::string json =
            "{\"asset\":{\"version\":\"2.0\"},\"scene\":0,\"scenes\":[{\"nodes\":[0]}],"
            "\"nodes\":[{\"mesh\":0}],"
            "\"meshes\":[{\"weights\":" + weights + ",\"primitives\":[{\"attributes\":{\"POSITION\":0,\"TANGENT\":1},\"mode\":4,"
            "\"targets\":[" + targets + "]}]}],"
            "\"accessors\":["
            "{\"bufferView\":0,\"componentType\":5126,\"count\":3,\"type\":\"VEC3\","
            "\"min\":[0,0,0],\"max\":[1,1,1]},"
            "{\"bufferView\":1,\"componentType\":5126,\"count\":3,\"type\":\"VEC4\"},"
            "{\"bufferView\":2,\"componentType\":5126,\"count\":3,\"type\":\"VEC3\"}],"
            "\"bufferViews\":[{\"buffer\":0,\"byteOffset\":0,\"byteLength\":36},"
            "{\"buffer\":0,\"byteOffset\":36,\"byteLength\":48},"
            "{\"buffer\":0,\"byteOffset\":84,\"byteLength\":36}],"
            "\"buffers\":[{\"byteLength\":120}]}";

    std::vector<uint8_t> bin(120, 0);
    float* fbin = reinterpret_cast<float*>(bin.data());
    // 3 vertices positions
    fbin[0] = 0.0f; fbin[1] = 0.0f; fbin[2] = 0.0f;
    fbin[3] = 1.0f; fbin[4] = 0.0f; fbin[5] = 0.0f;
    fbin[6] = 0.0f; fbin[7] = 1.0f; fbin[8] = 0.0f;
    // 3 vertices base tangents (vec4) at byteOffset 36 = float offset 9
    fbin[9]  = 1.0f; fbin[10] = 0.0f; fbin[11] = 0.0f; fbin[12] = 1.0f;
    fbin[13] = 1.0f; fbin[14] = 0.0f; fbin[15] = 0.0f; fbin[16] = 1.0f;
    fbin[17] = 1.0f; fbin[18] = 0.0f; fbin[19] = 0.0f; fbin[20] = 1.0f;
    // 3 vertices target tangent deltas (vec3) at byteOffset 84 = float offset 21
    fbin[21] = 0.0f; fbin[22] = 0.0f; fbin[23] = 0.0f;
    fbin[24] = 0.0f; fbin[25] = 0.0f; fbin[26] = 0.0f;
    fbin[27] = 0.0f; fbin[28] = 0.0f; fbin[29] = 0.0f;

    return assembleGlb(std::move(json), std::move(bin));
}

std::vector<uint8_t> makeAnimatedMorphTargetGlb(int morphTargetCount) {
    std::string targets;
    std::string weights = "[";
    for (int i = 0; i < morphTargetCount; ++i) {
        targets += (i == 0) ? R"({"POSITION":2})" : R"(,{"POSITION":2})";
        weights += (i == 0) ? "0.0" : ",0.0";
    }
    weights += "]";

    size_t const linearCount = 2u * size_t(morphTargetCount);
    size_t const cubicCount = 2u * 3u * size_t(morphTargetCount);
    size_t const linearBytes = linearCount * sizeof(float);
    size_t const cubicBytes = cubicCount * sizeof(float);
    size_t const linearOffset = 128u;
    size_t const cubicOffset = linearOffset + linearBytes;
    size_t const totalBytes = cubicOffset + cubicBytes;

    std::string json =
            R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[0]}],)"
            R"("nodes":[{"mesh":0}],)"
            R"("meshes":[{"weights":)" + weights +
            R"(,"primitives":[{"attributes":{"POSITION":0,"TANGENT":1},"mode":4,)"
            R"("targets":[)" + targets + R"(]}]}],)"
            R"("animations":[)"
            R"({"samplers":[{"input":3,"interpolation":"LINEAR","output":4}],)"
            R"("channels":[{"sampler":0,"target":{"node":0,"path":"weights"}}]},)"
            R"({"samplers":[{"input":3,"interpolation":"CUBICSPLINE","output":5}],)"
            R"("channels":[{"sampler":0,"target":{"node":0,"path":"weights"}}]}],)"
            R"("accessors":[)"
            R"({"bufferView":0,"componentType":5126,"count":3,"type":"VEC3",)"
            R"("min":[0,0,0],"max":[1,1,0]},)"
            R"({"bufferView":1,"componentType":5126,"count":3,"type":"VEC4"},)"
            R"({"bufferView":2,"componentType":5126,"count":3,"type":"VEC3",)"
            R"("min":[0,0,0],"max":[0,0,0]},)"
            R"({"bufferView":3,"componentType":5126,"count":2,"type":"SCALAR",)"
            R"("min":[0],"max":[1]},)"
            R"({"bufferView":4,"componentType":5126,"count":)" + std::to_string(linearCount) +
            R"(,"type":"SCALAR"},)"
            R"({"bufferView":5,"componentType":5126,"count":)" + std::to_string(cubicCount) +
            R"(,"type":"SCALAR"}],)"
            R"("bufferViews":[)"
            R"({"buffer":0,"byteOffset":0,"byteLength":36},)"
            R"({"buffer":0,"byteOffset":36,"byteLength":48},)"
            R"({"buffer":0,"byteOffset":84,"byteLength":36},)"
            R"({"buffer":0,"byteOffset":120,"byteLength":8},)"
            R"({"buffer":0,"byteOffset":)" + std::to_string(linearOffset) +
            R"(,"byteLength":)" + std::to_string(linearBytes) + R"(},)"
            R"({"buffer":0,"byteOffset":)" + std::to_string(cubicOffset) +
            R"(,"byteLength":)" + std::to_string(cubicBytes) + R"(}],)"
            R"("buffers":[{"byteLength":)" + std::to_string(totalBytes) + R"(}]})";

    std::vector<uint8_t> bin(totalBytes, 0);
    float* fbin = reinterpret_cast<float*>(bin.data());
    // 3 vertex positions at float offset 0: (0,0,0), (1,0,0), (0,1,0).
    fbin[3] = 1.0f;
    fbin[7] = 1.0f;
    // 3 vertex base tangents (vec4) at byteOffset 36 = float offset 9.
    for (int v = 0; v < 3; ++v) {
        fbin[9 + v * 4 + 0] = 1.0f;
        fbin[9 + v * 4 + 3] = 1.0f;
    }
    // 2 keyframe timestamps at byteOffset 120 = float offset 30.
    fbin[30] = 0.0f;
    fbin[31] = 1.0f;
    // Keyframe 1 linear weights.
    float* linearWeights = fbin + (linearOffset / sizeof(float));
    for (int i = 0; i < morphTargetCount; ++i) {
        linearWeights[morphTargetCount + i] = 1.0f;
    }
    // Keyframe 1 cubic spline vertex weights.
    float* cubicWeights = fbin + (cubicOffset / sizeof(float));
    for (int i = 0; i < morphTargetCount; ++i) {
        cubicWeights[4 * morphTargetCount + i] = 1.0f;
    }

    return assembleGlb(std::move(json), std::move(bin));
}

float3 sparseMorphPositionDelta(int v) {
    return { 0.25f * float(v + 1), 0.5f, -0.125f };
}

std::vector<uint8_t> makeSparseTangentMorphGlb(int morphTargetCount, int tangentTargetIndex,
        MorphMaterial material, bool withColors, bool withUvs) {
    std::string targets;
    std::string weights = "[";
    for (int i = 0; i < morphTargetCount; ++i) {
        if (i > 0) {
            targets += ",";
            weights += ",";
        }
        targets += (i == tangentTargetIndex) ? R"({"POSITION":3,"TANGENT":4})"
                                             : R"({"POSITION":3})";
        weights += "0.0";
    }
    weights += "]";

    std::string materialRef;
    std::string materials;
    std::string extensionsUsed;
    morphMaterialJson(material, &extensionsUsed, &materials, &materialRef);

    std::string const colorAttribute = withColors ? R"(,"COLOR_0":5)" : "";
    std::string const uvAttribute = withUvs ? R"(,"TEXCOORD_0":6)" : "";

    std::string json =
            R"({"asset":{"version":"2.0"},)" + extensionsUsed + materials +
            R"("scene":0,"scenes":[{"nodes":[0]}],"nodes":[{"mesh":0}],)"
            R"("meshes":[{"weights":)" + weights +
            R"(,"primitives":[{"attributes":{"POSITION":0,"NORMAL":1,"TANGENT":2)" +
            colorAttribute + uvAttribute + R"(},"mode":4)" + materialRef + R"(,"targets":[)" +
            targets + R"(]}]}],)"
            R"("accessors":[)"
            R"({"bufferView":0,"componentType":5126,"count":3,"type":"VEC3",)"
            R"("min":[0,0,0],"max":[1,1,0]},)"
            R"({"bufferView":1,"componentType":5126,"count":3,"type":"VEC3"},)"
            R"({"bufferView":2,"componentType":5126,"count":3,"type":"VEC4"},)"
            R"({"bufferView":3,"componentType":5126,"count":3,"type":"VEC3",)"
            R"("min":[0.25,0.5,-0.125],"max":[0.75,0.5,-0.125]},)"
            R"({"bufferView":4,"componentType":5126,"count":3,"type":"VEC3"},)"
            R"({"bufferView":5,"componentType":5126,"count":3,"type":"VEC4"},)"
            R"({"bufferView":6,"componentType":5126,"count":3,"type":"VEC2"}],)"
            R"("bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":36},)"
            R"({"buffer":0,"byteOffset":36,"byteLength":36},)"
            R"({"buffer":0,"byteOffset":72,"byteLength":48},)"
            R"({"buffer":0,"byteOffset":120,"byteLength":36},)"
            R"({"buffer":0,"byteOffset":156,"byteLength":36},)"
            R"({"buffer":0,"byteOffset":192,"byteLength":48},)"
            R"({"buffer":0,"byteOffset":240,"byteLength":24}],)"
            R"("buffers":[{"byteLength":264}]})";

    std::vector<uint8_t> bin(264, 0);
    float* fbin = reinterpret_cast<float*>(bin.data());
    // Base positions (3 x vec3) at float offset 0.
    fbin[3] = 1.0f;
    fbin[7] = 1.0f;
    // Base normals (3 x vec3) at float offset 9, all pointing along +Z.
    for (int v = 0; v < 3; ++v) {
        fbin[9 + v * 3 + 2] = 1.0f;
    }
    // Base tangents (3 x vec4) at float offset 18, all (1, 0, 0, 1).
    for (int v = 0; v < 3; ++v) {
        fbin[18 + v * 4 + 0] = 1.0f;
        fbin[18 + v * 4 + 3] = 1.0f;
    }
    // Morph position deltas (3 x vec3) at float offset 30.
    for (int v = 0; v < 3; ++v) {
        float3 const delta = sparseMorphPositionDelta(v);
        fbin[30 + v * 3 + 0] = delta.x;
        fbin[30 + v * 3 + 1] = delta.y;
        fbin[30 + v * 3 + 2] = delta.z;
    }
    // Morph tangent deltas (3 x vec3) at float offset 39 are left at zero.
    // Base colors (3 x vec4) at float offset 48.
    for (int v = 0; v < 3; ++v) {
        for (int c = 0; c < 4; ++c) {
            fbin[48 + v * 4 + c] = kSparseMorphBaseColor[c];
        }
    }
    // Base UVs (3 x vec2) at float offset 60, equal to the xy of the base positions.
    fbin[62] = 1.0f;
    fbin[65] = 1.0f;

    return assembleGlb(std::move(json), std::move(bin));
}

std::vector<uint8_t> makeColorSeamMorphGlb(MorphMaterial material) {
    std::string materialRef;
    std::string materials;
    std::string extensionsUsed;
    morphMaterialJson(material, &extensionsUsed, &materials, &materialRef);

    std::string json =
            R"({"asset":{"version":"2.0"},)" + extensionsUsed + materials +
            R"("scene":0,"scenes":[{"nodes":[0]}],"nodes":[{"mesh":0}],)"
            R"("meshes":[{"weights":[0.0],)"
            R"("primitives":[{"attributes":{"POSITION":0,"NORMAL":1,"TEXCOORD_0":2,"COLOR_0":3},)"
            R"("mode":4)" + materialRef + R"(,"targets":[{"POSITION":4}]}]}],)"
            R"("accessors":[)"
            R"({"bufferView":0,"componentType":5126,"count":6,"type":"VEC3",)"
            R"("min":[0,0,0],"max":[1,1,0]},)"
            R"({"bufferView":1,"componentType":5126,"count":6,"type":"VEC3"},)"
            R"({"bufferView":2,"componentType":5126,"count":6,"type":"VEC2"},)"
            R"({"bufferView":3,"componentType":5126,"count":6,"type":"VEC4"},)"
            R"({"bufferView":4,"componentType":5126,"count":6,"type":"VEC3",)"
            R"("min":[0,0,0],"max":[0,0,1]}],)"
            R"("bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":72},)"
            R"({"buffer":0,"byteOffset":72,"byteLength":72},)"
            R"({"buffer":0,"byteOffset":144,"byteLength":48},)"
            R"({"buffer":0,"byteOffset":192,"byteLength":96},)"
            R"({"buffer":0,"byteOffset":288,"byteLength":72}],)"
            R"("buffers":[{"byteLength":360}]})";

    static constexpr float2 kCorners[6] = {
            { 0, 0 }, { 1, 0 }, { 0, 1 },   // first triangle
            { 1, 0 }, { 1, 1 }, { 0, 1 },   // second triangle, sharing the edge (1,0)-(0,1)
    };

    std::vector<uint8_t> bin(360, 0);
    float* fbin = reinterpret_cast<float*>(bin.data());
    for (int v = 0; v < 6; ++v) {
        float2 const p = kCorners[v];
        // Positions (6 x vec3) at float offset 0.
        fbin[v * 3 + 0] = p.x;
        fbin[v * 3 + 1] = p.y;
        // Normals (6 x vec3) at float offset 18, all pointing along +Z.
        fbin[18 + v * 3 + 2] = 1.0f;
        // UVs (6 x vec2) at float offset 36, equal to the xy of the positions.
        fbin[36 + v * 2 + 0] = p.x;
        fbin[36 + v * 2 + 1] = p.y;
        // Colors (6 x vec4) at float offset 48, red for the first triangle, blue for the second.
        fbin[48 + v * 4 + (v < 3 ? 0 : 2)] = 1.0f;
        fbin[48 + v * 4 + 3] = 1.0f;
        // Morph position deltas (6 x vec3) at float offset 72, a function of the position only.
        fbin[72 + v * 3 + 2] = 0.5f * (p.x + p.y);
    }

    return assembleGlb(std::move(json), std::move(bin));
}

std::vector<uint8_t> makePositionlessPrimitiveGlb(bool includeValidSiblingPrimitive) {
    std::string primitives = R"({"attributes":{"TEXCOORD_0":0}})";
    if (includeValidSiblingPrimitive) {
        primitives += R"(,{"attributes":{"POSITION":1,"TEXCOORD_0":0}})";
    }

    std::string json =
            R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[0]}],)"
            R"("nodes":[{"mesh":0}],)"
            R"("meshes":[{"primitives":[)" + primitives + R"(]}],)"
            R"("accessors":[)"
            R"({"bufferView":0,"componentType":5126,"count":3,"type":"VEC2"},)"
            R"({"bufferView":1,"componentType":5126,"count":3,"type":"VEC3",)"
            R"("min":[0,0,0],"max":[1,1,0]}],)"
            R"("bufferViews":[)"
            R"({"buffer":0,"byteOffset":0,"byteLength":24},)"
            R"({"buffer":0,"byteOffset":24,"byteLength":36}],)"
            R"("buffers":[{"byteLength":60}]})";

    std::vector<uint8_t> bin(60, 0);
    float* fbin = reinterpret_cast<float*>(bin.data());
    // TEXCOORD_0 (3 x vec2) at float offset 0.
    fbin[2] = 1.0f;
    fbin[5] = 1.0f;
    // POSITION (3 x vec3) at float offset 6: (0,0,0), (1,0,0), (0,1,0).
    fbin[9] = 1.0f;
    fbin[13] = 1.0f;

    return assembleGlb(std::move(json), std::move(bin));
}

std::vector<uint8_t> makeInvalidDracoGlb() {
    std::string json = R"({
  "asset": { "version": "2.0" },
  "extensionsUsed": ["KHR_draco_mesh_compression"],
  "buffers": [
    { "byteLength": 64 }
  ],
  "bufferViews": [
    { "buffer": 0, "byteOffset": 0, "byteLength": 64 }
  ],
  "accessors": [
    {
      "componentType": 5126,
      "count": 3,
      "type": "VEC3",
      "min": [0, 0, 0],
      "max": [1, 1, 1]
    },
    {
      "componentType": 5126,
      "count": 3,
      "type": "VEC3"
    },
    {
      "componentType": 5123,
      "count": 3,
      "type": "SCALAR"
    }
  ],
  "meshes": [
    {
      "primitives": [
        {
          "attributes": { "POSITION": 0, "NORMAL": 1 },
          "indices": 2,
          "mode": 4,
          "extensions": {
            "KHR_draco_mesh_compression": {
              "bufferView": 0,
              "attributes": { "POSITION": 0, "NORMAL": 1 }
            }
          }
        }
      ]
    }
  ],
  "nodes": [
    { "mesh": 0 }
  ],
  "scenes": [
    { "nodes": [0] }
  ],
  "scene": 0
})";

    return assembleGlb(std::move(json), std::vector<uint8_t>(64, 0xff));
}

std::vector<uint8_t> makeDummyBufferOverflowGlb(uint64_t vertexCount) {
    std::string json = std::string(R"({
  "asset": { "version": "2.0" },
  "extensionsUsed": ["KHR_materials_unlit"],
  "buffers": [
    { "byteLength": 6 }
  ],
  "bufferViews": [
    { "buffer": 0, "byteOffset": 0, "byteLength": 6 }
  ],
  "accessors": [
    {
      "componentType": 5126,
      "count": )") + std::to_string(vertexCount) + R"(,
      "type": "VEC3",
      "min": [0, 0, 0],
      "max": [1, 1, 1]
    },
    {
      "bufferView": 0,
      "componentType": 5123,
      "count": 3,
      "type": "SCALAR"
    }
  ],
  "images": [
    {}
  ],
  "textures": [
    { "source": 0 }
  ],
  "materials": [
    {
      "pbrMetallicRoughness": {
        "baseColorTexture": { "index": 0 }
      },
      "extensions": {
        "KHR_materials_unlit": {}
      }
    }
  ],
  "meshes": [
    {
      "primitives": [
        {
          "attributes": { "POSITION": 0 },
          "indices": 1,
          "material": 0
        }
      ]
    }
  ],
  "nodes": [
    { "mesh": 0 }
  ],
  "scenes": [
    { "nodes": [0] }
  ],
  "scene": 0
})";

    return assembleGlb(std::move(json), std::vector<uint8_t>(6, 0));
}

std::vector<uint8_t> makeDeeplyNestedNodesGlb(size_t depth) {
    EXPECT_GT(depth, 1u);

    char buffer[7];

    std::string nodes;
    nodes.reserve(depth * 44);
    nodes += R"({"translation":[1,0,0],"children":[1]})";

    for (size_t i = 1; i < depth - 1; ++i) {
        nodes += R"(,{"translation":[1,0,0],"children":[)";
        // Re-use the buffer to convert the index to a string, avoiding heap allocations
        auto [ptr, _] = std::to_chars(buffer, buffer + sizeof(buffer), i + 1);
        nodes.append(buffer, ptr - buffer);
        nodes += "]}";
    }
    nodes += R"(,{"translation":[1,0,0],"mesh":0})";

    std::string json =
            R"({"asset":{"version":"2.0"},"extensionsUsed":["KHR_materials_unlit"],)"
            R"("scene":0,"scenes":[{"nodes":[0]}],)"
            R"("nodes":[)" + nodes + R"(],)"
            R"("materials":[{"extensions":{"KHR_materials_unlit":{}}}],)"
            R"("meshes":[{"primitives":[{"attributes":{"POSITION":0},"material":0}]}],)"
            R"("accessors":[{"bufferView":0,"componentType":5126,"count":3,"type":"VEC3",)"
            R"("min":[0,0,0],"max":[1,1,0]}],)"
            R"("bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":36}],)"
            R"("buffers":[{"byteLength":36}]})";

    std::vector<uint8_t> bin(36, 0);
    float* fbin = reinterpret_cast<float*>(bin.data());
    // 3 vertex positions: (0,0,0), (1,0,0), (0,1,0).
    fbin[3] = 1.0f;
    fbin[7] = 1.0f;

    return assembleGlb(std::move(json), std::move(bin));
}
  
std::vector<uint8_t> makeMeshoptSparseAccessorOobGlb() {
    constexpr uint32_t kVertexCount = 3u;
    constexpr uint16_t kOobSparseIndex = uint16_t(kVertexCount);

    std::vector<uint8_t> encodedIndices(meshopt_encodeIndexSequenceBound(1, kOobSparseIndex + 1));
    size_t const encodedSize = meshopt_encodeIndexSequence(encodedIndices.data(),
            encodedIndices.size(), &kOobSparseIndex, 1);
    EXPECT_GT(encodedSize, 0u);
    encodedIndices.resize(encodedSize);

    // Layout of the binary buffer:
    //   [0..36):   3 x vec3 float positions (starts with 0.0f, so bytes [0..2) are uint16_t(0))
    //   [36..72):  3 x vec3 float base normals
    //   [72..84):  1 x vec3 float sparse normal value
    //   [84..84+encodedSize): meshopt-compressed sparse index stream
    constexpr size_t kPositionsOffset = 0;
    constexpr size_t kPositionsBytes = kVertexCount * sizeof(float3);
    constexpr size_t kNormalsOffset = kPositionsOffset + kPositionsBytes;
    constexpr size_t kNormalsBytes = kVertexCount * sizeof(float3);
    constexpr size_t kSparseValuesOffset = kNormalsOffset + kNormalsBytes;
    constexpr size_t kSparseValuesBytes = sizeof(float3);
    constexpr size_t kMeshoptOffset = kSparseValuesOffset + kSparseValuesBytes;
    size_t const totalBytes = kMeshoptOffset + encodedIndices.size();

    std::vector<uint8_t> bin(totalBytes, 0);
    float* fbin = reinterpret_cast<float*>(bin.data());
    // Positions: (0,0,0), (1,0,0), (0,1,0). Vertex 0 is all zero bytes, which doubles as the
    // uncompressed fallback bytes for bufferView 3 so that raw pre-decode index validation sees 0.
    fbin[3] = 1.0f;
    fbin[7] = 1.0f;
    // Base normals: (0,0,1) for all 3 vertices.
    for (uint32_t v = 0; v < kVertexCount; ++v) {
        fbin[9 + v * 3 + 2] = 1.0f;
    }
    // Sparse normal replacement value: (0,1,0).
    fbin[18 + 1] = 1.0f;
    // Copy the meshopt-compressed sparse index stream.
    std::copy(encodedIndices.begin(), encodedIndices.end(), bin.begin() + kMeshoptOffset);

    std::string json = R"({
  "asset": { "version": "2.0" },
  "extensionsUsed": ["EXT_meshopt_compression"],
  "buffers": [
    { "byteLength": )" + std::to_string(totalBytes) + R"( }
  ],
  "bufferViews": [
    { "buffer": 0, "byteOffset": )" + std::to_string(kPositionsOffset) +
            R"(, "byteLength": )" + std::to_string(kPositionsBytes) + R"( },
    { "buffer": 0, "byteOffset": )" + std::to_string(kNormalsOffset) +
            R"(, "byteLength": )" + std::to_string(kNormalsBytes) + R"( },
    { "buffer": 0, "byteOffset": )" + std::to_string(kSparseValuesOffset) +
            R"(, "byteLength": )" + std::to_string(kSparseValuesBytes) + R"( },
    {
      "buffer": 0,
      "byteOffset": 0,
      "byteLength": 2,
      "extensions": {
        "EXT_meshopt_compression": {
          "buffer": 0,
          "byteOffset": )" + std::to_string(kMeshoptOffset) + R"(,
          "byteLength": )" + std::to_string(encodedIndices.size()) + R"(,
          "byteStride": 2,
          "count": 1,
          "mode": "INDICES"
        }
      }
    }
  ],
  "accessors": [
    {
      "bufferView": 0,
      "componentType": 5126,
      "count": 3,
      "type": "VEC3",
      "min": [0, 0, 0],
      "max": [1, 1, 0]
    },
    {
      "bufferView": 1,
      "componentType": 5126,
      "count": 3,
      "type": "VEC3",
      "sparse": {
        "count": 1,
        "indices": {
          "bufferView": 3,
          "componentType": 5123
        },
        "values": {
          "bufferView": 2
        }
      }
    }
  ],
  "meshes": [
    {
      "primitives": [
        {
          "attributes": { "POSITION": 0, "NORMAL": 1 },
          "mode": 4
        }
      ]
    }
  ],
  "nodes": [
    { "mesh": 0 }
  ],
  "scenes": [
    { "nodes": [0] }
  ],
  "scene": 0
})";

    return assembleGlb(std::move(json), std::move(bin));
}

std::vector<uint8_t> makeTranslationAnimationGlb(TranslationAnimationOptions const& options) {
    constexpr uint32_t kTimestampCount = 2u;
    constexpr uint32_t kTranslationBytes = kTimestampCount * 3u * sizeof(float);

    // Lay out the timeline. Without a stride the two timestamps are packed; with a stride the
    // second timestamp starts `timelineByteStride` bytes after the first and the gap is padded.
    const uint32_t stride = options.timelineByteStride;
    EXPECT_TRUE(stride == 0u || (stride >= 8u && stride % 4u == 0u))
            << "timelineByteStride must be 0 or a multiple of 4 that is at least 8";
    const uint32_t timelineBytes = stride ? stride * (kTimestampCount - 1u) + sizeof(float)
                                          : kTimestampCount * sizeof(float);

    std::vector<uint8_t> timeline(timelineBytes, 0);
    {
        float* floats = reinterpret_cast<float*>(timeline.data());
        const size_t floatsPerKey = stride ? stride / sizeof(float) : 1u;
        for (size_t i = 0; i < timelineBytes / sizeof(float); ++i) {
            floats[i] = kTranslationAnimationStridePadding;
        }
        floats[0] = 0.0f;
        floats[floatsPerKey] = options.lastTimestamp;
    }

    // Translations: (0, 0, 0) followed by kTranslationAnimationEnd.
    std::vector<uint8_t> translations(kTranslationBytes, 0);
    {
        float* floats = reinterpret_cast<float*>(translations.data());
        floats[3] = kTranslationAnimationEnd.x;
        floats[4] = kTranslationAnimationEnd.y;
        floats[5] = kTranslationAnimationEnd.z;
    }

    // Buffer 0 is the GLB BIN chunk. When the timeline is placed in an unloaded buffer, the BIN
    // chunk only contains the translations and buffer 1 (no URI) nominally holds the timeline.
    std::string buffers;
    std::string timelineView;
    std::string translationView;
    std::vector<uint8_t> bin;
    if (options.timelineInUnloadedBuffer) {
        bin = translations;
        buffers = "    { \"byteLength\": " + std::to_string(kTranslationBytes) + " },\n"
                  "    { \"byteLength\": " + std::to_string(timelineBytes) + " }\n";
        timelineView = "{ \"buffer\": 1, \"byteOffset\": 0, \"byteLength\": " +
                std::to_string(timelineBytes);
        translationView = "{ \"buffer\": 0, \"byteOffset\": 0, \"byteLength\": " +
                std::to_string(options.outputViewByteLength) + " }";
    } else {
        bin = timeline;
        bin.insert(bin.end(), translations.begin(), translations.end());
        buffers = "    { \"byteLength\": " + std::to_string(bin.size()) + " }\n";
        timelineView = "{ \"buffer\": 0, \"byteOffset\": 0, \"byteLength\": " +
                std::to_string(timelineBytes);
        translationView = "{ \"buffer\": 0, \"byteOffset\": " + std::to_string(timelineBytes) +
                ", \"byteLength\": " + std::to_string(options.outputViewByteLength) + " }";
    }
    if (stride) {
        timelineView += ", \"byteStride\": " + std::to_string(stride);
    }
    timelineView += " }";

    std::string json = std::string(R"({
  "asset": { "version": "2.0" },
  "buffers": [
)") + buffers + R"(  ],
  "bufferViews": [
    )" + timelineView + R"(,
    )" + translationView + R"(
  ],
  "accessors": [
    {
      "bufferView": 0,
      "componentType": 5126,
      "count": )" + std::to_string(kTimestampCount) + R"(,
      "type": "SCALAR",
      "min": [0],
      "max": [1]
    },
    {
      "bufferView": 1,
      "byteOffset": )" + std::to_string(options.outputAccessorByteOffset) + R"(,
      "componentType": 5126,
      "count": )" + std::to_string(kTimestampCount) + R"(,
      "type": "VEC3"
    }
  ],
  "animations": [
    {
      "samplers": [
        { "input": 0, "interpolation": "LINEAR", "output": 1 }
      ],
      "channels": [
        { "sampler": 0, "target": { "node": 0, "path": "translation" } }
      ]
    }
  ],
  "nodes": [
    { "name": "animated" }
  ],
  "scenes": [
    { "nodes": [0] }
  ],
  "scene": 0
})";

    return assembleGlb(std::move(json), std::move(bin));
}

} // namespace test
