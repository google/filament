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
#include <gtest/gtest.h>

#include <vector>

#include "dawn/native/D3D12Backend.h"
#include "src/dawn/common/SystemHandle.h"
#include "src/dawn/native/d3d12/DeviceD3D12.h"
#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/tests/white_box/SharedBufferMemoryTests.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/WGPUHelpers.h"
#include "src/utils/compiler.h"

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
    DAWN_UNSAFE_TODO(memcpy(mappedBufferBegin, &data, kBufferSize));
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

    SystemHandle fenceEvent = SystemHandle::Acquire(CreateEvent(nullptr, FALSE, FALSE, nullptr));
    if (fence->GetCompletedValue() < signaledValue) {
        fence->SetEventOnCompletion(signaledValue, fenceEvent.Get());
        WaitForSingleObject(fenceEvent.Get(), INFINITE);
    }
}

class ExistingD3D12ResourceBackend : public SharedBufferMemoryTestBackend {
  public:
    static Backend GetInstance() {
        static ExistingD3D12ResourceBackend b;
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
        D3D12_HEAP_PROPERTIES heapProperties = {heapType, D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                                D3D12_MEMORY_POOL_UNKNOWN, 0, 0};
        return CreateD3D12Buffer(device, heapProperties, bufferSize);
    }

