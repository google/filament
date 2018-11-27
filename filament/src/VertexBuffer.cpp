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

#include "details/Engine.h"

#include "FilamentAPI-impl.h"

#include <math/mat3.h>
#include <math/norm.h>
#include <math/quat.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/Panic.h>

namespace filament {

using namespace details;
using namespace math;

struct VertexBuffer::BuilderDetails {
    VertexBuffer::Builder::AttributeData mAttributes[MAX_ATTRIBUTE_BUFFERS_COUNT];
    AttributeBitset mDeclaredAttributes;
    uint32_t mVertexCount = 0;
    uint8_t mBufferCount = 0;
};

static bool hasIntegerTarget(VertexAttribute attribute) {
    return attribute == BONE_INDICES;
}

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

VertexBuffer::Builder& VertexBuffer::Builder::bufferCount(uint8_t bufferCount) noexcept {
    mImpl->mBufferCount = bufferCount;
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::attribute(VertexAttribute attribute,
        uint8_t bufferIndex,
        AttributeType attributeType,
        uint32_t byteOffset,
        uint8_t byteStride) noexcept {

    size_t attributeSize = Driver::getElementTypeSize(attributeType);
    if (byteStride == 0) {
        byteStride = (uint8_t)attributeSize;
    }

    if (size_t(attribute) < MAX_ATTRIBUTE_BUFFERS_COUNT &&
        size_t(bufferIndex) < MAX_ATTRIBUTE_BUFFERS_COUNT) {

#ifndef NDEBUG
        if (byteOffset & 0x3) {
            utils::slog.d << "[performance] VertexBuffer::Builder::attribute() "
                             "byteOffset not multiple of 4" << utils::io::endl;
        }
        if (byteStride & 0x3) {
            utils::slog.d << "[performance] VertexBuffer::Builder::attribute() "
                             "byteStride not multiple of 4" << utils::io::endl;
        }
#endif

        AttributeData& entry = mImpl->mAttributes[attribute];
        entry.buffer = bufferIndex;
        entry.offset = byteOffset;
        entry.stride = byteStride;
        entry.type = attributeType;
        if (hasIntegerTarget(attribute)) {
            entry.flags |= Driver::Attribute::FLAG_INTEGER_TARGET;
        }
        mImpl->mDeclaredAttributes.set(attribute);
    }
    return *this;
}

VertexBuffer::Builder& VertexBuffer::Builder::normalized(VertexAttribute attribute) noexcept {
    if (size_t(attribute) < MAX_ATTRIBUTE_BUFFERS_COUNT) {
        AttributeData& entry = mImpl->mAttributes[attribute];
        entry.flags |= Driver::Attribute::FLAG_NORMALIZED;
    }
    return *this;
}

VertexBuffer* VertexBuffer::Builder::build(Engine& engine) {
    if (!ASSERT_PRECONDITION_NON_FATAL(mImpl->mVertexCount > 0, "vertexCount cannot be 0")) {
        return nullptr;
    }

    if (!ASSERT_PRECONDITION_NON_FATAL(mImpl->mBufferCount > 0, "bufferCount cannot be 0")) {
        return nullptr;
    }

    return upcast(engine).createVertexBuffer(*this);
}

// ------------------------------------------------------------------------------------------------

namespace details {

FVertexBuffer::FVertexBuffer(FEngine& engine, const VertexBuffer::Builder& builder)
        : mVertexCount(builder->mVertexCount), mBufferCount(builder->mBufferCount) {
    std::copy(std::begin(builder->mAttributes), std::end(builder->mAttributes), mAttributes.begin());

    mDeclaredAttributes = builder->mDeclaredAttributes;
    uint8_t attributeCount = (uint8_t) mDeclaredAttributes.count();

    Driver::AttributeArray attributeArray;

    static_assert(attributeArray.size() == MAX_ATTRIBUTE_BUFFERS_COUNT,
            "Driver::Attribute and Builder::Attribute arrays must match");

    static_assert(sizeof(Driver::Attribute) == sizeof(Builder::AttributeData),
            "Driver::Attribute and Builder::Attribute must match");

    auto const& declaredAttributes = mDeclaredAttributes;
    auto const& attributes = mAttributes;
    #pragma nounroll
    for (size_t i = 0, n = attributeArray.size(); i < n; ++i) {
        if (declaredAttributes[i]) {
            attributeArray[i].offset = attributes[i].offset;
            attributeArray[i].stride = attributes[i].stride;
            attributeArray[i].buffer = attributes[i].buffer;
            attributeArray[i].type   = attributes[i].type;
            attributeArray[i].flags  = attributes[i].flags;
        }
    }

    FEngine::DriverApi& driver = engine.getDriverApi();
    mHandle = driver.createVertexBuffer(
            mBufferCount, attributeCount, mVertexCount, attributeArray, driver::BufferUsage::STATIC);
}

void FVertexBuffer::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyVertexBuffer(mHandle);
}

size_t FVertexBuffer::getVertexCount() const noexcept {
    return mVertexCount;
}

void FVertexBuffer::setBufferAt(FEngine& engine, uint8_t bufferIndex,
        driver::BufferDescriptor&& buffer, uint32_t byteOffset, uint32_t byteSize) {

    if (byteSize == 0) {
        byteSize = uint32_t(buffer.size);
    }

    if (bufferIndex < mBufferCount) {
        engine.getDriverApi().updateVertexBuffer(mHandle, bufferIndex,
                std::move(buffer), byteOffset, byteSize);
    } else {
        ASSERT_PRECONDITION_NON_FATAL(bufferIndex < mBufferCount,
                "bufferIndex must be < bufferCount");
    }
}

} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;

size_t VertexBuffer::getVertexCount() const noexcept {
    return upcast(this)->getVertexCount();
}

void VertexBuffer::setBufferAt(Engine& engine, uint8_t bufferIndex,
        driver::BufferDescriptor&& buffer, uint32_t byteOffset, uint32_t byteSize) {
    upcast(this)->setBufferAt(upcast(engine), bufferIndex,
            std::move(buffer), byteOffset, byteSize);
}

void VertexBuffer::populateTangentQuaternions(const QuatTangentContext& ctx) {
    if (!ASSERT_PRECONDITION_NON_FATAL(ctx.normals, "Normals must be provided")) {
        return;
    }

    // Define a small lambda that converts fp32 into the desired output format.
    void (*writeQuat)(math::quatf, uint8_t*);
    switch (ctx.quatType) {
        case HALF4:
            writeQuat = [] (quatf inquat, uint8_t* outquat) {
                *((quath*) outquat) = quath(inquat);
            };
            break;
        case SHORT4:
            writeQuat = [] (quatf inquat, uint8_t* outquat) {
                *((short4*) outquat) = packSnorm16(inquat.xyzw);
            };
            break;
        case FLOAT4:
            writeQuat = [] (quatf inquat, uint8_t* outquat) {
                *((quatf*) outquat) = inquat;
            };
            break;
    }

    const float3* normal = ctx.normals;
    const size_t nstride = ctx.normalsStride ? ctx.normalsStride : sizeof(math::float3);
    uint8_t* outquat = (uint8_t*) ctx.outBuffer;

    // If tangents are not provided, simply cross N with arbitrary vector (1, 0, 0)
    if (!ctx.tangents) {
        for (size_t qindex = 0, qcount = ctx.quatCount; qindex < qcount; ++qindex) {
            float3 n = *normal;
            float3 b = normalize(cross(n, float3{1, 0, 0}));
            float3 t = cross(n, b);
            writeQuat(mat3f::packTangentFrame({t, b, n}), outquat);
            normal = (const float3*) (((const uint8_t*) normal) + nstride);
            outquat += ctx.outStride;
        }
        return;
    }

    const float3* tanvec = &ctx.tangents->xyz;
    const float* tandir = &ctx.tangents->w;
    const size_t tstride = ctx.tangentsStride ? ctx.tangentsStride : sizeof(math::float4);

    for (size_t qindex = 0, qcount = ctx.quatCount; qindex < qcount; ++qindex) {
        float3 n = *normal;
        float3 t = *tanvec;
        float3 b = *tandir > 0 ? cross(t, n) : cross(n, t);
        writeQuat(mat3f::packTangentFrame({t, b, n}), outquat);
        normal = (const float3*) (((const uint8_t*) normal) + nstride);
        tanvec = (const float3*) (((const uint8_t*) tanvec) + tstride);
        tandir = (const float*) (((const uint8_t*) tandir) + tstride);
        outquat += ctx.outStride;
    }
}

} // namespace filament
