/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <utils/PagedArenaBitset.h>

#include <utils/compiler.h>
#include <utils/Slice.h>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>

namespace utils {

/**
 * PagedArenaBitset: THE HIERARCHY INVARIANT
 * ----------------------------------------
 * This bitset operates on a 4-level "Trust the Mask" architecture to enable
 * aggressive pruning of sparse domains. To maintain search integrity and
 * prevent memory corruption, the following structural invariant must be
 * preserved by all mutating operations:
 *
 * 1. INCLUSIVITY RULE:
 *    The hierarchy is strictly inclusive from the bottom up.
 *    [L3: Arena Bit] -> [L3: activeWordsMask] -> [L2: Directory Entry] -> [L1: Summary] -> [L0: Master]
 *
 *    - If a bit is set in an L3 word, its corresponding bit in `activeWordsMask` MUST be set.
 *    - If `activeWordsMask` is non-zero, the L2 Directory Entry MUST be a valid index.
 *    - If an L2 entry is valid, the corresponding bit in L1 MUST be set.
 *    - If any bit in L1 is set, the corresponding bit in L0 MUST be set.
 *
 * 2. THE GHOST-BIT PROHIBITION:
 *    Top-down updates are forbidden. Never update a Mask (L0/L1) before
 *    confirming the population (popcount > 0) of the underlying L3 page.
 *    Failure to follow this results in "Ghost Bits"—where a mask directs
 *    traversal to a region that has an INVALID_PAGE directory entry,
 *    triggering out-of-bounds access or assertions.
 *
 * 3. THE GARBAGE CONTRACT:
 *    Page allocations (L3) via emplace_back are uninitialized (garbage) for
 *    Data-Oriented Design.
 *    - RECYCLE: allocatePage() calls clear() ONLY when pulling from the free-list.
 *    - FULL WRITES: Merge/Union operations MUST fully overwrite the 64-word
 *      page array to clean garbage.
 *    - PARTIAL WRITES: Single-bit set() or sparse operations MUST manually
 *      clear the page upon its first allocation.
 *
 * 4. SEARCH PRUNING:
 *    Traversals (intersect, difference, forEach) skip entire 2^12 bit domains
 *    if the L0/L1 masks are 0. Furthermore, inside active pages, operations
 *    leverage the `activeWordsMask` to skip empty words via hardware CTZ intrinsics.
 *

 */

PagedArenaBitset::PagedArenaBitset()
    : mSummaryMask(MASK_WORDS, 0),          // All bits 0 (512 * u64 ->  4KiB)
      mDirectory(DIR_SIZE, INVALID_PAGE),   // All entries 0xFFFF (32768 * u16 -> 64KiB)
      mMasterMask({})
{
    // Place these near the top of PagedArenaBitset.cpp
    static_assert(std::is_trivially_copyable_v<Page>,
        "Page must be trivially copyable for high-performance vector reallocations.");

    static_assert(std::is_standard_layout_v<Page>,
        "Page must have standard layout for binary compatibility and memory alignment.");

    static_assert(sizeof(Page) == (WORDS_PER_PAGE + 1) * sizeof(uint64_t),
        "Page size mismatch: Expected words array + activeWordsMask.");

    // No reserve(mArena) here!
    // We only reserve mArena in intersect() where we know the size.
}

PagedArenaBitset::~PagedArenaBitset() = default;
PagedArenaBitset::PagedArenaBitset(PagedArenaBitset&&) noexcept = default;
PagedArenaBitset::PagedArenaBitset(PagedArenaBitset const&) = default;

uint16_t PagedArenaBitset::allocatePage() {
    if (!mFreePages.empty()) {
        uint16_t const idx = mFreePages.back();
        mFreePages.pop_back();
        assert(idx < mArena.size() && "Free list contains invalid arena index");
        mArena[idx].clear();
        return idx;
    }

    assert(mArena.size() < PagedArenaBitset::INVALID_PAGE && "Arena capacity exceeded directory index size");
    uint16_t const idx = static_cast<uint16_t>(mArena.size());
    mArena.emplace_back();
    return idx;
}

void PagedArenaBitset::freePage(uint32_t const dirIdx) {
    assert(dirIdx < PagedArenaBitset::DIR_SIZE && "Directory index out of bounds");
    uint16_t const pageIdx = mDirectory[dirIdx];

    assert(pageIdx != PagedArenaBitset::INVALID_PAGE && "Attempting to free an unallocated page");
    assert(pageIdx < mArena.size() && "Page index out of bounds");

    mFreePages.push_back(pageIdx);
    mDirectory[dirIdx] = INVALID_PAGE;
    mSummaryMask[dirIdx >> WORD_SHIFT] &= ~(1ULL << (dirIdx & WORD_MASK));
}

void PagedArenaBitset::reserve(size_t const size) {
    mArena.reserve(size);
}

PagedArenaBitset& PagedArenaBitset::copyFrom(const PagedArenaBitset& other) {
    assert(mSummaryMask.size() == MASK_WORDS && "FATAL: Attempted to use a moved-from PagedArenaBitset!");

    if (this == &other) return *this;

    mArena = other.mArena;
    mDirectory = other.mDirectory;
    mSummaryMask = other.mSummaryMask;

    // Simple scalar copies
    mMasterMask = other.mMasterMask;
    mSize = other.mSize;
    return *this;
}

bool PagedArenaBitset::operator[](uint32_t const index) const {
    assert(mSummaryMask.size() == MASK_WORDS && "FATAL: Attempted to use a moved-from PagedArenaBitset!");
    assert(index < (1ULL << DOMAIN_BITS) && "Index out of bounds");

    uint32_t const dirIdx = index >> PAGE_SHIFT;
    uint32_t const maskIdx = dirIdx >> WORD_SHIFT;
    uint32_t const bitInMask = dirIdx & WORD_MASK;

    if ((mSummaryMask[maskIdx] & (1ULL << bitInMask)) == 0) return false;

    uint16_t const pageIdx = mDirectory[dirIdx];
    uint32_t const wordIdx = (index >> WORD_SHIFT) & WORD_MASK;
    uint32_t const bitIdx = index & WORD_MASK;
    return (mArena[pageIdx].words[wordIdx] >> bitIdx) & 1ULL;
}

bool PagedArenaBitset::fetchAdd(uint32_t const index) {
    assert(mSummaryMask.size() == MASK_WORDS && "FATAL: Attempted to use a moved-from PagedArenaBitset!");
    assert(index < (1ULL << DOMAIN_BITS) && "Index out of bounds");

    uint32_t const dirIdx = index >> PAGE_SHIFT;
    uint32_t const maskIdx = dirIdx >> WORD_SHIFT;
    uint32_t const bitInMask = dirIdx & WORD_MASK;
    bool const pageExists = (mSummaryMask[maskIdx] & (1ULL << bitInMask)) != 0;

    uint16_t pageIdx;
    if (!pageExists) {
        pageIdx = allocatePage();
        mDirectory[dirIdx] = pageIdx;
        mSummaryMask[maskIdx] |= (1ULL << bitInMask);
        mMasterMask[maskIdx >> WORD_SHIFT] |= (1ULL << (maskIdx & WORD_MASK));
    } else {
        pageIdx = mDirectory[dirIdx];
    }

    uint32_t const wordIdx = (index >> WORD_SHIFT) & WORD_MASK;
    uint32_t const bitIdx = index & WORD_MASK;

    uint64_t& word = mArena[pageIdx].words[wordIdx];
    uint64_t const mask = 1ULL << bitIdx;

    bool const wasSet = (word & mask) != 0;
    if (!wasSet) {
        word |= mask;
        mArena[pageIdx].activeWordsMask |= (1ULL << wordIdx);
        mSize++;
    }
    return wasSet;
}

bool PagedArenaBitset::fetchRemove(uint32_t const index) {
    assert(mSummaryMask.size() == MASK_WORDS && "FATAL: Attempted to use a moved-from PagedArenaBitset!");
    assert(index < (1ULL << DOMAIN_BITS) && "Index out of bounds");

    uint32_t const dirIdx = index >> PAGE_SHIFT;
    uint32_t const maskIdx = dirIdx >> WORD_SHIFT;
    uint32_t const bitInMask = dirIdx & WORD_MASK;

    if ((mSummaryMask[maskIdx] & (1ULL << bitInMask)) == 0) return false;

    uint16_t const pageIdx = mDirectory[dirIdx];

    uint32_t const wordIdx = (index >> WORD_SHIFT) & WORD_MASK;
    uint32_t const bitIdx = index & WORD_MASK;

    uint64_t& word = mArena[pageIdx].words[wordIdx];
    uint64_t const mask = 1ULL << bitIdx;

    bool const wasSet = (word & mask) != 0;
    if (wasSet) {
        word &= ~mask;
        if (word == 0) {
            mArena[pageIdx].activeWordsMask &= ~(1ULL << wordIdx);
        }
        assert(mSize > 0 && "Size underflow");
        mSize--;
    }
    return wasSet;
}

PagedArenaBitset& PagedArenaBitset::intersect(const PagedArenaBitset& other) {
    // Assert moved-from state check (adjust macro based on your framework)
    assert(mSummaryMask.size() == MASK_WORDS && "FATAL: Attempted to use a moved-from PagedArenaBitset!");

    // Algorithmic Early Exits
    if (empty()) {
        return *this;
    }
    if (other.empty()) {
        clear();
        return *this;
    }

    // Traverse the 3-level directory hierarchy
    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t myMaster = mMasterMask[m];

        // Level 1: Master Directory Traversal
        while (myMaster) {
            int const maskBit = std::countr_zero(myMaster);
            myMaster &= myMaster - 1; // Clear lowest set bit to advance loop

            // Compute index into Level 2 (Summary Masks)
            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            // Load the summary masks (representing 64 software blocks/pages)
            uint64_t const myMask = mSummaryMask[maskIdx];
            uint64_t const otherMask = other.mSummaryMask[maskIdx];

            // Bits set in both summary masks mean pages overlap
            uint64_t keepMask = myMask & otherMask;
            // Bits set in mine but NOT in other mean pages must be dropped entirely
            uint64_t dropMask = myMask & ~otherMask;

            // --- DROP PASS ---
            // Efficiently free large ranges that exist in this bitset but not 'other'
            while (dropMask) {
                int const bit = std::countr_zero(dropMask);
                dropMask &= dropMask - 1; // Clear bit immediately for pipelining

                // Convert summary index bit to directory index (pointing to the Page ptr)
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                // Subtract entire population of this page before freeing
                mSize -= mArena[mDirectory[dirIdx]].popcount();
                freePage(dirIdx);
            }

            // --- KEEP PASS (Optimization Hybrid) ---
            // Process software blocks where data might actually intersect
            while (keepMask) {
                int const bit = std::countr_zero(keepMask);
                keepMask &= keepMask - 1; // Clear bit immediately to unblock CPU pipeline

                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                // Lookup actual Page structs in the Arena
                auto& [activeWordsMask, words] = mArena[mDirectory[dirIdx]];
                auto const& [activeWordsMaskOther, wordsOther] = other.mArena[other.mDirectory[dirIdx]];

                // Use the newly added page-level active words mask
                const uint64_t current_active_mask = activeWordsMask;

                uint32_t removedInPage = 0;
                uint64_t new_active_mask = 0;

                // 1. Process Overlapping Words
                // Words that contain data in BOTH pages.
                uint64_t overlap = current_active_mask & activeWordsMaskOther;
                while (overlap != 0) {
                    int const w = std::countr_zero(overlap);
                    overlap &= (overlap - 1); // Advance immediately for instruction parallelism

                    uint64_t const oldWord = words[w];
                    uint64_t const newWord = oldWord & wordsOther[w];
                    words[w] = newWord;

                    removedInPage += std::popcount(oldWord ^ newWord);

                    // Branchless mask update (newWord might become 0 after intersection)
                    new_active_mask |= (uint64_t(newWord != 0) << w);
                }

                // 2. Process Dead Words (Ghost Data Handling)
                // Words active in this page, but entirely empty in the other page.
                // Must be explicitly zeroed in memory to prevent memory corruption.
                uint64_t dead = current_active_mask & ~activeWordsMaskOther;
                while (dead != 0) {
                    int const w = std::countr_zero(dead);
                    dead &= (dead - 1); // Advance loop immediately

                    // Entirety of oldWord is being removed
                    removedInPage += std::popcount(words[w]);
                    words[w] = 0; // CRITICAL: Zero out dead word!
                    // Bit corresponding to word 'w' remains 0 in new_active_mask.
                }

                // Apply finalized updates to the page
                activeWordsMask = new_active_mask;
                mSize -= removedInPage;

                // Level 3 Cleanup: If entire page is now zero, free it
                if (new_active_mask == 0) {
                    freePage(dirIdx);
                }
            }

            // Level 2 Cleanup: If the entire summary mask word became zero,
            // clear its corresponding bit in the Master mask.
            if (mSummaryMask[maskIdx] == 0) {
                mMasterMask[m] &= ~(1ULL << maskBit);
            }
        }
    }

