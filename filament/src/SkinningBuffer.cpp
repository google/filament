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

#include <math/half.h>
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
            BufferUsage::DYNAMIC);

    if (builder->mInitialize) {
        // initialize the bones to identity (before rounding up)
        auto* out = driver.allocatePod<PerRenderableUibBone>(mBoneCount);
        std::uninitialized_fill_n(out, mBoneCount, PerRenderableUibBone{});
        driver.updateBufferObject(mHandle, {
            out, mBoneCount * sizeof(PerRenderableUibBone) }, 0);
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

static uint32_t packHalf2x16(half2 v) noexcept {
    uint32_t lo = getBits(v[0]);
    uint32_t hi = getBits(v[1]);
    return (hi << 16) | lo;
}

void FSkinningBuffer::setBones(FEngine& engine, Handle<backend::HwBufferObject> handle,
        RenderableManager::Bone const* transforms, size_t boneCount, size_t offset) noexcept {
    auto& driverApi = engine.getDriverApi();
    PerRenderableUibBone* UTILS_RESTRICT out = driverApi.allocatePod<PerRenderableUibBone>(boneCount);
    for (size_t i = 0, c = boneCount; i < c; ++i) {
        // the transform is stored in row-major, last row is not stored.
        mat4f transform(transforms[i].unitQuaternion);
        transform[3] = float4{ transforms[i].translation, 1.0f };
        out[i] = makeBone(transform);
    }
    driverApi.updateBufferObject(handle, {
                    out, boneCount * sizeof(PerRenderableUibBone) },
            offset * sizeof(PerRenderableUibBone));
}

PerRenderableUibBone FSkinningBuffer::makeBone(mat4f transform) noexcept {
    const mat3f cofactors = cof(transform.upperLeft());
    transform = transpose(transform); // row-major conversion
    return {
            .bone = {
                    .transform = {
                            transform[0],
                            transform[1],
                            transform[2]
                    },
                    .cof = {
                            packHalf2x16({ cofactors[0].x, cofactors[0].y }),
                            packHalf2x16({ cofactors[0].z, cofactors[1].x }),
                            packHalf2x16({ cofactors[1].y, cofactors[1].z }),
                            packHalf2x16({ cofactors[2].x, cofactors[2].y })
                            // cofactor[2][2] is not stored because we don't have space for it
                    }
            }
    };
}

void FSkinningBuffer::setBones(FEngine& engine, Handle<backend::HwBufferObject> handle,
        mat4f const* transforms, size_t boneCount, size_t offset) noexcept {
    auto& driverApi = engine.getDriverApi();
    PerRenderableUibBone* UTILS_RESTRICT out = driverApi.allocatePod<PerRenderableUibBone>(boneCount);
    for (size_t i = 0, c = boneCount; i < c; ++i) {
        // the transform is stored in row-major, last row is not stored.
        out[i] = makeBone(transforms[i]);
    }
    driverApi.updateBufferObject(handle, {
                    out, boneCount * sizeof(PerRenderableUibBone) },
            offset * sizeof(PerRenderableUibBone));
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

