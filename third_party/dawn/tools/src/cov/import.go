// Copyright 2022 The Dawn & Tint Authors
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

package cov

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

// File describes the coverage spans in a single source file.
type File struct {
	Path      string
	Covered   SpanList // Spans with coverage
	Uncovered SpanList // Compiled spans without coverage
}

// Coverage describes the coverage spans for all the source files for a single
// process invocation.
type Coverage struct {
	Files []File
}

// Env holds the environment settings for performing coverage processing.
type Env struct {
	Profdata string // path to the llvm-profdata tool
	Binary   string // path to the executable binary
	Cov      string // path to the llvm-cov tool (one of Cov or TurboCov must be supplied)
	TurboCov string // path to the turbo-cov tool (one of Cov or TurboCov must be supplied)
}

// TODO(crbug.com/344014313): Add unittest coverage.
// AllSourceFiles returns a *Coverage containing all the source files without
// coverage data. This populates the coverage view with files even if they
// didn't get compiled.
func (e Env) AllSourceFiles(fsReader oswrapper.FilesystemReader) *Coverage {
	var ignorePaths = map[string]bool{
		//
	}

	projectRoot := fileutils.DawnRoot(fsReader)

	// Gather all the source files to include them even if there is no coverage
	// information produced for these files. This highlights files that aren't
	// even compiled.
	cov := Coverage{}
	allFiles := map[string]struct{}{}
	fsReader.Walk(filepath.Join(projectRoot, "src"), func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		rel, err := filepath.Rel(projectRoot, path)
		if err != nil || ignorePaths[rel] {
			return filepath.SkipDir
		}
		if !info.IsDir() {
			switch filepath.Ext(path) {
			case ".h", ".c", ".cc", ".cpp", ".hpp":
				if _, seen := allFiles[rel]; !seen {
					cov.Files = append(cov.Files, File{Path: rel})
				}
			}
		}
		return nil
	})
	return &cov
}

// TODO(crbug.com/416755658): Add unittest coverage once exec is handled via
// dependency injection.
// Import uses the llvm-profdata and llvm-cov tools to import the coverage
// information from a .profraw file.
func (e Env) Import(profrawPath string, fsReader oswrapper.FilesystemReader) (*Coverage, error) {
	profdata := profrawPath + ".profdata"
	defer os.Remove(profdata)

	if e.Profdata == "" {
		return nil, fmt.Errorf("cov.Env.Profdata must be specified")
	}
	if e.TurboCov == "" && e.Cov == "" {
		return nil, fmt.Errorf("One of cov.Env.TurboCov or cov.Env.Cov must be specified")
	}

	if out, err := exec.Command(
		e.Profdata,
		"merge",
		"-sparse",
		profrawPath,
		"-output",
		profdata).CombinedOutput(); err != nil {
		return nil, fmt.Errorf("llvm-profdata errored: %w\n%v", err, string(out))
	}

	if e.TurboCov == "" {
		data, err := exec.Command(
			e.Cov,
			"export",
			e.Binary,
			"-instr-profile="+profdata,
			"-format=text",
			"-skip-expansions",
			"-skip-functions").CombinedOutput()
		if err != nil {
			return nil, fmt.Errorf("llvm-cov errored: %v\n%v", string(data), err)
		}
		cov, err := e.parseCov(data, fsReader)
		if err != nil {
			return nil, fmt.Errorf("failed to parse coverage json data: %w", err)
		}
		return cov, nil
	}

	data, err := exec.Command(e.TurboCov, e.Binary, profdata).CombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("turbo-cov errored: %v\n%v", string(data), err)
	}
	cov, err := e.parseTurboCov(data, fsReader)
	if err != nil {
		return nil, fmt.Errorf("failed to process turbo-cov output: %w", err)
	}

	return cov, nil
}

func appendSpan(spans []Span, span Span) []Span {
	if c := len(spans); c > 0 && spans[c-1].End == span.Start {
		spans[c-1].End = span.End
	} else {
		spans = append(spans, span)
	}
	return spans
}