    return *this;
}

PagedArenaBitset& PagedArenaBitset::difference(const PagedArenaBitset& other) {
    assert(mSummaryMask.size() == MASK_WORDS && "FATAL: Attempted to use a moved-from PagedArenaBitset!");

    if (empty() || other.empty()) {
        return *this;
    }

    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t overlapMaster = mMasterMask[m] & other.mMasterMask[m];

        while (overlapMaster) {
            int const maskBit = std::countr_zero(overlapMaster);
            overlapMaster &= overlapMaster - 1;

            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            uint64_t overlapMask = mSummaryMask[maskIdx] & other.mSummaryMask[maskIdx];

            while (overlapMask) {
                int const bit = std::countr_zero(overlapMask);
                overlapMask &= overlapMask - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                auto& [activeWordsMask, words] = mArena[mDirectory[dirIdx]];
                auto const& [activeWordsMaskOther, wordsOther] = other.mArena[other.mDirectory[dirIdx]];

                uint32_t removedInPage = 0;
                uint64_t new_mask = 0;

                // Overlap
                uint64_t overlap = activeWordsMask & activeWordsMaskOther;
                while (overlap != 0) {
                    int const w = std::countr_zero(overlap);
                    overlap &= (overlap - 1); // Advance immediately

                    uint64_t const oldWord = words[w];
                    uint64_t const newWord = oldWord & ~wordsOther[w];
                    words[w] = newWord;

                    removedInPage += std::popcount(oldWord ^ newWord);
                    new_mask |= (uint64_t(newWord != 0) << w);
                }

                // Preserved Words
                uint64_t const preserved = activeWordsMask & ~activeWordsMaskOther;
                new_mask |= preserved;

                activeWordsMask = new_mask;
                mSize -= removedInPage;

                if (activeWordsMask == 0) {
                    freePage(dirIdx);
                }
            }

            if (mSummaryMask[maskIdx] == 0) {
                mMasterMask[m] &= ~(1ULL << maskBit);
            }
        }
    }
    return *this;
}

