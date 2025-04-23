// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/BlitBufferToTexture.h"

#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderPassEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

namespace {

constexpr std::string_view kShaderCommonSrc = R"(
@vertex fn vert_fullscreen_quad(
  @builtin(vertex_index) vertex_index : u32
) -> @builtin(position) vec4f {
  const pos = array(
      vec2f(-1.0, -1.0),
      vec2f( 3.0, -1.0),
      vec2f(-1.0,  3.0));
  return vec4f(pos[vertex_index], 0.0, 1.0);
}

struct Params {
  srcOffset : u32,
  bytesPerRow : u32,
  dstOrigin : vec2u
};

@group(0) @binding(0) var<storage, read> src_buf : array<u32>;
@group(0) @binding(1) var<uniform> params : Params;

fn loadU8AsU32(byteOffset: u32) -> u32 {
    let uintOffset = byteOffset >> 2;
    let uintModOffset = byteOffset & 3;
    let bitShift = uintModOffset * 8;
    return (src_buf[uintOffset] >> bitShift) & 0xff;
}

fn loadU16AsU32(byteOffset: u32) -> u32 {
    let firstHalf = loadU8AsU32(byteOffset);
    let secondHalf = loadU8AsU32(byteOffset + 1);
    return firstHalf | (secondHalf << 8);
}

// byteOffset is expected to be aligned to 4.
fn loadU32(byteOffset: u32) -> u32 {
    let uintOffset = byteOffset >> 2;
    return src_buf[uintOffset];
}

// byteOffset is expected to be aligned to 4.
fn loadTwoU32s(byteOffset: u32) -> vec2u {
    return vec2u(loadU32(byteOffset), loadU32(byteOffset + 4));
}

@fragment fn blit_buffer_to_texture(
    @builtin(position) screen_position : vec4f
) -> @location(0) vec4f {
    let iposition = vec2u(screen_position.xy) - params.dstOrigin;

    let srcOffset = params.srcOffset + iposition.x * kPixelSize + iposition.y * params.bytesPerRow;

    return unpackData(srcOffset);
}
)";

constexpr std::string_view kUnpackR8Unorm = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    return unpack4x8unorm(loadU8AsU32(byteOffset));
}
)";

constexpr std::string_view kUnpackRG8Unorm = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    return unpack4x8unorm(loadU16AsU32(byteOffset));
}
)";

constexpr std::string_view kUnpackRGBA8Unorm = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    return unpack4x8unorm(loadU32(byteOffset));
}
)";

constexpr std::string_view kUnpackBGRA8Unorm = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    return unpack4x8unorm(loadU32(byteOffset)).bgra;
}
)";

constexpr std::string_view kUnpackRGB10A2Unorm = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    let data = loadU32(byteOffset);
    let r = f32((data & 0x3ff)) / 1023.0;
    let g = f32(((data >> 10) & 0x3ff)) / 1023.0;
    let b = f32(((data >> 20) & 0x3ff)) / 1023.0;
    let a = f32(((data >> 30) & 0x3)) / 3.0;
    return vec4f(r, g, b, a);
}
)";

constexpr std::string_view kUnpackR16Float = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    return vec4f(unpack2x16float(loadU16AsU32(byteOffset)), 0.0, 1.0);
}
)";

constexpr std::string_view kUnpackR16Unorm = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    return vec4f(f32(loadU16AsU32(byteOffset)) / f32(0xffff), 0.0, 0.0, 1.0);
}
)";

constexpr std::string_view kUnpackRG16Float = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    return vec4f(unpack2x16float(loadU32(byteOffset)), 0.0, 1.0);
}
)";

constexpr std::string_view kUnpackRG16Unorm = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    let word = loadU32(byteOffset);
    let x = f32(word & 0xffff);
    let y = f32(word >> 16);
    return vec4f(vec2f(x, y) / f32(0xffff), 0.0, 1.0);
}
)";

