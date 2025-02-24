// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//go:build darwin || linux
// +build darwin linux

package shell

import (
	"bytes"
	"fmt"
	"log"
	"os"
	"os/exec"
	"os/signal"
	"strconv"
	"syscall"
	"time"
)

func init() {
	// As we are going to be running a number of tests concurrently, we need to
	// limit the amount of virtual memory each test uses, otherwise memory
	// hungry tests can bring the whole system down into a swapping apocalypse.
	//
	// Linux has the setrlimit() function to limit a process (and child's)
	// virtual memory usage - but we cannot call this from the regres process
	// as this process may need more memory than the limit allows.
	//
	// Unfortunately golang has no native support for setting rlimits for child
	// processes (https://github.com/golang/go/issues/6603), so we instead wrap
	// the exec to the test executable with another child regres process using a
	// special --exec mode:
	//
	// [regres] -> [regres --exec <test-exe N args...>] -> [test-exe]
	//               ^^^^
	//          (calls rlimit() with memory limit of N bytes)

	if len(os.Args) > 3 && os.Args[1] == "--exec" {
		exe := os.Args[2]
		limit, err := strconv.ParseUint(os.Args[3], 10, 64)
		if err != nil {
			log.Fatalf("Expected memory limit as 3rd argument. %v\n", err)
		}
		if limit > 0 {
			if err := syscall.Setrlimit(syscall.RLIMIT_AS, &syscall.Rlimit{Cur: limit, Max: limit}); err != nil {
				log.Fatalln(fmt.Errorf("Setrlimit: %w", err))
			}
		}
		cmd := exec.Command(exe, os.Args[4:]...)
		cmd.Stdin = os.Stdin
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		if err := cmd.Start(); err != nil {
			os.Stderr.WriteString(err.Error())
			os.Exit(1)
		}
		// Forward signals to the child process
		c := make(chan os.Signal, 1)
		signal.Notify(c, os.Interrupt)
		go func() {
			for sig := range c {
				cmd.Process.Signal(sig)
			}
		}()
		cmd.Wait()
		close(c)
		os.Exit(cmd.ProcessState.ExitCode())
	}
}

// Exec runs the executable exe with the given arguments, in the working
// directory wd, with the custom environment flags.
// If the process does not finish within timeout a errTimeout will be returned.
func Exec(timeout time.Duration, exe, wd string, env []string, toStdin string, args ...string) ([]byte, error) {
	stdin := &bytes.Buffer{}
	stdin.WriteString(toStdin)

	// Shell via regres: --exec N <exe> <args...>
	// See main() for details.
	args = append([]string{"--exec", exe, fmt.Sprintf("%v", MaxProcMemory)}, args...)
	b := bytes.Buffer{}
	c := exec.Command(os.Args[0], args...)
	c.Dir = wd
	c.Env = env
	c.Stdin = stdin
	c.Stdout = &b
	c.Stderr = &b

	if err := c.Start(); err != nil {
		return nil, err
	}

	res := make(chan error)
	go func() { res <- c.Wait() }()

	select {
	case <-time.NewTimer(timeout).C:
		c.Process.Signal(syscall.SIGINT)
		time.Sleep(time.Second * 3)
		if c.ProcessState == nil || !c.ProcessState.Exited() {
			log.Printf("Process %v still has not exited, killing\n", c.Process.Pid)
			syscall.Kill(-c.Process.Pid, syscall.SIGKILL)
		}
		return b.Bytes(), ErrTimeout{exe, timeout}
	case err := <-res:
		return b.Bytes(), err
	}
}
