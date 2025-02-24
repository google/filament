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

package cov_test

import (
	"reflect"
	"testing"

	"swiftshader.googlesource.com/SwiftShader/tests/regres/cov"
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
