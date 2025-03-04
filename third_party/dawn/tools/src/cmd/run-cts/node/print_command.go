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

package node

import (
	"bytes"
	"encoding/json"
	"fmt"
	"os/exec"
	"strings"
)

func PrintCommand(cmd *exec.Cmd, skipVSCodeInfo bool) {
	maybeQuote := func(s string) string {
		if strings.ContainsAny(s, ` ,()"`) {
			s = strings.ReplaceAll(s, `"`, `\"`)
			s = fmt.Sprintf("\"%v\"", s)
		}
		return s
	}

	output := &strings.Builder{}
	fmt.Fprintln(output, "Running:")
	fmt.Fprintf(output, "  Cmd: %v ", maybeQuote(cmd.Path))
	for i, arg := range cmd.Args[1:] {
		if i > 0 {
			fmt.Fprint(output, " ")
		}
		fmt.Fprint(output, maybeQuote(arg))
	}
	fmt.Fprintln(output)
	fmt.Fprintf(output, "  Dir: %v\n\n", cmd.Dir)

	if !skipVSCodeInfo {
		fmt.Fprint(output, "  For VS Code launch.json:\n")
		launchCmd := struct {
			Program string   `json:"program"`
			Args    []string `json:"args"`
			Cwd     string   `json:"cwd"`
		}{
			Program: cmd.Path,
			Args:    cmd.Args[1:],
			Cwd:     cmd.Dir,
		}

		b := &bytes.Buffer{}
		e := json.NewEncoder(b)
		e.SetIndent("", "    ")
		e.Encode(launchCmd)
		s := b.String()
		// Remove object braces and add trailing comma
		s = strings.TrimPrefix(s, "{\n")
		s = strings.TrimSuffix(s, "\n}\n") + ",\n"
		fmt.Fprintln(output, s)
	}

	fmt.Print(output.String())
}
