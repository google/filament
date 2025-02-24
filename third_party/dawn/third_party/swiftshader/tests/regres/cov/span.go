// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
