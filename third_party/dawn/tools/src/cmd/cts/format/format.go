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

package format

import (
	"context"
	"flag"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		expectations string // expectations file path
	}
}

func (cmd) Name() string {
	return "format"
}

func (cmd) Desc() string {
	return "formats a WebGPU expectations.txt file"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	defaultExpectations := []string{
		common.DefaultExpectationsPath(),
		common.DefaultCompatExpectationsPath(),
	}
	flag.StringVar(&c.flags.expectations, "expectations", strings.Join(defaultExpectations, ","), "comma separated path to CTS expectation files to update")
	return nil, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	for _, path := range strings.Split(c.flags.expectations, ",") {
		path := strings.TrimSpace(path)

		ex, err := expectations.Load(path)
		if err != nil {
			return err
		}

		ex.Format()

		if err := ex.Save(path); err != nil {
			return err
		}
	}
	return nil
}
