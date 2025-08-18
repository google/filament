// Copyright 2024 The Dawn & Tint Authors
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
namespace dawn {
namespace {

class GLExplicitExtensionsTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // Without ANGLE we cannot have a GL context requesting extensions explicitly.
        DAWN_TEST_UNSUPPORTED_IF(!IsANGLE());
    }
};

TEST_P(GLExplicitExtensionsTests, Features) {
    // Make sure this toggle is inherited correctly during re-initialization
    EXPECT_EQ(HasToggleEnabled("gl_force_es_31_and_no_extensions"), true);

    // GL_KHR_texture_compression_astc_ldr
    EXPECT_EQ(device.HasFeature(wgpu::FeatureName::TextureCompressionASTC), false);
    // GL_KHR_texture_compression_astc_sliced_3d
    EXPECT_EQ(device.HasFeature(wgpu::FeatureName::TextureCompressionASTCSliced3D), false);
    // GL_AMD_gpu_shader_half_float
    EXPECT_EQ(device.HasFeature(wgpu::FeatureName::ShaderF16), false);
    // GL_EXT_float_blend
    EXPECT_EQ(device.HasFeature(wgpu::FeatureName::Float32Blendable), false);
}

TEST_P(GLExplicitExtensionsTests, Toggles) {
    // Make sure this toggle is inherited correctly during re-initialization
    EXPECT_EQ(HasToggleEnabled("gl_force_es_31_and_no_extensions"), true);

    // GL_EXT_read_format_bgra
    EXPECT_EQ(HasToggleEnabled("use_blit_for_bgra8unorm_texture_to_buffer_copy"), true);
}

DAWN_INSTANTIATE_TEST(GLExplicitExtensionsTests,
                      // Force GLES 3.1 context with fewest extensions requested.
                      OpenGLESBackend({"gl_force_es_31_and_no_extensions"}));

}  // anonymous namespace
}  // namespace dawn
