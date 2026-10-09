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

} // namespace test
