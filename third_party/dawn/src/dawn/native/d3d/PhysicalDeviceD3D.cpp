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

#include "dawn/native/d3d/PhysicalDeviceD3D.h"

#include <string>
#include <utility>

#include "dawn/common/WindowsUtils.h"
#include "dawn/native/d3d/BackendD3D.h"

namespace dawn::native::d3d {

PhysicalDevice::PhysicalDevice(Backend* backend,
                               ComPtr<IDXGIAdapter4> hardwareAdapter,
                               wgpu::BackendType backendType)
    : PhysicalDeviceBase(backendType),
      mHardwareAdapter(std::move(hardwareAdapter)),
      mBackend(backend) {}

PhysicalDevice::~PhysicalDevice() = default;

IDXGIAdapter4* PhysicalDevice::GetHardwareAdapter() const {
    return mHardwareAdapter.Get();
}

Backend* PhysicalDevice::GetBackend() const {
    return mBackend;
}

ResultOrError<PhysicalDeviceSurfaceCapabilities> PhysicalDevice::GetSurfaceCapabilities(
    InstanceBase*,
    const Surface*) const {
    PhysicalDeviceSurfaceCapabilities capabilities;

    // StorageBinding is not supported by DXGI swapchains.
    capabilities.usages = wgpu::TextureUsage::RenderAttachment |
                          wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                          wgpu::TextureUsage::CopyDst;

    // The list of format allowed with the DXGI flip model.
    capabilities.formats = {
        wgpu::TextureFormat::BGRA8Unorm,
        wgpu::TextureFormat::RGBA8Unorm,
        wgpu::TextureFormat::RGBA16Float,
        wgpu::TextureFormat::RGB10A2Unorm,
    };

    capabilities.presentModes = {
        wgpu::PresentMode::Fifo,
        wgpu::PresentMode::Immediate,
        wgpu::PresentMode::Mailbox,
    };

    capabilities.alphaModes = {
        wgpu::CompositeAlphaMode::Opaque,
        wgpu::CompositeAlphaMode::Premultiplied,
    };

    return capabilities;
}

MaybeError PhysicalDevice::InitializeImpl() {
    DXGI_ADAPTER_DESC1 adapterDesc;
    GetHardwareAdapter()->GetDesc1(&adapterDesc);

    mDeviceId = adapterDesc.DeviceId;
    mVendorId = adapterDesc.VendorId;
    mName = WCharToUTF8(adapterDesc.Description);

    if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        mAdapterType = wgpu::AdapterType::CPU;
    } else {
        // Assume it is a discrete GPU. If it is an integrated GPU, it will be overwritten later.
        mAdapterType = wgpu::AdapterType::DiscreteGPU;
    }

    // Convert the adapter's D3D driver version to a readable string like "24.21.13.9793".
    LARGE_INTEGER umdVersion;
    if (GetHardwareAdapter()->CheckInterfaceSupport(__uuidof(IDXGIDevice), &umdVersion) !=
        DXGI_ERROR_UNSUPPORTED) {
        uint64_t encodedVersion = umdVersion.QuadPart;
        uint16_t mask = 0xFFFF;
        mDriverVersion = {static_cast<uint16_t>((encodedVersion >> 48) & mask),
                          static_cast<uint16_t>((encodedVersion >> 32) & mask),
                          static_cast<uint16_t>((encodedVersion >> 16) & mask),
                          static_cast<uint16_t>(encodedVersion & mask)};
        switch (GetBackendType()) {
            case wgpu::BackendType::D3D11:
                mDriverDescription =
                    std::string("D3D11 driver version ") + mDriverVersion.ToString();
                break;
            case wgpu::BackendType::D3D12:
                mDriverDescription =
                    std::string("D3D12 driver version ") + mDriverVersion.ToString();
                break;
            default:
                DAWN_UNREACHABLE();
        }
    }

    return {};
}

}  // namespace dawn::native::d3d
