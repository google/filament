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

#include <math/mat3.h>
#include <math/norm.h>

#include <utils/Log.h>
#include <utils/Panic.h>

namespace filament {
namespace geometry {

using namespace filament::math;
using Builder = TangentSpaceMesh::Builder;
using Algorithm = TangentSpaceMesh::Algorithm;
using MethodPtr = void(*)(const TangentSpaceMeshInput*, TangentSpaceMeshOutput*);
using NormalsOnlyKernelPtr = void(*)(const float3& N, float3& T, float3& B);

struct TangentSpaceMeshInput {
    size_t vertexCount = 0;
    const float3* normals = nullptr;
    const float2* uvs = nullptr;
    const float3* positions = nullptr;
    const ushort3* triangles16 = nullptr;
    const uint3* triangles32 = nullptr;

    size_t normalStride = 0;
    size_t uvStride = 0;
    size_t positionStride = 0;
    size_t triangleCount = 0;

    Algorithm algorithm;
};

struct TangentSpaceMeshOutput {
    Algorithm algorithm;

    size_t triangleCount = 0;
    size_t vertexCount = 0;

    const quatf* tangentSpace = nullptr;
    const float2* uvs = nullptr;
    const float3* positions = nullptr;
    const uint3* triangles32 = nullptr;
    const ushort3* triangles16 = nullptr;
};

namespace {

const uint8_t NORMALS_BIT = 0x01;
const uint8_t UVS_BIT = 0x02;
const uint8_t POSITIONS_BIT = 0x04;
const uint8_t INDICES_BIT = 0x08;

// Input types
const uint8_t NORMALS = NORMALS_BIT;
const uint8_t POSITIONS_INDICES = POSITIONS_BIT | INDICES_BIT;
const uint8_t NORMALS_UVS_POSITIONS_INDICES = NORMALS_BIT | UVS_BIT | POSITIONS_BIT | INDICES_BIT;

std::string_view to_string(Algorithm algorithm) {
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
        default:
            PANIC_POSTCONDITION("Unknown algorithm %u", static_cast<uint8_t>(algorithm));
    }
}

#define IS_INPUT_TYPE(inputType, TYPE) ((inputType & TYPE) == TYPE)

Algorithm selectBestDefaultAlgorithm(uint8_t inputType) {
    Algorithm outAlgo;
    if (IS_INPUT_TYPE(inputType, NORMALS_UVS_POSITIONS_INDICES)) {
        outAlgo = Algorithm::MIKKTSPACE;
    } else if (IS_INPUT_TYPE(inputType, POSITIONS_INDICES)) {
        outAlgo = Algorithm::FLAT_SHADING;
    } else {
        ASSERT_PRECONDITION(inputType & NORMALS,
                "Must at least have normals or (positions + indices) as input");
        outAlgo = Algorithm::FRISVAD;
    }
    return outAlgo;
}

Algorithm selectAlgorithm(TangentSpaceMeshInput *input) {
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
            if (IS_INPUT_TYPE(inputType, NORMALS_UVS_POSITIONS_INDICES)) {
                outAlgo = Algorithm::MIKKTSPACE;
                foundAlgo = true;
            }
            break;
        case Algorithm::LENGYEL:
            if (IS_INPUT_TYPE(inputType, NORMALS_UVS_POSITIONS_INDICES)) {
                outAlgo = Algorithm::LENGYEL;
                foundAlgo = true;
            }
            break;
        case Algorithm::HUGHES_MOLLER:
            if (IS_INPUT_TYPE(inputType, NORMALS)) {
                outAlgo = Algorithm::HUGHES_MOLLER;
                foundAlgo = true;
            }
            break;
        case Algorithm::FRISVAD:
            if (IS_INPUT_TYPE(inputType, NORMALS)) {
                outAlgo = Algorithm::FRISVAD;
                foundAlgo = true;
            }
            break;
        case Algorithm::FLAT_SHADING:
            if (IS_INPUT_TYPE(inputType, POSITIONS_INDICES)) {
                outAlgo = Algorithm::FLAT_SHADING;
                foundAlgo = true;
            }
            break;
        default:
            PANIC_POSTCONDITION("Unknown algo %u", static_cast<uint8_t>(input->algorithm));
    }

