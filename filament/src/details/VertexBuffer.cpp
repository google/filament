/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "details/VertexBuffer.h"

#include "details/BufferObject.h"
#include "details/Engine.h"

#include "FilamentAPI-impl.h"

#include <math/quat.h>
#include <math/vec2.h>

#include <utils/Panic.h>

namespace filament {

using namespace backend;
using namespace filament::math;

struct VertexBuffer::BuilderDetails {
    FVertexBuffer::AttributeData mAttributes[MAX_VERTEX_ATTRIBUTE_COUNT];
    AttributeBitset mDeclaredAttributes;
    uint32_t mVertexCount = 0;
    uint8_t mBufferCount = 0;
    bool mBufferObjectsEnabled = false;
    bool mAdvancedSkinningEnabled = false; // TODO: use bits to save memory
};

using BuilderType = VertexBuffer;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

VertexBuffer::Builder& VertexBuffer::Builder::vertexCount(uint32_t vertexCount) noexcept {
    mImpl->mVertexCount = vertexCount;
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::enableBufferObjects(bool enabled) noexcept {
    mImpl->mBufferObjectsEnabled = enabled;
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::bufferCount(uint8_t bufferCount) noexcept {
    mImpl->mBufferCount = bufferCount;
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::attribute(VertexAttribute attribute,
        uint8_t bufferIndex,
        AttributeType attributeType,
        uint32_t byteOffset,
        uint8_t byteStride) noexcept {

    size_t const attributeSize = Driver::getElementTypeSize(attributeType);
    if (byteStride == 0) {
        byteStride = (uint8_t)attributeSize;
    }

    if (size_t(attribute) < MAX_VERTEX_ATTRIBUTE_COUNT &&
        size_t(bufferIndex) < MAX_VERTEX_ATTRIBUTE_COUNT) {

#ifndef NDEBUG
        if (byteOffset & 0x3u) {
            utils::slog.d << "[performance] VertexBuffer::Builder::attribute() "
                             "byteOffset not multiple of 4" << utils::io::endl;
        }
        if (byteStride & 0x3u) {
            utils::slog.d << "[performance] VertexBuffer::Builder::attribute() "
                             "byteStride not multiple of 4" << utils::io::endl;
        }
#endif

        FVertexBuffer::AttributeData& entry = mImpl->mAttributes[attribute];
        entry.buffer = bufferIndex;
        entry.offset = byteOffset;
        entry.stride = byteStride;
        entry.type = attributeType;
        mImpl->mDeclaredAttributes.set(attribute);
    } else {
        utils::slog.w << "Ignoring VertexBuffer attribute, the limit of " <<
                MAX_VERTEX_ATTRIBUTE_COUNT << " attributes has been exceeded" << utils::io::endl;
    }
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::normalized(VertexAttribute attribute,
        bool normalized) noexcept {
    if (size_t(attribute) < MAX_VERTEX_ATTRIBUTE_COUNT) {
        FVertexBuffer::AttributeData& entry = mImpl->mAttributes[attribute];
        if (normalized) {
            entry.flags |= Attribute::FLAG_NORMALIZED;
        } else {
            entry.flags &= ~Attribute::FLAG_NORMALIZED;
        }
    }
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::advancedSkinning(bool enabled) noexcept {
    mImpl->mAdvancedSkinningEnabled = enabled;
    return *this;
}

VertexBuffer* VertexBuffer::Builder::build(Engine& engine) {
    ASSERT_PRECONDITION(mImpl->mVertexCount > 0, "vertexCount cannot be 0");
    ASSERT_PRECONDITION(mImpl->mBufferCount > 0, "bufferCount cannot be 0");
    ASSERT_PRECONDITION(mImpl->mBufferCount <= MAX_VERTEX_BUFFER_COUNT,
            "bufferCount cannot be more than %d", MAX_VERTEX_BUFFER_COUNT);

    // Next we check if any unused buffer slots have been allocated. This helps prevent errors
    // because uploading to an unused slot can trigger undefined behavior in the backend.
    auto const& declaredAttributes = mImpl->mDeclaredAttributes;
    auto const& attributes = mImpl->mAttributes;
    utils::bitset32 attributedBuffers;
    for (size_t j = 0; j < MAX_VERTEX_ATTRIBUTE_COUNT; ++j) {
        if (declaredAttributes[j]) {
            attributedBuffers.set(attributes[j].buffer);
        }
    }

    ASSERT_PRECONDITION(attributedBuffers.count() == mImpl->mBufferCount,
            "At least one buffer slot was never assigned to an attribute.");

    return downcast(engine).createVertexBuffer(*this);
}

// ------------------------------------------------------------------------------------------------

FVertexBuffer::FVertexBuffer(FEngine& engine, const VertexBuffer::Builder& builder)
        : mVertexCount(builder->mVertexCount), mBufferCount(builder->mBufferCount),
          mBufferObjectsEnabled(builder->mBufferObjectsEnabled),
          mAdvancedSkinningEnabled(builder->mAdvancedSkinningEnabled){
    std::copy(std::begin(builder->mAttributes), std::end(builder->mAttributes), mAttributes.begin());

    mDeclaredAttributes = builder->mDeclaredAttributes;
    uint8_t const attributeCount = (uint8_t)mDeclaredAttributes.count();

    AttributeArray attributeArray;

    static_assert(attributeArray.size() == MAX_VERTEX_ATTRIBUTE_COUNT,
            "Attribute and Builder::Attribute arrays must match");

    static_assert(sizeof(Attribute) == sizeof(AttributeData),
            "Attribute and Builder::Attribute must match");

    if (mAdvancedSkinningEnabled) {
        ASSERT_PRECONDITION(!mDeclaredAttributes[VertexAttribute::BONE_INDICES],
          "Vertex buffer attribute BONE_INDICES is already defined, no advanced skinning is allowed");
        ASSERT_PRECONDITION(!mDeclaredAttributes[VertexAttribute::BONE_WEIGHTS],
          "Vertex buffer attribute BONE_WEIGHTS is already defined, no advanced skinning is allowed");
        ASSERT_PRECONDITION(mBufferCount < (MAX_VERTEX_BUFFER_COUNT - 2),
                        "Vertex buffer uses to many buffers (%u)", mBufferCount);
        mDeclaredAttributes.set(VertexAttribute::BONE_INDICES);
        mAttributes[VertexAttribute::BONE_INDICES].offset = 0;
        mAttributes[VertexAttribute::BONE_INDICES].stride = 8;
        mAttributes[VertexAttribute::BONE_INDICES].buffer = mBufferCount;
        mAttributes[VertexAttribute::BONE_INDICES].type = VertexBuffer::AttributeType::USHORT4;
        mAttributes[VertexAttribute::BONE_INDICES].flags = Attribute::FLAG_INTEGER_TARGET;
        mBufferCount++;
        mDeclaredAttributes.set(VertexAttribute::BONE_WEIGHTS);
        mAttributes[VertexAttribute::BONE_WEIGHTS].offset = 0;
        mAttributes[VertexAttribute::BONE_WEIGHTS].stride = 16;
        mAttributes[VertexAttribute::BONE_WEIGHTS].buffer = mBufferCount;
        mAttributes[VertexAttribute::BONE_WEIGHTS].type = VertexBuffer::AttributeType::FLOAT4;
        mAttributes[VertexAttribute::BONE_WEIGHTS].flags = 0;
        mBufferCount++;
    }

    size_t bufferSizes[MAX_VERTEX_BUFFER_COUNT] = {};

    auto const& declaredAttributes = mDeclaredAttributes;
    auto const& attributes = mAttributes;
    #pragma nounroll
    for (size_t i = 0, n = attributeArray.size(); i < n; ++i) {
        if (declaredAttributes[i]) {
            const uint32_t offset = attributes[i].offset;
            const uint8_t stride = attributes[i].stride;
            const uint8_t slot = attributes[i].buffer;

            attributeArray[i].offset = offset;
            attributeArray[i].stride = stride;
            attributeArray[i].buffer = slot;
            attributeArray[i].type   = attributes[i].type;
            attributeArray[i].flags  = attributes[i].flags;

            const size_t end = offset + mVertexCount * stride;
            bufferSizes[slot] = math::max(bufferSizes[slot], end);
        }
    }

    // Backends do not (and should not) know the semantics of each vertex attribute, but they
    // need to know whether the vertex shader consumes them as integers or as floats.
    // NOTE: This flag needs to be set regardless of whether the attribute is actually declared.
    attributeArray[BONE_INDICES].flags |= Attribute::FLAG_INTEGER_TARGET;

    FEngine::DriverApi& driver = engine.getDriverApi();

    // TODO: this should use a cache
    mVertexBufferInfoHandle = driver.createVertexBufferInfo(
            mBufferCount, attributeCount, attributeArray);

    mHandle = driver.createVertexBuffer(mVertexCount, mVertexBufferInfoHandle);

    // If buffer objects are not enabled at the API level, then we create them internally.
    if (!mBufferObjectsEnabled) {
        #pragma nounroll
        for (size_t i = 0; i < MAX_VERTEX_BUFFER_COUNT; ++i) {
            if (bufferSizes[i] > 0) {
                BufferObjectHandle const bo = driver.createBufferObject(bufferSizes[i],
                        backend::BufferObjectBinding::VERTEX, backend::BufferUsage::STATIC);
                driver.setVertexBufferObject(mHandle, i, bo);
                mBufferObjects[i] = bo;
            }
        }
    } else {
        // add buffer objects for indices and weights
        if (mAdvancedSkinningEnabled) {
            for (size_t i = mBufferCount - 2; i < mBufferCount; ++i) {
              BufferObjectHandle const bo = driver.createBufferObject(bufferSizes[i],
                          backend::BufferObjectBinding::VERTEX, backend::BufferUsage::STATIC);
              driver.setVertexBufferObject(mHandle, i, bo);
              mBufferObjects[i] = bo;
            }
        }
    }
}

void FVertexBuffer::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    if (!mBufferObjectsEnabled) {
        for (BufferObjectHandle const bo : mBufferObjects) {
            driver.destroyBufferObject(bo);
        }
    }
    driver.destroyVertexBuffer(mHandle);
    driver.destroyVertexBufferInfo(mVertexBufferInfoHandle);
}

size_t FVertexBuffer::getVertexCount() const noexcept {
    return mVertexCount;
}

void FVertexBuffer::setBufferAt(FEngine& engine, uint8_t bufferIndex,
        backend::BufferDescriptor&& buffer, uint32_t byteOffset) {
    ASSERT_PRECONDITION(!mBufferObjectsEnabled, "Please use setBufferObjectAt()");
    if (bufferIndex < mBufferCount) {
        assert_invariant(mBufferObjects[bufferIndex]);
        engine.getDriverApi().updateBufferObject(mBufferObjects[bufferIndex],
               std::move(buffer), byteOffset);
    } else {
        ASSERT_PRECONDITION(bufferIndex < mBufferCount, "bufferIndex must be < bufferCount");
    }
}

void FVertexBuffer::setBufferObjectAt(FEngine& engine, uint8_t bufferIndex,
        FBufferObject const * bufferObject) {
    ASSERT_PRECONDITION(mBufferObjectsEnabled, "Please use setBufferAt()");
    ASSERT_PRECONDITION(bufferObject->getBindingType() == BufferObject::BindingType::VERTEX,
            "Binding type must be VERTEX.");
    if (bufferIndex < mBufferCount) {
        auto hwBufferObject = bufferObject->getHwHandle();
        engine.getDriverApi().setVertexBufferObject(mHandle, bufferIndex, hwBufferObject);
        // store handle to recreate VertexBuffer in the case extra bone indices and weights definition
        // used only in buffer object mode
        mBufferObjects[bufferIndex] = hwBufferObject;
    } else {
        ASSERT_PRECONDITION(bufferIndex < mBufferCount, "bufferIndex must be < bufferCount");
    }
}

void FVertexBuffer::updateBoneIndicesAndWeights(FEngine& engine,
                                                std::unique_ptr<uint16_t[]> skinJoints,
                                                std::unique_ptr<float[]> skinWeights) {

    ASSERT_PRECONDITION(mAdvancedSkinningEnabled, "No advanced skinning enabled");
    auto jointsData = skinJoints.release();
    auto bdJoints = BufferDescriptor(
            jointsData, mVertexCount * 8,
            [](void *buffer, size_t size, void *user) {
                delete[] static_cast<uint16_t *>(buffer); });
    engine.getDriverApi().updateBufferObject(mBufferObjects[mBufferCount - 2],
               std::move(bdJoints), 0);

    auto weightsData = skinWeights.release();
    auto bdWeights = BufferDescriptor(
            weightsData, mVertexCount * 16,
            [](void *buffer, size_t size, void *user) {
                delete[] static_cast<float *>(buffer); });
    engine.getDriverApi().updateBufferObject(mBufferObjects[mBufferCount - 1],
                std::move(bdWeights), 0);

}
} // namespace filament
