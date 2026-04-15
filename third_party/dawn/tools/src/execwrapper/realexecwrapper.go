// Copyright 2025 The Dawn & Tint Authors
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

// Contains implementations of ExecWrapper interfaces that runs live commands
package execwrapper

import (
	"context"
	"io"
	"os/exec"
)

// RealExecWrapper is an implementation of ExecWrapper that uses the os/exec package
type RealExecWrapper struct{}

func CreateRealExecWrapper() RealExecWrapper {
	return RealExecWrapper{}
}

func (e RealExecWrapper) Command(name string, args ...string) CmdWrapper {
	return &RealCmdWrapper{cmd: exec.Command(name, args...)}
}

func (e RealExecWrapper) CommandContext(ctx context.Context, name string, args ...string) CmdWrapper {
	return &RealCmdWrapper{cmd: exec.CommandContext(ctx, name, args...)}
}

// RealCmdWrapper is an implementation of CmdWrapper that wraps live exec.Cmd instances.
type RealCmdWrapper struct {
	cmd    *exec.Cmd
	stdout io.Writer
	stderr io.Writer
}

func (e *RealCmdWrapper) WithStdout(stdout io.Writer) CmdWrapper {
	return &RealCmdWrapper{
		cmd:    e.cmd,
		stdout: stdout,
		stderr: e.stderr,
	}
}

func (e *RealCmdWrapper) WithStderr(stderr io.Writer) CmdWrapper {
	return &RealCmdWrapper{
		cmd:    e.cmd,
		stdout: e.stdout,
		stderr: stderr,
	}
}

func (e *RealCmdWrapper) Run() error {
	e.cmd.Stdout = e.stdout
	e.cmd.Stderr = e.stderr
	return e.cmd.Run()
}

func (e *RealCmdWrapper) RunWithCombinedOutput() ([]byte, error) {
	if e.stdout != nil || e.stderr != nil {
		return nil, ErrCombinedOutputWithWriters
	}
	return e.cmd.CombinedOutput()
}
