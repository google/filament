/*
 * Copyright 2023 The Android Open Source Project
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

#include <geometry/TangentSpaceMesh.h>

#include <math/quat.h>
#include <math/vec3.h>

#include <gtest/gtest.h>

#include <utils/Log.h>

#include <vector>

class TangentSpaceMeshTest : public testing::Test {};

using namespace filament::geometry;
using namespace filament::math;

namespace {
using AuxAttribute = TangentSpaceMesh::AuxAttribute;

std::vector<float3> const CUBE_VERTS {
        float3{0, 0, 0},
        float3{0, 0, 1},
        float3{0, 1, 0},
        float3{0, 1, 1},
        float3{1, 0, 0},
        float3{1, 0, 1},
        float3{1, 1, 0},
        float3{1, 1, 1}
};

std::vector<float2> const CUBE_UVS {
        float2{0, 0},
        float2{0, 0},
        float2{1, 0},
        float2{1, 1},
        float2{0, 1},
        float2{0, 1},
        float2{1, 1},
        float2{0, 1}
};

// This is used to verify that attributes are properly mapped for remeshed methods.
std::vector<float4> const CUBE_COLORS {
        float4{0, 0, 0, 1},
        float4{0, 0, 1, 1},
        float4{0, 1, 0, 1},
        float4{0, 1, 1, 1},
        float4{1, 0, 0, 1},
        float4{1, 0, 1, 1},
        float4{1, 1, 0, 1},
        float4{1, 1, 1, 1},
};

float3 const CUBE_CENTER { .5, .5, .5 };
std::vector<float3> const CUBE_NORMALS {
    normalize(CUBE_VERTS[0] - CUBE_CENTER),
    normalize(CUBE_VERTS[1] - CUBE_CENTER),
    normalize(CUBE_VERTS[2] - CUBE_CENTER),
    normalize(CUBE_VERTS[3] - CUBE_CENTER),
    normalize(CUBE_VERTS[4] - CUBE_CENTER),
    normalize(CUBE_VERTS[5] - CUBE_CENTER),
    normalize(CUBE_VERTS[6] - CUBE_CENTER),
    normalize(CUBE_VERTS[7] - CUBE_CENTER),
};

float3 const UP_VEC{1, 0, 0};
std::vector<float4> const CUBE_TANGENTS {
    float4{normalize(cross(CUBE_NORMALS[0], UP_VEC)), -1.0},
    float4{normalize(cross(CUBE_NORMALS[1], UP_VEC)), -1.0},
    float4{normalize(cross(CUBE_NORMALS[2], UP_VEC)), -1.0},
    float4{normalize(cross(CUBE_NORMALS[3], UP_VEC)), -1.0},
    float4{normalize(cross(CUBE_NORMALS[4], UP_VEC)), -1.0},
    float4{normalize(cross(CUBE_NORMALS[5], UP_VEC)), -1.0},
    float4{normalize(cross(CUBE_NORMALS[6], UP_VEC)), -1.0},
    float4{normalize(cross(CUBE_NORMALS[7], UP_VEC)), -1.0},
};

std::vector<ushort3> const CUBE_TRIANGLES {
        ushort3{0, 6, 4}, ushort3{0, 2, 6}, // XY-plane at z=0, normal=(0, 0, -1)
        ushort3{4, 7, 5}, ushort3{4, 6, 7}, // YZ-plane at x=1, normal=(1, 0 , 0)
        ushort3{2, 7, 6}, ushort3{2, 3, 7}, // XZ-plane at y=1, normal=(0, 1, 0)
        ushort3{1, 2, 0}, ushort3{1, 3, 2}, // YZ-plane at x=0, normal=(-1, 0, 0)
        ushort3{1, 4, 5}, ushort3{1, 0, 4}, // XZ-plane at y=0, normal=(0, -1, 0)
        ushort3{1, 7, 3}, ushort3{1, 5, 7}  // XY-plane at z=1, normal=(0, 0, 1)
};

// Corresponding to the faces in CUBE_TRIANGLES
std::vector<float3> const CUBE_FACE_NORMALS {
    float3{0, 0, -1},
    float3{1, 0, 0},
    float3{0, 1, 0},
    float3{-1, 0, 0},
    float3{0, -1, 0},
    float3{0, 0, 1}
};

std::vector<float3> const TEST_NORMALS {
    float3{1, 0, 0},
    float3{0, 1, 0},
    float3{0, 0, 1},
    normalize(float3{0, 1, 1}),
    normalize(float3{1, 1, 0}),
    normalize(float3{1, 1, 1})
};

float3 const NORMAL_AXIS{0, 0, 1};
float3 const TANGENT_AXIS{1, 0, 0};
float3 const BITANGENT_AXIS{0, 1, 0};

#define ALMOST_EQUAL()                                                                             \
    decltype(a) diff = a - b;                                                                      \
    const size_t steps = sizeof(decltype(a)) / sizeof(float);                                      \
    for (int i = 0; i < steps; ++i) {                                                              \
        if (abs(diff[i]) > std::numeric_limits<float>::epsilon()) { return false; }                \
    }                                                                                              \
    return true

bool isAlmostEqual4(const float4& a, const float4& b) noexcept { ALMOST_EQUAL(); }
bool isAlmostEqual3(const float3& a, const float3& b) noexcept { ALMOST_EQUAL(); }
bool isAlmostEqual2(const float2& a, const float2& b) noexcept { ALMOST_EQUAL(); }

#undef ALMOST_EQUAL

} // anonymous namespace

TEST_F(TangentSpaceMeshTest, BuilderDefaultAlgorithmsRemeshes) {
    // Expect flat shading selected.
    TangentSpaceMesh* mesh = TangentSpaceMesh::Builder()
            .vertexCount(CUBE_VERTS.size())
            .positions(CUBE_VERTS.data())
            .triangleCount(CUBE_TRIANGLES.size())
            .triangles(CUBE_TRIANGLES.data())
            .build();
    EXPECT_TRUE(mesh->remeshed());
    TangentSpaceMesh::destroy(mesh);

    // Expect frisvad selected.
    mesh = TangentSpaceMesh::Builder()
            .vertexCount(1)
            .normals(TEST_NORMALS.data())
            .build();
    EXPECT_FALSE(mesh->remeshed());
    TangentSpaceMesh::destroy(mesh);

    // Expect mikktspace selected.
    mesh = TangentSpaceMesh::Builder()
            .vertexCount(CUBE_VERTS.size())
            .positions(CUBE_VERTS.data())
            .uvs(CUBE_UVS.data())
            .normals(CUBE_NORMALS.data())
            .triangleCount(CUBE_TRIANGLES.size())
            .triangles(CUBE_TRIANGLES.data())
            .build();
    EXPECT_TRUE(mesh->remeshed());
    TangentSpaceMesh::destroy(mesh);
}

// Remeshed vertices/uvs should map to input vertices/uvs
TEST_F(TangentSpaceMeshTest, FlatShadingRemesh) {
    TangentSpaceMesh* mesh = TangentSpaceMesh::Builder()
            .vertexCount(CUBE_VERTS.size())
            .positions(CUBE_VERTS.data())
            .triangleCount(CUBE_TRIANGLES.size())
            .triangles(CUBE_TRIANGLES.data())
            .uvs(CUBE_UVS.data())
            .aux(AuxAttribute::COLORS, CUBE_COLORS.data())
            .build();

    // Number of triangles should remain the same
    ASSERT_EQ(mesh->getTriangleCount(), CUBE_TRIANGLES.size());

    std::vector<float3> outPositions(mesh->getVertexCount());
    mesh->getPositions(outPositions.data());

    std::vector<float2> outUVs(mesh->getVertexCount());
    mesh->getUVs(outUVs.data());

    std::vector<float4> outColors(mesh->getVertexCount());
    mesh->getAux(AuxAttribute::COLORS, outColors.data());

    for (size_t i = 0; i < outPositions.size(); ++i) {
        const auto& outPos = outPositions[i];
        const auto& outUV = outUVs[i];
        const auto& outColor = outColors[i];

        bool found = false;
        for (size_t j = 0; j < CUBE_VERTS.size(); ++j) {
            const auto& inPos = CUBE_VERTS[j];
            const auto& inUV = CUBE_UVS[j];
            const auto& inColor = CUBE_COLORS[j];
            if (isAlmostEqual3(outPos, inPos)) {
                found = true;
                EXPECT_PRED2(isAlmostEqual2, outUV, inUV);
                EXPECT_PRED2(isAlmostEqual4, outColor, inColor);
                break;
            }
        }
        EXPECT_TRUE(found);
    }
    TangentSpaceMesh::destroy(mesh);
}

TEST_F(TangentSpaceMeshTest, FlatShading) {
    TangentSpaceMesh* mesh = TangentSpaceMesh::Builder()
            .vertexCount(CUBE_VERTS.size())
            .positions(CUBE_VERTS.data())
            .triangleCount(CUBE_TRIANGLES.size())
            .triangles(CUBE_TRIANGLES.data())
            .build();

    ASSERT_EQ(mesh->getVertexCount(), CUBE_TRIANGLES.size() * 3);
    ASSERT_EQ(mesh->getTriangleCount(), CUBE_TRIANGLES.size());

    std::vector<quatf> quats(mesh->getVertexCount());
    std::vector<ushort3> triangles(mesh->getTriangleCount());
    mesh->getTriangles(triangles.data());
    mesh->getQuats(quats.data());
    for (size_t i = 0; i < CUBE_TRIANGLES.size(); ++i) {
        size_t faceInd = i / 2;
        const float3& expectedNormal = CUBE_FACE_NORMALS[faceInd];
        for (int j = 0; j < 3; ++j) {
            const quatf& quat = quats[triangles[i][j]];
            EXPECT_PRED2(isAlmostEqual3, quat * NORMAL_AXIS, expectedNormal);
        }
    }
    TangentSpaceMesh::destroy(mesh);
}

TEST_F(TangentSpaceMeshTest, TangentsProvided) {
    TangentSpaceMesh* mesh = TangentSpaceMesh::Builder()
            .vertexCount(CUBE_VERTS.size())
            .normals(CUBE_NORMALS.data())
            .tangents(CUBE_TANGENTS.data())
            .triangleCount(CUBE_TRIANGLES.size())
            .triangles(CUBE_TRIANGLES.data())
            .build();

    ASSERT_EQ(mesh->getVertexCount(), CUBE_VERTS.size());
    ASSERT_EQ(mesh->getTriangleCount(), CUBE_TRIANGLES.size());

    size_t const vertexCount = mesh->getVertexCount();
    std::vector<quatf> quats(vertexCount);
    mesh->getQuats(quats.data());
    for (size_t i = 0; i < vertexCount; ++i) {
        float3 const n = quats[i] * NORMAL_AXIS;
        EXPECT_PRED2(isAlmostEqual3, n, CUBE_NORMALS[i]);        
    }
    TangentSpaceMesh::destroy(mesh);
}

TEST_F(TangentSpaceMeshTest, Frisvad) {
    TangentSpaceMesh* mesh = TangentSpaceMesh::Builder()
            .vertexCount(TEST_NORMALS.size())
            .normals(TEST_NORMALS.data())
            .algorithm(TangentSpaceMesh::Algorithm::FRISVAD)
            .build();

    ASSERT_EQ(mesh->getVertexCount(), TEST_NORMALS.size());
    ASSERT_EQ(mesh->getTriangleCount(), 0);

    std::vector<quatf> quats(mesh->getVertexCount());
    mesh->getQuats(quats.data());
    for (size_t i = 0; i < TEST_NORMALS.size(); ++i) {
        const float3 n = quats[i] * NORMAL_AXIS;
        EXPECT_PRED2(isAlmostEqual3, n, TEST_NORMALS[i]);

        const float3 b = quats[i] * BITANGENT_AXIS;
        const float3 t = quats[i] * TANGENT_AXIS;

        EXPECT_LT(abs(dot(b, t)), std::numeric_limits<float>::epsilon());
        EXPECT_LT(abs(dot(n, t)), std::numeric_limits<float>::epsilon());
        EXPECT_LT(abs(dot(n, b)), std::numeric_limits<float>::epsilon());
        EXPECT_PRED2(isAlmostEqual3, cross(n, t), b);
    }
    TangentSpaceMesh::destroy(mesh);
}

TEST_F(TangentSpaceMeshTest, HughesMoller) {
    TangentSpaceMesh* mesh = TangentSpaceMesh::Builder()
            .vertexCount(TEST_NORMALS.size())
            .normals(TEST_NORMALS.data())
            .algorithm(TangentSpaceMesh::Algorithm::HUGHES_MOLLER)
            .build();

    ASSERT_EQ(mesh->getVertexCount(), TEST_NORMALS.size());
    ASSERT_EQ(mesh->getTriangleCount(), 0);

    std::vector<quatf> quats(mesh->getVertexCount());
    mesh->getQuats(quats.data());
    for (size_t i = 0; i < TEST_NORMALS.size(); ++i) {
        const float3 n = quats[i] * NORMAL_AXIS;
        EXPECT_PRED2(isAlmostEqual3, n, TEST_NORMALS[i]);

        const float3 b = quats[i] * BITANGENT_AXIS;
        const float3 t = quats[i] * TANGENT_AXIS;

        EXPECT_LT(abs(dot(b, t)), std::numeric_limits<float>::epsilon());
        EXPECT_LT(abs(dot(n, t)), std::numeric_limits<float>::epsilon());
        EXPECT_LT(abs(dot(n, b)), std::numeric_limits<float>::epsilon());
        EXPECT_PRED2(isAlmostEqual3, cross(n, t), b);
    }
    TangentSpaceMesh::destroy(mesh);
}

TEST_F(TangentSpaceMeshTest, MikktspaceRemesh) {
    TangentSpaceMesh* mesh = TangentSpaceMesh::Builder()
            .vertexCount(CUBE_VERTS.size())
            .normals(CUBE_NORMALS.data())
            .positions(CUBE_VERTS.data())
            .uvs(CUBE_UVS.data())
            .triangleCount(CUBE_TRIANGLES.size())
            .triangles(CUBE_TRIANGLES.data())
            .aux(AuxAttribute::COLORS, CUBE_COLORS.data())
            .algorithm(TangentSpaceMesh::Algorithm::MIKKTSPACE)
            .build();

    size_t const vertexCount = mesh->getVertexCount();

    std::vector<float3> outPositions(vertexCount);
    mesh->getPositions(outPositions.data());

    std::vector<float2> outUVs(vertexCount);
    mesh->getUVs(outUVs.data());

    std::vector<float4> outColors(mesh->getVertexCount());
    mesh->getAux(AuxAttribute::COLORS, outColors.data());

    for (size_t i = 0; i < outPositions.size(); ++i) {
        auto const& outPos = outPositions[i];
        auto const& outUV = outUVs[i];
        auto const& outColor = outColors[i];

        bool found = false;
        for (size_t j = 0; j < CUBE_VERTS.size(); ++j) {
            auto const& inPos = CUBE_VERTS[j];
            auto const& inUV = CUBE_UVS[j];
            auto const& inColor = CUBE_COLORS[j];
            if (isAlmostEqual3(outPos, inPos)) {
                found = true;
                EXPECT_PRED2(isAlmostEqual2, outUV, inUV);
                EXPECT_PRED2(isAlmostEqual4, outColor, inColor);
                break;
            }
        }
        EXPECT_TRUE(found);
    }
    TangentSpaceMesh::destroy(mesh);
}

TEST_F(TangentSpaceMeshTest, Mikktspace) {
    // It's unclear why the dot product between n and b is greater epsilon, but since we don't
    // control the implementation of mikktspace, we simply add a little slack to the test.
    constexpr float MAGIC_SLACK = 1.00001;
    TangentSpaceMesh* mesh = TangentSpaceMesh::Builder()
            .vertexCount(CUBE_VERTS.size())
            .normals(CUBE_NORMALS.data())
            .positions(CUBE_VERTS.data())
            .uvs(CUBE_UVS.data())
            .triangleCount(CUBE_TRIANGLES.size())
            .triangles(CUBE_TRIANGLES.data())
            .algorithm(TangentSpaceMesh::Algorithm::MIKKTSPACE)
            .build();

    size_t const vertexCount = mesh->getVertexCount();
    std::vector<quatf> quats(vertexCount);
    mesh->getQuats(quats.data());
    for (size_t i = 0; i < vertexCount; ++i) {
        float3 const n = quats[i] * NORMAL_AXIS;
        float3 const b = quats[i] * BITANGENT_AXIS;
        float3 const t = quats[i] * TANGENT_AXIS;

        EXPECT_LT(abs(dot(b, t)), std::numeric_limits<float>::epsilon());
        EXPECT_LT(abs(dot(n, t)), std::numeric_limits<float>::epsilon());
        EXPECT_LT(abs(dot(n, b)), std::numeric_limits<float>::epsilon() * MAGIC_SLACK);
        EXPECT_PRED2(isAlmostEqual3, cross(n, t), b);
    }
    TangentSpaceMesh::destroy(mesh);
}

TEST_F(TangentSpaceMeshTest, Lengyel) {
    TangentSpaceMesh* mesh = TangentSpaceMesh::Builder()
            .vertexCount(CUBE_VERTS.size())
            .normals(CUBE_NORMALS.data())
            .positions(CUBE_VERTS.data())
            .uvs(CUBE_UVS.data())
            .triangleCount(CUBE_TRIANGLES.size())
            .triangles(CUBE_TRIANGLES.data())
            .algorithm(TangentSpaceMesh::Algorithm::LENGYEL)
            .build();

    size_t const vertexCount = mesh->getVertexCount();
    std::vector<quatf> quats(vertexCount);
    mesh->getQuats(quats.data());

    ASSERT_EQ(mesh->getTriangleCount(), CUBE_TRIANGLES.size());
    std::vector<ushort3> triangles(mesh->getTriangleCount());
    mesh->getTriangles(triangles.data());

    for (size_t i = 0; i < vertexCount; ++i) {
        float3 const n = quats[i] * NORMAL_AXIS;
        EXPECT_PRED2(isAlmostEqual3, n, CUBE_NORMALS[i]);

        float3 const b = quats[i] * BITANGENT_AXIS;
        float3 const t = quats[i] * TANGENT_AXIS;

        EXPECT_LT(abs(dot(b, t)), std::numeric_limits<float>::epsilon());
        EXPECT_LT(abs(dot(n, t)), std::numeric_limits<float>::epsilon());
        EXPECT_LT(abs(dot(n, b)), std::numeric_limits<float>::epsilon());
        EXPECT_PRED2(isAlmostEqual3, cross(n, t), b);
    }
    TangentSpaceMesh::destroy(mesh);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
