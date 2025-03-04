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

// Package tok defines tokens that are produced by the Tint intrinsic definition
// lexer
package tok

import "fmt"

// Kind is an enumerator of token kinds
type Kind string

// Token enumerator types
const (
	InvalidToken Kind = "<invalid>"
	Identifier   Kind = "ident"
	Integer      Kind = "integer"
	Float        Kind = "float"
	String       Kind = "string"
	Match        Kind = "match"
	Import       Kind = "import"
	Implicit     Kind = "implicit"
	Function     Kind = "fn"
	Operator     Kind = "op"
	Constructor  Kind = "ctor"
	Converter    Kind = "conv"
	Type         Kind = "type"
	Enum         Kind = "enum"
	And          Kind = "&"
	AndAnd       Kind = "&&"
	Arrow        Kind = "->"
	Attr         Kind = "@"
	Assign       Kind = "="
	Colon        Kind = ":"
	Comma        Kind = ","
	Complement   Kind = "~"
	Divide       Kind = "/"
	Dot          Kind = "."
	Equal        Kind = "=="
	Ge           Kind = ">="
	Gt           Kind = ">"
	Lbrace       Kind = "{"
	Le           Kind = "<="
	Lparen       Kind = "("
	Lt           Kind = "<"
	Minus        Kind = "-"
	Modulo       Kind = "%"
	Not          Kind = "!"
	NotEqual     Kind = "!="
	Or           Kind = "|"
	OrOr         Kind = "||"
	Plus         Kind = "+"
	Rbrace       Kind = "}"
	Rparen       Kind = ")"
	Shl          Kind = "<<"
	Shr          Kind = ">>"
	Star         Kind = "*"
	Xor          Kind = "^"
	Lbracket     Kind = "["
	Rbracket     Kind = "]"
)

// Invalid represents an invalid token
var Invalid = Token{Kind: InvalidToken}

// Location describes a rune location in the source code
type Location struct {
	// 1-based line index
	Line int
	// 1-based column index
	Column int
	// 0-based rune index
	Rune int
	// Optional file path
	Filepath string
}

// Format implements the fmt.Formatter interface
func (l Location) Format(w fmt.State, verb rune) {
	if w.Flag('+') {
		if l.Filepath != "" {
			fmt.Fprintf(w, "%v:%v:%v[%v]", l.Filepath, l.Line, l.Column, l.Rune)
		} else {
			fmt.Fprintf(w, "%v:%v[%v]", l.Line, l.Column, l.Rune)
		}
	} else {
		if l.Filepath != "" {
			fmt.Fprintf(w, "%v:%v:%v", l.Filepath, l.Line, l.Column)
		} else {
			fmt.Fprintf(w, "%v:%v", l.Line, l.Column)
		}
	}
}

// Source describes a start and end range in the source code
type Source struct {
	S, E Location
}

// IsValid returns true if the source is valid
func (s Source) IsValid() bool {
	return s.S.Line != 0 && s.S.Column != 0 && s.E.Line != 0 && s.E.Column != 0
}

// Format implements the fmt.Formatter interface
func (s Source) Format(w fmt.State, verb rune) {
	if w.Flag('+') {
		fmt.Fprint(w, "[")
		s.S.Format(w, verb)
		fmt.Fprint(w, " - ")
		s.E.Format(w, verb)
		fmt.Fprint(w, "]")
	} else {
		s.S.Format(w, verb)
	}
}

// Token describes a parsed token
type Token struct {
	Kind   Kind
	Runes  []rune
	Source Source
}

// Format implements the fmt.Formatter interface
func (t Token) Format(w fmt.State, verb rune) {
	fmt.Fprint(w, "[")
	t.Source.Format(w, verb)
	fmt.Fprint(w, " ")
	fmt.Fprint(w, t.Kind)
	fmt.Fprint(w, " ")
	fmt.Fprint(w, string(t.Runes))
	fmt.Fprint(w, "]")
}
