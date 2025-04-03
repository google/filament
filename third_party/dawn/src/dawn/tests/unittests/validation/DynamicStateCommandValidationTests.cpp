// Copyright 2018 The Dawn & Tint Authors
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

#include <cmath>
#include <limits>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class SetViewportTest : public ValidationTest {
  protected:
    void TestViewportCall(bool success,
                          float x,
                          float y,
                          float width,
                          float height,
                          float minDepth,
                          float maxDepth) {
        utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, kWidth, kHeight);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetViewport(x, y, width, height, minDepth, maxDepth);
        pass.End();

        if (success) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    static constexpr uint32_t kWidth = 5;
    static constexpr uint32_t kHeight = 3;

    static constexpr int32_t kMaxViewportSize = 8192;  // maxTextureDimension2D default
    static constexpr int32_t kMaxViewportBounds = kMaxViewportSize * 2;
};

// Test to check basic use of SetViewport
TEST_F(SetViewportTest, Success) {
    TestViewportCall(true, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0);
}

// Test to check that NaN in viewport parameters is not allowed
TEST_F(SetViewportTest, ViewportParameterNaN) {
    TestViewportCall(false, NAN, 0.0, 1.0, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, NAN, 1.0, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, NAN, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, NAN, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, NAN, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 0.0, NAN);
}

// Test to check that Infinity in viewport parameters is not allowed
TEST_F(SetViewportTest, ViewportParameterInf) {
    TestViewportCall(false, INFINITY, 0.0, 1.0, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, INFINITY, 1.0, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, INFINITY, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, INFINITY, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, INFINITY, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 0.0, INFINITY);
}

// Test to check that an empty viewport is allowed.
TEST_F(SetViewportTest, EmptyViewport) {
    // Width of viewport is zero.
    TestViewportCall(true, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0);

    // Height of viewport is zero.
    TestViewportCall(true, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0);

    // Both width and height of viewport are zero.
    TestViewportCall(true, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
}

// Test to check that viewport larger than the framebuffer is allowed
TEST_F(SetViewportTest, ViewportLargerThanLimit) {
    // Control case: width and height are set to the render target size.
    TestViewportCall(true, 0.0, 0.0, kWidth, kHeight, 0.0, 1.0);

    // Width is larger than the rendertarget's width
    TestViewportCall(true, 0.0, 0.0, kWidth + 1.0, kHeight, 0.0, 1.0);
    TestViewportCall(true, 0.0, 0.0, nextafter(float{kWidth}, INFINITY), kHeight, 0.0, 1.0);

    // Height is larger than the rendertarget's height
    TestViewportCall(true, 0.0, 0.0, kWidth, kHeight + 1.0, 0.0, 1.0);
    TestViewportCall(true, 0.0, 0.0, kWidth, nextafter(float{kHeight}, INFINITY), 0.0, 1.0);

    // Width and Height are the max viewport size
    TestViewportCall(true, 0.0, 0.0, kMaxViewportSize, kHeight, 0.0, 1.0);
    TestViewportCall(true, 0.0, 0.0, kWidth, kMaxViewportSize, 0.0, 1.0);
    TestViewportCall(true, 0.0, 0.0, kMaxViewportSize, kMaxViewportSize, 0.0, 1.0);

    // Width is larger than the max viewport size
    TestViewportCall(false, 0.0, 0.0, kMaxViewportSize + 1.0, kMaxViewportSize, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, nextafter(float{kMaxViewportSize}, INFINITY),
                     kMaxViewportSize, 0.0, 1.0);

    // Height is larger than the max viewport size
    TestViewportCall(false, 0.0, 0.0, kMaxViewportSize, kMaxViewportSize + 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, kMaxViewportSize,
                     nextafter(float{kMaxViewportSize}, INFINITY), 0.0, 1.0);

    // x + width is larger than the max viewport bounds
    TestViewportCall(false, kMaxViewportSize + nextafter(float{kMaxViewportSize}, INFINITY), 0.0,
                     kMaxViewportSize - 1.0, kMaxViewportSize, 0.0, 1.0);
    TestViewportCall(false, kMaxViewportSize + 1.0, 0.0,
                     nextafter(float{kMaxViewportSize - 1.0}, INFINITY), kMaxViewportSize, 0.0,
                     1.0);

    // y + height is larger than the max viewport bounds
    TestViewportCall(false, 0.0, kMaxViewportSize + nextafter(float{kMaxViewportSize}, INFINITY),
                     kMaxViewportSize, kMaxViewportSize - 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, kMaxViewportSize + 1.0, kMaxViewportSize,
                     nextafter(float{kMaxViewportSize - 1.0}, INFINITY), 0.0, 1.0);
}

// Test to check that negative x in viewport is allowed within the bounds
TEST_F(SetViewportTest, NegativeXYWidthHeight) {
    // Control case: everything set to 0 is allowed.
    TestViewportCall(true, +0.0, +0.0, +0.0, +0.0, 0.0, 1.0);
    TestViewportCall(true, -0.0, -0.0, -0.0, -0.0, 0.0, 1.0);

    // Negative offsets are allowed up to the minimum viewport bounds
    TestViewportCall(true, -1.0, 0.0, 1.0, 1.0, 0.0, 1.0);
    TestViewportCall(true, 0.0, -1.0, 1.0, 1.0, 0.0, 1.0);

    TestViewportCall(true, -kMaxViewportBounds, 0.0, 1.0, 1.0, 0.0, 1.0);
    TestViewportCall(true, 0.0, -kMaxViewportBounds, 1.0, 1.0, 0.0, 1.0);

    TestViewportCall(false, -kMaxViewportBounds - 1.0, 0.0, 1.0, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, -kMaxViewportBounds - 1.0, 1.0, 1.0, 0.0, 1.0);

    // Negative width and height is disallowed.
    TestViewportCall(false, 0.0, 0.0, -1.0, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, -1.0, 0.0, 1.0);
}

// Test to check that minDepth out of range [0, 1] is disallowed
TEST_F(SetViewportTest, MinDepthOutOfRange) {
    // MinDepth is -1
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, -1.0, 1.0);

    // MinDepth is 2 or 1 + epsilon
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 2.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, nextafter(1.0f, INFINITY), 1.0);
}

