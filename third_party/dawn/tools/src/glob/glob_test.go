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

package glob

import (
	"fmt"
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestGlob(t *testing.T) {
	tests := []struct {
		name       string
		input      string
		want       []string
		wantErr    bool
		wantErrMsg string
	}{
		// We can't really test the error condition from the filepath.Abs() call, as
		// it basically only fails if the given path is not absolute and the cwd
		// does not exist.
		{
			name:  "Match no wildcard",
			input: "/a/1/file_1.txt",
			want:  []string{"/a/1/file_1.txt"},
		},
		{
			name:  "No match no wildcard is directory",
			input: "/a/1",
			want:  []string{},
		},
		{
			name:  "Match star wildcard files",
			input: "/a/1/*",
			want:  []string{"/a/1/file_1.txt", "/a/1/file_2.txt"},
		},
		{
			name:  "Match star wildcard child directory",
			input: "/a/*/file_1.txt",
			want:  []string{"/a/1/file_1.txt", "/a/2/file_1.txt", "/a/3/file_1.txt"},
		},
		{
			name:       "No match star wildcard parent directory",
			input:      "/*/1/file_1.txt",
			want:       nil,
			wantErr:    true,
			wantErrMsg: "open a: file does not exist",
		},
		{
			name:  "Match question wildcard files",
			input: "/a/1/file_?.txt",
			want:  []string{"/a/1/file_1.txt", "/a/1/file_2.txt"},
		},
		{
			name:  "No match question wildcard files",
			input: "/a/1/?file_1.txt",
			want:  []string{},
		},
		{
			name:  "Match question filecard child directory",
			input: "/a/?/file_1.txt",
			want:  []string{"/a/1/file_1.txt", "/a/2/file_1.txt", "/a/3/file_1.txt"},
		},
		{
			name:       "Match question filecard parent directory",
			input:      "/?/1/file_1.txt",
			want:       nil,
			wantErr:    true,
			wantErrMsg: "open a: file does not exist",
		},
		{
			name:  "No match no wildcard non existent file",
			input: "/a/1/file_3.txt",
			want:  []string{},
		},
		{
			name:  "No match no wildcard non existent directory",
			input: "/a/5/file_1.txt",
			want:  []string{},
		},
		{
			name:  "No match star wildcard non existent file",
			input: "/a/1/foo*_1.txt",
			want:  []string{},
		},
		{
			name:  "No match star wildcard non existent directory",
			input: "/a/5*/file_1.txt",
			want:  []string{},
		},
		{
			name:  "No match question wildcard non existent file",
			input: "/a/1/file?_1.txt",
			want:  []string{},
		},
		{
			name:  "No match question wildcard non existent directory",
			input: "/a/5?/file_1.txt",
			want:  []string{},
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			parentDirs := []string{"a", "b", "c"}
			childDirs := []string{"1", "2", "3"}
			fileNames := []string{"file_1.txt", "file_2.txt"}
			for _, pd := range parentDirs {
				for _, cd := range childDirs {
					wrapper.MkdirAll(fmt.Sprintf("/%s/%s", pd, cd), 0o700)
					for _, fn := range fileNames {
						wrapper.Create(fmt.Sprintf("/%s/%s/%s", pd, cd, fn))
					}
				}
			}

			matches, err := Glob(testCase.input, wrapper)
			require.Equal(t, testCase.want, matches)
			if testCase.wantErr {
				require.ErrorContains(t, err, testCase.wantErrMsg)
			} else {
				require.NoErrorf(t, err, "Failed to glob: %v", err)
			}
		})
	}
}

