// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
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

package fileutils_test

import (
	"path/filepath"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestCopyFile(t *testing.T) {
	tests := []struct {
		name    string
		srcPath string
		dstPath string
		setupFS func(t *testing.T, fs oswrapper.MemMapOSWrapper)
		wantErr bool
		verify  func(t *testing.T, fs oswrapper.MemMapOSWrapper)
	}{
		{
			name:    "Simple copy",
			srcPath: "/src/file.txt",
			dstPath: "/dst/file.txt",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/src", 0777))
				require.NoError(t, fs.WriteFile("/src/file.txt", []byte("hello world"), 0666))
				require.NoError(t, fs.MkdirAll("/dst", 0777))
			},
			wantErr: false,
			verify: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				content, err := fs.ReadFile("/dst/file.txt")
				require.NoError(t, err)
				require.Equal(t, "hello world", string(content))
			},
		},
		{
			name:    "Overwrite existing file",
			srcPath: "/src/file.txt",
			dstPath: "/dst/file.txt",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/src", 0777))
				require.NoError(t, fs.WriteFile("/src/file.txt", []byte("new content"), 0666))
				require.NoError(t, fs.MkdirAll("/dst", 0777))
				require.NoError(t, fs.WriteFile("/dst/file.txt", []byte("old content"), 0666))
			},
			wantErr: false,
			verify: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				content, err := fs.ReadFile("/dst/file.txt")
				require.NoError(t, err)
				require.Equal(t, "new content", string(content))
			},
		},
		{
			name:    "Source does not exist",
			srcPath: "/src/nonexistent.txt",
			dstPath: "/dst/file.txt",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/dst", 0777))
			},
			wantErr: true,
		},
		{
			name:    "Source is a directory",
			srcPath: "/src/dir",
			dstPath: "/dst/file.txt",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/src/dir", 0777))
				require.NoError(t, fs.MkdirAll("/dst", 0777))
			},
			wantErr: true,
		},
		{
			name:    "Destination directory does not exist",
			srcPath: "/src/file.txt",
			dstPath: "/nonexistent/dst/file.txt",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/src", 0777))
				require.NoError(t, fs.WriteFile("/src/file.txt", []byte("hello"), 0666))
			},
			wantErr: false, // CopyFile creates the destination directory
			verify: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				content, err := fs.ReadFile("/nonexistent/dst/file.txt")
				require.NoError(t, err)
				require.Equal(t, "hello", string(content))
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			if tc.setupFS != nil {
				tc.setupFS(t, wrapper)
			}

			err := fileutils.CopyFile(tc.dstPath, tc.srcPath, wrapper)

			if tc.wantErr {
				require.Error(t, err)
			} else {
				require.NoError(t, err)
			}

			if tc.verify != nil {
				tc.verify(t, wrapper)
			}
		})
	}
}

