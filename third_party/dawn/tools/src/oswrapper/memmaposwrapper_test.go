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

package oswrapper

import (
	"io"
	"io/fs"
	"os"
	"regexp"
	"testing"
	"time"

	"github.com/stretchr/testify/require"
)

func TestUsableAsOsWrapper(t *testing.T) {
	f := func(osWrapper OSWrapper) {
		_ = osWrapper.Environ()
	}

	wrapper := CreateMemMapOSWrapper()
	f(wrapper)
}

func TestEnviron(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	environ := wrapper.Environ()
	require.Equal(t, []string{}, environ)

	wrapper.MemMapEnvironProvider.Environment = map[string]string{
		"HOME": "/tmp",
		"PWD":  "/local",
	}
	environ = wrapper.Environ()
	require.Equal(t, []string{"HOME=/tmp", "PWD=/local"}, environ)
}

func TestGetenv(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	require.Equal(t, "", wrapper.Getenv("HOME"))

	wrapper.MemMapEnvironProvider.Environment = map[string]string{
		"HOME": "/tmp",
	}
	require.Equal(t, "/tmp", wrapper.Getenv("HOME"))
}

func TestGetwd(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	wd, err := wrapper.Getwd()
	require.NoErrorf(t, err, "Failed to get wd: %v", err)
	require.Equal(t, "/", wd)

	wrapper.MemMapEnvironProvider.Environment = map[string]string{
		"PWD": "/local",
	}
	wd, err = wrapper.Getwd()
	require.NoErrorf(t, err, "Failed to get wd: %v", err)
	require.Equal(t, "/local", wd)
}

func TestUserHomeDir(t *testing.T) {
	tests := []struct {
		name        string
		environment map[string]string
		want        string
	}{
		{
			name:        "Default",
			environment: map[string]string{},
			want:        "/",
		},
		{
			name: "POSIX",
			environment: map[string]string{
				"HOME":        "/posix",
				"USERPROFILE": "/windows",
			},
			want: "/posix",
		},
		{
			name: "Windows",
			environment: map[string]string{
				"USERPROFILE": "/windows",
			},
			want: "/windows",
		},
	}
	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper := CreateMemMapOSWrapper()
			wrapper.MemMapEnvironProvider.Environment = testCase.environment
			home, err := wrapper.UserHomeDir()
			require.NoErrorf(t, err, "Failed to get home dir: %v", err)
			require.Equal(t, testCase.want, home)
		})
	}
}

func TestOpen_Nonexistent(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	_, err := wrapper.Open("/foo.txt")
	require.ErrorContains(t, err, "open /foo.txt: file does not exist")
}

func TestOpenFile_Nonexistent(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	_, err := wrapper.OpenFile("/foo.txt", os.O_RDONLY, 0o700)
	require.ErrorContains(t, err, "open /foo.txt: file does not exist")
}

func TestReadFile_Nonexistent(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	_, err := wrapper.ReadFile("/foo.txt")
	require.ErrorContains(t, err, "open /foo.txt: file does not exist")
}

func TestStat_Nonexistent(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	_, err := wrapper.Stat("/foo.txt")
	require.ErrorContains(t, err, "open /foo.txt: file does not exist")
}

func TestWalk_Nonexistent(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()

	walkfunc := func(root string, info fs.FileInfo, err error) error {
		require.Equal(t, "/nonexistent", root)
		require.Equal(t, nil, info)
		require.ErrorContains(t, err, "open /nonexistent: file does not exist")
		return err
	}

	err := wrapper.Walk("/nonexistent", walkfunc)
	require.ErrorContains(t, err, "open /nonexistent: file does not exist")
}

func TestWriteFile_Open(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	err := wrapper.WriteFile("/foo.txt", []byte("content"), 0o700)
	require.NoErrorf(t, err, "Failed to write file: %v", err)

	file, err := wrapper.Open("/foo.txt")
	require.NoErrorf(t, err, "Failed to open file: %v", err)
	buffer := make([]byte, 7)
	bytesRead, err := file.Read(buffer)
	require.NoErrorf(t, err, "Failed to read from file: %v", err)
	require.Equal(t, []byte("content"), buffer)
	require.Equal(t, 7, bytesRead)
	bytesRead, err = file.Read(buffer)
	require.Error(t, io.EOF, "Did not get EOF")
	require.Equal(t, 0, bytesRead)
}

