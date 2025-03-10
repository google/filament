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

#include <d3d12.h>
#include <vector>
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/tests/white_box/SharedBufferMemoryTests.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {
constexpr uint32_t kBufferSize = 4;

struct FenceInfo {
    ComPtr<ID3D12Fence> fence;
    uint64_t signaledValue;
};

void WriteD3D12UploadBuffer(ID3D12Resource* resource, uint32_t data) {
    void* mappedBufferBegin;
    D3D12_RANGE range;
    range.Begin = 0;
    range.End = kBufferSize;
    resource->Map(0, &range, &mappedBufferBegin);
    memcpy(mappedBufferBegin, &data, kBufferSize);
    resource->Unmap(0, &range);
}

void CopyD3D12Resource(ID3D12Device* device, ID3D12Resource* source, ID3D12Resource* destination) {
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    ComPtr<ID3D12CommandQueue> commandQueue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    ComPtr<ID3D12GraphicsCommandList> commandList;

    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
                              IID_PPV_ARGS(&commandList));

    ID3D12CommandList* commandLists[] = {commandList.Get()};
    commandList->CopyResource(destination, source);
    commandList->Close();

    commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    ComPtr<ID3D12Fence> fence;
    device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&fence));
    UINT64 signaledValue = 1;
    commandQueue->Signal(fence.Get(), signaledValue);

    HANDLE fenceEvent = 0;
    if (fence->GetCompletedValue() < signaledValue) {
        fence->SetEventOnCompletion(signaledValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
}

class Backend : public SharedBufferMemoryTestBackend {
  public:
    static Backend* GetInstance() {
        static Backend b;
        return &b;
    }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter& adapter) const override {
        return {wgpu::FeatureName::SharedBufferMemoryD3D12Resource,
                wgpu::FeatureName::SharedFenceDXGISharedHandle};
    }

    wgpu::SharedBufferMemory CreateSharedBufferMemory(const wgpu::Device& device,
                                                      wgpu::BufferUsage usages,
                                                      uint32_t bufferSize,
                                                      uint32_t initializationData = 0) override {
        ComPtr<ID3D12Device> d3d12Device = CreateD3D12Device(device);

        D3D12_HEAP_TYPE d3d12HeapType;

        if (usages & wgpu::BufferUsage::MapWrite) {
            d3d12HeapType = D3D12_HEAP_TYPE_UPLOAD;
        } else if (usages & wgpu::BufferUsage::MapRead) {
            d3d12HeapType = D3D12_HEAP_TYPE_READBACK;
        } else {
            d3d12HeapType = D3D12_HEAP_TYPE_DEFAULT;
        }

        // To use a buffer with CreateConstantBufferView, it must be aligned to a constant.
        if (usages & wgpu::BufferUsage::Uniform) {
            bufferSize = Align(bufferSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        }

        ComPtr<ID3D12Resource> d3d12Resource =
            CreateD3D12Buffer(d3d12Device.Get(), d3d12HeapType, bufferSize);

        if (initializationData) {
            switch (d3d12HeapType) {
                case D3D12_HEAP_TYPE_UPLOAD:
                    WriteD3D12UploadBuffer(d3d12Resource.Get(), initializationData);
                    break;
                case D3D12_HEAP_TYPE_READBACK:
                case D3D12_HEAP_TYPE_DEFAULT: {
                    ComPtr<ID3D12Resource> uploadBuffer =
                        CreateD3D12Buffer(d3d12Device.Get(), D3D12_HEAP_TYPE_UPLOAD, bufferSize);
                    WriteD3D12UploadBuffer(uploadBuffer.Get(), initializationData);
                    CopyD3D12Resource(d3d12Device.Get(), uploadBuffer.Get(), d3d12Resource.Get());
                } break;
                default:
                    DAWN_UNREACHABLE();
            }
        }

        wgpu::SharedBufferMemoryDescriptor desc;
        native::d3d12::SharedBufferMemoryD3D12ResourceDescriptor sharedD3d12ResourceDesc;
        sharedD3d12ResourceDesc.resource = d3d12Resource.Get();
        desc.nextInChain = &sharedD3d12ResourceDesc;
        return device.ImportSharedBufferMemory(&desc);
    }

    ComPtr<ID3D12Device> CreateD3D12Device(const wgpu::Device& device,
                                           bool createWarpDevice = false) {
        ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;
        ComPtr<IDXGIFactory4> dxgiFactory;
        CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
        if (createWarpDevice) {
            dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter));
        } else {
            dxgiAdapter = native::d3d::GetDXGIAdapter(device.GetAdapter().Get());
            DXGI_ADAPTER_DESC adapterDesc;
            dxgiAdapter->GetDesc(&adapterDesc);
            dxgiFactory->EnumAdapterByLuid(adapterDesc.AdapterLuid, IID_PPV_ARGS(&dxgiAdapter));
        }

        ComPtr<ID3D12Device> d3d12Device;

        D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device),
                          &d3d12Device);

        return d3d12Device;
    }

    ComPtr<ID3D12Resource> CreateD3D12Buffer(ID3D12Device* device,
                                             D3D12_HEAP_TYPE heapType,
                                             uint32_t bufferSize = kBufferSize) {
        D3D12_RESOURCE_STATES initialResourceState;
        D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;
        switch (heapType) {
            case D3D12_HEAP_TYPE_UPLOAD:
                initialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
                break;
            case D3D12_HEAP_TYPE_READBACK:
                initialResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
                break;
            default:
                initialResourceState = D3D12_RESOURCE_STATE_COMMON;
                resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        D3D12_HEAP_PROPERTIES heapProperties = {heapType, D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                                D3D12_MEMORY_POOL_UNKNOWN, 0, 0};

        D3D12_RESOURCE_DESC descriptor;
        descriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        descriptor.Alignment = 0;
        descriptor.Width = bufferSize;
        descriptor.Height = 1;
        descriptor.DepthOrArraySize = 1;
        descriptor.MipLevels = 1;
        descriptor.Format = DXGI_FORMAT_UNKNOWN;
        descriptor.SampleDesc.Count = 1;
        descriptor.SampleDesc.Quality = 0;
        descriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        descriptor.Flags = resourceFlags;

        ComPtr<ID3D12Resource> resource;

        device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &descriptor,
                                        initialResourceState, {}, IID_PPV_ARGS(&resource));
        return resource;
    }

  private:
    Backend() {}
};

