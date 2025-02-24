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

#include "dawn/native/AHBFunctions.h"

namespace dawn::native {

namespace {

// Convert from AHardwareBuffer_Format to wgpu::TextureFormat. Returns:
//  - The exact corresponding wgpu::TextureFormat if the AHardwareBuffer format matches channel and
//  bit count.
//  - External if the format miss-matches in channel count or bit count but is still usable as an
//  OpenGL or Vulkan texture in Dawn.
//  - Undefined if the format is not applicable for WebGPU to import as a texture.
wgpu::TextureFormat FormatFromAHardwareBufferFormat(uint32_t ahbFormat) {
    switch (ahbFormat) {
        // Formats with exact WebGPU representations
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
            return wgpu::TextureFormat::RGBA8Unorm;
        case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
            return wgpu::TextureFormat::RGBA16Float;
        case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
            return wgpu::TextureFormat::RGB10A2Unorm;
        case AHARDWAREBUFFER_FORMAT_D16_UNORM:
            return wgpu::TextureFormat::Depth16Unorm;
        case AHARDWAREBUFFER_FORMAT_D24_UNORM:
            return wgpu::TextureFormat::Depth24Plus;
        case AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
            return wgpu::TextureFormat::Depth24PlusStencil8;
        case AHARDWAREBUFFER_FORMAT_D32_FLOAT:
            return wgpu::TextureFormat::Depth32Float;
        case AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT:
            return wgpu::TextureFormat::Depth32FloatStencil8;
        case AHARDWAREBUFFER_FORMAT_S8_UINT:
            return wgpu::TextureFormat::Stencil8;
        case AHARDWAREBUFFER_FORMAT_R8_UNORM:
            return wgpu::TextureFormat::R8Unorm;
        case AHARDWAREBUFFER_FORMAT_R16_UINT:
            return wgpu::TextureFormat::R16Uint;
        case AHARDWAREBUFFER_FORMAT_R16G16_UINT:
            return wgpu::TextureFormat::RG16Uint;

        // R8G8B8X8 is used in Vulkan as VK_FORMAT_R8G8B8A8_UNORM and GLES as GL_RGB8. In Vulkan the
        // alpha channel can be read and written but GL always treats it as opaque. Data can be
        // uploaded as if it's RGBA8.
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
            return wgpu::TextureFormat::RGBA8Unorm;

        // YUV formats are sampleable with external Vulkan samplers or as opaque samplers in GLES.
        // Treat them as External.
        case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
            return wgpu::TextureFormat::External;
        case AHARDWAREBUFFER_FORMAT_YCbCr_P010:
            return wgpu::TextureFormat::External;

        // RGB formats with no direct representation in WebGPU. Trivially sampleable and renderable
        // in Vulkan and GLES but data uploads are not currently supported.
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
            return wgpu::TextureFormat::External;
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
            return wgpu::TextureFormat::External;

        // R10G10B10A10 maps to Vulkan format VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 and no
        // GLES format. Not supported with no known use case.
        case AHARDWAREBUFFER_FORMAT_R10G10B10A10_UNORM:
            return wgpu::TextureFormat::Undefined;

        // Can be used as a Vulkan buffer or bound to a GLES buffer using GL_EXT_external_buffer.
        // These use cases are not currently supported.
        case AHARDWAREBUFFER_FORMAT_BLOB:
            return wgpu::TextureFormat::Undefined;

        // Android drivers make use of formats outside of the defined enums to represent internal
        // YUV formats used by the camera and video decoder. Treat these all as external and
        // sampleable.
        default:
            return wgpu::TextureFormat::External;
    }
}

}  // anonymous namespace

AHBFunctions::AHBFunctions() {
    if (!mNativeWindowLib.Open("libnativewindow.so") ||
        !mNativeWindowLib.GetProc(&Acquire, "AHardwareBuffer_acquire") ||
        !mNativeWindowLib.GetProc(&Release, "AHardwareBuffer_release") ||
        !mNativeWindowLib.GetProc(&Describe, "AHardwareBuffer_describe")) {
        mNativeWindowLib.Close();
    }
}

AHBFunctions::~AHBFunctions() = default;

bool AHBFunctions::IsValid() const {
    return mNativeWindowLib.Valid();
}

SharedTextureMemoryProperties GetAHBSharedTextureMemoryProperties(
    const AHBFunctions* ahbFunctions,
    ::AHardwareBuffer* aHardwareBuffer) {
    AHardwareBuffer_Desc aHardwareBufferDesc{};
    ahbFunctions->Describe(aHardwareBuffer, &aHardwareBufferDesc);

    SharedTextureMemoryProperties properties;
    properties.size = {aHardwareBufferDesc.width, aHardwareBufferDesc.height,
                       aHardwareBufferDesc.layers};
    properties.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    if (aHardwareBufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER) {
        properties.usage |= wgpu::TextureUsage::RenderAttachment;
    }
    if (aHardwareBufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE) {
        properties.usage |= wgpu::TextureUsage::TextureBinding;
    }
    if (aHardwareBufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER) {
        properties.usage |= wgpu::TextureUsage::StorageBinding;
    }
    properties.format = FormatFromAHardwareBufferFormat(aHardwareBufferDesc.format);

    return properties;
}

}  // namespace dawn::native
