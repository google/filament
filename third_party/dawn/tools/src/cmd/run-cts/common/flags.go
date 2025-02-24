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

package common

import (
	"flag"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/subcmd"
	"dawn.googlesource.com/dawn/tools/src/term"
)

// Flags holds the common flags for all CTS runners
type Flags struct {
	Verbose          bool
	Colors           bool
	NumRunners       int
	Log              string
	ResultsPath      string
	ExpectationsPath string
	CTS              string // Path to the CTS directory
	Node             string // Path to the Node executable
	Npx              string // Path to the npx executable
}

func (f *Flags) Register() {
	flag.BoolVar(&f.Verbose, "verbose", false, "print extra information while testing")
	flag.BoolVar(&f.Colors, "colors", term.CanUseAnsiEscapeSequences(), "enable / disable colors")
	flag.IntVar(&f.NumRunners, "j", runtime.NumCPU()/2, "number of concurrent runners. 0 runs serially")
	flag.StringVar(&f.Log, "log", "", "path to log file of tests run and result")
	flag.StringVar(&f.ResultsPath, "output", "", "path to write test results file")
	flag.StringVar(&f.ExpectationsPath, "expect", "", "path to expectations file")
	flag.StringVar(&f.CTS, "cts", defaultCtsPath(), "root directory of WebGPU CTS")
	flag.StringVar(&f.Node, "node", fileutils.NodePath(), "path to node executable")
	flag.StringVar(&f.Npx, "npx", defaultNpxPath(), "path to npx executable")
}

// Process processes the flags, returning a State.
// Note: Ensure you call Close() on the returned State
func (f *Flags) Process() (*State, error) {
	s := &State{
		resultsPath: f.ResultsPath,
	}

	// Create the stdout writer
	s.Stdout = term.NewAnsiWriter(f.Colors)

	// Check CTS path
	if f.CTS == "" {
		return nil, subcmd.InvalidCLA()
	}
	if !fileutils.IsDir(f.CTS) {
		return nil, fmt.Errorf("'%v' is not a directory", f.CTS)
	}
	absCTS, err := filepath.Abs(f.CTS)
	if err != nil {
		return nil, fmt.Errorf("unable to get absolute path for '%v'", f.CTS)
	}
	f.CTS = absCTS

	// Build the logger, if needed
	if f.Log != "" {
		writer, err := os.Create(f.Log)
		if err != nil {
			return nil, fmt.Errorf("failed to open log '%v': %w", f.Log, err)
		}
		s.Log = NewLogger(writer)
		s.logWriter = writer
	}

	// If an expectations file was specified, load it.
	if f.ExpectationsPath != "" {
		if err := s.Expectations.Load(f.ExpectationsPath); err != nil {
			return nil, err
		}
	}

	// Find node
	if f.Node == "" {
		return nil, fmt.Errorf("cannot find path to node. Specify with --node")
	}

	s.CTS = NewCTS(f.CTS, f.Npx, f.Node)

	return s, nil
}

// State holds the common state for commands
type State struct {
	Stdout       io.WriteCloser
	Log          Logger
	Expectations Expectations
	CTS          CTS
	resultsPath  string
	logWriter    io.WriteCloser
}

// Close closes s.Stdout and saves the results to the results path (if set)
func (s *State) Close(results Results) error {
	if err := s.Stdout.Close(); err != nil {
		return err
	}

	// If an result file was specified, save results to it.
	if s.resultsPath != "" {
		expectations := Expectations{}
		for testCase, result := range results {
			expectations[testCase] = result.Status
		}

		if err := expectations.Save(s.resultsPath); err != nil {
			return err
		}
	}

	if s.logWriter != nil {
		if err := s.logWriter.Close(); err != nil {
			return fmt.Errorf("failed to close log file: %w", err)
		}
	}

	if err := AddToListingMeta(s.CTS.path, results); err != nil {
		return err
	}

	return nil
}

// defaultCtsPath looks for the webgpu-cts directory in dawn's third_party
// directory. This is used as the default for the --cts command line flag.
func defaultCtsPath() string {
	if dawnRoot := fileutils.DawnRoot(); dawnRoot != "" {
		cts := filepath.Join(dawnRoot, "third_party/webgpu-cts")
		if info, err := os.Stat(cts); err == nil && info.IsDir() {
			return cts
		}
	}
	return ""
}

// defaultNpxPath looks for the npx executable on PATH
func defaultNpxPath() string {
	if p, err := exec.LookPath("npx"); err == nil {
		return p
	}
	return ""
}