    if (!foundAlgo) {
        outAlgo = selectBestDefaultAlgorithm(inputType);
        utils::slog.w << "Cannot satisfy algorithm=" << to_string(input->algorithm)
            << ". Selected algorithm=" << to_string(input->algorithm) << " instead"
            << utils::io::endl;
    }

    return outAlgo;
}
#undef IS_INPUT_TYPE

// The paper uses a Z-up world basis, which has been converted to Y-up here
void frisvadKernel(const float3& N, float3& T, float3& B) {
    if (N.y < -1.0f + std::numeric_limits<float>::epsilon()) {
        // Handle the singularity
        T = float3{-1.0f, 0.0f, 0.0f};
        B = float3{0.0f, 0.0f, -1.0f};
        return;
    }
    const float a = 1.0f / (1.0f + N.y);
    const float b = -N.z * N.x * a;
    T = float3(b, -N.z, 1.0f - N.z * N.z * a);
    B = float3(1.0f - N.x * N.x * a, -N.x, b);
}

void hughesMollerKernel(const float3& N, float3& T, float3& B) {
    if (abs(N.x) > abs(N.z) + std::numeric_limits<float>::epsilon()) {
        T = float3(-N.y, N.x, 0.0f);
    } else {
        T = float3(0.0f, -N.z, N.y);
    }
    T = normalize(T);
    B = cross(N, T);
}

void normalsOnlyMethod(const TangentSpaceMeshInput* input, TangentSpaceMeshOutput* output,
    NormalsOnlyKernelPtr kernel) {
    const size_t vertexCount = input->vertexCount;
    quatf* quats = new quatf[vertexCount];

    const float3* normal = input->normals;
    size_t nstride = input->normalStride ? input->normalStride : sizeof(float3);

    for (size_t qindex = 0; qindex < vertexCount; ++qindex) {
        float3 n = *normal;
        float3 b, t;
        kernel(n, t, b);
        quats[qindex] = mat3f::packTangentFrame({t, b, n});
        normal = (const float3*) (((const uint8_t*) normal) + nstride);
    }
    output->tangentSpace = quats;
    output->vertexCount = input->vertexCount;\
    output->triangleCount = input->triangleCount;\
    output->uvs = input->uvs;\
    output->positions = input->positions;\
    output->triangles32 = input->triangles32;\
    output->triangles16 = input->triangles16;
}

#define STRIDE_HELPER(ptr, index, stride, cast_type)\
    (*((cast_type*) (((const uint8_t*) ptr) + index * stride)))

void flatShadingMethod(const TangentSpaceMeshInput* input, TangentSpaceMeshOutput* output) {
    const size_t vertexCount = input->vertexCount;

    const float3* positions = input->positions;
    const size_t pstride = input->positionStride ? input->positionStride : sizeof(float3);

    const float2* uvs = input->uvs;
    const size_t uvstride = input->uvStride ? input->uvStride : sizeof(float2);

    const bool isTriangle16 = input->triangles16 != nullptr;
    const uint8_t* triangles = isTriangle16 ? (const uint8_t*) input->triangles16 :
            (const uint8_t*) input->triangles32;

    const size_t triangleCount = input->triangleCount;
    const size_t tstride = input->triangles16 ? sizeof(ushort3) : sizeof(uint3);

    const size_t outVertexCount = triangleCount * 3;
    float3* outPositions = new float3[outVertexCount];
    float2* outUvs = uvs ? new float2[outVertexCount] : nullptr;
    quatf* quats = new quatf[outVertexCount];

    const size_t outTriangleCount = triangleCount;
    uint3* outTriangles = new uint3[outTriangleCount];

    size_t vindex = 0;
    for (size_t tindex = 0; tindex < triangleCount; ++tindex) {
        uint3 tri = isTriangle16 ?
                uint3(STRIDE_HELPER(triangles, tindex, tstride, ushort3)) :
                STRIDE_HELPER(triangles, tindex, tstride, uint3);
        const float3 pa = STRIDE_HELPER(positions, tri.x, pstride, float3);
        const float3 pb = STRIDE_HELPER(positions, tri.y, pstride, float3);
        const float3 pc = STRIDE_HELPER(positions, tri.z, pstride, float3);

        uint i0 = vindex++, i1 = vindex++, i2 = vindex++;
        outTriangles[tindex] = uint3(i0, i1, i2);

        outPositions[i0] = pa;
        outPositions[i1] = pb;
        outPositions[i2] = pc;

        const float3 n = normalize(cross(pc - pb, pa - pb));
        float3 t, b;
        frisvadKernel(n, t, b);

        const quatf tspace = mat3f::packTangentFrame({t, b, n});
        quats[i0] = tspace;
        quats[i1] = tspace;
        quats[i2] = tspace;

        if (outUvs) {
            outUvs[i0] = STRIDE_HELPER(uvs, tri.x, uvstride, float2);
            outUvs[i1] = STRIDE_HELPER(uvs, tri.y, uvstride, float2);
            outUvs[i2] = STRIDE_HELPER(uvs, tri.z, uvstride, float2);
        }
    }

    output->tangentSpace = quats;
    output->positions = outPositions;
    output->vertexCount = outVertexCount;
    output->uvs = outUvs;
    output->triangles32 = outTriangles;
    output->triangleCount = outTriangleCount;
}
#undef STRIDE_HELPER

} // anonymous namespace

