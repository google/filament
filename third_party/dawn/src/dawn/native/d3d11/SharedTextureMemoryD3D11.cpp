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

#include "dawn/native/d3d11/SharedTextureMemoryD3D11.h"

#include <utility>

#include "dawn/native/D3D11Backend.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/KeyedMutex.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/TextureD3D11.h"

namespace dawn::native::d3d11 {

namespace {

ResultOrError<SharedTextureMemoryProperties>
PropertiesFromD3D11Texture(Device* device, ID3D11Texture2D* d3d11Texture, bool isSharedWithHandle) {
    D3D11_TEXTURE2D_DESC desc;
    d3d11Texture->GetDesc(&desc);
    DAWN_INVALID_IF(isSharedWithHandle && desc.ArraySize != 1,
                    "Resource shared with HANDLE, the ArraySize (%d) was not 1", desc.ArraySize);
    DAWN_INVALID_IF(desc.MipLevels != 1, "Resource MipLevels (%d) was not 1", desc.MipLevels);
    DAWN_INVALID_IF(desc.SampleDesc.Count != 1, "Resource SampleDesc.Count (%d) was not 1",
                    desc.SampleDesc.Count);

    const CombinedLimits& limits = device->GetLimits();
    DAWN_INVALID_IF(desc.Width > limits.v1.maxTextureDimension2D,
                    "Resource Width (%u) exceeds maxTextureDimension2D (%u).", desc.Width,
                    limits.v1.maxTextureDimension2D);
    DAWN_INVALID_IF(desc.Height > limits.v1.maxTextureDimension2D,
                    "Resource Height (%u) exceeds maxTextureDimension2D (%u).", desc.Height,
                    limits.v1.maxTextureDimension2D);

    SharedTextureMemoryProperties properties;
    properties.size = {static_cast<uint32_t>(desc.Width), static_cast<uint32_t>(desc.Height),
                       desc.ArraySize};

    DAWN_TRY_ASSIGN(properties.format, d3d::FromUncompressedColorDXGITextureFormat(desc.Format));

    // The usages that the underlying D3D11 texture supports are partially
    // dependent on its creation flags. Note that the SharedTextureMemory
    // frontend takes care of stripping out any usages that are not supported
    // for `format`.
    wgpu::TextureUsage textureBindingUsage = (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
                                                 ? wgpu::TextureUsage::TextureBinding
                                                 : wgpu::TextureUsage::None;
    wgpu::TextureUsage storageBindingUsage = (desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
                                                 ? wgpu::TextureUsage::StorageBinding
                                                 : wgpu::TextureUsage::None;
    wgpu::TextureUsage renderAttachmentUsage = (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
                                                   ? wgpu::TextureUsage::RenderAttachment
                                                   : wgpu::TextureUsage::None;

    properties.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                       textureBindingUsage | storageBindingUsage | renderAttachmentUsage;

    return properties;
}

}  // namespace

// static
ResultOrError<Ref<SharedTextureMemory>> SharedTextureMemory::Create(
    Device* device,
    StringView label,
    const SharedTextureMemoryDXGISharedHandleDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->handle == nullptr, "shared HANDLE is missing.");

    ComPtr<ID3D11Resource> d3d11Resource;
    DAWN_TRY(CheckHRESULT(device->GetD3D11Device5()->OpenSharedResource1(
                              descriptor->handle, IID_PPV_ARGS(&d3d11Resource)),
                          "D3D11 open shared handle"));

    D3D11_RESOURCE_DIMENSION type;
    d3d11Resource->GetType(&type);
    DAWN_INVALID_IF(type != D3D11_RESOURCE_DIMENSION_TEXTURE2D,
                    "Resource type (%d) was not Texture2D", type);

    ComPtr<ID3D11Texture2D> d3d11Texture;
    DAWN_TRY(
        CheckHRESULT(d3d11Resource.As(&d3d11Texture), "Cannot get ID3D11Texture2D from texture"));

    SharedTextureMemoryProperties properties;
    DAWN_TRY_ASSIGN(properties, PropertiesFromD3D11Texture(device, d3d11Texture.Get(),
                                                           /*isSharedWithHandle=*/true));

    auto result =
        AcquireRef(new SharedTextureMemory(device, label, properties, std::move(d3d11Resource)));
    result->Initialize();
    return result;
}

// static
ResultOrError<Ref<SharedTextureMemory>> SharedTextureMemory::Create(
    Device* device,
    StringView label,
    const SharedTextureMemoryD3D11Texture2DDescriptor* descriptor) {
    DAWN_INVALID_IF(!descriptor->texture, "D3D11 texture is missing.");

    ComPtr<ID3D11Resource> d3d11Resource;
    DAWN_TRY(CheckHRESULT(descriptor->texture.As(&d3d11Resource),
                          "Cannot get ID3D11Resource from texture"));

    ComPtr<ID3D11Device> textureDevice;
    d3d11Resource->GetDevice(textureDevice.GetAddressOf());
    DAWN_INVALID_IF(textureDevice.Get() != device->GetD3D11Device(),
                    "The D3D11 device of the texture and the D3D11 device of %s must be same.",
                    device);

    SharedTextureMemoryProperties properties;
    DAWN_TRY_ASSIGN(properties, PropertiesFromD3D11Texture(device, descriptor->texture.Get(),
                                                           /*isSharedWithHandle=*/false));

    auto result =
        AcquireRef(new SharedTextureMemory(device, label, properties, std::move(d3d11Resource)));
    result->Initialize();
    return result;
}

SharedTextureMemory::SharedTextureMemory(Device* device,
                                         StringView label,
                                         SharedTextureMemoryProperties properties,
                                         ComPtr<ID3D11Resource> resource)
    : d3d::SharedTextureMemory(device, label, properties), mResource(std::move(resource)) {
    ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex;
    mResource.As(&dxgiKeyedMutex);
    if (dxgiKeyedMutex) {
        mKeyedMutex = AcquireRef(new d3d::KeyedMutex(device, std::move(dxgiKeyedMutex)));
    }
}

void SharedTextureMemory::DestroyImpl() {
    mKeyedMutex = nullptr;
    mResource = nullptr;
}

ID3D11Resource* SharedTextureMemory::GetD3DResource() const {
    return mResource.Get();
}

d3d::KeyedMutex* SharedTextureMemory::GetKeyedMutex() const {
    return mKeyedMutex.Get();
}

ResultOrError<Ref<TextureBase>> SharedTextureMemory::CreateTextureImpl(
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    return Texture::CreateFromSharedTextureMemory(this, descriptor);
}

}  // namespace dawn::native::d3d11
