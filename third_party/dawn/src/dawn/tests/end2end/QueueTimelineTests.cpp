// Copyright 2020 The Dawn & Tint Authors
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

#include <memory>

#include "dawn/tests/DawnTest.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/StringViewMatchers.h"
#include "gmock/gmock.h"

namespace dawn {
namespace {

using testing::_;
using testing::EmptySizedString;
using testing::InSequence;
using testing::MockCppCallback;

using MockMapAsyncCallback = MockCppCallback<wgpu::BufferMapCallback<void>*>;
using MockQueueWorkDoneCallback = MockCppCallback<wgpu::QueueWorkDoneCallback<void>*>;

class QueueTimelineTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapRead;
        mMapReadBuffer = device.CreateBuffer(&descriptor);
    }

    wgpu::Buffer mMapReadBuffer;
};

// Test that mMapReadBuffer.MapAsync callback happens before queue.OnWorkDone callback
// when queue.OnSubmittedWorkDone is called after mMapReadBuffer.MapAsync. The callback order should
// happen in the order the functions are called.
TEST_P(QueueTimelineTests, MapRead_OnWorkDone) {
    MockMapAsyncCallback mockMapAsyncCb;
    MockQueueWorkDoneCallback mockQueueWorkDoneCb;

    InSequence sequence;
    EXPECT_CALL(mockMapAsyncCb, Call(wgpu::MapAsyncStatus::Success, EmptySizedString())).Times(1);
    EXPECT_CALL(mockQueueWorkDoneCb, Call(wgpu::QueueWorkDoneStatus::Success, EmptySizedString()))
        .Times(1);

    mMapReadBuffer.MapAsync(wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
                            wgpu::CallbackMode::AllowProcessEvents, mockMapAsyncCb.Callback());
    queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowProcessEvents,
                              mockQueueWorkDoneCb.Callback());

    WaitForAllOperations();
    mMapReadBuffer.Unmap();
}

// Test that the OnSubmittedWorkDone callbacks should happen in the order the functions are called.
TEST_P(QueueTimelineTests, OnWorkDone_OnWorkDone) {
    MockQueueWorkDoneCallback mockQueueWorkDoneCb1;
    MockQueueWorkDoneCallback mockQueueWorkDoneCb2;

    InSequence sequence;
    EXPECT_CALL(mockQueueWorkDoneCb1, Call(wgpu::QueueWorkDoneStatus::Success, EmptySizedString()))
        .Times(1);
    EXPECT_CALL(mockQueueWorkDoneCb2, Call(wgpu::QueueWorkDoneStatus::Success, EmptySizedString()))
        .Times(1);

    queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowProcessEvents,
                              mockQueueWorkDoneCb1.Callback());
    queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowProcessEvents,
                              mockQueueWorkDoneCb2.Callback());

    WaitForAllOperations();
}

DAWN_INSTANTIATE_TEST(QueueTimelineTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
