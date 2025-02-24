//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PoolAlloc.cpp:
//    Implements the class methods for PoolAllocator and Allocation classes.
//

#include "common/PoolAlloc.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/mathutil.h"
#include "common/platform.h"
#include "common/tls.h"

#if defined(ANGLE_WITH_ASAN)
#    include <sanitizer/asan_interface.h>
#endif

namespace angle
{
// If we are using guard blocks, we must track each individual allocation.  If we aren't using guard
// blocks, these never get instantiated, so won't have any impact.

class Allocation
{
  public:
    Allocation(size_t size, unsigned char *mem, Allocation *prev = 0)
        : mSize(size), mMem(mem), mPrevAlloc(prev)
    {
        // Allocations are bracketed:
        //
        //    [allocationHeader][initialGuardBlock][userData][finalGuardBlock]
        //
        // This would be cleaner with if (kGuardBlockSize)..., but that makes the compiler print
        // warnings about 0 length memsets, even with the if() protecting them.
#if defined(ANGLE_POOL_ALLOC_GUARD_BLOCKS)
        memset(preGuard(), kGuardBlockBeginVal, kGuardBlockSize);
        memset(data(), kUserDataFill, mSize);
        memset(postGuard(), kGuardBlockEndVal, kGuardBlockSize);
#endif
    }

    void checkAllocList() const;

    static size_t AlignedHeaderSize(uint8_t *allocationBasePtr, size_t alignment)
    {
        // Make sure that the data offset after the header is aligned to the given alignment.
        size_t base = reinterpret_cast<size_t>(allocationBasePtr);
        return rx::roundUpPow2(base + kGuardBlockSize + HeaderSize(), alignment) - base;
    }

    // Return total size needed to accommodate user buffer of 'size',
    // plus our tracking data and any necessary alignments.
    static size_t AllocationSize(uint8_t *allocationBasePtr,
                                 size_t size,
                                 size_t alignment,
                                 size_t *preAllocationPaddingOut)
    {
        // The allocation will be laid out as such:
        //
        //                      Aligned to |alignment|
        //                               ^
        //   preAllocationPaddingOut     |
        //        ___^___                |
        //       /       \               |
        //       <padding>[header][guard][data][guard]
        //       \___________ __________/
        //                   V
        //              dataOffset
        //
        // Note that alignment is at least as much as a pointer alignment, so the pointers in the
        // header are also necessarily aligned appropriately.
        //
        size_t dataOffset        = AlignedHeaderSize(allocationBasePtr, alignment);
        *preAllocationPaddingOut = dataOffset - HeaderSize() - kGuardBlockSize;

        return dataOffset + size + kGuardBlockSize;
    }

    // Given memory pointing to |header|, returns |data|.
    static uint8_t *GetDataPointer(uint8_t *memory, size_t alignment)
    {
        uint8_t *alignedPtr = memory + kGuardBlockSize + HeaderSize();

        // |memory| must be aligned already such that user data is aligned to |alignment|.
        ASSERT((reinterpret_cast<uintptr_t>(alignedPtr) & (alignment - 1)) == 0);

        return alignedPtr;
    }

  private:
    void checkGuardBlock(unsigned char *blockMem, unsigned char val, const char *locText) const;

    void checkAlloc() const
    {
        checkGuardBlock(preGuard(), kGuardBlockBeginVal, "before");
        checkGuardBlock(postGuard(), kGuardBlockEndVal, "after");
    }

    // Find offsets to pre and post guard blocks, and user data buffer
    unsigned char *preGuard() const { return mMem + HeaderSize(); }
    unsigned char *data() const { return preGuard() + kGuardBlockSize; }
    unsigned char *postGuard() const { return data() + mSize; }
    size_t mSize;            // size of the user data area
    unsigned char *mMem;     // beginning of our allocation (points to header)
    Allocation *mPrevAlloc;  // prior allocation in the chain

