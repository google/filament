// Copyright 2020 Google LLC
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

// Package match provides functions for performing filepath [?,*,**] wildcard
// matching.
package match

import (
	"fmt"
	"regexp"
	"strings"
)

// Test is the match predicate returned by New.
type Test func(path string) bool

// New returns a Test function that returns true iff the path matches the
// provided pattern.
//
// pattern uses forward-slashes for directory separators '/', and may use the
// following wildcards:
//
//	?  - matches any single non-separator character
//	*  - matches any sequence of non-separator characters
//	** - matches any sequence of characters including separators
func New(pattern string) (Test, error) {
	// Transform pattern into a regex by replacing the uses of `?`, `*`, `**`
	// with corresponding regex patterns.
	// As the pattern may contain other regex sequences, the string has to be
	// escaped. So:
	// a) Replace the patterns of `?`, `*`, `**` with unique placeholder tokens.
	// b) Escape the expression so that other sequences don't confuse the regex
	//    parser.
	// c) Replace the placeholder tokens with the corresponding regex tokens.

	// Temporary placeholder tokens
	const (
		starstar     = "••"
		star         = "•"
		questionmark = "¿"
	)
	// Check pattern doesn't contain any of our placeholder tokens
	for _, r := range []rune{'•', '¿'} {
		if strings.ContainsRune(pattern, r) {
			return nil, fmt.Errorf("Pattern must not contain '%c'", r)
		}
	}
	// Replace **, * and ? with placeholder tokens
	subbed := pattern
	subbed = strings.ReplaceAll(subbed, "**", starstar)
	subbed = strings.ReplaceAll(subbed, "*", star)
	subbed = strings.ReplaceAll(subbed, "?", questionmark)
	// Escape any remaining regex characters
	escaped := regexp.QuoteMeta(subbed)
	// Insert regex matchers for the substituted tokens
	regex := "^" + escaped + "$"
	regex = strings.ReplaceAll(regex, starstar, ".*")
	regex = strings.ReplaceAll(regex, star, "[^/]*")
	regex = strings.ReplaceAll(regex, questionmark, "[^/]")

	re, err := regexp.Compile(regex)
	if err != nil {
		return nil, fmt.Errorf(`Failed to compile regex "%v" for pattern "%v": %w`, regex, pattern, err)
	}
	return re.MatchString, nil
}
