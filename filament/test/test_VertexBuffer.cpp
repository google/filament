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

#include "CommandStreamDispatcher.h"
#include "MockDriver.h"

#include "details/Engine.h"
#include "details/VertexBuffer.h"

#include <filament/Engine.h>
#include <filament/RenderableManager.h>
#include <filament/VertexBuffer.h>

#include <private/backend/CommandStream.h>
#include <private/backend/Driver.h>

#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/Panic.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

using namespace filament;
using namespace filament::backend;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

namespace {

// State shared between the test fixture (main thread) and the driver it creates (backend thread).
struct AsyncTestState {
    std::atomic<uint32_t> capturedSize{ 0 };
    std::mutex mutex;
    std::condition_variable cv;
    // Status the test driver reports for every asynchronous creation call it enrolls.
    std::atomic<AsyncCallStatus> asyncCreationStatus{ AsyncCallStatus::COMPLETED };
    // When set, asynchronous creation calls are held back until Driver::terminate() flushes them,
    // mimicking a real backend whose in-flight jobs only settle during teardown.
    std::atomic_bool withholdAsyncCompletion{ false };
};

class CustomTestDriver : public MockDriver {
public:
    explicit CustomTestDriver(AsyncTestState* state) : mState(state) {
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
        ON_CALL(*this, isAsynchronousModeEnabled())
                .WillByDefault(Return(true));
        // Real backends drain their in-flight asynchronous jobs in terminate(), which runs on the
        // backend thread, after the engine has already cleaned up whatever the user leaked.
        ON_CALL(*this, terminate())
                .WillByDefault(Invoke([this]() { flushWithheldAsyncCallbacks(); }));
    }
    ~CustomTestDriver() override = default;

    // MockDriver drops scheduled callbacks on the floor, which would leak the
    // CountdownCallbackHandler that tracks an asynchronous creation. Run them instead.
    void scheduleCallback(CallbackHandler* handler, void* user,
            CallbackHandler::Callback callback) override {
        if (handler) {
            handler->post(user, callback);
        } else {
            callback(user);
        }
    }

    // The three asynchronous calls FVertexBuffer's creation countdown enrolls. Each reports
    // whichever status the test asked for: COMPLETED is a job that ran, CANCELED is one that was
    // canceled or dropped before it could.
    void createVertexBufferAsyncR(VertexBufferHandle, uint32_t, VertexBufferInfoHandle,
            CallbackHandler*, AsyncCallback const callback, void* user, utils::ImmutableCString&&) {
        reportAsyncCreationStatus(callback, user);
    }

    void createBufferObjectAsyncR(BufferObjectHandle, uint32_t, BufferObjectBinding, BufferUsage,
            CallbackHandler*, AsyncCallback const callback, void* user, utils::ImmutableCString&&) {
        reportAsyncCreationStatus(callback, user);
    }

    void setVertexBufferObjectAsyncR(AsyncCallId, VertexBufferHandle, uint32_t, BufferObjectHandle,
            CallbackHandler*, AsyncCallback const callback, void* user) {
        reportAsyncCreationStatus(callback, user);
    }

    // Implement the executed command callback to capture the size (Fake implementation)
    void createBufferObjectR(BufferObjectHandle h, uint32_t byteCount,
            BufferObjectBinding bindingType, BufferUsage usage, utils::ImmutableCString&& tag) {
        if (tag == "MyTestVertexBuffer") {
            mState->capturedSize.store(byteCount, std::memory_order_release);
            mState->cv.notify_one(); // Signal that the size has been captured
        }
    }

    // Force our concrete dispatcher to bind static calls to this subclass
    Dispatcher getDispatcher() const noexcept override;

private:
    void reportAsyncCreationStatus(AsyncCallback const callback, void* user) {
        if (!callback) {
            return;
        }
        if (mState->withholdAsyncCompletion.load(std::memory_order_acquire)) {
            mWithheldAsyncCallbacks.push_back({ callback, user });
            return;
        }
        callback(user, mState->asyncCreationStatus.load(std::memory_order_acquire));
    }

