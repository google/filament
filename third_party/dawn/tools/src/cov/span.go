// Copyright 2022 The Dawn & Tint Authors
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

package cov

import (
	"fmt"
	"sort"
)

// Location describes a single line-column position in a source file.
type Location struct {
	Line, Column int
}

func (l Location) String() string {
	return fmt.Sprintf("%v:%v", l.Line, l.Column)
}

// Compare returns -1 if l comes before o, 1 if l comes after o, otherwise 0.
func (l Location) Compare(o Location) int {
	switch {
	case l.Line < o.Line:
		return -1
	case l.Line > o.Line:
		return 1
	case l.Column < o.Column:
		return -1
	case l.Column > o.Column:
		return 1
	}
	return 0
}

// Before returns true if l comes before o.
func (l Location) Before(o Location) bool { return l.Compare(o) == -1 }

// After returns true if l comes after o.
func (l Location) After(o Location) bool { return l.Compare(o) == 1 }

// Span describes a start and end interval in a source file.
type Span struct {
	Start, End Location
}

func (s Span) String() string {
	return fmt.Sprintf("%v-%v", s.Start, s.End)
}

// Compare returns -1 if l comes before o, 1 if l comes after o, otherwise 0.
func (s Span) Compare(o Span) int {
	switch {
	case s.Start.Before(o.Start):
		return -1
	case o.Start.Before(s.Start):
		return 1
	case s.End.Before(o.End):
		return -1
	case o.End.Before(s.End):
		return 1
	}
	return 0
}

// Before returns true if span s comes before o.
func (s Span) Before(o Span) bool { return s.Compare(o) == -1 }

// Inside returns true if span s fits entirely inside o.
func (s Span) Inside(o Span) bool { return s.Start.Compare(o.Start) >= 0 && s.End.Compare(o.End) <= 0 }

// SpanList is a sorted list of spans. Use SpanList.Add() to insert new spans.
type SpanList []Span

// Add adds the Span to the SpanList, merging and expanding overlapping spans.
func (l *SpanList) Add(s Span) {
	//          [===]
	//  [0] [1]                | idxStart: 2 |  idxEnd: 2
	//                [0] [1]  | idxStart: 0 |  idxEnd: 0
	// [ 0 ] [ 1 ] [ 2 ] [ 3 ] | idxStart: 1 |  idxEnd: 2
	// [0]  [1]  [2]  [3]  [4] | idxStart: 2 |  idxEnd: 2
	idxStart := sort.Search(len(*l), func(i int) bool { return (*l)[i].End.Compare(s.Start) >= 0 })

	if idxStart < len(*l) && s.Inside((*l)[idxStart]) {
		return // No change.
	}

	idxEnd := sort.Search(len(*l), func(i int) bool { return (*l)[i].Start.Compare(s.End) > 0 })

	if idxStart < idxEnd {
		if first := (*l)[idxStart]; first.Start.Before(s.Start) {
			s.Start = first.Start
		}
		if last := (*l)[idxEnd-1]; last.End.After(s.End) {
			s.End = last.End
		}
	}

	merged := append(SpanList{}, (*l)[:idxStart]...)
	merged = append(merged, s)
	merged = append(merged, (*l)[idxEnd:]...)
	*l = merged
}

// Remove cuts out the Span from the SpanList, removing and trimming overlapping
// spans.
func (l *SpanList) Remove(s Span) {
	if s.Start == s.End {
		return // zero length == no split.
	}

	//          [===]
	//  [0] [1]                | idxStart: 2 |  idxEnd: 2
	//                [0] [1]  | idxStart: 0 |  idxEnd: 0
	// [ 0 ] [ 1 ] [ 2 ] [ 3 ] | idxStart: 1 |  idxEnd: 2
	// [0]  [1]  [2]  [3]  [4] | idxStart: 2 |  idxEnd: 2
	idxStart := sort.Search(len(*l), func(i int) bool { return (*l)[i].End.Compare(s.Start) > 0 })
	idxEnd := sort.Search(len(*l), func(i int) bool { return (*l)[i].Start.Compare(s.End) >= 0 })

	merged := append(SpanList{}, (*l)[:idxStart]...)

	if idxStart < idxEnd {
		first, last := (*l)[idxStart], (*l)[idxEnd-1]
		if first.Start.Compare(s.Start) < 0 {
			merged = append(merged, Span{first.Start, s.Start})
		}
		if last.End.Compare(s.End) > 0 {
			merged = append(merged, Span{s.End, last.End})
		}
	}

	merged = append(merged, (*l)[idxEnd:]...)
	*l = merged
}

// Compare returns -1 if l comes before o, 1 if l comes after o, otherwise 0.
func (l SpanList) Compare(o SpanList) int {
	switch {
	case len(l) < len(o):
		return -1
	case len(l) > len(o):
		return 1
	}
	for i, a := range l {
		switch a.Compare(o[i]) {
		case -1:
			return -1
		case 1:
			return 1
		}
	}
	return 0
}

// NumLines returns the total number of lines covered by all spans in the list.
func (l SpanList) NumLines() int {
	seen := map[int]struct{}{}
	for _, span := range l {
		for s := span.Start.Line; s <= span.End.Line; s++ {
			if _, ok := seen[s]; !ok {
				seen[s] = struct{}{}
			}
		}
	}
	return len(seen)
}
