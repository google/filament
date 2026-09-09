// Copyright 2023 The Dawn & Tint Authors
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

// fuzz is a helper for running the tint fuzzer executables
package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"sync/atomic"

	"dawn.googlesource.com/dawn/tools/src/execwrapper"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/progressbar"
	"dawn.googlesource.com/dawn/tools/src/transform"
	"dawn.googlesource.com/dawn/tools/src/utils"
)

// TODO(crbug.com/416755658): Add unittest coverage when exec calls are done
// via dependency injection.
// TODO(crbug.com/344014313): Add unittests once fileutils and term are updated
// to support dependency injection.
// TODO(crbug.com/344014313): Add unittests once term is converted to support
// dependency injection.

type TaskMode int

const (
	TaskModeRun TaskMode = iota
	TaskModeCheck
	TaskModeGenerate
	TaskModeTriage
	TaskModeBisect
	TaskModeBisectStep
	TaskModeExperiment
)

type FuzzMode int

func (f FuzzMode) String() string {
	switch f {
	case FuzzModeWgsl:
		return "wgsl"
	case FuzzModeIr:
		return "ir"
	default:
		return "<unknown>"
	}
}

const (
	FuzzModeWgsl FuzzMode = iota
	FuzzModeIr
)

// mainConfig represents the top-level configuration for the fuzz utility application.
// It stores the command-line flags and parameters parsed during startup, serving as
// the central config for deciding which tasks and modes are executed.
type mainConfig struct {
	verbose            bool
	dump               bool
	fuzzMode           FuzzMode
	cmdMode            TaskMode // meta-task being requested by the user, may require running multiple tasks internally
	mesaMode           bool
	filter             string
	inputs             string
	triageFile         string
	bisectFile         string
	knownFailing       string
	knownPassing       string
	bisectStep         bool
	isFix              bool
	skipInputTypeCheck bool
	build              string
	out                string
	numProcesses       int
	experimentPath     string
	machineName        string
	osWrapper          oswrapper.OSWrapper
	execWrapper        execwrapper.ExecWrapper
	progressBuilder    progressbar.Build
	exitFn             func(int)
}

func showUsage() {
	out := flag.CommandLine.Output()
	_, _ = fmt.Fprintln(out, `
fuzz is a helper for running the tint fuzzer executables and other related tasks

fuzz has 6, mutually exclusive, tasks that it can perform:
1. Run a fuzzer locally, requires no additional flag.
2. Check that a fuzzer successfully handles contents of -inputs, requires -check flag
3. Generate a fuzzer corpus based on contents of -inputs, requires -generate flag
4. Triage a specific fuzzer crash, requires -triage flag
5. Bisect a specific fuzzer crash test case, requires -bisect flag
6. Run performance and benchmarking experiments, requires -experiment flag

usage:
  fuzz [flags...]`)
	flag.PrintDefaults()
	_, _ = fmt.Fprintln(out, ``)
}