func TestScan(t *testing.T) {
	tests := []struct {
		name       string
		root       string
		condition  rule
		want       []string
		wantErr    bool
		wantErrMsg string
	}{
		{
			name:      "Scan everything",
			root:      "/",
			condition: func(path string, cond bool) bool { return true },
			want: []string{
				".other/1/file_1.txt",
				".other/1/file_2.txt",
				".other/2/file_1.txt",
				".other/2/file_2.txt",
				"a/1/file_1.txt",
				"a/1/file_2.txt",
				"a/2/file_1.txt",
				"a/2/file_2.txt",
				"b/1/file_1.txt",
				"b/1/file_2.txt",
				"b/2/file_1.txt",
				"b/2/file_2.txt",
			},
		},
		{
			name:      "Scan nothing",
			root:      "/",
			condition: func(path string, cond bool) bool { return false },
			want:      []string{},
		},
		{
			name:      "Scan everything with subdir root",
			root:      "/a",
			condition: func(path string, cond bool) bool { return true },
			want: []string{
				"1/file_1.txt",
				"1/file_2.txt",
				"2/file_1.txt",
				"2/file_2.txt",
			},
		},
		{
			name:      "Scan with rule",
			root:      "/",
			condition: func(path string, cond bool) bool { return strings.Contains(path, "/2/") },
			want: []string{
				".other/2/file_1.txt",
				".other/2/file_2.txt",
				"a/2/file_1.txt",
				"a/2/file_2.txt",
				"b/2/file_1.txt",
				"b/2/file_2.txt",
			},
		},
		{
			name:       "Empty root",
			root:       "",
			condition:  func(path string, cond bool) bool { return true },
			want:       nil,
			wantErr:    true,
			wantErrMsg: "open .git: file does not exist",
		},
		{
			name:       "Non-existent root",
			root:       "/asdf",
			condition:  func(path string, cond bool) bool { return true },
			want:       nil,
			wantErr:    true,
			wantErrMsg: "open /asdf: file does not exist",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			parentDirs := []string{"a", "b", ".git", ".other"}
			childDirs := []string{"1", "2"}
			fileNames := []string{"file_1.txt", "file_2.txt"}
			for _, pd := range parentDirs {
				for _, cd := range childDirs {
					wrapper.MkdirAll(fmt.Sprintf("/%s/%s", pd, cd), 0o700)
					for _, fn := range fileNames {
						wrapper.Create(fmt.Sprintf("/%s/%s/%s", pd, cd, fn))
					}
				}
			}

			matches, err := Scan(testCase.root, Config{Paths: searchRules{testCase.condition}}, wrapper)
			require.Equal(t, testCase.want, matches)
			if testCase.wantErr {
				require.ErrorContains(t, err, testCase.wantErrMsg)
			} else {
				require.NoErrorf(t, err, "Failed to scan: %v", err)
			}
		})
	}
}

func TestLoadConfig(t *testing.T) {
	tests := []struct {
		name         string
		cfgPath      string
		cfgJson      string
		wantNumRules int
		wantErr      bool
		wantErrMsg   string
	}{
		{
			name:    "Success",
			cfgPath: "/config.cfg",
			// This is what was in tools/src/cmd/trim-includes/config.cfg at the time of writing.
			cfgJson: `{
  "paths": [
    { "include": [ "src/**.h", "src/**.cc" ] },
    { "exclude": [ "src/**_windows.*", "src/**_other.*" ] }
  ]
}`,
			wantNumRules: 2,
		},
		{
			name:    "Both include and exclude",
			cfgPath: "/config.cfg",
			cfgJson: `{
  "paths": [
    { "include": [ "src/**.h", "src/**.cc" ], "exclude": [ "src/**.h" ] }
  ]
}`,
			wantErr:    true,
			wantErrMsg: "Rule cannot contain both include and exclude",
		},
		{
			name:       "Invalid path",
			cfgPath:    "/asdf.cfg",
			cfgJson:    "{}",
			wantErr:    true,
			wantErrMsg: "open /asdf.cfg: file does not exist",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			wrapper.WriteFile("/config.cfg", []byte(testCase.cfgJson), 0o700)

			cfg, err := LoadConfig(testCase.cfgPath, wrapper)
			// We can't check equality since the loaded Config contains function
			// pointers to anonymous functions. So, just check that the number of
			// rules matches.
			require.Equal(t, testCase.wantNumRules, len(cfg.Paths))
			if testCase.wantErr {
				require.ErrorContains(t, err, testCase.wantErrMsg)
			} else {
				require.NoErrorf(t, err, "Failed to load config: %v", err)
			}
		})
	}
}
