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

#include "private/filament/SibGenerator.h"

#include "details/Engine.h"

#include "FilamentAPI-impl.h"

#include <math/mat4.h>

#include <math/norm.h>

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
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder&
BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder&
BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

MorphTargetBuffer::Builder& MorphTargetBuffer::Builder::vertexCount(size_t vertexCount) noexcept {
    mImpl->mVertexCount = vertexCount;
    return *this;
}

MorphTargetBuffer::Builder& MorphTargetBuffer::Builder::count(size_t count) noexcept {
    mImpl->mCount = count;
    return *this;
}

MorphTargetBuffer* MorphTargetBuffer::Builder::build(Engine& engine) {
    return upcast(engine).createMorphTargetBuffer(*this);
}

// ------------------------------------------------------------------------------------------------

// This value is limited by ES3.0, ES3.0 only guarantees 2048.
// When you change this value, you must change MAX_MORPH_TARGET_BUFFER_WIDTH at getters.vs
constexpr size_t MAX_MORPH_TARGET_BUFFER_WIDTH = 2048;

static const auto FREE_CALLBACK =
        [](void* mem, size_t, void*) { ::free(mem); };

static inline size_t getWidth(size_t vertexCount) noexcept {
    return std::min(vertexCount, MAX_MORPH_TARGET_BUFFER_WIDTH);
}

static inline size_t getHeight(size_t vertexCount) noexcept {
    return (vertexCount + MAX_MORPH_TARGET_BUFFER_WIDTH) / MAX_MORPH_TARGET_BUFFER_WIDTH;
}

template<VertexAttribute A>
inline size_t getSize(size_t vertexCount) noexcept;

template<>
inline size_t getSize<VertexAttribute::POSITION>(size_t vertexCount) noexcept {
    const size_t stride = getWidth(vertexCount) * sizeof(float3);
    const size_t height = getHeight(vertexCount);
    return Texture::PixelBufferDescriptor::computeDataSize(
            Texture::PixelBufferDescriptor::PixelDataFormat::RGB,
            Texture::PixelBufferDescriptor::PixelDataType::FLOAT,
            stride, height, 1);
}

template<>
inline size_t getSize<VertexAttribute::TANGENTS>(size_t vertexCount) noexcept {
    const size_t stride = getWidth(vertexCount) * sizeof(short4);
    const size_t height = getHeight(vertexCount);
    return Texture::PixelBufferDescriptor::computeDataSize(
            Texture::PixelBufferDescriptor::PixelDataFormat::RG_INTEGER,
            Texture::PixelBufferDescriptor::PixelDataType::UINT,
            stride, height, 1);
}

FMorphTargetBuffer::FMorphTargetBuffer(FEngine& engine, const Builder& builder)
        : mSBuffer(PerRenderPrimitiveMorphingSib::SAMPLER_COUNT),
          mVertexCount(builder->mVertexCount),
          mCount(builder->mCount) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    mSbHandle = driver.createSamplerGroup(PerRenderPrimitiveMorphingSib::SAMPLER_COUNT);
    mPbHandle = driver.createTexture(SamplerType::SAMPLER_2D_ARRAY, 1, TextureFormat::RGB32F, 1,
            getWidth(mVertexCount), getHeight(mVertexCount), mCount, TextureUsage::DEFAULT);
    mTbHandle = driver.createTexture(SamplerType::SAMPLER_2D_ARRAY, 1, TextureFormat::RG32UI, 1,
            getWidth(mVertexCount), getHeight(mVertexCount), mCount,TextureUsage::DEFAULT);

    SamplerParams samplerParams{};
    samplerParams.filterMin = SamplerMinFilter::NEAREST;
    samplerParams.filterMag = SamplerMagFilter::NEAREST;
    mSBuffer.setSampler(PerRenderPrimitiveMorphingSib::POSITIONS, mPbHandle, samplerParams);
    mSBuffer.setSampler(PerRenderPrimitiveMorphingSib::TANGENTS, mTbHandle, samplerParams);
}

void FMorphTargetBuffer::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroySamplerGroup(mSbHandle);
    driver.destroyTexture(mTbHandle);
    driver.destroyTexture(mPbHandle);
}