Builder::Builder() noexcept
        :mMesh(new TangentSpaceMesh()) {}

Builder::~Builder() noexcept { }

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

Builder& Builder::normals(const float3* normals, size_t stride) noexcept {
    mMesh->mInput->normals = normals;
    mMesh->mInput->normalStride = stride;
    return *this;
}

Builder& Builder::uvs(const float2* uvs, size_t stride) noexcept {
    mMesh->mInput->uvs = uvs;
    mMesh->mInput->uvStride = stride;
    return *this;
}

Builder& Builder::positions(const float3* positions, size_t stride) noexcept {
    mMesh->mInput->positions = positions;
    mMesh->mInput->positionStride = stride;
    return *this;
}

Builder& Builder::triangleCount(size_t triangleCount) noexcept {
    mMesh->mInput->triangleCount = triangleCount;
    return *this;
}

Builder& Builder::triangles(const uint3* triangle32) noexcept {
    ASSERT_PRECONDITION(!mMesh->mInput->triangles16,
            "Triangles already provided in unsigned shorts");
    mMesh->mInput->triangles32 = triangle32;
    return *this;
}

Builder& Builder::triangles(const ushort3* triangle16) noexcept {
    ASSERT_PRECONDITION(!mMesh->mInput->triangles32, "Triangles already provided in unsigned ints");
    mMesh->mInput->triangles16 = triangle16;
    return *this;
}

Builder& Builder::algorithm(Algorithm algo) noexcept {
    mMesh->mInput->algorithm = algo;
    return *this;
}

TangentSpaceMesh* Builder::build() {
    // Work in progress. Not for use.
    Algorithm algo = selectAlgorithm(mMesh->mInput);
    MethodPtr method = nullptr;
    switch (algo) {
        case Algorithm::FRISVAD:
            method = [](const TangentSpaceMeshInput* input, TangentSpaceMeshOutput* output) {
                normalsOnlyMethod(input, output, frisvadKernel);
            };
            break;
        case Algorithm::HUGHES_MOLLER:
            method = [](const TangentSpaceMeshInput* input, TangentSpaceMeshOutput* output) {
                normalsOnlyMethod(input, output, hughesMollerKernel);
            };
            break;
        case Algorithm::FLAT_SHADING:
            method = flatShadingMethod;
            break;
        default:
            break;
    }
    assert_invariant(method);
    method(mMesh->mInput, mMesh->mOutput);
    return mMesh;
}

TangentSpaceMesh::TangentSpaceMesh() noexcept
        :mInput(new TangentSpaceMeshInput()), mOutput(new TangentSpaceMeshOutput()) {
}

