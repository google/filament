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

    quatf const* tangentSpace = nullptr;
    float2 const* uvs = nullptr;
    float3 const* positions = nullptr;
    uint3 const* triangles32 = nullptr;
    ushort3 const* triangles16 = nullptr;
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

std::string_view to_string(Algorithm algorithm) noexcept {
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

inline bool isInputType(const uint8_t inputType, const uint8_t checkType) noexcept {
    return ((inputType & checkType) == checkType);
}

template<typename InputType>
inline const InputType* pointerAdd(const InputType* ptr, size_t index, size_t stride) noexcept {
    return (InputType*) (((const uint8_t*) ptr) + (index * stride));
}

template<typename InputType>
inline InputType* pointerAdd(InputType* ptr, size_t index, size_t stride) noexcept {
    return (InputType*) (((uint8_t*) ptr) + (index * stride));
}

inline Algorithm selectBestDefaultAlgorithm(uint8_t inputType) {
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
            << ". Selected algorithm=" << to_string(input->algorithm) << " instead"
            << utils::io::endl;
    }

    return outAlgo;
}

// The paper uses a Z-up world basis, which has been converted to Y-up here
inline std::pair<float3, float3> frisvadKernel(const float3& n) {
    float3 b, t;
    if (n.y < -1.0f + std::numeric_limits<float>::epsilon()) {
        // Handle the singularity
        t = float3{-1.0f, 0.0f, 0.0f};
        b = float3{0.0f, 0.0f, -1.0f};
    } else {
        const float va = 1.0f / (1.0f + n.y);
        const float vb = -n.z * n.x * va;
        t = float3{vb, -n.z, 1.0f - n.z * n.z * va};
        b = float3{1.0f - n.x * n.x * va, -n.x, vb};
    }
    return {b, t};
}

void frisvadMethod(const TangentSpaceMeshInput* input, TangentSpaceMeshOutput* output)
        noexcept {
    const size_t vertexCount = input->vertexCount;
    quatf* quats = new quatf[vertexCount];

    const float3* UTILS_RESTRICT normals = input->normals;
    size_t nstride = input->normalStride ? input->normalStride : sizeof(float3);

    for (size_t qindex = 0; qindex < vertexCount; ++qindex) {
        const float3 n = *normals;
        const auto [b, t] = frisvadKernel(n);
        quats[qindex] = mat3f::packTangentFrame({t, b, n}, sizeof(int32_t));
        normals = pointerAdd(normals, 1, nstride);
    }
    output->tangentSpace = quats;
    output->vertexCount = input->vertexCount;
    output->triangleCount = input->triangleCount;
    output->uvs = input->uvs;
    output->positions = input->positions;
    output->triangles32 = input->triangles32;
    output->triangles16 = input->triangles16;
}


