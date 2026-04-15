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

package execwrapper_test

import (
	"bytes"
	"context"
	"errors"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/execwrapper"
	"github.com/stretchr/testify/require"
)

var errTest = errors.New("test error")

func TestTestCmdWrapper_Run(t *testing.T) {
	for _, tc := range []struct {
		name       string
		useContext bool
		stdout     []byte
		stderr     []byte
		err        error
		wantStdout string
		wantStderr string
		wantErr    error
	}{
		{
			name:       "Command/NoOutput",
			useContext: false,
			stdout:     nil,
			stderr:     nil,
			err:        nil,
			wantStdout: "",
			wantStderr: "",
			wantErr:    nil,
		},
		{
			name:       "Command/Stderr",
			useContext: false,
			stdout:     nil,
			stderr:     []byte("stderr"),
			err:        nil,
			wantStdout: "",
			wantStderr: "stderr",
			wantErr:    nil,
		},
		{
			name:       "Command/Stdout",
			useContext: false,
			stdout:     []byte("stdout"),
			stderr:     nil,
			err:        nil,
			wantStdout: "stdout",
			wantStderr: "",
			wantErr:    nil,
		},
		{
			name:       "Command/Error",
			useContext: false,
			stdout:     nil,
			stderr:     nil,
			err:        errTest,
			wantStdout: "",
			wantStderr: "",
			wantErr:    errTest,
		},
		{
			name:       "CommandContext/NoOutput",
			useContext: true,
			stdout:     nil,
			stderr:     nil,
			err:        nil,
			wantStdout: "",
			wantStderr: "",
			wantErr:    nil,
		},
		{
			name:       "CommandContext/Stderr",
			useContext: true,
			stdout:     nil,
			stderr:     []byte("stderr"),
			err:        nil,
			wantStdout: "",
			wantStderr: "stderr",
			wantErr:    nil,
		},
		{
			name:       "CommandContext/Stdout",
			useContext: true,
			stdout:     []byte("stdout"),
			stderr:     nil,
			err:        nil,
			wantStdout: "stdout",
			wantStderr: "",
			wantErr:    nil,
		},
		{
			name:       "CommandContext/Error",
			useContext: true,
			stdout:     nil,
			stderr:     nil,
			err:        errTest,
			wantStdout: "",
			wantStderr: "",
			wantErr:    errTest,
		},
	} {
		tc := tc
		t.Run(tc.name, func(t *testing.T) {
			var ew execwrapper.ExecWrapper
			if tc.err != nil {
				ew = execwrapper.NewTestExecWrapperForFailure(tc.err)
			} else {
				ew = execwrapper.NewTestExecWrapperForSuccess(tc.stdout, tc.stderr)
			}

			var cmd execwrapper.CmdWrapper
			if tc.useContext {
				cmd = ew.CommandContext(context.Background(), "test")
			} else {
				cmd = ew.Command("test")
			}

			var stdout, stderr bytes.Buffer
			cmd.WithStdout(&stdout).WithStderr(&stderr)

			err := cmd.Run()

			if tc.wantErr != nil {
				require.ErrorIs(t, err, tc.wantErr)
			} else {
				require.NoError(t, err)
				require.Equal(t, tc.wantStdout, stdout.String())
				require.Equal(t, tc.wantStderr, stderr.String())
			}
		})
	}
}

func TestTestCmdWrapper_RunCombinedOutput(t *testing.T) {
	for _, tc := range []struct {
		name       string
		useContext bool
		stdout     []byte
		stderr     []byte
		err        error
		withStdout bool
		withStderr bool
		wantOutput string
		wantErr    error
	}{
		{
			name:       "Command/Success",
			useContext: false,
			stdout:     []byte("stdout"),
			stderr:     []byte("stderr"),
			err:        nil,
			wantOutput: "stdoutstderr",
		},
		{
			name:       "Command/NoOutput",
			useContext: false,
			wantOutput: "",
		},
		{
			name:       "Command/Error",
			useContext: false,
			stdout:     []byte("stdout"),
			stderr:     []byte("stderr"),
			err:        errTest,
			wantErr:    errTest,
		},
		{
			name:       "Command/WithStdout",
			useContext: false,
			withStdout: true,
			wantErr:    execwrapper.ErrCombinedOutputWithWriters,
		},
		{
			name:       "Command/WithStderr",
			useContext: false,
			withStderr: true,
			wantErr:    execwrapper.ErrCombinedOutputWithWriters,
		},
		{
			name:       "CommandContext/Success",
			useContext: true,
			stdout:     []byte("stdout"),
			stderr:     []byte("stderr"),
			err:        nil,
			wantOutput: "stdoutstderr",
		},
		{
			name:       "CommandContext/NoOutput",
			useContext: true,
			wantOutput: "",
		},
		{
			name:       "CommandContext/Error",
			useContext: true,
			stdout:     []byte("stdout"),
			stderr:     []byte("stderr"),
			err:        errTest,
			wantErr:    errTest,
		},
		{
			name:       "CommandContext/WithStdout",
			useContext: true,
			withStdout: true,
			wantErr:    execwrapper.ErrCombinedOutputWithWriters,
		},
		{
			name:       "CommandContext/WithStderr",
			useContext: true,
			withStderr: true,
			wantErr:    execwrapper.ErrCombinedOutputWithWriters,
		},
	} {
		tc := tc
		t.Run(tc.name, func(t *testing.T) {
			var ew execwrapper.ExecWrapper
			if tc.err != nil {
				ew = execwrapper.NewTestExecWrapperForFailure(tc.err)
			} else {
				ew = execwrapper.NewTestExecWrapperForSuccess(tc.stdout, tc.stderr)
			}

			var cmd execwrapper.CmdWrapper
			if tc.useContext {
				cmd = ew.CommandContext(context.Background(), "test")
			} else {
				cmd = ew.Command("test")
			}

			if tc.withStdout {
				cmd.WithStdout(&bytes.Buffer{})
			}
			if tc.withStderr {
				cmd.WithStderr(&bytes.Buffer{})
			}

			out, err := cmd.RunWithCombinedOutput()

			if tc.wantErr != nil {
				require.ErrorIs(t, err, tc.wantErr)
			} else {
				require.NoError(t, err)
			}

			if tc.withStdout || tc.withStderr {
				require.Nil(t, out)
			} else {
				require.Equal(t, tc.wantOutput, string(out))
			}
		})
	}
}
