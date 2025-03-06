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

// run-parallel is a tool to run an executable with the provided templated
// arguments across all the hardware threads.
package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
	"runtime"
	"strings"
	"sync"
)

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
run-parallel is a tool to run an executable with the provided templated
arguments across all the hardware threads.

Usage:
  run-parallel <executable> [arguments...] -- [per-instance-value...]

  executable         - the path to the executable to run.
  arguments          - a list of arguments to pass to the executable.
                       Any occurrance of $ will be substituted with the
                       per-instance-value for the given invocation.
  per-instance-value - a list of values. The executable will be invoked for each
                       value in this list.`)
	os.Exit(1)
}

func run() error {
	onlyPrintFailures := flag.Bool("only-print-failures", false, "Omit output for processes that did not fail")
	flag.Parse()

	args := flag.Args()
	if len(args) < 2 {
		showUsage()
	}
	exe := args[0]
	args = args[1:]

	var perInstanceValues []string
	for i, arg := range args {
		if arg == "--" {
			perInstanceValues = args[i+1:]
			args = args[:i]
			break
		}
	}
	if perInstanceValues == nil {
		showUsage()
	}

	taskIndices := make(chan int, 64)
	type result struct {
		cmd    string
		msg    string
		failed bool
	}
	results := make([]result, len(perInstanceValues))

	numCPU := runtime.NumCPU()
	wg := sync.WaitGroup{}
	wg.Add(numCPU)
	for i := 0; i < numCPU; i++ {
		go func() {
			defer wg.Done()
			for idx := range taskIndices {
				taskArgs := make([]string, len(args))
				for i, arg := range args {
					taskArgs[i] = strings.ReplaceAll(arg, "$", perInstanceValues[idx])
				}
				success, out := invoke(exe, taskArgs)
				if !success || !*onlyPrintFailures {
					results[idx] = result{fmt.Sprint(append([]string{exe}, taskArgs...)), out, !success}
				}
			}
		}()
	}

	for i := range perInstanceValues {
		taskIndices <- i
	}
	close(taskIndices)

	wg.Wait()

	failed := false
	for _, result := range results {
		if result.msg != "" {
			fmt.Printf("'%v' returned %v\n", result.cmd, result.msg)
		}
		failed = failed || result.failed
	}
	if failed {
		os.Exit(1)
	}
	return nil
}

func invoke(exe string, args []string) (ok bool, output string) {
	cmd := exec.Command(exe, args...)
	out, err := cmd.CombinedOutput()
	str := string(out)
	if err != nil {
		if str != "" {
			return false, str
		}
		return false, err.Error()
	}
	return true, str
}
