/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "TangentSpaceMeshWrapper.h"

#include <utils/Panic.h>

#include <memory>
#include <unordered_map>
#include <cstring>

namespace filament::gltfio {

namespace {

using AuxType = TangentSpaceMeshWrapper::AuxType;
using Builder = TangentSpaceMeshWrapper::Builder;

template<typename T>
using is_supported_aux_t = TangentSpaceMeshWrapper::is_supported_aux_t<T>;

struct Passthrough {
    static constexpr int POSITION = 256;
    static constexpr int UV0 = 257;
    static constexpr int NORMALS = 258;
    static constexpr int TANGENTS = 259;
    static constexpr int TRIANGLES = 260;

    std::unordered_map<int, void const*> data;
    size_t vertexCount = 0;
    size_t triangleCount = 0;
};

// Note that the method signatures here match TangentSpaceMesh
struct PassthroughMesh {
    explicit PassthroughMesh(Passthrough const& passthrough)
        : mPassthrough(passthrough) {}

    void getPositions(float3* data) noexcept {
        size_t const nbytes = mPassthrough.vertexCount * sizeof(float3);
        std::memcpy(data, mPassthrough.data[Passthrough::POSITION], nbytes);
    }

    void getUVs(float2* data) noexcept {
        size_t const nbytes = mPassthrough.vertexCount * sizeof(float2);
        std::memcpy(data, mPassthrough.data[Passthrough::UV0], nbytes);
    }

    void getQuats(short4* data) noexcept {
        size_t const nbytes = mPassthrough.vertexCount * sizeof(short4);
        std::memcpy(data, mPassthrough.data[Passthrough::TANGENTS], nbytes);
    }

    void getTriangles(uint3* data) {
        size_t const nbytes = mPassthrough.triangleCount * sizeof(uint3);
        std::memcpy(data, mPassthrough.data[Passthrough::TRIANGLES], nbytes);
    }

    template<typename T>
    void getAux(AuxType attribute, T data) noexcept {
        size_t const nbytes = mPassthrough.vertexCount * sizeof(std::remove_pointer_t<T>);
        std::memcpy(data, (T) mPassthrough.data[static_cast<int>(attribute)], nbytes);
    }

    size_t getVertexCount() const noexcept { return mPassthrough.vertexCount; }
    size_t getTriangleCount() const noexcept { return mPassthrough.triangleCount; }

private:
    Passthrough mPassthrough;
};

// Note that the method signatures here match TangentSpaceMesh::Builder
struct PassthroughBuilder {
    void vertexCount(size_t count) noexcept { mPassthrough.vertexCount = count; }

    void normals(float3 const* normals) noexcept {
        mPassthrough.data[Passthrough::NORMALS] = normals;
    }

    void tangents(float4 const* tangents) noexcept {
        mPassthrough.data[Passthrough::TANGENTS] = tangents;
    }
    void uvs(float2 const* uvs) noexcept { mPassthrough.data[Passthrough::UV0] = uvs; }

    void positions(float3 const* positions) noexcept {
        mPassthrough.data[Passthrough::POSITION] = positions;
    }

    void triangleCount(size_t triangleCount) noexcept {
        mPassthrough.triangleCount = triangleCount;
    }

    void triangles(uint3 const* triangles) noexcept {
        mPassthrough.data[Passthrough::TRIANGLES] = triangles;
    }

    void aux(AuxType type, void* data) noexcept {
        mPassthrough.data[static_cast<int>(type)] = data;
    }

    PassthroughMesh* build() const noexcept {
        return new PassthroughMesh(mPassthrough);
    }

private:
    Passthrough mPassthrough;
    friend struct PassthroughMesh;
};

} // anonymous

#define DO_MESH_IMPL(METHOD, ...)                                                                  \
    do {                                                                                           \
        if (mTsMesh) {                                                                             \
            mTsMesh->METHOD(__VA_ARGS__);                                                          \
        } else {                                                                                   \
            mPassthroughMesh->METHOD(__VA_ARGS__);                                                 \
        }                                                                                          \
    } while (0)

#define DO_MESH_RET_IMPL(METHOD)                                                                   \
    do {                                                                                           \
        if (mTsMesh) {                                                                             \
            return mTsMesh->METHOD();                                                              \
        } else {                                                                                   \
            return mPassthroughMesh->METHOD();                                                     \
        }                                                                                          \
    } while (0)

struct TangentSpaceMeshWrapper::Impl {
    Impl(geometry::TangentSpaceMesh* tsMesh)
        : mTsMesh(tsMesh) {}

