// Copyright 2025 Google LLC
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
	"os"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

// tryBuild(), file.format(), file.stage(), and anything that calls those
// cannot currently be unittested since they call exec.Command().

func TestStripExtension(t *testing.T) {
	tests := []struct {
		name  string
		input string
		want  string
	}{
		{
			name:  "Single extension",
			input: "/foo.zip",
			want:  "/foo",
		},
		{
			name:  "No extension",
			input: "/foo",
			want:  "/foo",
		},
		{
			name:  "Multiple extensions",
			input: "/foo.tar.gz",
			want:  "/foo",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			require.Equal(t, testCase.want, stripExtension(testCase.input))
		})
	}
}

func TestFileIncludesLineNumbers(t *testing.T) {
	tests := []struct {
		name  string
		path  string
		lines []string
		want  []int
	}{
		{
			name: "Comments ignored",
			path: "foo.cc",
			lines: []string{
				"// Some comment",
				"#include \"some/file\"",
				"// Another comment",
				"#include \"another/file\"",
			},
			want: []int{1, 3},
		},
		{
			name: "Stdlib imports",
			path: "foo.cc",
			lines: []string{
				"#include <string>",
				"#include \"some/file\"",
				"#include \"another/file\"",
			},
			want: []int{0, 1, 2},
		},
		{
			name: "Header ignored",
			path: "foo.cc",
			lines: []string{
				"#include \"foo.h\"",
				"#include \"some/file\"",
				"#include \"another/file\"",
			},
			want: []int{1, 2},
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			f := file{
				path:  testCase.path,
				lines: testCase.lines,
			}
			require.Equal(t, testCase.want, f.includesLineNumbers())
		})
	}
}

func TestFileSave(t *testing.T) {
	tests := []struct {
		name       string
		path       string
		lines      []string
		lineMap    map[int]bool
		want       string
		wantErr    bool
		wantErrMsg string
	}{
		{
			name: "All lines saved",
			path: "/foo.cc",
			lines: []string{
				"Line 1",
				"Line 2",
			},
			lineMap: map[int]bool{
				0: true,
				1: true,
			},
			want: "Line 1\nLine 2",
		},
		{
			name: "False lines are omitted",
			path: "/foo.cc",
			lines: []string{
				"Line 1",
				"Line 2",
			},
			lineMap: map[int]bool{
				0: false,
				1: true,
			},
			want: "Line 2",
		},
		{
			name: "Missing lines are omitted",
			path: "/foo.cc",
			lines: []string{
				"Line 1",
				"Line 2",
			},
			lineMap: map[int]bool{
				1: true,
			},
			want: "Line 2",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			f := file{
				path:  testCase.path,
				lines: testCase.lines,
			}
			wrapper := oswrapper.CreateMemMapOSWrapper()
			err := f.save(testCase.lineMap, wrapper)
			if testCase.wantErr {
				require.ErrorContains(t, err, testCase.wantErrMsg)
				_, err := wrapper.Stat(testCase.path)
				require.ErrorIs(t, err, os.ErrNotExist)
			} else {
				require.NoErrorf(t, err, "Got error saving file: %v", err)
				contents, err := wrapper.ReadFile(testCase.path)
				require.NoErrorf(t, err, "Got error reading file: %v", err)
				require.Equal(t, testCase.want, string(contents[:]))
			}
		})
	}
}
