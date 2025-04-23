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

// Package ast defines AST nodes that are produced by the Tint intrinsic
// definition parser
package ast

import (
	"fmt"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/tok"
)

// AST is the parsed syntax tree of the intrinsic definition file
type AST struct {
	Enums        []EnumDecl
	Types        []TypeDecl
	Matchers     []MatcherDecl
	Builtins     []IntrinsicDecl
	Constructors []IntrinsicDecl
	Converters   []IntrinsicDecl
	Operators    []IntrinsicDecl
}

func (a AST) String() string {
	sb := strings.Builder{}
	for _, e := range a.Enums {
		fmt.Fprintf(&sb, "%v", e)
		fmt.Fprintln(&sb)
	}
	for _, p := range a.Types {
		fmt.Fprintf(&sb, "%v", p)
		fmt.Fprintln(&sb)
	}
	for _, m := range a.Matchers {
		fmt.Fprintf(&sb, "%v", m)
		fmt.Fprintln(&sb)
	}
	for _, b := range a.Builtins {
		fmt.Fprintf(&sb, "%v", b)
		fmt.Fprintln(&sb)
	}
	for _, o := range a.Constructors {
		fmt.Fprintf(&sb, "%v", o)
		fmt.Fprintln(&sb)
	}
	for _, o := range a.Converters {
		fmt.Fprintf(&sb, "%v", o)
		fmt.Fprintln(&sb)
	}
	for _, o := range a.Operators {
		fmt.Fprintf(&sb, "%v", o)
		fmt.Fprintln(&sb)
	}
	return sb.String()
}

// EnumDecl describes an enumerator
type EnumDecl struct {
	Source     tok.Source
	Name       string
	Attributes Attributes
	Entries    []EnumEntry
}

// Format implements the fmt.Formatter interface
func (e EnumDecl) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "enum %v {\n", e.Name)
	for _, e := range e.Entries {
		fmt.Fprintf(w, "  %v\n", e)
	}
	fmt.Fprintf(w, "}\n")
}

// EnumEntry describes an entry in a enumerator
type EnumEntry struct {
	Source     tok.Source
	Name       string
	Attributes Attributes
}

// Format implements the fmt.Formatter interface
func (e EnumEntry) Format(w fmt.State, verb rune) {
	if len(e.Attributes) > 0 {
		fmt.Fprintf(w, "%v %v", e.Attributes, e.Name)
	} else {
		fmt.Fprint(w, e.Name)
	}
}

// MatcherDecl describes a matcher declaration
type MatcherDecl struct {
	Source  tok.Source
	Name    string
	Options MatcherOptions
}

// Format implements the fmt.Formatter interface
func (m MatcherDecl) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "match %v", m.Name)
	fmt.Fprintf(w, ": ")
	m.Options.Format(w, verb)
}

// IntrinsicKind is either a Builtin, Operator, Initializer or Converter
type IntrinsicKind string

const (
	// Builtin is a builtin function (max, fract, etc).
	// Declared with 'fn'.
	Builtin IntrinsicKind = "builtin"
	// Operator is a unary or binary operator.
	// Declared with 'op'.
	Operator IntrinsicKind = "operator"
	// Constructor is a value constructor function.
	// Declared with 'init'.
	Constructor IntrinsicKind = "constructor"
	// Converter is a value conversion function.
	// Declared with 'conv'.
	Converter IntrinsicKind = "converter"
)

// IntrinsicDecl describes a builtin or operator declaration
type IntrinsicDecl struct {
	Source                 tok.Source
	Kind                   IntrinsicKind
	Name                   string
	Attributes             Attributes
	ImplicitTemplateParams []TemplateParam
	ExplicitTemplateParams []TemplateParam
	Parameters             Parameters
	ReturnType             *TemplatedName
}

// Format implements the fmt.Formatter interface
func (i IntrinsicDecl) Format(w fmt.State, verb rune) {
	switch i.Kind {
	case Builtin:
		fmt.Fprintf(w, "fn ")
	case Operator:
		fmt.Fprintf(w, "op ")
	case Constructor:
		fmt.Fprintf(w, "ctor ")
	case Converter:
		fmt.Fprintf(w, "conv ")
	}

	fmt.Fprintf(w, "%v", i.Name)
	if len(i.ExplicitTemplateParams) > 0 {
		fmt.Fprintf(w, "<")
		formatList(w, i.ExplicitTemplateParams)
		fmt.Fprintf(w, ">")
	}
	if len(i.ImplicitTemplateParams) > 0 {
		fmt.Fprintf(w, "[")
		formatList(w, i.ImplicitTemplateParams)
		fmt.Fprintf(w, "]")
	}
	i.Parameters.Format(w, verb)
	if i.ReturnType != nil {
		fmt.Fprintf(w, " -> ")
		i.ReturnType.Format(w, verb)
	}
}

// Parameters is a list of parameter
type Parameters []Parameter

