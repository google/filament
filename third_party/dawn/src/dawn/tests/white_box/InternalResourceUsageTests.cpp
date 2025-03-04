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

#include "dawn/native/dawn_platform.h"

namespace dawn {
namespace {

class InternalResourceUsageTests : public DawnTest {
  protected:
    wgpu::Buffer CreateBuffer(wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = usage;

        return device.CreateBuffer(&descriptor);
    }
};

// Verify it is an error to create a buffer with a buffer usage that should only be used
// internally.
TEST_P(InternalResourceUsageTests, InternalBufferUsage) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    ASSERT_DEVICE_ERROR(CreateBuffer(native::kReadOnlyStorageBuffer));

    ASSERT_DEVICE_ERROR(CreateBuffer(native::kInternalStorageBuffer));
}

DAWN_INSTANTIATE_TEST(InternalResourceUsageTests, NullBackend());

class InternalBindingTypeTests : public DawnTest {};

// Verify it is an error to create a bind group layout with a buffer binding type that should only
// be used internally.
TEST_P(InternalBindingTypeTests, InternalStorageBufferBindingType) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::BindGroupLayoutEntry bglEntry;
    bglEntry.binding = 0;
    bglEntry.buffer.type = native::kInternalStorageBufferBinding;
    bglEntry.visibility = wgpu::ShaderStage::Compute;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = &bglEntry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&bglDesc));
}

// Verify it is an error to create a bind group layout with a texture sample type that should only
// be used internally.
TEST_P(InternalBindingTypeTests, ErrorUseInternalResolveAttachmentSampleTypeExternally) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::BindGroupLayoutEntry bglEntry;
    bglEntry.binding = 0;
    bglEntry.texture.sampleType = native::kInternalResolveAttachmentSampleType;
    bglEntry.visibility = wgpu::ShaderStage::Fragment;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = &bglEntry;
    ASSERT_DEVICE_ERROR_MSG(device.CreateBindGroupLayout(&bglDesc),
                            testing::HasSubstr("invalid for WGPUTextureSampleType"));
}

DAWN_INSTANTIATE_TEST(InternalBindingTypeTests, NullBackend());

}  // anonymous namespace
}  // namespace dawn
