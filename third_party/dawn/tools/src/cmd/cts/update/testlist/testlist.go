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

package testlist

import (
	"context"
	"flag"
	"os"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
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
		ctsDir   string
		nodePath string
		output   string
	}
}

func (cmd) Name() string {
	return "update-testlist"
}

func (cmd) Desc() string {
	return "updates a CTS test list file"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	flag.StringVar(&c.flags.ctsDir, "cts", common.DefaultCTSPath(), "path to the CTS")
	flag.StringVar(&c.flags.nodePath, "node", fileutils.NodePath(), "path to node")
	flag.StringVar(&c.flags.output, "out", common.DefaultTestListPath(), "output testlist path")
	return nil, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	list, err := common.GenTestList(ctx, c.flags.ctsDir, c.flags.nodePath)
	if err != nil {
		return err
	}

	return os.WriteFile(c.flags.output, []byte(list), 0666)
}
