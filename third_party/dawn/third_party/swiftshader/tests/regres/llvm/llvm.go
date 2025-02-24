// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Package llvm provides functions and types for locating and using the llvm
// toolchains.
package llvm

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strconv"

	"swiftshader.googlesource.com/SwiftShader/tests/regres/util"
)

const maxLLVMVersion = 17

// Version holds the build version information of an LLVM toolchain.
type Version struct {
	Major, Minor, Point int
}

func (v Version) String() string {
	return fmt.Sprintf("%v.%v.%v", v.Major, v.Minor, v.Point)
}

// GreaterEqual returns true if v >= rhs.
func (v Version) GreaterEqual(rhs Version) bool {
	if v.Major > rhs.Major {
		return true
	}
	if v.Major < rhs.Major {
		return false
	}
	if v.Minor > rhs.Minor {
		return true
	}
	if v.Minor < rhs.Minor {
		return false
	}
	return v.Point >= rhs.Point
}

// Download downloads and verifies the LLVM toolchain for the current OS.
func (v Version) Download() ([]byte, error) {
	return v.DownloadForOS(runtime.GOOS)
}

// DownloadForOS downloads and verifies the LLVM toolchain for the given OS.
func (v Version) DownloadForOS(osName string) ([]byte, error) {
	url, sig, key, err := v.DownloadInfoForOS(osName)
	if err != nil {
		return nil, err
	}

	resp, err := http.Get(url)
	if err != nil {
		return nil, fmt.Errorf("Could not download LLVM from %v: %v", url, err)
	}
	defer resp.Body.Close()

	content, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("Could not download LLVM from %v: %v", url, err)
	}

	if sig != "" {
		sigfile, err := os.Open(sig)
		if err != nil {
			return nil, fmt.Errorf("Couldn't open file '%s': %v", sig, err)
		}
		defer sigfile.Close()

		keyfile, err := os.Open(key)
		if err != nil {
			return nil, fmt.Errorf("Couldn't open file '%s': %v", key, err)
		}
		defer keyfile.Close()

		if err := util.CheckPGP(bytes.NewReader(content), sigfile, keyfile); err != nil {
			return nil, err
		}
	}
	return content, nil
}

// DownloadInfoForOS returns the download url, signature and key for the given
// LLVM version for the given OS.
func (v Version) DownloadInfoForOS(os string) (url, sig, key string, err error) {
	switch v {
	case Version{10, 0, 0}:
		key = relfile("10.0.0.pub.key")
		switch os {
		case "linux":
			url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.0/clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz"
			sig = relfile("10.0.0-ubuntu.sig")
			return
		case "darwin":
			url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.0/clang+llvm-10.0.0-x86_64-apple-darwin.tar.xz"
			sig = relfile("10.0.0-darwin.sig")
			return
		case "windows":
			url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.0/LLVM-10.0.0-win64.exe"
			sig = relfile("10.0.0-win64.sig")
			return
		default:
			return "", "", "", fmt.Errorf("Unsupported OS: %v", os)
		}
	case Version{17, 0, 6}:
		switch os {
		case "linux":
			url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/clang+llvm-17.0.6-x86_64-linux-gnu-ubuntu-22.04.tar.xz"
			return
		case "darwin":
			url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/clang+llvm-17.0.6-arm64-apple-darwin22.0.tar.xz"
			return
		case "windows":
			url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/LLVM-17.0.6-win64.exe"
			return
		default:
			return "", "", "", fmt.Errorf("Unsupported OS: %v", os)
		}
	default:
		return "", "", "", fmt.Errorf("Unknown download for LLVM %v", v)
	}
}
func relfile(path string) string {
	_, thisFile, _, _ := runtime.Caller(1)
	thisDir := filepath.Dir(thisFile)
	return filepath.Join(thisDir, path)
}

// Toolchain holds the paths and version information about an LLVM toolchain.
type Toolchain struct {
	Version Version
	BinDir  string
}

// Toolchains is a list of Toolchain
type Toolchains []Toolchain

// Find looks for a toolchain with the specific version.
func (l Toolchains) Find(v Version) *Toolchain {
	for _, t := range l {
		if t.Version == v {
			return &t
		}
	}
	return nil
}

// FindAtLeast looks for a toolchain with the given version, returning the highest found version.
func (l Toolchains) FindAtLeast(v Version) *Toolchain {
	out := (*Toolchain)(nil)
	for _, t := range l {
		if t.Version.GreaterEqual(v) && (out == nil || out.Version.GreaterEqual(t.Version)) {
			t := t
			out = &t
		}
	}
	return out
}

// Search looks for llvm toolchains in paths.
// If paths is empty, then PATH is searched.
func Search(paths ...string) Toolchains {
	toolchains := map[Version]Toolchain{}
	search := func(name string) {
		if len(paths) > 0 {
			for _, path := range paths {
				if util.IsFile(path) {
					path = filepath.Dir(path)
				}
				if t := toolchain(path); t != nil {
					toolchains[t.Version] = *t
					continue
				}
				if t := toolchain(filepath.Join(path, "bin")); t != nil {
					toolchains[t.Version] = *t
					continue
				}
			}
		} else {
			path, err := exec.LookPath(name)
			if err == nil {
				if t := toolchain(filepath.Dir(path)); t != nil {
					toolchains[t.Version] = *t
				}
			}
		}
	}

	search("clang")
	for i := 8; i < maxLLVMVersion; i++ {
		search(fmt.Sprintf("clang-%d", i))
	}

	out := make([]Toolchain, 0, len(toolchains))
	for _, t := range toolchains {
		out = append(out, t)
	}
	sort.Slice(out, func(i, j int) bool { return out[i].Version.GreaterEqual(out[j].Version) })

	return out
}

// Clang returns the path to the clang executable.
func (t Toolchain) Clang() string {
	return filepath.Join(t.BinDir, "clang"+exeExt())
}

// ClangXX returns the path to the clang++ executable.
func (t Toolchain) ClangXX() string {
	return filepath.Join(t.BinDir, "clang++"+exeExt())
}

// Cov returns the path to the llvm-cov executable.
func (t Toolchain) Cov() string {
	return filepath.Join(t.BinDir, "llvm-cov"+exeExt())
}

// Profdata returns the path to the llvm-profdata executable.
func (t Toolchain) Profdata() string {
	return filepath.Join(t.BinDir, "llvm-profdata"+exeExt())
}

func toolchain(dir string) *Toolchain {
	t := Toolchain{BinDir: dir}
	if t.resolve() {
		return &t
	}
	return nil
}

func (t *Toolchain) resolve() bool {
	if !util.IsFile(t.Profdata()) { // llvm-profdata doesn't have --version flag
		return false
	}
	version, ok := parseVersion(t.Cov())
	t.Version = version
	return ok
}

func exeExt() string {
	switch runtime.GOOS {
	case "windows":
		return ".exe"
	default:
		return ""
	}
}

var versionRE = regexp.MustCompile(`(?:clang|LLVM) version ([0-9]+)\.([0-9]+)\.([0-9]+)`)

func parseVersion(tool string) (Version, bool) {
	out, err := exec.Command(tool, "--version").Output()
	if err != nil {
		return Version{}, false
	}
	matches := versionRE.FindStringSubmatch(string(out))
	if len(matches) < 4 {
		return Version{}, false
	}
	major, majorErr := strconv.Atoi(matches[1])
	minor, minorErr := strconv.Atoi(matches[2])
	point, pointErr := strconv.Atoi(matches[3])
	if majorErr != nil || minorErr != nil || pointErr != nil {
		return Version{}, false
	}
	return Version{major, minor, point}, true
}
