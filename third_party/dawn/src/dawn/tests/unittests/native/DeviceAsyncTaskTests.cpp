// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <utility>

#include "dawn/native/AsyncTask.h"
#include "mocks/DawnMockTest.h"

namespace dawn::native {
namespace {

using ::testing::Test;

class DeviceAsyncTaskTests : public DawnMockTest {};

// Test that a long async task's execution won't extend to after the device is dropped.
// Device dropping should wait for that task to finish.
TEST_F(DeviceAsyncTaskTests, LongAsyncTaskFinishesBeforeDeviceIsDropped) {
    std::atomic_bool done(false);

    // Simulate that an async task would take a long time to finish.
    dawn::native::AsyncTask asyncTask([&done] {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        done = true;
    });

    mDeviceMock->GetAsyncTaskManager()->PostTask(std::move(asyncTask));
    DropDevice();
    // Dropping the device should force the async task to finish.
    EXPECT_TRUE(done.load());
}

}  // anonymous namespace
}  // namespace dawn::native
