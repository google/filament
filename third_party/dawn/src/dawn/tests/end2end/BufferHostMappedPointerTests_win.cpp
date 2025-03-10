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

#include <utility>

#include "dawn/common/Log.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/windows_with_undefs.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/end2end/BufferHostMappedPointerTests.h"

namespace dawn {
namespace {

class VMBackend : public BufferHostMappedPointerTestBackend {
  public:
    static BufferHostMappedPointerTestBackend* GetInstance() {
        static VMBackend backend;
        return &backend;
    }

    const char* Name() const override { return "VirtualAlloc"; }

    std::pair<wgpu::Buffer, void*> CreateHostMappedBuffer(
        wgpu::Device device,
        wgpu::BufferUsage usage,
        size_t size,
        std::function<void(void*)> Populate) override {
        void* ptr = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        EXPECT_NE(ptr, nullptr);

        Populate(ptr);

        auto DeallocMemory = [=]() { VirtualFree(ptr, 0, MEM_RELEASE); };

        wgpu::BufferHostMappedPointer hostMappedDesc;
        hostMappedDesc.pointer = ptr;
        mDisposeCallback.Use([&](auto callback) {
            hostMappedDesc.disposeCallback = callback->Callback();
            hostMappedDesc.userdata = callback->MakeUserdata(ptr);
        });

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.usage = usage;
        bufferDesc.size = size;
        bufferDesc.nextInChain = &hostMappedDesc;

        wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
        if (dawn::native::CheckIsErrorForTesting(buffer.Get())) {
            DeallocMemory();
        } else {
            mDisposeCallback.Use([&](auto callback) {
                EXPECT_CALL(*callback, Call(ptr))
                    .WillOnce(testing::InvokeWithoutArgs(DeallocMemory));
            });
        }

        return std::make_pair(std::move(buffer), hostMappedDesc.pointer);
    }

  private:
    MutexProtected<testing::MockCallback<WGPUCallback>> mDisposeCallback;
};

class MMapBackend : public BufferHostMappedPointerTestBackend {
  public:
    static BufferHostMappedPointerTestBackend* GetInstance() {
        static MMapBackend backend;
        return &backend;
    }

    const char* Name() const override { return "FileMapping"; }

    std::pair<wgpu::Buffer, void*> CreateHostMappedBuffer(
        wgpu::Device device,
        wgpu::BufferUsage usage,
        size_t size,
        std::function<void(void*)> Populate) override {
        // Get the temp path string
        TCHAR tmpFilePath[MAX_PATH];
        DWORD dwRetVal = GetTempPath(MAX_PATH,      // length of the buffer
                                     tmpFilePath);  // buffer for path
        EXPECT_GT(dwRetVal, 0u);
        EXPECT_LE(dwRetVal, static_cast<DWORD>(MAX_PATH));

        TCHAR tmpFileName[MAX_PATH];
        EXPECT_GT(GetTempFileName(tmpFilePath,   // directory for tmp files
                                  TEXT("TMP"),   // temp file name prefix
                                  0,             // create unique name
                                  tmpFileName),  // buffer for name
                  0u);

        // Creates the new file
        HANDLE tmpFileHandle = CreateFile(tmpFileName,                   // file name
                                          GENERIC_READ | GENERIC_WRITE,  // open for read write
                                          0,                             // do not share
                                          NULL,                          // default security
                                          CREATE_ALWAYS,                 // overwrite existing
                                          FILE_ATTRIBUTE_NORMAL,         // normal file
                                          NULL);                         // no template
        EXPECT_NE(tmpFileHandle, INVALID_HANDLE_VALUE);

        LARGE_INTEGER largeSize;
        largeSize.QuadPart = size;
        HANDLE fileMappingHandle = CreateFileMapping(
            tmpFileHandle, nullptr, PAGE_READWRITE, largeSize.HighPart, largeSize.LowPart, nullptr);

        EXPECT_NE(fileMappingHandle, INVALID_HANDLE_VALUE);

        void* ptr = MapViewOfFile(fileMappingHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        EXPECT_NE(ptr, nullptr);
        Populate(ptr);

        auto DeallocMemory = [=]() {
            // Cleanup mapping, handles, and file.
            EXPECT_TRUE(UnmapViewOfFile(ptr));
            CloseHandle(fileMappingHandle);
            CloseHandle(tmpFileHandle);
            EXPECT_TRUE(DeleteFile(tmpFileName));
        };

        wgpu::BufferHostMappedPointer hostMappedDesc;
        hostMappedDesc.pointer = ptr;
        mDisposeCallback.Use([&](auto callback) {
            hostMappedDesc.disposeCallback = callback->Callback();
            hostMappedDesc.userdata = callback->MakeUserdata(ptr);
        });

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.usage = usage;
        bufferDesc.size = size;
        bufferDesc.nextInChain = &hostMappedDesc;

        wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
        if (dawn::native::CheckIsErrorForTesting(buffer.Get())) {
            DeallocMemory();
        } else {
            mDisposeCallback.Use([&](auto callback) {
                EXPECT_CALL(*callback, Call(ptr))
                    .WillOnce(testing::InvokeWithoutArgs(DeallocMemory));
            });
        }

        return std::make_pair(std::move(buffer), hostMappedDesc.pointer);
    }

  private:
    MutexProtected<testing::MockCallback<WGPUCallback>> mDisposeCallback;
};

DAWN_INSTANTIATE_PREFIXED_TEST_P(Win,
                                 BufferHostMappedPointerTests,
                                 {D3D12Backend()},
                                 {VMBackend::GetInstance(), MMapBackend::GetInstance()});

}  // anonymous namespace
}  // namespace dawn
