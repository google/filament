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

enum { POSITIONS_CHANGED = 0x1, TANGENTS_CHANGED = 0x02 };

constexpr size_t MAX_TEXTURE_WIDTH = 4096;

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
        :mSBuffer(PerRenderPrimitiveMorphingSib::SAMPLER_COUNT),
         mVertexCount(builder->mVertexCount),
         mCount(builder->mCount) {
    FEngine::DriverApi& driver = engine.getDriverApi();

    mTargets.resize(mCount, Target(getSize(mVertexCount)));
    for (auto& target : mTargets) {
        target.initialize();
    }

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

    for (auto& target : mTargets) {
        target.terminate();
    }
    mTargets.clear();
}

void FMorphTargets::setPositionsAt(size_t targetIndex, math::float3 const* positions, size_t count) {
    ASSERT_PRECONDITION(targetIndex < mCount, "targetIndex must be < count");

    auto& target = mTargets[targetIndex];

    ASSERT_PRECONDITION((int)sizeof(math::float3) * count <= target.getSize(),
            "MorphTargets (size=%lu) overflow (size=%lu)",
            target.getSize(), sizeof(math::float3) * count);

    target.setPositions(positions, count);
}

void FMorphTargets::setPositionsAt(size_t targetIndex, math::float4 const* positions, size_t count) {
    ASSERT_PRECONDITION(targetIndex < mCount, "targetIndex must be < count");

    auto& target = mTargets[targetIndex];

    ASSERT_PRECONDITION((int)sizeof(math::float3) * count <= target.getSize(),
            "MorphTargets (size=%lu) overflow (size=%lu)",
            target.getSize(), sizeof(math::float3) * count);

    target.setPositions(positions, count);
}

void FMorphTargets::setTangentsAt(size_t targetIndex, math::short4 const* tangents, size_t count) {
    ASSERT_PRECONDITION(targetIndex < mCount, "targetIndex must be < count");

    auto& target = mTargets[targetIndex];

    ASSERT_PRECONDITION((int)sizeof(math::short4) * count <= target.getSize(),
            "MorphTargets (size=%lu) overflow (size=%lu)",
            target.getSize(), sizeof(math::short4) * count);

    target.setTangents(tangents, count);
}

void FMorphTargets::commit(FEngine& engine) const noexcept {
    FEngine::DriverApi& driver = engine.getDriverApi();

    for (size_t ti = 0, c = mCount; ti != c; ++ti) {
        if (UTILS_UNLIKELY(mTargets[ti].isAnyDirty(POSITIONS_CHANGED))) {
            Texture::PixelBufferDescriptor buffer(mTargets[ti].getPositions(), mTargets[ti].getSize(),
                    Texture::Format::RGBA, Texture::Type::FLOAT);
            driver.update3DImage(mTbHandle, 0,
                    0, 0, getPositionIndex(ti),
                    getWidth(mVertexCount), getHeight(mVertexCount), 1,
                    std::move(buffer));
        }

        if (UTILS_UNLIKELY(mTargets[ti].isAnyDirty(TANGENTS_CHANGED))) {
            Texture::PixelBufferDescriptor buffer(mTargets[ti].getTangents(), mTargets[ti].getSize(),
                    Texture::Format::RGBA, Texture::Type::FLOAT);
            driver.update3DImage(mTbHandle, 0,
                    0, 0, getTangentIndex(ti),
                    getWidth(mVertexCount), getHeight(mVertexCount), 1,
                    std::move(buffer));
        }

        mTargets[ti].clearDirty();
    }

    if (mSBuffer.isDirty()) {
        driver.updateSamplerGroup(mSbHandle, std::move(mSBuffer.toCommandStream()));
    }
}

void FMorphTargets::bind(backend::DriverApi& driver) const noexcept {
    driver.bindSamplers(BindingPoints::PER_RENDERABLE_MORPHING, mSbHandle);
}

FMorphTargets::Target::Target(size_t size) noexcept
        : mSize(size) {
}

void FMorphTargets::Target::initialize() {
    mPositions = static_cast<float4*>(mAllocator.alloc(mSize));
    mTangents = static_cast<float4*>(mAllocator.alloc(mSize));
}

void FMorphTargets::Target::terminate() {
    mAllocator.free(mTangents, mSize);
    mAllocator.free(mPositions, mSize);
}

void FMorphTargets::Target::setPositions(math::float3 const* positions, size_t count) noexcept {
    std::transform(positions, positions + count, mPositions,
            [](const float3& p) { return float4(p, 1.0f); });
    mDirtyFlags |= POSITIONS_CHANGED;
}

void FMorphTargets::Target::setPositions(math::float4 const* positions, size_t count) noexcept {
    memcpy(mPositions, positions, sizeof(float4) * count);
    mDirtyFlags |= POSITIONS_CHANGED;
}

void FMorphTargets::Target::setTangents(math::short4 const* tangents, size_t count) noexcept {
    std::transform(tangents, tangents + count, mTangents,
            [](const short4& t) { return unpackSnorm16(t); });
    mDirtyFlags |= TANGENTS_CHANGED;
}

bool FMorphTargets::Target::isAnyDirty(uint8_t dirtyFlags) const noexcept {
    return mDirtyFlags & dirtyFlags;
}

void FMorphTargets::Target::clearDirty() const noexcept {
    mDirtyFlags = 0x0;
}

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

void MorphTargets::setPositionsAt(size_t targetIndex, math::float3 const* positions, size_t count) {
    upcast(this)->setPositionsAt(targetIndex, positions, count);
}

void MorphTargets::setPositionsAt(size_t targetIndex, math::float4 const* positions, size_t count) {
    upcast(this)->setPositionsAt(targetIndex, positions, count);
}

void MorphTargets::setTangentsAt(size_t targetIndex, math::short4 const* tangents, size_t count) {
    upcast(this)->setTangentsAt(targetIndex, tangents, count);
}

size_t MorphTargets::getVertexCount() const noexcept {
    return upcast(this)->getVertexCount();
}

size_t MorphTargets::getCount() const noexcept {
    return upcast(this)->getCount();
}

} // namespace filament

