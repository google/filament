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

	"github.com/spf13/afero"
)

type RealOSWrapper struct {
	RealEnvironProvider
	RealFilesystemReaderWriter
}

type RealEnvironProvider struct{}

type RealFilesystemReaderWriter struct {
	RealFilesystemReader
	RealFilesystemWriter
}

// Since these take Fs interface references instead of concrete implementations,
// we could theoretically share most of the implementation between this and
// MemMapFilesystemReader/Writer, relying on the fact that the real one will
// have an OsFs and the test one will with a MemMapFs. However, since afero.Fs
// does not implement MkdirTemp or Symlink, we would need to have some
// differences anyways. So, keep the implementations completely separate for
// now.
type RealFilesystemReader struct {
	fs afero.Fs
}
type RealFilesystemWriter struct {
	fs afero.Fs
}

func GetRealOSWrapper() RealOSWrapper {
	filesystem := afero.NewOsFs()
	return RealOSWrapper{
		RealEnvironProvider{},
		RealFilesystemReaderWriter{
			RealFilesystemReader{
				fs: filesystem,
			},
			RealFilesystemWriter{
				fs: filesystem,
			},
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

func (rfsr RealFilesystemReader) Open(name string) (afero.File, error) {
	return rfsr.fs.Open(name)
}

func (rfsr RealFilesystemReader) OpenFile(name string, flag int, perm os.FileMode) (afero.File, error) {
	return rfsr.fs.OpenFile(name, flag, perm)
}

func (RealFilesystemReader) ReadFile(name string) ([]byte, error) {
	return os.ReadFile(name)
}

func (RealFilesystemReader) Stat(name string) (os.FileInfo, error) {
	return os.Stat(name)
}

func (RealFilesystemReader) Walk(root string, fn filepath.WalkFunc) error {
	return filepath.Walk(root, fn)
}

func (rfsw RealFilesystemWriter) Create(name string) (afero.File, error) {
	return rfsw.fs.Create(name)
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
