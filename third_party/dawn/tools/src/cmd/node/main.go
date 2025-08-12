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

package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
	"dawn.googlesource.com/dawn/tools/src/dawn/node"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

// main entry point
func main() {
	wrapper := oswrapper.GetRealOSWrapper()
	nodeFlags := node.Flags{}
	opts := node.Options{
		AllowUnsafeAPIs: true,
	}

	var nodePath string
	lldb := false

	flag.Usage = func() {
		out := flag.CommandLine.Output()
		fmt.Fprintf(out, "node runs a .js file with 'navigator.gpu' provided by dawn/node.\n\n")
		flag.PrintDefaults()
	}

	flag.StringVar(&nodePath, "node", fileutils.NodePath(wrapper), "path to node executable")
	flag.Var(&nodeFlags, "flag", "flag to pass to dawn-node as flag=value. multiple flags must be passed in individually")
	flag.StringVar(&opts.BinDir, "bin", fileutils.BuildPath(wrapper), "path to the directory holding cts.js and dawn.node")
	flag.StringVar(&opts.Backend, "backend", "default", "backend to use: default|null|webgpu|d3d11|d3d12|metal|vulkan|opengl|opengles."+
		" set to 'vulkan' if VK_ICD_FILENAMES environment variable is set, 'default' otherwise")
	flag.StringVar(&opts.Adapter, "adapter", "", "name (or substring) of the GPU adapter to use")
	flag.BoolVar(&opts.Validate, "validate", false, "enable backend validation")
	flag.BoolVar(&opts.DumpShaders, "dump-shaders", false, "dump WGSL shaders. Enables --verbose")
	flag.BoolVar(&opts.DumpShadersPretty, "dump-shaders-pretty", false, "dump WGSL shaders, but don't run symbol renaming. May fail tests that shadow predeclared builtins. Enables --verbose")
	flag.BoolVar(&opts.UseFXC, "fxc", false, "Use FXC instead of DXC. Disables 'use_dxc' Dawn flag")
	flag.BoolVar(&lldb, "lldb", false, "launch node via lldb")
	flag.Parse()

	nodeFlags.SetOptions(opts)

	debugger := ""
	if lldb {
		debugger = "lldb"
	}

	if err := run(opts.BinDir, nodePath, nodeFlags, flag.Args(), debugger, wrapper); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

// run starts the
func run(binPath, nodePath string, flags node.Flags, args []string, debugger string, fsReader oswrapper.FilesystemReader) error {
	if len(args) == 0 {
		return fmt.Errorf("missing path to .js file")
	}

	scriptPath, err := filepath.Abs(args[0])
	if err != nil {
		return err
	}

	// Find node
	if nodePath == "" {
		return fmt.Errorf("cannot find path to node. Specify with --node")
	}

	for _, file := range []string{"cts.js", "dawn.node"} {
		if !fileutils.IsFile(filepath.Join(binPath, file), fsReader) {
			return fmt.Errorf("'%v' does not contain '%v'", binPath, file)
		}
	}

	ctsJS := filepath.Join(binPath, "cts.js")

	ctx := context.Background()
	if debugger == "" {
		timeoutCtx, cancel := context.WithTimeout(context.Background(), common.TestCaseTimeout)
		defer cancel()
		ctx = timeoutCtx
	}

	quotedFlags := make([]string, len(flags))
	for i, f := range flags {
		quotedFlags[i] = fmt.Sprintf("'%v'", f)
	}

	args = []string{
		"-e",
		fmt.Sprintf("const { create } = require('%v'); navigator = { gpu: create([%v]) }; require('%v')", ctsJS, strings.Join(quotedFlags, ", "), scriptPath),
	}

	exe := nodePath

	if debugger != "" {
		args = append([]string{"--", exe}, args...)
		exe, err = exec.LookPath(debugger)
		if err != nil {
			return err
		}
	}

	cmd := exec.CommandContext(ctx, exe, args...)
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}
