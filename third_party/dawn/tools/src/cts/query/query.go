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

// Package query provides helpers for parsing and mutating WebGPU CTS queries.
//
// The full query syntax is described at:
// https://github.com/gpuweb/cts/blob/main/docs/terms.md#queries
//
// Note that this package supports a superset of the official CTS query syntax,
// as this package permits parsing and printing of queries that do not end in a
// wildcard, whereas the CTS requires that all queries end in wildcards unless
// they identify a specific test.
// For example, the following queries are considered valid by this  package, but
// would be rejected by the CTS:
// `suite`, `suite:file`, `suite:file,file`, `suite:file,file:test`.
//
// This relaxation is intentional as the Query type is used for constructing and
// reducing query trees, and always requiring a wildcard adds unnecessary
// complexity.
package query

import (
	"fmt"
	"strings"
)

// Query represents a WebGPU test query
// Example queries:
//
//	'suite'
//	'suite:*'
//	'suite:file'
//	'suite:file,*'
//	'suite:file,file'
//	'suite:file,file,*'
//	'suite:file,file,file:test'
//	'suite:file,file,file:test:*'
//	'suite:file,file,file:test,test:case;*'
type Query struct {
	Suite string
	Files string
	Tests string
	Cases string
}

// Target is the target of a query, either a Suite, File, Test or Case.
type Target int

// Enumerators of Target
const (
	// The query targets a suite
	Suite Target = iota
	// The query targets one or more files
	Files
	// The query targets one or more tests
	Tests
	// The query targets one or more test cases
	Cases

	TargetCount
)

// Format writes the Target to the fmt.State
func (l Target) Format(f fmt.State, verb rune) {
	switch l {
	case Suite:
		fmt.Fprint(f, "suite")
	case Files:
		fmt.Fprint(f, "files")
	case Tests:
		fmt.Fprint(f, "tests")
	case Cases:
		fmt.Fprint(f, "cases")
	default:
		fmt.Fprint(f, "<invalid>")
	}
}

// Delimiter constants used by the query format
const (
	TargetDelimiter = ":"
	FileDelimiter   = ","
	TestDelimiter   = ","
	CaseDelimiter   = ";"
)

// Parse parses a query string
func Parse(s string) Query {
	parts := strings.Split(s, TargetDelimiter)
	q := Query{}
	switch len(parts) {
	default:
		q.Cases = strings.Join(parts[3:], TargetDelimiter)
		fallthrough
	case 3:
		q.Tests = parts[2]
		fallthrough
	case 2:
		q.Files = parts[1]
		fallthrough
	case 1:
		q.Suite = parts[0]
	}
	return q
}

// AppendFiles returns a new query with the strings appended to the 'files'
func (q Query) AppendFiles(f ...string) Query {
	if len(f) > 0 {
		if q.Files == "" {
			q.Files = strings.Join(f, FileDelimiter)
		} else {
			q.Files = q.Files + FileDelimiter + strings.Join(f, FileDelimiter)
		}
	}
	return q
}

// SplitFiles returns the separated 'files' part of the query
func (q Query) SplitFiles() []string {
	if q.Files != "" {
		return strings.Split(q.Files, FileDelimiter)
	}
	return nil
}

// AppendTests returns a new query with the strings appended to the 'tests'
func (q Query) AppendTests(t ...string) Query {
	if len(t) > 0 {
		if q.Tests == "" {
			q.Tests = strings.Join(t, TestDelimiter)
		} else {
			q.Tests = q.Tests + TestDelimiter + strings.Join(t, TestDelimiter)
		}
	}
	return q
}

// SplitTests returns the separated 'tests' part of the query
func (q Query) SplitTests() []string {
	if q.Tests != "" {
		return strings.Split(q.Tests, TestDelimiter)
	}
	return nil
}

// AppendCases returns a new query with the strings appended to the 'cases'
func (q Query) AppendCases(c ...string) Query {
	if len(c) > 0 {
		if q.Cases == "" {
			q.Cases = strings.Join(c, CaseDelimiter)
		} else {
			q.Cases = q.Cases + CaseDelimiter + strings.Join(c, CaseDelimiter)
		}
	}
	return q
}

// SplitCases returns the separated 'cases' part of the query
func (q Query) SplitCases() []string {
	if q.Cases != "" {
		return strings.Split(q.Cases, CaseDelimiter)
	}
	return nil
}

// Case parameters is a map of parameter name to parameter value
type CaseParameters map[string]string

