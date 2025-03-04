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

#include "dawn/native/CommandAllocator.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <new>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"

namespace dawn::native {

// TODO(cwallez@chromium.org): figure out a way to have more type safety for the iterator

CommandIterator::CommandIterator() {
    Reset();
}

CommandIterator::~CommandIterator() {
    DAWN_ASSERT(IsEmpty());
}

CommandIterator::CommandIterator(CommandIterator&& other) {
    if (!other.IsEmpty()) {
        mBlocks = std::move(other.mBlocks);
        other.Reset();
    }
    Reset();
}

CommandIterator& CommandIterator::operator=(CommandIterator&& other) {
    DAWN_ASSERT(IsEmpty());
    if (!other.IsEmpty()) {
        mBlocks = std::move(other.mBlocks);
        other.Reset();
    }
    Reset();
    return *this;
}

CommandIterator::CommandIterator(CommandAllocator allocator) : mBlocks(allocator.AcquireBlocks()) {
    Reset();
}

void CommandIterator::AcquireCommandBlocks(std::vector<CommandAllocator> allocators) {
    DAWN_ASSERT(IsEmpty());
    mBlocks.clear();

    size_t totalBlocksCount = 0;
    for (CommandAllocator& allocator : allocators) {
        totalBlocksCount += allocator.GetCommandBlocksCount();
    }

    mBlocks.reserve(totalBlocksCount);
    for (CommandAllocator& allocator : allocators) {
        CommandBlocks blocks = allocator.AcquireBlocks();
        if (!blocks.empty()) {
            for (BlockDef& block : blocks) {
                mBlocks.push_back(std::move(block));
            }
        }
    }
    Reset();
}

bool CommandIterator::NextCommandIdInNewBlock(uint32_t* commandId) {
    mCurrentBlock++;
    if (mCurrentBlock >= mBlocks.size()) {
        Reset();
        *commandId = detail::kEndOfBlock;
        return false;
    }
    mCurrentPtr = AlignPtr(mBlocks[mCurrentBlock].block.get(), alignof(uint32_t));
    return NextCommandId(commandId);
}

void CommandIterator::Reset() {
    mCurrentBlock = 0;

    if (mBlocks.empty()) {
        // This will case the first NextCommandId call to try to move to the next block and stop
        // the iteration immediately, without special casing the initialization.
        mCurrentPtr = reinterpret_cast<char*>(&mEndOfBlock);
    } else {
        mCurrentPtr = AlignPtr(mBlocks[0].block.get(), alignof(uint32_t));
    }
}

void CommandIterator::MakeEmptyAsDataWasDestroyed() {
    if (IsEmpty()) {
        return;
    }

    mCurrentPtr = reinterpret_cast<char*>(&mEndOfBlock);
    mBlocks.clear();
    Reset();
    DAWN_ASSERT(IsEmpty());
}

bool CommandIterator::IsEmpty() const {
    return mBlocks.empty();
}

// Potential TODO(crbug.com/dawn/835):
//  - Host the size and pointer to next block in the block itself to avoid having an allocation
//    in the vector
//  - Assume T's alignof is, say 64bits, static assert it, and make commandAlignment a constant
//    in Allocate
//  - Be able to optimize allocation to one block, for command buffers expected to live long to
//    avoid cache misses
//  - Better block allocation, maybe have Dawn API to say command buffer is going to have size
//    close to another

CommandAllocator::CommandAllocator() {
    ResetPointers();
}

CommandAllocator::~CommandAllocator() {
    Reset();
}

CommandAllocator::CommandAllocator(CommandAllocator&& other)
    : mBlocks(std::move(other.mBlocks)), mLastAllocationSize(other.mLastAllocationSize) {
    other.mBlocks.clear();
    if (!other.IsEmpty()) {
        mCurrentPtr = other.mCurrentPtr;
        mEndPtr = other.mEndPtr;
    } else {
        ResetPointers();
    }
    other.Reset();
}

CommandAllocator& CommandAllocator::operator=(CommandAllocator&& other) {
    Reset();
    if (!other.IsEmpty()) {
        std::swap(mBlocks, other.mBlocks);
        mLastAllocationSize = other.mLastAllocationSize;
        mCurrentPtr = other.mCurrentPtr;
        mEndPtr = other.mEndPtr;
    }
    other.Reset();
    return *this;
}

void CommandAllocator::Reset() {
    ResetPointers();
    mBlocks.clear();
    mLastAllocationSize = kDefaultBaseAllocationSize;
}

bool CommandAllocator::IsEmpty() const {
    return mCurrentPtr == reinterpret_cast<const char*>(&mPlaceholderSpace[0]);
}

size_t CommandAllocator::GetCommandBlocksCount() const {
    return mBlocks.size();
}

CommandBlocks&& CommandAllocator::AcquireBlocks() {
    DAWN_ASSERT(mCurrentPtr != nullptr && mEndPtr != nullptr);
    DAWN_ASSERT(IsPtrAligned(mCurrentPtr, alignof(uint32_t)));
    DAWN_ASSERT(mCurrentPtr + sizeof(uint32_t) <= mEndPtr);
    *reinterpret_cast<uint32_t*>(mCurrentPtr) = detail::kEndOfBlock;

    mCurrentPtr = nullptr;
    mEndPtr = nullptr;
    return std::move(mBlocks);
}

char* CommandAllocator::AllocateInNewBlock(uint32_t commandId,
                                           size_t commandSize,
                                           size_t commandAlignment) {
    // When there is not enough space, we signal the kEndOfBlock, so that the iterator knows
    // to move to the next one. kEndOfBlock on the last block means the end of the commands.
    uint32_t* idAlloc = reinterpret_cast<uint32_t*>(mCurrentPtr);
    *idAlloc = detail::kEndOfBlock;

    // We'll request a block that can contain at least the command ID, the command and an
    // additional ID to contain the kEndOfBlock tag.
    size_t requestedBlockSize = commandSize + kWorstCaseAdditionalSize;

    // The computation of the request could overflow.
    if (DAWN_UNLIKELY(requestedBlockSize <= commandSize)) {
        return nullptr;
    }

    if (DAWN_UNLIKELY(!GetNewBlock(requestedBlockSize))) {
        return nullptr;
    }
    return Allocate(commandId, commandSize, commandAlignment);
}

bool CommandAllocator::GetNewBlock(size_t minimumSize) {
    // Allocate blocks doubling sizes each time, to a maximum of 16k (or at least minimumSize).
    mLastAllocationSize = std::max(minimumSize, std::min(mLastAllocationSize * 2, size_t(16384)));

    auto block = std::unique_ptr<char[]>(new (std::nothrow) char[mLastAllocationSize]);
    if (DAWN_UNLIKELY(block == nullptr)) {
        return false;
    }

    mCurrentPtr = AlignPtr(block.get(), alignof(uint32_t));
    mEndPtr = block.get() + mLastAllocationSize;
    mBlocks.push_back({mLastAllocationSize, std::move(block)});
    return true;
}

void CommandAllocator::ResetPointers() {
    mCurrentPtr = reinterpret_cast<char*>(&mPlaceholderSpace[0]);
    mEndPtr = reinterpret_cast<char*>(&mPlaceholderSpace[1]);
}

}  // namespace dawn::native