void FMorphTargetBuffer::setPositionsAt(FEngine& engine, size_t targetIndex,
        math::float3 const* positions, size_t count) {
    ASSERT_PRECONDITION(targetIndex < mCount, "targetIndex must be < count");

    auto dstSize = getSize<VertexAttribute::POSITION>(mVertexCount);
    auto srcSize = sizeof(float3) * count;

    ASSERT_PRECONDITION(srcSize <= dstSize,
            "MorphTargetBuffer (size=%lu) overflow (size=%lu)", dstSize, srcSize);

    // We could use a pool instead of malloc() directly.
    auto* out = (float3*) malloc(dstSize);
    memcpy(out, positions, srcSize);
    updatePositionsAt(engine, targetIndex, out, dstSize);
}

void FMorphTargetBuffer::setPositionsAt(FEngine& engine, size_t targetIndex,
        math::float4 const* positions, size_t count) {
    ASSERT_PRECONDITION(targetIndex < mCount, "targetIndex must be < count");

    auto size = getSize<VertexAttribute::POSITION>(mVertexCount);

    ASSERT_PRECONDITION(count <= mVertexCount,
            "MorphTargetBuffer (count=%lu) overflow (count=%lu)", mVertexCount, count);

    // We could use a pool instead of malloc() directly.
    auto* out = (float3*) malloc(size);
    std::transform(positions, positions + count, out,
            [](const float4& p) { return float3(p.xyz); });
    updatePositionsAt(engine, targetIndex, out, size);
}

void FMorphTargetBuffer::setTangentsAt(FEngine& engine, size_t targetIndex,
        math::short4 const* tangents, size_t count) {
    ASSERT_PRECONDITION(targetIndex < mCount, "targetIndex must be < count");
    auto size = getSize<VertexAttribute::TANGENTS>(mVertexCount);

    ASSERT_PRECONDITION(sizeof(short4) * count <= size,
            "MorphTargetBuffer (size=%lu) overflow (size=%lu)", size, sizeof(short4) * count);

    // We could use a pool instead of malloc() directly.
    auto* out = (short4*) malloc(size);
    memcpy(out, tangents, sizeof(short4) * count);

    Texture::PixelBufferDescriptor buffer(out, size,Texture::Format::RG_INTEGER,
            Texture::Type::UINT, FREE_CALLBACK);
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.update3DImage(mTbHandle, 0, 0, 0, targetIndex,
            getWidth(mVertexCount), getHeight(mVertexCount), 1,
            std::move(buffer));
}

void FMorphTargetBuffer::commit(FEngine& engine) const noexcept {
    if (mSBuffer.isDirty()) {
        FEngine::DriverApi& driver = engine.getDriverApi();
        driver.updateSamplerGroup(mSbHandle, std::move(mSBuffer.toCommandStream()));
    }
}

void FMorphTargetBuffer::bind(backend::DriverApi& driver) const noexcept {
    driver.bindSamplers(BindingPoints::PER_RENDERABLE_MORPHING, mSbHandle);
}

void FMorphTargetBuffer::updatePositionsAt(FEngine& engine, size_t targetIndex,
        void* data, size_t size) {
    Texture::PixelBufferDescriptor buffer(data, size, Texture::Format::RGB, Texture::Type::FLOAT,
            FREE_CALLBACK);
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.update3DImage(mPbHandle, 0, 0, 0, targetIndex,
            getWidth(mVertexCount), getHeight(mVertexCount), 1,
            std::move(buffer));
}

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

void MorphTargetBuffer::setPositionsAt(Engine& engine, size_t targetIndex,
        math::float3 const* positions, size_t count) {
    upcast(this)->setPositionsAt(upcast(engine), targetIndex, positions, count);
}

void MorphTargetBuffer::setPositionsAt(Engine& engine, size_t targetIndex,
        math::float4 const* positions, size_t count) {
    upcast(this)->setPositionsAt(upcast(engine), targetIndex, positions, count);
}

void MorphTargetBuffer::setTangentsAt(Engine& engine, size_t targetIndex,
        math::short4 const* tangents, size_t count) {
    upcast(this)->setTangentsAt(upcast(engine), targetIndex, tangents, count);
}

size_t MorphTargetBuffer::getVertexCount() const noexcept {
    return upcast(this)->getVertexCount();
}

size_t MorphTargetBuffer::getCount() const noexcept {
    return upcast(this)->getCount();
}

} // namespace filament

