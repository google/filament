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

	"github.com/stretchr/testify/require"
)

// Tests for the RemoveAll() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_RemoveAll(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name          string
		setup         unittestSetup
		path          string
		expectMissing []string
		expectPresent []string
		expectedError
	}{
		{
			name: "Remove single file",
			path: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			expectMissing: []string{filepath.Join(root, "file.txt")},
		},
		{
			name: "Remove directory with contents",
			path: filepath.Join(root, "dir"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file1.txt"):           "content",
					filepath.Join(root, "dir", "subdir", "file2.txt"): "content",
				},
				initialDirs: []string{
					filepath.Join(root, "dir"),
					filepath.Join(root, "dir", "subdir"),
				},
			},
			expectMissing: []string{
				filepath.Join(root, "dir"),
				filepath.Join(root, "dir", "file1.txt"),
				filepath.Join(root, "dir", "subdir"),
				filepath.Join(root, "dir", "subdir", "file2.txt"),
			},
		},
		{
			name: "Remove directory with contents, others retained",
			path: filepath.Join(root, "dir"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file1.txt"):           "content",
					filepath.Join(root, "dir", "subdir", "file2.txt"): "content",
					filepath.Join(root, "other.txt"):                  "other content",
				},
				initialDirs: []string{
					filepath.Join(root, "dir"),
					filepath.Join(root, "dir", "subdir"),
					filepath.Join(root, "otherdir"),
				},
			},
			expectMissing: []string{
				filepath.Join(root, "dir"),
				filepath.Join(root, "dir", "file1.txt"),
				filepath.Join(root, "dir", "subdir"),
				filepath.Join(root, "dir", "subdir", "file2.txt"),
			},
			expectPresent: []string{
				filepath.Join(root, "other.txt"),
				filepath.Join(root, "otherdir"),
			},
		},
		{
			name: "Remove non-existent path",
			path: filepath.Join(root, "nonexistent"),
		},
		{
			name: "Path is a file, but parent is not a directory",
			path: filepath.Join(root, "a", "b"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "a"): "i am a file"},
			},
			expectPresent: []string{
				filepath.Join(root, "a"),
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Remove symlink to file",
			path: filepath.Join(root, "link_to_file"),
			setup: unittestSetup{
				initialFiles:    map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialSymlinks: map[string]string{filepath.Join(root, "link_to_file"): filepath.Join(root, "file.txt")},
			},
			expectMissing: []string{filepath.Join(root, "link_to_file")},
			expectPresent: []string{filepath.Join(root, "file.txt")},
		},
		{
			name: "Remove symlink to directory",
			path: filepath.Join(root, "link_to_dir"),
			setup: unittestSetup{
				initialFiles:    map[string]string{filepath.Join(root, "dir", "file.txt"): "content"},
				initialDirs:     []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{filepath.Join(root, "link_to_dir"): filepath.Join(root, "dir")},
			},
			expectMissing: []string{filepath.Join(root, "link_to_dir")},
			expectPresent: []string{
				filepath.Join(root, "dir"),
				filepath.Join(root, "dir", "file.txt"),
			},
		},
		{
			name: "Remove contents inside symlinked directory",
			path: filepath.Join(root, "link_to_dir", "subdir"),
			setup: unittestSetup{
				initialFiles:    map[string]string{filepath.Join(root, "dir", "subdir", "file.txt"): "content"},
				initialDirs:     []string{filepath.Join(root, "dir", "subdir")},
				initialSymlinks: map[string]string{filepath.Join(root, "link_to_dir"): filepath.Join(root, "dir")},
			},
			expectMissing: []string{
				filepath.Join(root, "link_to_dir", "subdir"),
				filepath.Join(root, "dir", "subdir"),
				filepath.Join(root, "dir", "subdir", "file.txt"),
			},
			expectPresent: []string{
				filepath.Join(root, "dir"),
				filepath.Join(root, "link_to_dir"),
			},
		},
		{
			name: "Remove broken symlink",
			path: filepath.Join(root, "broken_link"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{filepath.Join(root, "broken_link"): "nonexistent"},
			},
			expectMissing: []string{filepath.Join(root, "broken_link")},
		},
		{
			name: "Remove file through symlinked directory",
			path: filepath.Join(root, "link_to_dir", "file.txt"),
			setup: unittestSetup{
				initialFiles:    map[string]string{filepath.Join(root, "dir", "file.txt"): "content"},
				initialDirs:     []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{filepath.Join(root, "link_to_dir"): filepath.Join(root, "dir")},
			},
			expectMissing: []string{filepath.Join(root, "dir", "file.txt")},
			expectPresent: []string{
				filepath.Join(root, "dir"),
				filepath.Join(root, "link_to_dir"),
			},
		},
		{
			name: "Remove symlink loop",
			path: filepath.Join(root, "link1"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "link1"): "link2",
					filepath.Join(root, "link2"): "link1",
				},
			},
			expectMissing: []string{filepath.Join(root, "link1"), filepath.Join(root, "link2")},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			err := wrapper.RemoveAll(tc.path)

			for _, p := range tc.expectPresent {
				_, statErr := wrapper.Stat(p)
				require.NoError(t, statErr, "path '%s' should exist but it does not", p)
			}

			if tc.expectedError.Check(t, err) {
				return
			}

			for _, p := range tc.expectMissing {
				_, statErr := wrapper.Stat(p)
				require.Error(t, statErr, "path '%s' should not exist but it does", p)
				require.True(t, os.IsNotExist(statErr))
			}
		})
	}
}