// TODO(crbug.com/344014313): Add unittest coverage assuming test input for the
// raw argument can be reasonably created.
// https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
// https://stackoverflow.com/a/56792192
func (e Env) parseCov(raw []byte, fsReader oswrapper.FilesystemReader) (*Coverage, error) {
	// line int, col int, count int64, hasCount bool, isRegionEntry bool
	type segment []interface{}

	type file struct {
		// expansions ignored
		Name     string    `json:"filename"`
		Segments []segment `json:"segments"`
		// summary ignored
	}

	type data struct {
		Files []file `json:"files"`
	}

	root := struct {
		Data []data `json:"data"`
	}{}
	err := json.NewDecoder(bytes.NewReader(raw)).Decode(&root)
	if err != nil {
		return nil, err
	}

	projectRoot := fileutils.DawnRoot(fsReader)

	c := &Coverage{Files: make([]File, 0, len(root.Data[0].Files))}
	for _, f := range root.Data[0].Files {
		relpath, err := filepath.Rel(projectRoot, f.Name)
		if err != nil {
			return nil, err
		}
		if strings.HasPrefix(relpath, "..") {
			continue
		}
		file := File{Path: relpath}
		for sIdx := 0; sIdx+1 < len(f.Segments); sIdx++ {
			start := Location{(int)(f.Segments[sIdx][0].(float64)), (int)(f.Segments[sIdx][1].(float64))}
			end := Location{(int)(f.Segments[sIdx+1][0].(float64)), (int)(f.Segments[sIdx+1][1].(float64))}
			if covered := f.Segments[sIdx][2].(float64) != 0; covered {
				file.Covered = appendSpan(file.Covered, Span{start, end})
			} else {
				file.Uncovered = appendSpan(file.Uncovered, Span{start, end})
			}
		}
		if len(file.Covered) > 0 {
			c.Files = append(c.Files, file)
		}
	}

	return c, nil
}

// TODO(crbug.com/344014313): Add unittest coverage assuming test input for the
// data argument can be reasonably created.
// parseTurboCov parses coverage information from a `turbo-cov` file.
// See tools/src/cmd/turbo-cov/README.md for more information
func (e Env) parseTurboCov(data []byte, fsReader oswrapper.FilesystemReader) (*Coverage, error) {
	u32 := func() uint32 {
		out := binary.LittleEndian.Uint32(data)
		data = data[4:]
		return out
	}
	u8 := func() uint8 {
		out := data[0]
		data = data[1:]
		return out
	}
	str := func() string {
		len := u32()
		out := data[:len]
		data = data[len:]
		return string(out)
	}

	projectRoot := fileutils.DawnRoot(fsReader)

	numFiles := u32()
	c := &Coverage{Files: make([]File, 0, numFiles)}
	for i := 0; i < int(numFiles); i++ {
		path := str()
		relpath, err := filepath.Rel(projectRoot, path)
		if err != nil {
			return nil, err
		}
		if strings.HasPrefix(relpath, "..") {
			continue
		}

		file := File{Path: relpath}

		type segment struct {
			location Location
			count    int
			covered  bool
		}

		numSegements := u32()
		segments := make([]segment, numSegements)
		for j := range segments {
			segment := &segments[j]
			segment.location.Line = int(u32())
			segment.location.Column = int(u32())
			segment.count = int(u32())
			segment.covered = u8() != 0
		}

		for sIdx := 0; sIdx+1 < len(segments); sIdx++ {
			start := segments[sIdx].location
			end := segments[sIdx+1].location
			if segments[sIdx].covered {
				if segments[sIdx].count > 0 {
					file.Covered = appendSpan(file.Covered, Span{start, end})
				} else {
					file.Uncovered = appendSpan(file.Uncovered, Span{start, end})
				}
			}
		}

		if len(file.Covered) > 0 {
			c.Files = append(c.Files, file)
		}
	}

	return c, nil
}

// Path uniquely identifies a test that was run to produce coverage.
// Paths are split into a hierarchical sequence of strings, where the 0'th
// string represents the root of the hierarchy and the last string is typically
// the leaf name of the test.
type Path []string
