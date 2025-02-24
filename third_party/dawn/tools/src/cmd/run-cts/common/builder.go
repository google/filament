// Copyright 2023 The Dawn & Tint Authors
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
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"time"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

type Builder struct {
	Name string // 'standalone', 'node'
	Out  string // Absolute path to the output directory
	CTS  string // Absolute path to the CTS root directory
	npx  string // Path to the npx executable
}

// BuildIfRequired calls Build() if the CTS sources have been modified since the last build.
func (b *Builder) BuildIfRequired(verbose bool) error {
	name := fmt.Sprintf("cts %v", b.Name)

	// Scan the CTS source to determine the most recent change to the CTS source
	mostRecentSourceChange, err := scanSourceTimestamps(filepath.Join(b.Out, "../src"), verbose)
	if err != nil {
		return fmt.Errorf("failed to scan source files for modified timestamps: %w", err)
	}

	type Cache struct {
		BuildTimestamp map[string]time.Time // name -> most recent timestamp
	}
	cache := Cache{BuildTimestamp: map[string]time.Time{}}

	cachePath := ""
	if home, err := os.UserHomeDir(); err == nil {
		cacheDir := filepath.Join(home, ".cache/webgpu")
		cachePath = filepath.Join(cacheDir, "run-cts.json")
		os.MkdirAll(cacheDir, 0777)
	}

	needsRebuild := true
	if cachePath != "" { // consult the cache to see if we need to rebuild
		if cacheFile, err := os.Open(cachePath); err == nil {
			if err := json.NewDecoder(cacheFile).Decode(&cache); err == nil {
				if fileutils.IsDir(b.Out) {
					needsRebuild = mostRecentSourceChange.After(cache.BuildTimestamp[b.Name])
				}
			}
			cacheFile.Close()
		}
	}

	if verbose {
		fmt.Println(name, "needs rebuild:", needsRebuild)
	}

	if needsRebuild {
		if err := b.Build(verbose); err != nil {
			return fmt.Errorf("failed to build %v: %w", name, err)
		}
	}

	if cachePath != "" {
		// Update the cache timestamp
		if cacheFile, err := os.Create(cachePath); err == nil {
			cache.BuildTimestamp[b.Name] = mostRecentSourceChange
			json.NewEncoder(cacheFile).Encode(&cache)
			cacheFile.Close()
		}
	}

	return nil
}

// Build executes the necessary build commands to build the CTS, including
// copying the cache files from gen to the out directory and compiling the
// TypeScript files down to JavaScript.
func (b *Builder) Build(verbose bool) error {
	if verbose {
		start := time.Now()
		fmt.Printf("Building CTS %v...\n", b.Name)
		defer func() {
			fmt.Println("completed in", time.Since(start))
		}()
	}

	if err := os.MkdirAll(b.Out, 0777); err != nil {
		return err
	}

	if !fileutils.IsExe(b.npx) {
		return fmt.Errorf("cannot find npx at '%v'", b.npx)
	}

	for _, action := range []string{"run:generate-version", b.Name} {
		cmd := exec.Command(b.npx, "grunt", action)
		cmd.Dir = b.CTS
		out, err := cmd.CombinedOutput()
		if err != nil {
			return fmt.Errorf("%w: %v", err, string(out))
		}
		if verbose {
			fmt.Println(string(out))
		}
	}

	return nil
}
