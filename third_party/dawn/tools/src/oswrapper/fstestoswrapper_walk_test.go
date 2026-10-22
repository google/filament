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
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"syscall"
	"testing"

	"github.com/stretchr/testify/require"
)

// Tests for the Walk() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_Walk(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name          string
		setup         unittestSetup
		root          string
		walkFn        func(t *testing.T, visited *[]string) filepath.WalkFunc
		expectedPaths []string
		expectedError
	}{
		{
			name: "Walk directory structure",
			root: root,
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file2.txt"):          "",
					filepath.Join(root, "dir", "subdir", "file.txt"): "",
				},
				initialDirs: []string{filepath.Join(root, "dir", "subdir")},
			},
			walkFn: func(t *testing.T, visited *[]string) filepath.WalkFunc {
				return func(path string, info os.FileInfo, err error) error {
					require.NoError(t, err)
					*visited = append(*visited, path)
					return nil
				}
			},
			expectedPaths: []string{
				root,
				filepath.Join(root, "dir"),
				filepath.Join(root, "dir", "file2.txt"),
				filepath.Join(root, "dir", "subdir"),
				filepath.Join(root, "dir", "subdir", "file.txt"),
			},
		},
		{
			name: "Walk non-existent root",
			root: filepath.Join(root, "nonexistent"),
			walkFn: func(t *testing.T, visited *[]string) filepath.WalkFunc {
				return func(path string, info os.FileInfo, err error) error {
					require.Error(t, err, "expected an error for non-existent root")
					require.ErrorIs(t, err, os.ErrNotExist)
					require.Nil(t, info, "info should be nil on error")
					require.Equal(t, filepath.Join(root, "nonexistent"), path)
					*visited = append(*visited, path)
					return err // Propagate the error to stop the walk and return it from Walk()
				}
			},
			expectedPaths: []string{filepath.Join(root, "nonexistent")},
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Walk a file",
			root: filepath.Join(root, "file.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): ""},
			},
			walkFn: func(t *testing.T, visited *[]string) filepath.WalkFunc {
				return func(path string, info os.FileInfo, err error) error {
					require.NoError(t, err)
					*visited = append(*visited, path)
					return nil
				}
			},
			expectedPaths: []string{filepath.Join(root, "file.txt")},
		},
		{
			name: "Error from walk function",
			root: root,
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "a"): "",
					filepath.Join(root, "dir", "b"): "",
				},
				initialDirs: []string{filepath.Join(root, "dir")},
			},
			walkFn: func(t *testing.T, visited *[]string) filepath.WalkFunc {
				return func(path string, info os.FileInfo, err error) error {
					if strings.HasSuffix(path, "b") {
						return fmt.Errorf("stop walking")
					}
					return nil
				}
			},
			expectedError: expectedError{
				wantErrMsg: "stop walking",
			},
		},
		{
			name: "Skip a directory",
			root: root,
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "subdir", "file.txt"):      "",
					filepath.Join(root, "dir", "anotherdir", "file2.txt"): "",
				},
				initialDirs: []string{
					filepath.Join(root, "dir", "subdir"),
					filepath.Join(root, "dir", "anotherdir"),
				},
			},
			walkFn: func(t *testing.T, visited *[]string) filepath.WalkFunc {
				return func(path string, info os.FileInfo, err error) error {
					require.NoError(t, err)
					*visited = append(*visited, path)
					if info.IsDir() && path == filepath.Join(root, "dir", "subdir") {
						return filepath.SkipDir
					}
					return nil
				}
			},
			expectedPaths: []string{
				root,
				filepath.Join(root, "dir"),
				filepath.Join(root, "dir", "anotherdir"),
				filepath.Join(root, "dir", "anotherdir", "file2.txt"),
				filepath.Join(root, "dir", "subdir"),
			},
		},
		{
			name: "Walk symlink to directory",
			root: filepath.Join(root, "link"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file.txt"): "",
				},
				initialDirs: []string{
					filepath.Join(root, "dir"),
				},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): filepath.Join(root, "dir"),
				},
			},
			walkFn: func(t *testing.T, visited *[]string) filepath.WalkFunc {
				return func(path string, info os.FileInfo, err error) error {
					require.NoError(t, err)
					*visited = append(*visited, path)
					return nil
				}
			},
			expectedPaths: []string{
				filepath.Join(root, "link"),
			},
		},
		{
			name: "Walk through symlink to directory",
			root: filepath.Join(root, "link", "subdir"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "subdir", "file.txt"): "",
				},
				initialDirs: []string{
					filepath.Join(root, "dir", "subdir"),
				},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): filepath.Join(root, "dir"),
				},
			},
			walkFn: func(t *testing.T, visited *[]string) filepath.WalkFunc {
				return func(path string, info os.FileInfo, err error) error {
					require.NoError(t, err)
					*visited = append(*visited, path)
					return nil
				}
			},
			expectedPaths: []string{
				filepath.Join(root, "link", "subdir"),
				filepath.Join(root, "link", "subdir", "file.txt"),
			},
		},
		{
			name: "Walk directory containing a symlink",
			root: filepath.Join(root, "dir"),
			setup: unittestSetup{
				initialFiles: map[string]string{
					filepath.Join(root, "dir", "file.txt"):  "",
					filepath.Join(root, "other", "foo.txt"): "",
				},
				initialDirs: []string{
					filepath.Join(root, "dir"),
					filepath.Join(root, "other"),
				},
				initialSymlinks: map[string]string{
					filepath.Join(root, "dir", "link"): filepath.Join(root, "other"),
				},
			},
			walkFn: func(t *testing.T, visited *[]string) filepath.WalkFunc {
				return func(path string, info os.FileInfo, err error) error {
					require.NoError(t, err)
					*visited = append(*visited, path)
					return nil
				}
			},
			expectedPaths: []string{
				filepath.Join(root, "dir"),
				filepath.Join(root, "dir", "file.txt"),
				filepath.Join(root, "dir", "link"),
			},
		},
		{
			name: "Walk broken symlink",
			root: filepath.Join(root, "link"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): filepath.Join(root, "nonexistent"),
				},
			},
			walkFn: func(t *testing.T, visited *[]string) filepath.WalkFunc {
				return func(path string, info os.FileInfo, err error) error {
					require.NoError(t, err)
					*visited = append(*visited, path)
					return nil
				}
			},
			expectedPaths: []string{
				filepath.Join(root, "link"),
			},
		},
		{
			name: "Walk path with symlink loop",
			root: filepath.Join(root, "loop1", "foo"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "loop1"): filepath.Join(root, "loop2"),
					filepath.Join(root, "loop2"): filepath.Join(root, "loop1"),
				},
			},
			walkFn: func(t *testing.T, visited *[]string) filepath.WalkFunc {
				return func(path string, info os.FileInfo, err error) error {
					// We expect an error here because the root path cannot be resolved
					require.Error(t, err)
					require.ErrorIs(t, err, syscall.ELOOP)
					*visited = append(*visited, path)
					return err
				}
			},
			expectedPaths: []string{
				filepath.Join(root, "loop1", "foo"),
			},
			expectedError: expectedError{
				wantErrIs: syscall.ELOOP,
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)

			var visited []string
			var walkFn filepath.WalkFunc
			if tc.walkFn != nil {
				walkFn = tc.walkFn(t, &visited)
			}

			err := wrapper.Walk(tc.root, walkFn)

			if !tc.expectedError.Check(t, err) {
				if tc.expectedPaths != nil {
					require.ElementsMatch(t, tc.expectedPaths, visited)
				}
			}
		})
	}
}

