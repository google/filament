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

// Package install installs the tintd vscode extension for local development
package install

import (
	"bytes"
	"context"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"encoding/json"
	"flag"
	"fmt"
	"os/exec"
	"path/filepath"

	"dawn.googlesource.com/dawn/tools/src/cmd/tintd/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

func init() {
	common.Register(&Cmd{})
}

type Cmd struct {
	flags struct {
		symlink  bool
		buildDir string
		npmPath  string
	}
}

func (Cmd) Name() string {
	return "install"
}

func (Cmd) Desc() string {
	return `install installs the tintd vscode extension for local development`
}

func (c *Cmd) RegisterFlags(ctx context.Context, cfg *common.Config) ([]string, error) {
	dawnRoot := fileutils.DawnRoot(cfg.OsWrapper)
	npmPath, _ := exec.LookPath("npm")
	flag.BoolVar(&c.flags.symlink, "symlink", false, "create a symlink from the vscode extension directory to the build directory")
	flag.StringVar(&c.flags.buildDir, "build", filepath.Join(dawnRoot, "out", "active"), "the output build directory")
	flag.StringVar(&c.flags.npmPath, "npm", npmPath, "path to npm")

	return nil, nil
}

// TODO(crbug.com/416755658): Add unittest coverage once exec is handled via
// dependency injection.
func (c Cmd) Run(ctx context.Context, cfg *common.Config) error {
	pkgDir := c.findPackage(cfg.OsWrapper)
	if pkgDir == "" {
		return fmt.Errorf("could not find extension package directory at '%v'", c.flags.buildDir)
	}

	if !fileutils.IsExe(c.flags.npmPath, cfg.OsWrapper) {
		return fmt.Errorf("could not find npm")
	}

	// Build the package
	npmCmd := exec.Command(c.flags.npmPath, "install")
	npmCmd.Dir = pkgDir
	if out, err := npmCmd.CombinedOutput(); err != nil {
		return fmt.Errorf("npm install failed:\n%v\n%v", err, string(out))
	}

	// Load the package to get the name and version
	pkg := struct {
		Name    string
		Version string
	}{}
	packageJSONPath := filepath.Join(pkgDir, "package.json")
	packageJSON, err := cfg.OsWrapper.ReadFile(packageJSONPath)
	if err != nil {
		return fmt.Errorf("could not open '%v'", packageJSONPath)
	}
	if err := json.NewDecoder(bytes.NewReader(packageJSON)).Decode(&pkg); err != nil {
		return fmt.Errorf("could not parse '%v': %v", packageJSONPath, err)
	}

	home, err := cfg.OsWrapper.UserHomeDir()
	if err != nil {
		return fmt.Errorf("failed to obtain home directory: %w", err)
	}
	vscodeBaseExtsDir := filepath.Join(home, ".vscode", "extensions")
	if !fileutils.IsDir(vscodeBaseExtsDir, cfg.OsWrapper) {
		return fmt.Errorf("vscode extensions directory not found at '%v'", vscodeBaseExtsDir)
	}

	vscodeTintdDir := filepath.Join(vscodeBaseExtsDir, fmt.Sprintf("google.%v-%v", pkg.Name, pkg.Version))
	cfg.OsWrapper.RemoveAll(vscodeTintdDir)

	if c.flags.symlink {
		// Symlink the vscode extensions directory to the build directory
		if err := cfg.OsWrapper.Symlink(pkgDir, vscodeTintdDir); err != nil {
			return fmt.Errorf("failed to create symlink '%v' <- '%v': %w", pkgDir, vscodeTintdDir, err)
		}
	} else {
		// Copy the build directory to vscode extensions directory
		if err := fileutils.CopyDir(vscodeTintdDir, pkgDir, cfg.OsWrapper); err != nil {
			return fmt.Errorf("failed to copy '%v' to '%v': %w", pkgDir, vscodeTintdDir, err)
		}
	}

	return nil
}

// TODO(crbug.com/344014313): Add unittest coverage.
// findPackage looks for and returns the tintd package directory. Returns an empty string if not found.
func (c Cmd) findPackage(fsReader oswrapper.FilesystemReader) string {
	searchPaths := []string{
		filepath.Join(c.flags.buildDir, "gen/vscode"),
		c.flags.buildDir,
	}
	files := []string{"tintd", "package.json"}

nextDir:
	for _, dir := range searchPaths {
		for _, file := range files {
			if !fileutils.IsFile(filepath.Join(dir, file), fsReader) {
				continue nextDir
			}
		}
		return dir
	}

	return "" // Not found
}