PagedArenaBitset& PagedArenaBitset::merge(const PagedArenaBitset& other) {
    assert(mSummaryMask.size() == MASK_WORDS && "FATAL: Attempted to use a moved-from PagedArenaBitset!");

    if (other.empty() || this == &other) {
        return *this;
    }

    if (empty() && !other.empty()) {
        copyFrom(other);
        return *this;
    }

    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t otherMaster = other.mMasterMask[m];
        if (!otherMaster) continue;

        // Note: Don't update mMasterMask[m] yet.

        while (otherMaster) {
            int const maskBit = std::countr_zero(otherMaster);
            otherMaster &= otherMaster - 1;
            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            uint64_t otherMask = other.mSummaryMask[maskIdx];
            // Note: Don't update mSummaryMask[maskIdx] yet.

            while (otherMask) {
                int const bit = std::countr_zero(otherMask);
                otherMask &= otherMask - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                uint16_t const otherPageIdx = other.mDirectory[dirIdx];
                uint16_t myPageIdx = mDirectory[dirIdx];

                if (myPageIdx == INVALID_PAGE) {
                    myPageIdx = allocatePage();
                    mDirectory[dirIdx] = myPageIdx;

                    const auto& srcPage = other.mArena[otherPageIdx];
                    mArena[myPageIdx] = srcPage;

                    uint32_t const pCount = srcPage.popcount();
                    mSize += pCount;

                    // Update masks only because we confirmed data was added
                    mSummaryMask[maskIdx] |= (1ULL << bit);
                    mMasterMask[m] |= (1ULL << maskBit);
                } else {
                    auto& [activeWordsMask, words] = mArena[myPageIdx];
                    auto const& [activeWordsMaskOther, wordsOther] = other.mArena[otherPageIdx];
                    uint32_t newlySetBits = 0;

                    // Active Src Words
                    uint64_t srcMask = activeWordsMaskOther;
                    while (srcMask != 0) {
                        int const w = std::countr_zero(srcMask);
                        srcMask &= (srcMask - 1); // Advance immediately

                        uint64_t const oldWord = words[w];
                        uint64_t const otherWord = wordsOther[w];
                        uint64_t const newWord = oldWord | otherWord;
                        words[w] = newWord;

                        newlySetBits += std::popcount(oldWord ^ newWord);
                    }

                    activeWordsMask |= activeWordsMaskOther;

                    if (newlySetBits > 0) {
                        mSize += newlySetBits;
                        mSummaryMask[maskIdx] |= (1ULL << bit);
                        mMasterMask[m] |= (1ULL << maskBit);
                    }
                }
            }


        }
    }
    return *this;
}