func TestFSTestOSWrapper_Walk_MatchesReal(t *testing.T) {
	tests := []struct {
		name         string
		setup        matchesRealSetup
		root         string
		walkBehavior func(path string, info os.FileInfo) error
	}{
		{
			name: "Simple walk",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "subdir", "file.txt"): "",
					filepath.Join("dir", "file2.txt"):          "",
				},
				initialDirs: []string{
					"dir",
					filepath.Join("dir", "subdir"),
				},
			}},
			root: "dir",
		},
		{
			name: "Walk a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": ""},
			}},
			root: "file.txt",
		},
		{
			name: "Walk non-existent root",
			root: "nonexistent",
		},
		{
			name: "Skip a directory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "subdir", "file.txt"):     "",
					filepath.Join("dir", "anotherdir", "file.txt"): "",
				},
				initialDirs: []string{
					"dir",
					filepath.Join("dir", "subdir"),
					filepath.Join("dir", "anotherdir"),
				},
			}},
			root: "dir",
			walkBehavior: func(path string, info os.FileInfo) error {
				if info.IsDir() && strings.HasSuffix(path, "subdir") {
					return filepath.SkipDir
				}
				return nil
			},
		},
		{
			name: "Error from walk function",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "a"): "",
					filepath.Join("dir", "b"): "",
				},
				initialDirs: []string{"dir"},
			}},
			root: "dir",
			walkBehavior: func(path string, info os.FileInfo) error {
				if !info.IsDir() && strings.HasSuffix(path, "b") {
					return errors.New("stop walking")
				}
				return nil
			},
		},
		{
			name: "Walk symlink to directory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "file.txt"): "",
				},
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link": "dir",
				},
			}},
			root: "link",
		},
		{
			name: "Walk through symlink to directory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "subdir", "file.txt"): "",
				},
				initialDirs: []string{filepath.Join("dir", "subdir")},
				initialSymlinks: map[string]string{
					"link": "dir",
				},
			}},
			root: filepath.Join("link", "subdir"),
		},
		{
			name: "Walk directory containing a symlink",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					filepath.Join("dir", "file.txt"):  "",
					filepath.Join("other", "foo.txt"): "",
				},
				initialDirs: []string{"dir", "other"},
				initialSymlinks: map[string]string{
					filepath.Join("dir", "link"): filepath.Join("other"),
				},
			}},
			root: "dir",
		},
		{
			name: "Walk broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"link": "nonexistent",
				},
			}},
			root: "link",
		},
		{
			name: "Walk path with symlink loop",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"loop1": "loop2",
					"loop2": "loop1",
				},
			}},
			root: filepath.Join("loop1", "foo"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute and Compare
			var realPaths []string
			realWalkFn := func(path string, info os.FileInfo, err error) error {
				if err != nil {
					return err
				}
				relPath, err := filepath.Rel(realRoot, path)
				require.NoError(t, err)
				realPaths = append(realPaths, filepath.ToSlash(relPath))
				if tc.walkBehavior != nil {
					return tc.walkBehavior(relPath, info)
				}
				return nil
			}
			realErr := realFS.Walk(filepath.Join(realRoot, tc.root), realWalkFn)

			var testPaths []string
			testWalkFn := func(path string, info os.FileInfo, err error) error {
				if err != nil {
					return err
				}
				relPath := strings.TrimPrefix(path, "/")
				testPaths = append(testPaths, relPath)
				if tc.walkBehavior != nil {
					return tc.walkBehavior(relPath, info)
				}
				return nil
			}
			testErr := testFS.Walk(tc.root, testWalkFn)

			requireErrorsMatch(t, realErr, testErr)

			require.ElementsMatch(t, realPaths, testPaths)
		})
	}
}
