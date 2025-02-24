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
	"strings"
	"testing"

	"swiftshader.googlesource.com/SwiftShader/tests/regres/cov"
)

var (
	fileA = "coverage/file/a"
	fileB = "coverage/file/b"
	fileC = "coverage/file/c"
	fileD = "coverage/file/c"

	span0 = cov.Span{cov.Location{3, 2}, cov.Location{3, 9}}
	span1 = cov.Span{cov.Location{4, 1}, cov.Location{5, 1}}
	span2 = cov.Span{cov.Location{5, 5}, cov.Location{5, 7}}
	span3 = cov.Span{cov.Location{7, 2}, cov.Location{7, 7}}
)

//                a
//        ╭───────┴───────╮
//        b               c
//    ╭───┴───╮       ╭───┴───╮
//    d       e       f       g
//  ╭─┴─╮   ╭─┴─╮   ╭─┴─╮   ╭─┴─╮
//  h   i   j   k   l   m   n   o
//     ╭┴╮ ╭┴╮ ╭┴╮ ╭┴╮ ╭┴╮ ╭╯
//     p q r s t u v w x y z
//

func TestTree(t *testing.T) {
	tree := &cov.Tree{}

	t.Log("Add 'b' with the coverage [0,1]")
	tree.Add(cov.Path{"a", "b"}, coverage(fileA, span0, span1))

	//           [0,1]
	//            (a)
	//       ╭─────╯
	//       b

	checkSpans(t, tree.Spans(), span0, span1)
	checkTests(t, tree, `{a:{b}}`)
	checkCoverage(t, tree, fileA, `a:{[0,1]}`)

	t.Log("Add 'i' with the coverage [0,1]")
	tree.Add(cov.Path{"a", "b", "d", "i"}, coverage(fileA, span0, span1))

	//           [0,1]
	//            (a)
	//       ╭─────╯
	//       b
	//    ╭──╯
	//    d
	//    ╰─╮
	//      i
	checkSpans(t, tree.Spans(), span0, span1)
	checkTests(t, tree, `{a:{b:{d:{i}}}}`)
	checkCoverage(t, tree, fileA, `a:{[0,1]}`)

	t.Log("Add 'e' with the coverage [0,1,2]")
	tree.Add(cov.Path{"a", "b", "e"}, coverage(fileA, span0, span1, span2))

	//           [0,1]
	//            (a)
	//       ┏━━━━━┛
	//      (b)
	//    ╭──┺━━┓
	//    d    (e)[2]
	//    ╰─╮
	//      i
	checkSpans(t, tree.Spans(), span0, span1, span2)
	checkTests(t, tree, `{a:{b:{d:{i} e}}}`)
	checkCoverage(t, tree, fileA, `a:{[0,1] b:{e:{[2]}}}`)

	t.Log("Add 'n' with the coverage [0,3]")
	tree.Add(cov.Path{"a", "c", "g", "n"}, coverage(fileA, span0, span3))

	//            [0]
	//            (a)
	//       ┏━━━━━┻━━━━━┓
	//   [1](b)         (c)[3]
	//    ╭──┺━━┓        ╰──╮
	//    d    (e)[2]       g
	//    ╰─╮             ╭─╯
	//      i             n
	checkSpans(t, tree.Spans(), span0, span1, span2, span3)
	checkTests(t, tree, `{a:{b:{d:{i}e}c:{g:{n}}}}`)
	checkCoverage(t, tree, fileA, `a:{[0] b:{[1] e:{[2]}} c:{[3]}}`)

	t.Log("Add 'o' with the coverage [0, 3]")
	tree.Add(cov.Path{"a", "c", "g", "o"}, coverage(fileA, span0, span3))

	//              [0]
	//              (a)
	//       ┏━━━━━━━┻━━━━━━━┓
	//   [1](b)             (c)[3]
	//    ╭──┺━━┓            ╰──╮
	//    d    (e)[2]           g
	//    ╰─╮                 ╭─┴─╮
	//      i                 n   o
	checkSpans(t, tree.Spans(), span0, span1, span2, span3)
	checkTests(t, tree, `{a:{b:{d:{i}e}c:{g:{n o}}}}`)
	checkCoverage(t, tree, fileA, `a:{[0] b:{[1] e:{[2]}} c:{[3]}}`)

	t.Log("Add 'f' with the coverage [1]")
	tree.Add(cov.Path{"a", "c", "f"}, coverage(fileA, span1))

	//               (a)
	//       ┏━━━━━━━━┻━━━━━━━━┓
	// [0,1](b)               (c)
	//    ╭──┺━━┓           ┏━━┻━━┓
	//    d    (e)[2]   [1](f)   (g)[0,3]
	//    ╰─╮                   ╭─┴─╮
	//      i                   n   o
	checkSpans(t, tree.Spans(), span0, span1, span2, span3)
	checkTests(t, tree, `{a:{b:{d:{i} e} c:{f g:{n o}}}}`)
	checkCoverage(t, tree, fileA, `a:{b:{[0,1] e:{[2]}} c:{f:{[1]} g:{[0,3]}}}`)

	t.Log("Add 'j' with the coverage [3]")
	tree.Add(cov.Path{"a", "b", "e", "j"}, coverage(fileA, span3))

	//                   (a)
	//           ┏━━━━━━━━┻━━━━━━━━┓
	//          (b)               (c)
	//       ┏━━━┻━━━┓          ┏━━┻━━┓
	// [0,1](d)     (e)[3]  [1](f)   (g)[0,3]
	//       ╰─╮   ╭─╯              ╭─┴─╮
	//         i   j                n   o
	checkSpans(t, tree.Spans(), span0, span1, span2, span3)
	checkTests(t, tree, `{a:{b:{d:{i} e:{j}} c:{f g:{n o}}}}`)
	checkCoverage(t, tree, fileA, `a:{b:{d:{[0,1]} e:{[3]}} c:{f:{[1]} g:{[0,3]}}}`)

	t.Log("Add 'k' with the coverage [3]")
	tree.Add(cov.Path{"a", "b", "e", "k"}, coverage(fileA, span3))

	//                   (a)
	//           ┏━━━━━━━━┻━━━━━━━━┓
	//          (b)               (c)
	//       ┏━━━┻━━━┓          ┏━━┻━━┓
	// [0,1](d)     (e)[3]  [1](f)   (g)[0,3]
	//       ╰─╮   ╭─┴─╮            ╭─┴─╮
	//         i   j   k            n   o
	checkSpans(t, tree.Spans(), span0, span1, span2, span3)
	checkTests(t, tree, `{a:{b:{d:{i} e:{j k}} c:{f g:{n o}}}}`)
	checkCoverage(t, tree, fileA, `a:{b:{d:{[0,1]} e:{[3]}} c:{f:{[1]} g:{[0,3]}}}`)

	t.Log("Add 'v' with the coverage [1,2]")
	tree.Add(cov.Path{"a", "c", "f", "l", "v"}, coverage(fileA, span1, span2))

	//                   (a)
	//           ┏━━━━━━━━┻━━━━━━━━━━┓
	//          (b)                 (c)
	//       ┏━━━┻━━━┓            ┏━━┻━━┓
	// [0,1](d)     (e)[3]  [1,2](f)   (g)[0,3]
	//       ╰─╮   ╭─┴─╮        ╭─╯   ╭─┴─╮
	//         i   j   k        l     n   o
	//                         ╭╯
	//                         v
	checkSpans(t, tree.Spans(), span0, span1, span2, span3)
	checkTests(t, tree, `{a:{b:{d:{i} e:{j k}} c:{f:{l:{v}} g:{n o}}}}`)
	checkCoverage(t, tree, fileA, `a:{b:{d:{[0,1]} e:{[3]}} c:{f:{[1,2]} g:{[0,3]}}}`)

	t.Log("Add 'x' with the coverage [1,2]")
	tree.Add(cov.Path{"a", "c", "f", "l", "x"}, coverage(fileA, span1, span2))

	//                   (a)
	//           ┏━━━━━━━━┻━━━━━━━━━━┓
	//          (b)                 (c)
	//       ┏━━━┻━━━┓            ┏━━┻━━┓
	// [0,1](d)     (e)[3]  [1,2](f)   (g)[0,3]
	//       ╰─╮   ╭─┴─╮        ╭─╯   ╭─┴─╮
	//         i   j   k        l     n   o
	//                         ╭┴╮
	//                         v x
	checkSpans(t, tree.Spans(), span0, span1, span2, span3)
	checkTests(t, tree, `{a:{b:{d:{i} e:{j k}} c:{f:{l:{v x}} g:{n o}}}}`)
	checkCoverage(t, tree, fileA, `a:{b:{d:{[0,1]} e:{[3]}} c:{f:{[1,2]} g:{[0,3]}}}`)

	t.Log("Add 'z' with the coverage [2]")
	tree.Add(cov.Path{"a", "c", "g", "n", "z"}, coverage(fileA, span2))

	//                   (a)
	//           ┏━━━━━━━━┻━━━━━━━━━━━━┓
	//          (b)                   (c)
	//       ┏━━━┻━━━┓            ┏━━━━┻━━━━┓
	// [0,1](d)     (e)[3]  [1,2](f)       (g)
	//       ╰─╮   ╭─┴─╮        ╭─╯       ┏━┻━┓
	//         i   j   k        l    [2](n) (o)[0,3]
	//                         ╭┴╮      ╭╯
	//                         v x      z
	checkSpans(t, tree.Spans(), span0, span1, span2, span3)
	checkTests(t, tree, `{a:{b:{d:{i} e:{j k}} c:{f:{l:{v x}} g:{n: {z} o}}}}`)
	checkCoverage(t, tree, fileA, `a:{b:{d:{[0,1]} e:{[3]}} c:{f:{[1,2]} g:{n:{[2]} o:{[0,3]}}}}`)

	tree.Optimize()

	//                   (a)
	//           ┏━━━━━━━━┻━━━━━━━━━━━━┓
	//          (b)                   (c)
	//       ┏━━━┻━━━┓            ┏━━━━┻━━━━┓
	//   <0>(d)     (e)[3]    <2>(f)       (g)
	//       ╰─╮   ╭─┴─╮        ╭─╯       ┏━┻━┓
	//         i   j   k        l    [2](n) (o)<1>
	//                         ╭┴╮      ╭╯
	//                         v x      z
	checkSpans(t, tree.Spans(), span0, span1, span2, span3)
	checkGroups(t, tree.FileSpanGroups(fileA), map[cov.SpanGroupID]cov.SpanGroup{
		0: cov.SpanGroup{Spans: spans(0, 1)},
		1: cov.SpanGroup{Spans: spans(0, 3)},
		2: cov.SpanGroup{Spans: spans(1, 2)},
	})
	checkTests(t, tree, `{a:{b:{d:{i} e:{j k}} c:{f:{l:{v x}} g:{n: {z} o}}}}`)
	checkCoverage(t, tree, fileA, `a:{b:{d:{<0>} e:{[3]}} c:{f:{<2>} g:{n:{[2]} o:{<1>}}}}`)
}