void PagedArenaBitset::defragment() {
    std::vector<Page> newArena;
    newArena.reserve(mArena.size() - mFreePages.size());

    uint32_t actualSize = 0;

    // Trust the mask to find active pages
    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t myMaster = mMasterMask[m];
        while (myMaster) {
            int const maskBit = std::countr_zero(myMaster);
            myMaster &= myMaster - 1;
            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            uint64_t myMask = mSummaryMask[maskIdx];
            while (myMask) {
                int const bit = std::countr_zero(myMask);
                myMask &= myMask - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                uint16_t const oldPageIdx = mDirectory[dirIdx];
                Page const& page = mArena[oldPageIdx];
                uint32_t const pop = page.popcount();

                if (pop == 0) {
                    // Drop the ghost page
                    mDirectory[dirIdx] = INVALID_PAGE;
                    mSummaryMask[maskIdx] &= ~(1ULL << bit);
                } else {
                    // Pack the surviving page
                    uint16_t const newPageIdx = static_cast<uint16_t>(newArena.size());
                    newArena.push_back(page);
                    mDirectory[dirIdx] = newPageIdx;
                    actualSize += pop;
                }
            }
        }
    }

    assert(mSize == actualSize && "Size invariant violated during defragmentation");
    mSize = actualSize;

    mArena = std::move(newArena);
    mFreePages.clear();

    // Recalculate master mask
    std::fill(mMasterMask.begin(), mMasterMask.end(), 0);
    for (uint32_t i = 0; i < MASK_WORDS; ++i) {
        if (mSummaryMask[i] != 0) {
            mMasterMask[i >> WORD_SHIFT] |= (1ULL << (i & WORD_MASK));
        }
    }
}

void PagedArenaBitset::extractTo(std::vector<uint32_t>& outBuffer) const {
    outBuffer.clear();
    if (empty()) {
        return;
    }

    outBuffer.reserve(mSize);
    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t masterMask = mMasterMask[m];
        while (masterMask) {
            int const maskBit = std::countr_zero(masterMask);
            masterMask &= masterMask - 1;

            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;
            uint64_t mask = mSummaryMask[maskIdx];

            while (mask) {
                int const bit = std::countr_zero(mask);
                mask &= mask - 1;

                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;
                auto const& [activeWordsMask, words] = mArena[mDirectory[dirIdx]];
                uint64_t activeMask = activeWordsMask;

                while (activeMask != 0) {
                    int const w = std::countr_zero(activeMask);
                    uint64_t word = words[w];
                    while (word) {
                        int const wBit = std::countr_zero(word);
                        word &= word - 1;
                        outBuffer.push_back((dirIdx << PAGE_SHIFT) | (w << WORD_SHIFT) | wBit);
                    }
                    activeMask &= (activeMask - 1);
                }
            }
        }
    }
}

void PagedArenaBitset::Page::clear() {
    for (int i = 0; i < WORDS_PER_PAGE; ++i) {
        words[i] = 0;
    }
    activeWordsMask = 0;
}

uint32_t PagedArenaBitset::Page::popcount() const {
    uint32_t count = 0;
    uint64_t mask = activeWordsMask;
    while (mask != 0) {
        int const w = std::countr_zero(mask);
        count += std::popcount(words[w]);
        mask &= (mask - 1);
    }
    return count;
}

size_t PagedArenaBitset::size() const {
    assert(mSummaryMask.size() == MASK_WORDS && "FATAL: Attempted to use a moved-from PagedArenaBitset!");
    return mSize;
}

bool PagedArenaBitset::empty() const {
    assert(mSummaryMask.size() == MASK_WORDS && "FATAL: Attempted to use a moved-from PagedArenaBitset!");
    return mSize == 0;
}

void PagedArenaBitset::clear() {
    assert(mSummaryMask.size() == MASK_WORDS && "FATAL: Attempted to use a moved-from PagedArenaBitset!");
    std::fill(mSummaryMask.begin(), mSummaryMask.end(), 0);
    std::fill(mMasterMask.begin(), mMasterMask.end(), 0);
    mArena.clear();
    mFreePages.clear();
    mSize = 0;
}

void PagedArenaBitset::add(uint32_t const index) {
    fetchAdd(index);
}

void PagedArenaBitset::remove(uint32_t const index) {
    fetchRemove(index);
}

PagedArenaBitset& PagedArenaBitset::operator=(PagedArenaBitset&& rhs) noexcept {
    if (this != &rhs) {
        mSummaryMask = std::move(rhs.mSummaryMask);
        mDirectory = std::move(rhs.mDirectory);
        mArena = std::move(rhs.mArena);
        mFreePages = std::move(rhs.mFreePages);
        mSize = rhs.mSize;
        mMasterMask = rhs.mMasterMask;

        rhs.mSize = 0;
        std::fill(rhs.mMasterMask.begin(), rhs.mMasterMask.end(), 0);
    }
    return *this;
}

PagedArenaBitset PagedArenaBitset::clone() const {
    assert(mSummaryMask.size() == MASK_WORDS && "FATAL: Attempted to use a moved-from PagedArenaBitset!");
    return *this;
}

