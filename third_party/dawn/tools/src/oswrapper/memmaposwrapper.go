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

// Contains implementations of oswrapper interfaces that are fully in memory.
package oswrapper

import (
	"fmt"
	"math/rand"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"

	"github.com/spf13/afero"
)

type MemMapOSWrapper struct {
	MemMapEnvironProvider
	MemMapFilesystemReaderWriter
}

type MemMapEnvironProvider struct {
	Environment map[string]string
}

type MemMapFilesystemReaderWriter struct {
	MemMapFilesystemReader
	MemMapFilesystemWriter
}

type MemMapFilesystemReader struct {
	fs afero.Fs
}

type MemMapFilesystemWriter struct {
	fs afero.Fs
}

func CreateMemMapOSWrapper() MemMapOSWrapper {
	filesystem := afero.NewMemMapFs()
	return MemMapOSWrapper{
		MemMapEnvironProvider{},
		MemMapFilesystemReaderWriter{
			MemMapFilesystemReader{
				fs: filesystem,
			},
			MemMapFilesystemWriter{
				fs: filesystem,
			},
		},
	}
}

func (m MemMapEnvironProvider) Environ() []string {
	environment := []string{}
	for k, v := range m.Environment {
		environment = append(environment, fmt.Sprintf("%v=%v", k, v))
	}
	sort.Strings(environment)
	return environment
}

func (m MemMapEnvironProvider) Getenv(key string) string {
	return m.Environment[key]
}

func (m MemMapEnvironProvider) Getwd() (string, error) {
	wd, exists := m.Environment["PWD"]
	if exists {
		return wd, nil
	}
	return "/", nil
}

func (m MemMapEnvironProvider) UserHomeDir() (string, error) {
	homeDir, exists := m.Environment["HOME"]
	if exists {
		return homeDir, nil
	}
	homeDir, exists = m.Environment["USERPROFILE"]
	if exists {
		return homeDir, nil
	}
	return "/", nil
}

func (m MemMapFilesystemReader) Open(name string) (afero.File, error) {
	return m.fs.Open(name)
}

func (m MemMapFilesystemReader) OpenFile(name string, flag int, perm os.FileMode) (afero.File, error) {
	return m.fs.OpenFile(name, flag, perm)
}

func (m MemMapFilesystemReader) ReadFile(name string) ([]byte, error) {
	return afero.ReadFile(m.fs, name)
}

func (m MemMapFilesystemReader) Stat(name string) (os.FileInfo, error) {
	return m.fs.Stat(name)
}

func (m MemMapFilesystemReader) Walk(root string, fn filepath.WalkFunc) error {
	return afero.Walk(m.fs, root, fn)
}

func (m MemMapFilesystemWriter) Create(name string) (afero.File, error) {
	return m.fs.Create(name)
}

func (m MemMapFilesystemWriter) Mkdir(path string, perm os.FileMode) error {
	return m.fs.Mkdir(path, perm)
}

func (m MemMapFilesystemWriter) MkdirAll(path string, perm os.FileMode) error {
	return m.fs.MkdirAll(path, perm)
}

func (m MemMapFilesystemWriter) MkdirTemp(dir, pattern string) (string, error) {
	// Create a random string and substitute it as defined in the os package
	// documentation.
	randomString := strconv.Itoa(rand.Intn(100000))
	index := strings.LastIndex(pattern, "*")
	if index == -1 {
		pattern = pattern + randomString
	} else {
		pattern = pattern[:index] + randomString + pattern[index+1:]
	}
	path := filepath.Join(dir, pattern)

	return path, m.fs.MkdirAll(path, 0o700)
}

func (m MemMapFilesystemWriter) Remove(name string) error {
	// This appears to not actually function identically to os' implementation,
	// as it successfully removes non-empty directories.
	return m.fs.Remove(name)
}

func (m MemMapFilesystemWriter) RemoveAll(path string) error {
	return m.fs.RemoveAll(path)
}

func (m MemMapFilesystemWriter) Symlink(oldname, newname string) error {
	return fmt.Errorf("%s", "Symlink not currently supported in tests")
}

func (m MemMapFilesystemWriter) WriteFile(name string, data []byte, perm os.FileMode) error {
	return afero.WriteFile(m.fs, name, data, perm)
}
