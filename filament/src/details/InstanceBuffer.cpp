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

#include "details/Engine.h"

#include "FilamentAPI-impl.h"

#include <private/filament/UibStructs.h>

#include <filament/FilamentAPI.h>
#include <filament/Engine.h>
#include <filament/InstanceBuffer.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Panic.h>
#include <utils/StaticString.h>

#include <math/mat3.h>
#include <math/mat4.h>

#include <utility>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstddef>

namespace filament {

using namespace backend;

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

InstanceBuffer* InstanceBuffer::Builder::build(Engine& engine) const {
    FILAMENT_CHECK_PRECONDITION(mImpl->mInstanceCount >= 1) << "instanceCount must be >= 1.";
    FILAMENT_CHECK_PRECONDITION(mImpl->mInstanceCount <= engine.getMaxAutomaticInstances())
            << "instanceCount is " << mImpl->mInstanceCount
            << ", but instance count is limited to Engine::getMaxAutomaticInstances() ("
            << engine.getMaxAutomaticInstances() << ") instances when supplying transforms.";
    return downcast(engine).createInstanceBuffer(*this);
}

// ------------------------------------------------------------------------------------------------

FInstanceBuffer::FInstanceBuffer(FEngine&, const Builder& builder)
    : mName(builder.getName()) {
    mInstanceCount = builder->mInstanceCount;

    mLocalTransforms.reserve(mInstanceCount);
    mLocalTransforms.resize(mInstanceCount);    // this will initialize all transforms to identity

    if (builder->mLocalTransforms) {
        memcpy(mLocalTransforms.data(), builder->mLocalTransforms,
                sizeof(math::mat4f) * mInstanceCount);
    }
}

void FInstanceBuffer::terminate(FEngine&) {
    mHandle.clear();
    mOffset = 0;
}

FInstanceBuffer::~FInstanceBuffer() noexcept = default;

void FInstanceBuffer::setLocalTransforms(
        math::mat4f const* localTransforms, size_t const count, size_t const offset) {
    FILAMENT_CHECK_PRECONDITION(offset + count <= mInstanceCount)
            << "setLocalTransforms overflow. InstanceBuffer has only " << mInstanceCount
            << " instances, but trying to set " << count 
            << " transforms at offset " << offset << ".";
    memcpy(mLocalTransforms.data() + offset, localTransforms, sizeof(math::mat4f) * count);
}

math::mat4f const& FInstanceBuffer::getLocalTransform(size_t index) const noexcept {
    FILAMENT_CHECK_PRECONDITION(index < mInstanceCount)
            << "getLocalTransform overflow: 'index (" << index
            << ") must be < getInstanceCount() ("<< mInstanceCount << ").";
    return mLocalTransforms[index];
}

void FInstanceBuffer::prepare(
            BufferObjectHandle ubh,
            PerRenderableData* const UTILS_RESTRICT buffer, uint32_t const offset, uint32_t const count,
            math::mat4f const& rootTransform, PerRenderableData const& ubo) {

    // there is a precondition check for this, so this assert really should never trigger
    assert_invariant(count <= mInstanceCount);

    for (size_t i = 0, c = count; i < c; i++) {
        math::mat4f const model = rootTransform * mLocalTransforms[i];
        math::mat3f const m = math::mat3f::getTransformForNormals(model.upperLeft());
        buffer[offset + i] = ubo;
        buffer[offset + i].worldFromModelMatrix = model;
        buffer[offset + i].worldFromModelNormalMatrix = math::prescaleForNormals(m);
    }
    mHandle = ubh;
    mOffset = offset * sizeof(PerRenderableData);
}

} // namespace filament