constexpr std::string_view kUnpackRGBA16Float = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    let data = loadTwoU32s(byteOffset);
    return vec4f(unpack2x16float(data.x), unpack2x16float(data.y));
}
)";

constexpr std::string_view kUnpackRGBA16Unorm = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    let words = loadTwoU32s(byteOffset);
    let x = f32(words[0] & 0xffff);
    let y = f32(words[0] >> 16);
    let z = f32(words[1] & 0xffff);
    let w = f32(words[1] >> 16);
    return vec4f(x, y, z, w) / f32(0xffff);
}
)";

constexpr std::string_view kUnpackR32Float = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    return vec4f(bitcast<f32>(loadU32(byteOffset)), 0.0, 0.0, 1.0);
}
)";

constexpr std::string_view kUnpackRG32Float = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    let color = bitcast<vec2f>(loadTwoU32s(byteOffset));
    return vec4f(color, 0.0, 1.0);
}
)";

constexpr std::string_view kUnpackRGBA32Float = R"(
fn unpackData(byteOffset: u32) -> vec4f {
    return vec4f(bitcast<vec2f>(loadTwoU32s(byteOffset)),
                 bitcast<vec2f>(loadTwoU32s(byteOffset + 8)));
}
)";

std::string GenerateShaderSource(wgpu::TextureFormat format) {
    int pixelSize = 0;
    std::ostringstream ss;
    switch (format) {
        case wgpu::TextureFormat::R8Unorm:
            pixelSize = 1;
            ss << kUnpackR8Unorm;
            break;
        case wgpu::TextureFormat::RG8Unorm:
            pixelSize = 2;
            ss << kUnpackRG8Unorm;
            break;
        case wgpu::TextureFormat::RGBA8Unorm:
            pixelSize = 4;
            ss << kUnpackRGBA8Unorm;
            break;
        case wgpu::TextureFormat::BGRA8Unorm:
            pixelSize = 4;
            ss << kUnpackBGRA8Unorm;
            break;
        case wgpu::TextureFormat::RGB10A2Unorm:
            pixelSize = 4;
            ss << kUnpackRGB10A2Unorm;
            break;
        case wgpu::TextureFormat::R16Float:
            pixelSize = 2;
            ss << kUnpackR16Float;
            break;
        case wgpu::TextureFormat::R16Unorm:
            pixelSize = 2;
            ss << kUnpackR16Unorm;
            break;
        case wgpu::TextureFormat::RG16Float:
            pixelSize = 4;
            ss << kUnpackRG16Float;
            break;
        case wgpu::TextureFormat::RG16Unorm:
            pixelSize = 4;
            ss << kUnpackRG16Unorm;
            break;
        case wgpu::TextureFormat::RGBA16Float:
            pixelSize = 8;
            ss << kUnpackRGBA16Float;
            break;
        case wgpu::TextureFormat::RGBA16Unorm:
            pixelSize = 8;
            ss << kUnpackRGBA16Unorm;
            break;
        case wgpu::TextureFormat::R32Float:
            pixelSize = 4;
            ss << kUnpackR32Float;
            break;
        case wgpu::TextureFormat::RG32Float:
            pixelSize = 8;
            ss << kUnpackRG32Float;
            break;
        case wgpu::TextureFormat::RGBA32Float:
            pixelSize = 16;
            ss << kUnpackRGBA32Float;
            break;
        default:
            DAWN_UNREACHABLE();
    }

    ss << "const kPixelSize = " << pixelSize << ";\n";
    ss << kShaderCommonSrc;

    return ss.str();
}

