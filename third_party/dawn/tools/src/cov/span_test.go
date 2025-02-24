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

package cov_test

import (
	"reflect"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cov"
)

func TestSpanListAddNoMerge(t *testing.T) {
	l := cov.SpanList{}
	l.Add(span(3, 1, 3, 5))
	checkSpanList(t, l, span(3, 1, 3, 5))

	l.Add(span(4, 1, 4, 5))
	checkSpanList(t, l, span(3, 1, 3, 5), span(4, 1, 4, 5))

	l.Add(span(2, 1, 2, 5))
	checkSpanList(t, l, span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5))
}

func TestSpanListAddExpand(t *testing.T) {
	l := cov.SpanList{span(1, 1, 1, 5), span(5, 4, 5, 7), span(9, 1, 9, 5)}

	// Expand front (column)
	l.Add(span(5, 1, 5, 5))
	checkSpanList(t, l, span(1, 1, 1, 5), span(5, 1, 5, 7), span(9, 1, 9, 5))

	// Expand back (column)
	l.Add(span(5, 5, 5, 9))
	checkSpanList(t, l, span(1, 1, 1, 5), span(5, 1, 5, 9), span(9, 1, 9, 5))

	// Expand front (line)
	l.Add(span(4, 3, 5, 2))
	checkSpanList(t, l, span(1, 1, 1, 5), span(4, 3, 5, 9), span(9, 1, 9, 5))

	// Expand back (line)
	l.Add(span(5, 4, 6, 3))
	checkSpanList(t, l, span(1, 1, 1, 5), span(4, 3, 6, 3), span(9, 1, 9, 5))

	// Expand front (touching)
	l.Add(span(4, 2, 4, 3))
	checkSpanList(t, l, span(1, 1, 1, 5), span(4, 2, 6, 3), span(9, 1, 9, 5))

	// Expand back (touching)
	l.Add(span(6, 3, 6, 4))
	checkSpanList(t, l, span(1, 1, 1, 5), span(4, 2, 6, 4), span(9, 1, 9, 5))
}

func TestSpanListAddMergeOverlap(t *testing.T) {
	l := cov.SpanList{span(1, 1, 1, 5), span(5, 4, 5, 7), span(9, 1, 9, 5)}

	l.Add(span(1, 3, 5, 6))
	checkSpanList(t, l, span(1, 1, 5, 7), span(9, 1, 9, 5))

	l.Add(span(5, 5, 9, 3))
	checkSpanList(t, l, span(1, 1, 9, 5))
}

func TestSpanListAddMergeTouching(t *testing.T) {
	l := cov.SpanList{span(1, 1, 1, 5), span(5, 4, 5, 7), span(9, 1, 9, 5)}

	l.Add(span(1, 5, 9, 1))
	checkSpanList(t, l, span(1, 1, 9, 5))
}

func TestSpanListRemoveNothing(t *testing.T) {
	l := cov.SpanList{span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5)}

	l.Remove(span(1, 1, 2, 1))
	checkSpanList(t, l, span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5))

	l.Remove(span(2, 5, 3, 1))
	checkSpanList(t, l, span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5))

	l.Remove(span(3, 5, 4, 1))
	checkSpanList(t, l, span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5))

	l.Remove(span(4, 5, 10, 10))
	checkSpanList(t, l, span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5))
}

func TestSpanListRemoveWhole(t *testing.T) {
	l := cov.SpanList{span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5)}

	l.Remove(span(3, 1, 3, 5))
	checkSpanList(t, l, span(2, 1, 2, 5), span(4, 1, 4, 5))

	l.Remove(span(1, 1, 3, 3))
	checkSpanList(t, l, span(4, 1, 4, 5))

	l.Remove(span(3, 1, 4, 5))
	checkSpanList(t, l)
}

func TestSpanListRemoveZeroLength(t *testing.T) {
	l := cov.SpanList{span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5)}

	l.Remove(span(3, 1, 3, 1))
	checkSpanList(t, l, span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5))

	l.Remove(span(3, 5, 3, 5))
	checkSpanList(t, l, span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5))
}

func TestSpanListRemoveTrim(t *testing.T) {
	l := cov.SpanList{span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5)}

	l.Remove(span(2, 1, 2, 2))
	checkSpanList(t, l, span(2, 2, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5))

	l.Remove(span(2, 4, 2, 5))
	checkSpanList(t, l, span(2, 2, 2, 4), span(3, 1, 3, 5), span(4, 1, 4, 5))

	l.Remove(span(2, 5, 3, 2))
	checkSpanList(t, l, span(2, 2, 2, 4), span(3, 2, 3, 5), span(4, 1, 4, 5))

	l.Remove(span(3, 4, 3, 5))
	checkSpanList(t, l, span(2, 2, 2, 4), span(3, 2, 3, 4), span(4, 1, 4, 5))

	l.Remove(span(4, 1, 4, 2))
	checkSpanList(t, l, span(2, 2, 2, 4), span(3, 2, 3, 4), span(4, 2, 4, 5))

	l.Remove(span(4, 4, 4, 5))
	checkSpanList(t, l, span(2, 2, 2, 4), span(3, 2, 3, 4), span(4, 2, 4, 4))
}

func TestSpanListRemoveSplit(t *testing.T) {
	l := cov.SpanList{span(2, 1, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5)}

	l.Remove(span(2, 2, 2, 3))
	checkSpanList(t, l, span(2, 1, 2, 2), span(2, 3, 2, 5), span(3, 1, 3, 5), span(4, 1, 4, 5))

	l.Remove(span(3, 2, 3, 4))
	checkSpanList(t, l, span(2, 1, 2, 2), span(2, 3, 2, 5), span(3, 1, 3, 2), span(3, 4, 3, 5), span(4, 1, 4, 5))

	l.Remove(span(4, 2, 4, 2)) // zero length == no split
	checkSpanList(t, l, span(2, 1, 2, 2), span(2, 3, 2, 5), span(3, 1, 3, 2), span(3, 4, 3, 5), span(4, 1, 4, 5))
}

func span(startLine, startColumn, endLine, endColumn int) cov.Span {
	return cov.Span{
		Start: cov.Location{Line: startLine, Column: startColumn},
		End:   cov.Location{Line: endLine, Column: endColumn},
	}
}

func checkSpanList(t *testing.T, got cov.SpanList, expect ...cov.Span) {
	if expect == nil {
		expect = cov.SpanList{}
	}
	if !reflect.DeepEqual(got, cov.SpanList(expect)) {
		t.Errorf("SpanList not as expected.\nGot:\n%v\nExpect:\n%v", got, cov.SpanList(expect))
	}
}