    ComPtr<ID3D12Resource> CreateD3D12Buffer(ID3D12Device* device,
                                             D3D12_HEAP_PROPERTIES heapProperties,
                                             uint32_t bufferSize = kBufferSize) {
        D3D12_RESOURCE_STATES initialResourceState;
        D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;
        switch (heapProperties.Type) {
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
    ExistingD3D12ResourceBackend() {}
};

class SharedBufferMemoryExistingD3D12ResourceTests : public SharedBufferMemoryTests {};

// Ensure that importing a nullptr ID3D12Resource results in error.
TEST_P(SharedBufferMemoryExistingD3D12ResourceTests, NullResourceFailure) {
    native::d3d12::SharedBufferMemoryD3D12ResourceDescriptor sharedD3d12ResourceDesc;
    sharedD3d12ResourceDesc.resource = nullptr;
    wgpu::SharedBufferMemoryDescriptor desc;
    desc.nextInChain = &sharedD3d12ResourceDesc;
    ASSERT_DEVICE_ERROR(device.ImportSharedBufferMemory(&desc));
}

// Validate that importing an ID3D12Resource across devices results in failure. This is tested by
// creating a resource with a WARP device and attempting to use it on a non-WARP device.
TEST_P(SharedBufferMemoryExistingD3D12ResourceTests, CrossDeviceResourceImportFailure) {
    DAWN_TEST_UNSUPPORTED_IF(IsWARP());
    ComPtr<ID3D12Device> warpDevice =
        static_cast<ExistingD3D12ResourceBackend*>(GetParam().mBackend)
            ->CreateD3D12Device(device, true);
    ComPtr<ID3D12Resource> d3d12Resource =
        static_cast<ExistingD3D12ResourceBackend*>(GetParam().mBackend)
            ->CreateD3D12Buffer(warpDevice.Get(), D3D12_HEAP_TYPE_UPLOAD);
    wgpu::SharedBufferMemoryDescriptor desc;
    native::d3d12::SharedBufferMemoryD3D12ResourceDescriptor sharedD3d12ResourceDesc;
    sharedD3d12ResourceDesc.resource = d3d12Resource.Get();
    desc.nextInChain = &sharedD3d12ResourceDesc;

    ASSERT_DEVICE_ERROR(device.ImportSharedBufferMemory(&desc));
}

// Validate that importing an ID3D12Resource allocated on a CUSTOM heap that is equivalent to UPLOAD
// works correctly.
TEST_P(SharedBufferMemoryExistingD3D12ResourceTests, CustomUploadHeapImport) {
    ComPtr<ID3D12Device> d3d12Device =
        static_cast<ExistingD3D12ResourceBackend*>(GetParam().mBackend)
            ->CreateD3D12Device(device, false);
    D3D12_HEAP_PROPERTIES heapProperties =
        d3d12Device->GetCustomHeapProperties(0, D3D12_HEAP_TYPE_UPLOAD);
    wgpu::SharedBufferMemoryDescriptor desc;
    ComPtr<ID3D12Resource> d3d12Resource =
        static_cast<ExistingD3D12ResourceBackend*>(GetParam().mBackend)
            ->CreateD3D12Buffer(d3d12Device.Get(), heapProperties);
    native::d3d12::SharedBufferMemoryD3D12ResourceDescriptor sharedD3d12ResourceDesc;
    sharedD3d12ResourceDesc.resource = d3d12Resource.Get();
    desc.nextInChain = &sharedD3d12ResourceDesc;

    wgpu::SharedBufferMemory sharedBufferMemory = device.ImportSharedBufferMemory(&desc);
    ASSERT_TRUE(sharedBufferMemory.CreateBuffer().Get());
}

// Validate that importing an ID3D12Resource allocated on a CUSTOM heap that is equivalent to
// READBACK works correctly.
TEST_P(SharedBufferMemoryExistingD3D12ResourceTests, CustomReadbackHeapImport) {
    ComPtr<ID3D12Device> d3d12Device =
        static_cast<ExistingD3D12ResourceBackend*>(GetParam().mBackend)
            ->CreateD3D12Device(device, false);
    D3D12_HEAP_PROPERTIES heapProperties =
        d3d12Device->GetCustomHeapProperties(0, D3D12_HEAP_TYPE_READBACK);
    wgpu::SharedBufferMemoryDescriptor desc;
    ComPtr<ID3D12Resource> d3d12Resource =
        static_cast<ExistingD3D12ResourceBackend*>(GetParam().mBackend)
            ->CreateD3D12Buffer(d3d12Device.Get(), heapProperties);
    native::d3d12::SharedBufferMemoryD3D12ResourceDescriptor sharedD3d12ResourceDesc;
    sharedD3d12ResourceDesc.resource = d3d12Resource.Get();
    desc.nextInChain = &sharedD3d12ResourceDesc;

    wgpu::SharedBufferMemory sharedBufferMemory = device.ImportSharedBufferMemory(&desc);
    ASSERT_TRUE(sharedBufferMemory.CreateBuffer().Get());
}

// Validate that importing an ID3D12Resource allocated on a CUSTOM cross-adapter heap
// is equivalent to DEFAULT works correctly.
TEST_P(SharedBufferMemoryExistingD3D12ResourceTests, CustomCrossAdapterHeapImport) {
    ComPtr<ID3D12Device> d3d12Device =
        static_cast<ExistingD3D12ResourceBackend*>(GetParam().mBackend)
            ->CreateD3D12Device(device, false);

    D3D12_HEAP_PROPERTIES heapProperties = {
        D3D12_HEAP_TYPE_CUSTOM, D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE, D3D12_MEMORY_POOL_L0, 0, 0};

    D3D12_HEAP_DESC heapDesc = {kBufferSize, heapProperties,
                                D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
                                D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER};
    ComPtr<ID3D12Heap> heap;
    HRESULT hr = d3d12Device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap));
    ASSERT_EQ(hr, S_OK);

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = kBufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags =
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;

    ComPtr<ID3D12Resource> d3d12Resource;
    hr =
        d3d12Device->CreatePlacedResource(heap.Get(), 0, &resourceDesc, D3D12_RESOURCE_STATE_COMMON,
                                          nullptr, IID_PPV_ARGS(&d3d12Resource));
    ASSERT_EQ(hr, S_OK);

    wgpu::SharedBufferMemoryDescriptor desc;
    native::d3d12::SharedBufferMemoryD3D12ResourceDescriptor sharedD3d12ResourceDesc;
    sharedD3d12ResourceDesc.resource = d3d12Resource.Get();
    desc.nextInChain = &sharedD3d12ResourceDesc;

