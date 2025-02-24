// Copyright 2021 The Dawn & Tint Authors
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

// Package lut provides a look up table, which compresses indexed data
package lut

import (
	"sort"
)

// LUT is a look up table.
// The table holds a number of items that are stored in a linear list.
type LUT[T comparable] interface {
	// Add adds a sequence of items to the table.
	// The sequence of items stored at [offset, offset+N), where N is the
	// number of items added will remain equal, even after calling Compact().
	// Returns a pointer to the start index in the list.
	Add(items []T) *int
	// Compact reorders the table items so that the table storage is compacted
	// by shuffling data around and de-duplicating sequences of common data.
	// Each originally added sequence is preserved in the resulting table, with
	// the same contiguous ordering, but with a potentially different offset.
	// Heuristics are used to shorten the table length, by exploiting common
	// subsequences, and removing duplicate sequences.
	// Note that shortest common superstring is NP-hard, so heuristics are used.
	// Compact updates pointers returned by Add().
	Compact() []T
}

// New returns a new look up table
func New[T comparable]() LUT[T] {
	return &lut[T]{storage: []T{}}
}

// A sequence represents a span of entries in the table
type sequence struct {
	offset *int // Pointer to the start index of the sequence
	count  int  // Length of the sequence
}

// lut implements LUT
type lut[T comparable] struct {
	storage   []T        // The storage
	sequences []sequence // The entries in the LUT
}

func (t *lut[T]) Add(items []T) *int {
	if len(items) == 0 {
		return nil
	}

	offset := len(t.storage)
	count := len(items)
	t.storage = append(t.storage, items...)
	offsetPtr := &offset
	t.sequences = append(t.sequences, sequence{offsetPtr, count})
	return offsetPtr
}

// match describes a sequence that can be placed.
type match struct {
	dst int      // destination offset
	src sequence // source sequence
	len int      // number of items that matched
	idx int      // sequence index
}

func (t lut[T]) Compact() []T {
	// Generate int32 identifiers for each unique item in the table.
	// We use these to compare items instead of comparing the real data as this
	// function is comparison-heavy, and integer compares are cheap.
	srcIDs := t.itemIDs()
	dstIDs := make([]int32, len(srcIDs))

	// Make a copy the data held in the table, use the copy as the source, and
	// t.storage as the destination.
	srcData := make([]T, len(t.storage))
	copy(srcData, t.storage)
	dstData := t.storage

	// Sort all the sequences by length, with the largest first.
	// This helps 'seed' the compacted form with the largest items first.
	// This can improve the compaction as small sequences can pack into larger,
	// placed items.
	sort.SliceStable(t.sequences, func(i, j int) bool {
		return t.sequences[i].count > t.sequences[j].count
	})

	// unplaced is the list of sequences that have not yet been placed.
	// All sequences are initially unplaced.
	unplaced := make([]sequence, len(t.sequences))
	copy(unplaced, t.sequences)

	// placed is the list of sequences that have been placed.
	// Nothing is initially placed.
	placed := make([]sequence, 0, len(t.sequences))

	// remove removes the sequence in unplaced with the index i.
	remove := func(i int) {
		placed = append(placed, unplaced[i])
		if i > 0 {
			if i < len(unplaced)-1 {
				copy(unplaced[i:], unplaced[i+1:])
			}
			unplaced = unplaced[:len(unplaced)-1]
		} else {
			unplaced = unplaced[1:]
		}
	}

	// cp copies data from [srcOffset:srcOffset+count] to [dstOffset:dstOffset+count].
	cp := func(dstOffset, srcOffset, count int) {
		copy(dstData[dstOffset:dstOffset+count], srcData[srcOffset:srcOffset+count])
		copy(dstIDs[dstOffset:dstOffset+count], srcIDs[srcOffset:srcOffset+count])
	}

	// number of items that have been placed.
	newSize := 0

	// While there's sequences to place...
	for len(unplaced) > 0 {
		// Place the next largest, unplaced sequence at the end of the new list
		cp(newSize, *unplaced[0].offset, unplaced[0].count)
		*unplaced[0].offset = newSize
		newSize += unplaced[0].count
		remove(0)

		for {
			// Look for the sequence with the longest match against the
			// currently placed data. Any mismatches with currently placed data
			// will nullify the match. The head or tail of this sequence may
			// extend the currently placed data.
			best := match{}

			// For each unplaced sequence...
			for i := 0; i < len(unplaced); i++ {
				seq := unplaced[i]

				if best.len >= seq.count {
					// The best match is already at least as long as this
					// sequence and sequences are sorted by size, so best cannot
					// be beaten. Stop searching.
					break
				}

				// Perform a full sweep from left to right, scoring the match...
				for shift := -seq.count + 1; shift < newSize; shift++ {
					dstS := max(shift, 0)
					dstE := min(shift+seq.count, newSize)
					count := dstE - dstS
					srcS := *seq.offset - min(shift, 0)
					srcE := srcS + count

					if best.len < count {
						if equal(srcIDs[srcS:srcE], dstIDs[dstS:dstE]) {
							best = match{shift, seq, count, i}
						}
					}
				}
			}

			if best.src.offset == nil {
				// Nothing matched. Not even one element.
				// Resort to placing the next largest sequence at the end.
				break
			}

			if best.dst < 0 {
				// Best match wants to place the sequence to the left of the
				// current output. We have to shuffle everything...
				n := -best.dst
				copy(dstData[n:n+newSize], dstData)
				copy(dstIDs[n:n+newSize], dstIDs)
				newSize += n
				best.dst = 0
				for _, p := range placed {
					*p.offset += n
				}
			}

			// Place the best matching sequence.
			cp(best.dst, *best.src.offset, best.src.count)
			newSize = max(newSize, best.dst+best.src.count)
			*best.src.offset = best.dst
			remove(best.idx)
		}
	}

	// Shrink the output buffer to the new size.
	dstData = dstData[:newSize]

	// All done.
	return dstData
}

// Generate a set of identifiers for all the unique items in storage
func (t lut[T]) itemIDs() []int32 {
	storageSize := len(t.storage)
	keys := make([]int32, storageSize)
	dataToKey := map[T]int32{}
	for i := 0; i < storageSize; i++ {
		data := t.storage[i]
		key, found := dataToKey[data]
		if !found {
			key = int32(len(dataToKey))
			dataToKey[data] = key
		}
		keys[i] = key
	}
	return keys
}

func max(a, b int) int {
	if a < b {
		return b
	}
	return a
}

func min(a, b int) int {
	if a > b {
		return b
	}
	return a
}

func equal(a, b []int32) bool {
	for i, v := range a {
		if b[i] != v {
			return false
		}
	}
	return true
}
