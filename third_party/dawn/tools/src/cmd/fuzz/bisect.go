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
	"os"
	"path/filepath"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/execwrapper"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

// TODO(crbug.com/416755658): Add unittest coverage when exec calls are done
// via dependency injection.

type bisectStepResult int

const (
	BisectStepPassed bisectStepResult = iota
	BisectStepFailed
	BisectStepError // sync or build failed
)

type bisectConfig struct {
	*taskConfig
	failingHash string
	passingHash string
	isFix       bool
	fuzzerName  string
}

// verifyCommit checks out a commit, calls testBisectStep, and asserts that it matches the expected outcome.
func (bc *bisectConfig) verifyCommit(hash string, expectPass bool) error {
	var passed bool
	err := bc.atGitHash(hash, func() error {
		result, err := testBisectStep(bc.taskConfig)
		if err != nil {
			return err
		}
		passed = result == BisectStepPassed
		return nil
	})

	if err != nil {
		return fmt.Errorf("execution at %s failed: %w", hash, err)
	}

	if passed != expectPass {
		return fmt.Errorf("fuzzer execution returned pass=%v, but expected pass=%v", passed, expectPass)
	}
	return nil
}

// performBisect executes the standard git bisect routine by launching the current tool as an internal step subcommand.
func (bc *bisectConfig) performBisect() error {
	// Guarantee git bisect reset is called when bisection finishes
	defer func() {
		_, _ = bc.runCmd("git", "bisect", "reset")
	}()

	if _, err := bc.runCmd("git", "bisect", "start"); err != nil {
		return fmt.Errorf("failed to start git bisect: %w", err)
	}

	var err error
	if bc.isFix {
		// "bad" in git bisect is the target state we want to identify (the fix).
		// "good" in git bisect is the starting/original state (the breakage).
		_, err = bc.runCmd("git", "bisect", "bad", bc.passingHash)
		if err == nil {
			_, err = bc.runCmd("git", "bisect", "good", bc.failingHash)
		}
	} else {
		// "bad" in git bisect is the target state (the breakage).
		// "good" in git bisect is the working state (the passing).
		_, err = bc.runCmd("git", "bisect", "bad", bc.failingHash)
		if err == nil {
			_, err = bc.runCmd("git", "bisect", "good", bc.passingHash)
		}
	}
	if err != nil {
		return fmt.Errorf("failed to assign good/bad bounds to git bisect: %w", err)
	}

	self, err := os.Executable()
	if err != nil {
		return fmt.Errorf("failed to get absolute path of self: %w", err)
	}

	// Construct args to call ourselves back as the bisect-step handler
	args := []string{"bisect", "run", self, "-bisect-step", "-bisect=" + bc.bisectFile, "-build=" + bc.build}
	if bc.isFix {
		args = append(args, "-is-fix")
	}
	if bc.fuzzMode == FuzzModeIr {
		args = append(args, "-ir")
	}
	if bc.mesaMode {
		args = append(args, "-mesa")
	}
	if bc.verbose {
		args = append(args, "-verbose")
	}

	// Execute git bisect run
	if err := bc.runCmdUnbuffered("git", args...); err != nil {
		return fmt.Errorf("git bisect run failed: %w", err)
	}

	// Git bisect leaves refs/bisect/bad pointing to the identified commit.
	// Query refs/bisect/bad to fetch the exact found commit hash.
	if foundHashBytes, err := bc.runCmd("git", "rev-parse", "refs/bisect/bad"); err == nil {
		foundHash := strings.TrimSpace(string(foundHashBytes))
		bannerText := "FOUND BREAKAGE"
		if bc.isFix {
			bannerText = "FOUND FIX"
		}
		fmt.Println("\n================================================================================")
		fmt.Printf("%s @ %s\n", bannerText, foundHash)
		if logBytes, err := bc.runCmd("git", "log", "-1", "--oneline", foundHash); err == nil {
			fmt.Printf("%s", string(logBytes))
		}
		fmt.Println("================================================================================")
		fmt.Println()
	}

	return nil
}

