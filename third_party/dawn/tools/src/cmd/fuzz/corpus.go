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
	"bytes"
	"context"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/transform"
)

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
	if t.verbose {
		args = append(args, "--verbose")
	}

	cmdStr := fmt.Sprintf("%s %s", t.assembler, strings.Join(args, " "))

	if t.verbose {
		fmt.Println("Using assembler cmd: " + cmdStr)
	}
	fmt.Println("running assembler")

	out := &bytes.Buffer{}
	var stdout, stderr io.Writer = out, out
	if t.verbose {
		stdout = io.MultiWriter(out, os.Stdout)
		stderr = io.MultiWriter(out, os.Stderr)
	}

	cmd := t.execWrapper.CommandContext(ctx, t.assembler, args...).WithStdout(stdout).WithStderr(stderr)

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
