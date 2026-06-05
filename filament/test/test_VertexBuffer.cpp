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

#include "MockDriver.h"
#include "details/Engine.h"
#include "CommandStreamDispatcher.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "private/backend/CommandStream.h"
#include "private/backend/Driver.h"

#include <filament/Engine.h>
#include <filament/VertexBuffer.h>
#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace filament;
using namespace filament::backend;
using ::testing::_;
using ::testing::Return;

namespace {

class CustomTestDriver : public MockDriver {
public:
    explicit CustomTestDriver(std::atomic<uint32_t>* sizeDest, std::mutex* mutex, std::condition_variable* cv)
            : mSizeDest(sizeDest), mMutex(mutex), mCv(cv) {
        // Set up default nice mock behaviors to pass Engine validation checks
        ON_CALL(*this, getFeatureLevel())
                .WillByDefault(Return(FeatureLevel::FEATURE_LEVEL_1));
        ON_CALL(*this, isTextureFormatSupported(_))
                .WillByDefault(Return(true));
        ON_CALL(*this, isRenderTargetFormatSupported(_))
                .WillByDefault(Return(true));
        ON_CALL(*this, isTextureFormatMipmappable(_))
                .WillByDefault(Return(true));
        ON_CALL(*this, isTextureFormatFilterable(_))
                .WillByDefault(Return(true));
        ON_CALL(*this, getMaxTextureSize(_))
                .WillByDefault(Return(2048));
    }
    ~CustomTestDriver() override = default;

    // Implement the executed command callback to capture the size (Fake implementation)
    void createBufferObjectR(BufferObjectHandle h, uint32_t byteCount,
            BufferObjectBinding bindingType, BufferUsage usage, utils::ImmutableCString&& tag) {
        if (tag == "MyTestVertexBuffer") {
            mSizeDest->store(byteCount, std::memory_order_release);
            mCv->notify_one(); // Signal that the size has been captured
        }
    }

    // Force our concrete dispatcher to bind static calls to this subclass
    Dispatcher getDispatcher() const noexcept override;

private:
    std::atomic<uint32_t>* mSizeDest;
    std::mutex* mMutex;
    std::condition_variable* mCv;
};

Dispatcher CustomTestDriver::getDispatcher() const noexcept {
    return ConcreteDispatcher<CustomTestDriver>::make();
}

class CustomTestPlatform : public Platform {
public:
    explicit CustomTestPlatform(std::atomic<uint32_t>* sizeDest, std::mutex* mutex, std::condition_variable* cv)
            : mSizeDest(sizeDest), mMutex(mutex), mCv(cv) {}
    ~CustomTestPlatform() override = default;

    int getOSVersion() const noexcept override { return 0; }
    utils::CString getDeviceInfo(DeviceInfoType infoType, Driver* driver) const override { return {}; }

    Driver* createDriver(void* sharedContext, const Platform::DriverConfig& driverConfig) override {
        // Heap-allocate the driver. The Engine takes ownership and deletes it in FEngine destructor.
        return new ::testing::NiceMock<CustomTestDriver>(mSizeDest, mMutex, mCv);
    }

private:
    std::atomic<uint32_t>* mSizeDest;
    std::mutex* mMutex;
    std::condition_variable* mCv;
};

class VertexBufferTest : public ::testing::Test {
protected:
    VertexBufferTest()
            : mCapturedSize(0),
              mCustomPlatform(&mCapturedSize, &mMutex, &mCv) {
        mEngine = Engine::Builder()
                          .backend(Backend::NOOP)
                          .platform(&mCustomPlatform)
                          .build();
    }

    void TearDown() override {
        Engine::destroy(&mEngine);
    }

    void runSizingTest(VertexBuffer* vb, uint32_t expectedSize) {
        // Flush the command buffer so that the background render thread processes creation
        static_cast<FEngine*>(mEngine)->flush();

        // Safely block the main thread until the asynchronous creation command completes
        std::unique_lock<std::mutex> lock(mMutex);
        bool success = mCv.wait_for(lock, std::chrono::seconds(2), [this]() {
            return mCapturedSize.load(std::memory_order_acquire) != 0;
        });

        EXPECT_TRUE(success);
        EXPECT_EQ(mCapturedSize.load(), expectedSize);

        mEngine->destroy(vb);
    }

    Engine* mEngine = nullptr;
    std::atomic<uint32_t> mCapturedSize;
    std::mutex mMutex;
    std::condition_variable mCv;
    CustomTestPlatform mCustomPlatform;
};

TEST_F(VertexBufferTest, InterleavedBufferSize) {
    // Interleaved (array of struct) buffer, no padding at end.
    VertexBuffer* vb = VertexBuffer::Builder()
            .name("MyTestVertexBuffer")
            .vertexCount(100)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, ElementType::FLOAT4, 0, 68)
            // offset 56, size 12, so stride = 56+12=68 bytes
            .attribute(VertexAttribute::COLOR, 0, ElementType::FLOAT3, 56, 68)
            .build(*mEngine);

    runSizingTest(vb, 6800);
}

TEST_F(VertexBufferTest, ConcatenatedBufferSize) {
    // Concatenated (struct of array) vertex buffer, like vbotest.cpp.
    VertexBuffer* vb = VertexBuffer::Builder()
            .name("MyTestVertexBuffer")
            .vertexCount(100)
            .bufferCount(1)
            // Bytes 0-799
            .attribute(VertexAttribute::POSITION, 0, ElementType::FLOAT2, 0, 8)
            // Bytes 800-1199. This comes AFTER all the positions
            .attribute(VertexAttribute::COLOR, 0, ElementType::UBYTE4, 800, 4)
            .build(*mEngine);

    runSizingTest(vb, 1200);
}

TEST_F(VertexBufferTest, TrailingPaddingInterleavedBufferSize) {
    // A 32-byte aligned array-of-struct with trailing padding.
    VertexBuffer* vb = VertexBuffer::Builder()
            .name("MyTestVertexBuffer")
            .vertexCount(100)
            .bufferCount(1)
            // 0-11
            .attribute(VertexAttribute::POSITION, 0, ElementType::FLOAT3, 0, 32)
            // 12-19
            .attribute(VertexAttribute::UV0, 0, ElementType::FLOAT2, 12, 32)
            // Padding: 20-31
            .build(*mEngine);

    runSizingTest(vb, 3200);
}

} // namespace
