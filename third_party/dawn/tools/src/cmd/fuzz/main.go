// Copyright 2023 The Dawn & Tint Authors
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

// fuzz is a helper for running the tint fuzzer executables
package main

import (
	"bytes"
	"context"
	"flag"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"sync/atomic"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/progressbar"
	"dawn.googlesource.com/dawn/tools/src/term"
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
)

type FuzzMode int

const (
	FuzzModeWgsl FuzzMode = iota
	FuzzModeIr
)

type cmdConfig struct {
	verbose      bool
	dump         bool
	fuzzMode     FuzzMode
	cmdMode      TaskMode // meta-task being requested by the user, may require running multiple tasks internally
	filter       string
	inputs       string
	build        string
	out          string
	numProcesses int
	osWrapper    oswrapper.OSWrapper
}

func showUsage() {
	out := flag.CommandLine.Output()
	_, _ = fmt.Fprintln(out, `
fuzz is a helper for running the tint fuzzer executables and other related tasks

fuzz has 3, mutually exclusive, tasks that it can perform:
1. Run a fuzzer locally, requires no additional flag.
2. Check that a fuzzer successfully handles contents of -inputs, requires -check flag
3. Generate a fuzzer corpus based on contents of -inputs, requires -generate flag

usage:
  fuzz [flags...]`)
	flag.PrintDefaults()
	_, _ = fmt.Fprintln(out, ``)
}

func main() {
	c := cmdConfig{}
	c.osWrapper = oswrapper.GetRealOSWrapper()

	flag.Usage = showUsage

	check, generate, irMode := false, false, false
	flag.BoolVar(&c.verbose, "verbose", false, "print additional output")
	flag.BoolVar(&check, "check", false, "check that all the end-to-end tests in -inputs do not fail")
	flag.BoolVar(&generate, "generate", false, "generate fuzzing corpus based on -inputs")
	flag.BoolVar(&c.dump, "dump", false, "dumps shader input/output from fuzzer")
	flag.BoolVar(&irMode, "ir", false, "runs using IR fuzzer instead of WGSL fuzzer (This feature is a WIP)")
	flag.StringVar(&c.filter, "filter", "", "filter the fuzzing passes run to those with this substring")
	flag.StringVar(&c.inputs, "corpus", defaultWgslCorpusDir(c.osWrapper), "obsolete, use -inputs instead")
	flag.StringVar(&c.inputs, "inputs", defaultWgslCorpusDir(c.osWrapper), "the directory that holds the files to use")
	flag.StringVar(&c.build, "build", defaultBuildDir(c.osWrapper), "the build directory")
	flag.StringVar(&c.out, "out", "<tmp>", "the directory to store outputs to")
	flag.IntVar(&c.numProcesses, "j", runtime.NumCPU(), "number of concurrent fuzzers to run")
	flag.Parse()

	if check && generate {
		fmt.Println("cannot set -check and -generate flags at the same time")
		os.Exit(1)
	}

	switch {
	case check:
		c.cmdMode = TaskModeCheck
	case generate:
		c.cmdMode = TaskModeGenerate
	default:
		c.cmdMode = TaskModeRun
	}

	if irMode {
		c.fuzzMode = FuzzModeIr
	} else {
		c.fuzzMode = FuzzModeWgsl
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
	cmdConfig
	taskMode   TaskMode // specific task being run at this time, may be different from cmdConfig.cmdMode
	fuzzer     string   // path to the fuzzer binary, tint_wgsl_fuzzer or tint_ir_fuzzer
	assembler  string   // path to the test case assembler, tint_fuzz_as
	dictionary string   // path to dictionary to use for tint_wgsl_fuzzer
}

func run(c *cmdConfig) error {
	if !fileutils.IsDir(c.build, c.osWrapper) {
		return fmt.Errorf("build directory '%v' does not exist", c.build)
	}

	// Verify / create the output directory
	if c.out == "" || c.out == "<tmp>" {
		if tmp, err := c.osWrapper.MkdirTemp("", "tint_fuzz"); err == nil {
			defer c.osWrapper.RemoveAll(tmp)
			c.out = tmp
		} else {
			return err
		}
	}

	if !fileutils.IsDir(c.out, c.osWrapper) {
		return fmt.Errorf("output directory '%v' does not exist", c.out)
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
		default:
			err = fmt.Errorf("unknown task mode %d", t.taskMode)
		}
		if err != nil {
			return err
		}
	}
	return nil
}

