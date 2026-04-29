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

package main

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/container"
	"github.com/stretchr/testify/require"
)

func TestShouldConsiderLinesOfFile(t *testing.T) {
	tests := []struct {
		path     string
		expected bool
	}{
		{"src/foo.cpp", true},
		{"Doxyfile", false},
		{"package-lock.json", false},
		{"src/tint/builtin_table.inl", false},
		{"src/tint/lang/core/intrinsic/table.inl", false},
		{"src/tint/lang/core/foo.cc", false},
		{"src/tint/lang/core/bar.h", false},
		{"src/tint/lang/core/baz.otherextension", true},
		{"test/tint/baz.txt", false},
		{"third_party/gn/webgpu-cts/test_list.txt", false},
		{"third_party/OpenGL-Registry/src/qux.h", false},
		{"third_party/EGL-Registry/src/qux.h", false},
		{"webgpu-cts/a/b/c.js", false},
		{"src/external/petamoriken/a.h", false},
	}

	for _, test := range tests {
		require.Equal(t, test.expected, shouldConsiderLinesOfFile(test.path))
	}
}

func TestShouldConsiderLinesOfCommit(t *testing.T) {
	tests := []struct {
		hash     string
		expected bool
	}{
		{"some-random-hash", true},
		{"41e4d9a34c1d9dcb2eef3ff39ff9c1f987bfa02a", false},
		{"e87ac76f7ddf9237f3022cda90224bd0691fb318", false},
		{"b0acbd436dbd499505a3fa8bf89e69231ec4d1e0", false},
	}

	for _, test := range tests {
		require.Equal(t, test.expected, shouldConsiderLinesOfCommit(test.hash))
	}
}

func TestAuthorStatsCombine(t *testing.T) {
	a := AuthorStats{
		commits:    10,
		insertions: 100,
		deletions:  50,
		commitsByMonth: container.Map[string, int]{
			"2024-01": 5,
			"2024-02": 5,
		},
	}
	b := AuthorStats{
		commits:    5,
		insertions: 20,
		deletions:  10,
		commitsByMonth: container.Map[string, int]{
			"2024-02": 2,
			"2024-03": 3,
		},
	}

	c := combine(a, b)
	require.Equal(t, 15, c.commits)
	require.Equal(t, 120, c.insertions)
	require.Equal(t, 60, c.deletions)
	require.Equal(t, 3, len(c.commitsByMonth))
	require.Equal(t, 5, c.commitsByMonth["2024-01"])
	require.Equal(t, 7, c.commitsByMonth["2024-02"])
	require.Equal(t, 3, c.commitsByMonth["2024-03"])
}
