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

#include <math/vec3.h>
#include <math/vec4.h>
#include <utils/Log.h>
#include <utils/Panic.h>

namespace filament {
namespace geometry {

using namespace filament::math;
using Builder = TangentSpaceMesh::Builder;
using Algorithm = TangentSpaceMesh::Algorithm;

struct TangentSpaceMeshInput {
    size_t vertexCount = 0;
    const float3* normals = nullptr;
    const float4* tangents = nullptr;
    const float2* uvs = nullptr;
    const float3* positions = nullptr;
    const ushort3* triangles16 = nullptr;
    const uint3* triangles32 = nullptr;

    size_t normalStride = 0;
    size_t tangentStride = 0;
    size_t uvStride = 0;
    size_t positionStride = 0;
    size_t triangleCount = 0;

    Algorithm algorithm;
};

struct TangentSpaceMeshOutput {
    Algorithm algorithm;

    size_t triangleCount = 0;
    size_t vertexCount = 0;

    float4* tangents = nullptr;
    float2* uvs = nullptr;
    float3* positions = nullptr;
    uint3* triangles32 = nullptr;
    ushort3* triangles16 = nullptr;
};

namespace {

const uint8_t NORMALS_BIT = 0x01;
const uint8_t TANGENTS_BIT = 0x02;
const uint8_t UVS_BIT = 0x04;
const uint8_t POSITIONS_BIT = 0x08;
const uint8_t INDICES_BIT = 0x10;

// Input types
const uint8_t NORMALS = NORMALS_BIT;
const uint8_t NORMALS_TANGENTS = NORMALS_BIT | TANGENTS_BIT;
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
        case Algorithm::SIGN_OF_W:
            return "SIGN_OF_W";
        default:
            PANIC_POSTCONDITION("Unknown algorithm %u", static_cast<uint8_t>(algorithm));
    }
}

Algorithm selectBestDefaultAlgorithm(uint8_t inputType) {
    Algorithm outAlgo;
    if (inputType & NORMALS_UVS_POSITIONS_INDICES) {
        outAlgo = Algorithm::MIKKTSPACE;
    } else if (inputType & POSITIONS_INDICES) {
        outAlgo = Algorithm::FLAT_SHADING;
    } else if (inputType & NORMALS_TANGENTS) {
        outAlgo = Algorithm::SIGN_OF_W;
    } else {
        ASSERT_PRECONDITION(inputType & NORMALS,
                "Must at least have normals or (positions + indices) as input");
        outAlgo = Algorithm::FRISVAD;
    }
    return outAlgo;
}

#define IS_INPUT_TYPE(inputType, TYPE) ((inputType & TYPE) == TYPE)

Algorithm selectAlgorithm(TangentSpaceMeshInput *input) {
    uint8_t inputType = 0;
    if (input->normals) {
        inputType |= NORMALS_BIT;
    }
    if (input->tangents) {
        inputType |= TANGENTS_BIT;
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
        case Algorithm::SIGN_OF_W:
            if (IS_INPUT_TYPE(inputType, NORMALS_TANGENTS)) {
                outAlgo = Algorithm::SIGN_OF_W;
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

} // anonymous namespace

Builder::Builder() noexcept
        :mMesh(new TangentSpaceMesh()) {}

Builder::~Builder() noexcept { delete mMesh; }

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

Builder& Builder::tangents(const float4* tangents, size_t stride) noexcept {
    mMesh->mInput->tangents = tangents;
    mMesh->mInput->tangentStride = stride;
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
    mMesh->mOutput->algorithm = selectAlgorithm(mMesh->mInput);
    // "Working in progress. Not for use"
    return NULL;
}

TangentSpaceMesh::TangentSpaceMesh() noexcept
        :mInput(new TangentSpaceMeshInput()), mOutput(new TangentSpaceMeshOutput()) {
}

TangentSpaceMesh::~TangentSpaceMesh() noexcept {}

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
    return 0;
}

void TangentSpaceMesh::getPositions(float3*, size_t) const {
    ASSERT_PRECONDITION(mInput->positions, "Must provide input positions");
}

void TangentSpaceMesh::getUVs(float2*, size_t) const {
    ASSERT_PRECONDITION(mInput->uvs, "Must provide input UVs");
}

size_t TangentSpaceMesh::getTriangleCount() const {
    return 0;
}

void TangentSpaceMesh::getTriangles(uint3*) const {
    ASSERT_PRECONDITION(mInput->triangles16 || mInput->triangles32, "Must provide input triangles");
}

void TangentSpaceMesh::getTriangles(ushort3*) const {
    ASSERT_PRECONDITION(mInput->triangles16 || mInput->triangles32, "Must provide input triangles");
}

void TangentSpaceMesh::getQuats(quatf*, size_t) const noexcept {
}

void TangentSpaceMesh::getQuats(short4*, size_t) const noexcept {
}

void TangentSpaceMesh::getQuats(quath*, size_t) const noexcept {
}

Algorithm TangentSpaceMesh::getAlgorithm() const {
    return mOutput->algorithm;
}

}
}
