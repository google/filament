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

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>

#include "partition_alloc/pointers/raw_ptr.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"
#include "src/dawn/common/Math.h"
#include "src/dawn/common/Ref.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"
#include "src/utils/heap_array.h"
#include "src/utils/non_copyable.h"
#include "src/utils/span.h"

namespace dawn {
class MemoryBlockAllocator;
}  // namespace dawn

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
// TODO(crbug.com/491867541): Move Ref<T>s out of the commands and onto the CommandEncoder instead,
// so that we don't need to walk all the commands to free the Refs.

// These are the lists of blocks, should not be used directly, only through CommandAllocator
// and CommandIterator
using BlockDef = HeapArray<std::byte>;
using CommandBlocks = std::vector<BlockDef>;

namespace detail {
constexpr uint32_t kEndOfBlock = std::numeric_limits<uint32_t>::max();
constexpr uint32_t kAdditionalData = std::numeric_limits<uint32_t>::max() - 1;
}  // namespace detail

// The alignment used for all allocations in the command allocator. It is constant to help avoid
// complexity to handle variable alignment.
static inline constexpr size_t kMaxAllocatedCommandAlignment = sizeof(uint64_t);

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
        requires(std::is_same_v<UnderlyingType<E>, uint32_t>)
    bool NextCommandId(E* commandId) {
        std::optional<uint32_t> id = NextCommandId();
        *reinterpret_cast<uint32_t*>(commandId) = id.value_or(detail::kEndOfBlock);
        return id.has_value();
    }

    template <typename T>
        requires(alignof(T) <= kMaxAllocatedCommandAlignment)
    T* NextCommand() {
        return reinterpret_cast<T*>(NextCommand(sizeof(T)).data());
    }

    template <typename T, typename Index>
        requires(alignof(T) <= kMaxAllocatedCommandAlignment &&
                 (std::is_same_v<Index, size_t> || !std::is_integral_v<Index>))
    ityp::span<Index, const T> NextData(Index count) {
        size_t size = sizeof(T) * checked_cast<size_t>(count);
        // TODO(crbug.com/491867541): Move Ref<T>s out of the commands and onto the CommandEncoder
        // instead, so that we don't need to walk all the commands to free the Refs.
        if constexpr (IsRef<T>::value) {
            // SAFETY: If T is is a Ref, caller must ensure that the Refs have been properly
            // constructed in the command stream.
            return DAWN_UNSAFE_BUFFERS(ReinterpretSpan<const T, Index>(NextData(size)));
        } else {
            return ReinterpretSpan<const T, Index>(NextData(size));
        }
    }

    // Sets iterator to the beginning of the commands without emptying the list. This method can
    // be used if iteration was stopped early and the iterator needs to be restarted.
    void Reset();

    // This method must to be called after commands have been deleted. This indicates that the
    // commands have been submitted and they are no longer valid.
    void MakeEmptyAsDataWasDestroyed();

  private:
    bool IsEmpty() const;

    DAWN_FORCE_INLINE std::optional<uint32_t> NextCommandId() {
        DAWN_ASSERT(IsPtrAligned(mCurrentBlock.data(), kMaxAllocatedCommandAlignment));
        DAWN_ASSERT(mCurrentBlock.size() >= sizeof(uint32_t));

        uint32_t id = *reinterpret_cast<uint32_t*>(mCurrentBlock.data());

        if (id != detail::kEndOfBlock) {
            mCurrentBlock = mCurrentBlock.subspan(kMaxAllocatedCommandAlignment);
            return {id};
        }
        return NextCommandIdInNewBlock();
    }

    std::optional<uint32_t> NextCommandIdInNewBlock();

    DAWN_FORCE_INLINE Span<std::byte> NextCommand(size_t commandSize) {
        DAWN_ASSERT(IsPtrAligned(mCurrentBlock.data(), kMaxAllocatedCommandAlignment));

        size_t alignedSize = Align(commandSize, kMaxAllocatedCommandAlignment);
        return mCurrentBlock.TakeFirst(alignedSize).first(commandSize);
    }

    DAWN_FORCE_INLINE Span<std::byte> NextData(size_t dataSize) {
        std::optional<uint32_t> id = NextCommandId();
        DAWN_ASSERT(id.has_value());
        DAWN_ASSERT(id.value() == detail::kAdditionalData);

        return NextCommand(dataSize);
    }

    CommandBlocks mBlocks;
    size_t mCurrentBlockIndex = 0;

    // RAW_PTR_EXCLUSION: This is an extremely hot span during command iteration, but always points
    // to at least a valid uint32_t, either inside a block, or at mEndOfBlock.
    RAW_PTR_EXCLUSION Span<std::byte> mCurrentBlock;

    // Used to avoid a special case for empty iterators.
    alignas(kMaxAllocatedCommandAlignment) uint32_t mEndOfBlock = detail::kEndOfBlock;
    raw_ptr<MemoryBlockAllocator> mPool = nullptr;
};

class CommandAllocator : public NonCopyable {
  public:
    explicit CommandAllocator(MemoryBlockAllocator* pool);
    ~CommandAllocator();

    // NOTE: A moved-from CommandAllocator is reset to its initial empty state.
    CommandAllocator(CommandAllocator&&);
    CommandAllocator& operator=(CommandAllocator&&);