func TestCreate_Walk_NonexistentDirectory(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	_, err := wrapper.Create("/parent/foo.txt")
	require.NoErrorf(t, err, "Got error creating file without parent dir: %v", err)

	type input struct {
		root string
		info fs.FileInfo
		err  error
	}
	inputs := []input{}

	walkfunc := func(root string, info fs.FileInfo, err error) error {
		inputs = append(inputs, input{root: root, info: info, err: err})
		return err
	}

	err = wrapper.Walk("/parent", walkfunc)
	require.NoErrorf(t, err, "Got error walking: %v", err)
	require.Equal(t, 2, len(inputs))
	require.Equal(t, "/parent", inputs[0].root)
	require.True(t, inputs[0].info.IsDir())
	require.Equal(t, "/parent/foo.txt", inputs[1].root)
	require.False(t, inputs[1].info.IsDir())
}

func TestCreate_ReadFile(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	file, err := wrapper.Create("/foo.txt")
	require.NoErrorf(t, err, "Failed to create file: %v", err)

	bytesWritten, err := file.Write([]byte("asdf"))
	require.NoErrorf(t, err, "Failed to write to file: %v", err)
	require.Equal(t, 4, bytesWritten)

	contents, err := wrapper.ReadFile("/foo.txt")
	require.NoErrorf(t, err, "Failed to read file: %v", err)
	require.Equal(t, []byte("asdf"), contents)
}

func TestMkdir_Exists(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	err := wrapper.Mkdir("/parent", 0o700)
	require.NoErrorf(t, err, "Got error creating directory: %v", err)
	err = wrapper.Mkdir("/parent", 0o700)
	require.ErrorContains(t, err, "mkdir /parent: file already exists")
}

func TestMkdir_MkdirAll(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	err := wrapper.Mkdir("/parent", 0o700)
	require.NoErrorf(t, err, "Error creating parent: %v", err)

	err = wrapper.MkdirAll("/parent/child/grandchild", 0o600)
	require.NoErrorf(t, err, "Error creating grandchild: %v", err)

	type input struct {
		root string
		info fs.FileInfo
		err  error
	}
	inputs := []input{}

	walkfunc := func(root string, info fs.FileInfo, err error) error {
		inputs = append(inputs, input{root: root, info: info, err: err})
		return err
	}

	err = wrapper.Walk("/parent", walkfunc)
	require.NoErrorf(t, err, "Got error walking: %v", err)
	require.Equal(t, len(inputs), 3)

	require.Equal(t, "/parent", inputs[0].root)
	require.True(t, inputs[0].info.IsDir())
	require.Equal(t, os.FileMode(0o700)|fs.ModeDir, inputs[0].info.Mode())

	require.Equal(t, "/parent/child", inputs[1].root)
	require.True(t, inputs[1].info.IsDir())
	require.Equal(t, os.FileMode(0o600)|fs.ModeDir, inputs[1].info.Mode())

	require.Equal(t, "/parent/child/grandchild", inputs[2].root)
	require.True(t, inputs[2].info.IsDir())
	require.Equal(t, os.FileMode(0o600)|fs.ModeDir, inputs[2].info.Mode())
}

func TestMkdirAll_Exists(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	err := wrapper.MkdirAll("/parent", 0o700)
	require.NoErrorf(t, err, "Got error creating directory: %v", err)
	err = wrapper.MkdirAll("/parent", 0o700)
	require.NoErrorf(t, err, "Got error creating directory second time: %v", err)
}

func TestMkdirTemp(t *testing.T) {
	tests := []struct {
		name    string
		pattern string
		re      string
	}{
		{
			name:    "Default behavior",
			pattern: "tempdir",
			re:      "/tmp/tempdir\\d+",
		},
		{
			name:    "Single star front",
			pattern: "*tempdir",
			re:      "/tmp/\\d+tempdir",
		},
		{
			name:    "Single star middle",
			pattern: "temp*dir",
			re:      "/tmp/temp\\d+dir",
		},
		{
			name:    "Multiple stars",
			pattern: "temp*dir*",
			re:      "/tmp/temp\\*dir\\d+",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper := CreateMemMapOSWrapper()
			dir, err := wrapper.MkdirTemp("/tmp", testCase.pattern)
			require.NoErrorf(t, err, "Error creating temporary directory: %v", err)

			re, err := regexp.Compile(testCase.re)
			require.NoErrorf(t, err, "Error compiling regexp: %s", err)
			require.True(t, re.Match([]byte(dir)))
		})
	}
}