void hughesMollerMethod(const TangentSpaceMeshInput* input, TangentSpaceMeshOutput* output)
        noexcept {
    const size_t vertexCount = input->vertexCount;
    quatf* quats = new quatf[vertexCount];

    const float3* UTILS_RESTRICT normals = input->normals;
    size_t nstride = input->normalStride ? input->normalStride : sizeof(float3);

    for (size_t qindex = 0; qindex < vertexCount; ++qindex) {
        const float3 n = *normals;
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
    output->tangentSpace = quats;
    output->vertexCount = input->vertexCount;
    output->triangleCount = input->triangleCount;
    output->uvs = input->uvs;
    output->positions = input->positions;
    output->triangles32 = input->triangles32;
    output->triangles16 = input->triangles16;
}

void flatShadingMethod(const TangentSpaceMeshInput* input, TangentSpaceMeshOutput* output)
        noexcept {
    const float3* positions = input->positions;
    const size_t pstride = input->positionStride ? input->positionStride : sizeof(float3);

    const float2* uvs = input->uvs;
    const size_t uvstride = input->uvStride ? input->uvStride : sizeof(float2);

    const bool isTriangle16 = input->triangles16 != nullptr;
    const uint8_t* triangles = isTriangle16 ? (const uint8_t*) input->triangles16 :
            (const uint8_t*) input->triangles32;

    const size_t triangleCount = input->triangleCount;
    const size_t tstride = isTriangle16 ? sizeof(ushort3) : sizeof(uint3);

    const size_t outVertexCount = triangleCount * 3;
    float3* outPositions = new float3[outVertexCount];
    float2* outUvs = uvs ? new float2[outVertexCount] : nullptr;
    quatf* quats = new quatf[outVertexCount];

    const size_t outTriangleCount = triangleCount;
    uint3* outTriangles = new uint3[outTriangleCount];

    size_t vindex = 0;
    for (size_t tindex = 0; tindex < triangleCount; ++tindex) {
        uint3 tri = isTriangle16 ?
                uint3(*(ushort3*)(pointerAdd(triangles, tindex, tstride))) :
                *(uint3*)(pointerAdd(triangles, tindex, tstride));

        const float3 pa = *pointerAdd(positions, tri.x, pstride);
        const float3 pb = *pointerAdd(positions, tri.y, pstride);
        const float3 pc = *pointerAdd(positions, tri.z, pstride);

        uint32_t i0 = vindex++, i1 = vindex++, i2 = vindex++;
        outTriangles[tindex] = uint3{i0, i1, i2};

        outPositions[i0] = pa;
        outPositions[i1] = pb;
        outPositions[i2] = pc;

        const float3 n = normalize(cross(pc - pb, pa - pb));
        const auto [t, b] = frisvadKernel(n);

        const quatf tspace = mat3f::packTangentFrame({t, b, n}, sizeof(int32_t));
        quats[i0] = tspace;
        quats[i1] = tspace;
        quats[i2] = tspace;

        if (outUvs) {
            outUvs[i0] = *pointerAdd(uvs, tri.x, uvstride);
            outUvs[i1] = *pointerAdd(uvs, tri.y, uvstride);
            outUvs[i2] = *pointerAdd(uvs, tri.z, uvstride);
        }
    }

    output->tangentSpace = quats;
    output->positions = outPositions;
    output->vertexCount = outVertexCount;
    output->uvs = outUvs;
    output->triangles32 = outTriangles;
    output->triangleCount = outTriangleCount;
}

template<typename DataType, typename InputType>
inline void cleanOutputPointer(DataType*& ptr, InputType inputPtr) noexcept {
    if (ptr && ptr != (const DataType*) inputPtr) {
        delete[] ptr;
    }
    ptr = nullptr;
}

template <typename DataType>
inline void takeStride(DataType*& out, size_t stride) noexcept {
    out = (DataType*) (((uint8_t*) out) + stride);
}

template <typename OutputType>
using QuatConversionFunc = OutputType(*)(const quatf&);

template<typename OutputType>
inline void getQuatsImpl(OutputType* UTILS_RESTRICT out, const quatf* UTILS_RESTRICT tangentSpace,
        size_t vertexCount, size_t stride, QuatConversionFunc<OutputType> conversion) noexcept {
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
    mMesh->mInput->triangles32 = triangle32;
    return *this;
}

Builder& Builder::triangles(const ushort3* triangle16) noexcept {
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
        case Algorithm::FRISVAD:
            method = frisvadMethod;
            break;
        case Algorithm::HUGHES_MOLLER:
            method = hughesMollerMethod;
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
    cleanOutputPointer(mOutput->tangentSpace, (quatf*) nullptr);
    cleanOutputPointer(mOutput->uvs, mInput->uvs);
    cleanOutputPointer(mOutput->positions, mInput->positions);
    cleanOutputPointer(mOutput->triangles16, mInput->triangles16);
    cleanOutputPointer(mOutput->triangles32, mInput->triangles32);

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
    for (size_t i = 0; i < mOutput->vertexCount; ++i) {
        *positions = mOutput->positions[i];
        takeStride(positions, stride);
    }
}

void TangentSpaceMesh::getUVs(float2* uvs, size_t stride) const {
    ASSERT_PRECONDITION(mInput->uvs, "Must provide input UVs");
    stride = stride ? stride : sizeof(decltype(*uvs));
    for (size_t i = 0; i < mOutput->vertexCount; ++i) {
        *uvs = mOutput->uvs[i];
        takeStride(uvs, stride);
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
        *out = is16 ? uint3{mOutput->triangles16[i]} : mOutput->triangles32[i];
        takeStride(out, stride);
    }
}

void TangentSpaceMesh::getTriangles(ushort3* out) const {
    ASSERT_PRECONDITION(mInput->triangles16 || mInput->triangles32, "Must provide input triangles");

    const bool is16 = mOutput->triangles16 != nullptr;
    const size_t stride = sizeof(decltype(*out));
    for (size_t i = 0, c = mOutput->triangleCount; i < c; ++i) {
        if (is16) {
            *out = mOutput->triangles16[i];
        } else {
            const uint3 &tri = mOutput->triangles32[i];
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
    const quatf* tangentSpace = mOutput->tangentSpace;
    const size_t vertexCount = mOutput->vertexCount;
    for (size_t i = 0; i < vertexCount; ++i) {
        *out = tangentSpace[i];
        out = pointerAdd(out, 1, stride);
    }
}

void TangentSpaceMesh::getQuats(short4* out, size_t stride) const noexcept {
    stride = stride ? stride : sizeof(decltype((*out)));
    const quatf* tangentSpace = mOutput->tangentSpace;
    const size_t vertexCount = mOutput->vertexCount;
    for (size_t i = 0; i < vertexCount; ++i) {
        *out = packSnorm16(tangentSpace[i].xyzw);
        out = pointerAdd(out, 1, stride);
    }
}

void TangentSpaceMesh::getQuats(quath* out, size_t stride) const noexcept {
    stride = stride ? stride : sizeof(decltype((*out)));
    const quatf* tangentSpace = mOutput->tangentSpace;
    const size_t vertexCount = mOutput->vertexCount;
    for (size_t i = 0; i < vertexCount; ++i) {
        *out = quath(tangentSpace[i].xyzw);
        out = pointerAdd(out, 1, stride);
    }
}

Algorithm TangentSpaceMesh::getAlgorithm() const noexcept {
    return mOutput->algorithm;
}

}
}