// generateTaskConfig produces a taskConfig based off the supplied cmdConfig and specified TaskMode.
func generateTaskConfig(tm TaskMode, c *cmdConfig) (*taskConfig, error) {
	t := taskConfig{
		cmdConfig: *c,
		taskMode:  tm,
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
	case TaskModeCheck:
		fuzzerName := "tint_wgsl_fuzzer"
		if c.fuzzMode == FuzzModeIr {
			fuzzerName = "tint_ir_fuzzer"
		}
		dependencies = append(dependencies, depConfig{fuzzerName, &t.fuzzer})
	case TaskModeGenerate:
		if c.fuzzMode == FuzzModeIr {
			dependencies = append(dependencies, depConfig{"ir_fuzz_as", &t.assembler})
		}
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
			if !fileutils.IsExe(*config.path, t.osWrapper) {
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

	var pb *progressbar.ProgressBar
	if term.CanUseAnsiEscapeSequences() {
		pb = progressbar.New(os.Stdout, nil)
		defer pb.Stop()
	}
	var numDone uint32

	routine := func() error {
		for file := range remaining {
			atomic.AddUint32(&numDone, 1)
			if pb != nil {
				pb.Update(progressbar.Status{
					Total: len(files),
					Segments: []progressbar.Segment{
						{Count: int(atomic.LoadUint32(&numDone))},
					},
				})
			}

			if out, err := exec.Command(t.fuzzer, file).CombinedOutput(); err != nil {
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

// runFuzzer runs the fuzzer across t.numProcesses processes.
// The fuzzer will use t.inputs as the seed directory.
// New cases are written to t.out.
// Blocks until a fuzzer errors, or the process is interrupted.
func runFuzzer(t *taskConfig) error {
	ctx := utils.CancelOnInterruptContext(context.Background())
	ctx, cancel := context.WithCancel(ctx)
	defer cancel()

	args := generateFuzzerArgs(t)

	if t.verbose {
		fmt.Println("Using fuzzing cmd: " + t.fuzzer + " " + strings.Join(args, " "))
	}
	fmt.Println("running ", t.numProcesses, " fuzzer instances")

	errs := make(chan error, t.numProcesses)
	for i := 0; i < t.numProcesses; i++ {
		go func() {
			cmd := exec.CommandContext(ctx, t.fuzzer, args...)
			out := bytes.Buffer{}
			cmd.Stdout = &out
			cmd.Stderr = &out
			if t.verbose || t.dump {
				cmd.Stdout = io.MultiWriter(&out, os.Stdout)
				cmd.Stderr = io.MultiWriter(&out, os.Stderr)
			}
			if err := cmd.Run(); err != nil {
				if ctxErr := ctx.Err(); ctxErr != nil {
					errs <- ctxErr
				} else {
					errs <- fmt.Errorf("fuzzer process '%s' failed with error: %w\nOutput:\n%s", t.fuzzer, err, out.String())
				}
			} else {
				errs <- fmt.Errorf("fuzzer process '%s' unexpectedly terminated without error.\nOutput:\n%s", t.fuzzer, out.String())
			}
		}()
	}
	for err := range errs {
		return err
	}

	fmt.Println("done")
	return nil
}

// generateFuzzerArgs generates the arguments that need to be passed into the fuzzer binary call
func generateFuzzerArgs(t *taskConfig) []string {
	args := []string{t.out}

	if t.inputs != "" {
		args = append(args, t.inputs)
	}
	if t.dictionary != "" {
		args = append(args, "-dict="+t.dictionary)
	}
	if t.verbose {
		args = append(args, "--verbose")
	}
	if t.dump {
		args = append(args, "--dump")
	}
	if t.filter != "" {
		args = append(args, "--filter="+t.filter)
	}
	return args
}

// runCorpusGenerator converts a set of input test files into a fuzzer corpus
// The generator will use t.inputs as the source directory.
// The corpus will be written to t.out.
func runCorpusGenerator(t *taskConfig) error {
	switch t.fuzzMode {
	case FuzzModeWgsl:
		return runCorpusGeneratorWgsl(t)
	case FuzzModeIr:
		return runCorpusGeneratorIr(t)
	default:
		return fmt.Errorf("unknown fuzzer mode %d", t.fuzzMode)
	}
}

// runCorpusGeneratorWgsl converts a set of input test .wgsl files into a WGSL fuzzer corpus.
func runCorpusGeneratorWgsl(t *taskConfig) error {
	return gatherWgslFiles(t.inputs, t.out, t.osWrapper)
}

// runCorpusGeneratorIr converts a set of input test .wgsl files into an IR fuzzer corpus.
// It gathers the WGSL files, then forks out to an external binary (t.assembler) to perform the conversion.
func runCorpusGeneratorIr(t *taskConfig) error {
	tmp, err := t.osWrapper.MkdirTemp("", "wgsl_corpus_for_ir")
	if err != nil {
		return fmt.Errorf("failed to create temporary directory for WGSL files: %w", err)
	}
	defer t.osWrapper.RemoveAll(tmp)

	if err := gatherWgslFiles(t.inputs, tmp, t.osWrapper); err != nil {
		return fmt.Errorf("failed to gather WGSL files for IR corpus generation: %w", err)
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	args := []string{tmp, t.out}
	cmdStr := fmt.Sprintf("%s %s", t.assembler, strings.Join(args, " "))

	if t.verbose {
		fmt.Println("Using assembler cmd: " + cmdStr)
	}
	fmt.Println("running assembler")

	cmd := exec.CommandContext(ctx, t.assembler, args...)
	out := &bytes.Buffer{}
	cmd.Stdout = out
	cmd.Stderr = out
	if t.verbose {
		cmd.Stdout = io.MultiWriter(out, os.Stdout)
		cmd.Stderr = io.MultiWriter(out, os.Stderr)
	}

	if err := cmd.Run(); err != nil {
		return fmt.Errorf("failed to run IR corpus assembler.\n  command: %s\n  error: %w\n  output:\n%s", cmdStr, err, out.String())
	}

	fmt.Println("done")
	return nil
}

// gatherWgslFiles copies all the .wgsl files in a directory structure over to a flat directory
// structure, via replacing the path separators for the origins with underscores in the destination
// file names. It also filters out any '*.expected.*' files
func gatherWgslFiles(inputs string, out string, fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	fmt.Println("gathering and filtering .wgsl files")
	globPattern := filepath.Join(inputs, "**.wgsl")
	files, err := glob.Glob(globPattern, fsReaderWriter)
	if err != nil {
		return fmt.Errorf("failed to find .wgsl files with pattern '%v': %w", globPattern, err)
	}

	// Remove '*.expected.*'
	files = transform.Filter(files, func(s string) bool { return !strings.Contains(s, ".expected.") })

	// Map src file paths to dst filenames where the path separators have been converted to underscores
	mapping := make(map[string]string, len(files))
	for _, f := range files {
		// paths returned by glob.Glob are absolute, but only want to use the relative path in the dest name
		relPath, err := filepath.Rel(inputs, f)
		if err != nil {
			return fmt.Errorf("failed to calculate relative path for '%v' from base '%v': %w", f, inputs, err)
		}
		mapping[f] = strings.ReplaceAll(filepath.ToSlash(relPath), "/", "_")
	}

	for src, dest := range mapping {
		dstPath := filepath.Join(out, dest)
		if err := fileutils.CopyFile(dstPath, src, fsReaderWriter); err != nil {
			return fmt.Errorf("failed to copy '%v' to '%v': %w", src, dstPath, err)
		}
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
