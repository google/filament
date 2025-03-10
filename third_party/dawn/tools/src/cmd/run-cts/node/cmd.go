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

package node

import (
	"context"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
	"dawn.googlesource.com/dawn/tools/src/cov"
	"dawn.googlesource.com/dawn/tools/src/dawn/node"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

type flags struct {
	common.Flags
	bin                  string
	backend              string
	adapterName          string
	coverageFile         string
	isolated             bool
	build                bool
	validate             bool
	dumpShaders          bool
	dumpShadersPretty    bool
	fxc                  bool
	useIR                bool
	unrollConstEvalLoops bool
	genCoverage          bool
	compatibilityMode    bool
	debugCTS             bool
	skipVSCodeInfo       bool
	enforceDefaultLimits bool
	dawn                 node.Flags
}

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	state    common.State
	flags    flags
	coverage *common.Coverage
	query    string
}

func (cmd) IsDefaultCommand() {}

func (cmd) Name() string {
	return "node"
}

func (cmd) Desc() string {
	return "runs the CTS with dawn.node"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	unrollConstEvalLoopsDefault := runtime.GOOS != "windows"

	backendDefault := "default"
	if vkIcdFilenames := os.Getenv("VK_ICD_FILENAMES"); vkIcdFilenames != "" {
		backendDefault = "vulkan"
	}

	c.flags.Flags.Register()
	flag.StringVar(&c.flags.bin, "bin", fileutils.BuildPath(), "path to the directory holding cts.js and dawn.node")
	flag.BoolVar(&c.flags.isolated, "isolate", false, "run each test in an isolated process")
	flag.BoolVar(&c.flags.build, "build", true, "attempt to build the CTS before running")
	flag.BoolVar(&c.flags.validate, "validate", false, "enable backend validation")
	flag.Var(&c.flags.dawn, "flag", "flag to pass to dawn-node as flag=value. multiple flags must be passed in individually")
	flag.StringVar(&c.flags.backend, "backend", backendDefault, "backend to use: default|null|webgpu|d3d11|d3d12|metal|vulkan|opengl|opengles."+
		" set to 'vulkan' if VK_ICD_FILENAMES environment variable is set, 'default' otherwise")
	flag.StringVar(&c.flags.adapterName, "adapter", "", "name (or substring) of the GPU adapter to use")
	flag.BoolVar(&c.flags.dumpShaders, "dump-shaders", false, "dump WGSL shaders. Enables --verbose")
	flag.BoolVar(&c.flags.dumpShadersPretty, "dump-shaders-pretty", false, "dump WGSL shaders, but don't run symbol renaming. May fail tests that shadow predeclared builtins. Enables --verbose")
	flag.BoolVar(&c.flags.fxc, "fxc", false, "Use FXC instead of DXC. Disables 'use_dxc' Dawn flag")
	flag.BoolVar(&c.flags.useIR, "use-ir", false, "Use Tint's IR generator code path")
	flag.BoolVar(&c.flags.unrollConstEvalLoops, "unroll-const-eval-loops", unrollConstEvalLoopsDefault, "unroll loops in const-eval tests")
	flag.BoolVar(&c.flags.genCoverage, "coverage", false, "displays coverage data")
	flag.StringVar(&c.flags.coverageFile, "export-coverage", "", "write coverage data to the given path")
	flag.BoolVar(&c.flags.compatibilityMode, "compat", false, "run tests in compatibility mode")
	flag.BoolVar(&c.flags.debugCTS, "debug-cts", false, "enable CTS debugging option")
	flag.BoolVar(&c.flags.skipVSCodeInfo, "skip-vs-code-info", false, "skips emitting VS Code information")
	flag.BoolVar(&c.flags.enforceDefaultLimits, "enforce-default-limits", false, "enforce the default limits (note: powerPreference tests may fail)")

	return []string{"[query]"}, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	// Process the command line flags
	if err := c.processFlags(); err != nil {
		return err
	}

	if err := c.maybeInitCoverage(); err != nil {
		return err
	}

	if err := c.state.CTS.Node.BuildIfRequired(c.flags.Verbose); err != nil {
		return err
	}

	// Find all the test cases that match r.query.
	testCases, err := c.state.CTS.QueryTestCases(c.flags.Verbose, c.query)
	if err != nil {
		return fmt.Errorf("failed to gather tests: %w", err)
	}
	fmt.Printf("Testing %d test cases...\n", len(testCases))

	var runner func(ctx context.Context, testCases []common.TestCase, results chan<- common.Result)
	if c.flags.isolated {
		fmt.Println("Running in parallel isolated...")
		runner = c.runTestCasesWithCmdline
	} else {
		fmt.Println("Running in parallel with server...")
		runner = c.runTestCasesWithServers
	}

	resultStream := make(chan common.Result, 256)
	go func() {
		runner(ctx, testCases, resultStream)
		close(resultStream)
	}()

	results, err := common.StreamResults(ctx,
		c.flags.Colors,
		c.state,
		c.flags.Verbose,
		c.coverage,
		len(testCases),
		resultStream)
	if err != nil {
		return err
	}

	if err := c.state.Close(results); err != nil {
		return err
	}

	return nil
}

