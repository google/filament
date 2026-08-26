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
	"fmt"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

// TODO(crbug.com/416755658): Add unittest coverage when exec calls are done
// via dependency injection.

// reproStatus indicates the result of the reproduction attempt, failed reproduction is handled via Go errors
type reproStatus int

const (
	ReproStatusIdentical reproStatus = iota
	ReproStatusFixIdentifiers
)

// StatusString returns a human-readable description of the reproduction status.
func (r reproStatus) StatusString() string {
	switch r {
	case ReproStatusIdentical:
		return "Initial crash reproduced"
	case ReproStatusFixIdentifiers:
		return "Reproduced with `--fix-identifiers=true`"
	}
	panic("unreachable")
}

// NoteString returns a short parenthetical note for the reproduction status.
func (r reproStatus) NoteString() string {
	switch r {
	case ReproStatusIdentical:
		return " (identical to original)"
	case ReproStatusFixIdentifiers:
		return ""
	}
	panic("unreachable")
}

type triageConfig struct {
	*taskConfig
	inputBase     string
	reproFile     string
	logFile       string
	reportFile    string
	reproStatus   reproStatus
	inputsDisplay string
	irInput       string
	failingPass   string
	verboseOut    []byte
	filterArg     string
}

var fuzzCmdBaseArgs = []string{"-timeout=60", "--verbose"}

// runTriage performs an automated triage of a fuzzer crash.
// It verifies the reproduction, extracts human-readable IR and WGSL, identifies the failing transformation pass, and
// generates a detailed Markdown report. Returns an error if a subtask fails unexpectedly.
func runTriage(t *taskConfig) error {
	if !t.skipInputTypeCheck {
		if err := checkInputFileType(t.triageFile, t.fuzzMode, t.osWrapper); err != nil {
			return err
		}
	}

	tc := &triageConfig{taskConfig: t}
	tc.inputBase = filepath.Base(tc.triageFile)
	tc.reproFile = tc.inputBase + ".repro"
	tc.logFile = tc.inputBase + ".triage.log"
	tc.reportFile = tc.inputBase + ".triage.md"

	if tc.out != "" && tc.out != "<tmp>" {
		tc.reproFile = filepath.Join(tc.out, tc.reproFile)
		tc.logFile = filepath.Join(tc.out, tc.logFile)
		tc.reportFile = filepath.Join(tc.out, tc.reportFile)
	}

	fmt.Println("verifying reproduction...")
	fuzzerOutput, err := verifyReproduction(tc)
	if err != nil {
		return err
	}

	fmt.Println("extracting human readable input...")
	if err := extractHumanReadableInput(tc); err != nil {
		return err
	}

	fmt.Println("determining failing pass...")
	failingPass, err := determineFailingPass(fuzzerOutput)
	if err != nil {
		fmt.Printf("warning: %v\n", err)
	}
	tc.failingPass = failingPass

	fmt.Println("running specific pass with more logging...")
	if err := runSpecificPassAndLog(tc); err != nil {
		return err
	}

	fmt.Println("generating report...")
	return generateTriageReport(tc)
}

// verifyReproduction attempts to reproduce the crash using the fuzzer.
// If the initial run doesn't crash and execution in IR mode, it tries again with --fix-identifiers=true.
// It returns the fuzzer output string, and any error encountered.
func verifyReproduction(tc *triageConfig) (string, error) {
	out, err := tc.runCmd(tc.fuzzer, append(fuzzCmdBaseArgs, tc.triageFile)...)
	if err == nil {
		if tc.fuzzMode == FuzzModeIr {
			fmt.Println("initial run did not crash, attempting to fix identifiers...")
			out, err = tc.runCmd(tc.fuzzer, append(fuzzCmdBaseArgs, "--fix-identifiers=true", tc.triageFile)...)
			if err == nil {
				return "", fmt.Errorf("issue did not reproduce even with --fix-identifiers=true")
			}
			fmt.Println("crash reproduced with --fix-identifiers=true. updating test case...")
			tc.reproStatus = ReproStatusFixIdentifiers
			_, err = tc.runCmd(tc.assembler, "--strip-invalid-identifiers", "-o", tc.reproFile, tc.triageFile)
			if err != nil {
				return "", fmt.Errorf("failed to strip invalid identifiers: %w", err)
			}
			return string(out), nil
		}
		return "", fmt.Errorf("issue did not reproduce")
	}

	// Didn't need to modify, so Just copy original to reproFile
	if err := fileutils.CopyFile(tc.reproFile, tc.triageFile, tc.osWrapper); err != nil {
		return "", fmt.Errorf("failed to copy reproduction file: %w", err)
	}
	tc.reproStatus = ReproStatusIdentical
	return string(out), nil
}

