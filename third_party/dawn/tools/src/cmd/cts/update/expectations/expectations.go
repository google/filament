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

package expectations

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/auth"
	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"go.chromium.org/luci/auth/client/authcli"
)

func init() {
	common.Register(&cmd{})
}

type arrayFlags []string

func (i *arrayFlags) String() string {
	return strings.Join((*i), " ")
}

func (i *arrayFlags) Set(value string) error {
	*i = append(*i, value)
	return nil
}

type cmd struct {
	flags struct {
		results              common.ResultSource
		expectations         arrayFlags
		auth                 authcli.Flags
		verbose              bool
		generateExplicitTags bool // If true, the most explicit tags will be used instead of several broad ones
	}
}

func (cmd) Name() string {
	return "update-expectations"
}

func (cmd) Desc() string {
	return "updates a CTS expectations file"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	c.flags.results.RegisterFlags(cfg)
	c.flags.auth.Register(flag.CommandLine, auth.DefaultAuthOptions(cfg.OsWrapper))
	flag.BoolVar(&c.flags.verbose, "verbose", false, "emit additional logging")
	flag.BoolVar(&c.flags.generateExplicitTags, "generate-explicit-tags", false,
		"Use the most explicit tags for expectations instead of several broad ones")
	flag.Var(&c.flags.expectations, "expectations", "path to CTS expectations file(s) to update")
	return nil, nil
}

func loadTestList(path string) ([]query.Query, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("failed to load test list: %w", err)
	}
	lines := strings.Split(string(data), "\n")
	out := make([]query.Query, len(lines))
	for i, l := range lines {
		out[i] = query.Parse(l)
	}
	return out, nil
}

// TODO(crbug.com/344014313): Add unittests once expectations.Load() and
// expectations.Save() use dependency injection.
func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	if len(c.flags.expectations) == 0 {
		c.flags.expectations = common.DefaultExpectationsPaths(cfg.OsWrapper)
	}

	// Validate command line arguments
	auth, err := c.flags.auth.Options()
	if err != nil {
		return fmt.Errorf("failed to obtain authentication options: %w", err)
	}

	// Fetch the results
	log.Println("fetching results...")
	resultsByExecutionMode, err := c.flags.results.GetUnsuppressedFailingResults(ctx, cfg, auth)
	if err != nil {
		return err
	}

	// Merge to remove duplicates
	log.Println("removing duplicate results...")
	for name := range resultsByExecutionMode {
		resultsByExecutionMode[name] = result.Merge(resultsByExecutionMode[name])
	}

	log.Println("loading test list...")
	testlist, err := loadTestList(common.DefaultTestListPath(cfg.OsWrapper))
	if err != nil {
		return err
	}

	for _, expectationsFilename := range c.flags.expectations {
		// Load the expectations file
		log.Printf("loading expectations %s...\n", expectationsFilename)
		ex, err := expectations.Load(expectationsFilename)
		if err != nil {
			return err
		}

		(&ex).RemoveExpectationsForUnknownTests(&testlist)

		log.Printf("validating %s...\n", expectationsFilename)
		if diag := ex.Validate(); diag.NumErrors() > 0 {
			diag.Print(os.Stdout, expectationsFilename)
			return fmt.Errorf("validation failed")
		}

		// Update the expectations file with the results
		log.Printf("updating expectations %s...\n", expectationsFilename)
		name := result.ExecutionMode("core")
		if strings.Contains(expectationsFilename, "compat") {
			name = "compat"
		}

		err = ex.AddExpectationsForFailingResults(resultsByExecutionMode[name], c.flags.generateExplicitTags, c.flags.verbose)
		// TODO(crbug.com/372730248): Report actual diagnostics.
		diag := expectations.Diagnostics{}
		if err != nil {
			return err
		}

		// Print any diagnostics
		diag.Print(os.Stdout, expectationsFilename)

		// Save the updated expectations file
		err = ex.Save(expectationsFilename)
		if err != nil {
			break
		}
	}
	return err
}