bool PagedArenaBitset::isSubset(const PagedArenaBitset& subset, const PagedArenaBitset& superset) noexcept {
    // 1. Trivial Size & Identity Checks
    if (subset.mSize > superset.mSize) return false;
    if (subset.empty() || &subset == &superset) return true;
    if (superset.empty()) return false;

    // 2. Hierarchical Inclusion Check
    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t const subL0 = subset.mMasterMask[m];
        uint64_t const superL0 = superset.mMasterMask[m];

        // --- LEVEL 0 CONFLICT PASS ---
        // Blocks that exist in subset but NOT in superset.
        // This is only legal if EVERY underlying page inside them is an empty Ghost Page.
        uint64_t conflictL0 = subL0 & ~superL0;
        while (conflictL0) {
            int const maskBit = std::countr_zero(conflictL0);
            conflictL0 &= conflictL0 - 1;
            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            uint64_t subL1 = subset.mSummaryMask[maskIdx];
            while (subL1) {
                int const bit = std::countr_zero(subL1);
                subL1 &= subL1 - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;
                if (subset.mArena[subset.mDirectory[dirIdx]].activeWordsMask != 0) {
                    return false; // Found an actual active bit unique to subset!
                }
            }
        }

        // --- LEVEL 0 OVERLAP PASS ---
        // Process blocks shared by both sets.
        uint64_t overlapL0 = subL0 & superL0;
        while (overlapL0) {
            int const maskBit = std::countr_zero(overlapL0);
            overlapL0 &= overlapL0 - 1;
            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            uint64_t const subL1 = subset.mSummaryMask[maskIdx];
            uint64_t const superL1 = superset.mSummaryMask[maskIdx];

            // --- LEVEL 1 CONFLICT PASS ---
            // Pages unique to subset inside a shared block. Must be Ghost Pages.
            uint64_t conflictL1 = subL1 & ~superL1;
            while (conflictL1) {
                int const bit = std::countr_zero(conflictL1);
                conflictL1 &= conflictL1 - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;
                if (subset.mArena[subset.mDirectory[dirIdx]].activeWordsMask != 0) {
                    return false;
                }
            }

            // --- LEVEL 1 OVERLAP PASS ---
            // Both sets share these physical pages; perform word-level checks.
            uint64_t overlapL1 = subL1 & superL1;
            while (overlapL1) {
                int const bit = std::countr_zero(overlapL1);
                overlapL1 &= overlapL1 - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                uint16_t const dirSub = subset.mDirectory[dirIdx];
                uint16_t const dirSuper = superset.mDirectory[dirIdx];

                auto const& pageSub = subset.mArena[dirSub];
                auto const& pageSuper = superset.mArena[dirSuper];

                // Subset cannot have active words that superset lacks
                if ((pageSub.activeWordsMask & ~pageSuper.activeWordsMask) != 0) {
                    return false;
                }

                uint64_t maskOverlap = pageSub.activeWordsMask & pageSuper.activeWordsMask;
                while (maskOverlap != 0) {
                    int const w = std::countr_zero(maskOverlap);
                    maskOverlap &= (maskOverlap - 1);
                    if ((pageSub.words[w] & ~pageSuper.words[w]) != 0) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

// Internal helpers for N-way population count without allocations.
// Uses a template to support both utils::Slice and std::array (for unrolling).
// - If Collection is std::array: Compiler unrolls the loops (Inlined Path).
// - If Collection is utils::Slice: Compiler generates a standard loop (Dynamic Path).

template<typename Collection>
PagedArenaBitset& PagedArenaBitset::intersectInternal(PagedArenaBitset* UTILS_RESTRICT out, const Collection& inputs) {
    assert(std::size(inputs) <= MAX_MULTI_WAY_INPUTS && "FATAL: Exceeded maximum multi-way bitset inputs!");

    out->clear();
    if (std::empty(inputs)) {
        return *out;
    }

    // If ANY operand is empty, the intersection is totally empty.
    for (auto const* input : inputs) {
        if (input->empty()) {
            return *out;
        }
    }

    size_t const count = std::size(inputs);

    // The Generic N-Way Loop
    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t masterOverlap = inputs[0]->mMasterMask[m];

        // For std::array<..., 2>, the compiler fully unrolls this loop and eliminates the branch.
        for (size_t i = 1; i < count; ++i) {
            masterOverlap &= inputs[i]->mMasterMask[m];
        }

        while (masterOverlap) {
            int const maskBit = std::countr_zero(masterOverlap);
            masterOverlap &= masterOverlap - 1;
            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            uint64_t overlap = inputs[0]->mSummaryMask[maskIdx];
            for (size_t i = 1; i < count; ++i) {
                overlap &= inputs[i]->mSummaryMask[maskIdx];
            }

            while (overlap) {
                int const bit = std::countr_zero(overlap);
                overlap &= overlap - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                out->mArena.emplace_back();
                auto& [activeWordsMask, words] = out->mArena.back();
                uint32_t pop = 0;

                uint16_t dirs[MAX_MULTI_WAY_INPUTS];
                for (size_t i = 0; i < count; ++i) {
                    dirs[i] = inputs[i]->mDirectory[dirIdx];
                }

                uint64_t overlapWords = inputs[0]->mArena[dirs[0]].activeWordsMask;
                for (size_t i = 1; i < count; ++i) {
                    overlapWords &= inputs[i]->mArena[dirs[i]].activeWordsMask;
                }

                while (overlapWords != 0) {
                    int const w = std::countr_zero(overlapWords);
                    overlapWords &= (overlapWords - 1); // Advance immediately

                    uint64_t word = inputs[0]->mArena[dirs[0]].words[w];
                    for (size_t i = 1; i < count; ++i) {
                        word &= inputs[i]->mArena[dirs[i]].words[w];
                    }

                    words[w] = word;
                    activeWordsMask |= (uint64_t(word != 0) << w);
                    pop += std::popcount(word);
                }

                if (pop > 0) {
                    uint16_t const newPageIdx = static_cast<uint16_t>(out->mArena.size() - 1);
                    out->mDirectory[dirIdx] = newPageIdx;
                    out->mSummaryMask[maskIdx] |= (1ULL << bit);
                    out->mSize += pop;
                    out->mMasterMask[m] |= (1ULL << maskBit);
                } else {
                    out->mArena.pop_back();
                }
            }
        }
    }
    return *out;
}

template<typename Collection>
uint32_t PagedArenaBitset::intersectSizeInternal(const Collection& inputs) {
    assert(std::size(inputs) <= MAX_MULTI_WAY_INPUTS && "FATAL: Exceeded maximum multi-way bitset inputs!");

    if (std::empty(inputs)) {
        return 0;
    }
    if (std::size(inputs) == 1) {
        return inputs[0]->size();
    }

    // If ANY operand is empty, the intersection is totally empty.
    for (auto const* input : inputs) {
        if (input->empty()) {
            return 0;
        }
    }

    uint32_t totalPop = 0;
    const size_t count = std::size(inputs);

    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t masterOverlap = inputs[0]->mMasterMask[m];
        for (size_t i = 1; i < count; ++i) {
            masterOverlap &= inputs[i]->mMasterMask[m];
            if (masterOverlap == 0) break;
        }

        while (masterOverlap) {
            int const maskBit = std::countr_zero(masterOverlap);
            masterOverlap &= masterOverlap - 1;
            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            uint64_t overlap = inputs[0]->mSummaryMask[maskIdx];
            for (size_t i = 1; i < count; ++i) {
                overlap &= inputs[i]->mSummaryMask[maskIdx];
                if (overlap == 0) break;
            }

            while (overlap) {
                int const bit = std::countr_zero(overlap);
                overlap &= overlap - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                uint16_t dirs[MAX_MULTI_WAY_INPUTS];
                for (size_t i = 0; i < count; ++i) {
                    dirs[i] = inputs[i]->mDirectory[dirIdx];
                }

                uint64_t overlapWords = inputs[0]->mArena[dirs[0]].activeWordsMask;
                for (size_t i = 1; i < count; ++i) {
                    overlapWords &= inputs[i]->mArena[dirs[i]].activeWordsMask;
                }

                while (overlapWords != 0) {
                    int const w = std::countr_zero(overlapWords);
                    uint64_t word = inputs[0]->mArena[dirs[0]].words[w];
                    for (size_t i = 1; i < count; ++i) {
                        word &= inputs[i]->mArena[dirs[i]].words[w];
                    }
                    totalPop += std::popcount(word);
                    overlapWords &= (overlapWords - 1);
                }
            }
        }
    }
    return totalPop;
}

/**
 * N-WAY MERGE (REDUCTION PATTERN)
 * -------------------------------
 * While a single-pass N-way traversal is theoretically O(N) at the hierarchy
 * level, it is physically constrained by CPU register pressure and pointer
 * indirection inside the hottest loops.
 *
 * We implement this as a Pairwise Reduction (copyFrom + merge chain) because:
 * 1. INSTRUCTION DENSITY: The 2-way merge loop is simple enough for the CPU
 *    to fully pipeline, unroll, and potentially vectorize.
 * 2. REGISTER ALLOCATION: 2-way ops allow the compiler to pin pointers in
 *    registers, avoiding the stack spills caused by iterating over a collection.
 * 3. BRANCH PREDICTABILITY: Removing the inner collection loop eliminates
 *    thousands of unpredictable micro-branches per page.
 *
 * Result: Observed 3.4x speedup over N-way traversal in 6-way merge tests.
 */
template<typename Collection>
PagedArenaBitset& PagedArenaBitset::mergeInternal(PagedArenaBitset* UTILS_RESTRICT out, const Collection& inputs) {
    out->clear();
    if (std::empty(inputs)) return *out;

    auto it = std::begin(inputs);
    auto end = std::end(inputs);

    // 1. Initialize the output with the first non-empty bitset
    // We use copyFrom to get memcpy-like performance for the first set
    while (it != end && (*it)->empty()) {
        ++it;
    }

    if (it == end) {
        return *out; // All were empty
    }

    out->copyFrom(**it);
    ++it;

    // 2. Reduce the rest using the optimized in-place 2-way merge
    for (; it != end; ++it) {
        if (!(*it)->empty()) {
            out->merge(**it);
        }
    }
    return *out;
}

template<typename Collection>
uint32_t PagedArenaBitset::mergeSizeInternal(const Collection& inputs) {
    assert(std::size(inputs) <= MAX_MULTI_WAY_INPUTS && "FATAL: Exceeded maximum multi-way bitset inputs!");

    if (std::empty(inputs)) return 0;
    const size_t count = std::size(inputs);
    if (count == 1) return inputs[0]->size();

    uint32_t totalPop = 0;

    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t masterUnion = 0;
        for (size_t i = 0; i < count; ++i) {
            masterUnion |= inputs[i]->mMasterMask[m];
        }

        while (masterUnion) {
            int const maskBit = std::countr_zero(masterUnion);
            masterUnion &= masterUnion - 1;
            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            uint64_t summaryUnion = 0;
            for (size_t i = 0; i < count; ++i) {
                summaryUnion |= inputs[i]->mSummaryMask[maskIdx];
            }

            while (summaryUnion) {
                int const bit = std::countr_zero(summaryUnion);
                summaryUnion &= summaryUnion - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                // --- OPTIMIZATION START ---
                // Collect pointers to active word arrays once per page.
                // This eliminates 'hasPage' checks inside the 64-word loop.
                const uint64_t* activePages[MAX_MULTI_WAY_INPUTS];
                uint64_t combinedMask = 0;
                uint32_t activeCount = 0;
                for (size_t i = 0; i < count; ++i) {
                    uint16_t const d = inputs[i]->mDirectory[dirIdx];
                    if (d != INVALID_PAGE) {
                        auto const& page = inputs[i]->mArena[d];
                        activePages[activeCount++] = page.words;
                        combinedMask |= page.activeWordsMask;
                    }
                }
                // --- OPTIMIZATION END ---

                while (combinedMask != 0) {
                    int const w = std::countr_zero(combinedMask);
                    uint64_t combinedWord = 0;
                    for (uint32_t i = 0; i < activeCount; ++i) {
                        combinedWord |= activePages[i][w];
                    }
                    totalPop += std::popcount(combinedWord);
                    combinedMask &= (combinedMask - 1);
                }
            }
        }
    }
    return totalPop;
}

// ----------------------------------------------------------------------------------------------------------------

PagedArenaBitset& PagedArenaBitset::intersect(PagedArenaBitset* UTILS_RESTRICT out,
        const PagedArenaBitset& a, const PagedArenaBitset& b) {
    assert(out != &a && out != &b);
    out->clear();
    if (a.empty() || b.empty()) {
        return *out;
    }

    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t masterOverlap = a.mMasterMask[m] & b.mMasterMask[m];

        while (masterOverlap) {
            int const maskBit = std::countr_zero(masterOverlap);
            masterOverlap &= masterOverlap - 1;

            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;
            uint64_t summaryOverlap = a.mSummaryMask[maskIdx] & b.mSummaryMask[maskIdx];

            while (summaryOverlap) {
                int const bit = std::countr_zero(summaryOverlap);
                summaryOverlap &= summaryOverlap - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                uint16_t const pageAIdx = a.mDirectory[dirIdx];
                uint16_t const pageBIdx = b.mDirectory[dirIdx];

                assert(pageAIdx != PagedArenaBitset::INVALID_PAGE && pageBIdx != PagedArenaBitset::INVALID_PAGE &&
                        "Summary mask desync");

                auto const& [activeWordsMaskA, wordsA] = a.mArena[pageAIdx];
                auto const& [activeWordsMaskB, wordsB] = b.mArena[pageBIdx];

                out->mArena.emplace_back();
                auto& [activeWordsMask, words] = out->mArena.back();
                uint32_t pop = 0;

                uint64_t wordsOverlap = activeWordsMaskA & activeWordsMaskB;
                while (wordsOverlap != 0) {
                    int const w = std::countr_zero(wordsOverlap);
                    wordsOverlap &= (wordsOverlap - 1); // Advance immediately

                    uint64_t const val = wordsA[w] & wordsB[w];
                    words[w] = val;
                    activeWordsMask |= (uint64_t(val != 0) << w);
                    pop += std::popcount(val);
                }

                if (pop > 0) {
                    uint16_t const newPageIdx = static_cast<uint16_t>(out->mArena.size() - 1);
                    out->mDirectory[dirIdx] = newPageIdx;
                    out->mSummaryMask[maskIdx] |= (1ULL << bit);
                    out->mSize += pop;
                    out->mMasterMask[m] |= (1ULL << maskBit);
                } else {
                    out->mArena.pop_back();
                }
            }
        }
    }
    return *out;
}