    static constexpr unsigned char kGuardBlockBeginVal = 0xfb;
    static constexpr unsigned char kGuardBlockEndVal   = 0xfe;
    static constexpr unsigned char kUserDataFill       = 0xcd;
#if defined(ANGLE_POOL_ALLOC_GUARD_BLOCKS)
    static constexpr size_t kGuardBlockSize = 16;
    static constexpr size_t HeaderSize() { return sizeof(Allocation); }
#else
    static constexpr size_t kGuardBlockSize = 0;
    static constexpr size_t HeaderSize() { return 0; }
#endif
};

#if !defined(ANGLE_DISABLE_POOL_ALLOC)
class PageHeader
{
  public:
    PageHeader(PageHeader *nextPage, size_t pageCount)
        : nextPage(nextPage),
          pageCount(pageCount)
#    if defined(ANGLE_POOL_ALLOC_GUARD_BLOCKS)
          ,
          lastAllocation(nullptr)
#    endif
    {}

    ~PageHeader()
    {
#    if defined(ANGLE_POOL_ALLOC_GUARD_BLOCKS)
        if (lastAllocation)
        {
            lastAllocation->checkAllocList();
        }
#    endif
    }

    PageHeader *nextPage;
    size_t pageCount;
#    if defined(ANGLE_POOL_ALLOC_GUARD_BLOCKS)
    Allocation *lastAllocation;
#    endif
};
#endif

//
// Implement the functionality of the PoolAllocator class, which
// is documented in PoolAlloc.h.
//
PoolAllocator::PoolAllocator(int growthIncrement, int allocationAlignment)
    : mAlignment(allocationAlignment),
#if !defined(ANGLE_DISABLE_POOL_ALLOC)
      mPageSize(growthIncrement),
      mFreeList(nullptr),
      mInUseList(nullptr),
      mNumCalls(0),
      mTotalBytes(0),
#endif
      mLocked(false)
{
    initialize(growthIncrement, allocationAlignment);
}

void PoolAllocator::initialize(int pageSize, int alignment)
{
    mAlignment = alignment;
#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    mPageSize       = pageSize;
    mPageHeaderSkip = sizeof(PageHeader);

    // Alignment == 1 is a special fast-path where fastAllocate() is enabled
    if (mAlignment != 1)
    {
#endif
        // Adjust mAlignment to be at least pointer aligned and
        // power of 2.
        //
        size_t minAlign = sizeof(void *);
        if (mAlignment < minAlign)
        {
            mAlignment = minAlign;
        }
        mAlignment = gl::ceilPow2(static_cast<unsigned int>(mAlignment));
#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    }
    //
    // Don't allow page sizes we know are smaller than all common
    // OS page sizes.
    //
    if (mPageSize < 4 * 1024)
    {
        mPageSize = 4 * 1024;
    }

    //
    // A large mCurrentPageOffset indicates a new page needs to
    // be obtained to allocate memory.
    //
    mCurrentPageOffset = mPageSize;

#else  // !defined(ANGLE_DISABLE_POOL_ALLOC)
    mStack.push_back({});
#endif
}

PoolAllocator::~PoolAllocator()
{
#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    while (mInUseList)
    {
        PageHeader *next = mInUseList->nextPage;
        mInUseList->~PageHeader();
        delete[] reinterpret_cast<char *>(mInUseList);
        mInUseList = next;
    }
    // We should not check the guard blocks
    // here, because we did it already when the block was
    // placed into the free list.
    //
    while (mFreeList)
    {
        PageHeader *next = mFreeList->nextPage;
        delete[] reinterpret_cast<char *>(mFreeList);
        mFreeList = next;
    }
#else  // !defined(ANGLE_DISABLE_POOL_ALLOC)
    for (auto &allocs : mStack)
    {
        for (auto alloc : allocs)
        {
            free(alloc);
        }
    }
    mStack.clear();
#endif
}

//
// Check a single guard block for damage
//
void Allocation::checkGuardBlock(unsigned char *blockMem,
                                 unsigned char val,
                                 const char *locText) const
{
#if defined(ANGLE_POOL_ALLOC_GUARD_BLOCKS)
    for (size_t x = 0; x < kGuardBlockSize; x++)
    {
        if (blockMem[x] != val)
        {
            char assertMsg[80];
            // We don't print the assert message.  It's here just to be helpful.
            snprintf(assertMsg, sizeof(assertMsg),
                     "PoolAlloc: Damage %s %zu byte allocation at 0x%p\n", locText, mSize, data());
            assert(0 && "PoolAlloc: Damage in guard block");
        }
    }
#endif
}

void PoolAllocator::push()
{
#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    AllocState state = {mCurrentPageOffset, mInUseList};

    mStack.push_back(state);

    //
    // Indicate there is no current page to allocate from.
    //
    mCurrentPageOffset = mPageSize;
#else  // !defined(ANGLE_DISABLE_POOL_ALLOC)
    mStack.push_back({});
#endif
}

// Do a mass-deallocation of all the individual allocations that have occurred since the last
// push(), or since the last pop(), or since the object's creation.
//
// Single-page allocations are saved for future use unless the release strategy is All.
void PoolAllocator::pop(ReleaseStrategy releaseStrategy)
{
    if (mStack.size() < 1)
    {
        return;
    }

#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    PageHeader *page   = mStack.back().page;
    mCurrentPageOffset = mStack.back().offset;

    while (mInUseList != page)
    {
        // Grab the pageCount before calling the destructor.  While the destructor doesn't actually
        // touch this variable, it's confusing MSAN.
        const size_t pageCount = mInUseList->pageCount;
        PageHeader *nextInUse  = mInUseList->nextPage;

        // invoke destructor to free allocation list
        mInUseList->~PageHeader();

        if (pageCount > 1 || releaseStrategy == ReleaseStrategy::All)
        {
            delete[] reinterpret_cast<char *>(mInUseList);
        }
        else
        {
#    if defined(ANGLE_WITH_ASAN)
            // Clear any container annotations left over from when the memory
            // was last used. (crbug.com/1419798)
            __asan_unpoison_memory_region(mInUseList, mPageSize);
#    endif
            mInUseList->nextPage = mFreeList;
            mFreeList            = mInUseList;
        }
        mInUseList = nextInUse;
    }

    mStack.pop_back();
#else  // !defined(ANGLE_DISABLE_POOL_ALLOC)
    for (auto &alloc : mStack.back())
    {
        free(alloc);
    }
    mStack.pop_back();
#endif
}

//
// Do a mass-deallocation of all the individual allocations
// that have occurred.
//
void PoolAllocator::popAll()
{
    while (mStack.size() > 0)
        pop();
}

void *PoolAllocator::allocate(size_t numBytes)
{
    ASSERT(!mLocked);

#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    //
    // Just keep some interesting statistics.
    //
    ++mNumCalls;
    mTotalBytes += numBytes;

    uint8_t *currentPagePtr = reinterpret_cast<uint8_t *>(mInUseList) + mCurrentPageOffset;

    size_t preAllocationPadding = 0;
    size_t allocationSize =
        Allocation::AllocationSize(currentPagePtr, numBytes, mAlignment, &preAllocationPadding);

    // Integer overflow is unexpected.
    ASSERT(allocationSize >= numBytes);

    // Do the allocation, most likely case first, for efficiency.
    if (allocationSize <= mPageSize - mCurrentPageOffset)
    {
        // There is enough room to allocate from the current page at mCurrentPageOffset.
        uint8_t *memory = currentPagePtr + preAllocationPadding;
        mCurrentPageOffset += allocationSize;

        return initializeAllocation(memory, numBytes);
    }

    if (allocationSize > mPageSize - mPageHeaderSkip)
    {
        // If the allocation is larger than a whole page, do a multi-page allocation.  These are not
        // mixed with the others.  The OS is efficient in allocating and freeing multiple pages.

        // We don't know what the alignment of the new allocated memory will be, so conservatively
        // allocate enough memory for up to alignment extra bytes being needed.
        allocationSize = Allocation::AllocationSize(reinterpret_cast<uint8_t *>(mPageHeaderSkip),
                                                    numBytes, mAlignment, &preAllocationPadding);

        size_t numBytesToAlloc = allocationSize + mPageHeaderSkip + mAlignment;

        // Integer overflow is unexpected.
        ASSERT(numBytesToAlloc >= allocationSize);

        PageHeader *memory = reinterpret_cast<PageHeader *>(::new char[numBytesToAlloc]);
        if (memory == nullptr)
        {
            return nullptr;
        }

        // Use placement-new to initialize header
        new (memory) PageHeader(mInUseList, (numBytesToAlloc + mPageSize - 1) / mPageSize);
        mInUseList = memory;

        // Make next allocation come from a new page
        mCurrentPageOffset = mPageSize;

        // Now that we actually have the pointer, make sure the data pointer will be aligned.
        currentPagePtr = reinterpret_cast<uint8_t *>(memory) + mPageHeaderSkip;
        Allocation::AllocationSize(currentPagePtr, numBytes, mAlignment, &preAllocationPadding);

        return initializeAllocation(currentPagePtr + preAllocationPadding, numBytes);
    }

    uint8_t *newPageAddr = allocateNewPage(numBytes);
    return initializeAllocation(newPageAddr, numBytes);

#else  // !defined(ANGLE_DISABLE_POOL_ALLOC)

    void *alloc = malloc(numBytes + mAlignment - 1);
    mStack.back().push_back(alloc);

    intptr_t intAlloc = reinterpret_cast<intptr_t>(alloc);
    intAlloc          = rx::roundUpPow2<intptr_t>(intAlloc, mAlignment);
    return reinterpret_cast<void *>(intAlloc);
#endif
}

#if !defined(ANGLE_DISABLE_POOL_ALLOC)
uint8_t *PoolAllocator::allocateNewPage(size_t numBytes)
{
    // Need a simple page to allocate from.  Pick a page from the free list, if any.  Otherwise need
    // to make the allocation.
    PageHeader *memory;
    if (mFreeList)
    {
        memory    = mFreeList;
        mFreeList = mFreeList->nextPage;
    }
    else
    {
        memory = reinterpret_cast<PageHeader *>(::new char[mPageSize]);
        if (memory == nullptr)
        {
            return nullptr;
        }
    }
    // Use placement-new to initialize header
    new (memory) PageHeader(mInUseList, 1);
    mInUseList = memory;

    // Leave room for the page header.
    mCurrentPageOffset      = mPageHeaderSkip;
    uint8_t *currentPagePtr = reinterpret_cast<uint8_t *>(mInUseList) + mCurrentPageOffset;

    size_t preAllocationPadding = 0;
    size_t allocationSize =
        Allocation::AllocationSize(currentPagePtr, numBytes, mAlignment, &preAllocationPadding);

    mCurrentPageOffset += allocationSize;

    // The new allocation is made after the page header and any alignment required before it.
    return reinterpret_cast<uint8_t *>(mInUseList) + mPageHeaderSkip + preAllocationPadding;
}

void *PoolAllocator::initializeAllocation(uint8_t *memory, size_t numBytes)
{
#    if defined(ANGLE_POOL_ALLOC_GUARD_BLOCKS)
    new (memory) Allocation(numBytes, memory, mInUseList->lastAllocation);
    mInUseList->lastAllocation = reinterpret_cast<Allocation *>(memory);
#    endif

    return Allocation::GetDataPointer(memory, mAlignment);
}
#endif

void PoolAllocator::lock()
{
    ASSERT(!mLocked);
    mLocked = true;
}

void PoolAllocator::unlock()
{
    ASSERT(mLocked);
    mLocked = false;
}

//
// Check all allocations in a list for damage by calling check on each.
//
void Allocation::checkAllocList() const
{
    for (const Allocation *alloc = this; alloc != nullptr; alloc = alloc->mPrevAlloc)
    {
        alloc->checkAlloc();
    }
}

}  // namespace angle
