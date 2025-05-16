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

#include "details/MorphTargetBuffer.h"

#include <private/filament/SibStructs.h>

#include <details/Engine.h>

#include "FilamentAPI-impl.h"

#include <math/mat4.h>
#include <math/norm.h>

#include <utils/CString.h>
#include <utils/StaticString.h>

namespace filament {

using namespace backend;
using namespace math;

struct MorphTargetBuffer::BuilderDetails {
    size_t mVertexCount = 0;
    size_t mCount = 0;
};

using BuilderType = MorphTargetBuffer;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder&& rhs) noexcept = default;

MorphTargetBuffer::Builder& MorphTargetBuffer::Builder::vertexCount(size_t const vertexCount) noexcept {
    mImpl->mVertexCount = vertexCount;
    return *this;
}

MorphTargetBuffer::Builder& MorphTargetBuffer::Builder::count(size_t const count) noexcept {
    mImpl->mCount = count;
    return *this;
}

MorphTargetBuffer::Builder& MorphTargetBuffer::Builder::name(const char* name, size_t const len) noexcept {
    return BuilderNameMixin::name(name, len);
}

MorphTargetBuffer::Builder& MorphTargetBuffer::Builder::name(utils::StaticString const& name) noexcept {
    return BuilderNameMixin::name(name);
}

MorphTargetBuffer* MorphTargetBuffer::Builder::build(Engine& engine) {
    return downcast(engine).createMorphTargetBuffer(*this);
}

// ------------------------------------------------------------------------------------------------

// This value is limited by ES3.0, ES3.0 only guarantees 2048.
// When you change this value, you must change MAX_MORPH_TARGET_BUFFER_WIDTH at surface_getters.vs
constexpr size_t MAX_MORPH_TARGET_BUFFER_WIDTH = 2048;

static inline size_t getWidth(size_t const vertexCount) noexcept {
    return std::min(vertexCount, MAX_MORPH_TARGET_BUFFER_WIDTH);
}

static inline size_t getHeight(size_t const vertexCount) noexcept {
    return (vertexCount + MAX_MORPH_TARGET_BUFFER_WIDTH) / MAX_MORPH_TARGET_BUFFER_WIDTH;
}

template<VertexAttribute A>
inline size_t getSize(size_t vertexCount) noexcept;

template<>
inline size_t getSize<POSITION>(size_t const vertexCount) noexcept {
    const size_t stride = getWidth(vertexCount);
    const size_t height = getHeight(vertexCount);
    return Texture::PixelBufferDescriptor::computeDataSize(
            Texture::PixelBufferDescriptor::PixelDataFormat::RGBA,
            Texture::PixelBufferDescriptor::PixelDataType::FLOAT,
            stride, height, 1);
}

template<>
inline size_t getSize<TANGENTS>(size_t const vertexCount) noexcept {
    const size_t stride = getWidth(vertexCount);
    const size_t height = getHeight(vertexCount);
    return Texture::PixelBufferDescriptor::computeDataSize(
            Texture::PixelBufferDescriptor::PixelDataFormat::RGBA_INTEGER,
            Texture::PixelBufferDescriptor::PixelDataType::SHORT,
            stride, height, 1);
}

FMorphTargetBuffer::EmptyMorphTargetBuilder::EmptyMorphTargetBuilder() {
    mImpl->mVertexCount = 1;
    mImpl->mCount = 1;
}

FMorphTargetBuffer::FMorphTargetBuffer(FEngine& engine, const Builder& builder)
        : mVertexCount(builder->mVertexCount),
          mCount(builder->mCount) {

    if (UTILS_UNLIKELY(engine.getSupportedFeatureLevel() <= FeatureLevel::FEATURE_LEVEL_0)) {
        // feature level 0 doesn't support morph target buffers
        return;
    }

    FEngine::DriverApi& driver = engine.getDriverApi();

    // create buffer (here a texture) to store the morphing vertex data
    mPbHandle = driver.createTexture(SamplerType::SAMPLER_2D_ARRAY, 1,
            TextureFormat::RGBA32F, 1,
            getWidth(mVertexCount),
            getHeight(mVertexCount),
            mCount,
            TextureUsage::DEFAULT);

    mTbHandle = driver.createTexture(SamplerType::SAMPLER_2D_ARRAY, 1,
            TextureFormat::RGBA16I, 1,
            getWidth(mVertexCount),
            getHeight(mVertexCount),
            mCount,
            TextureUsage::DEFAULT);

    if (auto name = builder.getName(); !name.empty()) {
        driver.setDebugTag(mPbHandle.getId(), name);
        driver.setDebugTag(mTbHandle.getId(), std::move(name));
    }
}

void FMorphTargetBuffer::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    if (mTbHandle) {
        driver.destroyTexture(mTbHandle);
    }
    if (mPbHandle) {
        driver.destroyTexture(mPbHandle);
    }
}