func TestTreeOptInvertForCommon(t *testing.T) {
	tree := &cov.Tree{}

	tree.Add(cov.Path{"a", "b"}, coverage(fileA, span0))
	tree.Add(cov.Path{"a", "c"}, coverage(fileA, span0))
	tree.Add(cov.Path{"a", "d"}, coverage(fileA, span0))
	tree.Add(cov.Path{"a", "e"}, coverage(fileA, span1))
	tree.Add(cov.Path{"a", "f"}, coverage(fileA, span1))
	tree.Add(cov.Path{"a", "g"}, coverage(fileA, span0))
	tree.Add(cov.Path{"a", "h"}, coverage(fileA, span0))
	tree.Add(cov.Path{"a", "i"}, coverage(fileA, span0))

	//               (a)
	//  ┏━━━┳━━━┳━━━┳━┻━┳━━━┳━━━┳━━━┓
	// (b) (c) (d) (e) (f) (g) (h) (i)
	// [0] [0] [0] [1] [1] [0] [0] [0]
	checkSpans(t, tree.Spans(), span0, span1)
	checkTests(t, tree, `{a:{b c d e f g h i}}`)
	checkCoverage(t, tree, fileA, `a:{b:{[0]} c:{[0]} d:{[0]} e:{[1]} f:{[1]} g:{[0]} h:{[0]} i:{[0]}}`)

	tree.Optimize()

	//               [0]
	//               (a)
	//  ╭───┬───┬───┲━┻━┱───┬───┬───╮
	//  b   c   d  ┏┛   ┗┓  g   h   i
	//            (e)   (f)
	//            <0>   <0>
	checkSpans(t, tree.Spans(), span0, span1)
	checkGroups(t, tree.FileSpanGroups(fileA), map[cov.SpanGroupID]cov.SpanGroup{
		0: cov.SpanGroup{Spans: spans(0, 1)},
	})
	checkTests(t, tree, `{a:{b c d e f g h i}}`)
	checkCoverage(t, tree, fileA, `a:{[0] e:{<0>} f:{<0>}}`)
}