    wgpu::SharedBufferMemory sharedBufferMemory = device.ImportSharedBufferMemory(&desc);
    ASSERT_TRUE(sharedBufferMemory.CreateBuffer().Get());
}

// Tests that creating a buffer from SharedBufferMemory with mappedAtCreation=true is an error
// when the shared buffer memory does not have MapWrite usage.
TEST_P(SharedBufferMemoryExistingD3D12ResourceTests,
       CreateBufferMappedAtCreationWithoutMapWriteIsError) {
    constexpr wgpu::BufferUsage kStorageUsages =
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kStorageUsages, kBufferSize);
    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.size = properties.size;
    bufferDesc.usage = kStorageUsages;
    bufferDesc.mappedAtCreation = true;

    ASSERT_DEVICE_ERROR_MSG(
        memory.CreateBuffer(&bufferDesc),
        testing::HasSubstr(
            "mappedAtCreation=true requires the SharedBufferMemory to have MapWrite usage"));
}

// Tests that creating SharedBufferMemory emits a specific error message if Uniform usage specified.
TEST_P(SharedBufferMemoryExistingD3D12ResourceTests, UniformUsageValidation) {
    constexpr wgpu::BufferUsage kMapWriteUsages =
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.size = properties.size;
    bufferDesc.usage = properties.usage | wgpu::BufferUsage::Uniform;

    ASSERT_DEVICE_ERROR_MSG(memory.CreateBuffer(&bufferDesc), testing::HasSubstr("Uniform"));
}

// Verify that DuplicateHandle with the correct access rights including READ_CONTROL succeeds in
// OpenExistingHeapFromFileMapping() (control case for MissingReadControlAccessCausesFailure).
TEST_P(SharedBufferMemoryExistingD3D12ResourceTests, DuplicateWithReadControlAccessSucceeds) {
    ComPtr<ID3D12Device> d3d12Device =
        static_cast<ExistingD3D12ResourceBackend*>(GetParam().mBackend)
            ->CreateD3D12Device(device, false);
    ComPtr<ID3D12Device3> d3d12Device3;
    HRESULT hr = d3d12Device->QueryInterface(IID_PPV_ARGS(&d3d12Device3));
    DAWN_TEST_UNSUPPORTED_IF(hr != S_OK);

    LARGE_INTEGER largeSize = {};
    largeSize.QuadPart = kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment;
    SystemHandle handle =
        SystemHandle::Acquire(CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                                                largeSize.HighPart, largeSize.LowPart, nullptr));
    EXPECT_TRUE(handle.IsValid());

    SystemHandle duplicatedHandle;
    HANDLE process = GetCurrentProcess();
    constexpr DWORD kValidAccess = FILE_MAP_READ | FILE_MAP_WRITE | SECTION_QUERY | READ_CONTROL;
    EXPECT_TRUE(DuplicateHandle(process, handle.Get(), process, duplicatedHandle.GetMut(),
                                kValidAccess, FALSE, 0));

    // With READ_CONTROL present, OpenExistingHeapFromFileMapping should succeed.
    ComPtr<ID3D12Heap> d3d12Heap;
    hr = d3d12Device3->OpenExistingHeapFromFileMapping(duplicatedHandle.Get(),
                                                       IID_PPV_ARGS(&d3d12Heap));
    EXPECT_EQ(S_OK, hr);
}