PagedArenaBitset PagedArenaBitset::intersect(const PagedArenaBitset& a, const PagedArenaBitset& b) {
    PagedArenaBitset out;
    out.reserve(std::min(a.mArena.size(), b.mArena.size()));
    intersect(&out, a, b);
    return out;
}

PagedArenaBitset& PagedArenaBitset::intersectSpan(PagedArenaBitset* UTILS_RESTRICT out,
        Slice<const PagedArenaBitset* const> const inputs) {
    return intersectInternal(out, inputs);
}

uint32_t PagedArenaBitset::intersectSizeSpan(Slice<const PagedArenaBitset* const> const inputs) {
    // Passes as a dynamic runtime span. The compiler will preserve the loops in `intersectSizeInternal`.
    return intersectSizeInternal(inputs);
}

// ----------------------------------------------------------------------------------------------------------------

PagedArenaBitset& PagedArenaBitset::difference(PagedArenaBitset* UTILS_RESTRICT out,
        const PagedArenaBitset& a, const PagedArenaBitset& b) {
    assert(out != &a && out != &b);
    out->clear();
    if (a.empty() || &a == &b) {
        return *out;
    }

    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t maskA = a.mMasterMask[m];
        while (maskA) {
            int const maskBit = std::countr_zero(maskA);
            maskA &= maskA - 1;
            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            uint64_t const sumA = a.mSummaryMask[maskIdx];
            uint64_t const sumB = b.mSummaryMask[maskIdx];

            uint64_t disjoint = sumA & ~sumB;
            uint64_t overlap = sumA & sumB;

            while (disjoint) {
                int const bit = std::countr_zero(disjoint);
                disjoint &= disjoint - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                const Page& pageA = a.mArena[a.mDirectory[dirIdx]];

                uint16_t const newPageIdx = static_cast<uint16_t>(out->mArena.size());
                out->mArena.push_back(pageA);

                out->mDirectory[dirIdx] = newPageIdx;
                out->mSummaryMask[maskIdx] |= (1ULL << bit);
                out->mSize += pageA.popcount();
                out->mMasterMask[m] |= (1ULL << maskBit);
            }

            while (overlap) {
                int const bit = std::countr_zero(overlap);
                overlap &= overlap - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                auto const& [activeWordsMaskA, wordsA] = a.mArena[a.mDirectory[dirIdx]];
                auto const& [activeWordsMaskB, wordsB] = b.mArena[b.mDirectory[dirIdx]];

                out->mArena.emplace_back();
                auto& [activeWordsMask, words] = out->mArena.back();
                uint32_t pop = 0;

                // Overlap
                uint64_t overlapWords = activeWordsMaskA & activeWordsMaskB;
                while (overlapWords != 0) {
                    int const w = std::countr_zero(overlapWords);
                    overlapWords &= (overlapWords - 1); // Advance immediately

                    uint64_t const val = wordsA[w] & ~wordsB[w];
                    words[w] = val;
                    activeWordsMask |= (uint64_t(val != 0) << w);
                    pop += std::popcount(val);
                }

                // Unique to A
                uint64_t uniqueA = activeWordsMaskA & ~activeWordsMaskB;
                while (uniqueA != 0) {
                    int const w = std::countr_zero(uniqueA);
                    uint64_t const val = wordsA[w];
                    words[w] = val;
                    activeWordsMask |= (1ULL << w);
                    pop += std::popcount(val);
                    uniqueA &= (uniqueA - 1);
                }

                if (pop > 0) {
                    uint16_t const newPageIdx = static_cast<uint16_t>(out->mArena.size() - 1);
                    out->mDirectory[dirIdx] = newPageIdx;
                    out->mSummaryMask[maskIdx] |= (1ULL << bit);
                    out->mSize += pop;
                    out->mMasterMask[m] |= (1ULL << maskBit);
                } else {
                    out->mArena.pop_back();
                }
            }
        }
    }
    return *out;
}

