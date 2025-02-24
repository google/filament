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

package merge

import (
	"context"
	"flag"
	"fmt"
	"os"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		output string
	}
}

func (cmd) Name() string { return "merge" }

func (cmd) Desc() string { return "merges results files into one" }

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	flag.StringVar(&c.flags.output, "o", "results.txt", "output file. '-' writes to stdout")
	return []string{"first-results.txt", "second-results.txt ..."}, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	// Load each of the resultsByExecutionMode files and merge together
	resultsByExecutionMode := make(result.ResultsByExecutionMode)
	for _, path := range flag.Args() {
		// Load results
		r, err := result.Load(path)
		if err != nil {
			return fmt.Errorf("while reading '%v': %w", path, err)
		}
		// Combine and merge
		for _, test := range cfg.Tests {
			resultsByExecutionMode[test.ExecutionMode] = result.Merge(resultsByExecutionMode[test.ExecutionMode], r[test.ExecutionMode])
		}
	}

	// Open output file
	output := os.Stdout
	if c.flags.output != "-" {
		var err error
		output, err = os.Create(c.flags.output)
		if err != nil {
			return fmt.Errorf("failed to open output file '%v': %w", c.flags.output, err)
		}
		defer output.Close()
	}

	// Write out
	return result.Write(output, resultsByExecutionMode)
}
