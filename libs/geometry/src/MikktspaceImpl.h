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

#ifndef TNT_GEOMETRY_MIKKTSPACEIMPL_H
#define TNT_GEOMETRY_MIKKTSPACEIMPL_H

#include "TangentSpaceMeshInternal.h"

#include <math/quat.h>
#include <math/vec2.h>
#include <math/vec3.h>

#include <vector>

struct SMikkTSpaceContext;

namespace filament::geometry {

using namespace filament::math;

class MikktspaceImpl {
public:
    struct IOVertex {
        float3 position;
        float2 uv;
        quatf tangentSpace;
    };

    MikktspaceImpl(TangentSpaceMeshInput const* input) noexcept;

    MikktspaceImpl(MikktspaceImpl const&) = delete;
    MikktspaceImpl& operator=(MikktspaceImpl const&) = delete;

    void run(TangentSpaceMeshOutput* output) noexcept;

private:
    // sizeof(float3 + float2 + quatf) (pos, uv, tangent)
    static constexpr size_t const BASE_OUTPUT_SIZE = 36;

    static int getNumFaces(SMikkTSpaceContext const* context) noexcept;
    static int getNumVerticesOfFace(SMikkTSpaceContext const* context, int const iFace) noexcept;
    static void getPosition(SMikkTSpaceContext const* context, float fvPosOut[], int const iFace,
            int const iVert) noexcept;
    static void getNormal(SMikkTSpaceContext const* context, float fvNormOut[], int const iFace,
            int const iVert) noexcept;
    static void getTexCoord(SMikkTSpaceContext const* context, float fvTexcOut[], int const iFace,
            int const iVert) noexcept;
    static void setTSpaceBasic(SMikkTSpaceContext const* context, float const fvTangent[],
            float const fSign, int const iFace, int const iVert) noexcept;

    static MikktspaceImpl* getThis(SMikkTSpaceContext const* context) noexcept;

    inline const uint3 getTriangle(int const triangleIndex) const noexcept;

    int const mFaceCount;
    float3 const* mPositions;
    size_t const mPositionStride;
    float3 const* mNormals;
    size_t const mNormalStride;
    float2 const* mUVs;
    size_t const mUVStride;
    uint8_t const* mTriangles;
    bool mIsTriangle16;
    std::vector<std::tuple<uint8_t const*, size_t, size_t>> mInputAttribArrays;

    size_t mOutputElementSize;
    std::vector<uint8_t> mOutputData;
    std::vector<uint8_t> EMPTY_ELEMENT;
};

}// namespace filament::geometry

#endif//TNT_GEOMETRY_MIKKTSPACEIMPL_H