// Verify missing READ_CONTROL access in DuplicateHandle will cause failure in
// OpenExistingHeapFromFileMapping()
TEST_P(SharedBufferMemoryExistingD3D12ResourceTests, MissingReadControlAccessCausesFailure) {
    ComPtr<ID3D12Device> d3d12Device =
        static_cast<ExistingD3D12ResourceBackend*>(GetParam().mBackend)
            ->CreateD3D12Device(device, false);
    ComPtr<ID3D12Device3> d3d12Device3;
    HRESULT hr = d3d12Device->QueryInterface(IID_PPV_ARGS(&d3d12Device3));
    DAWN_TEST_UNSUPPORTED_IF(hr != S_OK);

    LARGE_INTEGER largeSize = {};
    largeSize.QuadPart = kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment;
    SystemHandle handle =
        SystemHandle::Acquire(CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                                                largeSize.HighPart, largeSize.LowPart, nullptr));
    EXPECT_TRUE(handle.IsValid());

    // Import the duplicated handle to align with the behavior in Chromium.
    SystemHandle duplicatedHandle;
    HANDLE process = GetCurrentProcess();
    constexpr DWORD kInvalidAccess = FILE_MAP_READ | FILE_MAP_WRITE | SECTION_QUERY;
    EXPECT_TRUE(DuplicateHandle(process, handle.Get(), process, duplicatedHandle.GetMut(),
                                kInvalidAccess, FALSE, 0));

    // Missing read control will cause an error when calling `OpenExistingHeapFromFileMapping`.
    ComPtr<ID3D12Heap> d3d12Heap;
    HRESULT error_hr = d3d12Device3->OpenExistingHeapFromFileMapping(duplicatedHandle.Get(),
                                                                     IID_PPV_ARGS(&d3d12Heap));
    EXPECT_NE(S_OK, error_hr);
}

// Base backend for SharedBufferMemory backed by a Windows file mapping handle. Subclasses only
// need to override RequiredFeatures() to declare the features they require.
class D3D12SharedMemoryFileHandleBackendBase : public SharedBufferMemoryTestBackend {
  public:
    void TearDown() override { mSharedMemoryHandle.Close(); }

    wgpu::SharedBufferMemory CreateSharedBufferMemory(const wgpu::Device& device,
                                                      wgpu::BufferUsage usages,
                                                      uint32_t bufferSize,
                                                      uint32_t initializationData = 0) override {
        uint64_t alignedHeapSize =
            Align(bufferSize, kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment);

        LARGE_INTEGER largeSize = {};
        largeSize.QuadPart = alignedHeapSize;
        // Create a named shared memory object by using INVALID_HANDLE_VALUE as input file handle.
        // See https://learn.microsoft.com/en-us/windows/win32/memory/creating-named-shared-memory.
        mSharedMemoryHandle = SystemHandle::Acquire(
            CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, largeSize.HighPart,
                              largeSize.LowPart, nullptr));
        EXPECT_TRUE(mSharedMemoryHandle.IsValid());

        if (initializationData) {
            // Get mapped pointer to the file mapping object for test
            void* ptr = MapViewOfFile(mSharedMemoryHandle.Get(), FILE_MAP_ALL_ACCESS, 0, 0, 0);
            EXPECT_NE(ptr, nullptr);

            DAWN_UNSAFE_TODO(memcpy(ptr, &initializationData, sizeof(initializationData)));

            UnmapViewOfFile(ptr);
        }

        wgpu::SharedBufferMemoryDescriptor desc;
        wgpu::SharedBufferMemoryFromWindowsHandleDescriptor sharedFileHandleDesc;
        sharedFileHandleDesc.handle = mSharedMemoryHandle.Get();
        sharedFileHandleDesc.size = alignedHeapSize;
        desc.nextInChain = &sharedFileHandleDesc;

        return device.ImportSharedBufferMemory(&desc);
    }

  protected:
    D3D12SharedMemoryFileHandleBackendBase() {}

  private:
    SystemHandle mSharedMemoryHandle;
};

class D3D12SharedMemoryFileHandleWithExtendedUsagesBackend
    : public D3D12SharedMemoryFileHandleBackendBase {
  public:
    static Backend GetInstance() {
        static D3D12SharedMemoryFileHandleWithExtendedUsagesBackend b;
        return &b;
    }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter& adapter) const override {
        return {wgpu::FeatureName::SharedBufferMemoryFromWindowsHandle,
                wgpu::FeatureName::SharedFenceDXGISharedHandle,
                wgpu::FeatureName::BufferMapExtendedUsages, wgpu::FeatureName::HostMappedPointer};
    }

  private:
    D3D12SharedMemoryFileHandleWithExtendedUsagesBackend() {}
};

