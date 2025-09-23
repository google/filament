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

// Contains implementation of oswrapper interfaces using fstest.
package oswrapper

import (
	"fmt"
	"io/fs"
	"os"
	"path"
	"path/filepath"
	"strings"
	"testing/fstest"
)

// FSTestOSWrapper is an in-memory implementation of OSWrapper for testing.
// It uses a map to simulate a filesystem, which can be used to create
// a read-only fstest.MapFS for read operations. Write operations
// manipulate the map directly.
type FSTestOSWrapper struct {
	FSTestEnvironProvider
	FSTestFilesystemReaderWriter
}

// CreateFSTestOSWrapper creates a new FSTestOSWrapper with an empty in-memory filesystem.
func CreateFSTestOSWrapper() FSTestOSWrapper {
	return FSTestOSWrapper{
		FSTestEnvironProvider{},
		FSTestFilesystemReaderWriter{
			FS: make(map[string]*fstest.MapFile),
		},
	}
}

// FSTestEnvironProvider is a stub implementation of EnvironProvider that panics if called.
type FSTestEnvironProvider struct{}

// FSTestFilesystemReaderWriter provides an in-memory implementation of FilesystemReaderWriter.
// It holds the map that represents the filesystem.
type FSTestFilesystemReaderWriter struct {
	FS map[string]*fstest.MapFile
}

// CleanPath converts an OS dependent path into a fs.FS compatible path that can be used as a key
// within FSTestOSWrapper
func (w FSTestFilesystemReaderWriter) CleanPath(pathStr string) string {
	// Replace all backslashes with forward slashes to handle Windows paths on any OS.
	p := strings.ReplaceAll(pathStr, "\\", "/")
	// Use the platform-agnostic path package to clean the path.
	p = path.Clean(p)

	// Normalize the path to be compatible with fs.FS
	p = strings.TrimPrefix(p, "/")
	if p == "" {
		p = "." // The canonical representation of the root in a fs.FS.
	}
	return p
}

// fs returns a read-only fs.FS view of the underlying map.
func (w FSTestFilesystemReaderWriter) fs() fs.FS {
	return fstest.MapFS(w.FS)
}

// --- EnvironProvider implementation ---

func (p FSTestEnvironProvider) Environ() []string {
	panic("Environ() is not currently implemented in fstest wrapper")
}

func (p FSTestEnvironProvider) Getenv(_ string) string {
	panic("Getenv() is not currently implemented in fstest wrapper")
}

func (p FSTestEnvironProvider) Getwd() (string, error) {
	panic("Getwd() is not currently implemented in fstest wrapper")
}

func (p FSTestEnvironProvider) UserHomeDir() (string, error) {
	panic("UserHomeDir() is not currently implemented in fstest wrapper")
}

// --- FilesystemReader implementation ---

func (w FSTestFilesystemReaderWriter) Open(name string) (File, error) {
	panic("Open() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) OpenFile(name string, flag int, perm os.FileMode) (File, error) {
	panic("OpenFile() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) ReadFile(name string) ([]byte, error) {
	panic("ReadFile() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) ReadDir(name string) ([]os.DirEntry, error) {
	panic("ReadDir() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) Stat(name string) (os.FileInfo, error) {
	panic("Stat() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) Walk(root string, fn filepath.WalkFunc) error {
	panic("Walk() is not currently implemented in fstest wrapper")
}

// --- FilesystemWriter implementation ---

func (w FSTestFilesystemReaderWriter) Create(name string) (File, error) {
	panic("Create() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) Mkdir(path string, perm os.FileMode) error {
	panic("Mkdir() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) MkdirAll(path string, perm os.FileMode) error {
	panic("MkdirAll() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) MkdirTemp(dir, pattern string) (string, error) {
	panic("MkdirTemp() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) Remove(name string) error {
	panic("Remove() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) RemoveAll(path string) error {
	panic("RemoveAll() is not currently implemented in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) Symlink(oldname, newname string) error {
	return fmt.Errorf("symlink not currently supported in fstest wrapper")
}

func (w FSTestFilesystemReaderWriter) WriteFile(name string, data []byte, perm os.FileMode) error {
	panic("WriteFile() is not currently implemented in fstest wrapper")
}