ResultOrError<Ref<RenderPipelineBase>> GetOrCreatePipeline(DeviceBase* device,
                                                           wgpu::TextureFormat format) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();
    {
        auto it = store->blitBufferToTexturePipelines.find(format);
        if (it != store->blitBufferToTexturePipelines.end()) {
            return it->second;
        }
    }

    // vertex shader's source.
    ShaderSourceWGSL wgslDesc = {};
    ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;

    // shader's source will depend on format key.
    std::string shaderCode = GenerateShaderSource(format);
    wgslDesc.code = shaderCode.c_str();
    Ref<ShaderModuleBase> shaderModule;
    DAWN_TRY_ASSIGN(shaderModule, device->CreateShaderModule(&shaderModuleDesc));

    FragmentState fragmentState = {};
    fragmentState.module = shaderModule.Get();
    fragmentState.entryPoint = "blit_buffer_to_texture";

    // Color target states.
    ColorTargetState colorTarget = {};
    colorTarget.format = format;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    RenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.label = "blit_buffer_to_texture";
    renderPipelineDesc.vertex.module = shaderModule.Get();
    renderPipelineDesc.vertex.entryPoint = "vert_fullscreen_quad";
    renderPipelineDesc.fragment = &fragmentState;

    // Bind group layout.
    Ref<BindGroupLayoutBase> bindGroupLayout;
    DAWN_TRY_ASSIGN(bindGroupLayout,
                    utils::MakeBindGroupLayout(
                        device,
                        {
                            {0, wgpu::ShaderStage::Fragment, kInternalReadOnlyStorageBufferBinding},
                            {1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform},
                        },
                        /* allowInternalBinding */ true));

    Ref<PipelineLayoutBase> pipelineLayout;
    DAWN_TRY_ASSIGN(pipelineLayout, utils::MakeBasicPipelineLayout(device, bindGroupLayout));
    renderPipelineDesc.layout = pipelineLayout.Get();

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, device->CreateRenderPipeline(&renderPipelineDesc));

    store->blitBufferToTexturePipelines.emplace(format, pipeline);
    return pipeline;
}

}  // anonymous namespace

bool IsFormatSupportedByBufferToTextureBlit(wgpu::TextureFormat format) {
    // TODO(348653642): Eventually we should support all non-compressed formats. For now, just list
    // a subset of them that we support.
    switch (format) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RGBA32Float:
            return true;
        default:
            return false;
    }
}

bool IsBufferToTextureBlitSupported(BufferBase* buffer,
                                    const TextureCopy& dst,
                                    const Extent3D& copyExtent) {
    if (!(buffer->GetInternalUsage() &
          (kReadOnlyStorageBuffer | kInternalStorageBuffer | wgpu::BufferUsage::Storage))) {
        return false;
    }

    if (!IsFormatSupportedByBufferToTextureBlit(dst.texture->GetFormat().format)) {
        return false;
    }

    if (dst.texture->GetDimension() == wgpu::TextureDimension::e1D) {
        // 1D texture cannot be rendered to so skip it.
        return false;
    }

    if (dst.aspect != Aspect::Color) {
        // Don't support multiplanar copies yet.
        return false;
    }

    // Must have non-zero copy size.
    return copyExtent.width * copyExtent.height * copyExtent.depthOrArrayLayers > 0;
}

