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
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
	"dawn.googlesource.com/dawn/tools/src/utils"
)

// runTestCasesWithServers spawns c.flags.NumRunners server instances to run all
// the test cases in testCases. The results of the tests are streamed to results.
// Blocks until all the tests have been run.
func (c *cmd) runTestCasesWithServers(ctx context.Context, testCases []common.TestCase, results chan<- common.Result) {
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
		id := i
		wg.Add(1)
		go func() {
			defer wg.Done()
			if err := c.runServer(ctx, id, testCases, testCaseIndices, results); err != nil {
				results <- common.Result{
					Status: common.Fail,
					Error:  fmt.Errorf("Test server error: %w", err),
				}
			}
		}()
	}

	wg.Wait()
}

// runServer starts a test runner server instance, takes case indices from
// testCaseIndices, and requests the server run the test with the given index.
// The result of the test run is written to the results chan.
// Once the testCaseIndices chan has been closed and all taken tests have been
// completed, the server process is shutdown and runServer returns.
func (c *cmd) runServer(
	ctx context.Context,
	id int,
	testCases []common.TestCase,
	testCaseIndices <-chan int,
	results chan<- common.Result) error {

	var port int
	testCaseLog := &bytes.Buffer{}

	stopServer := func() {}
	startServer := func() error {
		args := []string{
			"-e", "require('./out-node/common/runtime/server.js');",
			"--",
			// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
			// and slices away the first two arguments. When running with '-e', args
			// start at 1, so just inject a placeholder argument.
			"placeholder-arg",
			// Actual arguments begin here
			"--gpu-provider", filepath.Join(c.flags.bin, "cts.js"),
		}
		if c.flags.Colors {
			args = append(args, "--colors")
		}
		if c.coverage != nil {
			args = append(args, "--coverage")
		}
		if c.flags.Verbose {
			args = append(args,
				"--verbose",
				"--gpu-provider-flag", "verbose=1")
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
		if c.flags.debugCTS {
			args = append(args, "--debug")
		}
		for _, f := range c.flags.dawn {
			args = append(args, "--gpu-provider-flag", f)
		}

		cmd := exec.CommandContext(ctx, c.flags.Node, args...)

		writer := io.Writer(testCaseLog)
		if c.flags.Verbose {
			pw := &utils.PrefixWriter{
				Prefix: fmt.Sprintf("[%d] ", id),
				Writer: os.Stdout,
			}
			writer = io.MultiWriter(pw, writer)
		}

		pl := newPortListener(writer)

		cmd.Dir = c.flags.CTS
		cmd.Stdout = &pl
		cmd.Stderr = &pl

		if c.flags.Verbose {
			PrintCommand(cmd, c.flags.skipVSCodeInfo)
		}
		err := cmd.Start()
		if err != nil {
			return fmt.Errorf("failed to start test runner server: %v", err)
		}

		select {
		case port = <-pl.port:
			break // success
		case <-time.After(time.Second * 10):
			return fmt.Errorf("timeout waiting for server port:\n%v", pl.buffer.String())
		case <-ctx.Done(): // cancelled
			return ctx.Err()
		}

		// Load the cases
		postResp, postErr := http.Post(fmt.Sprintf("http://localhost:%v/load?%v", port, c.query), "", &bytes.Buffer{})
		if postErr != nil || postResp.StatusCode != http.StatusOK {
			msg := &strings.Builder{}
			fmt.Println(msg, "failed to load test cases: ", postErr)
			if body, err := io.ReadAll(postResp.Body); err == nil {
				fmt.Println(msg, string(body))
			} else {
				fmt.Println(msg, err)
			}
			return fmt.Errorf("%v", msg.String())
		}

		return nil
	}
	stopServer = func() {
		if port > 0 {
			go http.Post(fmt.Sprintf("http://localhost:%v/terminate", port), "", &bytes.Buffer{})
			time.Sleep(time.Millisecond * 100)
			port = 0
		}
	}

	for idx := range testCaseIndices {
		testCaseLog.Reset() // Clear the log for this test case

		if port == 0 {
			if err := startServer(); err != nil {
				return err
			}
		}

		res := common.Result{Index: idx, TestCase: testCases[idx]}

		type Response struct {
			Status       string
			Message      string
			CoverageData string
			DurationMS   float32
		}
		postResp, err := http.Post(fmt.Sprintf("http://localhost:%v/run?%v", port, testCases[idx]), "", &bytes.Buffer{})
		if err != nil {
			res.Error = fmt.Errorf("server POST failure. Restarting server... This can happen when there is a crash. Try running with --isolate.")
			res.Status = common.Fail
			results <- res
			stopServer()
			continue
		}

		if postResp.StatusCode == http.StatusOK {
			var resp Response
			if err := json.NewDecoder(postResp.Body).Decode(&resp); err != nil {
				res.Error = fmt.Errorf("server response decode failure")
				res.Status = common.Fail
				results <- res
				continue
			}

			res.Duration = time.Duration(resp.DurationMS*1000) * time.Microsecond

			switch resp.Status {
			case "pass":
				res.Status = common.Pass
				res.Message = resp.Message + testCaseLog.String()
			case "warn":
				res.Status = common.Warn
				res.Message = resp.Message + testCaseLog.String()
			case "fail":
				res.Status = common.Fail
				res.Message = resp.Message + testCaseLog.String()
			case "skip":
				res.Status = common.Skip
				res.Message = resp.Message + testCaseLog.String()
			default:
				res.Status = common.Fail
				res.Error = fmt.Errorf("unknown status: '%v'", resp.Status)
			}

			if resp.CoverageData != "" {
				coverage, covErr := c.coverage.Env.Import(resp.CoverageData)
				os.Remove(resp.CoverageData)
				if covErr != nil {
					if res.Message != "" {
						res.Message += "\n"
					}
					res.Message += fmt.Sprintf("could not import coverage data from '%v': %v", resp.CoverageData, covErr)
				}
				res.Coverage = coverage
			}
		} else {
			msg, err := io.ReadAll(postResp.Body)
			if err != nil {
				msg = []byte(err.Error())
			}
			res.Status = common.Fail
			res.Error = fmt.Errorf("server error: %v", string(msg))
		}
		results <- res
	}

	stopServer()
	return nil
}

// portListener implements io.Writer, monitoring written messages until a port
// is printed between '[[' ']]'. Once the port has been found, the parsed
// port number is written to the 'port' chan, and all subsequent writes are
// forwarded to writer.
type portListener struct {
	writer io.Writer
	buffer strings.Builder
	port   chan int
}

func newPortListener(w io.Writer) portListener {
	return portListener{w, strings.Builder{}, make(chan int)}
}

func (p *portListener) Write(data []byte) (n int, err error) {
	if p.port != nil {
		p.buffer.Write(data)
		str := p.buffer.String()

		idx := strings.Index(str, "[[")
		if idx < 0 {
			// Still waiting for the opening '[['
			return len(data), nil
		}

		str = str[idx+2:] // skip past '[['
		idx = strings.Index(str, "]]")
		if idx < 0 {
			// Still waiting for the closing ']]'
			return len(data), nil
		}

		port, err := strconv.Atoi(str[:idx])
		if err != nil {
			return 0, err
		}

		// Port found. Write it to the chan, and close the chan.
		p.port <- port
		close(p.port)
		p.port = nil

		str = strings.TrimRight(str[idx+2:], " \n")
		if len(str) == 0 {
			return len(data), nil
		}
		// Write out trailing text after the ']]'
		return p.writer.Write([]byte(str))
	}

	// Port has been found. Just forward the rest of the data to p.writer
	return p.writer.Write(data)
}
