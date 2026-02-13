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
	// We use a real wrapper here since ThisDir() is going to be based on the
	// actual filesystem anyways.
	wrapper := oswrapper.GetRealOSWrapper()
	dr := fileutils.DawnRoot(wrapper)
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

func TestBuildPath(t *testing.T) {
	tests := []struct {
		name    string
		setupFS func(fs oswrapper.MemMapOSWrapper) (expectedPath string) // Sets up FS and returns expected path
		want    string
	}{
		{
			name: "out/active exists",
			setupFS: func(fs oswrapper.MemMapOSWrapper) string {
				require.NoError(t, fileutils.SetUpFakeDawnRoot(fs))
				fakeDawnRoot := fileutils.DawnRoot(fs)
				require.NotEmpty(t, fakeDawnRoot, "Failed to find fake Dawn root")

				expected := filepath.Join(fakeDawnRoot, "out", "active")
				require.NoError(t, fs.MkdirAll(expected, 0777))
				return expected
			},
		},
		{
			name: "out/active does not exist",
			setupFS: func(fs oswrapper.MemMapOSWrapper) string {
				require.NoError(t, fileutils.SetUpFakeDawnRoot(fs))
				return "" // Expect empty string as out/active won't be found.
			},
		},
		{
			name: "dawn root does not exist",
			setupFS: func(fs oswrapper.MemMapOSWrapper) string {
				return "" // Expect empty string as dawn root won't be found.
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			expectedPath := tc.setupFS(wrapper)

			got := fileutils.BuildPath(wrapper)
			require.Equal(t, expectedPath, got)
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

func TestIsDir(t *testing.T) {
	tests := []struct {
		name    string
		path    string
		setupFS func(fs oswrapper.MemMapOSWrapper) // Sets up the filesystem
		want    bool
	}{
		{
			name: "Is a directory",
			path: "/a/b/c",
			setupFS: func(fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/a/b/c", 0777))
			},
			want: true,
		},
		{
			name: "Is a file",
			path: "/a/b/file.txt",
			setupFS: func(fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/a/b", 0777))
				require.NoError(t, fs.WriteFile("/a/b/file.txt", []byte("hello"), 0666))
			},
			want: false,
		},
		{
			name:    "Does not exist",
			path:    "/a/b/c",
			setupFS: nil,
			want:    false,
		},
		{
			name:    "Empty path",
			path:    "",
			setupFS: nil,
			want:    false,
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			if tc.setupFS != nil {
				tc.setupFS(wrapper)
			}

			got := fileutils.IsDir(tc.path, wrapper)
			require.Equal(t, tc.want, got)
		})
	}
}

func TestIsFile(t *testing.T) {
	tests := []struct {
		name    string
		path    string
		setupFS func(fs oswrapper.MemMapOSWrapper) // Sets up the filesystem
		want    bool
	}{
		{
			name: "Is a file",
			path: "/a/b/file.txt",
			setupFS: func(fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/a/b", 0777))
				require.NoError(t, fs.WriteFile("/a/b/file.txt", []byte("hello"), 0666))
			},
			want: true,
		},
		{
			name: "Is a directory",
			path: "/a/b/c",
			setupFS: func(fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/a/b/c", 0777))
			},
			want: false,
		},
		{
			name:    "Does not exist",
			path:    "/a/b/c",
			setupFS: nil,
			want:    false,
		},
		{
			name:    "Empty path",
			path:    "",
			setupFS: nil,
			want:    false,
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			if tc.setupFS != nil {
				tc.setupFS(wrapper)
			}

			got := fileutils.IsFile(tc.path, wrapper)
			require.Equal(t, tc.want, got)
		})
	}
}

func TestIsEmptyDir(t *testing.T) {
	tests := []struct {
		name    string
		path    string
		setupFS func(fs oswrapper.MemMapOSWrapper)
		want    bool
		wantErr bool
	}{
		{
			name:    "Path does not exist",
			path:    "/nonexistent",
			setupFS: nil,
			wantErr: true,
		},
		{
			name: "Path is a file",
			path: "/myfile.txt",
			setupFS: func(fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.WriteFile("/myfile.txt", []byte("content"), 0666))
			},
			wantErr: true,
		},
		{
			name: "Directory is empty",
			path: "/mydir",
			setupFS: func(fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/mydir", 0777))
			},
			want:    true,
			wantErr: false,
		},
		{
			name: "Directory with a file",
			path: "/mydir",
			setupFS: func(fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/mydir", 0777))
				require.NoError(t, fs.WriteFile(filepath.Join("/mydir", "file.txt"), []byte("content"), 0666))
			},
			want:    false,
			wantErr: false,
		},
		{
			name: "Directory with a subdirectory",
			path: "/mydir",
			setupFS: func(fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll(filepath.Join("/mydir", "subdir"), 0777))
			},
			want:    false,
			wantErr: false,
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			if tc.setupFS != nil {
				tc.setupFS(wrapper)
			}

			got, err := fileutils.IsEmptyDir(tc.path, wrapper)

			if tc.wantErr {
				require.Error(t, err)
			} else {
				require.NoError(t, err)
				require.Equal(t, tc.want, got)
			}
		})
	}
}
