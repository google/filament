/*
 * Copyright 2018 The Android Open Source Project
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

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>

#include <filameshio/filamesh.h>
#include <filameshio/MeshReader.h>

#include <math/half.h>
#include <math/mat3.h>
#include <math/norm.h>
#include <math/quat.h>
#include <math/vec3.h>

#include <gtest/gtest.h>

#include <sstream>

using namespace filament;
using namespace filamesh;
using namespace filament::math;
using namespace std;

static const Box unitBox = {
    .center = float3(0, 0, 0),
    .halfExtent = float3(1, 1, 1)
};

static const half4 positions[] = {
    { 0.0_h, 1.0_h, 2.0_h, 3.0_h },
    { 4.0_h, 5.0_h, 6.0_h, 7.0_h },
    { 8.0_h, 9.0_h, 10.0_h, 11.0_h },
};

static const short4 tangents[] = {
    packSnorm16(float4(0, 1, 2, 3)),
    packSnorm16(float4(4, 5, 6, 7)),
    packSnorm16(float4(8, 9, 10, 11))
};

static const ubyte4 colors[] = {
    ubyte4(0, 1, 2, 3),
    ubyte4(4, 5, 6, 7),
    ubyte4(8, 9, 10, 11)
};

static const half2 uv0[] = {
    { 0.0_h, 1.0_h },
    { 2.0_h, 3.0_h },
    { 4.0_h, 5.0_h },
};

static const uint16_t indices[] = { 0, 1, 2 };

static const Part parts[1] { Part {
    .offset = 0,
    .indexCount = 3,
    .minIndex = 0,
    .maxIndex = 0,
    .material = 0,
    .aabb = unitBox
}};

static const int vertexCount = 3;

static const uint32_t maxint = std::numeric_limits<uint32_t>::max();

struct InterleavedVertex {
    half4  position;
    short4 tangent;
    ubyte4 color;
    half2  uv0;
};

static const InterleavedVertex interleavedVertices[] = { {
    .position = half4 { 0.0_h, 1.0_h, 2.0_h, 3.0_h },
    .tangent = packSnorm16(float4(0, 1, 2, 3)),
    .color = ubyte4(0, 1, 2, 3),
    .uv0 = { 0.0_h, 0.1_h },
}, {
    .position = half4 { 4.0_h, 5.0_h, 6.0_h, 7.0_h },
    .tangent = packSnorm16(float4(4, 5, 6, 7)),
    .color = ubyte4(4, 5, 6, 7),
    .uv0 = { 0.2_h, 0.3_h },
}, {
    .position = half4 { 8.0_h, 9.0_h, 10.0_h, 11.0_h },
    .tangent = packSnorm16(float4(8, 9, 10, 11)),
    .color = ubyte4(8, 9, 10, 11),
    .uv0 = { 0.4_h, 0.5_h },
} };

class FilameshTest : public testing::Test {
protected:
    void SetUp() override {
        engine = Engine::create(Engine::Backend::NOOP);
    }

    void TearDown() override {
        Engine::destroy(&engine);
    }

    Engine* engine = nullptr;
};

template<typename T>
void write(std::ostream& out, const T* data, size_t nbytes) {
    out.write((const char*) data, nbytes);
}

TEST_F(FilameshTest, NonInterleaved) {
    // Serialize a single-triangle mesh with 1 UV set
    const Header header {
        .version = VERSION,
        .parts = 1,
        .aabb = unitBox,
        .offsetTangents = sizeof(positions),
        .offsetColor = sizeof(positions) + sizeof(tangents),
        .offsetUV0 = sizeof(positions) + sizeof(tangents) + sizeof(colors),
        .strideUV1 = maxint,
        .vertexCount = vertexCount,
        .vertexSize = sizeof(positions) + sizeof(tangents) + sizeof(colors) + sizeof(uv0),
        .indexType = IndexType::UI16,
        .indexCount = 3,
        .indexSize = sizeof(uint16_t) * 3
    };
    const uint32_t nmats = 1;
    const string matname = "DefaultMaterial";
    const uint32_t matnamelength = matname.size();

    stringstream stream(ios_base::out);
    write(stream, MAGICID, sizeof(MAGICID));
    write(stream, &header, sizeof(header));
    write(stream, positions, sizeof(positions));
    write(stream, tangents, sizeof(tangents));
    write(stream, colors, sizeof(colors));
    write(stream, uv0, sizeof(uv0));
    write(stream, indices, sizeof(indices));
    write(stream, parts, sizeof(parts));
    write(stream, &nmats, sizeof(nmats));
    write(stream, &matnamelength, sizeof(matnamelength));
    write(stream, matname.c_str(), matnamelength + 1);

    // Deserialize the mesh as a smoke test.
    MaterialInstance* mi = engine->getDefaultMaterial()->createInstance();
    auto mesh = MeshReader::loadMeshFromBuffer(engine, stream.str().data(), nullptr, nullptr, mi);
    auto& rm = engine->getRenderableManager();
    auto inst = rm.getInstance(mesh.renderable);
    EXPECT_EQ(rm.getPrimitiveCount(inst), 1);

    // Cleanup.
    engine->destroy(mesh.renderable);
    engine->destroy(mi);
}

TEST_F(FilameshTest, Interleaved) {
    // Serialize a single-triangle mesh with 1 UV set
    const Header header {
        .version = VERSION,
        .parts = 1,
        .aabb = unitBox,
        .flags = INTERLEAVED | TEXCOORD_SNORM16,
        .offsetPosition = offsetof(InterleavedVertex, position),
        .stridePosition = sizeof(InterleavedVertex),
        .offsetTangents = offsetof(InterleavedVertex, tangent),
        .strideTangents = sizeof(InterleavedVertex),
        .offsetColor = offsetof(InterleavedVertex, color),
        .strideColor = sizeof(InterleavedVertex),
        .offsetUV0 = offsetof(InterleavedVertex, uv0),
        .strideUV0 = sizeof(InterleavedVertex),
        .offsetUV1 = maxint,
        .strideUV1 = maxint,
        .vertexCount = vertexCount,
        .vertexSize = sizeof(interleavedVertices),
        .indexType = IndexType::UI16,
        .indexCount = 3,
        .indexSize = sizeof(uint16_t) * 3
    };
    const uint32_t nmats = 1;
    const string matname = "DefaultMaterial";
    const uint32_t matnamelength = matname.size();

    stringstream stream(ios_base::out);
    write(stream, MAGICID, sizeof(MAGICID));
    write(stream, &header, sizeof(header));
    write(stream, interleavedVertices, sizeof(interleavedVertices));
    write(stream, indices, sizeof(indices));
    write(stream, parts, sizeof(parts));
    write(stream, &nmats, sizeof(nmats));
    write(stream, &matnamelength, sizeof(matnamelength));
    write(stream, matname.c_str(), matnamelength + 1);

    // Deserialize the mesh as a smoke test.
    MaterialInstance* mi = engine->getDefaultMaterial()->createInstance();
    auto mesh = MeshReader::loadMeshFromBuffer(engine, stream.str().data(), nullptr, nullptr, mi);
    auto& rm = engine->getRenderableManager();
    auto inst = rm.getInstance(mesh.renderable);
    EXPECT_EQ(rm.getPrimitiveCount(inst), 1);

    // Cleanup.
    engine->destroy(mesh.renderable);
    engine->destroy(mi);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
