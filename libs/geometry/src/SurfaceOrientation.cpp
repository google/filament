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
#include <utils/debug.h>

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
    SurfaceOrientation* buildWithNormalsOnly();
    SurfaceOrientation* buildWithSuppliedTangents();
    SurfaceOrientation* buildWithUvs();
    SurfaceOrientation* buildWithFlatNormals();
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

SurfaceOrientation* Builder::build() {
    if (!ASSERT_PRECONDITION_NON_FATAL(mImpl->vertexCount > 0, "Vertex count must be non-zero.")) {
        return nullptr;
    }
    if (mImpl->triangles16 || mImpl->triangles32) {
        if (!ASSERT_PRECONDITION_NON_FATAL(mImpl->positions, "Positions are required.")) {
            return nullptr;
        }
        if (!ASSERT_PRECONDITION_NON_FATAL(!mImpl->triangles16 || !mImpl->triangles32,
                "Choose 16 or 32-bit indices, not both.")) {
            return nullptr;
        }
        if (!ASSERT_PRECONDITION_NON_FATAL(mImpl->triangleCount > 0, "Triangle count is required.")) {
            return nullptr;
        }
        if (mImpl->normals == nullptr) {
            return mImpl->buildWithFlatNormals();
        }
    }
    if (!ASSERT_PRECONDITION_NON_FATAL(mImpl->normals != nullptr, "Normals are required.")) {
        return nullptr;
    }
    if (mImpl->tangents != nullptr) {
        return mImpl->buildWithSuppliedTangents();
    }
    if (mImpl->uvs == nullptr) {
        return mImpl->buildWithNormalsOnly();
    }
    return mImpl->buildWithUvs();
}

static float3 randomPerp(const float3& n) {
    float3 perp = cross(n, float3{1, 0, 0});
    float sqrlen = dot(perp, perp);
    if (sqrlen <= std::numeric_limits<float>::epsilon()) {
        perp = cross(n, float3{0, 1, 0});
        sqrlen = dot(perp, perp);
    }
    return perp / sqrlen;
}

// Computes tangent space basis, from input normal vector, based on:
// Frisvad, Jeppe Revall: Building an Orthonormal Basis from a 3D Unit Vector Without Normalization
// Paper: https://backend.orbit.dtu.dk/ws/portalfiles/portal/126824972/onb_frisvad_jgt2012_v2.pdf
//
// The paper uses a Z-up world basis, which has been converted to Y-up here
static void frisvadTangentSpace(const float3& N, float3& T, float3& B) {
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

SurfaceOrientation* OrientationBuilderImpl::buildWithNormalsOnly() {
    vector<quatf> quats(vertexCount);

    const float3* normal = this->normals;
    size_t nstride = this->normalStride ? this->normalStride : sizeof(float3);

    for (size_t qindex = 0; qindex < vertexCount; ++qindex) {
        float3 n = *normal;
        float3 b, t;
        frisvadTangentSpace(n, t, b);
        quats[qindex] = mat3f::packTangentFrame({t, b, n});
        normal = (const float3*) (((const uint8_t*) normal) + nstride);
    }

    return new SurfaceOrientation(new OrientationImpl( { std::move(quats) } ));
}

SurfaceOrientation* OrientationBuilderImpl::buildWithSuppliedTangents() {
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

        // Some assets do not provide perfectly orthogonal tangents and normals, so we adjust the
        // tangent to enforce orthonormality. We would rather honor the exact normal vector than
        // the exact tangent vector since the latter is only used for bump mapping and anisotropic
        // lighting.
        t = *tandir > 0 ? cross(n, b) : cross(b, n);

        quats[qindex] = mat3f::packTangentFrame({t, b, n});
        normal = (const float3*) (((const uint8_t*) normal) + nstride);
        tanvec = (const float3*) (((const uint8_t*) tanvec) + tstride);
        tandir = (const float*) (((const uint8_t*) tandir) + tstride);
    }

    return new SurfaceOrientation(new OrientationImpl( { std::move(quats) } ));
}

// This method is based on:
//
// Computing Tangent Space Basis Vectors for an Arbitrary Mesh (Lengyel’s Method)
// http://www.terathon.com/code/tangent.html
//
// We considered mikktspace (which thankfully has a zlib-style license) but it would require
// re-indexing (i.e. welding) and is therefore a bit heavyweight. Note that the welding could be
// done via meshoptimizer.
//
SurfaceOrientation* OrientationBuilderImpl::buildWithUvs() {
    if (!ASSERT_PRECONDITION_NON_FATAL(this->normalStride == 0, "Non-zero normal stride not yet supported.")) {
        return nullptr;
    }
    if (!ASSERT_PRECONDITION_NON_FATAL(this->tangentStride == 0, "Non-zero tangent stride not yet supported.")) {
        return nullptr;
    }
    if (!ASSERT_PRECONDITION_NON_FATAL(this->uvStride == 0, "Non-zero uv stride not yet supported.")) {
        return nullptr;
    }
    if (!ASSERT_PRECONDITION_NON_FATAL(this->positionStride == 0, "Non-zero positions stride not yet supported.")) {
        return nullptr;
    }
    vector<float3> tan1(vertexCount);
    vector<float3> tan2(vertexCount);
    memset(tan1.data(), 0, sizeof(float3) * vertexCount);
    memset(tan2.data(), 0, sizeof(float3) * vertexCount);
    for (size_t a = 0; a < triangleCount; ++a) {
        uint3 tri = triangles16 ? uint3(triangles16[a]) : triangles32[a];
        assert_invariant(tri.x < vertexCount && tri.y < vertexCount && tri.z < vertexCount);
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
        float d = s1 * t2 - s2 * t1;
        float3 sdir, tdir;
        // In general we can't guarantee smooth tangents when the UV's are non-smooth, but let's at
        // least avoid divide-by-zero and fall back to normals-only method.
        if (d == 0.0) {
            const float3& n1 = normals[tri.x];
            sdir = randomPerp(n1);
            tdir = cross(n1, sdir);
        } else {
            sdir = {t2 * x1 - t1 * x2, t2 * y1 - t1 * y2, t2 * z1 - t1 * z2};
            tdir = {s1 * x2 - s2 * x1, s1 * y2 - s2 * y1, s1 * z2 - s2 * z1};
            float r = 1.0f / d;
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
    return new SurfaceOrientation(new OrientationImpl( { std::move(quats) } ));
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

SurfaceOrientation* OrientationBuilderImpl::buildWithFlatNormals() {
    float3* normals = new float3[vertexCount];
    for (size_t a = 0; a < triangleCount; ++a) {
        const uint3 tri = triangles16 ? uint3(triangles16[a]) : triangles32[a];
        assert_invariant(tri.x < vertexCount && tri.y < vertexCount && tri.z < vertexCount);
        const float3 v1 = positions[tri.x];
        const float3 v2 = positions[tri.y];
        const float3 v3 = positions[tri.z];
        const float3 normal = normalize(cross(v2 - v1, v3 - v1));
        normals[tri.x] = normal;
        normals[tri.y] = normal;
        normals[tri.z] = normal;
    }
    this->normals = normals;
    SurfaceOrientation* result = buildWithNormalsOnly();
    this->normals = nullptr;
    delete[] normals;
    return result;
}

} // namespace geometry
} // namespace filament
