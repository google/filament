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
	"io/fs"
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/require"
)

// Tests for the Symlink() function in FSTestOSWrapper

// Unlike other test files for FSTestOSWrapper, this does not contain a
// _MatchesReal set of tests. This is because the effect of a symlink is
// inherently tied to other functions. Thus, _MatchesReal coverage is handled
// by tests for other functions, e.g. Stat_MatchesReal contains cases for
// stat-ing with symlinks.

func TestFSTestOSWrapper_Symlink(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name          string
		setup         unittestSetup
		oldname       string
		newname       string
		expectedKey   string
		expectedError expectedError
	}{
		{
			name:    "Create valid symlink",
			oldname: "target",
			newname: filepath.Join(root, "link"),
		},
		{
			name:    "Create symlink in subdirectory",
			setup:   unittestSetup{initialDirs: []string{filepath.Join(root, "subdir")}},
			oldname: "../target",
			newname: filepath.Join(root, "subdir", "link"),
		},
		{
			name:    "Destination exists",
			setup:   unittestSetup{initialFiles: map[string]string{filepath.Join(root, "exists"): ""}},
			oldname: "target",
			newname: filepath.Join(root, "exists"),
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name:    "Parent directory does not exist",
			oldname: "target",
			newname: filepath.Join(root, "missing", "link"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name:    "Parent is not a directory",
			setup:   unittestSetup{initialFiles: map[string]string{filepath.Join(root, "file"): ""}},
			oldname: "target",
			newname: filepath.Join(root, "file", "link"),
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Parent is a symlink to a directory",
			setup: unittestSetup{
				initialDirs:     []string{filepath.Join(root, "real_dir")},
				initialSymlinks: map[string]string{filepath.Join(root, "link_dir"): filepath.Join(root, "real_dir")},
			},
			oldname:     "target",
			newname:     filepath.Join(root, "link_dir", "link"),
			expectedKey: filepath.Join(root, "real_dir", "link"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			err := wrapper.Symlink(tc.oldname, tc.newname)

			if tc.expectedError.Check(t, err) {
				return
			}

			// Verify the symlink entry
			checkPath := tc.newname
			if tc.expectedKey != "" {
				checkPath = tc.expectedKey
			}
			cleanedPath := wrapper.CleanPath(checkPath)
			file, ok := wrapper.FS[cleanedPath]
			require.True(t, ok, "symlink entry not found in FS map")
			require.Equal(t, fs.ModeSymlink, file.Mode&fs.ModeSymlink)
			require.Equal(t, tc.oldname, string(file.Data))
		})
	}
}

func TestFSTestOSWrapper_Symlink_MatchesReal(t *testing.T) {
	tests := []struct {
		name    string
		setup   matchesRealSetup
		oldname string
		newname string
	}{
		{
			name:    "Create valid symlink",
			oldname: "target",
			newname: "link",
		},
		{
			name:    "Create symlink in subdirectory",
			setup:   matchesRealSetup{unittestSetup{initialDirs: []string{"subdir"}}},
			oldname: "../target",
			newname: filepath.Join("subdir", "link"),
		},
		{
			name:    "Destination exists",
			setup:   matchesRealSetup{unittestSetup{initialFiles: map[string]string{"exists": ""}}},
			oldname: "target",
			newname: "exists",
		},
		{
			name:    "Parent directory does not exist",
			oldname: "target",
			newname: filepath.Join("missing", "link"),
		},
		{
			name:    "Parent is not a directory",
			setup:   matchesRealSetup{unittestSetup{initialFiles: map[string]string{"file": ""}}},
			oldname: "target",
			newname: filepath.Join("file", "link"),
		},
		{
			name: "Parent is a symlink to a directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs:     []string{"real_dir"},
				initialSymlinks: map[string]string{"link_dir": "real_dir"},
			}},
			oldname: "target",
			newname: filepath.Join("link_dir", "link"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			realErr := realFS.Symlink(tc.oldname, filepath.Join(realRoot, tc.newname))
			testErr := testFS.Symlink(tc.oldname, tc.newname)

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}
