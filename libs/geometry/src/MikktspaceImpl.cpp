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

#include "MikktspaceImpl.h"

#include <math/mat3.h>
#include <math/norm.h>


#include <meshoptimizer.h>
#include <mikktspace/mikktspace.h>

#include <vector>

namespace filament::geometry {

using namespace filament::math;

int MikktspaceImpl::getNumFaces(SMikkTSpaceContext const* context) noexcept {
    auto const wrapper = MikktspaceImpl::getThis(context);
    return wrapper->mFaceCount;
}

int MikktspaceImpl::getNumVerticesOfFace(SMikkTSpaceContext const* context,
        int const iFace) noexcept {
    return 3;
}

void MikktspaceImpl::getPosition(SMikkTSpaceContext const* context, float fvPosOut[],
        int const iFace, int const iVert) noexcept {
    auto const wrapper = MikktspaceImpl::getThis(context);
    float3 const pos = *pointerAdd(wrapper->mPositions, wrapper->getTriangle(iFace)[iVert],
            wrapper->mPositionStride);
    fvPosOut[0] = pos.x;
    fvPosOut[1] = pos.y;
    fvPosOut[2] = pos.z;
}

void MikktspaceImpl::getNormal(SMikkTSpaceContext const* context, float fvNormOut[],
        int const iFace, int const iVert) noexcept {
    auto const wrapper = MikktspaceImpl::getThis(context);
    float3 const normal = *pointerAdd(wrapper->mNormals, wrapper->getTriangle(iFace)[iVert],
            wrapper->mNormalStride);
    fvNormOut[0] = normal.x;
    fvNormOut[1] = normal.y;
    fvNormOut[2] = normal.z;
}

void MikktspaceImpl::getTexCoord(SMikkTSpaceContext const* context, float fvTexcOut[],
        int const iFace, int const iVert) noexcept {
    auto const wrapper = MikktspaceImpl::getThis(context);
    float2 const texc
            = *pointerAdd(wrapper->mUVs, wrapper->getTriangle(iFace)[iVert], wrapper->mUVStride);
    fvTexcOut[0] = texc.x;
    fvTexcOut[1] = texc.y;
}

void MikktspaceImpl::setTSpaceBasic(SMikkTSpaceContext const* context, float const fvTangent[],
        float const fSign, int const iFace, int const iVert) noexcept {
    auto const wrapper = MikktspaceImpl::getThis(context);
    uint32_t const vertInd = wrapper->getTriangle(iFace)[iVert];
    float3 const pos = *pointerAdd(wrapper->mPositions, vertInd, wrapper->mPositionStride);
    float3 const n = normalize(*pointerAdd(wrapper->mNormals, vertInd, wrapper->mNormalStride));
    float2 const uv = *pointerAdd(wrapper->mUVs, vertInd, wrapper->mUVStride);
    float3 const t{fvTangent[0], fvTangent[1], fvTangent[2]};
    float3 const b = fSign * normalize(cross(n, t));

    // TODO: packTangentFrame actually changes the orientation of b.
    quatf const quat = mat3f::packTangentFrame({t, b, n}, sizeof(int32_t));

    wrapper->mOutVertices.push_back({pos, uv, quat});
}

MikktspaceImpl::MikktspaceImpl(const TangentSpaceMeshInput* input) noexcept
    : mFaceCount((int) input->triangleCount),
      mPositions(input->positions),
      mPositionStride(input->positionStride ? input->positionStride : sizeof(float3)),
      mNormals(input->normals),
      mNormalStride(input->normalStride ? input->normalStride : sizeof(float3)),
      mUVs(input->uvs),
      mUVStride(input->uvStride ? input->uvStride : sizeof(float2)),
      mIsTriangle16(input->triangles16),
      mTriangles(
              input->triangles16 ? (uint8_t*) input->triangles16 : (uint8_t*) input->triangles32) {
    mOutVertices.reserve(mFaceCount * 3);
}

MikktspaceImpl* MikktspaceImpl::getThis(SMikkTSpaceContext const* context) noexcept {
    return (MikktspaceImpl*) context->m_pUserData;
}

inline const uint3 MikktspaceImpl::getTriangle(int const triangleIndex) const noexcept {
    const size_t tstride = mIsTriangle16 ? sizeof(ushort3) : sizeof(uint3);
    return mIsTriangle16 ? uint3(*(ushort3*) (pointerAdd(mTriangles, triangleIndex, tstride)))
                         : *(uint3*) (pointerAdd(mTriangles, triangleIndex, tstride));
}

void MikktspaceImpl::run(TangentSpaceMeshOutput* output) noexcept {
    SMikkTSpaceInterface interface {
        .m_getNumFaces = MikktspaceImpl::getNumFaces,
        .m_getNumVerticesOfFace = MikktspaceImpl::getNumVerticesOfFace,
        .m_getPosition = MikktspaceImpl::getPosition,
        .m_getNormal = MikktspaceImpl::getNormal,
        .m_getTexCoord = MikktspaceImpl::getTexCoord,
        .m_setTSpaceBasic = MikktspaceImpl::setTSpaceBasic,
    };
    SMikkTSpaceContext context{.m_pInterface = &interface, .m_pUserData = this};
    genTangSpaceDefault(&context);

    std::vector<unsigned int> remap(mOutVertices.size());
    size_t vertexCount = meshopt_generateVertexRemap(remap.data(), NULL, mOutVertices.size(),
            mOutVertices.data(), mOutVertices.size(), sizeof(IOVertex));

    std::vector<IOVertex> newVertices(vertexCount);
    meshopt_remapVertexBuffer((void*) newVertices.data(), mOutVertices.data(), mOutVertices.size(),
            sizeof(IOVertex), remap.data());

    uint3* triangles32 = output->triangles32.allocate(mFaceCount);
    meshopt_remapIndexBuffer((uint32_t*) triangles32, NULL, mOutVertices.size(), remap.data());

    float3* outPositions = output->positions.allocate(vertexCount);
    float2* outUVs = output->uvs.allocate(vertexCount);
    quatf* outQuats = output->tangentSpace.allocate(vertexCount);

    for (size_t i = 0; i < vertexCount; ++i) {
        outPositions[i] = newVertices[i].position;
        outUVs[i] = newVertices[i].uv;
        outQuats[i] = newVertices[i].tangentSpace;
    }

    output->vertexCount = vertexCount;
    output->triangleCount = mFaceCount;
}

}// namespace filament::geometry