    Impl(PassthroughMesh* passthroughMesh)
        : mPassthroughMesh(passthroughMesh) {}

    ~Impl() {
        if (mTsMesh) {
            geometry::TangentSpaceMesh::destroy(mTsMesh);
        }
        if (mPassthroughMesh) {
            delete mPassthroughMesh;
        }
    }

    inline size_t getVertexCount() const noexcept {
        DO_MESH_RET_IMPL(getVertexCount);
    }

    float3* getPositions() noexcept {
        size_t const nbytes = getVertexCount() * sizeof(float3);
        auto data = (float3*) malloc(nbytes);
        DO_MESH_IMPL(getPositions, data);
        return data;
    }

    float2* getUVs() noexcept {
        size_t const nbytes = getVertexCount() * sizeof(float2);
        auto data = (float2*) malloc(nbytes);
        DO_MESH_IMPL(getUVs, data);
        return data;
    }

    short4* getQuats() noexcept {
        size_t const nbytes = getVertexCount() * sizeof(short4);
        auto data = (short4*) malloc(nbytes);
        DO_MESH_IMPL(getQuats, data);
        return data;
    }

    uint3* getTriangles() {
        size_t const nbytes = getTriangleCount() * sizeof(uint3);
        auto data = (uint3*) malloc(nbytes);
        DO_MESH_IMPL(getTriangles, data);
        return data;
    }

    template<typename T, typename = is_supported_aux_t<T>>
    T getAux(AuxType attribute) noexcept {
        size_t const nbytes = getVertexCount() * sizeof(std::remove_pointer_t<T>);
        auto data = (T) malloc(nbytes);
        DO_MESH_IMPL(getAux, attribute, data);
        return data;
    }

    inline size_t getTriangleCount() const noexcept {
        DO_MESH_RET_IMPL(getTriangleCount);
    }

private:
    geometry::TangentSpaceMesh* mTsMesh = nullptr;
    PassthroughMesh* mPassthroughMesh = nullptr;
};

#undef DO_MESH_IMPL
#undef DO_MESH_RET_IMPL

#define DO_BUILDER_IMPL(METHOD, ...)                                                               \
    do {                                                                                           \
        if (mPassthroughBuilder) {                                                                 \
            mPassthroughBuilder->METHOD(__VA_ARGS__);                                              \
        } else {                                                                                   \
            mTsmBuilder->METHOD(__VA_ARGS__);                                                      \
        }                                                                                          \
    } while (0)

struct TangentSpaceMeshWrapper::Builder::Impl {
    explicit Impl(bool isUnlit)
        : mPassthroughBuilder(isUnlit ? std::make_unique<PassthroughBuilder>() : nullptr),
          mTsmBuilder(
                  !isUnlit ? std::make_unique<geometry::TangentSpaceMesh::Builder>() : nullptr) {}

    void vertexCount(size_t count) noexcept { DO_BUILDER_IMPL(vertexCount, count); }
    void normals(float3 const* normals) noexcept { DO_BUILDER_IMPL(normals, normals); }
    void tangents(float4 const* tangents) noexcept { DO_BUILDER_IMPL(tangents, tangents); }
    void uvs(float2 const* uvs) noexcept { DO_BUILDER_IMPL(uvs, uvs); }
    void positions(float3 const* positions) noexcept { DO_BUILDER_IMPL(positions, positions); }
    void triangles(uint3 const* triangles) noexcept { DO_BUILDER_IMPL(triangles, triangles); }
    void triangleCount(size_t count) noexcept { DO_BUILDER_IMPL(triangleCount, count); }