func main() {
	c := mainConfig{}
	c.osWrapper = oswrapper.GetRealOSWrapper()
	c.execWrapper = execwrapper.CreateRealExecWrapper()
	c.progressBuilder = progressbar.New
	c.exitFn = os.Exit

	flag.Usage = showUsage

	check, generate, irMode := false, false, false
	flag.BoolVar(&c.verbose, "verbose", false, "print additional output")
	flag.BoolVar(&check, "check", false, "check that all the end-to-end tests in -inputs do not fail")
	flag.BoolVar(&generate, "generate", false, "generate fuzzing corpus based on -inputs")
	flag.BoolVar(&c.dump, "dump", false, "dumps shader input/output from fuzzer")
	flag.BoolVar(&irMode, "ir", false, "runs using IR fuzzer instead of WGSL fuzzer")
	flag.BoolVar(&c.mesaMode, "mesa", false, "runs using Mesa fuzzer variants")
	flag.StringVar(&c.filter, "filter", "", "filter the fuzzing passes run to those with this substring")
	flag.StringVar(&c.inputs, "corpus", defaultWgslCorpusDir(c.osWrapper), "obsolete, use -inputs instead")
	flag.StringVar(&c.inputs, "inputs", defaultWgslCorpusDir(c.osWrapper), "the directory that holds the files to use")
	flag.StringVar(&c.triageFile, "triage", "", "triage a fuzzer crash")
	flag.StringVar(&c.bisectFile, "bisect", "", "bisect a fuzzer crash")
	flag.StringVar(&c.knownFailing, "known-failing", "", "known failing git hash or time")
	flag.StringVar(&c.knownPassing, "known-passing", "", "known passing git hash or time")
	flag.StringVar(&c.build, "build", defaultBuildDir(c.osWrapper), "the build directory")
	flag.StringVar(&c.out, "out", "<tmp>", "the directory to store outputs to")
	flag.IntVar(&c.numProcesses, "j", 0, "number of concurrent fuzzers to run (defaults to 1 for experiments, NumCPU for others)")
	flag.BoolVar(&c.bisectStep, "bisect-step", false, "internal flag used by git bisect run")
	flag.BoolVar(&c.isFix, "is-fix", false, "internal flag used by git bisect run to indicate if we are bisecting a fix")
	flag.BoolVar(&c.skipInputTypeCheck, "skip-input-type-check", false, "bypass the heuristic text/binary input file type check")
	flag.StringVar(&c.experimentPath, "experiment", "", "run an experiment using the configuration at <root> (WIP feature)")
	flag.StringVar(&c.machineName, "machine", "", "machine name to identify results")
	flag.Parse()

	if c.mesaMode && c.filter != "" {
		fmt.Println("cannot set -mesa and -filter flags at the same time, as Mesa fuzzers only run a single specific pass")
		os.Exit(1)
	}

	modeCount := 0
	if check {
		modeCount++
	}
	if generate {
		modeCount++
	}
	if c.triageFile != "" {
		modeCount++
	}
	if c.bisectFile != "" {
		modeCount++
	}
	if c.experimentPath != "" {
		modeCount++
	}

	if !c.bisectStep && modeCount > 1 {
		fmt.Println("cannot set more than one of -check, -generate, -triage, -bisect, and -experiment flags at the same time")
		os.Exit(1)
	}

	switch {
	case c.bisectStep:
		c.cmdMode = TaskModeBisectStep
	case check:
		c.cmdMode = TaskModeCheck
	case generate:
		c.cmdMode = TaskModeGenerate
	case c.triageFile != "":
		c.cmdMode = TaskModeTriage
	case c.bisectFile != "":
		c.cmdMode = TaskModeBisect
	case c.experimentPath != "":
		c.cmdMode = TaskModeExperiment
	default:
		c.cmdMode = TaskModeRun
	}

	if irMode {
		c.fuzzMode = FuzzModeIr
	} else {
		c.fuzzMode = FuzzModeWgsl
	}

	if c.numProcesses == 0 {
		// If running experiment default to single thread to avoid overloading system, otherwise try to run as many
		// threads as possible without swamping
		if c.cmdMode == TaskModeExperiment {
			c.numProcesses = 1
		} else {
			c.numProcesses = runtime.NumCPU()
		}
	}

	if c.numProcesses < 1 {
		c.numProcesses = 1
	}

	if c.cmdMode == TaskModeGenerate && (c.out == "" || c.out == "<tmp>") {
		fmt.Println("need to specify -output when using -generate")
		os.Exit(1)
	}

	if err := run(&c); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

type taskConfig struct {
	mainConfig
	taskMode   TaskMode // specific task being run at this time, may be different from mainConfig.cmdMode
	fuzzer     string   // path to the fuzzer binary, tint_wgsl_fuzzer or tint_ir_fuzzer
	assembler  string   // path to the test case assembler, tint_fuzz_as
	dictionary string   // path to dictionary to use for tint_wgsl_fuzzer
}

// runCmd executes a command with standardized output capturing and logging behavior.
func (t *taskConfig) runCmd(name string, args ...string) ([]byte, error) {
	if t.verbose {
		fmt.Printf("executing: %s %s\n", name, strings.Join(args, " "))
	}
	cmd := t.execWrapper.Command(name, args...)
	out, err := cmd.RunWithCombinedOutput()
	if t.verbose {
		fmt.Printf("output:\n%s\n", string(out))
	}
	return out, err
}

// runCmdUnbuffered executes a command with standardized logging, mapping output directly to the terminal.
// It does not capture the output.
func (t *taskConfig) runCmdUnbuffered(name string, args ...string) error {
	if t.verbose {
		fmt.Printf("executing: %s %s\n", name, strings.Join(args, " "))
	}
	cmd := t.execWrapper.Command(name, args...).WithStdout(os.Stdout).WithStderr(os.Stderr)
	return cmd.Run()
}

// atGitHash saves the current git state, checks out a specific hash, executes a
// function, and then restores the original git state.
func (t *taskConfig) atGitHash(hash string, fn func() error) error {
	origRefBytes, err := t.runCmd("git", "rev-parse", "--abbrev-ref", "HEAD")
	if err != nil {
		return fmt.Errorf("failed to get original HEAD reference: %w", err)
	}
	origRef := strings.TrimSpace(string(origRefBytes))
	if origRef == "HEAD" {
		origHeadBytes, err := t.runCmd("git", "rev-parse", "HEAD")
		if err != nil {
			return fmt.Errorf("failed to get original HEAD commit: %w", err)
		}
		origRef = strings.TrimSpace(string(origHeadBytes))
	}

	defer func() {
		fmt.Printf("Restoring repository to original state (%s)...\n", origRef)
		_, _ = t.runCmd("git", "checkout", origRef)
		_, _ = t.runCmd("gclient", "sync")
	}()

	fmt.Printf("Syncing repository to hash %s...\n", hash)
	if _, err := t.runCmd("git", "checkout", hash); err != nil {
		return fmt.Errorf("failed to checkout %s: %w", hash, err)
	}

	return fn()
}

func run(c *mainConfig) error {
	// Verify / create the directories needed for writing
	switch c.cmdMode {
	case TaskModeExperiment:
		// output/build directory checking is part of runExperiment, since the expected locations/content are based on
		// the values in the experiment config file.
	case TaskModeRun, TaskModeCheck, TaskModeGenerate:
		// These modes allow for using a temporary directory
		if c.out == "" || c.out == "<tmp>" {
			if tmp, err := c.osWrapper.MkdirTemp("", "tint_fuzz"); err == nil {
				defer c.osWrapper.RemoveAll(tmp)
				c.out = tmp
			} else {
				return err
			}
			break
		}
		fallthrough
	default:
		// If temporary directories are allowed, c.out should already have been set to the created value by this point
		if c.out == "" || c.out == "<tmp>" {
			return fmt.Errorf("temporary output is not allowed for '%v'", c.cmdMode)
		}

		err := c.osWrapper.MkdirAll(c.out, os.ModePerm)
		if err != nil {
			return err
		}

		// Check the build directory
		if !fileutils.IsDir(c.build, c.osWrapper) {
			return fmt.Errorf("build directory '%v' does not exist", c.build)
		}
	}

	queue := make([]*taskConfig, 0, 1)

	if c.fuzzMode == FuzzModeIr && (c.cmdMode == TaskModeRun || c.cmdMode == TaskModeCheck) {
		// The default input files are .wgsl files and tint_ir_fuzzer runs on .tirb files, so need
		// to convert them before running/checking
		if c.inputs == defaultWgslCorpusDir(c.osWrapper) {
			origOut := c.out
			tmp, err := c.osWrapper.MkdirTemp("", "ir_corpus")
			if err != nil {
				return fmt.Errorf("failed to create temporary directory for IR corpus: %w", err)
			}
			defer c.osWrapper.RemoveAll(tmp)

			c.out = tmp
			t, err := generateTaskConfig(TaskModeGenerate, c)
			if err != nil {
				return fmt.Errorf("failed to generate task config for IR corpus generation: %w", err)
			}
			queue = append(queue, t)

			c.out = origOut
			c.inputs = tmp
		}
	}

	t, err := generateTaskConfig(c.cmdMode, c)
	if err != nil {
		return fmt.Errorf("failed to generate task config for command mode %d: %w", c.cmdMode, err)
	}
	queue = append(queue, t)

	for _, t := range queue {
		var err error
		switch t.taskMode {
		case TaskModeRun:
			err = runFuzzer(t)
		case TaskModeCheck:
			err = checkFuzzer(t)
		case TaskModeGenerate:
			err = runCorpusGenerator(t)
		case TaskModeTriage:
			err = runTriage(t)
		case TaskModeBisect:
			err = runBisect(t)
		case TaskModeBisectStep:
			err = runBisectStep(t)
		case TaskModeExperiment:
			err = runExperiment(t)
		default:
			err = fmt.Errorf("unknown task mode %d", t.taskMode)
		}
		if err != nil {
			return err
		}
	}
	return nil
}

// generateTaskConfig produces a taskConfig based off the supplied mainConfig and specified TaskMode.
func generateTaskConfig(tm TaskMode, c *mainConfig) (*taskConfig, error) {
	t := taskConfig{
		mainConfig: *c,
		taskMode:   tm,
	}

	type depConfig struct {
		name string
		path *string
	}
	dependencies := make([]depConfig, 0)
	switch tm {
	case TaskModeRun:
		if c.fuzzMode == FuzzModeWgsl {
			dependencies = append(dependencies, depConfig{"dictionary.txt", &t.dictionary})
		}
		fallthrough
	case TaskModeCheck, TaskModeTriage, TaskModeBisect, TaskModeBisectStep:
		fuzzerName := "tint_wgsl_fuzzer"
		if c.fuzzMode == FuzzModeIr {
			fuzzerName = "tint_ir_fuzzer"
		}
		if c.mesaMode {
			fuzzerName = strings.Replace(fuzzerName, "_fuzzer", "_mesa_fuzzer", 1)
		}
		dependencies = append(dependencies, depConfig{fuzzerName, &t.fuzzer})

		if tm == TaskModeTriage {
			if c.fuzzMode == FuzzModeIr {
				dependencies = append(dependencies, depConfig{"ir_fuzz_dis", &t.assembler})
			} else {
				dependencies = append(dependencies, depConfig{"ir_fuzz_as", &t.assembler})
			}
		}
	case TaskModeGenerate:
		if c.fuzzMode == FuzzModeIr {
			dependencies = append(dependencies, depConfig{"ir_fuzz_as", &t.assembler})
		}
	case TaskModeExperiment:
		// No dependencies required to be pre-validated by this helper
	}

	// Verify all the required dependencies are present
	for _, config := range dependencies {
		switch {
		case filepath.Ext(config.name) == ".txt":
			*config.path = filepath.Join(filepath.Join(fileutils.DawnRoot(c.osWrapper), "src", "tint", "cmd", "fuzz", "wgsl"), config.name)
			if !fileutils.IsFile(*config.path, t.osWrapper) {
				return nil, fmt.Errorf("resource '%v' not found at '%v'. Please ensure the Dawn repository is correctly cloned and up-to-date", config.name, *config.path)
			}
		default:
			*config.path = filepath.Join(t.build, config.name+fileutils.ExeExt)
			if tm != TaskModeBisect && tm != TaskModeBisectStep && !fileutils.IsExe(*config.path, t.osWrapper) {
				return nil, fmt.Errorf("binary '%v' not found at '%v'. Please ensure the project has been built (e.g., with `ninja -C %s %s`)", config.name, *config.path, t.build, config.name)
			}
		}
	}

	return &t, nil
}

// checkFuzzer runs the fuzzer against all the test files the inputs directory,
// ensuring that the fuzzers do not error for the given file.
func checkFuzzer(t *taskConfig) error {
	var files []string
	var err error
	switch t.fuzzMode {
	case FuzzModeIr:
		files, err = glob.Glob(filepath.Join(t.inputs, "**.tirb"), t.osWrapper)
	case FuzzModeWgsl:
		files, err = glob.Glob(filepath.Join(t.inputs, "**.wgsl"), t.osWrapper)
	default:
		err = fmt.Errorf("unknown fuzzer mode %d", t.fuzzMode)
	}
	if err != nil {
		return err
	}

	// Remove '*.expected.*'
	files = transform.Filter(files, func(s string) bool { return !strings.Contains(s, ".expected.") })

	fmt.Printf("checking %v files...\n", len(files))

	remaining := transform.SliceToChan(files)

	pb := t.progressBuilder(os.Stdout, nil)
	defer pb.Stop()
	var numDone uint32

	routine := func() error {
		for file := range remaining {
			atomic.AddUint32(&numDone, 1)
			pb.Update(progressbar.Status{
				Total: len(files),
				Segments: []progressbar.Segment{
					{Count: int(atomic.LoadUint32(&numDone))},
				},
			})

			if out, err := t.runCmd(t.fuzzer, file); err != nil {
				_, fuzzer := filepath.Split(t.fuzzer)
				return fmt.Errorf("fuzzer '%s' failed to process file '%s' with error: %w\nOutput:\n%s", fuzzer, file, err, string(out))
			}
		}
		return nil
	}

	if err := utils.RunConcurrent(t.numProcesses, routine); err != nil {
		return err
	}

	fmt.Println("done")
	return nil
}

func defaultWgslCorpusDir(fsReader oswrapper.FilesystemReader) string {
	return filepath.Join(fileutils.DawnRoot(fsReader), "test", "tint")
}

func defaultBuildDir(fsReader oswrapper.FilesystemReader) string {
	return filepath.Join(fileutils.DawnRoot(fsReader), "out", "active")
}

// checkInputFileType performs a heuristic check on a single input file to ensure it matches
// the expected file type for the given fuzzer mode. WGSL mode expects text files, while IR
// mode expects binary files. It returns an error if the file type appears incorrect. This may
// have false positives, so there is a CLI escape hatch
func checkInputFileType(filePath string, mode FuzzMode, fsReader oswrapper.FilesystemReader) error {
	content, err := fsReader.ReadFile(filePath)
	if err != nil {
		return fmt.Errorf("failed to read input file for type check: %w", err)
	}

	isText := true
	for _, b := range content {
		// If byte is null or a control character other than newline, carriage return, or tab, consider it binary.
		if b < 0x20 && b != '\n' && b != '\r' && b != '\t' {
			isText = false
			break
		}
	}

	if mode == FuzzModeWgsl && !isText {
		return fmt.Errorf("wrong file type detected: expected a text file for WGSL mode, but '%s' appears to be binary. Did you forget to add the -ir flag? (If you are sure this is correct, use -skip-input-type-check)", filePath)
	} else if mode == FuzzModeIr && isText && len(content) > 0 {
		return fmt.Errorf("wrong file type detected: expected a binary file for IR mode, but '%s' appears to be text. Did you mean to remove the -ir flag? (If you are sure this is correct, use -skip-input-type-check)", filePath)
	}

	return nil
}
