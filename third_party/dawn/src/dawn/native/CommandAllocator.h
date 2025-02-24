// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_COMMANDALLOCATOR_H_
#define SRC_DAWN_NATIVE_COMMANDALLOCATOR_H_

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/common/NonCopyable.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"

namespace dawn::native {

// Allocation for command buffers should be fast. To avoid doing an allocation per command
// or to avoid copying commands when reallocing, we use a linear allocator in a growing set
// of large memory blocks. We also use this to have the format to be (u32 commandId, command),
// so that iteration over the commands is easy.

// Usage of the allocator and iterator:
//     CommandAllocator allocator;
//     DrawCommand* cmd = allocator.Allocate<DrawCommand>(CommandType::Draw);
//     // Fill command
//     // Repeat allocation and filling commands
//
//     CommandIterator commands(allocator);
//     CommandType type;
//     while(commands.NextCommandId(&type)) {
//         switch(type) {
//              case CommandType::Draw:
//                  DrawCommand* draw = commands.NextCommand<DrawCommand>();
//                  // Do the draw
//                  break;
//              // other cases
//         }
//     }

// Note that you need to extract the commands from the CommandAllocator before destroying it
// and must tell the CommandIterator when the allocated commands have been processed for
// deletion.

// These are the lists of blocks, should not be used directly, only through CommandAllocator
// and CommandIterator
struct BlockDef {
    size_t size;
    std::unique_ptr<char[]> block;
};
using CommandBlocks = std::vector<BlockDef>;

namespace detail {
constexpr uint32_t kEndOfBlock = std::numeric_limits<uint32_t>::max();
constexpr uint32_t kAdditionalData = std::numeric_limits<uint32_t>::max() - 1;
}  // namespace detail

class CommandAllocator;

class CommandIterator : public NonCopyable {
  public:
    CommandIterator();
    ~CommandIterator();

    CommandIterator(CommandIterator&& other);
    CommandIterator& operator=(CommandIterator&& other);

    // Shorthand constructor for acquiring CommandBlocks from a single CommandAllocator.
    explicit CommandIterator(CommandAllocator allocator);

    void AcquireCommandBlocks(std::vector<CommandAllocator> allocators);

    template <typename E>
    bool NextCommandId(E* commandId) {
        return NextCommandId(reinterpret_cast<uint32_t*>(commandId));
    }
    template <typename T>
    T* NextCommand() {
        return static_cast<T*>(NextCommand(sizeof(T), alignof(T)));
    }
    template <typename T>
    T* NextData(size_t count) {
        return static_cast<T*>(NextData(sizeof(T) * count, alignof(T)));
    }

    // Sets iterator to the beginning of the commands without emptying the list. This method can
    // be used if iteration was stopped early and the iterator needs to be restarted.
    void Reset();

    // This method must to be called after commands have been deleted. This indicates that the
    // commands have been submitted and they are no longer valid.
    void MakeEmptyAsDataWasDestroyed();

  private:
    bool IsEmpty() const;

    DAWN_FORCE_INLINE bool NextCommandId(uint32_t* commandId) {
        char* idPtr = AlignPtr(mCurrentPtr, alignof(uint32_t));
        DAWN_ASSERT(idPtr == reinterpret_cast<char*>(&mEndOfBlock) ||
                    idPtr + sizeof(uint32_t) <=
                        mBlocks[mCurrentBlock].block.get() + mBlocks[mCurrentBlock].size);

        uint32_t id = *reinterpret_cast<uint32_t*>(idPtr);

        if (id != detail::kEndOfBlock) {
            mCurrentPtr = idPtr + sizeof(uint32_t);
            *commandId = id;
            return true;
        }
        return NextCommandIdInNewBlock(commandId);
    }

    bool NextCommandIdInNewBlock(uint32_t* commandId);

    DAWN_FORCE_INLINE void* NextCommand(size_t commandSize, size_t commandAlignment) {
        char* commandPtr = AlignPtr(mCurrentPtr, commandAlignment);
        DAWN_ASSERT(commandPtr + sizeof(commandSize) <=
                    mBlocks[mCurrentBlock].block.get() + mBlocks[mCurrentBlock].size);

        mCurrentPtr = commandPtr + commandSize;
        return commandPtr;
    }

    DAWN_FORCE_INLINE void* NextData(size_t dataSize, size_t dataAlignment) {
        uint32_t id;
        bool hasId = NextCommandId(&id);
        DAWN_ASSERT(hasId);
        DAWN_ASSERT(id == detail::kAdditionalData);

        return NextCommand(dataSize, dataAlignment);
    }

    CommandBlocks mBlocks;
    // RAW_PTR_EXCLUSION: This is an extremely hot pointer during command iteration, but always
    // points to at least a valid uint32_t, either inside a block, or at mEndOfBlock.
    RAW_PTR_EXCLUSION char* mCurrentPtr = nullptr;
    size_t mCurrentBlock = 0;
    // Used to avoid a special case for empty iterators.
    uint32_t mEndOfBlock = detail::kEndOfBlock;
};

class CommandAllocator : public NonCopyable {
  public:
    CommandAllocator();
    ~CommandAllocator();

