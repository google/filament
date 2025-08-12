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
	"bytes"
	"context"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

// TODO(crbug.com/416755658): Add unittest coverage when there is a way to fake
// the node process.
// runTestCasesWithCmdline() calls the CTS command-line test runner to run each
// test case in a separate process. This reduces possibility of state leakage
// between tests.
// Up to c.flags.numRunners tests will be run concurrently.
func (c *cmd) runTestCasesWithCmdline(
	ctx context.Context, testCases []common.TestCase, results chan<- common.Result, fsReader oswrapper.FilesystemReader) {
	// Create a chan of test indices.
	// This will be read by the test runner goroutines.
	testCaseIndices := make(chan int, 256)
	go func() {
		for i := range testCases {
			testCaseIndices <- i
		}
		close(testCaseIndices)
	}()

	// Spin up the test runner goroutines
	wg := &sync.WaitGroup{}
	for i := 0; i < c.flags.NumRunners; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for idx := range testCaseIndices {
				res := c.runTestCaseWithCmdline(ctx, testCases[idx], fsReader)
				res.Index = idx
				results <- res
				if err := ctx.Err(); err != nil {
					return
				}
			}
		}()
	}
	wg.Wait()
}

// TODO(crbug.com/416755658): Add unittest coverage when exec is handled via
// dependency injection.
// runTestCaseWithCmdline() runs a single CTS test case with the command line tool,
// returning the test result.
func (c *cmd) runTestCaseWithCmdline(ctx context.Context, testCase common.TestCase, fsReader oswrapper.FilesystemReader) common.Result {
	ctx, cancel := context.WithTimeout(ctx, common.TestCaseTimeout)
	defer cancel()

	args := []string{
		"-e", "require('./out-node/common/runtime/cmdline.js');",
		"--",
		// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
		// and slices away the first two arguments. When running with '-e', args
		// start at 1, so just inject a placeholder argument.
		"placeholder-arg",
		// Actual arguments begin here
		"--gpu-provider", filepath.Join(c.flags.bin, "cts.js"),
		"--verbose", // always required to emit test pass results
		"--quiet",
	}
	if c.flags.Verbose {
		args = append(args, "--gpu-provider-flag", "verbose=1")
	}
	if c.coverage != nil {
		args = append(args, "--coverage")
	}
	if c.flags.Colors {
		args = append(args, "--colors")
	}
	if c.flags.unrollConstEvalLoops {
		args = append(args, "--unroll-const-eval-loops")
	}
	if c.flags.compatibilityMode {
		args = append(args, "--compat")
	}
	if c.flags.enforceDefaultLimits {
		args = append(args, "--enforce-default-limits")
	}
	if c.flags.blockAllFeatures {
		args = append(args, "--block-all-features")
	}
	for _, f := range c.flags.dawn {
		args = append(args, "--gpu-provider-flag", f)
	}
	args = append(args, string(testCase))

	cmd := exec.CommandContext(ctx, c.flags.Node, args...)
	cmd.Dir = c.flags.CTS

	var buf bytes.Buffer
	cmd.Stdout = &buf
	cmd.Stderr = &buf

	if c.flags.Verbose {
		PrintCommand(cmd, c.flags.skipVSCodeInfo)
	}

	start := time.Now()
	err := cmd.Run()

	msg := buf.String()
	res := common.Result{
		TestCase: testCase,
		Status:   common.Pass,
		Message:  msg,
		Error:    err,
		Duration: time.Since(start),
	}

	if err == nil && c.coverage != nil {
		const header = "Code-coverage: [["
		const footer = "]]"
		if headerStart := strings.Index(msg, header); headerStart >= 0 {
			if footerStart := strings.Index(msg[headerStart:], footer); footerStart >= 0 {
				footerStart += headerStart
				path := msg[headerStart+len(header) : footerStart]
				res.Message = msg[:headerStart] + msg[footerStart+len(footer):] // Strip out the coverage from the message
				coverage, covErr := c.coverage.Env.Import(path, fsReader)
				os.Remove(path)
				if covErr == nil {
					res.Coverage = coverage
				} else {
					err = fmt.Errorf("could not import coverage data from '%v': %v", path, covErr)
				}
			}
		}
		if err == nil && res.Coverage == nil {
			err = fmt.Errorf("failed to parse code coverage from output")
		}
	}

	switch {
	case errors.Is(err, context.DeadlineExceeded):
		res.Status = common.Timeout
	case err != nil, strings.Contains(msg, "[fail]"):
		res.Status = common.Fail
	case strings.Contains(msg, "[warn]"):
		res.Status = common.Warn
	case strings.Contains(msg, "[skip]"):
		res.Status = common.Skip
	case strings.Contains(msg, "[pass]"):
		break
	default:
		res.Status = common.Fail
		msg += "\ncould not parse test output"
	}

	if res.Error != nil {
		res.Message = fmt.Sprint(res.Message, res.Error)
	}
	return res
}
