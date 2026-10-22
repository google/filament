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

// Tests for the WriteFile() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_WriteFile(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name          string
		setup         unittestSetup
		path          string
		content       []byte
		mode          os.FileMode
		expectContent *string
		expectedMode  os.FileMode
		expectedError
	}{
		{
			name:          "Create new file",
			path:          filepath.Join(root, "newfile.txt"),
			content:       []byte("new content"),
			mode:          0666,
			expectContent: stringPtr("new content"),
			expectedMode:  0666,
		},
		{
			name: "Overwrite existing file",
			path: filepath.Join(root, "existing.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "existing.txt"): "old content"},
			},
			content:       []byte("overwritten"),
			mode:          0666,
			expectContent: stringPtr("overwritten"),
			expectedMode:  0666,
		},
		{
			name: "Create file in existing subdirectory",
			path: filepath.Join(root, "foo", "bar.txt"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "foo")},
			},
			content:       []byte("sub content"),
			mode:          0666,
			expectContent: stringPtr("sub content"),
			expectedMode:  0666,
		},
		{
			name:    "Error on non-existent directory",
			path:    filepath.Join(root, "new", "dir", "file.txt"),
			content: []byte("some data"),
			mode:    0666,
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Error on path is a directory",
			path: filepath.Join(root, "mydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
			content: []byte("some data"),
			mode:    0666,
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
		},
		{
			name: "Error on parent path is a file",
			path: filepath.Join(root, "file.txt", "another.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "i am a file"},
			},
			content: []byte("some data"),
			mode:    0666,
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name:          "Create new file with specific mode",
			path:          filepath.Join(root, "newfile_mode.txt"),
			content:       []byte("new content"),
			mode:          0755,
			expectContent: stringPtr("new content"),
			expectedMode:  0755,
		},
		{
			name: "Overwrite existing file with different mode",
			path: filepath.Join(root, "existing.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "existing.txt"): "old content"},
			},
			content:       []byte("overwritten"),
			mode:          0777,
			expectContent: stringPtr("overwritten"),
			expectedMode:  0666, // Mode should be preserved
		},
		{
			name: "Write to symlink to file",
			path: filepath.Join(root, "link_to_file"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "old"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_file"): "file.txt",
				},
			},
			content:       []byte("new"),
			mode:          0666,
			expectContent: stringPtr("new"),
		},
		{
			name: "Write to broken symlink",
			path: filepath.Join(root, "broken_link"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "broken_link"): "target.txt",
				},
			},
			content:       []byte("created"),
			mode:          0666,
			expectContent: stringPtr("created"),
		},
		{
			name: "Write to symlink to dir",
			path: filepath.Join(root, "link_to_dir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_dir"): "dir",
				},
			},
			content: []byte("fail"),
			mode:    0666,
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
		},
		{
			name: "Write to symlink loop",
			path: filepath.Join(root, "loop1"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "loop1"): "loop2",
					filepath.Join(root, "loop2"): "loop1",
				},
			},
			content: []byte("fail"),
			mode:    0666,
			expectedError: expectedError{
				wantErrIs: syscall.ELOOP,
			},
		},
		{
			name: "Write through symlink in path",
			path: filepath.Join(root, "link_dir", "file.txt"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "real_dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_dir"): "real_dir",
				},
			},
			content:       []byte("content"),
			mode:          0666,
			expectContent: stringPtr("content"),
		},
		{
			name: "Write through chain of symlinks",
			path: filepath.Join(root, "link1"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "target.txt"): "old"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link1"): "link2",
					filepath.Join(root, "link2"): "target.txt",
				},
			},
			content:       []byte("new"),
			mode:          0666,
			expectContent: stringPtr("new"),
		},
		{
			name: "Write to symlink with relative target",
			path: filepath.Join(root, "subdir", "link"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "subdir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "subdir", "link"): "../target.txt",
				},
			},
			content:       []byte("content"),
			mode:          0666,
			expectContent: stringPtr("content"),
		},
		{
			name: "Write through broken symlink in path",
			path: filepath.Join(root, "link", "file.txt"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): "missing",
				},
			},
			content: []byte("fail"),
			mode:    0666,
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			err := wrapper.WriteFile(tc.path, tc.content, tc.mode)

			if tc.expectedError.Check(t, err) {
				return
			}

			if tc.expectContent != nil {
				content, err := wrapper.ReadFile(tc.path)
				require.NoError(t, err)
				require.Equal(t, *tc.expectContent, string(content))
			}

			if tc.expectedMode != 0 {
				cleanedPath := wrapper.CleanPath(tc.path)
				file, ok := wrapper.FS[cleanedPath]
				require.True(t, ok, "file %v not found in test fs", cleanedPath)
				require.Equal(t, tc.expectedMode, file.Mode.Perm())
			}
		})
	}
}

