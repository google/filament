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

package main

import (
	"errors"
	"os"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

const validBenchJSON = `{
  "context": {
    "date": "2022-01-24T10:28:13+00:00",
    "host_name": "hostname",
    "executable": "./myexe",
    "num_cpus": 16,
    "mhz_per_cpu": 2400,
    "cpu_scaling_enabled": false,
    "caches": [],
    "load_avg": [],
    "library_build_type": "release"
  },
  "benchmarks": [
    {
      "name": "MyBenchmark",
      "run_name": "MyBenchmark",
      "run_type": "iteration",
      "repetitions": 0,
      "repetition_index": 0,
      "threads": 1,
      "iterations": 1,
      "real_time": 100,
      "cpu_time": 100,
      "time_unit": "ns"
    }
  ]
}`

func TestRun_Success(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("a.json", []byte(validBenchJSON), 0666))
	require.NoError(t, wrapper.WriteFile("b.json", []byte(validBenchJSON), 0666))

	err := run("a.json", "b.json", wrapper)
	require.NoError(t, err)
}

func TestRun_ReadErrorA(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("b.json", []byte(validBenchJSON), 0666))
	// a.json does not exist

	err := run("a.json", "b.json", wrapper)
	require.Error(t, err)
	var pathErr *os.PathError
	require.True(t, errors.As(err, &pathErr))
	require.Equal(t, "open", pathErr.Op)
	require.Equal(t, "a.json", pathErr.Path)
	require.ErrorIs(t, pathErr, os.ErrNotExist)
}

func TestRun_ParseErrorA(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("a.json", []byte("invalid json"), 0666))
	require.NoError(t, wrapper.WriteFile("b.json", []byte(validBenchJSON), 0666))

	err := run("a.json", "b.json", wrapper)
	require.Error(t, err)
	require.ErrorContains(t, err, "invalid character")
}

func TestRun_ReadErrorB(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("a.json", []byte(validBenchJSON), 0666))
	// b.json does not exist

	err := run("a.json", "b.json", wrapper)
	require.Error(t, err)
	var pathErr *os.PathError
	require.True(t, errors.As(err, &pathErr))
	require.Equal(t, "open", pathErr.Op)
	require.Equal(t, "b.json", pathErr.Path)
	require.ErrorIs(t, pathErr, os.ErrNotExist)
}

func TestRun_ParseErrorB(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("a.json", []byte(validBenchJSON), 0666))
	require.NoError(t, wrapper.WriteFile("b.json", []byte("invalid json"), 0666))

	err := run("a.json", "b.json", wrapper)
	require.Error(t, err)
	require.ErrorContains(t, err, "invalid character")
}