// Ensure that importing a nullptr ID3D12Resource results in error.
TEST_P(SharedBufferMemoryTests, nullResourceFailure) {
    native::d3d12::SharedBufferMemoryD3D12ResourceDescriptor sharedD3d12ResourceDesc;
    sharedD3d12ResourceDesc.resource = nullptr;
    wgpu::SharedBufferMemoryDescriptor desc;
    desc.nextInChain = &sharedD3d12ResourceDesc;
    ASSERT_DEVICE_ERROR(device.ImportSharedBufferMemory(&desc));
}

// Validate that importing an ID3D12Resource across devices results in failure. This is tested by
// creating a resource with a WARP device and attempting to use it on a non-WARP device.
TEST_P(SharedBufferMemoryTests, CrossDeviceResourceImportFailure) {
    DAWN_TEST_UNSUPPORTED_IF(IsWARP());
    ComPtr<ID3D12Device> warpDevice =
        static_cast<Backend*>(GetParam().mBackend)->CreateD3D12Device(device, true);
    ComPtr<ID3D12Resource> d3d12Resource =
        static_cast<Backend*>(GetParam().mBackend)
            ->CreateD3D12Buffer(warpDevice.Get(), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_FLAG_NONE);
    wgpu::SharedBufferMemoryDescriptor desc;
    native::d3d12::SharedBufferMemoryD3D12ResourceDescriptor sharedD3d12ResourceDesc;
    sharedD3d12ResourceDesc.resource = d3d12Resource.Get();
    desc.nextInChain = &sharedD3d12ResourceDesc;

    ASSERT_DEVICE_ERROR(device.ImportSharedBufferMemory(&desc));
}

DAWN_INSTANTIATE_PREFIXED_TEST_P(D3D12,
                                 SharedBufferMemoryTests,
                                 {D3D12Backend()},
                                 {Backend::GetInstance()});

}  // anonymous namespace
}  // namespace dawn
