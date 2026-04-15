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
	"os"
	"path/filepath"
	"runtime"
	"syscall"
	"testing"
	"testing/fstest"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

// Common code used by tests for fstestoswrapper.go.

// --- Generic helpers ---

// stringPtr is a helper to get a pointer to a string, used so test cases can have nil to indicate a
// string value isn't set, instead of testing for "".
func stringPtr(s string) *string {
	return &s
}

// getTestRoot returns the standard root path for the current OS.
func getTestRoot() string {
	if runtime.GOOS == "windows" {
		return "C:\\"
	}
	return "/"
}

// --- Unittest specific helpers ---

// unittestSetup is a helper for setting up the FSTestOSWrapper for an unittest.
type unittestSetup struct {
	initialFiles    map[string]string
	initialDirs     []string
	initialSymlinks map[string]string // map[linkPath]targetPath
}

// setup initializes the FSTestOSWrapper with the files and directories specified
// in the unittestSetup struct.
func (s unittestSetup) setup(t *testing.T) oswrapper.FSTestOSWrapper {
	t.Helper()
	testFS := oswrapper.CreateFSTestOSWrapper()

	for path, content := range s.initialFiles {
		require.NoError(t, testFS.MkdirAll(filepath.Dir(path), 0755))
		require.NoError(t, testFS.WriteFile(path, []byte(content), 0666))
	}
	for _, path := range s.initialDirs {
		require.NoError(t, testFS.MkdirAll(path, 0755))
	}
	for linkPath, targetPath := range s.initialSymlinks {
		require.NoError(t, testFS.MkdirAll(filepath.Dir(linkPath), 0755))
		require.NoError(t, testFS.Symlink(targetPath, linkPath))
	}
	return testFS
}

// expectedError is a helper struct for testing error conditions.
// Note: This is meant for usage in the non-*_MatchesReal unittests.
type expectedError struct {
	wantErrIs  error
	wantErrMsg string
}

// Check verifies that the given error `err` matches the expected error conditions.
// It returns true if an error was expected and found, indicating the test should stop.
func (e expectedError) Check(t *testing.T, err error) (stopTest bool) {
	t.Helper()
	if e.wantErrIs == nil && e.wantErrMsg == "" {
		require.NoError(t, err)
		return false
	}

	require.Error(t, err)
	if e.wantErrIs != nil {
		require.ErrorIs(t, err, e.wantErrIs)
	}
	if e.wantErrMsg != "" {
		require.ErrorContains(t, err, e.wantErrMsg)
	}
	return true
}

// --- *_MatchesReal specific helpers ---

// matchesRealSetup is a helper for setting up the filesystem for a test case that
// matches against the real OS.
// Note: This is meant for usage in the *_MatchesReal tests.
type matchesRealSetup struct {
	unittestSetup
}

// setup creates a temporary directory for the real OS, and initializes both the
// real and test filesystems with the files and directories specified in the
// matchesRealSetup struct.
func (s matchesRealSetup) setup(t *testing.T) (string, oswrapper.OSWrapper, oswrapper.FSTestOSWrapper) {
	t.Helper()

	// Setup the test wrapper using the embedded setup method.
	testFS := s.unittestSetup.setup(t)

	// Setup the real FS
	realRoot, err := os.MkdirTemp("", "real_fs_test_*")
	require.NoError(t, err)
	realFS := oswrapper.GetRealOSWrapper()

	for path, content := range s.initialFiles {
		realPath := filepath.Join(realRoot, path)
		require.NoError(t, os.MkdirAll(filepath.Dir(realPath), 0755))
		require.NoError(t, os.WriteFile(realPath, []byte(content), 0666))
	}
	for _, path := range s.initialDirs {
		require.NoError(t, os.MkdirAll(filepath.Join(realRoot, path), 0755))
	}
	for linkPath, targetPath := range s.initialSymlinks {
		realPath := filepath.Join(realRoot, linkPath)
		require.NoError(t, os.MkdirAll(filepath.Dir(realPath), 0755))
		require.NoError(t, os.Symlink(targetPath, realPath))
	}

	return realRoot, realFS, testFS
}

