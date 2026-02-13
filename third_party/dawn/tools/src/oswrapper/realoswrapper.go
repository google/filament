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

// Contains implementations of oswrapper interfaces that actually interact with the OS.
package oswrapper

import (
	"os"
	"path/filepath"
)

// RealOSWrapper provides a concrete implementation of OSWrapper that interacts
// with the real operating system.
type RealOSWrapper struct {
	RealEnvironProvider
	RealFilesystemReaderWriter
}

// RealEnvironProvider provides a concrete implementation of EnvironProvider.
type RealEnvironProvider struct{}

// RealFilesystemReaderWriter provides a concrete implementation of FilesystemReaderWriter.
type RealFilesystemReaderWriter struct {
	RealFilesystemReader
	RealFilesystemWriter
}

// RealFilesystemReader provides a concrete implementation of FilesystemReader
// that interacts with the real operating system.
type RealFilesystemReader struct{}

// RealFilesystemWriter provides a concrete implementation of FilesystemWriter
// that interacts with the real operating system.
type RealFilesystemWriter struct{}

// GetRealOSWrapper creates a new RealOSWrapper that interacts with the real OS.
func GetRealOSWrapper() RealOSWrapper {
	return RealOSWrapper{
		RealEnvironProvider{},
		RealFilesystemReaderWriter{
			RealFilesystemReader{},
			RealFilesystemWriter{},
		},
	}
}

func (RealEnvironProvider) Environ() []string {
	return os.Environ()
}

func (RealEnvironProvider) Getenv(key string) string {
	return os.Getenv(key)
}

func (RealEnvironProvider) Getwd() (string, error) {
	return os.Getwd()
}

func (RealEnvironProvider) UserHomeDir() (string, error) {
	return os.UserHomeDir()
}

func (RealFilesystemReader) Open(name string) (File, error) {
	return os.Open(name)
}

func (RealFilesystemReader) OpenFile(name string, flag int, perm os.FileMode) (File, error) {
	return os.OpenFile(name, flag, perm)
}

func (RealFilesystemReader) ReadFile(name string) ([]byte, error) {
	return os.ReadFile(name)
}

func (RealFilesystemReader) ReadDir(name string) ([]os.DirEntry, error) {
	return os.ReadDir(name)
}

func (RealFilesystemReader) Stat(name string) (os.FileInfo, error) {
	return os.Stat(name)
}

func (RealFilesystemReader) Walk(root string, fn filepath.WalkFunc) error {
	return filepath.Walk(root, fn)
}

func (RealFilesystemWriter) Create(name string) (File, error) {
	return os.Create(name)
}

func (RealFilesystemWriter) Mkdir(path string, perm os.FileMode) error {
	return os.Mkdir(path, perm)
}

func (RealFilesystemWriter) MkdirAll(path string, perm os.FileMode) error {
	return os.MkdirAll(path, perm)
}

func (RealFilesystemWriter) MkdirTemp(dir, pattern string) (string, error) {
	return os.MkdirTemp(dir, pattern)
}

func (RealFilesystemWriter) Remove(name string) error {
	return os.Remove(name)
}

func (RealFilesystemWriter) RemoveAll(path string) error {
	return os.RemoveAll(path)
}

func (RealFilesystemWriter) Symlink(oldname, newname string) error {
	return os.Symlink(oldname, newname)
}

func (RealFilesystemWriter) WriteFile(name string, data []byte, perm os.FileMode) error {
	return os.WriteFile(name, data, perm)
}
