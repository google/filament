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
	"log"
	"sort"
	"sync"
)

// Optimize optimizes the Tree by de-duplicating common spans into a tree of
// SpanGroups.
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
