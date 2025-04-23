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
	"testing"

	"github.com/stretchr/testify/require"
)

func TestMatchcase(t *testing.T) {
	tests := []struct {
		name          string
		pattern       string
		matchedString string
		expectedMatch bool
	}{
		{
			name:          "Exact match",
			pattern:       "test",
			matchedString: "test",
			expectedMatch: true,
		},
		{
			name:          "Exact match with starting glob",
			pattern:       "*test",
			matchedString: "test",
			expectedMatch: true,
		},
		{
			name:          "Exact match with trailing glob",
			pattern:       "test*",
			matchedString: "test",
			expectedMatch: true,
		},
		{
			name:          "Starting glob match",
			pattern:       "*foobar",
			matchedString: "test_foobar",
			expectedMatch: true,
		},
		{
			name:          "Trailing glob match",
			pattern:       "test_*",
			matchedString: "test_foobar",
			expectedMatch: true,
		},
		{
			name:          "Glob match in the middle",
			pattern:       "test*bar",
			matchedString: "test_foobar",
			expectedMatch: true,
		},
		{
			name:          "Match everything",
			pattern:       "*",
			matchedString: "test_foobar",
			expectedMatch: true,
		},
		{
			name:          "Multiple globs, starting and trailing",
			pattern:       "*_*",
			matchedString: "test_foobar",
			expectedMatch: true,
		},
		{
			name:          "Multiple globs in the middle",
			pattern:       "t*f*r",
			matchedString: "test_foobar",
			expectedMatch: true,
		},
		{
			name:          "Case sensitivity",
			pattern:       "test*",
			matchedString: "Test_foobar",
			expectedMatch: false,
		},
		{
			name:          "Escaped glob does not match non-*",
			pattern:       "test_\\*",
			matchedString: "test_.",
			expectedMatch: false,
		},
		{
			name:          "Escaped glob does match *",
			pattern:       "test_\\*",
			matchedString: "test_*",
			expectedMatch: true,
		},
		{
			name:          "No implicit starting glob",
			pattern:       "foobar",
			matchedString: "test_foobar",
			expectedMatch: false,
		},
		{
			name:          "No implicit trailing glob",
			pattern:       "test",
			matchedString: "test_foobar",
			expectedMatch: false,
		},
		{
			name:          "Offset calculated correctly",
			pattern:       "*t*_*_*",
			matchedString: "test_foobar",
			expectedMatch: false,
		},
	}
	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			g := NewReducedGlob(testCase.pattern)
			require.Equal(t, g.Matchcase(testCase.matchedString), testCase.expectedMatch)
		})
	}
}

func TestFindAllIndices(t *testing.T) {
	tests := []struct {
		name            string
		s               string
		substr          string
		expectedIndices []int
	}{
		{
			name:            "Empty string",
			s:               "",
			substr:          "*",
			expectedIndices: []int{},
		},
		{
			name:            "Non-empty string",
			s:               "*foo*bar*asdf*qwerty*",
			substr:          "*",
			expectedIndices: []int{0, 4, 8, 13, 20},
		},
	}
	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			require.Equal(t, findAllIndices(testCase.s, testCase.substr), testCase.expectedIndices)
		})
	}
}
