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

// Package expectations provides types and helpers for parsing, updating and
// writing WebGPU expectations files.
//
// See <dawn>/webgpu-cts/expectations.txt for more information.
package expectations

import (
	"fmt"
	"io"
	"os"
	"reflect"
	"sort"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/reducedglob"
)

// Content holds the full content of an expectations file.
type Content struct {
	Chunks []Chunk
	Tags   Tags
}

// Chunk is an optional comment followed by a run of expectations.
// A chunk ends at the first blank line, or at the transition from an
// expectation to a line-comment.
type Chunk struct {
	Comments     []string     // Line comments at the top of the chunk
	Expectations Expectations // Expectations for the chunk
}

// Type + enum for whether an Expectation's Query contains globs or not.
type ExpectationType int

const (
	UNDETERMINED ExpectationType = iota
	EXACT
	GLOB
)

// Expectation holds a single expectation line
type Expectation struct {
	Line            int                      // The 1-based line number of the expectation
	Bug             string                   // The associated bug URL for this expectation
	Tags            result.Tags              // Tags used to filter the expectation
	Query           string                   // The CTS query
	Status          []string                 // The expected result status
	Comment         string                   // Optional comment at end of line
	expectationType ExpectationType          // Cached value of whether |Query| is an exact match or not
	globMatcher     *reducedglob.ReducedGlob // Cached matcher for the case where expectationType == GLOB
}

// Expectations are a list of Expectation
type Expectations []Expectation

// Load loads the expectation file at 'path', returning a Content.
func Load(path string) (Content, error) {
	content, err := os.ReadFile(path)
	if err != nil {
		return Content{}, err
	}
	ex, err := Parse(path, string(content))
	if err != nil {
		return Content{}, err
	}
	return ex, nil
}

// Save saves the Content file to 'path'.
func (c Content) Save(path string) error {
	f, err := os.Create(path)
	if err != nil {
		return err
	}
	defer f.Close()

	return c.Write(f)
}

// Clone makes a deep-copy of the Content.
func (c Content) Clone() Content {
	chunks := make([]Chunk, len(c.Chunks))
	for i, c := range c.Chunks {
		chunks[i] = c.Clone()
	}
	return Content{chunks, c.Tags.Clone()}
}

// Empty returns true if the Content has no chunks.
func (c Content) Empty() bool {
	return len(c.Chunks) == 0
}

// Write writes the Content, in textual form, to the writer w.
func (c Content) Write(w io.Writer) error {
	for i, chunk := range c.Chunks {
		if i > 0 {
			if _, err := fmt.Fprintln(w); err != nil {
				return err
			}
		}
		for _, comment := range chunk.Comments {
			if _, err := fmt.Fprintln(w, comment); err != nil {
				return err
			}
		}
		for _, expectation := range chunk.Expectations {
			if _, err := fmt.Fprintln(w, expectation.AsExpectationFileString()); err != nil {
				return err
			}
		}
	}
	return nil
}

// String returns the Content as a string.
func (c Content) String() string {
	sb := strings.Builder{}
	c.Write(&sb)
	return sb.String()
}

// Format sorts each chunk of the Content in place.
func (c *Content) Format() {
	for _, chunk := range c.Chunks {
		chunk.Expectations.Sort()
	}
}

// RemoveExpectationsForUnknownTests modifies the Content in place so that all
// contained Expectations apply to tests in the given testlist.
func (c *Content) RemoveExpectationsForUnknownTests(testlist *[]query.Query) error {
	// Converting into a set allows us to much more efficiently check if a
	// non-wildcard expectation is for a valid test.
	knownTestNames := container.NewSet[string]()
	for _, testQuery := range *testlist {
		knownTestNames.Add(testQuery.ExpectationFileString())
	}

	prunedChunkSlice := make([]Chunk, 0)
	for _, chunk := range c.Chunks {
		prunedChunk := chunk.Clone()
		// If we don't have any expectations already, just add the chunk back
		// immediately to avoid removing comments, especially the header.
		if prunedChunk.IsCommentOnly() {
			prunedChunkSlice = append(prunedChunkSlice, prunedChunk)
			continue
		}

		prunedChunk.Expectations = make(Expectations, 0)
		for _, expectation := range chunk.Expectations {
			// We don't actually parse the query string into a Query since wildcards
			// are treated differently between expectations and CTS queries.
			if expectation.IsGlobExpectation() {
				for testName := range knownTestNames {
					if expectation.AppliesToTest(testName) {
						prunedChunk.Expectations = append(prunedChunk.Expectations, expectation)
						break
					}
				}
			} else {
				// We could technically use AppliesToTest() here like we do for glob
				// expectations, but Contains() will be faster due to use of a set.
				if knownTestNames.Contains(expectation.Query) {
					prunedChunk.Expectations = append(prunedChunk.Expectations, expectation)
				}
			}
		}

		if len(prunedChunk.Expectations) > 0 {
			prunedChunkSlice = append(prunedChunkSlice, prunedChunk)
		}
	}

	c.Chunks = prunedChunkSlice
	return nil
}

// IsCommentOnly returns true if the Chunk contains comments and no expectations.
func (c Chunk) IsCommentOnly() bool {
	return len(c.Comments) > 0 && len(c.Expectations) == 0
}

// Clone returns a deep-copy of the Chunk
func (c Chunk) Clone() Chunk {
	comments := make([]string, len(c.Comments))
	for i, c := range c.Comments {
		comments[i] = c
	}
	expectations := make([]Expectation, len(c.Expectations))
	for i, e := range c.Expectations {
		expectations[i] = e.Clone()
	}
	return Chunk{comments, expectations}
}

