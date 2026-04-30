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

// Tests for the ReadFile() function of FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_ReadFile(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		expectedContent []byte
		expectedError
	}{
		{
			name: "Read existing file",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "hello world"},
			},
			expectedContent: []byte("hello world"),
		},
		{
			name: "Read non-existent file",
			path: filepath.Join(root, "nonexistent.txt"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Read a directory",
			path: filepath.Join(root, "mydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
		},
		{
			name: "Read symlink to file",
			path: filepath.Join(root, "link_to_file"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_file"): "file.txt",
				},
			},
			expectedContent: []byte("content"),
		},
		{
			name: "Read symlink to dir",
			path: filepath.Join(root, "link_to_dir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_dir"): "dir",
				},
			},
			expectedError: expectedError{
				wantErrMsg: "is a directory",
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
			name: "Read symlink loop",
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
			name: "Read symlink chain",
			path: filepath.Join(root, "link1"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link1"): "link2",
					filepath.Join(root, "link2"): "file.txt",
				},
			},
			expectedContent: []byte("content"),
		},
		{
			name: "Read absolute symlink",
			path: filepath.Join(root, "abs_link"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "abs_link"): filepath.Join(root, "file.txt"),
				},
			},
			expectedContent: []byte("content"),
		},
		{
			name: "Read symlink to parent",
			path: filepath.Join(root, "subdir", "link"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialDirs:  []string{filepath.Join(root, "subdir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "subdir", "link"): "../file.txt",
				},
			},
			expectedContent: []byte("content"),
		},
		{
			name: "Read file through symlink dir",
			path: filepath.Join(root, "link_dir", "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "subdir", "file.txt"): "content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_dir"): "subdir",
				},
			},
			expectedContent: []byte("content"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			content, err := wrapper.ReadFile(tc.path)

			if tc.expectedError.Check(t, err) {
				return
			}

			require.Equal(t, tc.expectedContent, content)
		})
	}
}

func TestFSTestOSWrapper_ReadFile_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string
	}{
		{
			name: "Read existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "hello world",
				},
			}},
			path: "file.txt",
		},
		{
			name: "Error on non-existent file",
			path: "nonexistent.txt",
		},
		{
			name: "Error on path is a directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{
					"mydir",
				},
			}},
			path: "mydir",
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
			name: "Read symlink to dir",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link_to_dir": "dir",
				},
			}},
			path: "link_to_dir",
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
			name: "Read symlink loop",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"loop1": "loop2",
					"loop2": "loop1",
				},
			}},
			path: "loop1",
		},
		{
			name: "Read symlink chain",
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
			name: "Read symlink to parent",
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
			name: "Read file through symlink dir",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"subdir/file.txt": "content"},
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
			realContent, realErr := realFS.ReadFile(filepath.Join(realRoot, tc.path))
			testContent, testErr := testFS.ReadFile(tc.path)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				require.Equal(t, realContent, testContent)
			}
		})
	}
}
