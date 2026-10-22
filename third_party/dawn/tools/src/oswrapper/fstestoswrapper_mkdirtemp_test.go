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
	"strconv"
	"strings"
	"syscall"
	"testing"

	"github.com/stretchr/testify/require"
)

// Tests for the MkdirTemp() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_MkdirTemp(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name           string
		setup          unittestSetup
		dir            string
		pattern        string
		expectedPrefix string
		expectedSuffix string
		expectedError
	}{
		{
			name:    "Simple pattern",
			dir:     filepath.Join(root, "tmp"),
			pattern: "test-",
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "tmp")},
			},
			expectedPrefix: "test-",
		},
		{
			name:    "Pattern with star",
			dir:     filepath.Join(root, "tmp"),
			pattern: "test-*-suffix",
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "tmp")},
			},
			expectedPrefix: "test-",
			expectedSuffix: "-suffix",
		},
		{
			name:    "Base dir does not exist",
			dir:     filepath.Join(root, "nonexistent"),
			pattern: "test-",
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name:    "Base dir is a file",
			dir:     filepath.Join(root, "tmpfile"),
			pattern: "test-",
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "tmpfile"): ""},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name:    "Base dir is a symlink to a directory",
			dir:     filepath.Join(root, "link"),
			pattern: "test-",
			setup: unittestSetup{
				initialDirs:     []string{filepath.Join(root, "target")},
				initialSymlinks: map[string]string{filepath.Join(root, "link"): filepath.Join(root, "target")},
			},
			expectedPrefix: "test-",
		},
		{
			name:    "Base dir is a symlink to a file",
			dir:     filepath.Join(root, "link"),
			pattern: "test-",
			setup: unittestSetup{
				initialFiles:    map[string]string{filepath.Join(root, "tmpfile"): ""},
				initialSymlinks: map[string]string{filepath.Join(root, "link"): filepath.Join(root, "tmpfile")},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name:    "Base dir is a broken symlink",
			dir:     filepath.Join(root, "broken_link"),
			pattern: "test-",
			setup: unittestSetup{
				initialSymlinks: map[string]string{"broken_link": "nonexistent"},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name:    "Base dir is a symlink loop",
			dir:     filepath.Join(root, "loop1"),
			pattern: "test-",
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
		{
			name:    "Base dir is a chain of symlinks",
			dir:     filepath.Join(root, "link1"),
			pattern: "test-",
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "target")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link1"): filepath.Join(root, "link2"),
					filepath.Join(root, "link2"): filepath.Join(root, "target"),
				},
			},
			expectedPrefix: "test-",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)

			gotPath, err := wrapper.MkdirTemp(tc.dir, tc.pattern)

			if tc.expectedError.Check(t, err) {
				return
			}

			require.Equal(t, wrapper.CleanPath(tc.dir), wrapper.CleanPath(filepath.Dir(gotPath)))

			name := filepath.Base(gotPath)
			require.True(t, strings.HasPrefix(name, tc.expectedPrefix), "name '%s' should have prefix '%s'", name, tc.expectedPrefix)
			require.True(t, strings.HasSuffix(name, tc.expectedSuffix), "name '%s' should have suffix '%s'", name, tc.expectedSuffix)

			// Tests running in parallel could lead to variance in the random part of the name, so
			// just checking that is well-formed
			randomPart := strings.TrimPrefix(name, tc.expectedPrefix)
			randomPart = strings.TrimSuffix(randomPart, tc.expectedSuffix)
			_, err = strconv.ParseUint(randomPart, 10, 32)
			require.NoError(t, err, "random part '%s' is not a valid u32", randomPart)

			info, err := wrapper.Stat(gotPath)
			require.NoError(t, err, "Stat on created temp dir failed")
			require.True(t, info.IsDir(), "Created temp path is not a directory")
		})
	}
}

func TestFSTestOSWrapper_MkdirTemp_MatchesReal(t *testing.T) {
	tests := []struct {
		name    string
		setup   matchesRealSetup
		dir     string // base directory for MkdirTemp
		pattern string
	}{
		{
			name: "Simple creation",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"tmp"},
			}},
			dir:     "tmp",
			pattern: "test-",
		},
		{
			name: "Pattern with star",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"tmp"},
			}},
			dir:     "tmp",
			pattern: "test-*-suffix",
		},
		{
			name:    "Error on non-existent base dir",
			dir:     "nonexistent",
			pattern: "test-",
		},
		{
			name: "Error on base dir is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"tmpfile": ""},
			}},
			dir:     "tmpfile",
			pattern: "test-",
		},
		{
			name: "Base dir is a symlink to a directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs:     []string{"target"},
				initialSymlinks: map[string]string{"link": "target"},
			}},
			dir:     "link",
			pattern: "test-",
		},
		{
			name: "Base dir is a symlink to a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles:    map[string]string{"target_file": ""},
				initialSymlinks: map[string]string{"link_file": "target_file"},
			}},
			dir:     "link_file",
			pattern: "test-",
		},
		{
			name: "Base dir is a broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{"broken_link": "nonexistent"},
			}},
			dir:     "broken_link",
			pattern: "test-",
		},
		{
			name: "Base dir is a symlink loop",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"loop1": "loop2",
					"loop2": "loop1",
				},
			}},
			dir:     "loop1",
			pattern: "test-",
		},
		{
			name: "Base dir is a chain of symlinks",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"target"},
				initialSymlinks: map[string]string{
					"link1": "link2",
					"link2": "target",
				},
			}},
			dir:     "link1",
			pattern: "test-",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			realDir := filepath.Join(realRoot, tc.dir)
			testDir := tc.dir

			_, realErr := realFS.MkdirTemp(realDir, tc.pattern)
			_, testErr := testFS.MkdirTemp(testDir, tc.pattern)

			requireErrorsMatch(t, realErr, testErr)

			// Not using requireFileSystemsMatch here, because that would require FSTestOSWrapper to
			// implement the exact same behaviour as the real version. This would be difficult to
			// achieve, since the real version is intentionally designed to be hard to predict,
			// and its sources of entropy may include things like wall time or other system values.
			if realErr == nil {
				realEntries, err := os.ReadDir(realDir)
				require.NoError(t, err)
				testEntries, err := testFS.ReadDir(testDir)
				require.NoError(t, err)

				// Confirm a new directory was created for both
				require.Len(t, realEntries, 1, "expected one entry in real directory")
				require.Len(t, testEntries, 1, "expected one entry in test directory")

				realInfo, err := realEntries[0].Info()
				require.NoError(t, err)
				testInfo, err := testEntries[0].Info()
				require.NoError(t, err)

				require.True(t, realInfo.IsDir(), "real entry should be a directory")
				require.True(t, testInfo.IsDir(), "test entry should be a directory")

				// Check permissions are correct. os.MkdirTemp creates directories with mode 0700.
				require.Equal(t, os.FileMode(0700), realInfo.Mode().Perm(), "real directory permissions mismatch")
				require.Equal(t, os.FileMode(0700), testInfo.Mode().Perm(), "test directory permissions mismatch")
			}
		})
	}
}
