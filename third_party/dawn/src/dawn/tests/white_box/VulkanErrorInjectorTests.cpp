// Copyright 2019 The Dawn & Tint Authors
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
#include "partition_alloc/pointers/raw_ptr.h"

#include "dawn/common/Math.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/VulkanBackend.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {
namespace {

class VulkanErrorInjectorTests : public DawnTest {
  public:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

        mDeviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));
    }

  protected:
    raw_ptr<native::vulkan::Device> mDeviceVk;
};

TEST_P(VulkanErrorInjectorTests, InjectErrorOnCreateBuffer) {
    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.size = 16;
    createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    // Check that making a buffer works.
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        EXPECT_EQ(
            mDeviceVk->fn.CreateBuffer(mDeviceVk->GetVkDevice(), &createInfo, nullptr, &*buffer),
            VK_SUCCESS);
        mDeviceVk->fn.DestroyBuffer(mDeviceVk->GetVkDevice(), buffer, nullptr);
    }

    auto CreateTestBuffer = [&]() -> bool {
        VkBuffer buffer = VK_NULL_HANDLE;
        MaybeError err = CheckVkSuccess(
            mDeviceVk->fn.CreateBuffer(mDeviceVk->GetVkDevice(), &createInfo, nullptr, &*buffer),
            "vkCreateBuffer");
        if (err.IsError()) {
            // The handle should never be written to, even for mock failures.
            EXPECT_EQ(buffer, VK_NULL_HANDLE);
            err.AcquireError();
            return false;
        }
        EXPECT_NE(buffer, VK_NULL_HANDLE);

        // We never use the buffer, only test mocking errors on creation. Cleanup now.
        mDeviceVk->fn.DestroyBuffer(mDeviceVk->GetVkDevice(), buffer, nullptr);

        return true;
    };

    // Check that making a buffer inside CheckVkSuccess works.
    {
        EXPECT_TRUE(CreateTestBuffer());

        // The error injector call count should be empty
        EXPECT_EQ(AcquireErrorInjectorCallCount(), 0u);
    }

    // Test error injection works.
    EnableErrorInjector();
    {
        EXPECT_TRUE(CreateTestBuffer());
        EXPECT_TRUE(CreateTestBuffer());

        // The error injector call count should be two.
        EXPECT_EQ(AcquireErrorInjectorCallCount(), 2u);

        // Inject an error at index 0. The first should fail, the second succeed.
        {
            InjectErrorAt(0u);
            EXPECT_FALSE(CreateTestBuffer());
            EXPECT_TRUE(CreateTestBuffer());

            ClearErrorInjector();
        }

        // Inject an error at index 1. The second should fail, the first succeed.
        {
            InjectErrorAt(1u);
            EXPECT_TRUE(CreateTestBuffer());
            EXPECT_FALSE(CreateTestBuffer());

            ClearErrorInjector();
        }

        // Inject an error and then clear the injector. Calls should be successful.
        {
            InjectErrorAt(0u);
            DisableErrorInjector();

            EXPECT_TRUE(CreateTestBuffer());
            EXPECT_TRUE(CreateTestBuffer());

            ClearErrorInjector();
        }
    }
}

DAWN_INSTANTIATE_TEST(VulkanErrorInjectorTests, VulkanBackend());

}  // anonymous namespace
}  // namespace dawn::native::vulkan