#define CLEAN_HELPER(ptr, inputPtr) \
   if (ptr && ptr != inputPtr) delete [] ptr;\
   ptr = nullptr

TangentSpaceMesh::~TangentSpaceMesh() noexcept {
    CLEAN_HELPER(mOutput->tangentSpace, nullptr);
    CLEAN_HELPER(mOutput->uvs, mInput->uvs);
    CLEAN_HELPER(mOutput->positions, mInput->positions);
    CLEAN_HELPER(mOutput->triangles16, mInput->triangles16);
    CLEAN_HELPER(mOutput->triangles32, mInput->triangles32);
}
#undef CLEAN_HELPER

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

#define STRIDE_TO(out, stride) (out = (decltype(out)) (((uint8_t*) out) + stride))

void TangentSpaceMesh::getPositions(float3* positions, size_t stride) const {
    ASSERT_PRECONDITION(mInput->positions, "Must provide input positions");
    stride = stride ? stride : sizeof(decltype(*positions));
    for (size_t i = 0; i < mOutput->vertexCount; ++i) {
        *positions = mOutput->positions[i];
        STRIDE_TO(positions, stride);
    }
}

void TangentSpaceMesh::getUVs(float2* uvs, size_t stride) const {
    ASSERT_PRECONDITION(mInput->uvs, "Must provide input UVs");
    stride = stride ? stride : sizeof(decltype(*uvs));
    for (size_t i = 0; i < mOutput->vertexCount; ++i) {
        *uvs = mOutput->uvs[i];
        STRIDE_TO(uvs, stride);
    }
}

size_t TangentSpaceMesh::getTriangleCount() const noexcept {
    return mOutput->triangleCount;
}

void TangentSpaceMesh::getTriangles(uint3* out) const {
    ASSERT_PRECONDITION(mInput->triangles16 || mInput->triangles32, "Must provide input triangles");

    const bool is16 = mOutput->triangles16 != nullptr;
    const size_t stride = sizeof(decltype(*out));
    for (size_t i = 0; i < mOutput->triangleCount; ++i) {
        *out = is16 ? uint3(mOutput->triangles16[i]) : mOutput->triangles32[i];
        STRIDE_TO(out, stride);
    }
}

void TangentSpaceMesh::getTriangles(ushort3* out) const {
    ASSERT_PRECONDITION(mInput->triangles16 || mInput->triangles32, "Must provide input triangles");

    const bool is16 = mOutput->triangles16 != nullptr;
    const size_t stride = sizeof(decltype(*out));
    for (size_t i = 0; i < mOutput->triangleCount; ++i) {
        if (is16) {
            *out = mOutput->triangles16[i];
        } else {
            const uint3& tri = mOutput->triangles32[i];
            ASSERT_PRECONDITION(tri.x <= USHRT_MAX &&
                                tri.y <= USHRT_MAX &&
                                tri.z <= USHRT_MAX, "Overflow when casting uint3 to ushort3");
            *out = ushort3(static_cast<uint16_t>(tri.x),
                    static_cast<uint16_t>(tri.y),
                    static_cast<uint16_t>(tri.z));
        }
        STRIDE_TO(out, stride);
    }
}

#define GET_QUATS_IMPL(out, stride, conversion) \
    stride = stride ? stride : sizeof(decltype(*out));\
    for (size_t i = 0; i < mOutput->vertexCount; ++i) {\
        *out = conversion(mOutput->tangentSpace[i].xyzw);\
        STRIDE_TO(out, stride);\
    }

void TangentSpaceMesh::getQuats(quatf* out, size_t stride) const noexcept {
    GET_QUATS_IMPL(out, stride, quatf);
}

void TangentSpaceMesh::getQuats(short4* out, size_t stride) const noexcept {
    GET_QUATS_IMPL(out, stride, packSnorm16);
}

void TangentSpaceMesh::getQuats(quath* out, size_t stride) const noexcept {
    GET_QUATS_IMPL(out, stride, quath);
}
#undef GET_QUATS_IMPL
#undef STRIDE_TO

Algorithm TangentSpaceMesh::getAlgorithm() const noexcept {
    return mOutput->algorithm;
}

}
}
