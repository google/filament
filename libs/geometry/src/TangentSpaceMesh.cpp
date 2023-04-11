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

#include <geometry/TangentSpaceMesh.h>

#include "MikktspaceImpl.h"
#include "TangentSpaceMeshInternal.h"

#include <math/mat3.h>
#include <math/norm.h>

#include <utils/Log.h>
#include <utils/Panic.h>

#include <vector>

namespace filament {
namespace geometry {

using namespace filament::math;
using Builder = TangentSpaceMesh::Builder;
using Algorithm = TangentSpaceMesh::Algorithm;
using MethodPtr = void(*)(TangentSpaceMeshInput const*, TangentSpaceMeshOutput*);

namespace {

uint8_t const NORMALS_BIT = 0x01;
uint8_t const UVS_BIT = 0x02;
uint8_t const POSITIONS_BIT = 0x04;
uint8_t const INDICES_BIT = 0x08;

// Input types
uint8_t const NORMALS = NORMALS_BIT;
uint8_t const POSITIONS_INDICES = POSITIONS_BIT | INDICES_BIT;
uint8_t const NORMALS_UVS_POSITIONS_INDICES = NORMALS_BIT | UVS_BIT | POSITIONS_BIT | INDICES_BIT;

std::string_view to_string(Algorithm const algorithm) noexcept {
    switch (algorithm) {
        case Algorithm::DEFAULT:
            return "DEFAULT";
        case Algorithm::MIKKTSPACE:
            return "MIKKTSPACE";
        case Algorithm::LENGYEL:
            return "LENGYEL";
        case Algorithm::HUGHES_MOLLER:
            return "HUGHES_MOLLER";
        case Algorithm::FRISVAD:
            return "FRISVAD";
        case Algorithm::FLAT_SHADING:
            return "FLAT_SHADING";
    }
}

inline bool isInputType(uint8_t const inputType, uint8_t const checkType) noexcept {
    return ((inputType & checkType) == checkType);
}

template <typename InputType>
inline void takeStride(InputType*& out, size_t const stride) noexcept {
    out = pointerAdd(out, 1, stride);
}

inline Algorithm selectBestDefaultAlgorithm(uint8_t const inputType) {
    Algorithm outAlgo;
    if (isInputType(inputType, NORMALS_UVS_POSITIONS_INDICES)) {
        outAlgo = Algorithm::MIKKTSPACE;
    } else if (isInputType(inputType, POSITIONS_INDICES)) {
        outAlgo = Algorithm::FLAT_SHADING;
    } else {
        ASSERT_PRECONDITION(inputType & NORMALS,
                "Must at least have normals or (positions + indices) as input");
        outAlgo = Algorithm::FRISVAD;
    }
    return outAlgo;
}

Algorithm selectAlgorithm(TangentSpaceMeshInput *input) noexcept {
    uint8_t inputType = 0;
    if (input->normals) {
        inputType |= NORMALS_BIT;
    }
    if (input->positions) {
        inputType |= POSITIONS_BIT;
    }
    if (input->uvs) {
        inputType |= UVS_BIT;
    }
    if (input->triangles32 || input->triangles16) {
        inputType |= INDICES_BIT;
    }

    bool foundAlgo = false;
    Algorithm outAlgo = Algorithm::DEFAULT;
    switch (input->algorithm) {
        case Algorithm::DEFAULT:
            outAlgo = selectBestDefaultAlgorithm(inputType);
            foundAlgo = true;
            break;
        case Algorithm::MIKKTSPACE:
            if (isInputType(inputType, NORMALS_UVS_POSITIONS_INDICES)) {
                outAlgo = Algorithm::MIKKTSPACE;
                foundAlgo = true;
            }
            break;
        case Algorithm::LENGYEL:
            if (isInputType(inputType, NORMALS_UVS_POSITIONS_INDICES)) {
                outAlgo = Algorithm::LENGYEL;
                foundAlgo = true;
            }
            break;
        case Algorithm::HUGHES_MOLLER:
            if (isInputType(inputType, NORMALS)) {
                outAlgo = Algorithm::HUGHES_MOLLER;
                foundAlgo = true;
            }
            break;
        case Algorithm::FRISVAD:
            if (isInputType(inputType, NORMALS)) {
                outAlgo = Algorithm::FRISVAD;
                foundAlgo = true;
            }
            break;
        case Algorithm::FLAT_SHADING:
            if (isInputType(inputType, POSITIONS_INDICES)) {
                outAlgo = Algorithm::FLAT_SHADING;
                foundAlgo = true;
            }
            break;
    }

    if (!foundAlgo) {
        outAlgo = selectBestDefaultAlgorithm(inputType);
        utils::slog.w << "Cannot satisfy algorithm=" << to_string(input->algorithm)
            << ". Selected algorithm=" << to_string(outAlgo) << " instead"
            << utils::io::endl;
    }

    return outAlgo;
}

// The paper uses a Z-up world basis, which has been converted to Y-up here
inline std::pair<float3, float3> frisvadKernel(float3 const& n) {
    float3 b, t;
    if (n.y < -1.0f + std::numeric_limits<float>::epsilon()) {
        // Handle the singularity
        t = float3{-1.0f, 0.0f, 0.0f};
        b = float3{0.0f, 0.0f, -1.0f};
    } else {
        float const va = 1.0f / (1.0f + n.y);
        float const vb = -n.z * n.x * va;
        t = float3{vb, -n.z, 1.0f - n.z * n.z * va};
        b = float3{1.0f - n.x * n.x * va, -n.x, vb};
    }
    return {b, t};
}

void frisvadMethod(TangentSpaceMeshInput const* input, TangentSpaceMeshOutput* output)
        noexcept {
    size_t const vertexCount = input->vertexCount;
    quatf* quats = output->tangentSpace.allocate(vertexCount);

    float3 const* UTILS_RESTRICT normals = input->normals;
    size_t nstride = input->normalStride ? input->normalStride : sizeof(float3);

    for (size_t qindex = 0; qindex < vertexCount; ++qindex) {
        float3 const n = *normals;
        auto const [b, t] = frisvadKernel(n);
        quats[qindex] = mat3f::packTangentFrame({t, b, n}, sizeof(int32_t));
        normals = pointerAdd(normals, 1, nstride);
    }

    output->vertexCount = input->vertexCount;
    output->triangleCount = input->triangleCount;
    output->uvs.borrow(input->uvs);
    output->positions.borrow(input->positions);
    output->triangles32.borrow(input->triangles32);
    output->triangles16.borrow(input->triangles16);
}

void hughesMollerMethod(TangentSpaceMeshInput const* input, TangentSpaceMeshOutput* output)
        noexcept {
    size_t const vertexCount = input->vertexCount;
    quatf* quats = output->tangentSpace.allocate(vertexCount);

    float3 const* UTILS_RESTRICT normals = input->normals;
    size_t nstride = input->normalStride ? input->normalStride : sizeof(float3);

    for (size_t qindex = 0; qindex < vertexCount; ++qindex) {
        float3 const n = *normals;
        float3 b, t;

        if (abs(n.x) > abs(n.z) + std::numeric_limits<float>::epsilon()) {
            t = float3{-n.y, n.x, 0.0f};
        } else {
            t = float3{0.0f, -n.z, n.y};
        }
        t = normalize(t);
        b = cross(n, t);

        quats[qindex] = mat3f::packTangentFrame({t, b, n}, sizeof(int32_t));
        normals = pointerAdd(normals, 1, nstride);
    }
    output->vertexCount = input->vertexCount;
    output->triangleCount = input->triangleCount;
    output->uvs.borrow(input->uvs);
    output->positions.borrow(input->positions);
    output->triangles32.borrow(input->triangles32);
    output->triangles16.borrow(input->triangles16);
}

void flatShadingMethod(TangentSpaceMeshInput const* input, TangentSpaceMeshOutput* output)
        noexcept {
    float3 const* positions = input->positions;
    size_t const pstride = input->positionStride ? input->positionStride : sizeof(float3);

    float2 const* uvs = input->uvs;
    size_t const uvstride = input->uvStride ? input->uvStride : sizeof(float2);

    bool const isTriangle16 = input->triangles16 != nullptr;
    uint8_t const* triangles = isTriangle16 ? (uint8_t const*) input->triangles16 :
            (uint8_t const*) input->triangles32;

    size_t const triangleCount = input->triangleCount;
    size_t const tstride = isTriangle16 ? sizeof(ushort3) : sizeof(uint3);

    size_t const outVertexCount = triangleCount * 3;
    float3* outPositions = output->positions.allocate(outVertexCount);
    float2* outUvs = uvs ? output->uvs.allocate(outVertexCount) : nullptr;
    quatf* quats = output->tangentSpace.allocate(outVertexCount);

    size_t const outTriangleCount = triangleCount;
    uint3* outTriangles = output->triangles32.allocate(outTriangleCount);

    size_t vindex = 0;
    for (size_t tindex = 0; tindex < triangleCount; ++tindex) {
        uint3 tri = isTriangle16 ?
                uint3(*(ushort3*)(pointerAdd(triangles, tindex, tstride))) :
                *(uint3*)(pointerAdd(triangles, tindex, tstride));

        float3 const pa = *pointerAdd(positions, tri.x, pstride);
        float3 const pb = *pointerAdd(positions, tri.y, pstride);
        float3 const pc = *pointerAdd(positions, tri.z, pstride);

        uint32_t i0 = vindex++, i1 = vindex++, i2 = vindex++;
        outTriangles[tindex] = uint3{i0, i1, i2};

        outPositions[i0] = pa;
        outPositions[i1] = pb;
        outPositions[i2] = pc;

        float3 const n = normalize(cross(pc - pb, pa - pb));
        const auto [t, b] = frisvadKernel(n);

        quatf const tspace = mat3f::packTangentFrame({t, b, n}, sizeof(int32_t));
        quats[i0] = tspace;
        quats[i1] = tspace;
        quats[i2] = tspace;

        if (outUvs) {
            outUvs[i0] = *pointerAdd(uvs, tri.x, uvstride);
            outUvs[i1] = *pointerAdd(uvs, tri.y, uvstride);
            outUvs[i2] = *pointerAdd(uvs, tri.z, uvstride);
        }
    }

    output->vertexCount = outVertexCount;
    output->triangleCount = outTriangleCount;
}

void mikktspaceMethod(TangentSpaceMeshInput const* input, TangentSpaceMeshOutput* output) {
    MikktspaceImpl impl(input);
    impl.run(output);
}

inline float3 randomPerp(float3 const& n) {
    float3 perp = cross(n, float3{1, 0, 0});
    float sqrlen = dot(perp, perp);
    if (sqrlen <= std::numeric_limits<float>::epsilon()) {
        perp = cross(n, float3{0, 1, 0});
        sqrlen = dot(perp, perp);
    }
    return perp / sqrlen;
}

void lengyelMethod(TangentSpaceMeshInput const* input, TangentSpaceMeshOutput* output) {
    size_t const vertexCount = input->vertexCount;
    size_t const triangleCount = input->triangleCount;
    size_t const positionStride = input->positionStride ? input->positionStride : sizeof(float3);
    size_t const normalStride = input->normalStride ? input->normalStride : sizeof(float3);
    size_t const uvStride = input->uvStride ? input->uvStride : sizeof(float2);
    auto const* triangles16 = input->triangles16;
    auto const* triangles32 = input->triangles32;
    auto const* positions = input->positions;
    auto const* uvs = input->uvs;
    auto const* normals = input->normals;

    std::vector<float3> tan1(vertexCount, float3{0.0f});
    std::vector<float3> tan2(vertexCount, float3{0.0f});
    for (size_t a = 0; a < triangleCount; ++a) {
        uint3 tri = triangles16 ? uint3(triangles16[a]) : triangles32[a];
        assert_invariant(tri.x < vertexCount && tri.y < vertexCount && tri.z < vertexCount);
        float3 const& v1 = *pointerAdd(positions, tri.x, positionStride);
        float3 const& v2 = *pointerAdd(positions, tri.y, positionStride);
        float3 const& v3 = *pointerAdd(positions, tri.z, positionStride);
        float2 const& w1 = *pointerAdd(uvs, tri.x, uvStride);
        float2 const& w2 = *pointerAdd(uvs, tri.y, uvStride);
        float2 const& w3 = *pointerAdd(uvs, tri.z, uvStride);
        float const x1 = v2.x - v1.x;
        float const x2 = v3.x - v1.x;
        float const y1 = v2.y - v1.y;
        float const y2 = v3.y - v1.y;
        float const z1 = v2.z - v1.z;
        float const z2 = v3.z - v1.z;
        float const s1 = w2.x - w1.x;
        float const s2 = w3.x - w1.x;
        float const t1 = w2.y - w1.y;
        float const t2 = w3.y - w1.y;
        float const d = s1 * t2 - s2 * t1;
        float3 sdir, tdir;
        // In general we can't guarantee smooth tangents when the UV's are non-smooth, but let's at
        // least avoid divide-by-zero and fall back to normals-only method.
        if (d == 0.0) {
            float3 const& n1 = *pointerAdd(normals, tri.x, normalStride);
            sdir = randomPerp(n1);
            tdir = cross(n1, sdir);
        } else {
            sdir = {t2 * x1 - t1 * x2, t2 * y1 - t1 * y2, t2 * z1 - t1 * z2};
            tdir = {s1 * x2 - s2 * x1, s1 * y2 - s2 * y1, s1 * z2 - s2 * z1};
            float const r = 1.0f / d;
            sdir *= r;
            tdir *= r;
        }
        tan1[tri.x] += sdir;
        tan1[tri.y] += sdir;
        tan1[tri.z] += sdir;
        tan2[tri.x] += tdir;
        tan2[tri.y] += tdir;
        tan2[tri.z] += tdir;
    }

    quatf* quats = output->tangentSpace.allocate(vertexCount);
    for (size_t a = 0; a < vertexCount; a++) {
        float3 const& n = *pointerAdd(normals, a, normalStride);
        float3 const& t1 = tan1[a];
        float3 const& t2 = tan2[a];

        // Gram-Schmidt orthogonalize
        float3 const t = normalize(t1 - n * dot(n, t1));

        // Calculate handedness
        float const w = (dot(cross(n, t1), t2) < 0.0f) ? -1.0f : 1.0f;

        float3 b = w < 0 ? cross(t, n) : cross(n, t);
        quats[a] = mat3f::packTangentFrame({t, b, n}, sizeof(int32_t));
    }

    output->vertexCount = vertexCount;
    output->triangleCount = triangleCount;
    output->uvs.borrow(uvs);
    output->positions.borrow(positions);
    output->triangles32.borrow(triangles32);
    output->triangles16.borrow(triangles16);
}

} // anonymous namespace

Builder::Builder() noexcept
        :mMesh(new TangentSpaceMesh()) {}

Builder::~Builder() noexcept {
    delete mMesh;
}

Builder::Builder(Builder&& that) noexcept {
    std::swap(mMesh, that.mMesh);
}

Builder& Builder::operator=(Builder&& that) noexcept {
    std::swap(mMesh, that.mMesh);
    return *this;
}

Builder& Builder::vertexCount(size_t vertexCount) noexcept {
    mMesh->mInput->vertexCount = vertexCount;
    return *this;
}

Builder& Builder::normals(float3 const* normals, size_t stride) noexcept {
    mMesh->mInput->normals = normals;
    mMesh->mInput->normalStride = stride;
    return *this;
}

Builder& Builder::uvs(float2 const* uvs, size_t stride) noexcept {
    mMesh->mInput->uvs = uvs;
    mMesh->mInput->uvStride = stride;
    return *this;
}

Builder& Builder::positions(float3 const* positions, size_t stride) noexcept {
    mMesh->mInput->positions = positions;
    mMesh->mInput->positionStride = stride;
    return *this;
}

Builder& Builder::triangleCount(size_t triangleCount) noexcept {
    mMesh->mInput->triangleCount = triangleCount;
    return *this;
}

Builder& Builder::triangles(uint3 const* triangle32) noexcept {
    mMesh->mInput->triangles32 = triangle32;
    return *this;
}

Builder& Builder::triangles(ushort3 const* triangle16) noexcept {
    mMesh->mInput->triangles16 = triangle16;
    return *this;
}

Builder& Builder::algorithm(Algorithm algo) noexcept {
    mMesh->mInput->algorithm = algo;
    return *this;
}

TangentSpaceMesh* Builder::build() {
    ASSERT_PRECONDITION(!mMesh->mInput->triangles32 || !mMesh->mInput->triangles16,
            "Cannot provide both uint32 triangles and uint16 triangles");

    // Work in progress. Not for use.
    mMesh->mOutput->algorithm = selectAlgorithm(mMesh->mInput);
    MethodPtr method = nullptr;
    switch (mMesh->mOutput->algorithm) {
        case Algorithm::MIKKTSPACE:
            method = mikktspaceMethod;
            break;
        case Algorithm::LENGYEL:
            method = lengyelMethod;
            break;
        case Algorithm::HUGHES_MOLLER:
            method = hughesMollerMethod;
            break;
        case Algorithm::FRISVAD:
            method = frisvadMethod;
            break;
        case Algorithm::FLAT_SHADING:
            method = flatShadingMethod;
            break;
        default:
            break;
    }
    assert_invariant(method);
    method(mMesh->mInput, mMesh->mOutput);

    auto meshPtr = mMesh;
    // Reset the state.
    mMesh = new TangentSpaceMesh();

    return meshPtr;
}

void TangentSpaceMesh::destroy(TangentSpaceMesh* mesh) noexcept {
    delete mesh;
}

TangentSpaceMesh::TangentSpaceMesh() noexcept
        :mInput(new TangentSpaceMeshInput()), mOutput(new TangentSpaceMeshOutput()) {
}

TangentSpaceMesh::~TangentSpaceMesh() noexcept {
    delete mOutput;
    delete mInput;
}

TangentSpaceMesh::TangentSpaceMesh(TangentSpaceMesh&& that) noexcept {
    std::swap(mInput, that.mInput);
    std::swap(mOutput, that.mOutput);
}

TangentSpaceMesh& TangentSpaceMesh::operator=(TangentSpaceMesh&& that) noexcept {
    std::swap(mInput, that.mInput);
    std::swap(mOutput, that.mOutput);
    return *this;
}

size_t TangentSpaceMesh::getVertexCount() const noexcept {
    return mOutput->vertexCount;
}

void TangentSpaceMesh::getPositions(float3* positions, size_t stride) const {
    ASSERT_PRECONDITION(mInput->positions, "Must provide input positions");
    stride = stride ? stride : sizeof(decltype(*positions));
    auto outPositions = mOutput->positions.get();
    for (size_t i = 0; i < mOutput->vertexCount; ++i) {
        *positions = outPositions[i];
        takeStride(positions, stride);
    }
}

void TangentSpaceMesh::getUVs(float2* uvs, size_t stride) const {
    ASSERT_PRECONDITION(mInput->uvs, "Must provide input UVs");
    stride = stride ? stride : sizeof(decltype(*uvs));
    auto outUvs = mOutput->uvs.get();
    for (size_t i = 0; i < mOutput->vertexCount; ++i) {
        *uvs = outUvs[i];
        takeStride(uvs, stride);
    }
}

size_t TangentSpaceMesh::getTriangleCount() const noexcept {
    return mOutput->triangleCount;
}

void TangentSpaceMesh::getTriangles(uint3* out) const {
    ASSERT_PRECONDITION(mInput->triangles16 || mInput->triangles32, "Must provide input triangles");

    bool const is16 = (bool) mOutput->triangles16;
    auto triangles16 = mOutput->triangles16.get();
    auto triangles32 = mOutput->triangles32.get();
    size_t const stride = sizeof(decltype(*out));
    for (size_t i = 0; i < mOutput->triangleCount; ++i) {
        *out = is16 ? uint3{triangles16[i]} : triangles32[i];
        takeStride(out, stride);
    }
}

void TangentSpaceMesh::getTriangles(ushort3* out) const {
    ASSERT_PRECONDITION(mInput->triangles16 || mInput->triangles32, "Must provide input triangles");

    const bool is16 = (bool) mOutput->triangles16;
    auto triangles16 = mOutput->triangles16.get();
    auto triangles32 = mOutput->triangles32.get();
    const size_t stride = sizeof(decltype(*out));
    for (size_t i = 0, c = mOutput->triangleCount; i < c; ++i) {
        if (is16) {
            *out = triangles16[i];
        } else {
            uint3 const& tri = triangles32[i];
            ASSERT_PRECONDITION(tri.x <= USHRT_MAX &&
                                tri.y <= USHRT_MAX &&
                                tri.z <= USHRT_MAX, "Overflow when casting uint3 to ushort3");
            *out = ushort3{static_cast<uint16_t>(tri.x),
                           static_cast<uint16_t>(tri.y),
                           static_cast<uint16_t>(tri.z)};
        }
        takeStride(out, stride);
    }
}

void TangentSpaceMesh::getQuats(quatf* out, size_t stride) const noexcept {
    stride = stride ? stride : sizeof(decltype((*out)));
    auto tangentSpace = mOutput->tangentSpace.get();
    size_t const vertexCount = mOutput->vertexCount;
    for (size_t i = 0; i < vertexCount; ++i) {
        *out = tangentSpace[i];
        takeStride(out, stride);
    }
}

void TangentSpaceMesh::getQuats(short4* out, size_t stride) const noexcept {
    stride = stride ? stride : sizeof(decltype((*out)));
    auto tangentSpace = mOutput->tangentSpace.get();
    size_t const vertexCount = mOutput->vertexCount;
    for (size_t i = 0; i < vertexCount; ++i) {
        *out = packSnorm16(tangentSpace[i].xyzw);
        takeStride(out, stride);
    }
}

void TangentSpaceMesh::getQuats(quath* out, size_t stride) const noexcept {
    stride = stride ? stride : sizeof(decltype((*out)));
    auto tangentSpace = mOutput->tangentSpace.get();
    size_t const vertexCount = mOutput->vertexCount;
    for (size_t i = 0; i < vertexCount; ++i) {
        *out = quath(tangentSpace[i].xyzw);
        takeStride(out, stride);
    }
}

Algorithm TangentSpaceMesh::getAlgorithm() const noexcept {
    return mOutput->algorithm;
}

}
}
