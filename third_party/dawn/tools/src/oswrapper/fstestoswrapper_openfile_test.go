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

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

// Tests for the OpenFile() function in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_OpenFile(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		flag            int
		action          func(t *testing.T, file oswrapper.File) // Action to perform on the opened file
		expectedContent *string                                 // Expected content of the file *after* the action
		expectedError
	}{
		{
			name: "O_RDONLY - Read existing file",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "read me"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				content, err := io.ReadAll(file)
				require.NoError(t, err)
				require.Equal(t, "read me", string(content))
			},
		},
		{
			name: "O_RDONLY - Error on write",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "read only"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("fail"))
				require.Error(t, err)
			},
			expectedContent: stringPtr("read only"),
		},
		{
			name: "O_WRONLY - Write to existing file",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_WRONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "initial"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("newtial"),
		},
		{
			name: "O_WRONLY - Error on read",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_WRONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "write only"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := io.ReadAll(file)
				require.Error(t, err)
			},
			expectedContent: stringPtr("write only"),
		},
		{
			name: "O_RDWR - Read and Write",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "start"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				// This should not advance the write offset
				content, err := io.ReadAll(file)
				require.NoError(t, err)
				require.Equal(t, "start", string(content))

				// Seek to the beginning to overwrite
				_, err = file.Seek(0, io.SeekStart)
				require.NoError(t, err)

				_, err = file.Write([]byte("finish"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("finish"),
		},
		{
			name: "O_APPEND - Append to file",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_WRONLY | os.O_APPEND,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "first,"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("second"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("first,second"),
		},
		{
			name: "O_CREATE - Create new file",
			path: filepath.Join(root, "newfile.txt"),
			flag: os.O_WRONLY | os.O_CREATE,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("created"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("created"),
		},
		{
			name: "O_CREATE | O_EXCL - Fail if exists",
			path: filepath.Join(root, "existing.txt"),
			flag: os.O_WRONLY | os.O_CREATE | os.O_EXCL,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "existing.txt"): "..."},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name: "O_TRUNC - Truncate existing file",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_WRONLY | os.O_TRUNC,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "to be truncated"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new data"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("new data"),
		},
		{
			name: "Error - Open non-existent for reading",
			path: filepath.Join(root, "no.txt"),
			flag: os.O_RDONLY,
			expectedError: expectedError{
				wantErrIs: os.ErrNotExist,
			},
		},
		{
			name: "Error - Path is a directory",
			path: filepath.Join(root, "mydir"),
			flag: os.O_WRONLY,
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "mydir")},
			},
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
		},
		{
			name: "Error - Open symlink to directory for writing",
			path: filepath.Join(root, "link_to_dir"),
			flag: os.O_WRONLY,
			setup: unittestSetup{
				initialDirs: []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_dir"): "dir",
				},
			},
			expectedError: expectedError{
				wantErrMsg: "is a directory",
			},
		},
		{
			name: "Open symlink to file",
			path: filepath.Join(root, "link_to_file"),
			flag: os.O_RDONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_file"): "file.txt",
				},
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Write to symlink to file",
			path: filepath.Join(root, "link_to_file"),
			flag: os.O_WRONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "old"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_file"): "file.txt",
				},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("new"),
		},
		{
			name: "Open broken symlink",
			path: filepath.Join(root, "broken_link"),
			flag: os.O_RDONLY,
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
			name: "Create file via symlink",
			path: filepath.Join(root, "link_to_new"),
			flag: os.O_WRONLY | os.O_CREATE,
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "link_to_new"): "new.txt",
				},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("created"))
				require.NoError(t, err)
			},
			// We check the symlink path content (which should read the target)
			expectedContent: stringPtr("created"),
		},
		{
			name: "O_EXCL fail on existing symlink",
			path: filepath.Join(root, "link"),
			flag: os.O_WRONLY | os.O_CREATE | os.O_EXCL,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): ""},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): "file.txt",
				},
			},
			expectedError: expectedError{
				wantErrIs: os.ErrExist,
			},
		},
		{
			name: "Open symlink loop",
			path: filepath.Join(root, "loop1"),
			flag: os.O_RDONLY,
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
			name: "Symlink chain",
			path: filepath.Join(root, "link1"),
			flag: os.O_RDONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "target"): "content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link1"): "link2",
					filepath.Join(root, "link2"): "target",
				},
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Absolute symlink",
			path: filepath.Join(root, "abs_link"),
			flag: os.O_RDONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "target"): "content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "abs_link"): filepath.Join(root, "target"),
				},
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Relative symlink with parent ref",
			path: filepath.Join(root, "dir", "link"),
			flag: os.O_RDONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "target"): "content"},
				initialDirs:  []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "dir", "link"): "../target",
				},
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Symlink in subdirectory",
			path: filepath.Join(root, "dir", "link"),
			flag: os.O_RDONLY,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "dir", "target"): "content"},
				initialDirs:  []string{filepath.Join(root, "dir")},
				initialSymlinks: map[string]string{
					filepath.Join(root, "dir", "link"): "target",
				},
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Path with multiple symlinks",
			path: filepath.Join(root, "link1", "link2", "file.txt"),
			flag: os.O_RDONLY,
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
		{
			name: "Create file via broken symlink chain",
			path: filepath.Join(root, "link1"),
			flag: os.O_WRONLY | os.O_CREATE,
			setup: unittestSetup{
				initialSymlinks: map[string]string{
					filepath.Join(root, "link1"): "link2",
					filepath.Join(root, "link2"): "target.txt",
				},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("created"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("created"),
		},
		{
			name: "Truncate via symlink",
			path: filepath.Join(root, "link"),
			flag: os.O_WRONLY | os.O_TRUNC,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "old content"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): "file.txt",
				},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("new"),
		},
		{
			name: "Append via symlink",
			path: filepath.Join(root, "link"),
			flag: os.O_WRONLY | os.O_APPEND,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "old"},
				initialSymlinks: map[string]string{
					filepath.Join(root, "link"): "file.txt",
				},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new"))
				require.NoError(t, err)
			},
			expectedContent: stringPtr("oldnew"),
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := tc.setup.setup(t)
			file, err := wrapper.OpenFile(tc.path, tc.flag, 0666)

			if tc.expectedError.Check(t, err) {
				return
			}

			require.NotNil(t, file)
			defer file.Close()
			if tc.action != nil {
				tc.action(t, file)
			}

			// Close the file to ensure contents are flushed.
			require.NoError(t, file.Close())

			if tc.expectedContent != nil {
				content, err := wrapper.ReadFile(tc.path)
				require.NoError(t, err)
				require.Equal(t, *tc.expectedContent, string(content))
			}
		})
	}
}