// Format implements the fmt.Formatter interface
func (l Parameters) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "(")
	for i, p := range l {
		if i > 0 {
			fmt.Fprintf(w, ", ")
		}
		p.Attributes.Format(w, verb)
		p.Format(w, verb)
	}
	fmt.Fprintf(w, ")")
}

// Parameter describes a single parameter of a function
type Parameter struct {
	Source     tok.Source
	Attributes Attributes
	Name       string // Optional
	Type       TemplatedName
}

// Format implements the fmt.Formatter interface
func (p Parameter) Format(w fmt.State, verb rune) {
	if p.Name != "" {
		fmt.Fprintf(w, "%v: ", p.Name)
	}
	p.Type.Format(w, verb)
}

// MatcherOptions is a list of TemplatedNames or MemberNames
type MatcherOptions struct {
	Types TemplatedNames
	Enums MemberNames
}

// Format implements the fmt.Formatter interface
func (o MatcherOptions) Format(w fmt.State, verb rune) {
	for i, mo := range o.Types {
		if i > 0 {
			fmt.Fprintf(w, " | ")
		}
		mo.Format(w, verb)
	}
	for i, mo := range o.Enums {
		if i > 0 {
			fmt.Fprintf(w, " | ")
		}
		mo.Format(w, verb)
	}
}

// TemplatedNames is a list of TemplatedName
// Example:
//
//	a<b>, c<d, e>
type TemplatedNames []TemplatedName

// Format implements the fmt.Formatter interface
func (l TemplatedNames) Format(w fmt.State, verb rune) {
	formatList(w, l)
}

// TemplatedName is an identifier with optional templated arguments
// Example:
//
//	vec<N, T>
type TemplatedName struct {
	Source       tok.Source
	Name         string
	TemplateArgs TemplatedNames
}

// Format implements the fmt.Formatter interface
func (t TemplatedName) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "%v", t.Name)
	if len(t.TemplateArgs) > 0 {
		fmt.Fprintf(w, "<")
		t.TemplateArgs.Format(w, verb)
		fmt.Fprintf(w, ">")
	}
}

// MemberNames is a list of MemberName
// Example:
//
//	a.b, c.d
type MemberNames []MemberName

// Format implements the fmt.Formatter interface
func (l MemberNames) Format(w fmt.State, verb rune) {
	formatList(w, l)
}

// MemberName is two identifiers separated by a dot (Owner.Member)
type MemberName struct {
	Source tok.Source
	Owner  string
	Member string
}

// Format implements the fmt.Formatter interface
func (t MemberName) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "%v.%v", t.Owner, t.Member)
}

// TypeDecl describes a type declaration
type TypeDecl struct {
	Source         tok.Source
	Attributes     Attributes
	Name           string
	TemplateParams []TemplateParam
}

// Format implements the fmt.Formatter interface
func (p TypeDecl) Format(w fmt.State, verb rune) {
	if len(p.Attributes) > 0 {
		p.Attributes.Format(w, verb)
		fmt.Fprintf(w, " type %v", p.Name)
	}
	fmt.Fprintf(w, "type %v", p.Name)
	if len(p.TemplateParams) > 0 {
		fmt.Fprintf(w, "<")
		formatList(w, p.TemplateParams)
		fmt.Fprintf(w, ">")
	}
}

// TemplateParam describes a template parameter with optional type
// Example:
//
//	<Name>
//	<Name: Type>
type TemplateParam struct {
	Source tok.Source
	Name   string
	Type   TemplatedName // Optional
}

// Format implements the fmt.Formatter interface
func (t TemplateParam) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "%v", t.Name)
	if t.Type.Name != "" {
		fmt.Fprintf(w, " : ")
		t.Type.Format(w, verb)
	}
}

// Attributes is a list of Attribute
// Example:
//
//	[[a(x), b(y)]]
type Attributes []Attribute

// Format implements the fmt.Formatter interface
func (l Attributes) Format(w fmt.State, verb rune) {
	for _, d := range l {
		fmt.Fprint(w, "@")
		d.Format(w, verb)
		fmt.Fprint(w, " ")
	}
}

// Take looks up the attribute with the given name. If the attribute is found
// it is removed from the Attributes list and returned, otherwise nil is
// returned and the Attributes are not altered.
func (l *Attributes) Take(name string) *Attribute {
	for i, a := range *l {
		if a.Name == name {
			*l = append((*l)[:i], (*l)[i+1:]...)
			return &a
		}
	}
	return nil
}

// Attribute describes a single attribute
// Example:
//
//	@a(x)
type Attribute struct {
	Source tok.Source
	Name   string
	Values []any
}

// Format implements the fmt.Formatter interface
func (d Attribute) Format(w fmt.State, verb rune) {
	fmt.Fprintf(w, "%v", d.Name)
	if len(d.Values) > 0 {
		fmt.Fprintf(w, "(")
		formatList(w, d.Values)
		fmt.Fprintf(w, ")")
	}
}

func formatList[T any](w fmt.State, list []T) {
	for i, v := range list {
		if i > 0 {
			fmt.Fprint(w, ", ")
		}
		fmt.Fprintf(w, "%v", v)
	}
}