// runBisect orchestrates the automated bisection of a fuzzer testcase.
// It verifies that both passing and failing points behave as expected and then executes a git bisect run.
func runBisect(t *taskConfig) error {
	if t.bisectFile == "" {
		return fmt.Errorf("-bisect flag requires a test case file path")
	}
	if t.knownFailing == "" {
		return fmt.Errorf("-known-failing flag is required when bisecting")
	}

	if !t.skipInputTypeCheck {
		if err := checkInputFileType(t.bisectFile, t.fuzzMode, t.osWrapper); err != nil {
			return err
		}
	}

	bc := &bisectConfig{taskConfig: t}

	bc.fuzzerName = filepath.Base(t.fuzzer)

	// Verify build directory exists and is a GN build (contains args.gn)
	argsGnPath := filepath.Join(t.build, "args.gn")
	if !fileutils.IsFile(argsGnPath, t.osWrapper) {
		return fmt.Errorf("build directory '%v' does not appear to be a GN build directory (missing args.gn), which is required for bisecting", t.build)
	}

	var err error
	failingEP, err := resolveEndpoint(bc.knownFailing, bc.execWrapper)
	if err != nil {
		return fmt.Errorf("failed to resolve known-failing '%s': %w", bc.knownFailing, err)
	}

	passingEP, err := resolveEndpoint(bc.knownPassing, bc.execWrapper)
	if err != nil {
		return fmt.Errorf("failed to resolve known-passing '%s': %w", bc.knownPassing, err)
	}

	if failingEP.timestamp.Before(passingEP.timestamp) {
		bc.isFix = true
		fmt.Println("Detected flow: FAILING commit is older than PASSING commit (looking for the FIX).")
	} else if passingEP.timestamp.Before(failingEP.timestamp) {
		bc.isFix = false
		fmt.Println("Detected flow: PASSING commit is older than FAILING commit (looking for the BREAKAGE).")
	} else {
		return fmt.Errorf("failing and passing endpoints resolve to the same time (%v), cannot bisect", failingEP.timestamp)
	}

	// Resolve the endpoints to final exact hashes based on older/newer boundary roles
	bc.failingHash, err = resolveEndpointToHash(failingEP, bc.isFix, bc.execWrapper)
	if err != nil {
		return fmt.Errorf("failed to resolve known-failing date bound: %w", err)
	}

	bc.passingHash, err = resolveEndpointToHash(passingEP, !bc.isFix, bc.execWrapper)
	if err != nil {
		return fmt.Errorf("failed to resolve known-passing date bound: %w", err)
	}

	fmt.Printf("Resolved known-failing to commit: %s\n", bc.failingHash)
	fmt.Printf("Resolved known-passing to commit: %s\n", bc.passingHash)

	// Determine lineage
	mergeBaseBytes, err := bc.runCmd("git", "merge-base", bc.failingHash, bc.passingHash)
	if err != nil {
		return fmt.Errorf("failed to find merge-base between %s and %s: %w", bc.failingHash, bc.passingHash, err)
	}
	mergeBase := strings.TrimSpace(string(mergeBaseBytes))

	if bc.isFix {
		if mergeBase != bc.failingHash {
			return fmt.Errorf("the failing commit (%s) and passing commit (%s) are not in a direct ancestral relationship (merge-base is %s)", bc.failingHash, bc.passingHash, mergeBase)
		}
	} else {
		if mergeBase != bc.passingHash {
			return fmt.Errorf("the passing commit (%s) and failing commit (%s) are not in a direct ancestral relationship (merge-base is %s)", bc.passingHash, bc.failingHash, mergeBase)
		}
	}

	// Verify failing commit behaves as expected
	fmt.Printf("Verifying known-failing commit (%s)...\n", bc.failingHash)
	if err := bc.verifyCommit(bc.failingHash, false); err != nil {
		return fmt.Errorf("verification of known-failing commit %s failed: %w", bc.failingHash, err)
	}
	fmt.Println("Verification succeeded: fuzzer failed on known-failing commit as expected.")

	// Verify passing commit behaves as expected
	fmt.Printf("Verifying known-passing commit (%s)...\n", bc.passingHash)
	if err := bc.verifyCommit(bc.passingHash, true); err != nil {
		return fmt.Errorf("verification of known-passing commit %s failed: %w", bc.passingHash, err)
	}
	fmt.Println("Verification succeeded: fuzzer passed on known-passing commit as expected.")

	// Run bisection
	fmt.Println("Both bounds verified. Commencing bisection...")
	if err := bc.performBisect(); err != nil {
		return fmt.Errorf("bisection failed: %w", err)
	}

	return nil
}

// testBisectStep runs gclient sync, builds t.fuzzer, and runs the test case, returning the outcome.
func testBisectStep(t *taskConfig) (bisectStepResult, error) {
	fuzzerName := filepath.Base(t.fuzzer)
	if err := t.syncAndBuildTargets([]string{fuzzerName}); err != nil {
		return BisectStepError, err
	}

	fmt.Printf("--> Running fuzzer %s against test case...\n", fuzzerName)
	_, err := t.runCmd(t.fuzzer, "-timeout=60", t.bisectFile)
	if err != nil {
		return BisectStepFailed, nil
	}
	return BisectStepPassed, nil
}

