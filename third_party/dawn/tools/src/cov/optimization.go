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
	"log"
	"sort"
	"sync"
)

// Optimize optimizes the Tree by de-duplicating common spans into a tree of SpanGroups.
//
// Breaking down tests into group hierarchies provide a natural way to structure
// coverage data, as tests of the same suite, file or test are likely to have
// similar coverage spans.
//
// For each source file in the codebase, we create a tree of SpanGroups, where the
// leaves are the test cases.
//
// For example, given the following Paths:
//
//	a.b.d.h
//	a.b.d.i.n
//	a.b.d.i.o
//	a.b.e.j
//	a.b.e.k.p
//	a.b.e.k.q
//	a.c.f
//	a.c.g.l.r
//	a.c.g.m
//
// We would construct the following tree:
//
//	             a
//	      ╭──────┴──────╮
//	      b             c
//	  ╭───┴───╮     ╭───┴───╮
//	  d       e     f       g
//	╭─┴─╮   ╭─┴─╮         ╭─┴─╮
//	h   i   j   k         l   m
//	   ╭┴╮     ╭┴╮        │
//	   n o     p q        r
//
// Each leaf node in this tree (`h`, `n`, `o`, `j`, `p`, `q`, `f`, `r`, `m`)
// represent a test case, and non-leaf nodes (`a`, `b`, `c`, `d`, `e`, `g`, `i`,
// `k`, `l`) are suite, file or tests.
//
// To begin, we create a test tree structure, and associate the full list of test
// coverage spans with every leaf node (test case) in this tree.
//
// This data structure hasn't given us any compression benefits yet, but we can
// now do a few tricks to dramatically reduce number of spans needed to describe
// the graph:
//
//	~ Optimization 1: Common span promotion ~
//
// The first compression scheme is to promote common spans up the tree when they
// are common for all children. This will reduce the number of spans needed to be
// encoded in the final file.
//
// For example, if the test group `a` has 4 children that all share the same span
// `X`:
//
//	         a
//	   ╭───┬─┴─┬───╮
//	   b   c   d   e
//	[X,Y] [X] [X] [X,Z]
//
// Then span `X` can be promoted up to `a`:
//
//	      [X]
//	       a
//	 ╭───┬─┴─┬───╮
//	 b   c   d   e
//	[Y] []   [] [Z]
//
//	~ Optimization 2: Span XOR promotion ~
//
// This idea can be extended further, by not requiring all the children to share
// the same span before promotion. If *most* child nodes share the same span, we
// can still promote the span, but this time we *remove* the span from the
// children *if they had it*, and *add* the span to children *if they didn't
// have it*.
//
// For example, if the test group `a` has 4 children with 3 that share the span
// `X`:
//
//	         a
//	   ╭───┬─┴─┬───╮
//	   b   c   d   e
//	[X,Y] [X]  [] [X,Z]
//
// Then span `X` can be promoted up to `a` by flipping the presence of `X` on the
// child nodes:
//
//	      [X]
//	       a
//	 ╭───┬─┴─┬───╮
//	 b   c   d   e
//	[Y] []  [X] [Z]
//
// This process repeats up the tree.
//
// With this optimization applied, we now need to traverse the tree from root to
// leaf in order to know whether a given span is in use for the leaf node (test case):
//
// * If the span is encountered an *odd* number of times during traversal, then
// the span is *covered*.
// * If the span is encountered an *even* number of times during traversal, then
// the span is *not covered*.
//
// See tools/src/cov/coverage_test.go for more examples of this optimization.
//
//	~ Optimization 3: Common span grouping ~
//
// With real world data, we encounter groups of spans that are commonly found
// together. To further reduce coverage data, the whole graph is scanned for common
// span patterns, and are indexed by each tree node.
// The XOR'ing of spans as described above is performed as if the spans were not
// grouped.
//
//	~ Optimization 4: Lookup tables ~
//
// All spans, span-groups and strings are stored in de-duplicated tables, and are
// indexed wherever possible.
func (t *Tree) Optimize() {
	log.Printf("Optimizing coverage tree...")

	// Start by gathering all of the unique spansets
	wg := sync.WaitGroup{}
	wg.Add(len(t.files))
	for _, file := range t.files {
		file := file
		go func() {
			defer wg.Done()
			o := optimizer{}
			for idx, tc := range file.tcm {
				o.invertForCommon(tc, &t.testRoot.children[idx])
			}
			o.createGroups(file)
		}()
	}
	wg.Wait()
}

