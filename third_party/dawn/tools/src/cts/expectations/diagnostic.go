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

package expectations

import (
	"fmt"
	"io"
	"strings"
)

// Severity is an enumerator of diagnostic severity
type Severity string

const (
	Error   Severity = "error"
	Warning Severity = "warning"
	Note    Severity = "note"
)

// Diagnostic holds a line, column, message and severity.
// Diagnostic also implements the 'error' interface.
type Diagnostic struct {
	Severity Severity
	Line     int // 1-based
	Column   int // 1-based
	Message  string
}

func (e Diagnostic) String() string {
	sb := &strings.Builder{}
	if e.Line > 0 {
		fmt.Fprintf(sb, "%v", e.Line)
		if e.Column > 0 {
			fmt.Fprintf(sb, ":%v", e.Column)
		}
		sb.WriteString(" ")
	}
	sb.WriteString(string(e.Severity))
	sb.WriteString(": ")
	sb.WriteString(e.Message)
	return sb.String()
}

// Error implements the 'error' interface.
func (e Diagnostic) Error() string { return e.String() }

// Diagnostics is a list of diagnostic
type Diagnostics []Diagnostic

// NumErrors returns number of errors in the diagnostics
func (l Diagnostics) NumErrors() int {
	count := 0
	for _, d := range l {
		if d.Severity == Error {
			count++
		}
	}
	return count
}

// Print prints the list of diagnostics to 'w'
func (l Diagnostics) Print(w io.Writer, path string) {
	for _, d := range l {
		fmt.Fprintf(w, "%v:%v %v\n", path, d.Line, d.Message)
	}
}