// CaseParameters returns all the case parameters of the query
func (q Query) CaseParameters() CaseParameters {
	if q.Cases != "" {
		out := CaseParameters{}
		for _, c := range strings.Split(q.Cases, CaseDelimiter) {
			idx := strings.IndexRune(c, '=')
			if idx < 0 {
				out[c] = ""
			} else {
				k, v := c[:idx], c[idx+1:]
				out[k] = v
			}
		}
		return out
	}
	return nil
}

// Append returns the query with the additional strings appended to the target
func (q Query) Append(t Target, n ...string) Query {
	switch t {
	case Suite:
		switch len(n) {
		case 0:
			return q
		case 1:
			if q.Suite != "" {
				panic("cannot append suite when query already contains suite")
			}
			return Query{Suite: n[0]}
		default:
			panic("cannot append more than one suite")
		}
	case Files:
		return q.AppendFiles(n...)
	case Tests:
		return q.AppendTests(n...)
	case Cases:
		return q.AppendCases(n...)
	}
	panic("invalid target")
}

// Target returns the target of the query
func (q Query) Target() Target {
	if q.Files != "" {
		if q.Tests != "" {
			if q.Cases != "" {
				return Cases
			}
			return Tests
		}
		return Files
	}
	return Suite
}

// IsWildcard returns true if the query ends with a wildcard
func (q Query) IsWildcard() bool {
	switch q.Target() {
	case Suite:
		return q.Suite == "*"
	case Files:
		return strings.HasSuffix(q.Files, "*")
	case Tests:
		return strings.HasSuffix(q.Tests, "*")
	case Cases:
		return strings.HasSuffix(q.Cases, "*")
	}
	panic("invalid target")
}

// String returns the query formatted as a string
func (q Query) String() string {
	sb := strings.Builder{}
	sb.WriteString(q.Suite)
	if q.Files != "" {
		sb.WriteString(TargetDelimiter)
		sb.WriteString(q.Files)
		if q.Tests != "" {
			sb.WriteString(TargetDelimiter)
			sb.WriteString(q.Tests)
			if q.Cases != "" {
				sb.WriteString(TargetDelimiter)
				sb.WriteString(q.Cases)
			}
		}
	}
	return sb.String()
}

// ExpectationFileString returns the query formatted as a string, suitable for
// use directly in expectation files.
func (q Query) ExpectationFileString() string {
	baseString := q.String()
	// Expectation files want a trailing ":" for test queries.
	if q.Target() == Tests && !q.IsWildcard() {
		baseString += TargetDelimiter
	}
	return baseString
}

// Compare compares the relative order of q and o, returning:
//
//	-1 if q should come before o
//	 1 if q should come after o
//	 0 if q and o are identical
func (q Query) Compare(o Query) int {
	for _, cmp := range []struct{ a, b string }{
		{q.Suite, o.Suite},
		{q.Files, o.Files},
		{q.Tests, o.Tests},
		{q.Cases, o.Cases},
	} {
		if cmp.a < cmp.b {
			return -1
		}
		if cmp.a > cmp.b {
			return 1
		}
	}

	return 0
}

// Contains returns true if q is a superset of o
func (q Query) Contains(o Query) bool {
	if q.Suite != o.Suite {
		return false
	}
	{
		a, b := q.SplitFiles(), o.SplitFiles()
		for i, f := range a {
			if f == "*" {
				return true
			}
			if i >= len(b) || b[i] != f {
				return false
			}
		}
		if len(a) < len(b) {
			return false
		}
	}
	{
		a, b := q.SplitTests(), o.SplitTests()
		for i, f := range a {
			if f == "*" {
				return true
			}
			if i >= len(b) || b[i] != f {
				return false
			}
		}
		if len(a) < len(b) {
			return false
		}
	}
	{
		a, b := q.CaseParameters(), o.CaseParameters()
		for key, av := range a {
			if bv, found := b[key]; found && av != bv {
				return false
			}
		}
	}
	return true
}

// Callback function for Query.Walk()
//
//	q is the query for the current segment.
//	t is the target of the query q.
//	n is the name of the new segment.
type WalkCallback func(q Query, t Target, n string) error

// Walk calls 'f' for each suite, file, test segment, and calls f once for all
// cases. If f returns an error then walking is immediately terminated and the
// error is returned.
func (q Query) Walk(f WalkCallback) error {
	p := Query{Suite: q.Suite}

	if err := f(p, Suite, q.Suite); err != nil {
		return err
	}

	for _, file := range q.SplitFiles() {
		p = p.AppendFiles(file)
		if err := f(p, Files, file); err != nil {
			return err
		}
	}

	for _, test := range q.SplitTests() {
		p = p.AppendTests(test)
		if err := f(p, Tests, test); err != nil {
			return err
		}
	}

	if q.Cases != "" {
		if err := f(q, Cases, q.Cases); err != nil {
			return err
		}
	}

	return nil
}