class SharedBufferMemoryD3D12SharedFileHandleWithExtendedUsagesTests
    : public SharedBufferMemoryTests {};

// Ensure that importing a nullptr handle results in error.
TEST_P(SharedBufferMemoryD3D12SharedFileHandleWithExtendedUsagesTests, nullResourceFailure) {
    wgpu::SharedBufferMemoryFromWindowsHandleDescriptor sharedFileHandleDesc;
    sharedFileHandleDesc.handle = nullptr;
    sharedFileHandleDesc.size = kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment;
    wgpu::SharedBufferMemoryDescriptor desc;
    desc.nextInChain = &sharedFileHandleDesc;
    ASSERT_DEVICE_ERROR(device.ImportSharedBufferMemory(&desc));
}

// Ensure that heap size not being a multiple of 65536 (D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
// results in error.
TEST_P(SharedBufferMemoryD3D12SharedFileHandleWithExtendedUsagesTests, MemorySizeNotAlignFailure) {
    constexpr uint32_t kUnAlignedSize = kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment / 2;

    LARGE_INTEGER largeSize = {};
    largeSize.QuadPart = kUnAlignedSize;
    // Create a named shared memory object by using INVALID_HANDLE_VALUE as input file handle.
    // See https://learn.microsoft.com/en-us/windows/win32/memory/creating-named-shared-memory.
    SystemHandle sharedMemoryHandle =
        SystemHandle::Acquire(CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                                                largeSize.HighPart, largeSize.LowPart, nullptr));
    EXPECT_TRUE(sharedMemoryHandle.IsValid());

    wgpu::SharedBufferMemoryFromWindowsHandleDescriptor sharedFileHandleDesc;
    sharedFileHandleDesc.handle = nullptr;
    sharedFileHandleDesc.size = kUnAlignedSize;
    wgpu::SharedBufferMemoryDescriptor desc;
    desc.nextInChain = &sharedFileHandleDesc;
    ASSERT_DEVICE_ERROR(device.ImportSharedBufferMemory(&desc));
}

// Tests that no error occurs when we create a SharedBufferMemory with Uniform usage.
TEST_P(SharedBufferMemoryD3D12SharedFileHandleWithExtendedUsagesTests, UniformUsageValidation) {
    constexpr wgpu::BufferUsage kMapWriteUsages =
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(
        device, kMapWriteUsages, kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment);
    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.size = properties.size;
    bufferDesc.usage = properties.usage | wgpu::BufferUsage::Uniform;

    memory.CreateBuffer(&bufferDesc);
}

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    D3D12,
    SharedBufferMemoryTests,
    {D3D12Backend()},
    {ExistingD3D12ResourceBackend::GetInstance(),
     D3D12SharedMemoryFileHandleWithExtendedUsagesBackend::GetInstance()});
// As D3D12 backend is filtered out on Windows x86, we need below to allow uninstantiated gtests.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SharedBufferMemoryExistingD3D12ResourceTests);
DAWN_INSTANTIATE_PREFIXED_TEST_P(D3D12,
                                 SharedBufferMemoryExistingD3D12ResourceTests,
                                 {D3D12Backend()},
                                 {ExistingD3D12ResourceBackend::GetInstance()});

// As D3D12 backend is filtered out on Windows x86, we need below to allow uninstantiated gtests.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(
    SharedBufferMemoryD3D12SharedFileHandleWithExtendedUsagesTests);
DAWN_INSTANTIATE_PREFIXED_TEST_P(
    D3D12,
    SharedBufferMemoryD3D12SharedFileHandleWithExtendedUsagesTests,
    {D3D12Backend()},
    {D3D12SharedMemoryFileHandleWithExtendedUsagesBackend::GetInstance()});