// runBisectStep implements a single git bisect step invoked by git bisect run.
// It calls syncBuildAndRun and returns an appropriate exit codes (0 for good, 1 for bad, 125 for skipped/unbuildable).
// It intentionally does not use atGitHash, because git bisect should be managing the repo state.
func runBisectStep(t *taskConfig) error {
	fmt.Print("Running bisect step for commit: ")
	if out, err := t.runCmd("git", "rev-parse", "HEAD"); err == nil {
		fmt.Printf("%s", string(out))
	} else {
		fmt.Println("unknown")
	}

	result, err := testBisectStep(t)
	if err != nil {
		fmt.Printf("Step execution failed: %v. Skipping commit.\n", err)
		t.exitFn(125)
		return nil
	}

	// Translate to git bisect exit code
	if t.isFix {
		if result == BisectStepPassed {
			fmt.Println("Fuzzer passed. This commit has the fix (bad for bisect).")
			t.exitFn(1)
		} else {
			fmt.Println("Fuzzer failed. This commit does NOT have the fix (good for bisect).")
			t.exitFn(0)
		}
	} else {
		if result == BisectStepPassed {
			fmt.Println("Fuzzer passed. This commit does NOT have the breakage (good for bisect).")
			t.exitFn(0)
		} else {
			fmt.Println("Fuzzer failed. This commit has the breakage (bad for bisect).")
			t.exitFn(1)
		}
	}

	return nil
}

type endpoint struct {
	timestamp time.Time
	isDate    bool
	dateStr   string
	hash      string
}

// getRepoLocation retrieves the timezone location of the HEAD commit of the repository.
func getRepoLocation(execWrapper execwrapper.ExecWrapper) (*time.Location, error) {
	out, err := execWrapper.Command("git", "log", "-1", "--format=%cI", "HEAD").RunWithCombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("failed to get commit time of HEAD: %w", err)
	}
	timeStr := strings.TrimSpace(string(out))
	parsedTime, err := time.Parse(time.RFC3339, timeStr)
	if err != nil {
		return nil, fmt.Errorf("failed to parse HEAD commit time %s: %w", timeStr, err)
	}
	return parsedTime.Location(), nil
}

// resolveEndpoint parses an input string to determine if it is a hash or a date.
func resolveEndpoint(input string, execWrapper execwrapper.ExecWrapper) (*endpoint, error) {
	if input == "" {
		input = "HEAD"
	}

	// Try treating it as a hash or ref first
	out, err := execWrapper.Command("git", "rev-parse", "--verify", input+"^{commit}").RunWithCombinedOutput()
	if err == nil {
		hash := strings.TrimSpace(string(out))
		out, err = execWrapper.Command("git", "log", "-1", "--format=%cI", hash).RunWithCombinedOutput()
		if err != nil {
			return nil, fmt.Errorf("failed to get commit time for %s: %w", hash, err)
		}
		timeStr := strings.TrimSpace(string(out))
		parsedTime, err := time.Parse(time.RFC3339, timeStr)
		if err != nil {
			return nil, fmt.Errorf("failed to parse commit time %s: %w", timeStr, err)
		}
		return &endpoint{hash: hash, timestamp: parsedTime}, nil
	}

	// Get repo location based on HEAD timezone to interpret input dates consistently with repo history
	repoLoc, err := getRepoLocation(execWrapper)
	if err != nil {
		repoLoc = time.Local
	}

	// Strictly require YYYY-MM-DD format for imprecise dates
	parsedTime, err := time.ParseInLocation("2006-01-02", input, repoLoc)
	if err != nil {
		return nil, fmt.Errorf("could not resolve '%s' as a git ref, and it does not match required YYYY-MM-DD date format", input)
	}

	return &endpoint{
		isDate:    true,
		dateStr:   input,
		timestamp: parsedTime,
	}, nil
}

// resolveEndpointToHash resolves an endpoint to an exact git commit hash based on whether it is the older or newer bound.
func resolveEndpointToHash(ep *endpoint, isOlderBound bool, execWrapper execwrapper.ExecWrapper) (string, error) {
	if !ep.isDate {
		return ep.hash, nil
	}

	if isOlderBound {
		// Older bound date: want the last hash of the previous day, i.e. last commit BEFORE the date D (D 00:00:00).
		timeStr := ep.timestamp.Format(time.RFC3339)
		out, err := execWrapper.Command("git", "log", "-1", "--format=%H", "--before="+timeStr).RunWithCombinedOutput()
		if err != nil {
			return "", fmt.Errorf("failed to get last commit before %s: %w (output: %s)", timeStr, err, strings.TrimSpace(string(out)))
		}
		hash := strings.TrimSpace(string(out))
		if hash == "" {
			return "", fmt.Errorf("no commits found before %s", ep.dateStr)
		}
		return hash, nil
	} else {
		// Newer bound date: want the first hash of the next day, i.e. first commit ON or AFTER (D + 1 day).
		nextDay := ep.timestamp.AddDate(0, 0, 1)
		timeStr := nextDay.Format(time.RFC3339)
		out, err := execWrapper.Command("git", "log", "--format=%H", "--since="+timeStr, "--reverse").RunWithCombinedOutput()
		if err != nil {
			return "", fmt.Errorf("failed to get commits since %s: %w (output: %s)", timeStr, err, strings.TrimSpace(string(out)))
		}
		lines := strings.Split(strings.TrimSpace(string(out)), "\n")
		if len(lines) > 0 && lines[0] != "" {
			return lines[0], nil
		}
		return "", fmt.Errorf("no commits found on or after next day %s", nextDay.Format("2006-01-02"))
	}
}
