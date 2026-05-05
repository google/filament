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

// Tests for the ReadDir() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_ReadDir(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		expectedEntries []string
		expectedError
	}{
		{
			name: "Read empty directory",
			path: filepath.Join(root, "emptydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "emptydir")},
			},
			expectedEntries: []string{},
		},
		{
			name: "Read directory with files and subdirectories",
			path: filepath.Join(root, "dir"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file1.txt"):  "",
					filepath.Join(root, "dir", "z_file.txt"): "",
				},
				initialDirs: []string{filepath.Join(root, "dir", "subdir", "nested")},
			},
			expectedEntries: []string{"file1.txt", "subdir", "z_file.txt"},
		},
		{
			name: "Read non-existent directory",
			path: filepath.Join(root, "nonexistent"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Read a file",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): ""},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Read symlink to directory",
			path: filepath.Join(root, "link_to_dir"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file1.txt"): "",
				},
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_dir"): "dir",
				},
			},
			expectedEntries: []string{"file1.txt"},
		},
		{
			name: "Read symlink to file",
			path: filepath.Join(root, "link_to_file"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): ""},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_file"): "file.txt",
				},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Read through symlink to non-existent item",
			path: filepath.Join(root, "link_to_dir", "nonexistent"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_dir"): filepath.Join(root, "dir"),
				},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Read broken symlink",
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
			name: "ReadDir symlink loop",
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
			name: "ReadDir symlink chain",
			path: filepath.Join(root, "link1"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file.txt"): "",
				},
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link1"): "link2",
					filepath.Join(root, "link2"): "dir",
				},
			},
			expectedEntries: []string{"file.txt"},
		},
		{
			name: "ReadDir absolute symlink",
			path: filepath.Join(root, "abs_link"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file.txt"): "",
				},
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "abs_link"): filepath.Join(root, "dir"),
				},
			},
			expectedEntries: []string{"file.txt"},
		},
		{
			name: "ReadDir symlink to parent",
			path: filepath.Join(root, "subdir", "link"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file.txt"): "",
				},
				initialDirs: []string{filepath.Join(root, "subdir"), filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "subdir", "link"): "../dir",
				},
			},
			expectedEntries: []string{"file.txt"},
		},
		{
			name: "ReadDir through symlink path component",
			path: filepath.Join(root, "link_dir", "subdir"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "subdir", "file.txt"): "",
				},
				initialDirs: []string{filepath.Join(root, "dir", "subdir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_dir"): "dir",
				},
			},
			expectedEntries: []string{"file.txt"},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			entries, err := wrapper.ReadDir(tc.path)

			if tc.expectedError.Check(t, err) {
				return
			}

			entryNames := make([]string, len(entries))
			for i, e := range entries {
				entryNames[i] = e.Name()
			}
			require.ElementsMatch(t, tc.expectedEntries, entryNames)
		})
	}
}

func TestFSTestOSWrapper_ReadDir_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string
	}{
		{
			name: "Read empty directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"emptydir"},
			}},
			path: "emptydir",
		},
		{
			name: "Read directory with files and subdirectories",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "file1.txt"):  "content",
					filepath.Join("dir", "z_file.txt"): "content",
				},
				initialDirs: []string{
					"dir",
					filepath.Join("dir", "subdir", "nested"),
				},
			}},
			path: "dir",
		},
		{
			name: "Error on non-existent directory",
			path: "nonexistent",
		},
		{
			name: "Error on path is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
			}},
			path: "file.txt",
		},
		{
			name: "Read symlink to directory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "file1.txt"): "content",
				},
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link_to_dir": "dir",
				},
			}},
			path: "link_to_dir",
		},
		{
			name: "Read symlink to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
				initialSymlinks: map[string]string{
					"link_to_file": "file.txt",
				},
			}},
			path: "link_to_file",
		},
		{
			name: "Read through symlink to non-existent item",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link_to_dir": "dir",
				},
			}},
			path: filepath.Join("link_to_dir", "nonexistent"),
		},
		{
			name: "Read broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"broken_link": "nonexistent",
				},
			}},
			path: "broken_link",
		},
		{
			name: "ReadDir symlink loop",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"loop1": "loop2",
					"loop2": "loop1",
				},
			}},
			path: "loop1",
		},
		{
			name: "ReadDir symlink chain",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "file.txt"): "content",
				},
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link1": "link2",
					"link2": "dir",
				},
			}},
			path: "link1",
		},
		{
			name: "ReadDir symlink to parent",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "file.txt"): "content",
				},
				initialDirs: []string{"subdir", "dir"},
				initialSymlinks: map[string]string{
					"subdir/link": "../dir",
				},
			}},
			path: "subdir/link",
		},
		{
			name: "ReadDir through symlink path component",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "subdir", "file.txt"): "content",
				},
				initialDirs: []string{filepath.Join("dir", "subdir")},
				initialSymlinks: map[string]string{
					"link_dir": "dir",
				},
			}},
			path: "link_dir/subdir",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realEntries, realErr := realFS.ReadDir(filepath.Join(realRoot, tc.path))
			testEntries, testErr := testFS.ReadDir(tc.path)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				realNames := make([]string, len(realEntries))
				for i, e := range realEntries {
					realNames[i] = e.Name()
				}
				testNames := make([]string, len(testEntries))
				for i, e := range testEntries {
					testNames[i] = e.Name()
				}
				require.ElementsMatch(t, realNames, testNames)
			}
		})
	}
}
