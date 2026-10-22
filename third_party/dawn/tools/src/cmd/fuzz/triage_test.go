// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
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
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestDetermineFailingPass(t *testing.T) {
	tests := []struct {
		name         string
		fuzzerOutput string
		wantPass     string
		wantErr      bool
	}{
		{
			name: "Pass with Bullet Running",
			fuzzerOutput: `
Some preceding log lines...
• Running: tint::spirv::writer::IRFuzzer
Other trailing lines...`,
			wantPass: "tint::spirv::writer::IRFuzzer",
			wantErr:  false,
		},
		{
			name: "Pass with Running pass",
			fuzzerOutput: `
Some preceding log lines...
Running pass: glsl.Printer
Other trailing lines...`,
			wantPass: "glsl.Printer",
			wantErr:  false,
		},
		{
			name: "Multiple passes (latest one wins)",
			fuzzerOutput: `
• Running: core.Robustness
Running pass: glsl.Printer
Running pass: spirv.ExpandImplicitSplats`,
			wantPass: "spirv.ExpandImplicitSplats",
			wantErr:  false,
		},
		{
			name:         "Empty output",
			fuzzerOutput: "",
			wantPass:     "",
			wantErr:      true,
		},
		{
			name: "No matches",
			fuzzerOutput: `
Some error lines...
libFuzzer: deadly signal
#0 0x559ced90fb45 in __sanitizer_print_stack_trace`,
			wantPass: "",
			wantErr:  true,
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			got, err := determineFailingPass(tc.fuzzerOutput)
			if tc.wantErr {
				require.Error(t, err)
			} else {
				require.NoError(t, err)
				require.Equal(t, tc.wantPass, got)
			}
		})
	}
}

func TestGenerateTriageReport(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()

	tc := &triageConfig{
		taskConfig: &taskConfig{
			mainConfig: mainConfig{
				osWrapper:  wrapper,
				triageFile: "some-crash-file",
				fuzzMode:   FuzzModeIr,
			},
			fuzzer: "tint_ir_fuzzer",
		},
		inputBase:     "some-crash-file",
		reproFile:     "some-crash-file.repro",
		logFile:       "some-crash-file.triage.log",
		reportFile:    "some-crash-file.triage.md",
		reproStatus:   ReproStatusIdentical,
		inputsDisplay: "## IR Input\n```\n$B1: { ... }\n```\n",
		irInput:       "$B1: { ... }",
		failingPass:   "tint::glsl::writer::IRFuzzer",
		verboseOut: []byte(`
SomeOtherConfig{
  foo: bar
}
=========================================================
== IR dump before glsl.Printer:
$B1: {
  # Root block
}
========

SubstituteOverridesConfig{
  map: []
}
=========================================================
== IR dump before glsl.Printer:
$B1: {
  # Root block - modified
}
========
#0 0x12345 in __sanitizer_print_stack_trace
`),
		filterArg: "--filter=tint::glsl::writer::IRFuzzer",
	}

	err := generateTriageReport(tc)
	require.NoError(t, err)

	// Verify that the report file was created
	content, err := wrapper.ReadFile("some-crash-file.triage.md")
	require.NoError(t, err)
	reportStr := string(content)

	// Check for key elements of our triage report structure
	require.Contains(t, reportStr, "# Triage Report for some-crash-file")
	require.Contains(t, reportStr, "## Status\nInitial crash reproduced")
	require.Contains(t, reportStr, "### Fuzzer\n`tint_ir_fuzzer`")
	require.Contains(t, reportStr, "### Original File\n`some-crash-file`")
	require.Contains(t, reportStr, "### Reproduction File\n`some-crash-file.repro` (identical to original)")
	require.Contains(t, reportStr, "## Reproduction Instructions")
	require.Contains(t, reportStr, "tint_ir_fuzzer -timeout=60 --verbose --dump-ir=true --filter=tint::glsl::writer::IRFuzzer some-crash-file.repro")
	require.Contains(t, reportStr, "## Failing Pass\n`tint::glsl::writer::IRFuzzer`")
	require.Contains(t, reportStr, "## Transforms Run\n```\nglsl.Printer\nglsl.Printer\n```")
	require.Contains(t, reportStr, "## Failing Transform\n`glsl.Printer`")
	require.Contains(t, reportStr, "## Failing Transform Config\n```\nSubstituteOverridesConfig{\n  map: []\n}\n```")
	require.Contains(t, reportStr, "## IR Dump before failure\n\n```\n$B1: {\n  # Root block - modified\n}\n```")
	require.Contains(t, reportStr, "## Crash Stack\n```\n#0 0x12345 in __sanitizer_print_stack_trace\n")
}

func TestGenerateTriageReportResilience(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()

	tc := &triageConfig{
		taskConfig: &taskConfig{
			mainConfig: mainConfig{
				osWrapper:  wrapper,
				triageFile: "some-crash-file",
				fuzzMode:   FuzzModeIr,
			},
			fuzzer: "tint_ir_fuzzer",
		},
		inputBase:     "some-crash-file",
		reproFile:     "some-crash-file.repro",
		logFile:       "some-crash-file.triage.log",
		reportFile:    "some-crash-file.triage.md",
		reproStatus:   ReproStatusIdentical,
		inputsDisplay: "## IR Input\n```\n$B1: { ... }\n```\n",
		irInput:       "$B1: { ... }",
		failingPass:   "", // Empty failing pass
		verboseOut:    []byte(`just some random logs without ir dump or backtrace`),
		filterArg:     "", // Empty filterArg
	}

	err := generateTriageReport(tc)
	require.NoError(t, err)

	// Verify that the report file was created
	content, err := wrapper.ReadFile("some-crash-file.triage.md")
	require.NoError(t, err)
	reportStr := string(content)

	// Check for key resilience features of our triage report structure
	require.Contains(t, reportStr, "# Triage Report for some-crash-file")
	require.Contains(t, reportStr, "## Reproduction Instructions")
	// Should not have --filter, and should have single spaces
	require.Contains(t, reportStr, "tint_ir_fuzzer -timeout=60 --verbose --dump-ir=true some-crash-file.repro")
	require.Contains(t, reportStr, "## Failing Pass\n`Unknown (failed to identify from fuzzer output)`")
	require.Contains(t, reportStr, "## Transforms Run\n```\nNone\n```")
	require.Contains(t, reportStr, "## Failing Transform\n`Unknown`")
	require.Contains(t, reportStr, "## Failing Transform Config\nNone")
	require.Contains(t, reportStr, "## Crash Stack\n```\nUnknown (failed to extract stack trace)\n```")
}
