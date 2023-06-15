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

#include <details/Engine.h>
#include <private/filament/UibStructs.h>

#include "FilamentAPI-impl.h"

#include <math/mat3.h>
#include <math/vec3.h>

namespace filament {

using namespace backend;

struct InstanceBuffer::BuilderDetails {
    size_t mInstanceCount = 0;
    math::mat4f const* mLocalTransforms = nullptr;
};

using BuilderType = InstanceBuffer;
BuilderType::Builder::Builder(size_t instanceCount) noexcept {
    mImpl->mInstanceCount = instanceCount;
}

BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

InstanceBuffer::Builder& InstanceBuffer::Builder::localTransforms(
        math::mat4f const* localTransforms) noexcept {
    mImpl->mLocalTransforms = localTransforms;
    return *this;
}

InstanceBuffer* InstanceBuffer::Builder::build(Engine& engine) {
    ASSERT_PRECONDITION(mImpl->mInstanceCount >= 1, "instanceCount must be >= 1.");
    ASSERT_PRECONDITION(mImpl->mInstanceCount <= engine.getMaxAutomaticInstances(),
            "instanceCount is %zu, but instance count is limited to "
            "Engine::getMaxAutomaticInstances() (%zu) instances when supplying transforms.",
            mImpl->mInstanceCount,
            engine.getMaxAutomaticInstances());
    return downcast(engine).createInstanceBuffer(*this);
}

// ------------------------------------------------------------------------------------------------

FInstanceBuffer::FInstanceBuffer(FEngine& engine, const Builder& builder) {
    mInstanceCount = builder->mInstanceCount;

    mLocalTransforms.reserve(mInstanceCount);
    mLocalTransforms.resize(mInstanceCount);    // this will initialize all transforms to identity

    if (builder->mLocalTransforms) {
        memcpy(mLocalTransforms.data(), builder->mLocalTransforms,
                sizeof(math::mat4f) * mInstanceCount);
    }
}

void FInstanceBuffer::setLocalTransforms(
        math::mat4f const* localTransforms, size_t count, size_t offset) {
    ASSERT_PRECONDITION(offset + count <= mInstanceCount,
            "setLocalTransforms overflow. InstanceBuffer has only %zu instances, but trying to set "
            "%zu transforms at offset %zu.",
            mInstanceCount,
            count,
            offset);
    memcpy(mLocalTransforms.data() + offset, localTransforms, sizeof(math::mat4f) * count);
}

void FInstanceBuffer::prepare(FEngine& engine, math::mat4f rootTransform,
        const PerRenderableData& ubo, Handle<HwBufferObject> handle) {
    DriverApi& driver = engine.getDriverApi();

    // TODO: allocate this staging buffer from a pool.
    uint32_t stagingBufferSize = sizeof(PerRenderableUib);
    PerRenderableData* stagingBuffer = (PerRenderableData*)::malloc(stagingBufferSize);
    // TODO: consider using JobSystem to parallelize this.
    for (size_t i = 0, c = mInstanceCount; i < c; i++) {
        stagingBuffer[i] = ubo;
        math::mat4f model = rootTransform * mLocalTransforms[i];
        stagingBuffer[i].worldFromModelMatrix = model;

        math::mat3f m = math::mat3f::getTransformForNormals(model.upperLeft());
        stagingBuffer[i].worldFromModelNormalMatrix = math::prescaleForNormals(m);
    }
    driver.updateBufferObject(handle, {
            stagingBuffer, stagingBufferSize,
            +[](void* buffer, size_t, void*) {
                ::free(buffer);
            }
    }, 0);
}

void FInstanceBuffer::terminate(FEngine& engine) {
}

} // namespace filament