type optimizer struct{}

// createGroups looks for common SpanSets, and creates indexable span groups
// which are then used instead.
func (o *optimizer) createGroups(f *treeFile) {
	const minSpansInGroup = 2

	type spansetKey string
	spansetMap := map[spansetKey]SpanSet{}

	f.tcm.traverse(func(tc *TestCoverage) {
		if len(tc.Spans) >= minSpansInGroup {
			key := spansetKey(tc.Spans.String())
			if _, ok := spansetMap[key]; !ok {
				spansetMap[key] = tc.Spans
			}
		}
	})

	if len(spansetMap) == 0 {
		return
	}

	type spansetInfo struct {
		key spansetKey
		set SpanSet // fully expanded set
		grp SpanGroup
		id  SpanGroupID
	}
	spansets := make([]*spansetInfo, 0, len(spansetMap))
	for key, set := range spansetMap {
		spansets = append(spansets, &spansetInfo{
			key: key,
			set: set,
			grp: SpanGroup{Spans: set},
		})
	}

	// Sort by number of spans in each sets starting with the largest.
	sort.Slice(spansets, func(i, j int) bool {
		a, b := spansets[i].set, spansets[j].set
		switch {
		case len(a) > len(b):
			return true
		case len(a) < len(b):
			return false
		}
		return a.List().Compare(b.List()) == -1 // Just to keep output stable
	})

	// Assign IDs now that we have stable order.
	for i := range spansets {
		spansets[i].id = SpanGroupID(i)
	}

	// Loop over the spanGroups starting from the largest, and try to fold them
	// into the larger sets.
	// This is O(n^2) complexity.
nextSpan:
	for i, a := range spansets[:len(spansets)-1] {
		for _, b := range spansets[i+1:] {
			if len(a.set) > len(b.set) && a.set.containsAll(b.set) {
				extend := b.id // Do not take address of iterator!
				a.grp.Spans = a.set.removeAll(b.set)
				a.grp.Extend = &extend
				continue nextSpan
			}
		}
	}

	// Rebuild a map of spansetKey to SpanGroup
	spangroupMap := make(map[spansetKey]*spansetInfo, len(spansets))
	for _, s := range spansets {
		spangroupMap[s.key] = s
	}

	// Store the groups in the tree
	f.spangroups = make(map[SpanGroupID]SpanGroup, len(spansets))
	for _, s := range spansets {
		f.spangroups[s.id] = s.grp
	}

	// Update all the uses.
	f.tcm.traverse(func(tc *TestCoverage) {
		key := spansetKey(tc.Spans.String())
		if g, ok := spangroupMap[key]; ok {
			tc.Spans = nil
			tc.Group = &g.id
		}
	})
}

// invertCommon looks for tree nodes with the majority of the child nodes with
// the same spans. This span is promoted up to the parent, and the children
// have the span inverted.
func (o *optimizer) invertForCommon(tc *TestCoverage, t *Test) {
	wg := sync.WaitGroup{}
	wg.Add(len(tc.Children))
	for id, child := range tc.Children {
		id, child := id, child
		go func() {
			defer wg.Done()
			o.invertForCommon(child, &t.children[id])
		}()
	}
	wg.Wait()

	counts := map[SpanID]int{}
	for _, child := range tc.Children {
		for span := range child.Spans {
			counts[span] = counts[span] + 1
		}
	}

	for span, count := range counts {
		if count > len(t.children)/2 {
			tc.Spans = tc.Spans.invert(span)
			for _, idx := range t.indices {
				child := tc.Children.index(idx)
				child.Spans = child.Spans.invert(span)
				if child.deletable() {
					delete(tc.Children, idx)
				}
			}
		}
	}
}