// extractHumanReadableInput attempts to convert the reproduction file into human-readable IR and WGSL.
// Depending on the fuzzer mode, it uses the assembler or disassembler as needed.
// Returns formatted Markdown sections for the report and the raw IR input string.
func extractHumanReadableInput(tc *triageConfig) error {
	var irInputStatus string
	var wgslInput string
	var wgslInputStatus string

	// Always try to get IR input
	if tc.fuzzMode == FuzzModeIr {
		irOut, err := tc.runCmd(tc.assembler, tc.reproFile)
		if err != nil {
			irInputStatus = fmt.Sprintf("Failed to extract IR: `%v`", err)
		} else {
			tc.irInput = string(irOut)
		}

		wgslOut, err := tc.runCmd(tc.assembler, "--emit-wgsl", tc.reproFile)
		if err != nil {
			wgslInputStatus = fmt.Sprintf("ir_fuzz_dis could not produce WGSL: `%v`", err)
		} else {
			wgslInput = string(wgslOut)
		}
	} else {
		irOut, err := tc.runCmd(tc.assembler, "--emit-ir", tc.reproFile)
		if err != nil {
			irInputStatus = fmt.Sprintf("ir_fuzz_as could not produce IR: `%v`", err)
		} else {
			tc.irInput = string(irOut)
		}

		wgslOut, err := tc.osWrapper.ReadFile(tc.reproFile)
		if err != nil {
			wgslInputStatus = fmt.Sprintf("Failed to read WGSL: `%v`", err)
		} else {
			wgslInput = string(wgslOut)
		}
	}

	irInputDisplay := ""
	if tc.irInput != "" {
		irInputDisplay = "```\n" + tc.irInput + "```"
	}

	wgslInputDisplay := ""
	if wgslInput != "" {
		wgslInputDisplay = "```\n" + wgslInput + "```"
	}

	irSection := fmt.Sprintf("## IR Input\n%s\n%s\n", irInputStatus, irInputDisplay)
	wgslSection := fmt.Sprintf("## WGSL Input\n%s\n%s\n", wgslInputStatus, wgslInputDisplay)

	if tc.fuzzMode == FuzzModeIr {
		tc.inputsDisplay = irSection + "\n" + wgslSection
	} else {
		tc.inputsDisplay = wgslSection + "\n" + irSection
	}
	return nil
}

// determineFailingPass parses the fuzzer output to identify the fuzzing pass that was being executed before the crash
// occurred. It searches backwards from the end of the output for known pass-execution markers. Returns the failing pass
// or an error if one cannot be identified
func determineFailingPass(fuzzerOutput string) (string, error) {
	lines := strings.Split(fuzzerOutput, "\n")
	for i := len(lines) - 1; i >= 0; i-- {
		line := strings.TrimSpace(lines[i])
		if strings.HasPrefix(line, "• Running:") {
			return strings.TrimSpace(strings.TrimPrefix(line, "• Running:")), nil
		}
		if strings.Contains(line, "Running pass:") {
			parts := strings.Split(line, "Running pass:")
			if len(parts) > 1 {
				return strings.TrimSpace(parts[1]), nil
			}
		}
	}
	return "", fmt.Errorf("failed to identify failing pass from fuzzer output")
}

// runSpecificPassAndLog re-runs the fuzzer on the reproduction file, specifically targeting the
// failing pass with verbose logging and IR dumping enabled. The output is captured and written
// to a log file. Returns the verbose output, the filter argument used, and any error.
func runSpecificPassAndLog(tc *triageConfig) error {
	args := append(fuzzCmdBaseArgs, "--dump-ir=true")
	if tc.failingPass != "" {
		tc.filterArg = "--filter=" + tc.failingPass
		args = append(args, tc.filterArg)
	} else {
		tc.filterArg = ""
	}
	args = append(args, tc.reproFile)

	// Ignore command exit status issues (command is expected to crash)
	tc.verboseOut, _ = tc.runCmd(tc.fuzzer, args...)

	if err := tc.osWrapper.WriteFile(tc.logFile, tc.verboseOut, 0644); err != nil {
		return fmt.Errorf("failed to write log file: %w", err)
	}
	return nil
}

