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

// Tests for the Create() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_Create(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		contentToWrite  []byte
		expectedContent *string
		expectedError
		verify func(t *testing.T, wrapper oswrapper.FSTestOSWrapper)
	}{
		{
			name:            "Create new file and write",
			path:            filepath.Join(root, "newfile.txt"),
			contentToWrite:  []byte("hello"),
			expectedContent: stringPtr("hello"),
		},
		{
			name: "Truncate existing file",
			path: filepath.Join(root, "existing.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "existing.txt"): "old content"},
			},
			contentToWrite:  []byte("new"),
			expectedContent: stringPtr("new"),
		},
		{
			name: "Create file in existing subdirectory",
			path: filepath.Join(root, "foo", "bar.txt"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "foo")},
			},
			contentToWrite:  []byte("sub content"),
			expectedContent: stringPtr("sub content"),
		},
		{
			name: "Create file in non-existent directory",
			path: filepath.Join(root, "new", "dir", "file.txt"),
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Path is a directory",
			path: filepath.Join(root, "mydir"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
		},
		{
			name: "Parent path is a file",
			path: filepath.Join(root, "file.txt", "another.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "i am a file"},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Create via symlink to directory",
			path: filepath.Join(root, "link", "file.txt"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): filepath.Join(root, "dir"),
				},
			},
			contentToWrite:  []byte("content"),
			expectedContent: stringPtr("content"),
		},
		{
			name: "Create via symlink chain to directory",
			path: filepath.Join(root, "link1", "file.txt"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link2"): filepath.Join(root, "dir"),
					filepath.Join(root, "link1"): filepath.Join(root, "link2"),
				},
			},
			contentToWrite:  []byte("chain"),
			expectedContent: stringPtr("chain"),
		},
		{
			name: "Create overwrites symlink target",
			path: filepath.Join(root, "link"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "target.txt"): "old"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): filepath.Join(root, "target.txt"),
				},
			},
			contentToWrite: []byte("new"),
			verify: func(t *testing.T, wrapper oswrapper.FSTestOSWrapper) {
				targetPath := filepath.Join(root, "target.txt")
				content, err := wrapper.ReadFile(targetPath)
				require.NoError(t, err)
				require.Equal(t, "new", string(content))
			},
		},
		{
			name: "Create on symlink to non-existent file",
			path: filepath.Join(root, "link"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): filepath.Join(root, "target.txt"),
				},
			},
			contentToWrite: []byte("hello"),
			verify: func(t *testing.T, wrapper oswrapper.FSTestOSWrapper) {
				targetPath := filepath.Join(root, "target.txt")
				content, err := wrapper.ReadFile(targetPath)
				require.NoError(t, err)
				require.Equal(t, "hello", string(content))
			},
		},
		{
			name: "Create fails if path is symlink to directory",
			path: filepath.Join(root, "link"),
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): filepath.Join(root, "dir"),
				},
			},
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
		},
		{
			name: "Create fails if parent component is symlink to file",
			path: filepath.Join(root, "link", "sub.txt"),
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): filepath.Join(root, "file.txt"),
				},
			},
			expectedError: expectedError{
				wantErrMsg: "not a directory",
			},
		},
		{
			name: "Create fails with broken symlink in path",
			path: filepath.Join(root, "link", "file.txt"),
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): filepath.Join(root, "target"),
				},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Create fails with symlink loop",
			path: filepath.Join(root, "loop1", "file.txt"),
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
			file, err := wrapper.Create(tc.path)

			if tc.expectedError.Check(t, err) {
				return
			}

			// For success cases:
			require.NotNil(t, file)
			defer file.Close()

			if len(tc.contentToWrite) > 0 {
				n, err := file.Write(tc.contentToWrite)
				require.NoError(t, err)
				require.Equal(t, len(tc.contentToWrite), n)
			}

			// Close the file to ensure contents are flushed to the in-memory map.
			require.NoError(t, file.Close())

			if tc.expectedContent != nil {
				// To verify content, we must resolve the path since we might have written through a symlink
				// and ReadFile follows symlinks too.
				// But specifically for "Create overwrites symlink target", we might want to verify the target explicitly.
				// ReadFile does handle symlinks, so reading tc.path should work.

				content, err := wrapper.ReadFile(tc.path)
				require.NoError(t, err)
				require.Equal(t, *tc.expectedContent, string(content))

				// If it was a symlink test, verify the target directly too if possible.
				// The wrapper doesn't expose ReadLink easily without peeking into FS, but ReadFile is enough.
			}

			if tc.verify != nil {
				tc.verify(t, wrapper)
			}
		})
	}
}