    template<typename T, typename = is_supported_aux_t<T>>
    void aux(AuxType type, T data) {
        DO_BUILDER_IMPL(aux, type, data);
    }

    TangentSpaceMeshWrapper* build() {
        auto ret = new TangentSpaceMeshWrapper();
        if (mPassthroughBuilder) {
            ret->mImpl = new TangentSpaceMeshWrapper::Impl{ mPassthroughBuilder->build() };
        } else {
            ret->mImpl = new TangentSpaceMeshWrapper::Impl{ mTsmBuilder->build() };
        }
        return ret;
    }

private:
    std::unique_ptr<PassthroughBuilder> mPassthroughBuilder;
    std::unique_ptr<geometry::TangentSpaceMesh::Builder> mTsmBuilder;
};

#undef DO_BUILDER_IMPL

Builder::Builder(bool isUnlit)
    : mImpl(new Impl{isUnlit}) {}


Builder& Builder::vertexCount(size_t count) noexcept {
    mImpl->vertexCount(count);
    return *this;
}

Builder& Builder::normals(float3 const* normals) noexcept {
    mImpl->normals(normals);
    return *this;
}

Builder& Builder::tangents(float4 const* tangents) noexcept {
    mImpl->tangents(tangents);
    return *this;
}

Builder& Builder::uvs(float2 const* uvs) noexcept {
    mImpl->uvs(uvs);
    return *this;
}

Builder& Builder::positions(float3 const* positions) noexcept{
    mImpl->positions(positions);
    return *this;
}

Builder& Builder::triangleCount(size_t triangleCount) noexcept {
    mImpl->triangleCount(triangleCount);
    return *this;
}

Builder& Builder::triangles(uint3 const* triangles) noexcept {
    mImpl->triangles(triangles);
    return *this;
}

template Builder& Builder::aux<float2*>(AuxType attribute, float2* data);
template Builder& Builder::aux<float3*>(AuxType attribute, float3* data);
template Builder& Builder::aux<float4*>(AuxType attribute, float4* data);
template Builder& Builder::aux<ushort3*>(AuxType attribute, ushort3* data);
template Builder& Builder::aux<ushort4*>(AuxType attribute, ushort4* data);

template<typename T, typename>
Builder& Builder::aux(AuxType type, T data) {
    mImpl->aux<T>(type, data);
    return *this;
}

TangentSpaceMeshWrapper* Builder::build() {
    return mImpl->build();
}

void TangentSpaceMeshWrapper::destroy(TangentSpaceMeshWrapper* mesh) {
    assert_invariant(mesh->mImpl);
    assert_invariant(mesh);
    delete mesh->mImpl;
    delete mesh;
}

float3* TangentSpaceMeshWrapper::getPositions() noexcept { return mImpl->getPositions(); }
float2* TangentSpaceMeshWrapper::getUVs() noexcept { return mImpl->getUVs(); }
short4* TangentSpaceMeshWrapper::getQuats() noexcept { return mImpl->getQuats(); }
uint3* TangentSpaceMeshWrapper::getTriangles() { return mImpl->getTriangles(); }
size_t TangentSpaceMeshWrapper::getVertexCount() const noexcept { return mImpl->getVertexCount(); }

template float2* TangentSpaceMeshWrapper::getAux<float2*>(AuxType attribute) noexcept;
template float3* TangentSpaceMeshWrapper::getAux<float3*>(AuxType attribute) noexcept;
template float4* TangentSpaceMeshWrapper::getAux<float4*>(AuxType attribute) noexcept;
template ushort3* TangentSpaceMeshWrapper::getAux<ushort3*>(AuxType attribute) noexcept;
template ushort4* TangentSpaceMeshWrapper::getAux<ushort4*>(AuxType attribute) noexcept;

template<typename T, typename>
T TangentSpaceMeshWrapper::getAux(AuxType attribute) noexcept {
    return mImpl->getAux<T>(attribute);
}

size_t TangentSpaceMeshWrapper::getTriangleCount() const noexcept {
    return mImpl->getTriangleCount();
}

} // filament::gltfio
