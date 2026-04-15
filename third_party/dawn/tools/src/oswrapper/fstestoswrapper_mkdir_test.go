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

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

// Tests for the Mkdir() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_Mkdir(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name         string
		setup        unittestSetup
		path         string
		mode         os.FileMode
		expectedMode os.FileMode
		verify       func(t *testing.T, fs oswrapper.FSTestOSWrapper)
		expectedError
	}{
		{
			name:         "Create new directory",
			path:         filepath.Join(root, "newdir"),
			mode:         0755,
			expectedMode: 0755,
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				info, err := fs.Stat(filepath.Join(root, "newdir"))
				require.NoError(t, err)
				require.True(t, info.IsDir())
			},
		},
		{
			name:         "Create in existing subdirectory",
			path:         filepath.Join(root, "existing", "newdir"),
			mode:         0755,
			expectedMode: 0755,
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "existing")},
			},
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				info, err := fs.Stat(filepath.Join(root, "existing", "newdir"))
				require.NoError(t, err)
				require.True(t, info.IsDir())
			},
		},
		{
			name:         "Create new directory with specific mode",
			path:         filepath.Join(root, "newdir_mode"),
			mode:         0700,
			expectedMode: 0700,
		},
		{
			name: "Directory already exists",
			path: filepath.Join(root, "mydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name: "Path is a file",
			path: filepath.Join(root, "myfile.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "myfile.txt"): ""},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name: "Parent directory does not exist",
			path: filepath.Join(root, "nonexistent", "newdir"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Parent path is a file",
			path: filepath.Join(root, "file.txt", "newdir"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): ""},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Path is dot",
			path: ".",
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name: "Create inside symlinked directory",
			path: filepath.Join(root, "link_to_dir", "newdir"),
			mode: 0755,
			setup: unittestSetup{
				initialDirs:     []string{filepath.Join(root, "real_dir")},
				initialSymlinks: map[string]string{filepath.Join(root, "link_to_dir"): filepath.Join(root, "real_dir")},
			},
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				// Verify it exists in the real location
				info, err := fs.Stat(filepath.Join(root, "real_dir", "newdir"))
				require.NoError(t, err)
				require.True(t, info.IsDir())

				// Verify it can be accessed via the symlink
				info, err = fs.Stat(filepath.Join(root, "link_to_dir", "newdir"))
				require.NoError(t, err)
				require.True(t, info.IsDir())
			},
		},
		{
			name: "Parent is symlink to file",
			path: filepath.Join(root, "link_to_file", "newdir"),
			setup: unittestSetup{
				initialFiles:    map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialSymlinks: map[string]string{filepath.Join(root, "link_to_file"): filepath.Join(root, "file.txt")},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Path is broken symlink",
			path: filepath.Join(root, "broken_link"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{filepath.Join(root, "broken_link"): filepath.Join(root, "nonexistent")},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name: "Parent path is broken symlink",
			path: filepath.Join(root, "broken_link", "newdir"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{filepath.Join(root, "broken_link"): filepath.Join(root, "nonexistent")},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Symlink loop in path",
			path: filepath.Join(root, "loop1", "newdir"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "loop1"): filepath.Join(root, "loop2"),
					filepath.Join(root, "loop2"): filepath.Join(root, "loop1"),
				},
			},
			expectedError: expectedError{
				wantErrIs: syscall.ELOOP,
			},
		},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			err := wrapper.Mkdir(tc.path, tc.mode)

			if tc.expectedError.Check(t, err) {
				return
			}

			if tc.verify != nil {
				tc.verify(t, wrapper)
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

func TestFSTestOSWrapper_Mkdir_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string // path to Mkdir
	}{
		{
			name: "Create new directory",
			path: "newdir",
		},
		{
			name: "Create in existing subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"foo"},
			}},
			path: filepath.Join("foo", "newdir"),
		},
		{
			name: "Create in existing nested subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{filepath.Join("foo", "bar")},
			}},
			path: filepath.Join("foo", "bar", "newdir"),
		},
		{
			name: "Error on existing directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"mydir"},
			}},
			path: "mydir",
		},
		{
			name: "Error on existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"myfile": "content"},
			}},
			path: "myfile",
		},
		{
			name: "Error on non-existent parent",
			path: filepath.Join("nonexistent", "newdir"),
		},
		{
			name: "Error on parent is file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
			}},
			path: filepath.Join("file.txt", "newdir"),
		},
		{
			name: "Error on dot",
			path: ".",
		},
		{
			name: "Create inside symlinked directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs:     []string{"real_dir"},
				initialSymlinks: map[string]string{"link_to_dir": "real_dir"},
			}},
			path: filepath.Join("link_to_dir", "newdir"),
		},
		{
			name: "Parent is symlink to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles:    map[string]string{"file.txt": "content"},
				initialSymlinks: map[string]string{"link_to_file": "file.txt"},
			}},
			path: filepath.Join("link_to_file", "newdir"),
		},
		{
			name: "Path is broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{"broken_link": "nonexistent"},
			}},
			path: "broken_link",
		},
		{
			name: "Parent path is broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{"broken_link": "nonexistent"},
			}},
			path: filepath.Join("broken_link", "newdir"),
		},
		{
			name: "Symlink loop in path",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"loop1": "loop2",
					"loop2": "loop1",
				},
			}},
			path: filepath.Join("loop1", "newdir"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realErr := realFS.Mkdir(filepath.Join(realRoot, tc.path), 0755)
			testErr := testFS.Mkdir(tc.path, 0755)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}