func TestFSTestOSWrapper_RemoveAll_MatchesReal(t *testing.T) {
	tests := []struct {
		name         string
		setup        matchesRealSetup
		pathToRemove string
	}{
		{
			name: "Remove single file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
			}},
			pathToRemove: "file.txt",
		},
		{
			name: "Remove directory with contents",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "file1.txt"):           "content",
					filepath.Join("dir", "subdir", "file2.txt"): "content",
				},
				initialDirs: []string{
					"dir",
					filepath.Join("dir", "subdir"),
				},
			}},
			pathToRemove: "dir",
		},
		{
			name: "Remove directory with contents, others retained",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "file1.txt"):           "content",
					filepath.Join("dir", "subdir", "file2.txt"): "content",
					"other.txt": "other content",
				},
				initialDirs: []string{
					"dir",
					filepath.Join("dir", "subdir"),
					"otherdir",
				},
			}},
			pathToRemove: "dir",
		},
		{
			name:         "Remove non-existent path",
			pathToRemove: "nonexistent",
		},
		{
			name: "Path is a file, but parent is not a directory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"a": "i am a file",
				},
			}},
			pathToRemove: filepath.Join("a", "b"),
		},
		{
			name: "Remove symlink to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles:    map[string]string{"file.txt": "content"},
				initialSymlinks: map[string]string{"link_to_file": "file.txt"},
			}},
			pathToRemove: "link_to_file",
		},
		{
			name: "Remove symlink to directory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles:    map[string]string{filepath.Join("dir", "file.txt"): "content"},
				initialDirs:     []string{"dir"},
				initialSymlinks: map[string]string{"link_to_dir": "dir"},
			}},
			pathToRemove: "link_to_dir",
		},
		{
			name: "Remove contents inside symlinked directory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles:    map[string]string{filepath.Join("dir", "subdir", "file.txt"): "content"},
				initialDirs:     []string{filepath.Join("dir", "subdir")},
				initialSymlinks: map[string]string{"link_to_dir": "dir"},
			}},
			pathToRemove: filepath.Join("link_to_dir", "subdir"),
		},
		{
			name: "Remove broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{"broken_link": "nonexistent"},
			}},
			pathToRemove: "broken_link",
		},
		{
			name: "Remove file through symlinked directory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles:    map[string]string{filepath.Join("dir", "file.txt"): "content"},
				initialDirs:     []string{"dir"},
				initialSymlinks: map[string]string{"link_to_dir": "dir"},
			}},
			pathToRemove: filepath.Join("link_to_dir", "file.txt"),
		},
		{
			name: "Remove symlink loop",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"link1": "link2",
					"link2": "link1",
				},
			}},
			pathToRemove: "link1",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute
			realErr := realFS.RemoveAll(filepath.Join(realRoot, tc.pathToRemove))
			testErr := testFS.RemoveAll(tc.pathToRemove)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}