    // NOTE: A moved-from CommandAllocator is reset to its initial empty state.
    CommandAllocator(CommandAllocator&&);
    CommandAllocator& operator=(CommandAllocator&&);

    // Frees all blocks held by the allocator and restores it to its initial empty state.
    void Reset();

    bool IsEmpty() const;

    template <typename T, typename E>
    T* Allocate(E commandId) {
        static_assert(sizeof(E) == sizeof(uint32_t));
        static_assert(alignof(E) == alignof(uint32_t));
        static_assert(alignof(T) <= kMaxSupportedAlignment);
        T* result =
            reinterpret_cast<T*>(Allocate(static_cast<uint32_t>(commandId), sizeof(T), alignof(T)));
        if (!result) {
            return nullptr;
        }
        new (result) T;
        return result;
    }

    template <typename T>
    T* AllocateData(size_t count) {
        static_assert(alignof(T) <= kMaxSupportedAlignment);
        T* result = reinterpret_cast<T*>(AllocateData(sizeof(T) * count, alignof(T)));
        if (!result) {
            return nullptr;
        }
        for (size_t i = 0; i < count; i++) {
            new (result + i) T;
        }
        return result;
    }

    size_t GetCommandBlocksCount() const;

  private:
    // This is used for some internal computations and can be any power of two as long as code
    // using the CommandAllocator passes the static_asserts.
    static constexpr size_t kMaxSupportedAlignment = 8;

    // To avoid checking for overflows at every step of the computations we compute an upper
    // bound of the space that will be needed in addition to the command data.
    static constexpr size_t kWorstCaseAdditionalSize =
        sizeof(uint32_t) + kMaxSupportedAlignment + alignof(uint32_t) + sizeof(uint32_t);

    // The default value of mLastAllocationSize.
    static constexpr size_t kDefaultBaseAllocationSize = 2048;

    friend CommandIterator;
    CommandBlocks&& AcquireBlocks();

    DAWN_FORCE_INLINE char* Allocate(uint32_t commandId,
                                     size_t commandSize,
                                     size_t commandAlignment) {
        DAWN_ASSERT(mCurrentPtr != nullptr);
        DAWN_ASSERT(mEndPtr != nullptr);
        DAWN_ASSERT(commandId != detail::kEndOfBlock);

        // It should always be possible to allocate one id, for kEndOfBlock tagging,
        DAWN_ASSERT(IsPtrAligned(mCurrentPtr, alignof(uint32_t)));
        DAWN_ASSERT(mEndPtr >= mCurrentPtr);
        DAWN_ASSERT(static_cast<size_t>(mEndPtr - mCurrentPtr) >= sizeof(uint32_t));

        // The memory after the ID will contain the following:
        //   - the current ID
        //   - padding to align the command, maximum kMaxSupportedAlignment
        //   - the command of size commandSize
        //   - padding to align the next ID, maximum alignof(uint32_t)
        //   - the next ID of size sizeof(uint32_t)

        // This can't overflow because by construction mCurrentPtr always has space for the next
        // ID.
        size_t remainingSize = static_cast<size_t>(mEndPtr - mCurrentPtr);

        // The good case were we have enough space for the command data and upper bound of the
        // extra required space.
        if ((remainingSize >= kWorstCaseAdditionalSize) &&
            (remainingSize - kWorstCaseAdditionalSize >= commandSize)) {
            uint32_t* idAlloc = reinterpret_cast<uint32_t*>(mCurrentPtr);
            *idAlloc = commandId;

            char* commandAlloc = AlignPtr(mCurrentPtr + sizeof(uint32_t), commandAlignment);
            mCurrentPtr = AlignPtr(commandAlloc + commandSize, alignof(uint32_t));

            return commandAlloc;
        }
        return AllocateInNewBlock(commandId, commandSize, commandAlignment);
    }

    char* AllocateInNewBlock(uint32_t commandId, size_t commandSize, size_t commandAlignment);

    DAWN_FORCE_INLINE char* AllocateData(size_t commandSize, size_t commandAlignment) {
        return Allocate(detail::kAdditionalData, commandSize, commandAlignment);
    }

    bool GetNewBlock(size_t minimumSize);

    void ResetPointers();

    CommandBlocks mBlocks;
    size_t mLastAllocationSize = kDefaultBaseAllocationSize;

    // Data used for the block range at initialization so that the first call to Allocate sees
    // there is not enough space and calls GetNewBlock. This avoids having to special case the
    // initialization in Allocate.
    uint32_t mPlaceholderSpace[1] = {0};

    // Pointers to the current range of allocation in the block. Guaranteed to allow for at
    // least one uint32_t if not nullptr, so that the special kEndOfBlock command id can always
    // be written. Nullptr iff the blocks were moved out.
    // RAW_PTR_EXCLUSION: These are extremely hot pointers during command allocation, but always
    // set to a valid slice (either the placeholder space, or a real allocated block).
    RAW_PTR_EXCLUSION char* mCurrentPtr = nullptr;
    RAW_PTR_EXCLUSION char* mEndPtr = nullptr;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDALLOCATOR_H_
