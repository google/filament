/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include <filament/BufferObject.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/VertexBuffer.h>

#include <backend/BufferDescriptor.h>
#include <backend/DriverEnums.h>

#include <utils/Panic.h>

#include <gtest/gtest.h>

#include <stddef.h>
#include <stdint.h>

using namespace filament;
using namespace filament::backend;

namespace {

// Backing store for the BufferDescriptors below. Large enough that a descriptor can always claim
// more bytes than any buffer built here has capacity for, so the descriptor itself is never the
// thing that overflows -- the precondition under test is.
uint8_t gScratch[8192] = {};

BufferDescriptor makeDescriptor(size_t const size) {
    return BufferDescriptor(gScratch, size);
}

// The buffers built by the fixture below, and their resulting byte capacities.
constexpr size_t INDEX_COUNT = 100;
constexpr size_t INDEX_CAPACITY = INDEX_COUNT * sizeof(uint16_t);   // USHORT -> 200 bytes

constexpr size_t VERTEX_COUNT = 100;
constexpr size_t VERTEX_STRIDE = 16;                                // FLOAT4
constexpr size_t VERTEX_CAPACITY = VERTEX_COUNT * VERTEX_STRIDE;    // 1600 bytes

constexpr size_t BUFFER_OBJECT_CAPACITY = 256;

// A size large enough that `byteOffset + size` wraps around to a small value in size_t. An
// additive `byteOffset + size <= capacity` check computes 7 for this pair and wrongly accepts
// the write; the two-comparison form rejects it on `size <= capacity` alone.
constexpr size_t WRAPPING_SIZE = SIZE_MAX - 8;
constexpr uint32_t WRAPPING_SIZE_OFFSET = 16;

class BufferBoundsTest : public ::testing::Test {
protected:
    void SetUp() override {
        mEngine = Engine::Builder().backend(Backend::NOOP).build();
        ASSERT_NE(mEngine, nullptr);
    }

    void TearDown() override {
        Engine::destroy(&mEngine);
    }

    IndexBuffer* buildIndexBuffer() const {
        return IndexBuffer::Builder()
                .indexCount(INDEX_COUNT)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*mEngine);
    }

    VertexBuffer* buildVertexBuffer() const {
        return VertexBuffer::Builder()
                .vertexCount(VERTEX_COUNT)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, ElementType::FLOAT4, 0, VERTEX_STRIDE)
                .build(*mEngine);
    }

    BufferObject* buildBufferObject() const {
        return BufferObject::Builder()
                .size(BUFFER_OBJECT_CAPACITY)
                .build(*mEngine);
    }

    Engine* mEngine = nullptr;
};

// A precondition failure surfaces differently depending on how the library was built: it throws
// when exceptions are enabled, and aborts otherwise.
#if GTEST_HAS_EXCEPTIONS
#define EXPECT_OVERFLOW_REJECTED(statement) \
        EXPECT_THROW(statement, utils::PreconditionPanic)
#else
#define EXPECT_OVERFLOW_REJECTED(statement) \
        EXPECT_DEATH(statement, "buffer overflow")
#endif

// ---------------------------------------------------------------------------------------------
// IndexBuffer
// ---------------------------------------------------------------------------------------------

TEST_F(BufferBoundsTest, IndexBufferAcceptsExactFit) {
    IndexBuffer* ib = buildIndexBuffer();
    // Filling the buffer completely is the largest legal write.
    ib->setBuffer(*mEngine, makeDescriptor(INDEX_CAPACITY), 0);
    mEngine->destroy(ib);
}

TEST_F(BufferBoundsTest, IndexBufferAcceptsOffsetWriteEndingAtCapacity) {
    IndexBuffer* ib = buildIndexBuffer();
    // A partial write whose range ends exactly on the last byte is still in bounds.
    ib->setBuffer(*mEngine, makeDescriptor(INDEX_CAPACITY / 2), INDEX_CAPACITY / 2);
    mEngine->destroy(ib);
}

TEST_F(BufferBoundsTest, IndexBufferRejectsOversizedWrite) {
    IndexBuffer* ib = buildIndexBuffer();
    EXPECT_OVERFLOW_REJECTED(ib->setBuffer(*mEngine, makeDescriptor(INDEX_CAPACITY + 4), 0));
    mEngine->destroy(ib);
}

TEST_F(BufferBoundsTest, IndexBufferRejectsWriteRunningPastEnd) {
    IndexBuffer* ib = buildIndexBuffer();
    // Size alone fits, and offset alone is in range, but together they run off the end.
    EXPECT_OVERFLOW_REJECTED(
            ib->setBuffer(*mEngine, makeDescriptor(INDEX_CAPACITY / 2), INDEX_CAPACITY / 2 + 4));
    mEngine->destroy(ib);
}

