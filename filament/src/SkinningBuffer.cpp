/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "details/SkinningBuffer.h"

#include "components/RenderableManager.h"

#include "details/Engine.h"

#include "FilamentAPI-impl.h"

#include <math/mat4.h>

namespace filament {

using namespace backend;
using namespace math;

struct SkinningBuffer::BuilderDetails {
    uint32_t mBoneCount = 0;
    bool mInitialize = false;
};

using BuilderType = SkinningBuffer;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;


SkinningBuffer::Builder& SkinningBuffer::Builder::boneCount(uint32_t boneCount) noexcept {
    mImpl->mBoneCount = boneCount;
    return *this;
}

SkinningBuffer::Builder& SkinningBuffer::Builder::initialize(bool initialize) noexcept {
    mImpl->mInitialize = initialize;
    return *this;
}

SkinningBuffer* SkinningBuffer::Builder::build(Engine& engine) {
    return upcast(engine).createSkinningBuffer(*this);
}

// ------------------------------------------------------------------------------------------------

FSkinningBuffer::FSkinningBuffer(FEngine& engine, const Builder& builder)
        : mBoneCount(builder->mBoneCount) {
    FEngine::DriverApi& driver = engine.getDriverApi();

    // According to the OpenGL ES 3.2 specification in 7.6.3 Uniform
    // Buffer Object Bindings:
    //
    //     the uniform block must be populated with a buffer object with a size no smaller
    //     than the minimum required size of the uniform block (the value of
    //     UNIFORM_BLOCK_DATA_SIZE).

    mHandle = driver.createBufferObject(
            getPhysicalBoneCount(mBoneCount) * sizeof(PerRenderableUibBone),
            BufferObjectBinding::UNIFORM,
            BufferUsage::DYNAMIC,
            false);

    if (builder->mInitialize) {
        // initialize the bones to identity (before rounding up)
        size_t size = mBoneCount * sizeof(PerRenderableUibBone);
        auto* out = (PerRenderableUibBone*)driver.allocate(size);
        std::uninitialized_fill_n(out, mBoneCount, PerRenderableUibBone{});
        driver.updateBufferObject(mHandle, { out, size }, 0);
    }
}

void FSkinningBuffer::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyBufferObject(mHandle);
}

void FSkinningBuffer::setBones(FEngine& engine,
        RenderableManager::Bone const* transforms, size_t count, size_t offset) {

    ASSERT_PRECONDITION((offset + count) <= mBoneCount,
            "SkinningBuffer (size=%lu) overflow (boneCount=%u, offset=%u)",
            (unsigned)mBoneCount, (unsigned)count, (unsigned)offset);

    setBones(engine, mHandle, transforms, count, offset);
}

void FSkinningBuffer::setBones(FEngine& engine,
        math::mat4f const* transforms, size_t count, size_t offset) {

    ASSERT_PRECONDITION((offset + count) <= mBoneCount,
            "SkinningBuffer (size=%lu) overflow (boneCount=%u, offset=%u)",
            (unsigned)mBoneCount, (unsigned)count, (unsigned)offset);

    setBones(engine, mHandle, transforms, count, offset);
}

void FSkinningBuffer::setBones(FEngine& engine, Handle<backend::HwBufferObject> handle,
        RenderableManager::Bone const* transforms, size_t boneCount, size_t offset) noexcept {
    auto& driverApi = engine.getDriverApi();
    size_t size = boneCount * sizeof(PerRenderableUibBone);
    PerRenderableUibBone* UTILS_RESTRICT out = (PerRenderableUibBone*)driverApi.allocate(size);
    for (size_t i = 0, c = boneCount; i < c; ++i) {
        out[i].q = transforms[i].unitQuaternion;
        out[i].t.xyz = transforms[i].translation;
        out[i].s = out[i].ns = { 1, 1, 1, 0 };
    }
    driverApi.updateBufferObject(handle, { out, size },
            offset * sizeof(PerRenderableUibBone));
}

void FSkinningBuffer::setBones(FEngine& engine, Handle<backend::HwBufferObject> handle,
        mat4f const* transforms, size_t boneCount, size_t offset) noexcept {
    auto& driverApi = engine.getDriverApi();
    size_t size = boneCount * sizeof(PerRenderableUibBone);
    PerRenderableUibBone* UTILS_RESTRICT out = (PerRenderableUibBone*)driverApi.allocate(size);
    for (size_t i = 0, c = boneCount; i < c; ++i) {
        FSkinningBuffer::makeBone(&out[i], transforms[i]);
    }
    driverApi.updateBufferObject(handle, { out, size },
            offset * sizeof(PerRenderableUibBone));
}

void FSkinningBuffer::makeBone(PerRenderableUibBone* UTILS_RESTRICT out, mat4f const& t) noexcept {
    mat4f m(t);

    // figure out the scales
    float4 s = { length(m[0]), length(m[1]), length(m[2]), 0.0f };
    if (dot(cross(m[0].xyz, m[1].xyz), m[2].xyz) < 0) {
        s[2] = -s[2];
    }

    // compute the inverse scales
    float4 is = { 1.0f/s.x, 1.0f/s.y, 1.0f/s.z, 0.0f };

    // normalize the matrix
    m[0] *= is[0];
    m[1] *= is[1];
    m[2] *= is[2];

    out->s = s;
    out->q = m.toQuaternion();
    out->t = m[3];
    out->ns = is / max(abs(is));
}

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

void SkinningBuffer::setBones(Engine& engine,
        RenderableManager::Bone const* transforms, size_t count, size_t offset) {
    upcast(this)->setBones(upcast(engine), transforms, count, offset);
}

void SkinningBuffer::setBones(Engine& engine,
        math::mat4f const* transforms, size_t count, size_t offset) {
    upcast(this)->setBones(upcast(engine), transforms, count, offset);
}

size_t SkinningBuffer::getBoneCount() const noexcept {
    return upcast(this)->getBoneCount();
}

} // namespace filament

