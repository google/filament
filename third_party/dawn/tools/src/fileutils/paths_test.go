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

package fileutils_test

import (
	"path/filepath"
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/google/go-cmp/cmp"
	"github.com/stretchr/testify/require"
)

func TestThisLine(t *testing.T) {
	td := fileutils.ThisLine()
	if !strings.HasSuffix(td, "paths_test.go:42") {
		t.Errorf("TestThisLine() returned %v", td)
	}
}

func TestThisDir(t *testing.T) {
	td := fileutils.ThisDir()
	if !strings.HasSuffix(td, "utils") {
		t.Errorf("ThisDir() returned %v", td)
	}
}

func TestDawnRoot(t *testing.T) {
	dr := fileutils.DawnRoot()
	rel, err := filepath.Rel(dr, fileutils.ThisDir())
	if err != nil {
		t.Fatalf("%v", err)
	}
	got := filepath.ToSlash(rel)
	expect := `tools/src/fileutils`
	if diff := cmp.Diff(got, expect); diff != "" {
		t.Errorf("DawnRoot() returned %v.\n%v", dr, diff)
	}
}

func TestExpandHome(t *testing.T) {
	tests := []struct {
		name  string
		input string
		want  string
	}{
		{
			name:  "No substitution",
			input: "/foo/bar",
			want:  "/foo/bar",
		},
		{
			name:  "Single substitution",
			input: "~/foo",
			want:  "/home/foo",
		},
		{
			name:  "Multi substitution",
			input: "~/foo/~",
			want:  "/home/foo//home",
		},
		{
			name:  "Trailing slash",
			input: "~/foo/",
			want:  "/home/foo/",
		},
		{
			name:  "Only home",
			input: "~",
			want:  "/home",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			wrapper.Environment = map[string]string{
				"HOME": "/home",
			}

			expandedPath := fileutils.ExpandHome(testCase.input, wrapper)
			require.Equal(t, testCase.want, expandedPath)
		})
	}
}

func TestCommonRootDir(t *testing.T) {
	for _, test := range []struct {
		a, b   string
		expect string
	}{
		{"", "", "/"},
		{"a/b/c", "d/e/f", ""},
		{"a/", "b", ""},
		{"a/b/c", "a/b", "a/b/"},
		{"a/b/c/", "a/b", "a/b/"},
		{"a/b/c/", "a/b/", "a/b/"},
		{"a/b/c", "a/b/d", "a/b/"},
		{"a/b/c", "a/bc", "a/"},
	} {
		if got := fileutils.CommonRootDir(test.a, test.b); got != test.expect {
			t.Errorf("CommonRootDir('%v', '%v') returned '%v'.\nExpected: '%v'", test.a, test.b, got, test.expect)
		}
		if got := fileutils.CommonRootDir(test.b, test.a); got != test.expect {
			t.Errorf("CommonRootDir('%v', '%v') returned '%v'.\nExpected: '%v'", test.b, test.a, got, test.expect)
		}
	}
}
