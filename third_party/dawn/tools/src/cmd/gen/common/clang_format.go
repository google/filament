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

package common

import (
	"fmt"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

var clangFormatPath string

// ClangFormat invokes clang-format to format the file content 'src'.
// Returns the formatted file.
func ClangFormat(src string) (string, error) {
	if clangFormatPath == "" {
		path, err := findClangFormat()
		if err != nil {
			return "", err
		}
		clangFormatPath = path
	}
	cmd := exec.Command(clangFormatPath)
	cmd.Stdin = strings.NewReader(src)
	out, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("clang-format failed:\n%v\n%v", string(out), err)
	}
	return string(out), nil
}

// Looks for clang-format in the 'buildtools' directory, falling back to PATH
func findClangFormat() (string, error) {
	dawnRoot := fileutils.DawnRoot()
	var path string
	switch runtime.GOOS {
	case "linux":
		path = filepath.Join(dawnRoot, "buildtools/linux64/clang-format")
	case "darwin":
		path = filepath.Join(dawnRoot, "buildtools/mac/clang-format")
	case "windows":
		path = filepath.Join(dawnRoot, "buildtools/win/clang-format.exe")
	}
	if fileutils.IsExe(path) {
		return path, nil
	}
	var err error
	path, err = exec.LookPath("clang-format")
	if err != nil {
		return "", fmt.Errorf("failed to find clang-format: %w", err)
	}
	return path, nil
}
