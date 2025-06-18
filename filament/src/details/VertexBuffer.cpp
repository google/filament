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

#include <filament/MaterialEnums.h>
#include <filament/VertexBuffer.h>

#include <backend/DriverEnums.h>
#include <backend/BufferDescriptor.h>

#include <utils/CString.h>
#include <utils/Logger.h>
#include <utils/Panic.h>
#include <utils/StaticString.h>
#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <utility>

#include <stddef.h>
#include <stdint.h>

namespace filament {

namespace {

// TODO: reconcile this value (defined in VertexBuffer.h) with DriverEnums' MAX_VERTEX_BUFFER_COUNT
constexpr size_t DOCUMENTED_MAX_VERTEX_BUFFER_COUNT = 8;

} // anonymous

using namespace backend;
using namespace filament::math;

struct VertexBuffer::BuilderDetails {
    struct AttributeData : Attribute {
        AttributeData() : Attribute{ .type = ElementType::FLOAT4 } {
            static_assert(sizeof(Attribute) == sizeof(AttributeData),
                    "Attribute and Builder::Attribute must match");
        }
    };
    std::array<AttributeData, MAX_VERTEX_ATTRIBUTE_COUNT> mAttributes{};
    AttributeBitset mDeclaredAttributes;
    uint32_t mVertexCount = 0;
    uint8_t mBufferCount = 0;
    bool mBufferObjectsEnabled = false;
    bool mAdvancedSkinningEnabled = false; // TODO: use bits to save memory
};

using BuilderType = VertexBuffer;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder&& rhs) noexcept = default;

