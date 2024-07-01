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

#include <string.h>

#include <stddef.h>
#include <stdint.h>

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
    return downcast(engine).createSkinningBuffer(*this);
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
            getPhysicalBoneCount(mBoneCount) * sizeof(PerRenderableBoneUib::BoneData),
            BufferObjectBinding::UNIFORM,
            BufferUsage::DYNAMIC);

    if (builder->mInitialize) {
        // initialize the bones to identity (before rounding up)
        auto* out = driver.allocatePod<PerRenderableBoneUib::BoneData>(mBoneCount);
        std::uninitialized_fill_n(out, mBoneCount, FSkinningBuffer::makeBone({}));
        driver.updateBufferObject(mHandle, {
            out, mBoneCount * sizeof(PerRenderableBoneUib::BoneData) }, 0);
    }
}

void FSkinningBuffer::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyBufferObject(mHandle);
}

void FSkinningBuffer::setBones(FEngine& engine,
        RenderableManager::Bone const* transforms, size_t count, size_t offset) {
    FILAMENT_CHECK_PRECONDITION((offset + count) <= mBoneCount)
            << "SkinningBuffer (size=" << (unsigned)mBoneCount
            << ") overflow (boneCount=" << (unsigned)count << ", offset=" << (unsigned)offset
            << ")";

    setBones(engine, mHandle, transforms, count, offset);
}

void FSkinningBuffer::setBones(FEngine& engine,
        math::mat4f const* transforms, size_t count, size_t offset) {
    FILAMENT_CHECK_PRECONDITION((offset + count) <= mBoneCount)
            << "SkinningBuffer (size=" << (unsigned)mBoneCount
            << ") overflow (boneCount=" << (unsigned)count << ", offset=" << (unsigned)offset
            << ")";

    setBones(engine, mHandle, transforms, count, offset);
}

UTILS_UNUSED
static uint32_t packHalf2x16(half2 v) noexcept {
    uint32_t lo = getBits(v[0]);
    uint32_t hi = getBits(v[1]);
    return (hi << 16) | lo;
}

void FSkinningBuffer::setBones(FEngine& engine, Handle<backend::HwBufferObject> handle,
        RenderableManager::Bone const* transforms, size_t boneCount, size_t offset) noexcept {
    auto& driverApi = engine.getDriverApi();
    auto* UTILS_RESTRICT out = driverApi.allocatePod<PerRenderableBoneUib::BoneData>(boneCount);
    for (size_t i = 0, c = boneCount; i < c; ++i) {
        // the transform is stored in row-major, last row is not stored.
        mat4f transform(transforms[i].unitQuaternion);
        transform[3] = float4{ transforms[i].translation, 1.0f };
        out[i] = makeBone(transform);
    }
    driverApi.updateBufferObject(handle, {
                    out, boneCount * sizeof(PerRenderableBoneUib::BoneData) },
            offset * sizeof(PerRenderableBoneUib::BoneData));
}

PerRenderableBoneUib::BoneData FSkinningBuffer::makeBone(mat4f transform) noexcept {
    const mat3f cofactors = cof(transform.upperLeft());
    transform = transpose(transform); // row-major conversion
    return {
            .transform = {
                    transform[0],
                    transform[1],
                    transform[2]
            },
            .cof0 = cofactors[0],
            .cof1x = cofactors[1].x
    };
}

void FSkinningBuffer::setBones(FEngine& engine, Handle<backend::HwBufferObject> handle,
        mat4f const* transforms, size_t boneCount, size_t offset) noexcept {
    auto& driverApi = engine.getDriverApi();
    auto* UTILS_RESTRICT out = driverApi.allocatePod<PerRenderableBoneUib::BoneData>(boneCount);
    for (size_t i = 0, c = boneCount; i < c; ++i) {
        // the transform is stored in row-major, last row is not stored.
        out[i] = makeBone(transforms[i]);
    }
    driverApi.updateBufferObject(handle, { out, boneCount * sizeof(PerRenderableBoneUib::BoneData) },
            offset * sizeof(PerRenderableBoneUib::BoneData));
}

// This value is limited by ES3.0, ES3.0 only guarantees 2048.
// When you change this value, you must change MAX_SKINNING_BUFFER_WIDTH at getters.vs
constexpr size_t MAX_SKINNING_BUFFER_WIDTH = 2048;

static inline size_t getSkinningBufferWidth(size_t pairCount) noexcept {
    return std::clamp(pairCount, size_t(1), MAX_SKINNING_BUFFER_WIDTH);
}

static inline size_t getSkinningBufferHeight(size_t pairCount) noexcept {
    return std::max(size_t(1),
            (pairCount + MAX_SKINNING_BUFFER_WIDTH - 1) / MAX_SKINNING_BUFFER_WIDTH);
}

inline size_t getSkinningBufferSize(size_t pairCount) noexcept {
    const size_t stride = getSkinningBufferWidth(pairCount);
    const size_t height = getSkinningBufferHeight(pairCount);
    return Texture::PixelBufferDescriptor::computeDataSize(
            Texture::PixelBufferDescriptor::PixelDataFormat::RG,
            Texture::PixelBufferDescriptor::PixelDataType::FLOAT,
            stride, height, 1);
}

UTILS_NOINLINE
void updateDataAt(backend::DriverApi& driver,
        Handle<HwTexture> handle, PixelDataFormat format, PixelDataType type,
        const utils::FixedCapacityVector<math::float2>& pairs,
        size_t count) {

    size_t const elementSize = sizeof(float2);
    size_t const size = getSkinningBufferSize(count);
    auto* out = (float2*)malloc(size);
    std::memcpy(out, pairs.begin(), size);

    size_t const textureWidth = getSkinningBufferWidth(count);
    size_t const lineCount = count / textureWidth;
    size_t const lastLineCount = count % textureWidth;

    // 'out' buffer is going to be used up to 2 times, so for simplicity we use a shared_buffer
    // to manage its lifetime. One side effect of this is that the callbacks below will allocate
    // a small object on the heap. (inspired by MorphTargetBuffered)
    std::shared_ptr<void> const allocation((void*)out, ::free);

    if (lineCount) {
        // update the full-width lines if any
        driver.update3DImage(handle, 0, 0, 0, 0,
                textureWidth, lineCount, 1,
                PixelBufferDescriptor::make(
                        out, textureWidth * lineCount * elementSize,
                        format, type, [allocation](void const*, size_t) {}
                ));
        out += lineCount * textureWidth;
    }

    if (lastLineCount) {
        // update the last partial line if any
        driver.update3DImage(handle, 0, 0, lineCount, 0,
                lastLineCount, 1, 1,
                PixelBufferDescriptor::make(
                        out, lastLineCount * elementSize,
                        format, type, [allocation](void const*, size_t) {}
                ));
    }
}

backend::TextureHandle FSkinningBuffer::createIndicesAndWeightsHandle(
        FEngine& engine, size_t count) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    // create a texture for skinning pairs data (bone index and weight)
    return driver.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RG32F, 1,
            getSkinningBufferWidth(count),
            getSkinningBufferHeight(count), 1,
            TextureUsage::DEFAULT);
}

void FSkinningBuffer::setIndicesAndWeightsData(FEngine& engine,
        backend::Handle<backend::HwTexture> textureHandle,
        const utils::FixedCapacityVector<math::float2>& pairs, size_t count) {

    FEngine::DriverApi& driver = engine.getDriverApi();
    updateDataAt(driver, textureHandle,
            Texture::Format::RG, Texture::Type::FLOAT,
            pairs, count);
}

} // namespace filament