MaybeError BlitBufferToTexture(DeviceBase* device,
                               CommandEncoder* commandEncoder,
                               BufferBase* buffer,
                               const TexelCopyBufferLayout& src,
                               const TextureCopy& dst,
                               const Extent3D& copyExtent) {
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());

    // This function assumes bytesPerRow is multiples of 4. Normally it's required that
    // bytesPerRow is aligned to 256. However some backends might enable
    // DawnTexelCopyBufferRowAlignment feature to relax the alignment. Currently only D3D11 backend
    // enables this feature, and the relaxed alignment there is 4.
    DAWN_ASSERT((src.bytesPerRow % 4) == 0);

    DAWN_ASSERT(buffer->GetInternalUsage() &
                (kReadOnlyStorageBuffer | kInternalStorageBuffer | wgpu::BufferUsage::Storage));

    DAWN_ASSERT(copyExtent.width > 0 && copyExtent.height > 0 && copyExtent.depthOrArrayLayers > 0);

    // Allow internal usages since we need to use the destination
    // as a render attachment.
    auto scope = commandEncoder->MakeInternalUsageScope();

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, GetOrCreatePipeline(device, dst.texture->GetFormat().format));

    Ref<BindGroupLayoutBase> bgl;
    DAWN_TRY_ASSIGN(bgl, pipeline->GetBindGroupLayout(0));

    const auto ssboAlignment = device->GetLimits().v1.minStorageBufferOffsetAlignment;
    DAWN_ASSERT(IsPowerOfTwo(ssboAlignment));

    wgpu::TextureViewDimension viewDimension;
    uint32_t baseDepth = 0;
    uint32_t baseArray = 0;
    uint32_t depthStep = 0;
    uint32_t arrayStep = 0;
    switch (dst.texture->GetDimension()) {
        case wgpu::TextureDimension::e1D:
            DAWN_UNREACHABLE();
            break;
        case wgpu::TextureDimension::e3D:
            viewDimension = wgpu::TextureViewDimension::e3D;
            baseDepth = dst.origin.z;
            depthStep = 1;
            break;
        default:
            viewDimension = wgpu::TextureViewDimension::e2D;
            baseArray = dst.origin.z;
            arrayStep = 1;
            break;
    }

    for (uint32_t z = 0; z < copyExtent.depthOrArrayLayers; ++z) {
        Ref<TextureViewBase> dstView;
        {
            TextureViewDescriptor viewDesc = {};
            viewDesc.dimension = viewDimension;
            viewDesc.baseArrayLayer = baseArray + arrayStep * z;
            viewDesc.arrayLayerCount = 1;
            viewDesc.baseMipLevel = dst.mipLevel;
            viewDesc.mipLevelCount = 1;
            DAWN_TRY_ASSIGN(dstView, dst.texture->CreateView(&viewDesc));
        }

        const uint64_t srcOffset = src.offset + z * src.rowsPerImage * src.bytesPerRow;
        const uint64_t srcBufferBindingOffset = AlignDown(srcOffset, ssboAlignment);
        const uint32_t shaderReadOffset = static_cast<uint32_t>(srcOffset & (ssboAlignment - 1));
        Ref<BufferBase> paramsBuffer;
        {
            DAWN_TRY_ASSIGN(paramsBuffer,
                            device->GetOrCreateTemporaryUniformBuffer(sizeof(uint32_t) * 4));

            uint32_t params[4];
            params[0] = shaderReadOffset;
            params[1] = src.bytesPerRow;
            params[2] = dst.origin.x;
            params[3] = dst.origin.y;
            commandEncoder->APIWriteBuffer(paramsBuffer.Get(), 0,
                                           reinterpret_cast<const uint8_t*>(&params[0]),
                                           sizeof(params));
        }

        Ref<BindGroupBase> bindGroup;
        DAWN_TRY_ASSIGN(bindGroup, utils::MakeBindGroup(device, bgl,
                                                        {
                                                            {0, buffer, srcBufferBindingOffset},
                                                            {1, paramsBuffer},
                                                        },
                                                        UsageValidationMode::Internal));

        RenderPassColorAttachment colorAttachment;
        colorAttachment.view = dstView.Get();
        if (depthStep) {
            colorAttachment.depthSlice = baseDepth + depthStep * z;
        }
        colorAttachment.loadOp = wgpu::LoadOp::Load;
        colorAttachment.storeOp = wgpu::StoreOp::Store;

        RenderPassDescriptor rpDesc = {};
        rpDesc.colorAttachmentCount = 1;
        rpDesc.colorAttachments = &colorAttachment;

        Ref<RenderPassEncoder> pass = commandEncoder->BeginRenderPass(&rpDesc);
        // Bind the resources.
        pass->APISetBindGroup(0, bindGroup.Get());
        pass->APISetViewport(dst.origin.x, dst.origin.y, copyExtent.width, copyExtent.height, 0.f,
                             1.f);

        // Draw to perform the blit.
        pass->APISetPipeline(pipeline.Get());
        pass->APIDraw(3, 1, 0, 0);

        pass->End();
    }
    return {};
}

}  // namespace dawn::native
