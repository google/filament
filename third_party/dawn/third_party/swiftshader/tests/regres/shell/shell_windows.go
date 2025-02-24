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

package shell

import (
	"bytes"
	"os/exec"
	"time"
)

// Exec runs the executable exe with the given arguments, in the working
// directory wd, with the custom environment flags.
// If the process does not finish within timeout a errTimeout will be returned.
func Exec(timeout time.Duration, exe, wd string, env []string, toStdin string, args ...string) ([]byte, error) {
	stdin := &bytes.Buffer{}
	stdin.WriteString(toStdin)

	b := bytes.Buffer{}
	c := exec.Command(exe, args...)
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
		c.Process.Kill()
		return b.Bytes(), ErrTimeout{exe, timeout}
	case err := <-res:
		return b.Bytes(), err
	}
}