func (c Chunk) ContainedWithinList(chunkList *[]Chunk) bool {
	for _, otherChunk := range *chunkList {
		if reflect.DeepEqual(c, otherChunk) {
			return true
		}
	}
	return false
}

// IsGlobExpectation returns whether the Expectation is a glob expectation or
// not. Glob-iness is cached after the first call.
func (e *Expectation) IsGlobExpectation() bool {
	if e.expectationType != UNDETERMINED {
		return e.expectationType == GLOB
	}

	// Count the total number of escaped and unescaped wildcard characters. If
	// they do not match, then that means we have at least one glob, which means
	// this is a glob expectation.
	numEscapedWildcards := strings.Count(e.Query, reducedglob.ESCAPED_WILDCARD)
	numNonEscapedWildcards := strings.Count(e.Query, reducedglob.UNESCAPED_WILDCARD)
	if numEscapedWildcards == numNonEscapedWildcards {
		e.expectationType = EXACT
		return false
	}
	e.expectationType = GLOB
	return true
}

// ensureGlobMatcherIsSet creates and caches a reducedglob.ReducedGlob for the
// Expectation's Query field. Should only be called in cases where
// IsGlobExpectation() returns true.
func (e *Expectation) ensureGlobMatcherIsSet() {
	if e.globMatcher != nil {
		return
	}
	if e.expectationType != GLOB {
		panic("ensureGlobMatcherIsSet should only be ever be called when the for glob expectations")
	}
	e.globMatcher = reducedglob.NewReducedGlob(e.Query)
}

// AppliesToResult returns whether the Expectation applies to the test + config
// represented by the Result.
func (e Expectation) AppliesToResult(r result.Result) bool {
	// Tags apply as long as the Expectation's tags are a subset of the Result's
	// tags.
	tagsApply := r.Tags.ContainsAll(e.Tags)
	queryApplies := e.AppliesToTest(r.Query.ExpectationFileString())

	return tagsApply && queryApplies
}

// AppliesToTest returns whether the Expectation applies to the test |name|.
// This does NOT take into account the tags contained within the Expectation,
// only whether the name matches.
func (e Expectation) AppliesToTest(name string) bool {
	// The query is a glob expectation, we need to perform a more complex
	// comparison. Otherwise, we can just check for an exact match.
	if e.IsGlobExpectation() {
		e.ensureGlobMatcherIsSet()
		return e.globMatcher.Matchcase(name)
	} else {
		return e.Query == name
	}
}

// AsExpectationFileString returns the human-readable form of the expectation
// that matches the syntax of the expectation files.
func (e Expectation) AsExpectationFileString() string {
	parts := []string{}
	if e.Bug != "" {
		parts = append(parts, e.Bug)
	}
	if len(e.Tags) > 0 {
		parts = append(parts, fmt.Sprintf("[ %v ]", strings.Join(e.Tags.List(), " ")))
	}
	parts = append(parts, e.Query)
	parts = append(parts, fmt.Sprintf("[ %v ]", strings.Join(e.Status, " ")))
	if e.Comment != "" {
		parts = append(parts, e.Comment)
	}
	return strings.Join(parts, " ")
}

// Clone makes a deep-copy of the Expectation.
func (e Expectation) Clone() Expectation {
	out := Expectation{
		Line:    e.Line,
		Bug:     e.Bug,
		Query:   e.Query,
		Comment: e.Comment,
	}
	if e.Tags != nil {
		out.Tags = e.Tags.Clone()
	}
	if e.Status != nil {
		out.Status = append([]string{}, e.Status...)
	}
	return out
}

// Compare compares the relative order of a and b, returning:
//
//	-1 if a should come before b
//	 1 if a should come after b
//	 0 if a and b are identical
//
// Note: Only comparing bug, tags, and query (in that order).
func (e Expectation) Compare(b Expectation) int {
	switch strings.Compare(e.Bug, b.Bug) {
	case -1:
		return -1
	case 1:
		return 1
	}
	switch strings.Compare(result.TagsToString(e.Tags), result.TagsToString(b.Tags)) {
	case -1:
		return -1
	case 1:
		return 1
	}
	switch strings.Compare(e.Query, b.Query) {
	case -1:
		return -1
	case 1:
		return 1
	}
	return 0
}

// ComparePrioritizeQuery is the same as Compare, but compares in the following
// order: query, tags, bug.
func (e Expectation) ComparePrioritizeQuery(other Expectation) int {
	switch strings.Compare(e.Query, other.Query) {
	case -1:
		return -1
	case 1:
		return 1
	}
	switch strings.Compare(result.TagsToString(e.Tags), result.TagsToString(other.Tags)) {
	case -1:
		return -1
	case 1:
		return 1
	}
	switch strings.Compare(e.Bug, other.Bug) {
	case -1:
		return -1
	case 1:
		return 1
	}
	return 0
}

// Sort sorts the expectations in-place
func (e Expectations) Sort() {
	sort.Slice(e, func(i, j int) bool { return e[i].Compare(e[j]) < 0 })
}

// SortPrioritizeQuery sorts the expectations in-place, prioritizing the query for
// sorting order.
func (e Expectations) SortPrioritizeQuery() {
	sort.Slice(e, func(i, j int) bool { return e[i].ComparePrioritizeQuery(e[j]) < 0 })
}