    void flushWithheldAsyncCallbacks() {
        auto const status = mState->asyncCreationStatus.load(std::memory_order_acquire);
        // Dispatch from a local: a callback is free to enqueue more asynchronous work.
        decltype(mWithheldAsyncCallbacks) withheld;
        withheld.swap(mWithheldAsyncCallbacks);
        for (auto [callback, user]: withheld) {
            callback(user, status);
        }
    }

    AsyncTestState* mState;
    // Only touched from the backend thread: the async entry points above and terminate().
    std::vector<std::pair<AsyncCallback, void*>> mWithheldAsyncCallbacks;
};

Dispatcher CustomTestDriver::getDispatcher() const noexcept {
    return ConcreteDispatcher<CustomTestDriver>::make();
}

class CustomTestPlatform : public Platform {
public:
    explicit CustomTestPlatform(AsyncTestState* state) : mState(state) {}
    ~CustomTestPlatform() override = default;

    int getOSVersion() const noexcept override { return 0; }
    utils::CString getDeviceInfo(DeviceInfoType infoType, Driver* driver) const override { return {}; }

    Driver* createDriver(void* sharedContext, const Platform::DriverConfig& driverConfig) override {
        // Heap-allocate the driver. The Engine takes ownership and deletes it in FEngine destructor.
        return new ::testing::NiceMock<CustomTestDriver>(mState);
    }

private:
    AsyncTestState* mState;
};

class VertexBufferTest : public ::testing::Test {
protected:
    VertexBufferTest() : mCustomPlatform(&mState) {
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
        std::unique_lock<std::mutex> lock(mState.mutex);
        bool success = mState.cv.wait_for(lock, std::chrono::seconds(2), [this]() {
            return mState.capturedSize.load(std::memory_order_acquire) != 0;
        });

        EXPECT_TRUE(success);
        EXPECT_EQ(mState.capturedSize.load(), expectedSize);

        mEngine->destroy(vb);
    }

    // Spins until the asynchronous creation reaches one of its terminal states.
    static bool waitUntilCreationSettled(VertexBuffer const* vb) {
        auto const deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (std::chrono::steady_clock::now() < deadline) {
            if (downcast(vb)->isCreationSettled()) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return false;
    }

    // Builds a VertexBuffer asynchronously, then blocks until the test driver has reported
    // `status` for every call the creation countdown enrolls.
    VertexBuffer* buildAsyncVertexBuffer(AsyncCallStatus const status) {
        mState.asyncCreationStatus.store(status, std::memory_order_release);

        VertexBuffer* vb = buildAsyncVertexBuffer();

        return waitUntilCreationSettled(vb) ? vb : nullptr;
    }

    // Builds a VertexBuffer asynchronously and flushes, without waiting for creation to settle.
    VertexBuffer* buildAsyncVertexBuffer() {
        VertexBuffer* vb = VertexBuffer::Builder()
                .name("MyAsyncVertexBuffer")
                .vertexCount(100)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, ElementType::FLOAT4, 0, 16)
                .async(nullptr, nullptr, nullptr)
                .build(*mEngine);

        // Flush so that the background render thread processes the creation commands.
        static_cast<FEngine*>(mEngine)->flush();

        return vb;
    }

    // Adds a renderable component drawing `vb` to `entity`, the public path that ends up
    // recording the buffer's backend handle into a render primitive.
    RenderableManager::Builder::Result buildRenderable(VertexBuffer* vb,
            utils::Entity const entity) const {
        return RenderableManager::Builder(1)
                .boundingBox({{ 0, 0, 0 }, { 1, 1, 1 }})
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb)
                .build(*mEngine, entity);
    }

    Engine* mEngine = nullptr;
    AsyncTestState mState;
    CustomTestPlatform mCustomPlatform;
};