func TestTreeOptDontInvertForCommon(t *testing.T) {
	tree := &cov.Tree{}

	tree.Add(cov.Path{"a", "b"}, coverage(fileA, span0))
	tree.Add(cov.Path{"a", "c"}, coverage(fileA, span0))
	tree.Add(cov.Path{"a", "d"}, coverage(fileA, span0))
	tree.Add(cov.Path{"a", "e"}, coverage(fileA, span1))
	tree.Add(cov.Path{"a", "f"}, coverage(fileA, span1))
	tree.Add(cov.Path{"a", "g"}, coverage(fileA, span2))
	tree.Add(cov.Path{"a", "h"}, coverage(fileA, span2))
	tree.Add(cov.Path{"a", "i"}, coverage(fileA, span2))

	//               (a)
	//  ┏━━━┳━━━┳━━━┳━┻━┳━━━┳━━━┳━━━┓
	// (b) (c) (d) (e) (f) (g) (h) (i)
	// [0] [0] [0] [1] [1] [2] [2] [2]
	checkSpans(t, tree.Spans(), span0, span1, span2)
	checkTests(t, tree, `{a:{b c d e f g h i}}`)
	checkCoverage(t, tree, fileA, `a:{b:{[0]} c:{[0]} d:{[0]} e:{[1]} f:{[1]} g:{[2]} h:{[2]} i:{[2]}}`)

	tree.Optimize()

	//               (a)
	//  ┏━━━┳━━━┳━━━┳━┻━┳━━━┳━━━┳━━━┓
	// (b) (c) (d) (e) (f) (g) (h) (i)
	// [0] [0] [0] [1] [1] [2] [2] [2]
	checkSpans(t, tree.Spans(), span0, span1, span2)
	checkTests(t, tree, `{a:{b c d e f g h i}}`)
	checkCoverage(t, tree, fileA, `a:{b:{[0]} c:{[0]} d:{[0]} e:{[1]} f:{[1]} g:{[2]} h:{[2]} i:{[2]}}`)
}

