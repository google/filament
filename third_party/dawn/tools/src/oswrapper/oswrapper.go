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

// Package oswrapper provides interfaces for the built-in os and filepath packages.
// This facilitates better testing via dependency injection.
package oswrapper

import (
	"io"
	"os"
	"path/filepath"
)

// File is an interface that abstracts file operations, combining standard io interfaces.
// This allows for mocking and using different file implementations (real vs. in-memory).
type File interface {
	io.Reader
	io.Writer
	io.Seeker
	io.Closer
	Stat() (os.FileInfo, error)
}

// OSWrapper is a wrapper around all filesystem and environment-related os functions.
type OSWrapper interface {
	EnvironProvider
	FilesystemReaderWriter
}

// EnvironProvider is a wrapper around environment-related os functions.
type EnvironProvider interface {
	// Environ returns the environment variables as a sorted slice of "key=value" strings.
	Environ() []string
	// Getenv retrieves the value of the environment variable named by the key.
	Getenv(key string) string
	// Getwd returns a rooted path name corresponding to the current directory.
	Getwd() (string, error)
	// UserHomeDir returns the current user's home directory.
	UserHomeDir() (string, error)
}

// FilesystemReaderWriter is a wrapper around read and write-related filesystem os functions.
type FilesystemReaderWriter interface {
	FilesystemReader
	FilesystemWriter
}

// FilesystemReader is a wrapper around the read-related filesystem os functions.
type FilesystemReader interface {
	// Open opens the named file for reading.
	Open(name string) (File, error)
	// OpenFile is the generalized open call.
	OpenFile(name string, flag int, perm os.FileMode) (File, error)
	// ReadFile reads the file named by filename and returns the contents.
	ReadFile(name string) ([]byte, error)
	// ReadDir reads the named directory, returning all its directory entries.
	ReadDir(name string) ([]os.DirEntry, error)
	// Stat returns a FileInfo describing the named file.
	Stat(name string) (os.FileInfo, error)
	// Walk walks the file tree rooted at root, calling fn for each file or directory.
	Walk(root string, fn filepath.WalkFunc) error
}

// FilesystemWriter is a wrapper around the write-related filesystem os functions.
type FilesystemWriter interface {
	// Create creates the named file.
	Create(name string) (File, error)
	// Mkdir creates a new directory with the specified name and permission bits.
	Mkdir(path string, perm os.FileMode) error
	// MkdirAll creates a directory named path, along with any necessary parents.
	MkdirAll(path string, perm os.FileMode) error
	// MkdirTemp creates a new temporary directory in the directory dir.
	MkdirTemp(dir, pattern string) (string, error)
	// Remove removes the named file
	Remove(name string) error
	// RemoveAll removes path and any children it contains.
	RemoveAll(path string) error
	// Symlink creates newname as a symbolic link to oldname.
	Symlink(oldname, newname string) error
	// WriteFile writes data to a file named by filename.
	WriteFile(name string, data []byte, perm os.FileMode) error
}
