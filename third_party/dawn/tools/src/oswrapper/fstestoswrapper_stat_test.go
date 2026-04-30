// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
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
package oswrapper_test

import (
	"os"
	"path/filepath"
	"syscall"
	"testing"

	"github.com/stretchr/testify/require"
)

// Tests for the Stat() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_Stat(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name   string
		setup  unittestSetup
		path   string
		verify func(t *testing.T, info os.FileInfo)
		expectedError
	}{
		{
			name: "Stat a file",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			verify: func(t *testing.T, info os.FileInfo) {
				require.False(t, info.IsDir())
				require.Equal(t, "file.txt", info.Name())
				require.Equal(t, int64(7), info.Size())
			},
		},
		{
			name: "Stat a directory",
			path: filepath.Join(root, "dir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
			},
			verify: func(t *testing.T, info os.FileInfo) {
				require.True(t, info.IsDir())
				require.Equal(t, "dir", info.Name())
			},
		},
		{
			name: "Stat non-existent path",
			path: filepath.Join(root, "nonexistent"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Stat symlink to file",
			path: filepath.Join(root, "link_to_file"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_file"): "file.txt",
				},
			},
			verify: func(t *testing.T, info os.FileInfo) {
				require.False(t, info.IsDir())
				require.Equal(t, "link_to_file", info.Name())
				require.Equal(t, int64(7), info.Size())
			},
		},
		{
			name: "Stat symlink to dir",
			path: filepath.Join(root, "link_to_dir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_dir"): "dir",
				},
			},
			verify: func(t *testing.T, info os.FileInfo) {
				require.True(t, info.IsDir())
				require.Equal(t, "link_to_dir", info.Name())
			},
		},
		{
			name: "Stat broken symlink",
			path: filepath.Join(root, "broken_link"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "broken_link"): "nonexistent",
				},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Stat symlink to symlink",
			path: filepath.Join(root, "link1"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link1"): "link2",
					filepath.Join(root, "link2"): "file.txt",
				},
			},
			verify: func(t *testing.T, info os.FileInfo) {
				require.False(t, info.IsDir())
				require.Equal(t, "link1", info.Name())
				require.Equal(t, int64(7), info.Size())
			},
		},
		{
			name: "Stat symlink loop",
			path: filepath.Join(root, "loop1"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "loop1"): "loop2",
					filepath.Join(root, "loop2"): "loop1",
				},
			},
			expectedError: expectedError{
				wantErrIs: syscall.ELOOP,
			},
		},
		{
			name: "Stat absolute symlink",
			path: filepath.Join(root, "abs_link"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "abs_link"): filepath.Join(root, "file.txt"),
				},
			},
			verify: func(t *testing.T, info os.FileInfo) {
				require.False(t, info.IsDir())
				require.Equal(t, "abs_link", info.Name())
				require.Equal(t, int64(7), info.Size())
			},
		},
		{
			name: "Stat symlink to parent",
			path: filepath.Join(root, "subdir", "link"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialDirs:  []string{filepath.Join(root, "subdir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "subdir", "link"): "../file.txt",
				},
			},
			verify: func(t *testing.T, info os.FileInfo) {
				require.False(t, info.IsDir())
				require.Equal(t, "link", info.Name())
				require.Equal(t, int64(7), info.Size())
			},
		},
		{
			name: "Stat through symlink path component",
			path: filepath.Join(root, "link_dir", "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "subdir", "file.txt"): "content"},
				initialDirs:  []string{filepath.Join(root, "subdir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_dir"): "subdir",
				},
			},
			verify: func(t *testing.T, info os.FileInfo) {
				require.False(t, info.IsDir())
				require.Equal(t, "file.txt", info.Name())
				require.Equal(t, int64(7), info.Size())
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			info, err := wrapper.Stat(tc.path)

			if tc.expectedError.Check(t, err) {
				return
			}

			if tc.verify != nil {
				tc.verify(t, info)
			}
		})
	}
}

func TestFSTestOSWrapper_Stat_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string
	}{
		{
			name: "Stat a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "content",
				},
			}},
			path: "file.txt",
		},
		{
			name: "Stat a directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{
					"dir",
				},
			}},
			path: "dir",
		},
		{
			name: "Stat non-existent path",
			path: "nonexistent",
		},
		{
			name: "Stat empty path",
			path: "",
		},
		{
			name: "Stat symlink to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
				initialSymlinks: map[string]string{
					"link_to_file": "file.txt",
				},
			}},
			path: "link_to_file",
		},
		{
			name: "Stat symlink to dir",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link_to_dir": "dir",
				},
			}},
			path: "link_to_dir",
		},
		{
			name: "Stat broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"broken_link": "nonexistent",
				},
			}},
			path: "broken_link",
		},
		{
			name: "Stat symlink loop",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"loop1": "loop2",
					"loop2": "loop1",
				},
			}},
			path: "loop1",
		},
		{
			name: "Stat symlink chain",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
				initialSymlinks: map[string]string{
					"link1": "link2",
					"link2": "file.txt",
				},
			}},
			path: "link1",
		},
		{
			name: "Stat symlink to parent",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
				initialDirs:  []string{"subdir"},
				initialSymlinks: map[string]string{
					"subdir/link": "../file.txt",
				},
			}},
			path: "subdir/link",
		},
		{
			name: "Stat through symlink path component",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"subdir/file.txt": "content"},
				initialDirs:  []string{"subdir"},
				initialSymlinks: map[string]string{
					"link_dir": "subdir",
				},
			}},
			path: "link_dir/file.txt",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realInfo, realErr := realFS.Stat(filepath.Join(realRoot, tc.path))
			testInfo, testErr := testFS.Stat(tc.path)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				// An empty path appears to be treated as the CWD, so the real and test
				// names cannot be directly compared.
				// TODO(crbug.com/436025865): Update this to check for the fake CWD
				// instead of "." when the filesystem is CWD-aware.
				if tc.path == "" {
					require.Equal(t, testInfo.Name(), ".")
					require.Equal(t, realInfo.Name(), filepath.Base(realRoot))
				} else {
					require.Equal(t, realInfo.Name(), testInfo.Name())
				}
				require.Equal(t, realInfo.IsDir(), testInfo.IsDir())
				// The size of a directory is system-dependent, so only compare sizes for files.
				if !realInfo.IsDir() {
					require.Equal(t, realInfo.Size(), testInfo.Size())
				}
			}
		})
	}
}
