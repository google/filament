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

// Package oswrapper provides interfaces for the exec package.
// This facilitates better testing via dependency injection.
package execwrapper

import (
	"context"
	"errors"
	"io"
)

var (
	ErrCombinedOutputWithWriters = errors.New("RunWithCombinedOutput cannot be called with custom writers")
)

// ExecWrapper is a factory for dependency injection wrappers for calls to exec
type ExecWrapper interface {
	// Command is a builder for CmdWrappers without a provided Context
	Command(name string, args ...string) CmdWrapper
	// CommandContext is a builder for CmdWrappers with a provided Context
	CommandContext(ctx context.Context, name string, args ...string) CmdWrapper
}

// CmdWrapper is a dependency injection wrapper around calls to exec
type CmdWrapper interface {
	// WithStdout returns a new CmdWrapper with the output stream set.
	WithStdout(stdout io.Writer) CmdWrapper
	// WithStderr returns a new CmdWrapper with the error stream set.
	WithStderr(stderr io.Writer) CmdWrapper
	// Run executes the Cmd.
	Run() error
	// RunWithCombinedOutput executes the Cmd and returns its combined output. Calling this with stdout or stderr set return ErrCombinedOutputWithWriters.
	RunWithCombinedOutput() ([]byte, error)
}
