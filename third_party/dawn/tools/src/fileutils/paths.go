// Copyright 2022 The Dawn & Tint Authors
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
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

// ThisLine returns the filepath and line number of the calling function
func ThisLine() string {
	_, file, line, ok := runtime.Caller(1)
	if !ok {
		return ""
	}
	return fmt.Sprintf("%v:%v", file, line)
}

// ThisDir returns the directory of the caller function
func ThisDir() string {
	_, file, _, ok := runtime.Caller(1)
	if !ok {
		return ""
	}
	return filepath.Dir(file)
}

// DawnRoot returns the path to the dawn project's root directory or empty
// string if not found.
func DawnRoot() string {
	return pathOfFileInParentDirs(ThisDir(), "DEPS")
}

// pathOfFileInParentDirs looks for file with `name` in paths starting from
// `path`, and up into parent directories, returning the clean path in which the
// file is found, or empty string if not found.
func pathOfFileInParentDirs(path string, name string) string {
	sep := string(filepath.Separator)
	path, _ = filepath.Abs(path)
	numDirs := strings.Count(path, sep) + 1
	for i := 0; i < numDirs; i++ {
		test := filepath.Join(path, name)
		if _, err := os.Stat(test); err == nil {
			return filepath.Clean(path)
		}

		path = path + sep + ".."
	}
	return ""
}

// ExpandHome returns the string with all occurrences of '~' replaced with the
// user's home directory. The the user's home directory cannot be found, then
// the input string is returned.
func ExpandHome(path string, environProvider oswrapper.EnvironProvider) string {
	if strings.ContainsRune(path, '~') {
		if home, err := environProvider.UserHomeDir(); err == nil {
			return strings.ReplaceAll(path, "~", home)
		}
	}
	return path
}

// NodePath looks for the node binary, first in dawn's third_party directory,
// falling back to PATH.
func NodePath() string {
	if dawnRoot := DawnRoot(); dawnRoot != "" {
		node := filepath.Join(dawnRoot, "third_party/node")
		if info, err := os.Stat(node); err == nil && info.IsDir() {
			path := ""
			switch fmt.Sprintf("%v/%v", runtime.GOOS, runtime.GOARCH) { // See `go tool dist list`
			case "darwin/amd64":
				path = filepath.Join(node, "node-darwin-x64/bin/node")
			case "darwin/arm64":
				path = filepath.Join(node, "node-darwin-arm64/bin/node")
			case "linux/amd64":
				path = filepath.Join(node, "node-linux-x64/bin/node")
			case "windows/amd64":
				path = filepath.Join(node, "node.exe")
			}
			if _, err := os.Stat(path); err == nil {
				return path
			}
		}
	}

	if path, err := exec.LookPath("node"); err == nil {
		return path
	}

	return ""
}

// BuildPath looks for the binary output directory at '<dawn>/out/active'.
// Returns the path if found, otherwise an empty string.
func BuildPath() string {
	if dawnRoot := DawnRoot(); dawnRoot != "" {
		bin := filepath.Join(dawnRoot, "out/active")
		if info, err := os.Stat(bin); err == nil && info.IsDir() {
			return bin
		}
	}
	return ""
}

// IsDir returns true if the path resolves to a directory
func IsDir(path string) bool {
	s, err := os.Stat(path)
	if err != nil {
		return false
	}
	return s.IsDir()
}

// IsFile returns true if the path resolves to a file
func IsFile(path string) bool {
	s, err := os.Stat(path)
	if err != nil {
		return false
	}
	return !s.IsDir()
}

// CommonRootDir returns the common directory for pathA and pathB
func CommonRootDir(pathA, pathB string) string {
	pathA, pathB = filepath.ToSlash(pathA), filepath.ToSlash(pathB) // Normalize to forward-slash
	if !strings.HasSuffix(pathA, "/") {
		pathA += "/"
	}
	if !strings.HasSuffix(pathB, "/") {
		pathB += "/"
	}
	n := len(pathA)
	if len(pathB) < n {
		n = len(pathB)
	}
	common := ""
	for i := 0; i < n; i++ {
		a, b := pathA[i], pathB[i]
		if a != b {
			break
		}
		if a == '/' {
			common = pathA[:i+1]
		}
	}
	return common
}
