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
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

// Tests for behavior on closed files in FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestFSTestOSWrapper_ClosedFile(t *testing.T) {
	root := getTestRoot()
	tests := []struct {
		name            string
		setup           unittestSetup
		path            string
		flag            int
		action          func(t *testing.T, file oswrapper.File) // Action to perform on the closed file
		expectedContent *string                                 // Expected content of the file *after* the action
		expectedError
	}{
		{
			name: "Read on closed file fails",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Read(make([]byte, 1))
				require.Error(t, err)
				require.ErrorIs(t, err, os.ErrClosed)
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Write on closed file fails",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Write([]byte("fail"))
				require.Error(t, err)
				require.ErrorIs(t, err, os.ErrClosed)
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Seek on closed file fails",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Seek(0, io.SeekStart)
				require.Error(t, err)
				require.ErrorIs(t, err, os.ErrClosed)
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Stat on closed file fails",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				_, err := file.Stat()
				require.Error(t, err)
				require.ErrorIs(t, err, os.ErrClosed)
			},
			expectedContent: stringPtr("content"),
		},
		{
			name: "Closed on closed file fails",
			path: filepath.Join(root, "file.txt"),
			flag: os.O_RDWR,
			setup: unittestSetup{
				initialFiles: map[string]string{filepath.Join(root, "file.txt"): "content"},
			},
			action: func(t *testing.T, file oswrapper.File) {
				err := file.Close()
				require.Error(t, err)
				require.ErrorIs(t, err, os.ErrClosed)
			},
			expectedContent: stringPtr("content"),
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

			require.NoError(t, file.Close())
			if tc.action != nil {
				tc.action(t, file)
			}

			if tc.expectedContent != nil {
				content, err := wrapper.ReadFile(tc.path)
				require.NoError(t, err)
				require.Equal(t, *tc.expectedContent, string(content))
			}
		})
	}
}

func TestFSTestOSWrapper_ClosedFile_MatchesReal(t *testing.T) {
	setup := matchesRealSetup{unittestSetup{
		initialFiles: map[string]string{
			"file.txt": "content",
		},
	}}
	path := "file.txt"
	flag := os.O_RDWR

	tests := []struct {
		name      string
		operation func(t *testing.T, realFile, testFile oswrapper.File)
	}{
		{
			name: "Read",
			operation: func(t *testing.T, realFile, testFile oswrapper.File) {
				_, realErr := realFile.Read(make([]byte, 1))
				_, testErr := testFile.Read(make([]byte, 1))
				requireErrorsMatch(t, realErr, testErr)
			},
		},
		{
			name: "Write",
			operation: func(t *testing.T, realFile, testFile oswrapper.File) {
				_, realErr := realFile.Write([]byte("fail"))
				_, testErr := testFile.Write([]byte("fail"))
				requireErrorsMatch(t, realErr, testErr)
			},
		},
		{
			name: "Seek",
			operation: func(t *testing.T, realFile, testFile oswrapper.File) {
				_, realErr := realFile.Seek(0, io.SeekStart)
				_, testErr := testFile.Seek(0, io.SeekStart)
				requireErrorsMatch(t, realErr, testErr)
			},
		},
		{
			name: "Stat",
			operation: func(t *testing.T, realFile, testFile oswrapper.File) {
				_, realErr := realFile.Stat()
				_, testErr := testFile.Stat()
				requireErrorsMatch(t, realErr, testErr)
			},
		},
		{
			name: "Close",
			operation: func(t *testing.T, realFile, testFile oswrapper.File) {
				realErr := realFile.Close()
				testErr := testFile.Close()
				requireErrorsMatch(t, realErr, testErr)
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			realRoot, realFS, testFS := setup.setup(t)
			defer os.RemoveAll(realRoot)

			realFile, realOpenErr := realFS.OpenFile(filepath.Join(realRoot, path), flag, 0666)
			require.NoError(t, realOpenErr)
			require.NoError(t, realFile.Close())

			testFile, testOpenErr := testFS.OpenFile(path, flag, 0666)
			require.NoError(t, testOpenErr)
			require.NoError(t, testFile.Close())

			tc.operation(t, realFile, testFile)
			requireFileSystemsMatch(t, realRoot, testFS)
		})
	}
}
