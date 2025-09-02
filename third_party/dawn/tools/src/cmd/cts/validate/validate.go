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

package validate

import (
	"context"
	"flag"
	"fmt"
	"os"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
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
		expectations arrayFlags // expectations file path
		slow         string     // slow test expectations file path
	}
}

func (cmd) Name() string {
	return "validate"
}

func (cmd) Desc() string {
	return "validates a WebGPU expectations.txt file"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	slowExpectations := common.DefaultSlowExpectationsPath(cfg.OsWrapper)
	flag.Var(&c.flags.expectations, "expectations", "path to CTS expectations file(s) to validate")
	flag.StringVar(&c.flags.slow, "slow", slowExpectations, "path to CTS slow expectations file to validate")
	return nil, nil
}

// TODO(crbug.com/344014313): Add unittests once expectations.Load() uses
// dependency injection.
func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	if len(c.flags.expectations) == 0 {
		c.flags.expectations = common.DefaultExpectationsPaths(cfg.OsWrapper)
	}

	for _, expectationFilename := range c.flags.expectations {
		// Load expectations.txt
		content, err := expectations.Load(expectationFilename)
		if err != nil {
			return err
		}
		diags := content.Validate()

		// Print any diagnostics
		diags.Print(os.Stdout, expectationFilename)
		if numErrs := diags.NumErrors(); numErrs > 0 {
			return fmt.Errorf("%v errors found in %v", numErrs, expectationFilename)
		}
	}

	// Load slow_tests.txt
	content, err := expectations.Load(c.flags.slow)
	if err != nil {
		return err
	}

	diags := content.ValidateSlowTests()
	// Print any diagnostics
	diags.Print(os.Stdout, c.flags.slow)
	if numErrs := diags.NumErrors(); numErrs > 0 {
		return fmt.Errorf("%v errors found", numErrs)
	}

	fmt.Println("no issues found")
	return nil
}