// requireFileSystemsMatch walks the real filesystem at realRoot and compares its
// structure and file content against the provided FSTestOSWrapper.
// Note: This is meant for usage in *_MatchesReal tests.
func requireFileSystemsMatch(t *testing.T, realRoot string, testFS oswrapper.FSTestOSWrapper) {
	t.Helper()

	realMap := make(map[string]*fstest.MapFile)
	// We use filepath.Walk which uses Lstat, so we see symlinks.
	err := filepath.Walk(realRoot, func(path string, info os.FileInfo, err error) error {
		require.NoError(t, err)
		if path == realRoot {
			return nil
		}

		relPath, err := filepath.Rel(realRoot, path)
		require.NoError(t, err)

		// Use the same path cleaning logic as FSTestOSWrapper.
		mapKey := testFS.FSTestFilesystemReaderWriter.CleanPath(relPath)

		mapFile := &fstest.MapFile{Mode: info.Mode()}
		if info.Mode()&os.ModeSymlink != 0 {
			target, err := os.Readlink(path)
			require.NoError(t, err)
			mapFile.Data = []byte(target)
		} else if !info.IsDir() {
			data, err := os.ReadFile(path)
			require.NoError(t, err)
			mapFile.Data = data
		}
		realMap[mapKey] = mapFile
		return nil
	})
	require.NoError(t, err)

	testMap := testFS.FS
	require.Len(t, testMap, len(realMap), "Filesystems have a different number of entries")

	for key, realFile := range realMap {
		testFile, ok := testMap[key]
		require.True(t, ok, "Path '%s' exists in real FS but not in FS under test", key)

		// Compare file modes. Note: fs.FS doesn't strictly guarantee strict equality of all bits,
		// but for our purposes (Dir, Symlink, Perms) they should match.
		// However, fstest.MapFS might normalize modes.
		require.Equal(t, realFile.Mode.IsDir(), testFile.Mode.IsDir(), "IsDir mismatch for '%s'", key)
		require.Equal(t, realFile.Mode&os.ModeSymlink, testFile.Mode&os.ModeSymlink, "IsSymlink mismatch for '%s'", key)
		// TODO(crbug.com/436025865): Add a check that the permissions match once
		// the mock filesystem supports umasks.

		if !realFile.Mode.IsDir() {
			require.Equal(t, realFile.Data, testFile.Data, "Content/Target mismatch for '%s'", key)
		}
	}
}

// requireErrorsMatch asserts that the real and test errors are compatible.
// Both can be nil. If the real error is a well-known os error, the test error
// must be the same. Otherwise, it just checks that both are non-nil.
// Note: This is meant for usage in *_MatchesReal tests.
func requireErrorsMatch(t *testing.T, realErr, testErr error) {
	t.Helper()
	if realErr != nil {
		require.Error(t, testErr, "Real FS errored but FS under test did not.\nReal error: %v", realErr)

		// For certain well-defined errors, we expect the test wrapper to return
		// the exact same error type.
		if errors.Is(realErr, os.ErrNotExist) {
			require.ErrorIs(t, testErr, os.ErrNotExist, "Real error is os.ErrNotExist, but test error is not")
		} else if errors.Is(realErr, os.ErrExist) {
			require.ErrorIs(t, testErr, os.ErrExist, "Real error is os.ErrExist, but test error is not")
		} else if errors.Is(realErr, syscall.ENOTEMPTY) {
			require.ErrorIs(t, testErr, syscall.ENOTEMPTY, "Real error is syscall.ENOTEMPTY, but test error is not")
		} else if errors.Is(realErr, os.ErrClosed) {
			require.ErrorIs(t, testErr, os.ErrClosed, "Real error is os.ErrClosed, but test error is not")
		}
		// For other errors (e.g., 'is a directory', 'directory not empty'),
		// the exact error message can be OS-dependent. In these cases, just
		// checking that *an* error occurred is sufficient.
	} else {
		require.NoError(t, testErr, "FS under test errored but Real FS did not.\nTest error: %v", testErr)
	}
}
