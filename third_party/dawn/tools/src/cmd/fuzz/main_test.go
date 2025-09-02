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

package main

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestGatherWgslFiles(t *testing.T) {
	tests := []struct {
		name      string
		inputsDir string
		outDir    string
		setupFS   func(t *testing.T, fs oswrapper.MemMapOSWrapper)
		wantErr   bool
		verify    func(t *testing.T, fs oswrapper.MemMapOSWrapper)
	}{
		{
			name:      "Basic copy",
			inputsDir: "/in",
			outDir:    "/out",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/in/subdir", 0777))
				require.NoError(t, fs.MkdirAll("/out", 0777))
				require.NoError(t, fs.WriteFile("/in/a.wgsl", []byte("shader a"), 0666))
				require.NoError(t, fs.WriteFile("/in/b.txt", []byte("not a shader"), 0666))
				require.NoError(t, fs.WriteFile("/in/c.expected.wgsl", []byte("should be ignored"), 0666))
				require.NoError(t, fs.WriteFile("/in/subdir/d.wgsl", []byte("shader d"), 0666))
			},
			wantErr: false,
			verify: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				// Check that expected files were created with correct content
				contentA, err := fs.ReadFile("/out/a.wgsl")
				require.NoError(t, err)
				require.Equal(t, "shader a", string(contentA))

				contentD, err := fs.ReadFile("/out/subdir_d.wgsl")
				require.NoError(t, err)
				require.Equal(t, "shader d", string(contentD))

				// Check that other files were not copied
				_, err = fs.Stat("/out/b.txt")
				require.Error(t, err, "b.txt should not have been copied")

				_, err = fs.Stat("/out/c.expected.wgsl")
				require.Error(t, err, "c.expected.wgsl should not have been copied")
			},
		},
		{
			name:      "Complex subdirectories",
			inputsDir: "/in",
			outDir:    "/out",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/in/a/b", 0777))
				require.NoError(t, fs.MkdirAll("/out", 0777))
				require.NoError(t, fs.WriteFile("/in/a/b/c.wgsl", []byte("shader c"), 0666))
			},
			wantErr: false,
			verify: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				content, err := fs.ReadFile("/out/a_b_c.wgsl")
				require.NoError(t, err)
				require.Equal(t, "shader c", string(content))
			},
		},
		{
			name:      "Empty input directory",
			inputsDir: "/in",
			outDir:    "/out",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/in", 0777))
				require.NoError(t, fs.MkdirAll("/out", 0777))
			},
			wantErr: false,
			verify: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				isEmpty, err := fileutils.IsEmptyDir("/out", fs)
				require.NoError(t, err)
				require.True(t, isEmpty)
			},
		},
		{
			name:      "Output directory does not exist",
			inputsDir: "/in",
			outDir:    "/out",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/in", 0777))
				require.NoError(t, fs.WriteFile("/in/a.wgsl", []byte("shader a"), 0666))
			},
			wantErr: false,
			verify: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				// Check that output directory was created
				info, err := fs.Stat("/out")
				require.NoError(t, err)
				require.True(t, info.IsDir())

				// Check that file was copied
				content, err := fs.ReadFile("/out/a.wgsl")
				require.NoError(t, err)
				require.Equal(t, "shader a", string(content))
			},
		},
		{
			name:      "Input directory does not exist",
			inputsDir: "/nonexistent",
			outDir:    "/out",
			setupFS: func(t *testing.T, fs oswrapper.MemMapOSWrapper) {
				require.NoError(t, fs.MkdirAll("/out", 0777))
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

			err := gatherWgslFiles(tc.inputsDir, tc.outDir, wrapper)

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
