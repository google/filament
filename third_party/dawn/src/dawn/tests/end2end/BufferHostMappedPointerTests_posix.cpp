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

#include <sys/mman.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/end2end/BufferHostMappedPointerTests.h"

namespace dawn {
namespace {

class MMapBackend : public BufferHostMappedPointerTestBackend {
  public:
    static BufferHostMappedPointerTestBackend* GetInstance() {
        static MMapBackend backend;
        return &backend;
    }

    const char* Name() const override { return "mmap"; }

    std::pair<wgpu::Buffer, void*> CreateHostMappedBuffer(
        wgpu::Device device,
        wgpu::BufferUsage usage,
        size_t size,
        std::function<void(void*)> Populate) override {
        // Create a temporary file.
        char filename[] = "tmpXXXXXX";
        int fd = mkstemp(filename);
        EXPECT_GT(fd, -1);

        unlink(filename);

        // Write the initial data.
        std::vector<char> initialData(size);
        Populate(initialData.data());
        EXPECT_EQ(write(fd, initialData.data(), size), (signed)size);

        // Memory map the file.
        void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

        auto UnmapMemory = [=]() {
            munmap(ptr, size);
            close(fd);
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
            UnmapMemory();
        } else {
            mDisposeCallback.Use([&](auto callback) {
                EXPECT_CALL(*callback, Call(ptr)).WillOnce(testing::InvokeWithoutArgs(UnmapMemory));
            });
        }

        return std::make_pair(std::move(buffer), hostMappedDesc.pointer);
    }

  private:
    MutexProtected<testing::MockCallback<WGPUCallback>> mDisposeCallback;
};

DAWN_INSTANTIATE_PREFIXED_TEST_P(Posix,
                                 BufferHostMappedPointerTests,
                                 {MetalBackend(), VulkanBackend()},
                                 {MMapBackend::GetInstance()});

}  // anonymous namespace
}  // namespace dawn