TEST_F(BufferBoundsTest, IndexBufferRejectsOffsetThatWouldWrap) {
    // byteOffset is near the top of uint32_t. Note this does not actually wrap on a 64-bit
    // size_t, since byteOffset promotes before the addition -- it wraps only where size_t is
    // 32-bit. See IndexBufferRejectsSizeThatWouldWrap for the case that wraps everywhere.
    IndexBuffer* ib = buildIndexBuffer();
    EXPECT_OVERFLOW_REJECTED(ib->setBuffer(*mEngine, makeDescriptor(64), 0xFFFFFFF0u));
    mEngine->destroy(ib);
}

TEST_F(BufferBoundsTest, IndexBufferRejectsSizeThatWouldWrap) {
    IndexBuffer* ib = buildIndexBuffer();
    EXPECT_OVERFLOW_REJECTED(
            ib->setBuffer(*mEngine, makeDescriptor(WRAPPING_SIZE), WRAPPING_SIZE_OFFSET));
    mEngine->destroy(ib);
}

TEST_F(BufferBoundsTest, IndexBufferRejectsOversizedAsyncWrite) {
    IndexBuffer* ib = buildIndexBuffer();
    // The async setter validates the same way as the synchronous one.
    EXPECT_OVERFLOW_REJECTED(ib->setBufferAsync(
            *mEngine, makeDescriptor(INDEX_CAPACITY + 4), 0, nullptr, nullptr, nullptr));
    mEngine->destroy(ib);
}

// ---------------------------------------------------------------------------------------------
// VertexBuffer
// ---------------------------------------------------------------------------------------------

TEST_F(BufferBoundsTest, VertexBufferAcceptsExactFit) {
    VertexBuffer* vb = buildVertexBuffer();
    vb->setBufferAt(*mEngine, 0, makeDescriptor(VERTEX_CAPACITY), 0);
    mEngine->destroy(vb);
}

TEST_F(BufferBoundsTest, VertexBufferRejectsOversizedWrite) {
    VertexBuffer* vb = buildVertexBuffer();
    EXPECT_OVERFLOW_REJECTED(
            vb->setBufferAt(*mEngine, 0, makeDescriptor(VERTEX_CAPACITY + 4), 0));
    mEngine->destroy(vb);
}

TEST_F(BufferBoundsTest, VertexBufferRejectsWriteRunningPastEnd) {
    VertexBuffer* vb = buildVertexBuffer();
    EXPECT_OVERFLOW_REJECTED(
            vb->setBufferAt(*mEngine, 0, makeDescriptor(VERTEX_CAPACITY / 2),
                    VERTEX_CAPACITY / 2 + 4));
    mEngine->destroy(vb);
}

TEST_F(BufferBoundsTest, VertexBufferRejectsOffsetThatWouldWrap) {
    VertexBuffer* vb = buildVertexBuffer();
    EXPECT_OVERFLOW_REJECTED(vb->setBufferAt(*mEngine, 0, makeDescriptor(64), 0xFFFFFFF0u));
    mEngine->destroy(vb);
}

TEST_F(BufferBoundsTest, VertexBufferRejectsSizeThatWouldWrap) {
    VertexBuffer* vb = buildVertexBuffer();
    EXPECT_OVERFLOW_REJECTED(
            vb->setBufferAt(*mEngine, 0, makeDescriptor(WRAPPING_SIZE), WRAPPING_SIZE_OFFSET));
    mEngine->destroy(vb);
}

// ---------------------------------------------------------------------------------------------
// BufferObject
// ---------------------------------------------------------------------------------------------

TEST_F(BufferBoundsTest, BufferObjectAcceptsExactFit) {
    BufferObject* bo = buildBufferObject();
    bo->setBuffer(*mEngine, makeDescriptor(BUFFER_OBJECT_CAPACITY), 0);
    mEngine->destroy(bo);
}

TEST_F(BufferBoundsTest, BufferObjectRejectsOversizedWrite) {
    BufferObject* bo = buildBufferObject();
    EXPECT_OVERFLOW_REJECTED(
            bo->setBuffer(*mEngine, makeDescriptor(BUFFER_OBJECT_CAPACITY + 4), 0));
    mEngine->destroy(bo);
}

TEST_F(BufferBoundsTest, BufferObjectRejectsWriteRunningPastEnd) {
    BufferObject* bo = buildBufferObject();
    EXPECT_OVERFLOW_REJECTED(
            bo->setBuffer(*mEngine, makeDescriptor(BUFFER_OBJECT_CAPACITY / 2),
                    BUFFER_OBJECT_CAPACITY / 2 + 4));
    mEngine->destroy(bo);
}

TEST_F(BufferBoundsTest, BufferObjectRejectsOffsetThatWouldWrap) {
    BufferObject* bo = buildBufferObject();
    EXPECT_OVERFLOW_REJECTED(bo->setBuffer(*mEngine, makeDescriptor(64), 0xFFFFFFF0u));
    mEngine->destroy(bo);
}

TEST_F(BufferBoundsTest, BufferObjectRejectsSizeThatWouldWrap) {
    BufferObject* bo = buildBufferObject();
    EXPECT_OVERFLOW_REJECTED(
            bo->setBuffer(*mEngine, makeDescriptor(WRAPPING_SIZE), WRAPPING_SIZE_OFFSET));
    mEngine->destroy(bo);
}

} // namespace