// TestFSTestOSWrapper_Create_MatchesReal tests that the behavior of FSTestOSWrapper.Create
// matches the behavior of the real os.Create function.
func TestFSTestOSWrapper_Create_MatchesReal(t *testing.T) {
	tests := []struct {
		name           string
		setup          matchesRealSetup
		path           string // path to Create
		contentToWrite []byte
	}{
		{
			name:           "Create new file",
			path:           "newfile.txt",
			contentToWrite: []byte("hello"),
		},
		{
			name: "Truncate existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"existing.txt": "old content",
				},
			}},
			path:           "existing.txt",
			contentToWrite: []byte("new"),
		},
		{
			name: "Create file in existing subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{
					"foo",
				},
			}},
			path:           filepath.Join("foo", "bar.txt"),
			contentToWrite: []byte("sub content"),
		},
		{
			name: "Error on non-existent directory",
			path: filepath.Join("new", "dir", "file.txt"),
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
			name: "Error on parent path is a file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "i am a file",
				},
			}},
			path: filepath.Join("file.txt", "another.txt"),
		},
		{
			name: "Create via symlink to directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link": "dir",
				},
			}},
			path:           filepath.Join("link", "file.txt"),
			contentToWrite: []byte("content"),
		},
		{
			name: "Create via symlink chain to directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link2": "dir",
					"link1": "link2",
				},
			}},
			path:           filepath.Join("link1", "file.txt"),
			contentToWrite: []byte("chain"),
		},
		{
			name: "Create overwrites symlink target",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"target.txt": "old",
				},
				initialSymlinks: map[string]string{
					"link": "target.txt",
				},
			}},
			path:           "link",
			contentToWrite: []byte("new"),
		},
		{
			name: "Create on symlink to non-existent file",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"link": "target.txt",
				},
			}},
			path:           "link",
			contentToWrite: []byte("hello"),
		},
		{
			name: "Error if path is symlink to directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link": "dir",
				},
			}},
			path: "link",
		},
		{
			name: "Error if parent component is symlink to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "content",
				},
				initialSymlinks: map[string]string{
					"link": "file.txt",
				},
			}},
			path: filepath.Join("link", "sub.txt"),
		},
		{
			name: "Error with broken symlink in path",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"link": "target",
				},
			}},
			path: filepath.Join("link", "file.txt"),
		},
		{
			name: "Error with symlink loop",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"loop1": "loop2",
					"loop2": "loop1",
				},
			}},
			path: filepath.Join("loop1", "file.txt"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			realPath := filepath.Join(realRoot, tc.path)
			realFile, realErr := realFS.Create(realPath)
			if realErr == nil {
				if len(tc.contentToWrite) > 0 {
					_, err := realFile.Write(tc.contentToWrite)
					require.NoError(t, err)
				}
				require.NoError(t, realFile.Close())
			}

			testFile, testErr := testFS.Create(tc.path)
			if testErr == nil {
				if len(tc.contentToWrite) > 0 {
					_, err := testFile.Write(tc.contentToWrite)
					require.NoError(t, err)
				}
				require.NoError(t, testFile.Close())
			}

			requireErrorsMatch(t, realErr, testErr)
			if realErr == nil {
				requireFileSystemsMatch(t, realRoot, testFS)
			}
		})
	}
}