func TestCopyDir(t *testing.T) {
	tests := []struct {
		name    string
		srcPath string
		dstPath string
		setupFS func(t *testing.T, fs oswrapper.MemMapOSWrapper)
		wantErr bool
		verify  func(t *testing.T, fs oswrapper.MemMapOSWrapper)
	}{
		{
			name:    "Copy to non-existent destination",
			srcPath: "/src/data",
			dstPath: "/dst",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/src/data/subdir", 0777))
				require.NoError(t, fs.WriteFile("/src/data/file1.txt", []byte("file1"), 0666))
			},
			wantErr: false,
			verify: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.True(t, fileutils.IsDir("/dst", fs))
				content, err := fs.ReadFile(filepath.Join("/dst", "file1.txt"))
				require.NoError(t, err)
				require.Equal(t, "file1", string(content))
				require.True(t, fileutils.IsDir(filepath.Join("/dst", "subdir"), fs))
			},
		},
		{
			name:    "Overwrite existing destination",
			srcPath: "/src/new_data",
			dstPath: "/dst",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				// Source
				require.NoError(t, fs.MkdirAll("/src/new_data", 0777))
				require.NoError(t, fs.WriteFile("/src/new_data/new_file.txt", []byte("new"), 0666))
				// Destination with old content
				require.NoError(t, fs.MkdirAll("/dst/old_subdir", 0777))
				require.NoError(t, fs.WriteFile("/dst/old_file.txt", []byte("old"), 0666))
			},
			wantErr: false,
			verify: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				// Check new file exists
				content, err := fs.ReadFile(filepath.Join("/dst", "new_file.txt"))
				require.NoError(t, err)
				require.Equal(t, "new", string(content))

				// Check old files are gone
				require.False(t, fileutils.IsFile(filepath.Join("/dst", "old_file.txt"), fs))
				require.False(t, fileutils.IsDir(filepath.Join("/dst", "old_subdir"), fs))
			},
		},
		{
			name:    "Copy complex directory structure",
			srcPath: "/src/complex",
			dstPath: "/dst",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				// Source with complex structure
				require.NoError(t, fs.MkdirAll("/src/complex/a", 0777))
				require.NoError(t, fs.MkdirAll("/src/complex/b/c", 0777))
				require.NoError(t, fs.WriteFile("/src/complex/root.txt", []byte("root"), 0666))
				require.NoError(t, fs.WriteFile("/src/complex/a/a.txt", []byte("a"), 0666))
				require.NoError(t, fs.WriteFile("/src/complex/b/b.txt", []byte("b"), 0666))
				require.NoError(t, fs.WriteFile("/src/complex/b/c/c.txt", []byte("c"), 0666))
				// Empty destination
				require.NoError(t, fs.MkdirAll("/dst", 0777))
			},
			wantErr: false,
			verify: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				content, err := fs.ReadFile(filepath.Join("/dst", "root.txt"))
				require.NoError(t, err)
				require.Equal(t, "root", string(content))
				content, err = fs.ReadFile(filepath.Join("/dst", "a", "a.txt"))
				require.NoError(t, err)
				require.Equal(t, "a", string(content))
				content, err = fs.ReadFile(filepath.Join("/dst", "b", "b.txt"))
				require.NoError(t, err)
				require.Equal(t, "b", string(content))
				content, err = fs.ReadFile(filepath.Join("/dst", "b", "c", "c.txt"))
				require.NoError(t, err)
				require.Equal(t, "c", string(content))
			},
		},
		{
			name:    "Copy empty directory to existing destination",
			srcPath: "/src/empty",
			dstPath: "/dst",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/src/empty", 0777))
				require.NoError(t, fs.MkdirAll("/dst", 0777))
				require.NoError(t, fs.WriteFile("/dst/old_file.txt", []byte("old"), 0666))
			},
			wantErr: false,
			verify: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				isEmpty, err := fileutils.IsEmptyDir("/dst", fs)
				require.NoError(t, err)
				require.True(t, isEmpty)
			},
		},
		{
			name:    "Source does not exist",
			srcPath: "/src/nonexistent",
			dstPath: "/dst",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/dst", 0777))
			},
			wantErr: true,
		},
		{
			name:    "Source is a file",
			srcPath: "/src/file.txt",
			dstPath: "/dst",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.WriteFile("/src/file.txt", []byte("i am a file"), 0666))
				require.NoError(t, fs.MkdirAll("/dst", 0777))
			},
			wantErr: true,
		},
		{
			name:    "Destination is a file",
			srcPath: "/src/data",
			dstPath: "/dst_is_a_file",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/src/data", 0777))
				require.NoError(t, fs.WriteFile("/dst_is_a_file", []byte("i am a file"), 0666))
			},
			wantErr: true,
		},
		{
			name:    "Destination is a subdirectory of source",
			srcPath: "/src",
			dstPath: "/src/sub",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/src/sub", 0777))
			},
			wantErr: true,
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			if tc.setupFS != nil {
				tc.setupFS(t, wrapper)
			}

			err := fileutils.CopyDir(tc.dstPath, tc.srcPath, wrapper)

			require.Equal(t, tc.wantErr, err != nil, "CopyDir() error = %v, wantErr %v", err, tc.wantErr)

			if tc.verify != nil {
				tc.verify(t, wrapper)
			}
		})
	}
}