void FMorphTargetBuffer::setPositionsAt(FEngine& engine, size_t const targetIndex,
        float3 const* positions, size_t const count, size_t const offset) {
    FILAMENT_CHECK_PRECONDITION(offset + count <= mVertexCount)
            << "MorphTargetBuffer (size=" << (unsigned)mVertexCount
            << ") overflow (count=" << (unsigned)count << ", offset=" << (unsigned)offset << ")";

    auto size = getSize<POSITION>(count);

    FILAMENT_CHECK_PRECONDITION(targetIndex < mCount)
            << targetIndex << " target index must be < " << mCount;

    // We could use a pool instead of malloc() directly.
    auto* out = (float4*) malloc(size);
    std::transform(positions, positions + count, out,
            [](const float3& p) { return float4(p, 1.0f); });

    FEngine::DriverApi& driver = engine.getDriverApi();
    updateDataAt(driver, mPbHandle,
            Texture::Format::RGBA, Texture::Type::FLOAT,
            (char const*)out, sizeof(float4), targetIndex,
            count, offset);
}

void FMorphTargetBuffer::setPositionsAt(FEngine& engine, size_t const targetIndex,
        float4 const* positions, size_t const count, size_t const offset) {
    FILAMENT_CHECK_PRECONDITION(offset + count <= mVertexCount)
            << "MorphTargetBuffer (size=" << (unsigned)mVertexCount
            << ") overflow (count=" << (unsigned)count << ", offset=" << (unsigned)offset << ")";

    auto size = getSize<POSITION>(count);

    FILAMENT_CHECK_PRECONDITION(targetIndex < mCount)
            << targetIndex << " target index must be < " << mCount;

    // We could use a pool instead of malloc() directly.
    auto* out = (float4*) malloc(size);
    memcpy(out, positions, sizeof(float4) * count);

    FEngine::DriverApi& driver = engine.getDriverApi();
    updateDataAt(driver, mPbHandle,
            Texture::Format::RGBA, Texture::Type::FLOAT,
            (char const*)out, sizeof(float4), targetIndex,
            count, offset);
}

void FMorphTargetBuffer::setTangentsAt(FEngine& engine, size_t const targetIndex,
        short4 const* tangents, size_t const count, size_t const offset) {
    FILAMENT_CHECK_PRECONDITION(offset + count <= mVertexCount)
            << "MorphTargetBuffer (size=" << (unsigned)mVertexCount
            << ") overflow (count=" << (unsigned)count << ", offset=" << (unsigned)offset << ")";

    const auto size = getSize<TANGENTS>(count);

    FILAMENT_CHECK_PRECONDITION(targetIndex < mCount)
            << targetIndex << " target index must be < " << mCount;

    // We could use a pool instead of malloc() directly.
    auto* out = (short4*) malloc(size);
    memcpy(out, tangents, sizeof(short4) * count);

    FEngine::DriverApi& driver = engine.getDriverApi();
    updateDataAt(driver, mTbHandle,
            Texture::Format::RGBA_INTEGER, Texture::Type::SHORT,
            (char const*)out, sizeof(short4), targetIndex,
            count, offset);
}

UTILS_NOINLINE
void FMorphTargetBuffer::updateDataAt(DriverApi& driver,
        Handle<HwTexture> handle, PixelDataFormat const format, PixelDataType const type,
        const char* out, size_t const elementSize,
        size_t const targetIndex, size_t const count, size_t const offset) {

    size_t yoffset              = offset / MAX_MORPH_TARGET_BUFFER_WIDTH;
    size_t const xoffset        = offset % MAX_MORPH_TARGET_BUFFER_WIDTH;
    size_t const textureWidth   = getWidth(mVertexCount);
    size_t const alignment      = ((textureWidth - xoffset) % textureWidth);
    size_t const lineCount      = (count > alignment) ? (count - alignment) / textureWidth : 0;
    size_t const lastLineCount  = (count > alignment) ? (count - alignment) % textureWidth : 0;

    // 'out' buffer is going to be used up to 3 times, so for simplicity we use a shared_buffer
    // to manage its lifetime. One side effect of this is that the callbacks below will allocate
    // a small object on the heap.
    std::shared_ptr<void> const allocation((void*)out, free);

    // Note: because the texture width is up to 2048, we're expecting that most of the time
    // only a single texture update call will be necessary (i.e. that there are no more
    // than 2048 vertices).

    // TODO: we could improve code locality a bit if we handled the "partial single line" and
    //       the "full first several lines" with the first call to update3DImage below.

    if (xoffset) {
        // update the first partial line if any
        driver.update3DImage(handle, 0, xoffset, yoffset, targetIndex,
                min(count, textureWidth - xoffset), 1, 1,
                PixelBufferDescriptor::make(
                        out, (textureWidth - xoffset) * elementSize,
                        format, type,
                        [allocation](void const*, size_t) {}
                ));
        yoffset++;
        out += min(count, textureWidth - xoffset) * elementSize;
    }

    if (lineCount) {
        // update the full-width lines if any
        driver.update3DImage(handle, 0, 0, yoffset, targetIndex,
                textureWidth, lineCount, 1, PixelBufferDescriptor::make(
                        out, (textureWidth * lineCount) * elementSize,
                        format, type,
                        [allocation](void const*, size_t) {}
                ));
        yoffset += lineCount;
        out += (lineCount * textureWidth) * elementSize;
    }

    if (lastLineCount) {
        // update the last partial line if any
        driver.update3DImage(handle, 0, 0, yoffset, targetIndex,
                lastLineCount, 1, 1, PixelBufferDescriptor::make(
                        out, lastLineCount * elementSize,
                        format, type,
                        [allocation](void const*, size_t) {}
                ));
    }
}

} // namespace filament

