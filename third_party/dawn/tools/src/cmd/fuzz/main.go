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
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"sync/atomic"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/progressbar"
	"dawn.googlesource.com/dawn/tools/src/term"
	"dawn.googlesource.com/dawn/tools/src/transform"
	"dawn.googlesource.com/dawn/tools/src/utils"
)

const (
	wgslDictionaryRelPath = "src/tint/cmd/fuzz/wgsl/dictionary.txt"
)

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
fuzz is a helper for running the tint fuzzer executables

fuzz can check that the corpus does not trigger any issues with the fuzzers, and
simplify running of the fuzzers locally.

usage:
  fuzz [flags...]`)
	flag.PrintDefaults()
	fmt.Println(``)
	os.Exit(1)
}

func run() error {
	t := tool{}

	check := false
	build := ""
	flag.BoolVar(&t.verbose, "verbose", false, "print additional output")
	flag.BoolVar(&check, "check", false, "check that all the end-to-end test do not fail")
	flag.BoolVar(&t.dump, "dump", false, "dumps shader input/output from fuzzer")
	flag.StringVar(&t.filter, "filter", "", "filter the fuzzers run to those with this substring")
	flag.StringVar(&t.corpus, "corpus", defaultCorpusDir(), "the corpus directory")
	flag.StringVar(&build, "build", defaultBuildDir(), "the build directory")
	flag.StringVar(&t.out, "out", "<tmp>", "the directory to hold generated test files")
	flag.IntVar(&t.numProcesses, "j", runtime.NumCPU(), "number of concurrent fuzzers to run")
	flag.Parse()

	if t.numProcesses < 1 {
		t.numProcesses = 1
	}

	if !fileutils.IsDir(build) {
		return fmt.Errorf("build directory '%v' does not exist", build)
	}

	// Verify / create the output directory
	if t.out == "" || t.out == "<tmp>" {
		if tmp, err := os.MkdirTemp("", "tint_fuzz"); err == nil {
			defer os.RemoveAll(tmp)
			t.out = tmp
		} else {
			return err
		}
	}
	if !fileutils.IsDir(t.out) {
		return fmt.Errorf("output directory '%v' does not exist", t.out)
	}

	// Verify all of the fuzzer executables are present
	for _, fuzzer := range []struct {
		name string
		path *string
	}{
		{"tint_wgsl_fuzzer", &t.wgslFuzzer},
	} {
		*fuzzer.path = filepath.Join(build, fuzzer.name+fileutils.ExeExt)
		if !fileutils.IsExe(*fuzzer.path) {
			return fmt.Errorf("fuzzer not found at '%v'", *fuzzer.path)
		}
	}

	// If --check was passed, then just ensure that all the files in the corpus
	// directory don't upset the fuzzers
	if check {
		return t.check()
	}

	// Run the fuzzers
	return t.run()
}

type tool struct {
	verbose      bool   // prints the name of each fuzzer before running
	dump         bool   // dumps shader input/output from fuzzer
	filter       string // filter fuzzers to those with this substring
	corpus       string // directory of test files
	out          string // where to emit new test files
	wgslFuzzer   string // path to tint_wgsl_fuzzer
	numProcesses int    // number of concurrent processes to spawn
}

// check() runs the fuzzers against all the .wgsl files under the corpus directory,
// ensuring that the fuzzers do not error for the given file.
func (t tool) check() error {
	wgslFiles, err := glob.Glob(filepath.Join(t.corpus, "**.wgsl"))
	if err != nil {
		return err
	}

	// Remove '*.expected.wgsl'
	wgslFiles = transform.Filter(wgslFiles, func(s string) bool { return !strings.Contains(s, ".expected.") })

	log.Printf("checking %v files...\n", len(wgslFiles))

	remaining := transform.SliceToChan(wgslFiles)

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
					Total: len(wgslFiles),
					Segments: []progressbar.Segment{
						{Count: int(atomic.LoadUint32(&numDone))},
					},
				})
			}

			if out, err := exec.Command(t.wgslFuzzer, file).CombinedOutput(); err != nil {
				_, fuzzer := filepath.Split(t.wgslFuzzer)
				return fmt.Errorf("%v run with %v failed with %v\n\n%v", fuzzer, file, err, string(out))
			}
		}
		return nil
	}

	if err = utils.RunConcurrent(t.numProcesses, routine); err != nil {
		return err
	}

	log.Printf("done")
	return nil
}

// run() runs the fuzzers across t.numProcesses processes.
// The fuzzers will use t.corpus as the seed directory.
// New cases are written to t.out.
// Blocks until a fuzzer errors, or the process is interrupted.
func (t tool) run() error {
	ctx := utils.CancelOnInterruptContext(context.Background())
	ctx, cancel := context.WithCancel(ctx)
	defer cancel()

	dictPath, err := filepath.Abs(filepath.Join(fileutils.DawnRoot(), wgslDictionaryRelPath))
	if err != nil || !fileutils.IsFile(dictPath) {
		return fmt.Errorf("failed to obtain the dictionary.txt path: %w", err)
	}

	args := []string{t.out, t.corpus,
		"-dict=" + dictPath,
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

	fmt.Println("running", t.numProcesses, "fuzzer instances")
	errs := make(chan error, t.numProcesses)
	for i := 0; i < t.numProcesses; i++ {
		go func() {
			cmd := exec.CommandContext(ctx, t.wgslFuzzer, args...)
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
					_, fuzzer := filepath.Split(t.wgslFuzzer)
					errs <- fmt.Errorf("%v failed with %v\n\n%v", fuzzer, err, out.String())
				}
			} else {
				errs <- fmt.Errorf("fuzzer unexpectedly terminated without error:\n%v", out.String())
			}
		}()
	}
	for err := range errs {
		return err
	}
	return nil
}

func defaultCorpusDir() string {
	return filepath.Join(fileutils.DawnRoot(), "test/tint")
}

func defaultBuildDir() string {
	return filepath.Join(fileutils.DawnRoot(), "out/active")
}
