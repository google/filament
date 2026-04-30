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
	"io"
	"os"
	"path/filepath"
	"syscall"
	"testing"

	"github.com/stretchr/testify/require"
)

// Tests for the Open() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_Open(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		expectedContent *string
		expectedError
	}{
		{
			name: "Open existing file",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "file.txt"): "hello world",
				},
			},
			expectedContent: stringPtr("hello world"),
		},
		{
			name: "Open non-existent file",
			path: filepath.Join(root, "nonexistent.txt"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Open a directory",
			path: filepath.Join(root, "mydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
		},
		{
			name: "Parent path is a file",
			path: filepath.Join(root, "file.txt", "another.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "file.txt"): "i am a file",
				},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Open symlink to file",
			path: filepath.Join(root, "link_to_file"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "symlink content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_file"): "file.txt",
				},
			},
			expectedContent: stringPtr("symlink content"),
		},
		{
			name: "Open symlink to directory",
			path: filepath.Join(root, "link_to_dir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_dir"): "dir",
				},
			},
		},
		{
			name: "Open broken symlink",
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
			name: "Open symlink loop",
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
			name: "Open symlink to symlink (chain)",
			path: filepath.Join(root, "link1"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "target"): "target content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link1"): "link2",
					filepath.Join(root, "link2"): "target",
				},
			},
			expectedContent: stringPtr("target content"),
		},
		{
			name: "Open absolute symlink",
			path: filepath.Join(root, "abs_link"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "target"): "target content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "abs_link"): filepath.Join(root, "target"),
				},
			},
			expectedContent: stringPtr("target content"),
		},
		{
			name: "Open relative symlink with parent ref",
			path: filepath.Join(root, "dir", "link"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "target"): "target content"},
				initialDirs:  []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "dir", "link"): "../target",
				},
			},
			expectedContent: stringPtr("target content"),
		},
		{
			name: "Open symlink in subdirectory",
			path: filepath.Join(root, "dir", "link"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "dir", "target"): "target content"},
				initialDirs:  []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "dir", "link"): "target",
				},
			},
			expectedContent: stringPtr("target content"),
		},
		{
			name: "Open path with multiple symlinks",
			path: filepath.Join(root, "link1", "link2", "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "real_dir1", "real_dir2", "file.txt"): "content"},
				initialDirs:  []string{filepath.Join(root, "real_dir1"), filepath.Join(root, "real_dir1", "real_dir2")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link1"):              "real_dir1",
					filepath.Join(root, "real_dir1", "link2"): "real_dir2",
				},
			},
			expectedContent: stringPtr("content"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			file, err := wrapper.Open(tc.path)

			if tc.expectedError.Check(t, err) {
				return
			}

			require.NotNil(t, file)
			defer file.Close()

			if tc.expectedContent != nil {
				content, err := io.ReadAll(file)
				require.NoError(t, err)
				require.Equal(t, *tc.expectedContent, string(content))
			}
		})
	}
}

func TestFSTestOSWrapper_Open_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string // path to Open
	}{
		{
			name: "Open existing file",
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
			name: "Open a directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{
					"mydir",
				},
			}},
			path: "mydir",
		},
		{
			name: "Error on parent path is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "i am a file",
				},
			}},
			path: filepath.Join("file.txt", "another.txt"),
		},
		{
			name: "Open symlink to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "symlink content"},
				initialSymlinks: map[string]string{
					"link_to_file": "file.txt",
				},
			}},
			path: "link_to_file",
		},
		{
			name: "Open symlink to directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link_to_dir": "dir",
				},
			}},
			path: "link_to_dir",
		},
		{
			name: "Open broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"broken_link": "nonexistent",
				},
			}},
			path: "broken_link",
		},
		{
			name: "Open symlink loop",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"loop1": "loop2",
					"loop2": "loop1",
				},
			}},
			path: "loop1",
		},
		{
			name: "Open symlink to symlink (chain)",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"target": "target content"},
				initialSymlinks: map[string]string{
					"link1": "link2",
					"link2": "target",
				},
			}},
			path: "link1",
		},
		{
			name: "Open relative symlink with parent ref",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"target": "target content"},
				initialDirs:  []string{"dir"},
				initialSymlinks: map[string]string{
					filepath.Join("dir", "link"): "../target",
				},
			}},
			path: filepath.Join("dir", "link"),
		},
		{
			name: "Open symlink in subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{filepath.Join("dir", "target"): "target content"},
				initialDirs:  []string{"dir"},
				initialSymlinks: map[string]string{
					filepath.Join("dir", "link"): "target",
				},
			}},
			path: filepath.Join("dir", "link"),
		},
		{
			name: "Open path with multiple symlinks",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{filepath.Join("real_dir1", "real_dir2", "file.txt"): "content"},
				initialDirs:  []string{filepath.Join("real_dir1", "real_dir2")},
				initialSymlinks: map[string]string{
					"link1":                             "real_dir1",
					filepath.Join("real_dir1", "link2"): "real_dir2",
				},
			}},
			path: filepath.Join("link1", "link2", "file.txt"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			realFile, realErr := realFS.Open(filepath.Join(realRoot, tc.path))
			if realErr == nil {
				defer realFile.Close()
			}
			testFile, testErr := testFS.Open(tc.path)
			if testErr == nil {
				defer testFile.Close()
			}

			requireErrorsMatch(t, realErr, testErr)

			if realErr == nil {
				realInfo, err := realFile.Stat()
				require.NoError(t, err)
				testInfo, err := testFile.Stat()
				require.NoError(t, err)

				require.Equal(t, realInfo.IsDir(), testInfo.IsDir(), "IsDir mismatch for opened path")

				if !realInfo.IsDir() {
					realContent, err := io.ReadAll(realFile)
					require.NoError(t, err)
					testContent, err := io.ReadAll(testFile)
					require.NoError(t, err)
					require.Equal(t, realContent, testContent)
				}
			}
		})
	}
}
