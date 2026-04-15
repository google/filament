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
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

// Tests for the MkdirAll() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_MkdirAll(t *testing.T) {
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
			name:         "Create new nested directory",
			path:         filepath.Join(root, "a", "b", "c"),
			mode:         0755,
			expectedMode: 0755,
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				for _, p := range []string{
					filepath.Join(root, "a"),
					filepath.Join(root, "a", "b"),
					filepath.Join(root, "a", "b", "c"),
				} {
					info, err := fs.Stat(p)
					require.NoError(t, err)
					require.True(t, info.IsDir())
					file, ok := fs.FS[fs.CleanPath(p)]
					require.True(t, ok)
					require.Equal(t, os.FileMode(0755), file.Mode.Perm())
				}
			},
		},
		{
			name:         "Create in existing subdirectory",
			path:         filepath.Join(root, "a", "b"),
			mode:         0755,
			expectedMode: 0755,
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "a")},
			},
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				info, err := fs.Stat(filepath.Join(root, "a", "b"))
				require.NoError(t, err)
				require.True(t, info.IsDir())
			},
		},
		{
			name:         "Create new nested directory with specific mode",
			path:         filepath.Join(root, "d", "e", "f"),
			mode:         0700,
			expectedMode: 0700,
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				for _, p := range []string{
					filepath.Join(root, "d"),
					filepath.Join(root, "d", "e"),
					filepath.Join(root, "d", "e", "f"),
				} {
					info, err := fs.Stat(p)
					require.NoError(t, err)
					require.True(t, info.IsDir())
					file, ok := fs.FS[fs.CleanPath(p)]
					require.True(t, ok)
					require.Equal(t, os.FileMode(0700), file.Mode.Perm())
				}
			},
		},
		{
			name: "Path already exists as directory",
			path: filepath.Join(root, "a", "b"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "a", "b")},
			},
		},
		{
			name: "Part of path is a file",
			path: filepath.Join(root, "a", "b", "c"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "a", "b"): "i am a file"},
				initialDirs:  []string{filepath.Join(root, "a")},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Destination path is a file",
			path: filepath.Join(root, "a", "b"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "a", "b"): "i am a file"},
				initialDirs:  []string{filepath.Join(root, "a")},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name: "Create nested directory through symlink",
			path: filepath.Join(root, "link_to_dir", "a", "b"),
			mode: 0755,
			setup: unittestSetup{
				initialDirs:     []string{filepath.Join(root, "real_dir")},
				initialSymlinks: map[string]string{filepath.Join(root, "link_to_dir"): filepath.Join(root, "real_dir")},
			},
			verify: func(t *testing.T, fs oswrapper.FSTestOSWrapper) {
				// Verify it exists in the real location
				info, err := fs.Stat(filepath.Join(root, "real_dir", "a", "b"))
				require.NoError(t, err)
				require.True(t, info.IsDir())
			},
		},
		{
			name: "Part of path is a broken symlink",
			path: filepath.Join(root, "broken_link", "a"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{filepath.Join(root, "broken_link"): filepath.Join(root, "nonexistent")},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist, // MkdirAll returns EEXIST because the broken link exists as a file/link
			},
		},
		{
			name: "Symlink loop in path",
			path: filepath.Join(root, "loop1", "a"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "loop1"): filepath.Join(root, "loop2"),
					filepath.Join(root, "loop2"): filepath.Join(root, "loop1"),
				},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist, // MkdirAll recurses on ELOOP and hits EEXIST on the loop link itself
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			err := wrapper.MkdirAll(tc.path, tc.mode)

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

func TestFSTestOSWrapper_MkdirAll_MatchesReal(t *testing.T) {
	tests := []struct {
		name  string
		setup matchesRealSetup
		path  string // path to MkdirAll
	}{
		{
			name: "Create new nested directory",
			path: filepath.Join("a", "b", "c"),
		},
		{
			name: "Create in existing subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"a"},
			}},
			path: filepath.Join("a", "b"),
		},
		{
			name: "Path already exists as directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{filepath.Join("a", "b", "c")},
			}},
			path: filepath.Join("a", "b", "c"),
		},
		{
			name: "Part of path is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{filepath.Join("a", "b"): "i am a file"},
				initialDirs:  []string{"a"},
			}},
			path: filepath.Join("a", "b", "c"),
		},
		{
			name: "Destination is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{filepath.Join("a", "b"): "i am a file"},
				initialDirs:  []string{"a"},
			}},
			path: filepath.Join("a", "b"),
		},
		{
			name: "Create nested directory through symlink",
			setup: matchesRealSetup{unittestSetup{
				initialDirs:     []string{"real_dir"},
				initialSymlinks: map[string]string{"link_to_dir": "real_dir"},
			}},
			path: filepath.Join("link_to_dir", "a", "b"),
		},
		{
			name: "Part of path is a broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{"broken_link": "nonexistent"},
			}},
			path: filepath.Join("broken_link", "a"),
		},
		{
			name: "Symlink loop in path",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"loop1": "loop2",
					"loop2": "loop1",
				},
			}},
			path: filepath.Join("loop1", "a"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realErr := realFS.MkdirAll(filepath.Join(realRoot, tc.path), 0755)
			testErr := testFS.MkdirAll(tc.path, 0755)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}