// Test to check that minDepth out of range [0, 1] is disallowed
TEST_F(SetViewportTest, MaxDepthOutOfRange) {
    // MaxDepth is -1
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 1.0, -1.0);

    // MaxDepth is 2 or 1 + epsilon
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 1.0, 2.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 1.0, nextafter(1.0f, INFINITY));
}

// Test to check that minDepth equal or greater than maxDepth is disallowed
TEST_F(SetViewportTest, MinDepthEqualOrGreaterThanMaxDepth) {
    TestViewportCall(true, 0.0, 0.0, 1.0, 1.0, 0.5, 0.5);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 0.8, 0.5);
}

class SetScissorTest : public ValidationTest {
  protected:
    void TestScissorCall(bool success, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, kWidth, kHeight);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetScissorRect(x, y, width, height);
        pass.End();

        if (success) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    static constexpr uint32_t kWidth = 5;
    static constexpr uint32_t kHeight = 3;
};

// Test to check basic use of SetScissor
TEST_F(SetScissorTest, Success) {
    TestScissorCall(true, 0, 0, kWidth, kHeight);
    TestScissorCall(true, 0, 0, 1, 1);
}

// Test to check that an empty scissor is allowed
TEST_F(SetScissorTest, EmptyScissor) {
    // Scissor width is 0
    TestScissorCall(true, 0, 0, 0, kHeight);

    // Scissor height is 0
    TestScissorCall(true, 0, 0, kWidth, 0);

    // Both scissor width and height are 0
    TestScissorCall(true, 0, 0, 0, 0);
}

// Test to check that various scissors contained in the framebuffer is allowed
TEST_F(SetScissorTest, ScissorContainedInFramebuffer) {
    // Width and height are set to the render target size.
    TestScissorCall(true, 0, 0, kWidth, kHeight);

    // Width/height at the limit with 0 x/y is valid.
    TestScissorCall(true, kWidth, 0, 0, kHeight);
    TestScissorCall(true, 0, kHeight, kWidth, 0);
}

// Test to check that a scissor larger than the framebuffer is disallowed
TEST_F(SetScissorTest, ScissorLargerThanFramebuffer) {
    // Width/height is larger than the rendertarget's width/height.
    TestScissorCall(false, 0, 0, kWidth + 1, kHeight);
    TestScissorCall(false, 0, 0, kWidth, kHeight + 1);

    // x + width is larger than the rendertarget's width.
    TestScissorCall(false, 2, 0, kWidth - 1, kHeight);
    TestScissorCall(false, kWidth, 0, 1, kHeight);
    TestScissorCall(false, std::numeric_limits<uint32_t>::max(), 0, kWidth, kHeight);

    // x + height is larger than the rendertarget's height.
    TestScissorCall(false, 0, 2, kWidth, kHeight - 1);
    TestScissorCall(false, 0, kHeight, kWidth, 1);
    TestScissorCall(false, 0, std::numeric_limits<uint32_t>::max(), kWidth, kHeight);
}

class SetBlendConstantTest : public ValidationTest {
  protected:
    void TestBlendConstantCall(bool success, const wgpu::Color color) {
        PlaceholderRenderPass renderPass(device);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetBlendConstant(&color);
            pass.End();
        }

        if (success) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
};

// Test to check basic use of SetBlendConstantTest
TEST_F(SetBlendConstantTest, Success) {
    constexpr wgpu::Color kTransparentBlack{0.0f, 0.0f, 0.0f, 0.0f};
    constexpr wgpu::Color kAnyColorValue{-1.0f, 42.0f, -0.0f, 0.0f};

    TestBlendConstantCall(true, kTransparentBlack);
    TestBlendConstantCall(true, kAnyColorValue);
}

// Test that SetBlendConstant does not allow NaN.
TEST_F(SetBlendConstantTest, NaN) {
    TestBlendConstantCall(false, {NAN, 0.0f, 0.0f, 0.0f});
    TestBlendConstantCall(false, {0.0f, NAN, 0.0f, 0.0f});
    TestBlendConstantCall(false, {0.0f, 0.0f, NAN, 0.0f});
    TestBlendConstantCall(false, {0.0f, 0.0f, 0.0f, NAN});
}

// Test that SetBlendConstant does not allow Infinity.
TEST_F(SetBlendConstantTest, Infinity) {
    TestBlendConstantCall(false, {INFINITY, 0.0f, 0.0f, 0.0f});
    TestBlendConstantCall(false, {0.0f, INFINITY, 0.0f, 0.0f});
    TestBlendConstantCall(false, {0.0f, 0.0f, INFINITY, 0.0f});
    TestBlendConstantCall(false, {0.0f, 0.0f, 0.0f, INFINITY});
}

class SetStencilReferenceTest : public ValidationTest {};

// Test to check basic use of SetStencilReferenceTest
TEST_F(SetStencilReferenceTest, Success) {
    PlaceholderRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetStencilReference(0);
        pass.End();
    }
    encoder.Finish();
}

// Test that SetStencilReference allows any bit to be set
TEST_F(SetStencilReferenceTest, AllBitsAllowed) {
    PlaceholderRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetStencilReference(0xFFFFFFFF);
        pass.End();
    }
    encoder.Finish();
}

}  // anonymous namespace
}  // namespace dawn