PagedArenaBitset PagedArenaBitset::difference(const PagedArenaBitset& a, const PagedArenaBitset& b) {
    PagedArenaBitset out;
    out.reserve(a.mArena.size());
    difference(&out, a, b);
    return out;
}

uint32_t PagedArenaBitset::differenceSize(const PagedArenaBitset& a, const PagedArenaBitset& b) noexcept {
    if (a.empty() || &a == &b) {
        return 0;
    }
    if (b.empty()) {
        return a.size();
    }

    uint32_t totalPop = 0;
    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t aMaster = a.mMasterMask[m];
        if (aMaster == 0) continue;

        while (aMaster) {
            int const maskBit = std::countr_zero(aMaster);
            aMaster &= aMaster - 1;
            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            uint64_t aSummary = a.mSummaryMask[maskIdx];
            uint64_t const bSummary = b.mSummaryMask[maskIdx];

            while (aSummary) {
                int const bit = std::countr_zero(aSummary);
                aSummary &= aSummary - 1;

                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;
                uint16_t const dirA = a.mDirectory[dirIdx];

                // OPTIMIZATION: Trust the L1 mask!
                // If B doesn't have this bit set, skip reading B's directory entirely.
                if ((bSummary & (1ULL << bit)) == 0) {
                    auto const& [activeWordsMaskA, wordsA] = a.mArena[dirA];
                    uint64_t maskA = activeWordsMaskA;
                    while (maskA != 0) {
                        int const w = std::countr_zero(maskA);
                        totalPop += std::popcount(wordsA[w]);
                        maskA &= (maskA - 1);
                    }
                } else {
                    uint16_t const dirB = b.mDirectory[dirIdx];
                    auto const& [activeWordsMaskA, wordsA] = a.mArena[dirA];
                    auto const& [activeWordsMaskB, wordsB] = b.mArena[dirB];

                    // Overlap
                    uint64_t overlap = activeWordsMaskA & activeWordsMaskB;
                    while (overlap != 0) {
                        int const w = std::countr_zero(overlap);
                        totalPop += std::popcount(wordsA[w] & ~wordsB[w]);
                        overlap &= (overlap - 1);
                    }

                    // Unique to A
                    uint64_t uniqueA = activeWordsMaskA & ~activeWordsMaskB;
                    while (uniqueA != 0) {
                        int const w = std::countr_zero(uniqueA);
                        totalPop += std::popcount(wordsA[w]);
                        uniqueA &= (uniqueA - 1);
                    }
                }
            }
        }
    }
    return totalPop;
}