func TestFSTestOSWrapper_OpenFile_MatchesReal(t *testing.T) {
	tests := []struct {
		name   string
		setup  matchesRealSetup
		path   string
		flag   int
		action func(t *testing.T, file oswrapper.File)
	}{
		{
			name: "O_RDONLY - Read existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "read me",
				},
			}},
			path: "file.txt",
			flag: os.O_RDONLY,
		},
		{
			name: "O_RDONLY - Error on write",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "read only",
				},
			}},
			path: "file.txt",
			flag: os.O_RDONLY,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("fail"))
				require.Error(t, err)
			},
		},
		{
			name: "O_WRONLY - Write to existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "initial",
				},
			}},
			path: "file.txt",
			flag: os.O_WRONLY,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new"))
				require.NoError(t, err)
			},
		},
		{
			name: "O_WRONLY - Error on read",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "write only",
				},
			}},
			path: "file.txt",
			flag: os.O_WRONLY,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := io.ReadAll(file)
				require.Error(t, err)
			},
		},
		{
			name: "O_RDWR - Read and Write",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "start",
				},
			}},
			path: "file.txt",
			flag: os.O_RDWR,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("finish"))
				require.NoError(t, err)
			},
		},
		{
			name: "O_APPEND - Append to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "first,",
				},
			}},
			path: "file.txt",
			flag: os.O_WRONLY | os.O_APPEND,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("second"))
				require.NoError(t, err)
			},
		},
		{
			name: "O_CREATE - Create new file",
			path: "newfile.txt",
			flag: os.O_WRONLY | os.O_CREATE,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("created"))
				require.NoError(t, err)
			},
		},
		{
			name: "O_CREATE | O_EXCL - Fail if exists",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"existing.txt": "...",
				},
			}},
			path: "existing.txt",
			flag: os.O_WRONLY | os.O_CREATE | os.O_EXCL,
		},
		{
			name: "O_TRUNC - Truncate existing file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{
					"file.txt": "to be truncated",
				},
			}},
			path: "file.txt",
			flag: os.O_WRONLY | os.O_TRUNC,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new data"))
				require.NoError(t, err)
			},
		},
		{
			name: "Error - Open non-existent for reading",
			path: "no.txt",
			flag: os.O_RDONLY,
		},
		{
			name: "Error - Path is a directory",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"mydir"},
			}},
			path: "mydir",
			flag: os.O_WRONLY,
		},
		{
			name: "Error - Open symlink to directory for writing",
			setup: matchesRealSetup{unittestSetup{
				initialDirs: []string{"dir"},
				initialSymlinks: map[string]string{
					"link_to_dir": "dir",
				},
			}},
			path: "link_to_dir",
			flag: os.O_WRONLY,
		},
		{
			name: "Open symlink to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "content"},
				initialSymlinks: map[string]string{
					"link_to_file": "file.txt",
				},
			}},
			path: "link_to_file",
			flag: os.O_RDONLY,
		},
		{
			name: "Write to symlink to file",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "old"},
				initialSymlinks: map[string]string{
					"link_to_file": "file.txt",
				},
			}},
			path: "link_to_file",
			flag: os.O_WRONLY,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new"))
				require.NoError(t, err)
			},
		},
		{
			name: "Open broken symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"broken_link": "nonexistent",
				},
			}},
			path: "broken_link",
			flag: os.O_RDONLY,
		},
		{
			name: "Create file via symlink",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"link_to_new": "new.txt",
				},
			}},
			path: "link_to_new",
			flag: os.O_WRONLY | os.O_CREATE,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("created"))
				require.NoError(t, err)
			},
		},
		{
			name: "O_EXCL fail on existing symlink",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": ""},
				initialSymlinks: map[string]string{
					"link": "file.txt",
				},
			}},
			path: "link",
			flag: os.O_WRONLY | os.O_CREATE | os.O_EXCL,
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
			flag: os.O_RDONLY,
		},
		{
			name: "Symlink chain",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"target": "content"},
				initialSymlinks: map[string]string{
					"link1": "link2",
					"link2": "target",
				},
			}},
			path: "link1",
			flag: os.O_RDONLY,
		},
		{
			name: "Relative symlink with parent ref",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"target": "content"},
				initialDirs:  []string{"dir"},
				initialSymlinks: map[string]string{
					filepath.Join("dir", "link"): "../target",
				},
			}},
			path: filepath.Join("dir", "link"),
			flag: os.O_RDONLY,
		},
		{
			name: "Symlink in subdirectory",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{filepath.Join("dir", "target"): "content"},
				initialDirs:  []string{"dir"},
				initialSymlinks: map[string]string{
					filepath.Join("dir", "link"): "target",
				},
			}},
			path: filepath.Join("dir", "link"),
			flag: os.O_RDONLY,
		},
		{
			name: "Path with multiple symlinks",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{filepath.Join("real_dir1", "real_dir2", "file.txt"): "content"},
				initialDirs:  []string{filepath.Join("real_dir1", "real_dir2")},
				initialSymlinks: map[string]string{
					"link1":                             "real_dir1",
					filepath.Join("real_dir1", "link2"): "real_dir2",
				},
			}},
			path: filepath.Join("link1", "link2", "file.txt"),
			flag: os.O_RDONLY,
		},
		{
			name: "Create file via broken symlink chain",
			setup: matchesRealSetup{unittestSetup{
				initialSymlinks: map[string]string{
					"link1": "link2",
					"link2": "target.txt",
				},
			}},
			path: "link1",
			flag: os.O_WRONLY | os.O_CREATE,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("created"))
				require.NoError(t, err)
			},
		},
		{
			name: "Truncate via symlink",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "old content"},
				initialSymlinks: map[string]string{
					"link": "file.txt",
				},
			}},
			path: "link",
			flag: os.O_WRONLY | os.O_TRUNC,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new"))
				require.NoError(t, err)
			},
		},
		{
			name: "Append via symlink",
			setup: matchesRealSetup{unittestSetup{
				initialFiles: map[string]string{"file.txt": "old"},
				initialSymlinks: map[string]string{
					"link": "file.txt",
				},
			}},
			path: "link",
			flag: os.O_WRONLY | os.O_APPEND,
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("new"))
				require.NoError(t, err)
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := tc.setup.setup(t)
			defer os.RemoveAll(realRoot)

			// Execute on Real FS
			realFile, realErr := realFS.OpenFile(filepath.Join(realRoot, tc.path), tc.flag, 0666)
			if realErr == nil {
				defer realFile.Close()
				if tc.action != nil {
					tc.action(t, realFile)
				}
				require.NoError(t, realFile.Close())
			}

			// Execute on Test FS
			testFile, testErr := testFS.OpenFile(tc.path, tc.flag, 0666)
			if testErr == nil {
				defer testFile.Close()
				if tc.action != nil {
					tc.action(t, testFile)
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