func checkSpans(t *testing.T, got []cov.Span, expect ...cov.Span) {
	if !reflect.DeepEqual(got, expect) {
		t.Errorf("Spans not as expected.\nGot:    %+v\nExpect: %+v", got, expect)
	}
}

func checkGroups(t *testing.T, got, expect map[cov.SpanGroupID]cov.SpanGroup) {
	if !reflect.DeepEqual(got, expect) {
		t.Errorf("SpanGroupss not as expected.\nGot:    %+v\nExpect: %+v", got, expect)
	}
}

func checkTests(t *testing.T, tree *cov.Tree, expect string) {
	g, e := tree.Tests().String(tree.Strings()), expect
	if tg, te := trimWS(g), trimWS(e); tg != te {
		t.Errorf("Tests not as expected.\nGot:\n%v\nExpect:\n%v\n------\nGot:    %v\nExpect: %v", g, e, tg, te)
	}
}

func checkCoverage(t *testing.T, tree *cov.Tree, file string, expect string) {
	g, e := tree.FileCoverage(file).String(tree.Tests(), tree.Strings()), expect
	if tg, te := trimWS(g), trimWS(e); tg != te {
		t.Errorf("Coverage not as expected.\nGot:\n%v\nExpect:\n%v\n------\nGot:    %v\nExpect: %v", g, e, tg, te)
	}
}

func trimWS(s string) string {
	s = strings.ReplaceAll(s, " ", "")
	s = strings.ReplaceAll(s, "\n", "")
	return s
}

func coverage(file string, spans ...cov.Span) *cov.Coverage {
	return &cov.Coverage{
		[]cov.File{
			cov.File{
				Path:    file,
				Covered: spans,
			},
		},
	}
}

func spans(ids ...cov.SpanID) cov.SpanSet {
	out := make(cov.SpanSet, len(ids))
	for _, id := range ids {
		out[id] = struct{}{}
	}
	return out
}

func TestTreeEncodeDecode(t *testing.T) {
	orig := &cov.Tree{}
	orig.Add(cov.Path{"a", "b"}, coverage(fileA, span0, span1))
	orig.Add(cov.Path{"a", "b", "d", "i"}, coverage(fileA, span0, span1))
	orig.Add(cov.Path{"a", "b", "e"}, coverage(fileA, span0, span1, span2))
	orig.Add(cov.Path{"a", "c", "g", "n"}, coverage(fileB, span0, span3))
	orig.Add(cov.Path{"a", "c", "g", "o"}, coverage(fileB, span0, span3))
	orig.Add(cov.Path{"a", "c", "f"}, coverage(fileA, span1))
	orig.Add(cov.Path{"a", "b", "e", "j"}, coverage(fileC, span3))
	orig.Add(cov.Path{"a", "b", "e", "k"}, coverage(fileA, span3))
	orig.Add(cov.Path{"a", "c", "f", "l", "v"}, coverage(fileA, span1, span2))
	orig.Add(cov.Path{"a", "c", "f", "l", "x"}, coverage(fileA, span1, span2))
	orig.Add(cov.Path{"a", "c", "g", "n", "z"}, coverage(fileC, span2))
	orig.Add(cov.Path{"a", "b"}, coverage(fileA, span0, span1))
	orig.Add(cov.Path{"a", "b"}, coverage(fileA, span0, span1))

	origJSON := orig.JSON("revision goes here")
	read, revision, err := cov.ReadJSON(strings.NewReader(origJSON))
	if err != nil {
		t.Fatalf("cov.ReadJSON() failed with: %v", err)
	}
	readJSON := read.JSON(revision)
	if origJSON != readJSON {
		t.Fatalf("Encode -> Decode -> Encode produced different results:\nOriginal:\n\n%v\n\nRead:\n\n%v", origJSON, readJSON)
	}
}