VertexBuffer::Builder& VertexBuffer::Builder::vertexCount(uint32_t const vertexCount) noexcept {
    mImpl->mVertexCount = vertexCount;
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::enableBufferObjects(bool const enabled) noexcept {
    mImpl->mBufferObjectsEnabled = enabled;
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::bufferCount(uint8_t const bufferCount) noexcept {
    mImpl->mBufferCount = bufferCount;
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::attribute(VertexAttribute const attribute,
        uint8_t const bufferIndex,
        AttributeType const attributeType,
        uint32_t const byteOffset,
        uint8_t byteStride) noexcept {

    size_t const attributeSize = Driver::getElementTypeSize(attributeType);
    if (byteStride == 0) {
        byteStride = uint8_t(attributeSize);
    }

    if (size_t(attribute) < MAX_VERTEX_ATTRIBUTE_COUNT &&
            size_t(bufferIndex) < MAX_VERTEX_ATTRIBUTE_COUNT) {
        auto& entry = mImpl->mAttributes[attribute];
        entry.buffer = bufferIndex;
        entry.offset = byteOffset;
        entry.stride = byteStride;
        entry.type = attributeType;
        if (attribute == BONE_INDICES) {
            // BONE_INDICES must always be an integer type
            entry.flags |= Attribute::FLAG_INTEGER_TARGET;
        }

        mImpl->mDeclaredAttributes.set(attribute);
    } else {
        LOG(WARNING) << "Ignoring VertexBuffer attribute, the limit of "
                     << MAX_VERTEX_ATTRIBUTE_COUNT << " attributes has been exceeded";
    }
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::normalized(VertexAttribute const attribute,
        bool const normalized) noexcept {
    if (size_t(attribute) < MAX_VERTEX_ATTRIBUTE_COUNT) {
        auto& entry = mImpl->mAttributes[attribute];
        if (normalized) {
            entry.flags |= Attribute::FLAG_NORMALIZED;
        } else {
            entry.flags &= ~Attribute::FLAG_NORMALIZED;
        }
    }
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::advancedSkinning(bool const enabled) noexcept {
    mImpl->mAdvancedSkinningEnabled = enabled;
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::name(const char* name, size_t const len) noexcept {
    return BuilderNameMixin::name(name, len);
}

VertexBuffer::Builder& VertexBuffer::Builder::name(utils::StaticString const& name) noexcept {
    return BuilderNameMixin::name(name);
}

VertexBuffer* VertexBuffer::Builder::build(Engine& engine) {
    FILAMENT_CHECK_PRECONDITION(mImpl->mVertexCount > 0) << "vertexCount cannot be 0";
    FILAMENT_CHECK_PRECONDITION(mImpl->mBufferCount > 0) << "bufferCount cannot be 0";

    static_assert(DOCUMENTED_MAX_VERTEX_BUFFER_COUNT <= MAX_VERTEX_BUFFER_COUNT);

    if (downcast(engine).features.engine.debug.assert_vertex_buffer_count_exceeds_8) {
        FILAMENT_CHECK_PRECONDITION(mImpl->mBufferCount <= DOCUMENTED_MAX_VERTEX_BUFFER_COUNT)
                << "bufferCount cannot be more than " << DOCUMENTED_MAX_VERTEX_BUFFER_COUNT;
    } else if (mImpl->mBufferCount > DOCUMENTED_MAX_VERTEX_BUFFER_COUNT) {
        LOG(WARNING) << "bufferCount cannot be more than " << DOCUMENTED_MAX_VERTEX_BUFFER_COUNT;
    }

    // Next we check if any unused buffer slots have been allocated. This helps prevent errors
    // because uploading to an unused slot can trigger undefined behavior in the backend.
    auto const& declaredAttributes = mImpl->mDeclaredAttributes;
    auto const& attributes = mImpl->mAttributes;
    utils::bitset32 attributedBuffers;

    declaredAttributes.forEachSetBit([&](size_t const j) {

        FILAMENT_CHECK_PRECONDITION((attributes[j].offset & 0x3u) == 0)
                << "attribute " << j << " offset=" << attributes[j].offset
                << " is not multiple of 4";

        FILAMENT_CHECK_PRECONDITION((attributes[j].stride & 0x3u) == 0)
                << "attribute " << j << " stride=" << attributes[j].stride
                << " is not multiple of 4";

        if (engine.getActiveFeatureLevel() == FeatureLevel::FEATURE_LEVEL_0) {
            FILAMENT_CHECK_PRECONDITION(!(attributes[j].flags & Attribute::FLAG_INTEGER_TARGET))
                    << "Attribute::FLAG_INTEGER_TARGET not supported at FEATURE_LEVEL_0";
        }

        // also checks that we don't use an invalid type with integer attributes
        if (attributes[j].flags & Attribute::FLAG_INTEGER_TARGET) {
            using ET = ElementType;
            constexpr uint32_t invalidIntegerTypes =
                    (1 << int(ET::FLOAT)) |
                    (1 << int(ET::FLOAT2)) |
                    (1 << int(ET::FLOAT3)) |
                    (1 << int(ET::FLOAT4)) |
                    (1 << int(ET::HALF)) |
                    (1 << int(ET::HALF2)) |
                    (1 << int(ET::HALF3)) |
                    (1 << int(ET::HALF4));

            FILAMENT_CHECK_PRECONDITION(!(invalidIntegerTypes & (1 << int(attributes[j].type))))
                    << "invalid integer vertex attribute type " << int(attributes[j].type);
        }

        // update set of used buffers
        attributedBuffers.set(attributes[j].buffer);
    });

    FILAMENT_CHECK_PRECONDITION(attributedBuffers.count() == mImpl->mBufferCount)
            << "At least one buffer slot was never assigned to an attribute.";

    if (mImpl->mAdvancedSkinningEnabled) {
        FILAMENT_CHECK_PRECONDITION(!mImpl->mDeclaredAttributes[VertexAttribute::BONE_INDICES])
                << "Vertex buffer attribute BONE_INDICES is already defined, "
                   "no advanced skinning is allowed";
        FILAMENT_CHECK_PRECONDITION(!mImpl->mDeclaredAttributes[VertexAttribute::BONE_WEIGHTS])
                << "Vertex buffer attribute BONE_WEIGHTS is already defined, "
                   "no advanced skinning is allowed";
        FILAMENT_CHECK_PRECONDITION(mImpl->mBufferCount < (MAX_VERTEX_BUFFER_COUNT - 2))
                << "Vertex buffer uses to many buffers (" << mImpl->mBufferCount << ")";
    }

    return downcast(engine).createVertexBuffer(*this);
}

// ------------------------------------------------------------------------------------------------

FVertexBuffer::FVertexBuffer(FEngine& engine, const Builder& builder)
        : mVertexCount(builder->mVertexCount), mBufferCount(builder->mBufferCount),
          mBufferObjectsEnabled(builder->mBufferObjectsEnabled),
          mAdvancedSkinningEnabled(builder->mAdvancedSkinningEnabled){
    std::copy(std::begin(builder->mAttributes), std::end(builder->mAttributes), mAttributes.begin());
    mDeclaredAttributes = builder->mDeclaredAttributes;

    if (mAdvancedSkinningEnabled) {
        mAttributes[BONE_INDICES] = {
                .offset = 0,
                .stride = 8,
                .buffer = mBufferCount,
                .type = AttributeType::USHORT4,
                .flags = Attribute::FLAG_INTEGER_TARGET,
        };
        mDeclaredAttributes.set(BONE_INDICES);
        mBufferCount++;

        mAttributes[BONE_WEIGHTS] = {
                .offset = 0,
                .stride = 16,
                .buffer = mBufferCount,
                .type = AttributeType::FLOAT4,
                .flags = 0,
        };
        mDeclaredAttributes.set(BONE_WEIGHTS);
        mBufferCount++;
    } else {
        // Because the Material's SKN variant supports both skinning and morphing, it expects
        // all attributes related to *both* to be present. In turn, this means that a VertexBuffer
        // used for skinning and/or morphing, needs to provide all related attributes.
        // Currently, the backend must handle disabled arrays in the VertexBuffer that are declared
        // in the shader. In GL this happens automatically, in vulkan/metal, the backends have to
        // use dummy buffers.
        // - A complication is that backends need to know if an attribute is declared as float or
        // integer in the shader, regardless of if the attribute is enabled or not in the
        // VertexBuffer (e.g. the morphing attributes could be disabled because we're only using
        // skinning).
        // - Another complication is that the SKN variant is selected by the renderable
        // (as opposed to the RenderPrimitive), so it's possible and valid for a primitive
        // that isn't skinned nor morphed to be rendered with the SKN variant (morphing/skinning
        // will then be disabled dynamically).
        //
        // Because of that we need to set FLAG_INTEGER_TARGET on all attributes that we know are
        // integer in the shader and the bottom line is that BONE_INDICES always needs to be set to
        // FLAG_INTEGER_TARGET.
        mAttributes[BONE_INDICES].flags |= Attribute::FLAG_INTEGER_TARGET;
    }

    FEngine::DriverApi& driver = engine.getDriverApi();

    mVertexBufferInfoHandle = engine.getVertexBufferInfoFactory().create(driver,
            mBufferCount, mDeclaredAttributes.count(), mAttributes);

    mHandle = driver.createVertexBuffer(mVertexCount, mVertexBufferInfoHandle);
    if (auto name = builder.getName(); !name.empty()) {
        driver.setDebugTag(mHandle.getId(), std::move(name));
    }

    // calculate buffer sizes
    size_t bufferSizes[MAX_VERTEX_BUFFER_COUNT] = {};
    #pragma nounroll
    for (size_t i = 0, n = mAttributes.size(); i < n; ++i) {
        if (mDeclaredAttributes[i]) {
            const uint32_t offset = mAttributes[i].offset;
            const uint8_t stride = mAttributes[i].stride;
            const uint8_t slot = mAttributes[i].buffer;
            const size_t end = offset + mVertexCount * stride;
            if (slot != Attribute::BUFFER_UNUSED) {
                assert_invariant(slot < MAX_VERTEX_BUFFER_COUNT);
                bufferSizes[slot] = std::max(bufferSizes[slot], end);
            }
        }
    }

    if (!mBufferObjectsEnabled) {
        // If buffer objects are not enabled at the API level, then we create them internally.
        #pragma nounroll
        for (size_t index = 0; index < MAX_VERTEX_BUFFER_COUNT; ++index) {
            size_t const i = mAttributes[index].buffer;
            if (i != Attribute::BUFFER_UNUSED) {
                assert_invariant(bufferSizes[i] > 0);
                if (!mBufferObjects[i]) {
                    BufferObjectHandle const bo = driver.createBufferObject(bufferSizes[i],
                            BufferObjectBinding::VERTEX, BufferUsage::STATIC);
                    if (auto name = builder.getName(); !name.empty()) {
                        driver.setDebugTag(bo.getId(), std::move(name));
                    }
                    driver.setVertexBufferObject(mHandle, i, bo);
                    mBufferObjects[i] = bo;
                }
            }
        }
    } else {
        // in advanced skinning mode, we manage the BONE_INDICES and BONE_WEIGHTS arrays ourselves,
        // so we have to set the corresponding buffer objects.
        if (mAdvancedSkinningEnabled) {
            for (auto const index : { BONE_INDICES, BONE_WEIGHTS }) {
                size_t const i = mAttributes[index].buffer;
                assert_invariant(i != Attribute::BUFFER_UNUSED);
                assert_invariant(bufferSizes[i] > 0);
                if (!mBufferObjects[i]) {
                    BufferObjectHandle const bo = driver.createBufferObject(bufferSizes[i],
                            BufferObjectBinding::VERTEX, BufferUsage::STATIC);
                    if (auto name = builder.getName(); !name.empty()) {
                        driver.setDebugTag(bo.getId(), std::move(name));
                    }
                    driver.setVertexBufferObject(mHandle, i, bo);
                    mBufferObjects[i] = bo;
                }
            }
        }
    }
}

void FVertexBuffer::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    if (!mBufferObjectsEnabled) {
        for (BufferObjectHandle const& bo : mBufferObjects) {
            driver.destroyBufferObject(bo);
        }
    }
    driver.destroyVertexBuffer(mHandle);
    engine.getVertexBufferInfoFactory().destroy(driver, mVertexBufferInfoHandle);
}

size_t FVertexBuffer::getVertexCount() const noexcept {
    return mVertexCount;
}

void FVertexBuffer::setBufferAt(FEngine& engine, uint8_t const bufferIndex,
        backend::BufferDescriptor&& buffer, uint32_t const byteOffset) {

    FILAMENT_CHECK_PRECONDITION(!mBufferObjectsEnabled)
            << "buffer objects enabled, use setBufferObjectAt() instead";

    FILAMENT_CHECK_PRECONDITION(bufferIndex < mBufferCount)
            << "bufferIndex must be < bufferCount";

    FILAMENT_CHECK_PRECONDITION((byteOffset & 0x3) == 0)
        << "byteOffset must be a multiple of 4";

    engine.getDriverApi().updateBufferObject(mBufferObjects[bufferIndex],
            std::move(buffer), byteOffset);
}

void FVertexBuffer::setBufferObjectAt(FEngine& engine, uint8_t const bufferIndex,
        FBufferObject const * bufferObject) {

    FILAMENT_CHECK_PRECONDITION(mBufferObjectsEnabled)
            << "buffer objects disabled, use setBufferAt() instead";

    FILAMENT_CHECK_PRECONDITION(bufferObject->getBindingType() == BufferObject::BindingType::VERTEX)
            << "bufferObject binding type must be VERTEX but is "
            << to_string(bufferObject->getBindingType());

    FILAMENT_CHECK_PRECONDITION(bufferIndex < mBufferCount)
            << "bufferIndex must be < bufferCount";

    auto const hwBufferObject = bufferObject->getHwHandle();
    engine.getDriverApi().setVertexBufferObject(mHandle, bufferIndex, hwBufferObject);
    // store handle to recreate VertexBuffer in the case extra bone indices and weights definition
    // used only in buffer object mode
    mBufferObjects[bufferIndex] = hwBufferObject;
}

void FVertexBuffer::updateBoneIndicesAndWeights(FEngine& engine,
        std::unique_ptr<uint16_t[]> skinJoints,
        std::unique_ptr<float[]> skinWeights) {
    FILAMENT_CHECK_PRECONDITION(mAdvancedSkinningEnabled) << "No advanced skinning enabled";
    auto jointsData = skinJoints.release();
    uint8_t const indicesIndex = mAttributes[BONE_INDICES].buffer;
    engine.getDriverApi().updateBufferObject(mBufferObjects[indicesIndex],
            { jointsData, mVertexCount * 8,
              [](void* buffer, size_t, void*) { delete[] static_cast<uint16_t*>(buffer); } },
            0);

    auto weightsData = skinWeights.release();
    uint8_t const weightsIndex = mAttributes[BONE_WEIGHTS].buffer;
    engine.getDriverApi().updateBufferObject(mBufferObjects[weightsIndex],
            { weightsData, mVertexCount * 16,
              [](void* buffer, size_t, void*) { delete[] static_cast<float*>(buffer); } },
            0);
}

} // namespace filament
