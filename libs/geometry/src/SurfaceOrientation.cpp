/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <geometry/SurfaceOrientation.h>

#include <utils/Panic.h>

#include <math/mat3.h>
#include <math/norm.h>

#include <vector>

namespace filament {
namespace geometry {

using namespace filament::math;
using std::vector;
using Builder = SurfaceOrientation::Builder;

struct OrientationBuilderImpl {
    size_t vertexCount = 0;
    const float3* normals = nullptr;
    const float4* tangents = nullptr;
    const float2* uvs = nullptr;
    const float3* positions = nullptr;
    const uint3* triangles32 = nullptr;
    const ushort3* triangles16 = nullptr;
    size_t normalStride = 0;
    size_t tangentStride = 0;
    size_t uvStride = 0;
    size_t positionStride = 0;
    size_t triangleCount = 0;
    SurfaceOrientation buildWithNormalsOnly();
    SurfaceOrientation buildWithSuppliedTangents();
    SurfaceOrientation buildWithUvs();
};

struct OrientationImpl {
    vector<quatf> quaternions;
};

Builder::Builder() noexcept : mImpl(new OrientationBuilderImpl) {}

Builder::~Builder() noexcept { delete mImpl; }

Builder::Builder(Builder&& that) noexcept {
    std::swap(mImpl, that.mImpl);
}

Builder& Builder::operator=(Builder&& that) noexcept {
    std::swap(mImpl, that.mImpl);
    return *this;
}

Builder& Builder::vertexCount(size_t vertexCount) noexcept {
    mImpl->vertexCount = vertexCount;
    return *this;
}

Builder& Builder::normals(const float3* normals, size_t stride) noexcept {
    mImpl->normals = normals;
    mImpl->normalStride = stride;
    return *this;
}

Builder& Builder::tangents(const float4* tangents, size_t stride) noexcept {
    mImpl->tangents = tangents;
    mImpl->tangentStride = stride;
    return *this;
}

Builder& Builder::uvs(const float2* uvs, size_t stride) noexcept {
    mImpl->uvs = uvs;
    mImpl->uvStride = stride;
    return *this;
}

Builder& Builder::positions(const float3* positions, size_t stride) noexcept {
    mImpl->positions = positions;
    mImpl->positionStride = stride;
    return *this;
}

Builder& Builder::triangleCount(size_t triangleCount) noexcept {
    mImpl->triangleCount = triangleCount;
    return *this;
}

Builder& Builder::triangles(const uint3* triangles) noexcept {
    mImpl->triangles32 = triangles;
    return *this;
}

Builder& Builder::triangles(const ushort3* triangles) noexcept {
    mImpl->triangles16 = triangles;
    return *this;
}

SurfaceOrientation Builder::build() {
    ASSERT_PRECONDITION(mImpl->normals != nullptr, "Normals are required.");
    ASSERT_PRECONDITION(mImpl->vertexCount > 0, "Vertex count must be non-zero.");
    if (mImpl->tangents != nullptr) {
        return mImpl->buildWithSuppliedTangents();
    }
    if (mImpl->uvs == nullptr) {
        return mImpl->buildWithNormalsOnly();
    }
    bool hasTriangles = mImpl->triangles16 || mImpl->triangles32;
    bool bothTypes = mImpl->triangles16 && mImpl->triangles32;
    ASSERT_PRECONDITION(hasTriangles && mImpl->positions,
            "When using UVs, positions and triangles are required.");
    ASSERT_PRECONDITION(!bothTypes, "Choose 16 or 32-bit indices, not both.");
    ASSERT_PRECONDITION(mImpl->triangleCount > 0,
            "When using UVs, triangle count is required.");
    return mImpl->buildWithUvs();
}

SurfaceOrientation OrientationBuilderImpl::buildWithNormalsOnly() {
    vector<quatf> quats(vertexCount);

    const float3* normal = this->normals;
    size_t nstride = this->normalStride ? this->normalStride : sizeof(float3);

    for (size_t qindex = 0; qindex < vertexCount; ++qindex) {
        float3 n = *normal;
        float3 perp = cross(n, float3{1, 0, 0});
        float sqrlen = dot(perp, perp);
        if (sqrlen <= std::numeric_limits<float>::epsilon()) {
            perp = cross(n, float3{0, 1, 0});
            sqrlen = dot(perp, perp);
        }
        float3 b = perp / sqrlen;
        float3 t = cross(n, b);
        quats[qindex] = mat3f::packTangentFrame({t, b, n});
        normal = (const float3*) (((const uint8_t*) normal) + nstride);
    }

    return SurfaceOrientation(new OrientationImpl( { std::move(quats) } ));
}

SurfaceOrientation OrientationBuilderImpl::buildWithSuppliedTangents() {
    vector<quatf> quats(vertexCount);

    const float3* normal = this->normals;
    size_t nstride = this->normalStride ? this->normalStride : sizeof(float3);

    const float3* tanvec = (const float3*) this->tangents;
    const float* tandir = &this->tangents->w;
    size_t tstride = this->tangentStride ? this->tangentStride : sizeof(float4);

    for (size_t qindex = 0; qindex < vertexCount; ++qindex) {
        float3 n = *normal;
        float3 t = *tanvec;
        float3 b = *tandir > 0 ? cross(t, n) : cross(n, t);
        quats[qindex] = mat3f::packTangentFrame({t, b, n});
        normal = (const float3*) (((const uint8_t*) normal) + nstride);
        tanvec = (const float3*) (((const uint8_t*) tanvec) + tstride);
        tandir = (const float*) (((const uint8_t*) tandir) + tstride);
    }

    return SurfaceOrientation(new OrientationImpl( { std::move(quats) } ));
}

// This method is based on:
//
// Computing Tangent Space Basis Vectors for an Arbitrary Mesh (Lengyel’s Method)
// http://www.terathon.com/code/tangent.html
//
// We considered mikktspace (which thankfully has a zlib-style license) but it would require
// re-indexing via meshoptimizer and is therefore a bit heavyweight.
//
SurfaceOrientation OrientationBuilderImpl::buildWithUvs() {
    ASSERT_PRECONDITION(this->normalStride == 0, "Non-zero normal stride not yet supported.");
    ASSERT_PRECONDITION(this->tangentStride == 0, "Non-zero tangent stride not yet supported.");
    ASSERT_PRECONDITION(this->uvStride == 0, "Non-zero uv stride not yet supported.");
    ASSERT_PRECONDITION(this->positionStride == 0, "Non-zero positions stride not yet supported.");
    vector<float3> tan1(vertexCount);
    vector<float3> tan2(vertexCount);
    memset(tan1.data(), 0, sizeof(float3) * vertexCount);
    memset(tan2.data(), 0, sizeof(float3) * vertexCount);
    for (size_t a = 0; a < triangleCount; ++a) {
        uint3 tri = triangles16 ? uint3(triangles16[a]) : triangles32[a];
        const float3& v1 = positions[tri.x];
        const float3& v2 = positions[tri.y];
        const float3& v3 = positions[tri.z];
        const float2& w1 = uvs[tri.x];
        const float2& w2 = uvs[tri.y];
        const float2& w3 = uvs[tri.z];
        float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;
        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;
        float r = 1.0F / (s1 * t2 - s2 * t1);
        float3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                (t2 * z1 - t1 * z2) * r);
        float3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                (s1 * z2 - s2 * z1) * r);
        tan1[tri.x] += sdir;
        tan1[tri.y] += sdir;
        tan1[tri.z] += sdir;
        tan2[tri.x] += tdir;
        tan2[tri.y] += tdir;
        tan2[tri.z] += tdir;
    }

