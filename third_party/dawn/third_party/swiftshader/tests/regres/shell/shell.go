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

// Package shell provides functions for running sub-processes.
package shell

import (
	"fmt"
	"time"
)

// MaxProcMemory is the maximum virtual memory per child process.
// Note: This is not used on Windows, as there is no sensible way to limit
// process memory.
var MaxProcMemory uint64 = 6 * 1024 * 1024 * 1024 // 6 GiB

// Shell runs the executable exe with the given arguments, in the working
// directory wd.
// If the process does not finish within timeout a errTimeout will be returned.
func Shell(timeout time.Duration, exe, wd string, args ...string) error {
	return Env(timeout, exe, wd, nil, args...)
}

// Env runs the executable exe with the given arguments, in the working
// directory wd, with the custom env.
// If the process does not finish within timeout a errTimeout will be returned.
func Env(timeout time.Duration, exe, wd string, env []string, args ...string) error {
	if out, err := Exec(timeout, exe, wd, env, "", args...); err != nil {
		return fmt.Errorf("%s\n%w", out, err)
	}
	return nil
}

// ErrTimeout is the error returned when a process does not finish with its
// permitted time.
type ErrTimeout struct {
	process string
	timeout time.Duration
}

func (e ErrTimeout) Error() string {
	return fmt.Sprintf("'%v' did not return after %v", e.process, e.timeout)
}