// ----------------------------------------------------------------------------------------------------------------

PagedArenaBitset& PagedArenaBitset::merge(PagedArenaBitset* UTILS_RESTRICT out,
        const PagedArenaBitset& a, const PagedArenaBitset& b) {
    assert(out != &a && out != &b);

    if (a.empty()) {
        out->copyFrom(b);
        return *out;
    }
    if (b.empty()) {
        out->copyFrom(a);
        return *out;
    }

    out->clear();

    for (uint32_t m = 0; m < MASTER_WORDS; ++m) {
        uint64_t masterUnion = a.mMasterMask[m] | b.mMasterMask[m];
        while (masterUnion) {
            int const maskBit = std::countr_zero(masterUnion);
            masterUnion &= masterUnion - 1;
            uint32_t const maskIdx = (m << WORD_SHIFT) | maskBit;

            uint64_t summaryUnion = a.mSummaryMask[maskIdx] | b.mSummaryMask[maskIdx];
            while (summaryUnion) {
                int const bit = std::countr_zero(summaryUnion);
                summaryUnion &= summaryUnion - 1;
                uint32_t const dirIdx = (maskIdx << WORD_SHIFT) | bit;

                uint16_t const pageAIdx = a.mDirectory[dirIdx];
                uint16_t const pageBIdx = b.mDirectory[dirIdx];

                out->mArena.emplace_back();
                Page& newPage = out->mArena.back();
                uint32_t pop = 0;

                if (pageAIdx != INVALID_PAGE && pageBIdx != INVALID_PAGE) {
                    auto const& [activeWordsMaskA, wordsA] = a.mArena[pageAIdx];
                    auto const& [activeWordsMaskB, wordsB] = b.mArena[pageBIdx];

                    newPage.activeWordsMask = activeWordsMaskA | activeWordsMaskB;
                    uint64_t combinedMask = newPage.activeWordsMask;

                    while (combinedMask != 0) {
                        int const w = std::countr_zero(combinedMask);
                        newPage.words[w] = wordsA[w] | wordsB[w];
                        pop += std::popcount(newPage.words[w]);
                        combinedMask &= (combinedMask - 1);
                    }
                } else {
                    // Optimized: Use a reference to avoid a Page copy if pop ends up being 0
                    // (though for merge, if the page exists in A or B, pop is guaranteed > 0)
                    const Page& source = (pageAIdx != INVALID_PAGE) ? a.mArena[pageAIdx] : b.mArena[pageBIdx];
                    newPage = source;
                    pop = source.popcount(); // Ensure this uses std::popcount loop
                }

                if (pop > 0) {
                    // Only update masks and directory if the page actually contains data
                    uint16_t const newPageIdx = static_cast<uint16_t>(out->mArena.size() - 1);
                    out->mDirectory[dirIdx] = newPageIdx;

                    // Update masks lazily here
                    out->mSummaryMask[maskIdx] |= (1ULL << bit);
                    out->mMasterMask[m] |= (1ULL << maskBit);

                    out->mSize += pop;
                } else {
                    out->mArena.pop_back();
                }
            }
        }
    }
    return *out;
}

PagedArenaBitset PagedArenaBitset::merge(const PagedArenaBitset& a, const PagedArenaBitset& b) {
    PagedArenaBitset out;
    out.reserve(std::max(a.mArena.size(), b.mArena.size()));
    merge(&out, a, b);
    return out;
}

PagedArenaBitset& PagedArenaBitset::mergeSpan(PagedArenaBitset* UTILS_RESTRICT out,
        Slice<const PagedArenaBitset* const> const inputs) {
    return mergeInternal(out, inputs);
}

uint32_t PagedArenaBitset::mergeSizeSpan(Slice<const PagedArenaBitset* const> const inputs) {
    // Passes as a dynamic runtime span. The compiler will preserve the loops in `intersectSizeInternal`.
    return mergeSizeInternal(inputs);
}

void PagedArenaBitset::swap(PagedArenaBitset& other) noexcept {
    mSummaryMask.swap(other.mSummaryMask);
    mDirectory.swap(other.mDirectory);
    mArena.swap(other.mArena);
    mFreePages.swap(other.mFreePages);
    std::swap(mSize, other.mSize);
    std::swap(mMasterMask, other.mMasterMask);
}

PagedArenaBitset exchange(PagedArenaBitset& object, PagedArenaBitset&& newValue) {
    return std::exchange(object, std::move(newValue));
}

PagedArenaBitset exchangeAndClear(PagedArenaBitset& object) {
    // this is similar to std::move() but this leaves the object in the default constructed state (cleared)
    return std::exchange(object, {});
}

} // namespace utils
