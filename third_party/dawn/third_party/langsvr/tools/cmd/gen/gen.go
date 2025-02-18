// Copyright 2024 The langsvr Authors
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

// gen generates the Language Server Protocol C++ source files from lsp.json
package main

import (
	"bytes"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/google/langsvr/tools/cmd/gen/json"
	"github.com/google/langsvr/tools/cmd/gen/resolver"
	"github.com/google/langsvr/tools/fileutils"
	"github.com/google/langsvr/tools/template"
)

const lspVersion = "3.17"

type fileAndLine struct {
	file string
	line int
}

func main() {
	if err := run(); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

func run() error {
	projectRoot := fileutils.ProjectRoot()

	jsonPath := filepath.Join(projectRoot, "third_party/lsprotocol/generator/lsp.json")
	jsonFile, err := os.Open(jsonPath)
	if err != nil {
		return err
	}
	defer jsonFile.Close()

	model, err := json.Decode(jsonFile)
	if err != nil {
		return err
	}

	protocol, err := resolver.Resolve(model)
	if err != nil {
		return err
	}

	for _, relPath := range []string{"include/langsvr/lsp/lsp.h", "src/lsp/lsp.cc"} {
		tmplRelPath := relPath + ".tmpl"
		t, err := template.FromFile(filepath.Join(projectRoot, tmplRelPath))
		if err != nil {
			return err
		}
		outPath := filepath.Join(projectRoot, relPath)

		// Load the old file
		existing, err := os.ReadFile(outPath)
		if err != nil {
			existing = nil
		}

		buffer := &bytes.Buffer{}
		buffer.WriteString(header(string(existing), filepath.ToSlash(tmplRelPath), "//"))
		if err := t.Run(buffer, protocol, template.Functions{}); err != nil {
			return err
		}

		formatted, err := ClangFormat(buffer.String())
		if err != nil {
			return err
		}

		if err := os.WriteFile(outPath, []byte(formatted), 0666); err != nil {
			return err
		}

	}
	return nil
}

var re = regexp.MustCompile(`• Copyright (\d+) The`)

// header returns the header text to emit at the top of the file.
// header takes the existing file content, so that the copyright year can be
// preserved. For a new file this is an empty string.
func header(existing, templatePath, comment string) string {
	const text = `• Copyright %v The langsvr Authors
	•
	• Redistribution and use in source and binary forms, with or without
	• modification, are permitted provided that the following conditions are met:
	•
	• 1. Redistributions of source code must retain the above copyright notice, this
	•    list of conditions and the following disclaimer.
	•
	• 2. Redistributions in binary form must reproduce the above copyright notice,
	•    this list of conditions and the following disclaimer in the documentation
	•    and/or other materials provided with the distribution.
	•
	• 3. Neither the name of the copyright holder nor the names of its
	•    contributors may be used to endorse or promote products derived from
	•    this software without specific prior written permission.
	•
	• THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	• AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	• IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	• DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	• FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	• DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	• SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	• CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	• OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	• OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	‣
	• File generated by 'tools/cmd/gen' using the template:
	•   %v
	•
	• To regenerate run: 'go run ./tools/cmd/gen'
	•
	•                       Do not modify this file directly
	‣
	`

	copyrightYear := time.Now().Year()

	// Replace comment characters with '•'
	existing = strings.ReplaceAll(existing, comment, "•")

	// Look for the existing copyright year
	if match := re.FindStringSubmatch(string(existing)); len(match) == 2 {
		if year, err := strconv.Atoi(match[1]); err == nil {
			copyrightYear = year
		}
	}

	// Replace '•' with comment characters, '‣' with a line of comment characters
	out := strings.ReplaceAll(text, "•", comment)
	out = strings.ReplaceAll(out, "‣", strings.Repeat(comment, 80/len(comment)))

	return fmt.Sprintf(out, copyrightYear, templatePath)
}

var clangFormatPath string

// ClangFormat invokes clang-format to format the file content 'src'.
// Returns the formatted file.
func ClangFormat(src string) (string, error) {
	if clangFormatPath == "" {
		path, err := exec.LookPath("clang-format")
		if err != nil {
			return "", err
		}
		clangFormatPath = path
	}
	cmd := exec.Command(clangFormatPath)
	cmd.Stdin = strings.NewReader(src)
	out, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("clang-format failed:\n%v\n%v", string(out), err)
	}
	return string(out), nil
}
