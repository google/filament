// Copyright 2025 The Dawn & Tint Authors
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

package reducedglob

import (
	"slices"
	"strings"
)

// This package provides wildcard matching for "*" characters. However, unlike
// third-party "glob" Go packages or Python's fnmatch module, this intentionally
// does not match other types of wildcards such as "?". This is based on the
// Python implementation in typ
// https://chromium.googlesource.com/catapult/+/5b409767f0b2a05044d7139c3c000773da013775/third_party/typ/typ/reduced_glob.py

const (
	ESCAPED_WILDCARD   = "\\*"
	UNESCAPED_WILDCARD = "*"
)

// Struct representing a "compiled" glob that only matches "*" characters.
// Should not be created directly. Use NewReducedGlob() instead.
type ReducedGlob struct {
	pattern    string
	substrings []string
}

// NewReducedGlob creates and returns a new ReducedGlob instance with the
// provided pattern.
func NewReducedGlob(pattern string) *ReducedGlob {
	rg := ReducedGlob{pattern: pattern}
	rg.computeSubstrings()
	return &rg
}

// computeSubstrings performs the one-time computations to get the list of
// substrings a ReducedGlob looks for.
func (rg *ReducedGlob) computeSubstrings() {
	// Find all indices of * characters, ignoring those escaped via \*. Then, use
	// that to create an ordered list of substrings that we need to have, with
	// any number of characters permitted between them.

	// Calculate indices of un-escaped wildcards.
	allWildcardIndices := findAllIndices(rg.pattern, UNESCAPED_WILDCARD)
	escapedWildcardIndices := findAllIndices(rg.pattern, ESCAPED_WILDCARD)
	// Offset by 1 so that the indices match those in allWildcardIndices.
	for i, value := range escapedWildcardIndices {
		escapedWildcardIndices[i] = value + 1
	}

	unescapedWildcardIndices := []int{}
	for _, index := range allWildcardIndices {
		if !slices.Contains(escapedWildcardIndices, index) {
			unescapedWildcardIndices = append(unescapedWildcardIndices, index)
		}
	}

	// Split |pattern| into strings to match using the calculated indices.
	previousIndex := 0
	for _, index := range unescapedWildcardIndices {
		rg.substrings = append(rg.substrings, rg.pattern[previousIndex:index])
		previousIndex = index + 1
	}
	rg.substrings = append(rg.substrings, rg.pattern[previousIndex:])
	for i, substr := range rg.substrings {
		rg.substrings[i] = strings.ReplaceAll(substr, ESCAPED_WILDCARD, UNESCAPED_WILDCARD)
	}
}

// Matchcase checks whether |name| matches the stored pattern. Case-sensitive.
func (rg *ReducedGlob) Matchcase(name string) bool {
	if len(rg.substrings) == 0 {
		panic("ReducedGlob used without calling computeSubstrings()")
	}

	// Look for each substring in order, shifting the starting point to avoid
	// anything we've matched already.
	startingIndex := 0
	for i, substr := range rg.substrings {
		substrStartIndex := strings.Index(name[startingIndex:], substr)
		if substrStartIndex == -1 {
			return false
		}

		// The first substring is special since we need to ensure that |name| starts
		// with it. Otherwise, we could potentially match later in the string, which
		// would implicitly add a * to the front of the stored pattern.
		if i == 0 && substrStartIndex != 0 {
			return false
		}

		// Similarly, the last substring is special since we need to ensure that all
		// characters in |name| were matched. Otherwise, we would implicitly add a
		// * to the end of the stored pattern.
		if i+1 == len(rg.substrings) && !strings.HasSuffix(name, substr) {
			return false
		}

		// Consume everything we just matched.
		startingIndex += substrStartIndex + len(substr)
	}
	return true
}

// findAllIndices finds all indices where |substr| exists within |s|.
func findAllIndices(s, substr string) []int {
	allIndices := []int{}
	offset := 0
	index := strings.Index(s, substr)
	for index != -1 {
		allIndices = append(allIndices, index+offset)
		offset += index + 1
		index = strings.Index(s[offset:], substr)
	}
	return allIndices
}
