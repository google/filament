/*
 * Copyright (C) 2025 The Android Open Source Project
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

#pragma once

#include <utils/compiler.h>

#include <algorithm>
#include <cstddef>
#include <iterator>

namespace filament {

/**
 * Finds and processes consecutive runs of equivalent elements in a range, compacting them in-place.
 *
 * This function iterates through the range [first, last) and identifies contiguous runs of elements
 * that are deemed equivalent by the `equivalent` predicate. Each run is then broken into smaller
 * chunks of at most `MaxRunSize`.
 *
 * For each chunk with more than one element, the `processor` is called. The first element of the
 * chunk is moved to its final, stable destination, and an iterator to this destination is passed
 * to the `processor`. This allows the processor to safely modify the representative element and
 * store a stable iterator to it.
 *
 * After processing, the range is compacted such that only the representative element of each
 * processed chunk (and all non-run elements) remain.
 *
 * This function is optimized for trivially copyable types by removing runtime checks against
 * self-move-assignment, which are unnecessary for such types.
 *
 * @tparam MaxRunSize The maximum size of a chunk that the processor can be called on.
 * @tparam Iterator The type of the iterators for the range.
 * @tparam BinaryPredicate A callable that takes two elements and returns true if they are equivalent.
 * @tparam Processor A callable that processes a chunk. Its signature is:
 *                   `void(Iterator dst, Iterator chunkBegin, Iterator chunkEnd)`
 *                   - `dst`: A stable iterator to the destination of the chunk's representative element.
 *                   - `chunkBegin`: An iterator to the beginning of the original chunk.
 *                   - `chunkEnd`: An iterator to the end of the original chunk.
 *
 * @param first An iterator to the beginning of the range.
 * @param last An iterator to the end of the range.
 * @param equivalent The binary predicate to check for equivalence between adjacent elements.
 * @param processor The callable to process each valid chunk.
 *
 * @return An iterator to the new logical end of the compacted range.
 *
 * @par Complexity
 * The function operates in O(N) time, where N is the number of elements in the range [first, last).
 * - **Best Case (no runs):** A single pass is made using `std::adjacent_find` to confirm there are
 *   no runs.
 * - **Worst Case (all elements are in runs):** Each element is visited and moved at most once.
 */

template<size_t MaxRunSize, typename Iterator, typename BinaryPredicate, typename Processor>
static Iterator compact(Iterator first, Iterator last,
        BinaryPredicate&& equivalent, Processor&& processor) {
    if (first == last) {
        return last;
    }

    // Fast path: find the first run. If no runs are found, we don't need to do anything.
    auto runBegin = std::adjacent_find(first, last, equivalent);
    if (runBegin == last) {
        return last;
    }

    // A run was found, so we now take the slow path.
    // Elements before runBegin are already in their correct places.
    auto dst = runBegin;
    auto src = runBegin;

    using value_type = std::iterator_traits<Iterator>::value_type;

    while (src != last) {
        // Find the end of the current run.
        auto runEnd = src + 1;
        while (runEnd != last && equivalent(*(runEnd - 1), *runEnd)) {
            ++runEnd;
        }

        auto runSize = std::distance(src, runEnd);

        if (UTILS_UNLIKELY(runSize > 1)) {
            // We have a run to process. Chunk it and process each chunk.
            for (auto chunkBegin = src; chunkBegin != runEnd; ) {
                auto chunkSize = std::min(size_t(std::distance(chunkBegin, runEnd)), MaxRunSize);
                auto chunkEnd = chunkBegin + chunkSize;

                if constexpr (std::is_trivially_copyable_v<value_type>) {
                    *dst = *chunkBegin;
                } else {
                    if (dst != chunkBegin) {
                        *dst = std::move(*chunkBegin);
                    }
                }

                if (chunkSize > 1) {
                    processor(dst, chunkBegin, chunkEnd);
                }
                ++dst;
                chunkBegin = chunkEnd;
            }
        } else {
            // Not a run, just a single element. Move it to the destination.
            if constexpr (std::is_trivially_copyable_v<value_type>) {
                *dst = *src;
            } else {
                if (dst != src) {
                    *dst = std::move(*src);
                }
            }
            ++dst;
        }
        src = runEnd;
    }
    return dst;
}

} // namespace filament
