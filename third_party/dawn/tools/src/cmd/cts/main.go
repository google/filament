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

// cts is a collection of sub-commands for operating on the WebGPU CTS.
//
// To view available commands run: '<dawn>/tools/run cts --help'
package main

import (
	"context"
	"fmt"
	"os"
	"path/filepath"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/resultsdb"
	"dawn.googlesource.com/dawn/tools/src/subcmd"

	// Register sub-commands
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/credentialscheck"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/expectationcoverage"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/export"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/format"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/merge"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/results"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/roll"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/time"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/treemap"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/update/expectations"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/update/testlist"
	_ "dawn.googlesource.com/dawn/tools/src/cmd/cts/validate"
)

func main() {
	ctx := context.Background()

	cfg, err := common.LoadConfig(filepath.Join(fileutils.ThisDir(), "config.json"))
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	cfg.OsWrapper = oswrapper.GetRealOSWrapper()
	cfg.Querier, err = resultsdb.NewBigQueryClient(ctx, resultsdb.DefaultQueryProject)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	if err := subcmd.Run(ctx, *cfg, common.Commands()...); err != nil {
		if err != subcmd.ErrInvalidCLA {
			fmt.Fprintln(os.Stderr, err)
		}
		os.Exit(1)
	}
}
