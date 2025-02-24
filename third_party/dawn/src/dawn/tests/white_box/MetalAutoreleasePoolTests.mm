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

#include "dawn/tests/DawnTest.h"

#include "dawn/native/Device.h"
#include "dawn/native/metal/Forward.h"
#include "dawn/native/metal/QueueMTL.h"

namespace dawn::native {
namespace {

using namespace metal;

class MetalAutoreleasePoolTests : public DawnTest {
  private:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

        mMtlQueue = ToBackend(FromAPI(device.Get())->GetQueue());
    }

  protected:
    Queue* mMtlQueue = nullptr;
};

// Test that the MTLCommandBuffer owned by the pending command context can
// outlive an autoreleasepool block.
TEST_P(MetalAutoreleasePoolTests, CommandBufferOutlivesAutorelease) {
    @autoreleasepool {
        // Get the recording context which will allocate a MTLCommandBuffer.
        // It will get autoreleased at the end of this block.
        mMtlQueue->GetPendingCommandContext();
    }

    // Submitting the command buffer should succeed.
    ASSERT_TRUE(mMtlQueue->SubmitPendingCommandBuffer().IsSuccess());
}

// Test that the MTLBlitCommandEncoder owned by the pending command context
// can outlive an autoreleasepool block.
TEST_P(MetalAutoreleasePoolTests, EncoderOutlivesAutorelease) {
    @autoreleasepool {
        // Get the recording context which will allocate a MTLCommandBuffer.
        // Begin a blit encoder.
        // Both will get autoreleased at the end of this block.
        mMtlQueue->GetPendingCommandContext()->EnsureBlit();
    }

    // Submitting the command buffer should succeed.
    mMtlQueue->GetPendingCommandContext()->EndBlit();
    ASSERT_TRUE(mMtlQueue->SubmitPendingCommandBuffer().IsSuccess());
}

DAWN_INSTANTIATE_TEST(MetalAutoreleasePoolTests, MetalBackend());

}  // anonymous namespace
}  // namespace dawn::native