TEST_F(VertexBufferTest, CompletedCreationBuildsRenderable) {
    // Every asynchronous creation call runs, so the buffer is populated and usable.
    VertexBuffer* vb = buildAsyncVertexBuffer(AsyncCallStatus::COMPLETED);
    ASSERT_NE(vb, nullptr);
    EXPECT_TRUE(vb->isCreationComplete());

    utils::Entity const entity = utils::EntityManager::get().create();
    EXPECT_EQ(buildRenderable(vb, entity), RenderableManager::Builder::Success);
    EXPECT_TRUE(downcast(vb)->getHwHandle());

    mEngine->getRenderableManager().destroy(entity);
    utils::EntityManager::get().destroy(entity);
    mEngine->destroy(vb);
}

TEST_F(VertexBufferTest, CanceledCreationRejectedByRenderableBuilder) {
    // Same buffer, but every asynchronous creation call reports itself canceled, so this one
    // settles without its backend resources ever being generated. Drawing it would record buffers
    // that were never generated, so the builder has to reject it. How the precondition reports
    // that depends on how the library was built: it throws when exceptions are enabled, and
    // aborts otherwise.
    VertexBuffer* vb = buildAsyncVertexBuffer(AsyncCallStatus::CANCELED);
    ASSERT_NE(vb, nullptr);
    EXPECT_FALSE(vb->isCreationComplete());

    utils::Entity const entity = utils::EntityManager::get().create();
#if GTEST_HAS_EXCEPTIONS
    EXPECT_THROW(buildRenderable(vb, entity), utils::PreconditionPanic);
#else
    EXPECT_DEATH(buildRenderable(vb, entity), "creation is still in progress or was canceled");
#endif

    utils::EntityManager::get().destroy(entity);
    mEngine->destroy(vb);
}

TEST_F(VertexBufferTest, CanceledCreationRejectedBySetGeometryAt) {
    // A renderable can also be re-pointed at another buffer after it was built, which is the
    // second way an unusable buffer reaches a render primitive.
    VertexBuffer* usable = buildAsyncVertexBuffer(AsyncCallStatus::COMPLETED);
    VertexBuffer* canceled = buildAsyncVertexBuffer(AsyncCallStatus::CANCELED);
    ASSERT_NE(usable, nullptr);
    ASSERT_NE(canceled, nullptr);

    utils::Entity const entity = utils::EntityManager::get().create();
    ASSERT_EQ(buildRenderable(usable, entity), RenderableManager::Builder::Success);

    auto& rcm = mEngine->getRenderableManager();
    auto const instance = rcm.getInstance(entity);
    auto const repoint = [&]() {
        rcm.setGeometryAt(instance, 0, RenderableManager::PrimitiveType::TRIANGLES, canceled, 0, 3);
    };
#if GTEST_HAS_EXCEPTIONS
    EXPECT_THROW(repoint(), utils::PreconditionPanic);
#else
    EXPECT_DEATH(repoint(), "creation is still in progress or was canceled");
#endif

    rcm.destroy(entity);
    utils::EntityManager::get().destroy(entity);
    mEngine->destroy(canceled);
    mEngine->destroy(usable);
}

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

TEST_F(VertexBufferTest, LeakedAsyncBufferOutlivesShutdownCleanup) {
    // Regression test for the use-after-free in FEngine::cleanupResourceList: the shutdown path
    // that cleans up what the user leaked used to free the frontend object unconditionally, while
    // the still-pending creation callback held a pointer to it and later wrote its status.
    mState.withholdAsyncCompletion.store(true, std::memory_order_release);

    VertexBuffer* vb = buildAsyncVertexBuffer();
    ASSERT_NE(vb, nullptr);

    // Creation cannot settle before the driver terminates, so shutdown is guaranteed to see this
    // buffer mid-creation. It is deliberately never destroyed: Engine::destroy() has to clean it
    // up, and then keep it alive until the withheld callbacks run on the backend thread.
    ASSERT_FALSE(downcast(vb)->isCreationSettled());

    // Asserts inside shutdown() catch a deferred destruction that never resolves; a sanitizer
    // build catches the frontend object being freed too early.
    Engine::destroy(&mEngine);
}

} // namespace
