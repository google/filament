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

#ifndef SRC_DAWN_WIRE_INLINESHAREDMEMORYMANAGER_H_
#define SRC_DAWN_WIRE_INLINESHAREDMEMORYMANAGER_H_

#include <cstddef>
#include <cstdint>
#include <memory>

#include "absl/container/flat_hash_map.h"
#include "dawn/wire/dawn_wire_export.h"
#include "src/dawn/common/MutexProtected.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/common/RefCounted.h"
#include "src/dawn/common/SystemHandle.h"
#include "src/utils/span.h"
#include "src/utils/typed_integer.h"

namespace dawn::wire {

// Identifies a SharedMemory reference in an InlineSharedMemoryManager.
using SharedMemoryID = TypedInteger<struct SharedMemoryIDT, uint64_t>;

// A ref-counted shared memory allocation.
class SharedMemory : public RefCounted {
  public:
    SharedMemory(SystemHandle handle, Span<std::byte> data);
    ~SharedMemory() override;

    Span<std::byte> GetMappedSpan() const { return mData; }

    // The underlying OS handle of the shared memory.
    const SystemHandle& GetSystemHandle() const { return mHandle; }

  private:
    SystemHandle mHandle;
    Span<std::byte> mData;
};

// `InlineSharedMemoryManager` manages all the shared memory allocations for inline memory transfer
// services.
class InlineSharedMemoryManager {
  public:
    InlineSharedMemoryManager();
    virtual ~InlineSharedMemoryManager();

    // Creates a new `SharedMemory` of at least `size` bytes.
    virtual Ref<SharedMemory> CreateSharedMemory(size_t size);

    // Registers `sharedMemory` for the transfer through dawn wire and returns a unique ID assigned
    // to it. The ID can be used by the wire server to retrieve the shared memory from the
    // `InlineSharedMemoryManager`.
    SharedMemoryID PutOnWireAndGetID(SharedMemory* sharedMemory);

    // Acquires the `SharedMemory` from the wire with the ID returned by `PutOnWireAndGetID`.
    Ref<SharedMemory> AcquireFromWire(SharedMemoryID id);

  private:
    struct AllSharedMemoryOnWire {
        absl::flat_hash_map<SharedMemoryID, Ref<SharedMemory>> idToSharedMemory;
        SharedMemoryID nextId{1u};
    };
    MutexProtected<AllSharedMemoryOnWire> mAllSharedMemoryOnWire;
};

DAWN_WIRE_EXPORT std::shared_ptr<InlineSharedMemoryManager> CreateInlineSharedMemoryManager();

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_INLINESHAREDMEMORYMANAGER_H_