func TestFSTestOSWrapper_WriteFile_MatchesReal(t *testing.T) {
	tests := []struct {
		name    string
		setup   matchesRealSetup
		path    string
		content []byte
	}{
		{
			name:    "Create new file",
			path:    "newfile.txt",
			content: []byte("new content"),
		},
		{
			name: "Overwrite existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"existing.txt": "old content",
				},
			}},
			path:    "existing.txt",
			content: []byte("overwritten"),
		},
		// TODO(crbug.com/436025865): Add a test to check the file permissions on
		// overwritten files. This is not currently feasible since file permissions
		// will differ from the real filesystem due to the mock filesystem not
		// supporting umask.
		{
			name: "Create file in existing subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"foo"},
			}},
			path:    filepath.Join("foo", "bar.txt"),
			content: []byte("sub content"),
		},
		{
			name:    "Error on non-existent directory",
			path:    filepath.Join("new", "dir", "file.txt"),
			content: []byte("some data"),
		},
		{
			name: "Error on path is a directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"mydir"},
			}},
			path:    "mydir",
			content: []byte("some data"),
		},
		{
			name: "Error on parent path is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "i am a file",
				},
			}},
			path:    filepath.Join("file.txt", "another.txt"),
			content: []byte("some data"),
		},
		{
			name: "Write to symlink to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "old"},
				initialSymlinks: map[string]string{
					"link_to_file": "file.txt",
				},
			}},
			path:    "link_to_file",
			content: []byte("new"),
		},
		{
			name: "Write to broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"broken_link": "target.txt",
				},
			}},
			path:    "broken_link",
			content: []byte("created"),
		},
		{
			name: "Error on symlink to directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link_to_dir": "dir",
				},
			}},
			path:    "link_to_dir",
			content: []byte("fail"),
		},
		{
			name: "Write to symlink loop",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"loop1": "loop2",
					"loop2": "loop1",
				},
			}},
			path:    "loop1",
			content: []byte("fail"),
		},
		{
			name: "Write through symlink in path",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"real_dir"},
				initialSymlinks: map[string]string{
					"link_dir": "real_dir",
				},
			}},
			path:    filepath.Join("link_dir", "file.txt"),
			content: []byte("content"),
		},
		{
			name: "Write through chain of symlinks",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"target.txt": "old"},
				initialSymlinks: map[string]string{
					"link1": "link2",
					"link2": "target.txt",
				},
			}},
			path:    "link1",
			content: []byte("new"),
		},
		{
			name: "Write to symlink with relative target",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"subdir"},
				initialSymlinks: map[string]string{
					filepath.Join("subdir", "link"): "../target.txt",
				},
			}},
			path:    filepath.Join("subdir", "link"),
			content: []byte("content"),
		},
		{
			name: "Write through broken symlink in path",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"link": "missing",
				},
			}},
			path:    filepath.Join("link", "file.txt"),
			content: []byte("fail"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realErr := realFS.WriteFile(filepath.Join(realRoot, tc.path), tc.content, 0666)
			testErr := testFS.WriteFile(tc.path, tc.content, 0666)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}