// Backend for platforms that support SharedBufferMemoryFromWindowsHandle and
// SharedFenceDXGISharedHandle but don't support BufferMapExtendedUsages.
class D3D12SharedMemoryFileHandleBackend : public D3D12SharedMemoryFileHandleBackendBase {
  public:
    static Backend GetInstance() {
        static D3D12SharedMemoryFileHandleBackend b;
        return &b;
    }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter& adapter) const override {
        return {wgpu::FeatureName::SharedBufferMemoryFromWindowsHandle,
                wgpu::FeatureName::SharedFenceDXGISharedHandle};
    }

  private:
    D3D12SharedMemoryFileHandleBackend() {}
};

class SharedBufferMemoryD3D12SharedFileHandleTests : public SharedBufferMemoryTests {};

// Tests that a buffer with MapWrite|CopySrc usages can be created from shared buffer memory,
// written via mappedAtCreation, copied to a destination buffer, and the destination contains
// the expected data.
TEST_P(SharedBufferMemoryD3D12SharedFileHandleTests, MapWriteCopySrcUsageSucceeds) {
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(
        device, wgpu::BufferUsage::None, kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment);
    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    DAWN_TEST_UNSUPPORTED_IF(!(properties.usage & wgpu::BufferUsage::MapWrite));

    constexpr uint32_t kTestData = 0x12345678;
    constexpr uint64_t kTestDataSize = sizeof(kTestData);

    // Create the source buffer from shared memory with mappedAtCreation = true.
    wgpu::BufferDescriptor srcDesc = {};
    srcDesc.size = kTestDataSize;
    srcDesc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    srcDesc.mappedAtCreation = true;
    wgpu::Buffer srcBuffer = memory.CreateBuffer(&srcDesc);
    ASSERT_TRUE(srcBuffer.Get());

    wgpu::SharedBufferMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = false;
    ASSERT_EQ(wgpu::Status::Success, memory.BeginAccess(srcBuffer, &beginDesc));

    // Write the test data through the mapped range and then unmap.
    uint32_t* mappedData = static_cast<uint32_t*>(srcBuffer.GetMappedRange(0, kTestDataSize));
    ASSERT_NE(nullptr, mappedData);
    *mappedData = kTestData;
    srcBuffer.Unmap();

    // Copy the source buffer to a regular device destination buffer.
    wgpu::BufferDescriptor dstDesc = {};
    dstDesc.size = kTestDataSize;
    dstDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&dstDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 0, kTestDataSize);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    wgpu::SharedBufferMemoryEndAccessState endState = {};
    ASSERT_EQ(wgpu::Status::Success, memory.EndAccess(srcBuffer, &endState));

    // Verify the destination buffer contains the data that was written to the source.
    EXPECT_BUFFER_U32_EQ(kTestData, dstBuffer, 0);
}

// Tests that a buffer with MapRead|CopyDst usages can be created from shared buffer memory,
// receive a copy from a source buffer, and the mapped contents match the source data.
TEST_P(SharedBufferMemoryD3D12SharedFileHandleTests, MapReadCopyDstUsageSucceeds) {
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(
        device, wgpu::BufferUsage::None, kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment);
    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    DAWN_TEST_UNSUPPORTED_IF(!(properties.usage & wgpu::BufferUsage::MapRead));

    constexpr uint32_t kTestData = 0x87654321;
    constexpr uint64_t kTestDataSize = sizeof(kTestData);

    // Create the destination buffer from shared memory with `MapRead|CopyDst` usages.
    wgpu::BufferDescriptor dstDesc = {};
    dstDesc.size = kTestDataSize;
    dstDesc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = memory.CreateBuffer(&dstDesc);
    ASSERT_TRUE(dstBuffer.Get());

    // Create a device-owned source buffer pre-filled with the test data.
    wgpu::Buffer srcBuffer =
        utils::CreateBufferFromData(device, &kTestData, kTestDataSize, wgpu::BufferUsage::CopySrc);

    wgpu::SharedBufferMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = false;
    ASSERT_EQ(wgpu::Status::Success, memory.BeginAccess(dstBuffer, &beginDesc));

    // Copy the source data into the destination shared buffer and then map it to verify.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 0, kTestDataSize);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    MapAsyncAndWait(dstBuffer, wgpu::MapMode::Read, 0, kTestDataSize);
    const uint32_t* mappedData =
        static_cast<const uint32_t*>(dstBuffer.GetConstMappedRange(0, kTestDataSize));
    ASSERT_NE(nullptr, mappedData);
    EXPECT_EQ(kTestData, *mappedData);
    dstBuffer.Unmap();

    wgpu::SharedBufferMemoryEndAccessState endState = {};
    ASSERT_EQ(wgpu::Status::Success, memory.EndAccess(dstBuffer, &endState));
}

