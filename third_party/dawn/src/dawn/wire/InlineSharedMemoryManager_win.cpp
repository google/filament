// Copyright 2026 The Dawn & Tint Authors
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

#include "src/utils/windows_with_undefs.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <utility>

#include "src/dawn/common/Constants.h"
#include "src/dawn/common/Math.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/common/SystemHandle.h"
#include "src/dawn/wire/InlineSharedMemoryManager.h"
#include "src/utils/span.h"

namespace dawn::wire {

namespace {

class SharedMemoryWin : public SharedMemory {
  public:
    SharedMemoryWin(SystemHandle handle, std::span<std::byte> data)
        : SharedMemory(std::move(handle), data) {}

    ~SharedMemoryWin() override {
        std::span<std::byte> data = GetMappedSpan();
        if (data.data() != nullptr) {
            UnmapViewOfFile(data.data());
        }
    }
};

class InlineSharedMemoryManagerImpl_Win : public InlineSharedMemoryManager {
  public:
    Ref<SharedMemory> CreateSharedMemory(size_t size) override {
        const uint64_t alignedSize = Align(static_cast<uint64_t>(size),
                                           kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment);
        HANDLE rawHandle = CreateFileMappingW(
            INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, static_cast<DWORD>(alignedSize >> 32),
            static_cast<DWORD>(alignedSize & 0xFFFFFFFF), nullptr);
        if (rawHandle == nullptr) {
            return nullptr;
        }
        SystemHandle handle = SystemHandle::Acquire(rawHandle);

        void* pointer = MapViewOfFile(rawHandle, FILE_MAP_ALL_ACCESS, 0, 0, size);
        if (pointer == nullptr) {
            return nullptr;
        }

        // SAFETY: the pointer returned by a successful MapViewOfFile points to at least `size`
        // valid bytes.
        auto data = DAWN_UNSAFE_BUFFERS(Span<std::byte>{static_cast<std::byte*>(pointer), size});
        return AcquireRef(new SharedMemoryWin(std::move(handle), data));
    }
};

}  // namespace

std::shared_ptr<InlineSharedMemoryManager> CreateInlineSharedMemoryManager() {
    return std::make_shared<InlineSharedMemoryManagerImpl_Win>();
}

}  // namespace dawn::wire