    vector<quatf> quats(vertexCount);
    for (size_t a = 0; a < vertexCount; a++) {
        const float3& n = normals[a];
        const float3& t1 = tan1[a];
        const float3& t2 = tan2[a];

        // Gram-Schmidt orthogonalize
        float3 t = normalize(t1 - n * dot(n, t1));

        // Calculate handedness
        float w = (dot(cross(n, t1), t2) < 0.0f) ? -1.0f : 1.0f;

        float3 b = w < 0 ? cross(t, n) : cross(n, t);
        quats[a] = mat3f::packTangentFrame({t, b, n});
    }
    return SurfaceOrientation(new OrientationImpl( { std::move(quats) } ));
}

SurfaceOrientation::SurfaceOrientation(OrientationImpl* impl) noexcept : mImpl(impl) {}

SurfaceOrientation::~SurfaceOrientation() noexcept { delete mImpl; }

SurfaceOrientation::SurfaceOrientation(SurfaceOrientation&& that) noexcept {
    std::swap(mImpl, that.mImpl);
}

SurfaceOrientation& SurfaceOrientation::operator=(SurfaceOrientation&& that) noexcept {
    std::swap(mImpl, that.mImpl);
    return *this;
}

size_t SurfaceOrientation::getVertexCount() const noexcept {
    return mImpl->quaternions.size();
}

void SurfaceOrientation::getQuats(quatf* out, size_t quatCount, size_t stride) const noexcept {
    const vector<quatf>& in = mImpl->quaternions;
    quatCount = std::min(quatCount, in.size());
    stride = stride ? stride : sizeof(decltype(*out));
    for (size_t i = 0; i < quatCount; ++i) {
        *out = in[i];
        out = (decltype(out)) (((uint8_t*) out) + stride);
    }
}

void SurfaceOrientation::getQuats(short4* out, size_t quatCount, size_t stride) const noexcept {
    const vector<quatf>& in = mImpl->quaternions;
    quatCount = std::min(quatCount, in.size());
    stride = stride ? stride : sizeof(decltype(*out));
    for (size_t i = 0; i < quatCount; ++i) {
        *out = packSnorm16(in[i].xyzw);
        out = (decltype(out)) (((uint8_t*) out) + stride);
    }
}

void SurfaceOrientation::getQuats(quath* out, size_t quatCount, size_t stride) const noexcept {
    const vector<quatf>& in = mImpl->quaternions;
    quatCount = std::min(quatCount, in.size());
    stride = stride ? stride : sizeof(decltype(*out));
    for (size_t i = 0; i < quatCount; ++i) {
        *out = quath(in[i]);
        out = (decltype(out)) (((uint8_t*) out) + stride);
    }
}

} // namespace geometry
} // namespace filament