// A regression test against `CanUseCopyResource()` with the the buffers created from shared buffer
// memory. `CanUseCopyResource()` must compare actual D3D12 resource widths instead of
// `GetAllocatedSize()` because for an external buffer, GetAllocatedSize() equals the WebGPU buffer
// size, which may be smaller than the D3D12 resource backing it (65536-byte aligned heap here).
TEST_P(SharedBufferMemoryD3D12SharedFileHandleTests,
       CopyFromSubsizedExternalBufferDoesNotUseCopyResource) {
    // The SharedBufferMemory is backed by a 65536-byte D3D12 resource (heap alignment requirement).
    // A 4-byte WebGPU buffer created from it has GetAllocatedSize() = 4, D3D12 width = 65536.
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(
        device, wgpu::BufferUsage::None, kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment);
    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);
    DAWN_TEST_UNSUPPORTED_IF(!(properties.usage & wgpu::BufferUsage::MapWrite));

    constexpr uint32_t kTestData = 0xDEADBEEF;
    constexpr uint64_t kTestDataSize = sizeof(kTestData);

    wgpu::BufferDescriptor srcDesc = {};
    srcDesc.size = kTestDataSize;
    srcDesc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    srcDesc.mappedAtCreation = true;
    wgpu::Buffer srcBuffer = memory.CreateBuffer(&srcDesc);
    ASSERT_TRUE(srcBuffer.Get());

    wgpu::SharedBufferMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = false;
    ASSERT_EQ(wgpu::Status::Success, memory.BeginAccess(srcBuffer, &beginDesc));

    uint32_t* mapped = static_cast<uint32_t*>(srcBuffer.GetMappedRange(0, kTestDataSize));
    ASSERT_NE(nullptr, mapped);
    *mapped = kTestData;
    srcBuffer.Unmap();

    // Regular Dawn CopyDst buffer: D3D12 resource width = kTestDataSize.
    // Old bug: GetAllocatedSize() both = 4 → CanUseCopyResource = true
    //          → CopyResource(4-byte D3D12, 65536-byte D3D12) → D3D12 validation error.
    // Fixed:   D3D12 widths 4 ≠ 65536 → CanUseCopyResource = false → CopyBufferRegion.
    wgpu::BufferDescriptor dstDesc = {};
    dstDesc.size = kTestDataSize;
    dstDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&dstDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 0, kTestDataSize);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    wgpu::SharedBufferMemoryEndAccessState endState = {};
    ASSERT_EQ(wgpu::Status::Success, memory.EndAccess(srcBuffer, &endState));

    EXPECT_BUFFER_U32_EQ(kTestData, dstBuffer, 0);
}

// As D3D12 backend is filtered out on Windows x86, we need below to allow uninstantiated gtests.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SharedBufferMemoryD3D12SharedFileHandleTests);
DAWN_INSTANTIATE_PREFIXED_TEST_P(D3D12,
                                 SharedBufferMemoryD3D12SharedFileHandleTests,
                                 {D3D12Backend()},
                                 {D3D12SharedMemoryFileHandleBackend::GetInstance()});

}  // anonymous namespace
}  // namespace dawn