// generateTriageReport constructs a Markdown report summarizing the fuzzer crash triage results,
// including the reproduction steps, failing pass, IR dumps, and stack trace.
func generateTriageReport(tc *triageConfig) error {
	// Parse verboseOut for IR dump and stack trace
	irDump := ""
	stackTrace := ""
	var transformsRun []string
	failingTransform := ""
	var failingTransformConfig []string
	inConfig := false
	vLines := strings.Split(string(tc.verboseOut), "\n")

	inDump := false
	for _, line := range vLines {
		if strings.HasPrefix(line, "== IR dump before") {
			irDump = "" // Keep only the last one before crash
			inDump = true
			inConfig = false

			transform := strings.TrimPrefix(line, "== IR dump before")
			transform = strings.TrimSuffix(transform, ":")
			transform = strings.TrimSpace(transform)
			transformsRun = append(transformsRun, transform)
			failingTransform = transform
			continue
		}
		if inDump {
			if strings.HasPrefix(line, "========") {
				inDump = false
				continue
			}
			irDump += line + "\n"
		} else {
			if strings.HasPrefix(line, "#0 ") || strings.HasPrefix(line, "    #0 ") || strings.Contains(line, "stack trace:") || strings.Contains(line, "internal compiler error:") {
				if stackTrace == "" {
					idx := strings.Index(string(tc.verboseOut), line)
					stackTrace = string(tc.verboseOut)[idx:]
				}
			} else {
				trimmed := strings.TrimRight(line, " \r")
				if strings.HasSuffix(trimmed, "Config{") {
					failingTransformConfig = []string{}
					inConfig = true
				}
				if inConfig {
					if strings.HasPrefix(line, "====") {
						inConfig = false
					} else {
						failingTransformConfig = append(failingTransformConfig, line)
					}
				}
			}
		}
	}

	irDumpStatus := ""
	irDumpDisplay := ""
	if irDump == "" {
		irDumpStatus = "crash on original input"
	} else if strings.TrimSpace(irDump) == strings.TrimSpace(tc.irInput) {
		irDumpStatus = "input not modified before crash"
	} else {
		irDumpDisplay = "```\n" + irDump + "```"
	}

	failingPassDisplay := tc.failingPass
	if failingPassDisplay == "" {
		failingPassDisplay = "Unknown (failed to identify from fuzzer output)"
	}

	failingTransformDisplay := failingTransform
	if failingTransformDisplay == "" {
		failingTransformDisplay = "Unknown"
	}

	failingTransformConfigDisplay := "None"
	if len(failingTransformConfig) > 0 {
		failingTransformConfigDisplay = "```\n" + strings.Join(failingTransformConfig, "\n") + "\n```"
	}

	transformsRunDisplay := strings.Join(transformsRun, "\n")
	if transformsRunDisplay == "" {
		transformsRunDisplay = "None"
	}

	reproCmdParts := []string{tc.fuzzer}
	reproCmdParts = append(reproCmdParts, fuzzCmdBaseArgs...)
	reproCmdParts = append(reproCmdParts, "--dump-ir=true")
	if tc.filterArg != "" {
		reproCmdParts = append(reproCmdParts, tc.filterArg)
	}
	reproCmdParts = append(reproCmdParts, tc.reproFile)
	reproCmd := strings.Join(reproCmdParts, " ")

	stackTraceDisplay := stackTrace
	if stackTraceDisplay == "" {
		stackTraceDisplay = "Unknown (failed to extract stack trace)"
	}

	report := fmt.Sprintf(`# Triage Report for %s

## Status
%s

### Fuzzer
`+"`"+`%s`+"`"+`

### Original File
`+"`"+`%s`+"`"+`

### Reproduction File
`+"`"+`%s`+"`"+`%s

%s
## Reproduction Instructions
`+"```"+`
%s
`+"```"+`

## Failing Pass
`+"`"+`%s`+"`"+`

## Transforms Run
`+"```"+`
%s
`+"```"+`

## Failing Transform
`+"`"+`%s`+"`"+`

## Failing Transform Config
%s

## IR Dump before failure
%s
%s

## Crash Stack
`+"```"+`
%s
`+"```"+`
`,
		tc.inputBase,
		tc.reproStatus.StatusString(),
		filepath.Base(tc.fuzzer),
		tc.triageFile,
		tc.reproFile, tc.reproStatus.NoteString(),
		tc.inputsDisplay,
		reproCmd,
		failingPassDisplay,
		transformsRunDisplay,
		failingTransformDisplay,
		failingTransformConfigDisplay,
		irDumpStatus,
		irDumpDisplay,
		stackTraceDisplay)

	fmt.Println("\n--- TRIAGE REPORT ---")
	fmt.Println(report)
	fmt.Println("----------------------")

	if err := tc.osWrapper.WriteFile(tc.reportFile, []byte(report), 0644); err != nil {
		return fmt.Errorf("failed to write report file: %w", err)
	}

	fmt.Printf("\nTriage complete.\nRepro: %s\nReport: %s\n", tc.reproFile, tc.reportFile)
	return nil
}
