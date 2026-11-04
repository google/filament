/*
* Copyright (C) 2023 The Android Open Source Project
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

#include "details/InstanceBuffer.h"

#include "FilamentAPI-impl.h"

#include "details/Engine.h"

#include <private/filament/UibStructs.h>

#include <filament/Engine.h>
#include <filament/FilamentAPI.h>
#include <filament/InstanceBuffer.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Panic.h>
#include <utils/StaticString.h>

#include <math/mat3.h>
#include <math/mat4.h>

#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <utility>

namespace filament {

using namespace backend;

namespace {

UTILS_ALWAYS_INLINE
bool isTransformIdentical(math::mat4f const& a, math::mat4f const& b) noexcept {
#if defined(__ARM_NEON) && defined(__aarch64__)
    uint32x4_t const eq0 = vceqq_f32(vld1q_f32(&a[0].x), vld1q_f32(&b[0].x));
    uint32x4_t const eq1 = vceqq_f32(vld1q_f32(&a[1].x), vld1q_f32(&b[1].x));
    uint32x4_t const eq2 = vceqq_f32(vld1q_f32(&a[2].x), vld1q_f32(&b[2].x));
    uint32x4_t const eq3 = vceqq_f32(vld1q_f32(&a[3].x), vld1q_f32(&b[3].x));
    uint32x4_t const eq = vandq_u32(vandq_u32(eq0, eq1), vandq_u32(eq2, eq3));
    return vminvq_u32(eq) != 0;
#else
    return a == b;
#endif
}

} // anonymous namespace

struct InstanceBuffer::BuilderDetails {
    size_t mInstanceCount = 0;
    math::mat4f const* mLocalTransforms = nullptr;
};

using BuilderType = InstanceBuffer;
BuilderType::Builder::Builder(size_t const instanceCount) noexcept {
    mImpl->mInstanceCount = instanceCount;
}

BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder&& rhs) noexcept = default;

InstanceBuffer::Builder& InstanceBuffer::Builder::localTransforms(
        math::mat4f const* localTransforms) noexcept {
    mImpl->mLocalTransforms = localTransforms;
    return *this;
}

InstanceBuffer::Builder& InstanceBuffer::Builder::name(const char* name, size_t const len) noexcept {
    return BuilderNameMixin::name(name, len);
}

InstanceBuffer::Builder& InstanceBuffer::Builder::name(utils::StaticString const& name) noexcept {
    return BuilderNameMixin::name(name);
}

InstanceBuffer::Builder& InstanceBuffer::Builder::name(utils::ImmutableCString const& name) noexcept {
    return BuilderNameMixin::name(name);
}

InstanceBuffer* InstanceBuffer::Builder::build(Engine& engine) const {
    FILAMENT_CHECK_PRECONDITION(mImpl->mInstanceCount >= 1) << "instanceCount must be >= 1.";
    FILAMENT_CHECK_PRECONDITION(mImpl->mInstanceCount <= engine.getMaxAutomaticInstances())
            << "instanceCount is " << mImpl->mInstanceCount
            << ", but instance count is limited to Engine::getMaxAutomaticInstances() ("
            << engine.getMaxAutomaticInstances() << ") instances when supplying transforms.";
    return downcast(engine).createInstanceBuffer(*this);
}

// ------------------------------------------------------------------------------------------------

FInstanceBuffer::FInstanceBuffer(FEngine& engine, const Builder& builder)
    : mName(builder.getName()),
      mUid(engine.allocateInstanceBufferUid()) {
    mInstanceCount = builder->mInstanceCount;

    mLocalTransforms.reserve(mInstanceCount);
    mLocalTransforms.resize(mInstanceCount);    // this will initialize all transforms to identity

    if (builder->mLocalTransforms) {
        memcpy(mLocalTransforms.data(), builder->mLocalTransforms,
                sizeof(math::mat4f) * mInstanceCount);
    }
}

void FInstanceBuffer::terminate(FEngine&) {
    mIndex = 0;
}

FInstanceBuffer::~FInstanceBuffer() noexcept = default;

void FInstanceBuffer::setLocalTransforms(
        math::mat4f const* localTransforms, size_t const count, size_t const offset) {
    FILAMENT_CHECK_PRECONDITION(count <= mInstanceCount && offset <= mInstanceCount - count)
            << "setLocalTransforms overflow. InstanceBuffer has only " << mInstanceCount
            << " instances, but trying to set " << count 
            << " transforms at offset " << offset << ".";
    // only an actual change bumps the revision, so that rewriting the same transforms doesn't
    // report any motion
    math::mat4f* const dst = mLocalTransforms.data() + offset;
    bool changed = false;
    for (size_t i = 0; i < count; i++) {
        if (!isTransformIdentical(dst[i], localTransforms[i])) {
            changed = true;
            break;
        }
    }
    if (changed) {
        memcpy(dst, localTransforms, sizeof(math::mat4f) * count);
        mRevision++;
    }
}

math::mat4f const& FInstanceBuffer::getLocalTransform(size_t index) const {
    FILAMENT_CHECK_PRECONDITION(index < mInstanceCount)
            << "getLocalTransform overflow: 'index (" << index
            << ") must be < getInstanceCount() ("<< mInstanceCount << ").";
    return mLocalTransforms[index];
}

// reads back a matrix stored in a std140 UBO structure
static math::mat4 toMat4(std140::mat44 const& m) noexcept {
    math::mat4 r;
    for (size_t c = 0; c < 4; c++) {
        for (size_t l = 0; l < 4; l++) {
            r[c][l] = m[c][l];
        }
    }
    return r;
}

bool FInstanceBuffer::prepare(
            PerRenderableData* const UTILS_RESTRICT buffer, uint32_t const index, uint32_t const count,
            math::mat4f const& rootTransform, PerRenderableData const& ubo,
            math::mat4f const* const prevLocalTransforms) {

    // there is a precondition check for this, so this assert really should never trigger
    assert_invariant(count <= mInstanceCount);

    for (size_t i = 0, c = count; i < c; i++) {
        math::mat4f const model = rootTransform * mLocalTransforms[i];
        math::mat3f const m = math::mat3f::getTransformForNormals(model.upperLeft());
        buffer[index + i] = ubo;
        buffer[index + i].worldFromModelMatrix = model;
        buffer[index + i].worldFromModelNormalMatrix = math::prescaleForNormals(m);
    }
    mIndex = index;

    bool moved = false;
    if (UTILS_UNLIKELY(prevLocalTransforms)) {
        // Motion vectors: an instance whose local transform changed from L' to L gets its own
        // previous-from-current transform, M * R * L' * inverse(L) * inverse(R), where R is the
        // root transform and M is ubo.prevWorldFromWorldMatrix (the root's motion). The other
        // instances keep M, copied with `ubo` above, which is also the fallback when R or L is
        // singular. Computed in double precision.
        math::mat4 const R{ rootTransform };
        double const detR = det(R);
        if (UTILS_VERY_LIKELY(detR != 0.0)) {
            math::mat4 const MR = toMat4(ubo.prevWorldFromWorldMatrix) * R;
            math::mat4 const invR = inverse(R, detR);
            for (size_t i = 0, c = count; i < c; i++) {
                math::mat4f const& L = mLocalTransforms[i];
                math::mat4f const& prevL = prevLocalTransforms[i];
                if (!isTransformIdentical(L, prevL)) {
                    math::mat4 const Ld{ L };
                    double const detL = det(Ld);
                    if (UTILS_VERY_LIKELY(detL != 0.0)) {
                        buffer[index + i].prevWorldFromWorldMatrix =
                                math::mat4f{ MR * math::mat4{ prevL } * inverse(Ld, detL) * invR };
                        moved = true;
                    }
                }
            }
        }
    }
    return moved;
}

} // namespace filament

