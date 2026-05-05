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

// Contains implementations of ExecWrapper interfaces for testing environments
package execwrapper

import (
	"bytes"
	"context"
	"io"
)

// TestExecWrapper is an implementation of the ExecWrapper interface for use in
// testing.
type TestExecWrapper struct {
	// stdout return for any CmdWrapper built by this instance.
	Stdout []byte
	// stderr return for any CmdWrapper built by this instance.
	Stderr []byte
	// error return for any CmdWrapper built by this instance.
	Err error
}

// NewTestExecWrapperForFailure constructs a wrapper for commands that fail
func NewTestExecWrapperForFailure(err error) *TestExecWrapper {
	return &TestExecWrapper{
		Stdout: nil,
		Stderr: nil,
		Err:    err,
	}
}

// NewTestExecWrapperForSuccess constructs a wrapper for commands that succeed
func NewTestExecWrapperForSuccess(stdout, stderr []byte) *TestExecWrapper {
	return &TestExecWrapper{
		Stdout: stdout,
		Stderr: stderr,
		Err:    nil,
	}
}

func (t *TestExecWrapper) Command(_ string, _ ...string) CmdWrapper {
	return &TestCmdWrapper{
		stdout: t.Stdout,
		stderr: t.Stderr,
		err:    t.Err,
	}
}

func (t *TestExecWrapper) CommandContext(_ context.Context, name string, args ...string) CmdWrapper {
	return t.Command(name, args...)
}

type TestCmdWrapper struct {
	stdoutWriter io.Writer
	stderrWriter io.Writer
	stdout       []byte
	stderr       []byte
	err          error
}

func (t *TestCmdWrapper) WithStdout(stdout io.Writer) CmdWrapper {
	t.stdoutWriter = stdout
	return t
}

func (t *TestCmdWrapper) WithStderr(stderr io.Writer) CmdWrapper {
	t.stderrWriter = stderr
	return t
}

func (t *TestCmdWrapper) Run() error {
	if t.stdoutWriter != nil {
		t.stdoutWriter.Write(t.stdout)
	}
	if t.stderrWriter != nil {
		t.stderrWriter.Write(t.stderr)
	}
	return t.err
}

func (t *TestCmdWrapper) RunWithCombinedOutput() ([]byte, error) {
	if t.stdoutWriter != nil || t.stderrWriter != nil {
		return nil, ErrCombinedOutputWithWriters
	}
	var b bytes.Buffer
	b.Write(t.stdout)
	b.Write(t.stderr)
	return b.Bytes(), t.err
}
