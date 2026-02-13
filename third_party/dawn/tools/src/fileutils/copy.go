// Copyright 2024 The Dawn & Tint Authors
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

package fileutils

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

// CopyFile copies the file from 'src' to 'dst', creating the destination directory
// if needed and overwriting the destination file if it already exists.
// It preserves the file mode from the source.
func CopyFile(dst, src string, fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	srcInfo, err := fsReaderWriter.Stat(src)
	if err != nil {
		return fmt.Errorf("cannot stat source '%v': %w", src, err)
	}
	if srcInfo.IsDir() {
		return fmt.Errorf("source '%v' is a directory, not a file", src)
	}

	s, err := fsReaderWriter.Open(src)
	if err != nil {
		return fmt.Errorf("failed to open source file '%v': %w", src, err)
	}
	defer s.Close()

	if err := fsReaderWriter.MkdirAll(filepath.Dir(dst), 0755); err != nil {
		return fmt.Errorf("failed to create destination directory '%v': %w", filepath.Dir(dst), err)
	}

	d, err := fsReaderWriter.OpenFile(dst, os.O_CREATE|os.O_TRUNC|os.O_WRONLY, srcInfo.Mode())
	if err != nil {
		return fmt.Errorf("failed to open destination file '%v': %w", dst, err)
	}
	defer d.Close()

	if _, err = io.Copy(d, s); err != nil {
		return fmt.Errorf("failed to copy data from '%v' to '%v': %w", src, dst, err)
	}

	return nil
}

// CopyDir recursively copies the content of the 'src' directory to 'dst'.
// If 'dst' exists, it will be completely overwritten with the content of 'src'.
// If 'dst' does not exist, it will be created.
func CopyDir(dst, src string, fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	srcInfo, err := fsReaderWriter.Stat(src)
	if err != nil {
		return fmt.Errorf("cannot stat source '%v': %w", src, err)
	}
	if !srcInfo.IsDir() {
		return fmt.Errorf("source '%v' is not a directory", src)
	}

	var dstMode = srcInfo.Mode()
	if dstInfo, err := fsReaderWriter.Stat(dst); err == nil {
		if !dstInfo.IsDir() {
			return fmt.Errorf("destination '%v' is a file, not a directory", dst)
		}
		dstMode = dstInfo.Mode() // Preserve original dst permissions
	} else if !os.IsNotExist(err) { // Some other error
		return fmt.Errorf("cannot stat destination '%v': %w", dst, err)
	}

	// To prevent recursion, ensure dst is not a subdirectory of src.
	cleanSrc := filepath.Clean(src)
	cleanDst := filepath.Clean(dst)
	if strings.HasPrefix(cleanDst, cleanSrc) && (cleanDst == cleanSrc || cleanDst[len(cleanSrc)] == filepath.Separator) {
		return fmt.Errorf("cannot copy directory '%s' into itself '%s'", src, dst)
	}

	if err := fsReaderWriter.RemoveAll(dst); err != nil && !os.IsNotExist(err) {
		return fmt.Errorf("failed to remove destination directory '%v': %w", dst, err)
	}
	if err := fsReaderWriter.MkdirAll(dst, dstMode); err != nil {
		return fmt.Errorf("failed to create destination directory '%v': %w", dst, err)
	}

	return fsReaderWriter.Walk(src, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		relPath, err := filepath.Rel(src, path)
		if err != nil {
			return fmt.Errorf("failed to calculate relative path for '%s': %w", path, err)
		}

		if relPath == "." { // Skip the root directory itself.
			return nil
		}

		dstPath := filepath.Join(dst, relPath)

		if info.IsDir() {
			return fsReaderWriter.MkdirAll(dstPath, info.Mode())
		}

		return CopyFile(dstPath, path, fsReaderWriter)
	})
}
