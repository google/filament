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

package merge

import (
	"context"
	"flag"
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

// NOTE: All of the tests in this file temporarily change global state due to
// changing the command line. As such, they cannot be run in parallel.

func TestRun_OutputFile(t *testing.T) {
	oldCommandLine := flag.CommandLine
	defer func() { flag.CommandLine = oldCommandLine }()

	osWrapper := oswrapper.CreateFSTestOSWrapper()

	file1Content := strings.TrimSpace(`
query1 tag=a Pass 1s false
query2 tag=a Fail 1s false
core
`)
	file2Content := strings.TrimSpace(`
query1 tag=a Pass 2s false
query3 tag=b Pass 1s false
core
`)
	require.NoError(t, osWrapper.WriteFile("in1.txt", []byte(file1Content), 0644))
	require.NoError(t, osWrapper.WriteFile("in2.txt", []byte(file2Content), 0644))

	cfg := common.Config{
		OsWrapper: osWrapper,
		Tests: []common.TestConfig{
			{ExecutionMode: "core"},
		},
	}

	flag.CommandLine = flag.NewFlagSet("merge", flag.ContinueOnError)
	require.NoError(t, flag.CommandLine.Parse([]string{"in1.txt", "in2.txt"}))

	c := &cmd{}
	c.flags.output = "out.txt"
	ctx := context.Background()
	require.NoError(t, c.Run(ctx, cfg))

	outBytes, err := osWrapper.ReadFile("out.txt")
	require.NoError(t, err)
	outStr := string(outBytes)

	// Expected output:
	// query1: Pass (merged)
	// query2: Fail
	// query3: Pass
	// duration for query1 should be (1s+2s)/2 = 1.5s
	expectedStr := strings.TrimSpace(`
query1 tag=a Pass 1.5s false
query2 tag=a Fail 1s false
query3 tag=b Pass 1s false
core
`)

	compareResults(require.New(t), outStr, expectedStr)
}

// Testing stdout is not currently done since there is no good way to capture
// while os.Stdout is used directly by the function.

func TestRun_MissingInput(t *testing.T) {
	oldCommandLine := flag.CommandLine
	defer func() { flag.CommandLine = oldCommandLine }()

	osWrapper := oswrapper.CreateFSTestOSWrapper()
	cfg := common.Config{
		OsWrapper: osWrapper,
		Tests: []common.TestConfig{
			{ExecutionMode: "core"},
		},
	}

	flag.CommandLine = flag.NewFlagSet("merge", flag.ContinueOnError)
	require.NoError(t, flag.CommandLine.Parse([]string{"missing.txt"}))

	c := &cmd{}
	c.flags.output = "out.txt"

	ctx := context.Background()
	err := c.Run(ctx, cfg)
	require.Error(t, err)
	require.ErrorContains(t, err, "while reading 'missing.txt'")
	require.ErrorContains(t, err, "file does not exist")
}

func TestRun_CreateFailure(t *testing.T) {
	oldCommandLine := flag.CommandLine
	defer func() { flag.CommandLine = oldCommandLine }()

	osWrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, osWrapper.WriteFile("in.txt", []byte("core\n"), 0644))

	cfg := common.Config{
		OsWrapper: osWrapper,
		Tests: []common.TestConfig{
			{ExecutionMode: "core"},
		},
	}

	flag.CommandLine = flag.NewFlagSet("merge", flag.ContinueOnError)
	require.NoError(t, flag.CommandLine.Parse([]string{"in.txt"}))

	c := &cmd{}
	// Use a path in a non-existent directory to trigger a creation error.
	// fstestoswrapper.Create (via OpenFile) fails if the parent directory does not exist.
	c.flags.output = "subdir/out.txt"

	ctx := context.Background()
	err := c.Run(ctx, cfg)
	require.Error(t, err)
	require.ErrorContains(t, err, "failed to open output file 'subdir/out.txt'")
}

func compareResults(r *require.Assertions, gotStr, expectedStr string) {
	parse := func(s string) result.ResultsByExecutionMode {
		reader := strings.NewReader(s)
		res, err := result.Read(reader)
		r.NoError(err, "Failed to parse result string:\n%v", s)
		return res
	}

	got := parse(gotStr)
	expected := parse(expectedStr)

	// Sort lists for deterministic comparison
	for _, l := range got {
		l.Sort()
	}
	for _, l := range expected {
		l.Sort()
	}

	r.Equal(expected, got)
}