func (c *cmd) processFlags() error {
	// Check mandatory arguments
	if c.flags.bin == "" {
		return fmt.Errorf("-bin is not set. It defaults to <dawn>/out/active (%v) which does not exist", filepath.Join(fileutils.DawnRoot(), "out/active"))
	}
	if !fileutils.IsDir(c.flags.bin) {
		return fmt.Errorf("'%v' is not a directory", c.flags.bin)
	}
	for _, file := range []string{"cts.js", "dawn.node"} {
		if !fileutils.IsFile(filepath.Join(c.flags.bin, file)) {
			return fmt.Errorf("'%v' does not contain '%v'", c.flags.bin, file)
		}
	}

	// Make paths absolute
	absBin, err := filepath.Abs(c.flags.bin)
	if err != nil {
		return fmt.Errorf("unable to get absolute path for '%v'", c.flags.bin)
	}
	c.flags.bin = absBin

	// The test query is the optional unnamed argument
	c.query = "webgpu:*"
	switch len(flag.Args()) {
	case 0:
	case 1:
		c.query = flag.Args()[0]
	default:
		return fmt.Errorf("only a single query can be provided")
	}

	c.flags.dawn.SetOptions(node.Options{
		BinDir:            c.flags.bin,
		Backend:           c.flags.backend,
		Adapter:           c.flags.adapterName,
		Validate:          c.flags.validate,
		AllowUnsafeAPIs:   true,
		DumpShaders:       c.flags.dumpShaders,
		DumpShadersPretty: c.flags.dumpShadersPretty,
		UseFXC:            c.flags.fxc,
		UseIR:             c.flags.useIR,
	})

	if c.flags.dumpShaders {
		c.flags.Verbose = true
	}
	if c.flags.dumpShadersPretty {
		c.flags.Verbose = true
	}

	state, err := c.flags.Process()
	if err != nil {
		return err
	}
	c.state = *state

	return nil
}

func (c *cmd) maybeInitCoverage() error {
	if !c.flags.genCoverage && c.flags.coverageFile == "" {
		return nil
	}

	profdata, err := exec.LookPath("llvm-profdata")
	if err != nil {
		profdata = ""
		if runtime.GOOS == "darwin" {
			profdata = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/llvm-profdata"
			if !fileutils.IsExe(profdata) {
				profdata = ""
			}
		}
	}
	if profdata == "" {
		return fmt.Errorf("failed to find llvm-profdata, required for --coverage")
	}

	llvmCov := ""
	turboCov := filepath.Join(c.flags.bin, "turbo-cov"+fileutils.ExeExt)
	if !fileutils.IsExe(turboCov) {
		turboCov = ""
		if path, err := exec.LookPath("llvm-cov"); err == nil {
			llvmCov = path
		} else {
			return fmt.Errorf("failed to find turbo-cov or llvm-cov")
		}
	}
	c.coverage = &common.Coverage{
		OutputFile: c.flags.coverageFile,
		Env: &cov.Env{
			Profdata: profdata,
			Binary:   filepath.Join(c.flags.bin, "dawn.node"),
			Cov:      llvmCov,
			TurboCov: turboCov,
		},
	}

	return nil
}
