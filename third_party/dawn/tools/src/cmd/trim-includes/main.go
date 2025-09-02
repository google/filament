// Copyright 2021 The Dawn & Tint Authors
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

// trim-includes is a tool to try removing unnecessary include statements from
// all .cc and .h files in the tint project.
//
// trim-includes removes each #include from each file, then runs the provided
// build script to determine whether the line was necessary. If the include is
// required, it is restored, otherwise it is left deleted.
// After all the #include statements have been tested, the file is
// clang-formatted and git staged.
package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
	"sync"
	"time"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

var (
	// Path to the build script to run after each attempting to remove each
	// #include
	buildScript = ""
)

func main() {
	if err := run(oswrapper.GetRealOSWrapper()); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
trim-includes is a tool to try removing unnecessary include statements from all
.cc and .h files in the tint project.

trim-includes removes each #include from each file, then runs the provided build
script to determine whether the line was necessary. If the include is required,
it is restored, otherwise it is left deleted.
After all the #include statements have been tested, the file is clang-formatted
and git staged.

Usage:
  trim-includes <path-to-build-script>`)
	os.Exit(1)
}

// TODO(crbug.com/416755658): Add unittest coverage once exec is handled via
// dependency injection.
func run(fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	flag.Parse()
	args := flag.Args()
	if len(args) < 1 {
		showUsage()
	}

	var err error
	buildScript, err = exec.LookPath(args[0])
	if err != nil {
		return err
	}
	buildScript, err = filepath.Abs(buildScript)
	if err != nil {
		return err
	}

	cfg, err := glob.LoadConfig("config.cfg", fsReaderWriter)
	if err != nil {
		return err
	}

	fmt.Println("Checking the project builds with no changes...")
	ok, err := tryBuild()
	if err != nil {
		return err
	}
	if !ok {
		return fmt.Errorf("Project does not build without edits")
	}

	fmt.Println("Scanning for files...")
	paths, err := glob.Scan(fileutils.DawnRoot(fsReaderWriter), cfg, fsReaderWriter)
	if err != nil {
		return err
	}

	fmt.Printf("Loading %v source files...\n", len(paths))
	files, err := loadFiles(paths, fsReaderWriter)
	if err != nil {
		return err
	}

	for fileIdx, file := range files {
		fmt.Printf("[%d/%d]: %v\n", fileIdx+1, len(files), file.path)
		includeLines := file.includesLineNumbers()
		enabled := make(map[int]bool, len(file.lines))
		for i := range file.lines {
			enabled[i] = true
		}
		for includeIdx, line := range includeLines {
			fmt.Printf("    [%d/%d]: %v", includeIdx+1, len(includeLines), file.lines[line])
			enabled[line] = false
			if err := file.save(enabled, fsReaderWriter); err != nil {
				return err
			}
			ok, err := tryBuild()
			if err != nil {
				return err
			}
			if ok {
				fmt.Printf(" removed\n")
				// Wait a bit so file timestamps get an opportunity to change.
				// Attempting to save too soon after a successful build may
				// result in a false-positive build.
				time.Sleep(time.Second)
			} else {
				fmt.Printf(" required\n")
				enabled[line] = true
			}
		}
		if err := file.save(enabled, fsReaderWriter); err != nil {
			return err
		}
		if err := file.format(); err != nil {
			return err
		}
		if err := file.stage(); err != nil {
			return err
		}
	}
	fmt.Println("Done")
	return nil
}

// Attempt to build the project. Returns true on successful build, false if
// there was a build failure.
func tryBuild() (bool, error) {
	cmd := exec.Command("sh", "-c", buildScript)
	out, err := cmd.CombinedOutput()
	switch err := err.(type) {
	case nil:
		return cmd.ProcessState.Success(), nil
	case *exec.ExitError:
		return false, nil
	default:
		return false, fmt.Errorf("Test failed with error: %v\n%v", err, string(out))
	}
}

type file struct {
	path     string
	lines    []string
	fsReader oswrapper.FilesystemReader
}

var includeRE = regexp.MustCompile(`^\s*#include (?:\"([^"]*)\"|:?\<([^"]*)\>)`)

// Returns the file path with the extension stripped
func stripExtension(path string) string {
	if dot := strings.IndexRune(path, '.'); dot > 0 {
		return path[:dot]
	}
	return path
}

// Returns the zero-based line numbers of all #include statements in the file
func (f *file) includesLineNumbers() []int {
	out := []int{}
	for i, l := range f.lines {
		matches := includeRE.FindStringSubmatch(l)
		if len(matches) == 0 {
			continue
		}

		include := matches[1]
		if include == "" {
			include = matches[2]
		}

		if strings.HasSuffix(stripExtension(f.path), stripExtension(include)) {
			// Don't remove #include for header of cc
			continue
		}

		out = append(out, i)
	}
	return out
}

// Saves the file, omitting the lines with the zero-based line number that are
// either not in `lines` or have a `false` value.
func (f *file) save(lines map[int]bool, fsWriter oswrapper.FilesystemWriter) error {
	content := []string{}
	for i, l := range f.lines {
		if lines[i] {
			content = append(content, l)
		}
	}
	data := []byte(strings.Join(content, "\n"))
	return fsWriter.WriteFile(f.path, data, 0666)
}

// Runs clang-format on the file
func (f *file) format() error {
	err := exec.Command("clang-format", "-i", f.path).Run()
	if err != nil {
		return fmt.Errorf("Couldn't format file '%v': %w", f.path, err)
	}
	return nil
}

// Runs git add on the file
func (f *file) stage() error {
	err := exec.Command("git", "-C", fileutils.DawnRoot(f.fsReader), "add", f.path).Run()
	if err != nil {
		return fmt.Errorf("Couldn't stage file '%v': %w", f.path, err)
	}
	return nil
}

// TODO(crbug.com/344014313): Add unittest coverage.
// Loads all the files with the given file paths, splitting their content into
// into lines.
func loadFiles(paths []string, fsReader oswrapper.FilesystemReader) ([]file, error) {
	wg := sync.WaitGroup{}
	wg.Add(len(paths))
	files := make([]file, len(paths))
	errs := make([]error, len(paths))
	for i, path := range paths {
		i, path := i, filepath.Join(fileutils.DawnRoot(fsReader), path)
		go func() {
			defer wg.Done()
			body, err := fsReader.ReadFile(path)
			if err != nil {
				errs[i] = fmt.Errorf("Failed to open %v: %w", path, err)
			} else {
				content := string(body)
				lines := strings.Split(content, "\n")
				files[i] = file{path: path, lines: lines, fsReader: fsReader}
			}
		}()
	}
	wg.Wait()
	for _, err := range errs {
		if err != nil {
			return nil, err
		}
	}
	return files, nil
}
