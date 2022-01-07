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

#include "details/MorphTargets.h"

#include "private/filament/SibGenerator.h"

#include "details/Engine.h"

#include "FilamentAPI-impl.h"

#include <math/mat4.h>

#include <math/norm.h>

namespace filament {

using namespace backend;
using namespace math;

struct MorphTargets::BuilderDetails {
    size_t mVertexCount = 0;
    size_t mCount = 0;
};

using BuilderType = MorphTargets;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

MorphTargets::Builder& MorphTargets::Builder::vertexCount(size_t vertexCount) noexcept {
    mImpl->mVertexCount = vertexCount;
    return *this;
}

MorphTargets::Builder& MorphTargets::Builder::count(size_t count) noexcept {
    mImpl->mCount = count;
    return *this;
}

MorphTargets* MorphTargets::Builder::build(Engine& engine) {
    return upcast(engine).createMorphTargets(*this);
}

// ------------------------------------------------------------------------------------------------

constexpr size_t MAX_TEXTURE_WIDTH = 4096;

static const auto FREE_CALLBACK = [](void* mem, size_t, void*) { free(mem); };

static inline size_t getWidth(size_t vertexCount) noexcept {
    return std::min(vertexCount, MAX_TEXTURE_WIDTH);
}

static inline size_t getHeight(size_t vertexCount) noexcept {
    return (vertexCount + MAX_TEXTURE_WIDTH) / MAX_TEXTURE_WIDTH;
}

static inline size_t getDepth(size_t targetCount) noexcept {
    return targetCount * 2;
}

static inline size_t getSize(size_t vertexCount) noexcept {
    const size_t stride = getWidth(vertexCount) * sizeof(float4);
    return Texture::PixelBufferDescriptor::computeDataSize(
            Texture::PixelBufferDescriptor::PixelDataFormat::RGBA,
            Texture::PixelBufferDescriptor::PixelDataType::FLOAT,
            stride, getHeight(vertexCount), 1);
}

static inline size_t getPositionIndex(size_t targetIndex) noexcept {
    return targetIndex * 2 + 0;
}

static inline size_t getTangentIndex(size_t targetIndex) noexcept {
    return targetIndex * 2 + 1;
}

FMorphTargets::FMorphTargets(FEngine& engine, const Builder& builder)
        : mSBuffer(PerRenderPrimitiveMorphingSib::SAMPLER_COUNT),
          mVertexCount(builder->mVertexCount),
          mCount(builder->mCount) {
    FEngine::DriverApi& driver = engine.getDriverApi();

    mSbHandle = driver.createSamplerGroup(PerRenderPrimitiveMorphingSib::SAMPLER_COUNT);

    mTbHandle = driver.createTexture(SamplerType::SAMPLER_2D_ARRAY, 1, TextureFormat::RGBA32F, 1,
            getWidth(mVertexCount), getHeight(mVertexCount), getDepth(mCount),
            TextureUsage::DEFAULT);

    mSBuffer.setSampler(PerRenderPrimitiveMorphingSib::TARGETS,
            { mTbHandle, { .filterMin = SamplerMinFilter::NEAREST, .filterMag = SamplerMagFilter::NEAREST } });
}

void FMorphTargets::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();

    driver.destroySamplerGroup(mSbHandle);
    driver.destroyTexture(mTbHandle);
}

void FMorphTargets::setPositionsAt(FEngine& engine, size_t targetIndex, math::float3 const* positions, size_t count) {
    ASSERT_PRECONDITION(targetIndex < mCount, "targetIndex must be < count");

    auto size = getSize(mVertexCount);

    ASSERT_PRECONDITION((int)sizeof(math::float3) * count <= size,
            "MorphTargets (size=%lu) overflow (size=%lu)",
            size, sizeof(math::float3) * count);

    auto* out = (float4*) malloc(size);
    std::transform(positions, positions + count, out,
            [](const float3& p) { return float4(p, 1.0f); });

    Texture::PixelBufferDescriptor buffer(out, size, Texture::Format::RGBA, Texture::Type::FLOAT,
            FREE_CALLBACK);

    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.update3DImage(mTbHandle, 0, 0, 0, getPositionIndex(targetIndex),
            getWidth(mVertexCount), getHeight(mVertexCount), 1,
            std::move(buffer));
}

void FMorphTargets::setPositionsAt(FEngine& engine, size_t targetIndex, math::float4 const* positions, size_t count) {
    ASSERT_PRECONDITION(targetIndex < mCount, "targetIndex must be < count");

    auto size = getSize(mVertexCount);

    ASSERT_PRECONDITION((int)sizeof(math::float4) * count <= size,
            "MorphTargets (size=%lu) overflow (size=%lu)",
            size, sizeof(math::float4) * count);

    auto* out = (float4*) malloc(size);
    memcpy(out, positions, sizeof(math::float4) * count);

    Texture::PixelBufferDescriptor buffer(out, size, Texture::Format::RGBA, Texture::Type::FLOAT,
            FREE_CALLBACK);

    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.update3DImage(mTbHandle, 0, 0, 0, getPositionIndex(targetIndex),
            getWidth(mVertexCount), getHeight(mVertexCount), 1,
            std::move(buffer));
}

void FMorphTargets::setTangentsAt(FEngine& engine, size_t targetIndex, math::short4 const* tangents, size_t count) {
    ASSERT_PRECONDITION(targetIndex < mCount, "targetIndex must be < count");

    auto size = getSize(mVertexCount);

    ASSERT_PRECONDITION((int)sizeof(math::short4) * count <= size,
            "MorphTargets (size=%lu) overflow (size=%lu)",
            size, sizeof(math::short4) * count);

    auto* out = (float4*) malloc(size);
    std::transform(tangents, tangents + count, out,
            [](const short4& t) { return unpackSnorm16(t); });

    Texture::PixelBufferDescriptor buffer(out, size, Texture::Format::RGBA, Texture::Type::FLOAT,
            FREE_CALLBACK);

    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.update3DImage(mTbHandle, 0, 0, 0, getTangentIndex(targetIndex),
            getWidth(mVertexCount), getHeight(mVertexCount), 1,
            std::move(buffer));
}

void FMorphTargets::commit(FEngine& engine) const noexcept {
    if (mSBuffer.isDirty()) {
        FEngine::DriverApi& driver = engine.getDriverApi();
        driver.updateSamplerGroup(mSbHandle, std::move(mSBuffer.toCommandStream()));
    }
}

void FMorphTargets::bind(backend::DriverApi& driver) const noexcept {
    driver.bindSamplers(BindingPoints::PER_RENDERABLE_MORPHING, mSbHandle);
}

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

void MorphTargets::setPositionsAt(Engine& engine, size_t targetIndex, math::float3 const* positions, size_t count) {
    upcast(this)->setPositionsAt(upcast(engine), targetIndex, positions, count);
}

void MorphTargets::setPositionsAt(Engine& engine, size_t targetIndex, math::float4 const* positions, size_t count) {
    upcast(this)->setPositionsAt(upcast(engine), targetIndex, positions, count);
}

void MorphTargets::setTangentsAt(Engine& engine, size_t targetIndex, math::short4 const* tangents, size_t count) {
    upcast(this)->setTangentsAt(upcast(engine), targetIndex, tangents, count);
}

size_t MorphTargets::getVertexCount() const noexcept {
    return upcast(this)->getVertexCount();
}

size_t MorphTargets::getCount() const noexcept {
    return upcast(this)->getCount();
}

} // namespace filament