    // Frees all blocks held by the allocator and restores it to its initial empty state.
    void Reset();
    void Destroy();

    bool IsEmpty() const;

    template <typename T, typename E>
        requires(std::is_same_v<UnderlyingType<E>, uint32_t> &&
                 alignof(T) <= kMaxAllocatedCommandAlignment)
    T* Allocate(E commandId) {
        Span<std::byte> allocation = Allocate(static_cast<uint32_t>(commandId), sizeof(T));
        DAWN_CHECK(allocation.data() != nullptr);  // Crash on OOM

        T* result = reinterpret_cast<T*>(allocation.data());
        new (result) T;
        return result;
    }

    template <typename T, typename Index>
        requires(alignof(T) <= kMaxAllocatedCommandAlignment &&
                 (std::is_same_v<Index, size_t> || !std::is_integral_v<Index>))
    ityp::span<Index, T> AllocateData(Index count) {
        size_t size = sizeof(T) * checked_cast<size_t>(count);
        Span<std::byte> allocation = AllocateData(size);
        DAWN_CHECK(allocation.data() != nullptr);  // Crash on OOM
        ityp::span<Index, T> results;
        // TODO(crbug.com/491867541): Move Ref<T>s out of the commands and onto the CommandEncoder
        // instead, so that we don't need to walk all the commands to free the Refs.
        if constexpr (IsRef<T>::value) {
            // SAFETY: Reintepreting Ref<T> is OK because Ref<T> has the same layout as T* which is
            // a trivially_copyable type. We also manually handle releasing the Ref<T> in
            // FreeCommands.
            results = DAWN_UNSAFE_BUFFERS(ReinterpretSpan<T, Index>(allocation));
        } else {
            results = ReinterpretSpan<T, Index>(allocation);
        }
        for (auto& value : results) {
            new (&value) T;
        }
        return results;
    }

    size_t GetCommandBlocksCount() const;

  private:
    // To avoid checking for overflows at every step of the computations we compute an upper
    // bound of the space that will be needed in addition to the command data.
    static constexpr size_t kWorstCaseAdditionalSize =
        kMaxAllocatedCommandAlignment +  // The CommandID + padding before the command.
        kMaxAllocatedCommandAlignment +  // The padding after the command data.
        sizeof(uint32_t);                // The EndOfBlock tag.

    friend CommandIterator;
    CommandBlocks&& AcquireBlocks();

    DAWN_FORCE_INLINE Span<std::byte> Allocate(uint32_t commandId, size_t commandSize) {
        DAWN_ASSERT(!mCurrentBlock.empty());
        DAWN_ASSERT(commandId != detail::kEndOfBlock);

        // It should always be possible to allocate one id, for kEndOfBlock tagging,
        DAWN_ASSERT(IsPtrAligned(mCurrentBlock.data(), kMaxAllocatedCommandAlignment));
        DAWN_ASSERT(mCurrentBlock.size() >= sizeof(uint32_t));

        // The memory after the ID will contain the following:
        //   - the current ID
        //   - padding to align the command to kMaxAllocatedCommandAlignment
        //   - the command of size commandSize
        //   - padding to align the next ID to kMaxAllocatedCommandAlignment
        //   - the next ID of size sizeof(uint32_t)

        // The good case were we have enough space for the command data and upper bound of the
        // extra required space.
        if ((mCurrentBlock.size() >= kWorstCaseAdditionalSize) &&
            (mCurrentBlock.size() - kWorstCaseAdditionalSize >= commandSize)) {
            Span<std::byte> idAlloc = mCurrentBlock.TakeFirst(kMaxAllocatedCommandAlignment);
            *reinterpret_cast<uint32_t*>(idAlloc.data()) = commandId;

            size_t alignedSize = Align(commandSize, kMaxAllocatedCommandAlignment);
            return mCurrentBlock.TakeFirst(alignedSize).first(commandSize);
        }
        return AllocateInNewBlock(commandId, commandSize);
    }

    Span<std::byte> AllocateInNewBlock(uint32_t commandId, size_t commandSize);

    DAWN_FORCE_INLINE Span<std::byte> AllocateData(size_t commandSize) {
        return Allocate(detail::kAdditionalData, commandSize);
    }

    void AppendNewBlock(size_t minimumSize);

    void ResetPointers();

    raw_ptr<MemoryBlockAllocator> mPool;
    CommandBlocks mBlocks;

    // Data used for the block range at initialization so that the first call to Allocate sees
    // there is not enough space and calls AppendNewBlock. This avoids having to special case the
    // initialization in Allocate.
    alignas(kMaxAllocatedCommandAlignment) std::array<std::byte, 4> mPlaceholderSpace = {};

    // Span to the current range of allocation in the block. Guaranteed to allow for at least one
    // uint32_t if not empty, so that the special kEndOfBlock command id can always be written.
    // Empty iff the blocks were moved out.
    // RAW_PTR_EXCLUSION: This is an extremely hot span during command allocation, but always set
    // to a valid allocation (either the placeholder space, or a real allocated block).
    RAW_PTR_EXCLUSION Span<std::byte> mCurrentBlock;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDALLOCATOR_H_
