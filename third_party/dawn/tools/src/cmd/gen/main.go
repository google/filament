// Copyright 2021 The Dawn & Tint Authors
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

// gen generates code for the Tint project.
package main

import (
	"context"
	"fmt"
	"os"

	"dawn.googlesource.com/dawn/tools/src/cmd/gen/build"
	"dawn.googlesource.com/dawn/tools/src/cmd/gen/common"
	"dawn.googlesource.com/dawn/tools/src/cmd/gen/templates"
	"dawn.googlesource.com/dawn/tools/src/subcmd"

	// Register sub-commands
	_ "dawn.googlesource.com/dawn/tools/src/cmd/gen/templates"
)

func main() {
	ctx := context.Background()
	cfg := &common.Config{}
	cfg.RegisterFlags()

	if err := subcmd.Run(ctx, cfg, common.Commands()...); err != nil {
		if err != subcmd.ErrInvalidCLA {
			fmt.Fprintln(os.Stderr, err)
		}
		os.Exit(1)
	}
}

func init() {
	common.Register(&cmdAll{})
}

type cmdAll struct {
}

func (cmdAll) IsDefaultCommand() {}

func (cmdAll) Name() string {
	return "all"
}

func (cmdAll) Desc() string {
	return `all runs all the generators`
}

func (c *cmdAll) RegisterFlags(ctx context.Context, cfg *common.Config) ([]string, error) {
	return nil, nil
}

func (c cmdAll) Run(ctx context.Context, cfg *common.Config) error {
	templatesCmd := templates.Cmd{}
	if err := templatesCmd.Run(ctx, cfg); err != nil {
		return err
	}
	buildCmd := build.Cmd{}
	if err := buildCmd.Run(ctx, cfg); err != nil {
		return err
	}
	return nil
}
