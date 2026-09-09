// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
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
	"bytes"
	"context"
	"fmt"
	"io"
	"os"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/utils"
)

// runFuzzer runs the fuzzer across t.numProcesses processes.
// The fuzzer will use t.inputs as the seed directory.
// New cases are written to t.out.
// Blocks until a fuzzer errors, or the process is interrupted.
func runFuzzer(t *taskConfig) error {
	ctx := utils.CancelOnInterruptContext(context.Background())
	ctx, cancel := context.WithCancel(ctx)
	defer cancel()

	args := generateFuzzerArgs(t)

	if t.verbose {
		fmt.Println("Using fuzzing cmd: " + t.fuzzer + " " + strings.Join(args, " "))
	}
	fmt.Println("running ", t.numProcesses, " fuzzer instances")

	errs := make(chan error, t.numProcesses)
	for i := 0; i < t.numProcesses; i++ {
		go func() {
			out := bytes.Buffer{}
			var stdout, stderr io.Writer = &out, &out
			if t.verbose || t.dump {
				stdout = io.MultiWriter(&out, os.Stdout)
				stderr = io.MultiWriter(&out, os.Stderr)
			}

			cmd := t.execWrapper.CommandContext(ctx, t.fuzzer, args...).WithStdout(stdout).WithStderr(stderr)

			if err := cmd.Run(); err != nil {
				if ctxErr := ctx.Err(); ctxErr != nil {
					errs <- ctxErr
				} else {
					errs <- fmt.Errorf("fuzzer process '%s' failed with error: %w\nOutput:\n%s", t.fuzzer, err, out.String())
				}
			} else {
				errs <- fmt.Errorf("fuzzer process '%s' unexpectedly terminated without error.\nOutput:\n%s", t.fuzzer, out.String())
			}
		}()
	}
	for err := range errs {
		return err
	}

	fmt.Println("done")
	return nil
}

// generateFuzzerArgs generates the arguments that need to be passed into the fuzzer binary call
func generateFuzzerArgs(t *taskConfig) []string {
	args := []string{t.out}

	if t.inputs != "" {
		args = append(args, t.inputs)
	}
	if t.dictionary != "" {
		args = append(args, "-dict="+t.dictionary)
	}
	if t.verbose {
		args = append(args, "--verbose")
	}
	if t.dump {
		args = append(args, "--dump")
	}
	if t.filter != "" {
		args = append(args, "--filter="+t.filter)
	}
	return args
}
