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
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"
)

type CTS struct {
	path       string // Path to the CTS directory
	npx        string // Path to npx executable
	node       string // Path to node executable
	Standalone Builder
	Node       Builder
}

func NewCTS(cts, npx, node string) CTS {
	return CTS{
		path: cts,
		npx:  npx,
		node: node,
		Standalone: Builder{
			Name: "standalone",
			Out:  filepath.Join(cts, "out"),
			CTS:  cts,
			npx:  npx,
		},
		Node: Builder{
			Name: "node",
			Out:  filepath.Join(cts, "out-node"),
			CTS:  cts,
			npx:  npx,
		},
	}
}

// QueryTestCases returns all the test cases that match query.
func (c *CTS) QueryTestCases(verbose bool, query string) ([]TestCase, error) {
	if verbose {
		start := time.Now()
		fmt.Println("Gathering test cases...")
		defer func() {
			fmt.Println("completed in", time.Since(start))
		}()
	}

	args := append([]string{
		"-e", "require('./out-node/common/runtime/cmdline.js');",
		"--", // Start of arguments
		// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
		// and slices away the first two arguments. When running with '-e', args
		// start at 1, so just inject a placeholder argument.
		"placeholder-arg",
		"--list",
	}, query)

	cmd := exec.Command(c.node, args...)
	cmd.Dir = c.path
	out, err := cmd.CombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("%v %v\n%w\n%v", c.node, args, err, string(out))
	}

	lines := strings.Split(string(out), "\n")
	list := make([]TestCase, 0, len(lines))
	for _, line := range lines {
		if line != "" {
			list = append(list, TestCase(line))
		}
	}
	return list, nil
}

// scanSourceTimestamps scans all the .js and .ts files in all subdirectories of
// dir, and returns the file with the most recent timestamp.
func scanSourceTimestamps(dir string, verbose bool) (time.Time, error) {
	if verbose {
		start := time.Now()
		fmt.Println("Scanning .js / .ts files for changes...")
		defer func() {
			fmt.Println("completed in", time.Since(start))
		}()
	}

	mostRecentChange := time.Time{}
	err := filepath.Walk(dir, func(path string, info os.FileInfo, err error) error {
		switch filepath.Ext(path) {
		case ".ts", ".js":
			if info.ModTime().After(mostRecentChange) {
				mostRecentChange = info.ModTime()
			}
		}
		return nil
	})
	if err != nil {
		return time.Time{}, err
	}
	return mostRecentChange, nil
}