func TestRemove(t *testing.T) {
	tests := []struct {
		name          string
		filesToCreate []string
		dirsToCreate  []string
		toRemove      string
		wantErr       bool
		wantErrMsg    string
	}{
		{
			name:       "Non-existent",
			toRemove:   "/foo.txt",
			wantErr:    true,
			wantErrMsg: "remove /foo.txt: file does not exist",
		},
		{
			name:          "File",
			filesToCreate: []string{"/foo.txt"},
			toRemove:      "/foo.txt",
		},
		{
			name:         "Empty dir",
			dirsToCreate: []string{"/foo"},
			toRemove:     "/foo",
		},
		{
			// This differs from the real os implementation, as removing a non-empty
			// dir is supposed to fail.
			name:         "Non-empty dir",
			dirsToCreate: []string{"/foo/bar"},
			toRemove:     "/foo",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper := CreateMemMapOSWrapper()
			for _, f := range testCase.filesToCreate {
				_, err := wrapper.Create(f)
				require.NoErrorf(t, err, "Failed to create file: %v", err)
			}
			for _, d := range testCase.dirsToCreate {
				err := wrapper.MkdirAll(d, 0o700)
				require.NoErrorf(t, err, "Failed to create directory: %v", err)
			}
			err := wrapper.Remove(testCase.toRemove)
			if testCase.wantErr {
				require.ErrorContains(t, err, testCase.wantErrMsg)
			} else {
				require.NoErrorf(t, err, "Got unexpected error: %v")
			}

			// Make sure things were actually deleted.
			walkfunc := func(root string, info fs.FileInfo, err error) error {
				require.Equal(t, "/", root)
				return err
			}

			err = wrapper.Walk("/", walkfunc)
			require.NoErrorf(t, err, "Error walking: %v", err)
		})
	}
}

func TestRemoveAll(t *testing.T) {
	tests := []struct {
		name          string
		filesToCreate []string
		dirsToCreate  []string
		toRemove      string
	}{
		{
			name:     "Non-existent",
			toRemove: "/foo.txt",
		},
		{
			name:          "File",
			filesToCreate: []string{"/foo.txt"},
			toRemove:      "/foo.txt",
		},
		{
			name:         "Empty dir",
			dirsToCreate: []string{"/foo"},
			toRemove:     "/foo",
		},
		{
			// This differs from the real os implementation, as removing a non-empty
			// dir is supposed to fail.
			name:         "Non-empty dir",
			dirsToCreate: []string{"/foo/bar"},
			toRemove:     "/foo",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper := CreateMemMapOSWrapper()
			for _, f := range testCase.filesToCreate {
				_, err := wrapper.Create(f)
				require.NoErrorf(t, err, "Failed to create file: %v", err)
			}
			for _, d := range testCase.dirsToCreate {
				err := wrapper.MkdirAll(d, 0o700)
				require.NoErrorf(t, err, "Failed to create directory: %v", err)
			}
			err := wrapper.RemoveAll(testCase.toRemove)
			require.NoErrorf(t, err, "Got unexpected error: %v")

			// Make sure things were actually deleted.
			walkfunc := func(root string, info fs.FileInfo, err error) error {
				require.Equal(t, "/", root)
				return err
			}

			err = wrapper.Walk("/", walkfunc)
			require.NoErrorf(t, err, "Error walking: %v", err)
		})
	}
}

func TestWriteFile_OpenFile(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	err := wrapper.WriteFile("/foo.txt", []byte("content"), 0o700)
	require.NoErrorf(t, err, "Failed to write file: %v", err)

	file, err := wrapper.OpenFile("/foo.txt", os.O_RDONLY, 0o700)
	require.NoErrorf(t, err, "Failed to open file: %v", err)
	buffer := make([]byte, 7)
	bytesRead, err := file.Read(buffer)
	require.NoErrorf(t, err, "Failed to read from file: %v", err)
	require.Equal(t, []byte("content"), buffer)
	require.Equal(t, 7, bytesRead)
	bytesRead, err = file.Read(buffer)
	require.Errorf(t, io.EOF, "Did not get EOF: %v", err)
	require.Equal(t, 0, bytesRead)
}

func TestWriteFile_ReadFile(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	err := wrapper.WriteFile("/foo.txt", []byte("content"), 0o700)
	require.NoErrorf(t, err, "Failed to write file: %v", err)

	contents, err := wrapper.ReadFile("/foo.txt")
	require.NoErrorf(t, err, "Failed to read file: %v", err)
	require.Equal(t, []byte("content"), contents)
}

func TestWriteFile_Stat(t *testing.T) {
	wrapper := CreateMemMapOSWrapper()
	approxWriteTime := time.Now()
	err := wrapper.WriteFile("/foo.txt", []byte("content"), 0o700)
	require.NoErrorf(t, err, "Failed to write file: %v", err)

	fileInfo, err := wrapper.Stat("/foo.txt")
	require.NoErrorf(t, err, "Got error stat-ing file: %v", err)
	require.Equal(t, "foo.txt", fileInfo.Name())
	require.Equal(t, int64(7), fileInfo.Size())
	require.Equal(t, os.FileMode(0o700), fileInfo.Mode())
	// Assert that the actual mod time is within 10ms of when we requested the
	// write.
	require.LessOrEqual(t, fileInfo.ModTime().UnixMilli()-approxWriteTime.UnixMilli(), int64(10))
	require.False(t, fileInfo.IsDir())
}
