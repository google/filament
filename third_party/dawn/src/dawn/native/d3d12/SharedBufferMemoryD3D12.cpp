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

#include "dawn/native/d3d12/SharedBufferMemoryD3D12.h"

#include <utility>

#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/SharedFenceD3D.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/native/d3d12/BufferD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/QueueD3D12.h"

namespace dawn::native::d3d12 {

SharedBufferMemory::SharedBufferMemory(Device* device,
                                       StringView label,
                                       SharedBufferMemoryProperties properties,
                                       ComPtr<ID3D12Resource> resource)
    : SharedBufferMemoryBase(device, label, properties), mResource(std::move(resource)) {}

void SharedBufferMemory::DestroyImpl() {
    ToBackend(GetDevice())->ReferenceUntilUnused(std::move(mResource));
}

// static
ResultOrError<Ref<SharedBufferMemory>> SharedBufferMemory::Create(
    Device* device,
    StringView label,
    const SharedBufferMemoryD3D12ResourceDescriptor* descriptor) {
    DAWN_INVALID_IF(!descriptor->resource, "D3D12 resource is missing.");

    ComPtr<ID3D12Resource> d3d12Resource = descriptor->resource;

    ComPtr<ID3D12Device> resourceDevice;
    d3d12Resource->GetDevice(__uuidof(resourceDevice), &resourceDevice);
    DAWN_INVALID_IF(resourceDevice.Get() != device->GetD3D12Device(),
                    "The D3D12 device of the resource and the D3D12 device of %s must be same.",
                    device);

    D3D12_RESOURCE_DESC desc = d3d12Resource->GetDesc();
    DAWN_INVALID_IF(desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER,
                    "Resource dimension (%d) was not Buffer", desc.Dimension);

    D3D12_HEAP_PROPERTIES heapProperties;
    D3D12_HEAP_FLAGS heapFlags;
    d3d12Resource->GetHeapProperties(&heapProperties, &heapFlags);

    wgpu::BufferUsage usages = wgpu::BufferUsage::None;

    switch (heapProperties.Type) {
        case D3D12_HEAP_TYPE_UPLOAD:
            usages |= wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
            break;
        case D3D12_HEAP_TYPE_READBACK:
            usages |= wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
            break;
        case D3D12_HEAP_TYPE_DEFAULT:
            usages |= wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst |
                      wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index |
                      wgpu::BufferUsage::Indirect | wgpu::BufferUsage::QueryResolve;
            if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
                usages |= wgpu::BufferUsage::Storage;
            }
            if (IsAligned(desc.Width, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)) {
                usages |= wgpu::BufferUsage::Uniform;
            }
            break;
        case D3D12_HEAP_TYPE_CUSTOM:
            return DAWN_VALIDATION_ERROR(
                "ID3D12Resources allocated on D3D12_HEAP_TYPE_CUSTOM heaps are not supported by "
                "SharedBufferMemory.");
        default:
            DAWN_UNREACHABLE();
    }
    SharedBufferMemoryProperties properties;
    properties.size = desc.Width;
    properties.usage = usages;

    auto result =
        AcquireRef(new SharedBufferMemory(device, label, properties, std::move(d3d12Resource)));
    result->Initialize();
    return result;
}

ResultOrError<Ref<BufferBase>> SharedBufferMemory::CreateBufferImpl(
    const UnpackedPtr<BufferDescriptor>& descriptor) {
    return Buffer::CreateFromSharedBufferMemory(this, descriptor);
}

ID3D12Resource* SharedBufferMemory::GetD3DResource() const {
    return mResource.Get();
}

MaybeError SharedBufferMemory::BeginAccessImpl(
    BufferBase* buffer,
    const UnpackedPtr<BeginAccessDescriptor>& descriptor) {
    DAWN_TRY(descriptor.ValidateSubset<>());
    for (size_t i = 0; i < descriptor->fenceCount; ++i) {
        SharedFenceBase* fence = descriptor->fences[i];

        SharedFenceExportInfo exportInfo;
        DAWN_TRY(fence->ExportInfo(&exportInfo));
        switch (exportInfo.type) {
            case wgpu::SharedFenceType::DXGISharedHandle:
                DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceDXGISharedHandle),
                                "Required feature (%s) is missing.",
                                wgpu::FeatureName::SharedFenceDXGISharedHandle);
                break;
            default:
                return DAWN_VALIDATION_ERROR("Unsupported fence type %s.", exportInfo.type);
        }
    }

    return {};
}

ResultOrError<FenceAndSignalValue> SharedBufferMemory::EndAccessImpl(
    BufferBase* buffer,
    ExecutionSerial lastUsageSerial,
    UnpackedPtr<EndAccessState>& state) {
    DAWN_TRY(state.ValidateSubset<>());
    DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceDXGISharedHandle),
                    "Required feature (%s) is missing.",
                    wgpu::FeatureName::SharedFenceDXGISharedHandle);

    Ref<d3d::SharedFence> sharedFence;
    DAWN_TRY_ASSIGN(sharedFence, ToBackend(GetDevice()->GetQueue())->GetOrCreateSharedFence());

    return FenceAndSignalValue{std::move(sharedFence), static_cast<uint64_t>(lastUsageSerial)};
}

}  // namespace dawn::native::d3d12
