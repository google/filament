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

package query_test

import (
	"fmt"
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"github.com/google/go-cmp/cmp"
	"github.com/stretchr/testify/assert"
)

var Q = query.Parse

func TestTargetFormat(t *testing.T) {
	type Test struct {
		target query.Target
		expect string
	}

	for _, test := range []Test{
		{query.Suite, "suite"},
		{query.Files, "files"},
		{query.Tests, "tests"},
		{query.Cases, "cases"},
		{query.Target(-1), "<invalid>"},
	} {
		s := strings.Builder{}
		_, err := fmt.Fprint(&s, test.target)
		if err != nil {
			t.Errorf("Fprint() returned %v", err)
			continue
		}
		if diff := cmp.Diff(s.String(), test.expect); diff != "" {
			t.Errorf("Fprint('%v')\n%v", test.target, diff)
		}
	}
}

func TestAppendFiles(t *testing.T) {
	type Test struct {
		base   query.Query
		files  []string
		expect query.Query
	}

	for _, test := range []Test{
		{Q("suite"), []string{}, Q("suite")},
		{Q("suite"), []string{"x"}, Q("suite:x")},
		{Q("suite"), []string{"x", "y"}, Q("suite:x,y")},
		{Q("suite:a"), []string{}, Q("suite:a")},
		{Q("suite:a"), []string{"x"}, Q("suite:a,x")},
		{Q("suite:a"), []string{"x", "y"}, Q("suite:a,x,y")},
		{Q("suite:a,b"), []string{}, Q("suite:a,b")},
		{Q("suite:a,b"), []string{"x"}, Q("suite:a,b,x")},
		{Q("suite:a,b"), []string{"x", "y"}, Q("suite:a,b,x,y")},
		{Q("suite:a,b:c"), []string{}, Q("suite:a,b:c")},
		{Q("suite:a,b:c"), []string{"x"}, Q("suite:a,b,x:c")},
		{Q("suite:a,b:c"), []string{"x", "y"}, Q("suite:a,b,x,y:c")},
		{Q("suite:a,b:c,d"), []string{}, Q("suite:a,b:c,d")},
		{Q("suite:a,b:c,d"), []string{"x"}, Q("suite:a,b,x:c,d")},
		{Q("suite:a,b:c,d"), []string{"x", "y"}, Q("suite:a,b,x,y:c,d")},
		{Q("suite:a,b:c,d:e"), []string{}, Q("suite:a,b:c,d:e")},
		{Q("suite:a,b:c,d:e"), []string{"x"}, Q("suite:a,b,x:c,d:e")},
		{Q("suite:a,b:c,d:e"), []string{"x", "y"}, Q("suite:a,b,x,y:c,d:e")},
		{Q("suite:a,b:c,d:e;f"), []string{}, Q("suite:a,b:c,d:e;f")},
		{Q("suite:a,b:c,d:e;f"), []string{"x"}, Q("suite:a,b,x:c,d:e;f")},
		{Q("suite:a,b:c,d:e;f"), []string{"x", "y"}, Q("suite:a,b,x,y:c,d:e;f")},
	} {
		got := test.base.AppendFiles(test.files...)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("'%v'.AppendFiles(%v)\n%v", test.base, test.files, diff)
		}
	}
}

func TestSplitFiles(t *testing.T) {
	type Test struct {
		query  query.Query
		expect []string
	}

	for _, test := range []Test{
		{Q("suite"), nil},
		{Q("suite:a"), []string{"a"}},
		{Q("suite:a,b"), []string{"a", "b"}},
		{Q("suite:a,b:c"), []string{"a", "b"}},
		{Q("suite:a,b:c,d"), []string{"a", "b"}},
		{Q("suite:a,b:c,d:e"), []string{"a", "b"}},
		{Q("suite:a,b:c,d:e;f"), []string{"a", "b"}},
	} {
		got := test.query.SplitFiles()
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("'%v'.SplitFiles()\n%v", test.query, diff)
		}
	}
}

func TestAppendTests(t *testing.T) {
	type Test struct {
		base   query.Query
		files  []string
		expect query.Query
	}

	for _, test := range []Test{
		{Q("suite"), []string{}, Q("suite")},
		{Q("suite"), []string{"x"}, Q("suite::x")},
		{Q("suite"), []string{"x", "y"}, Q("suite::x,y")},
		{Q("suite:a"), []string{}, Q("suite:a")},
		{Q("suite:a"), []string{"x"}, Q("suite:a:x")},
		{Q("suite:a"), []string{"x", "y"}, Q("suite:a:x,y")},
		{Q("suite:a,b"), []string{}, Q("suite:a,b")},
		{Q("suite:a,b"), []string{"x"}, Q("suite:a,b:x")},
		{Q("suite:a,b"), []string{"x", "y"}, Q("suite:a,b:x,y")},
		{Q("suite:a,b:c"), []string{}, Q("suite:a,b:c")},
		{Q("suite:a,b:c"), []string{"x"}, Q("suite:a,b:c,x")},
		{Q("suite:a,b:c"), []string{"x", "y"}, Q("suite:a,b:c,x,y")},
		{Q("suite:a,b:c,d"), []string{}, Q("suite:a,b:c,d")},
		{Q("suite:a,b:c,d"), []string{"x"}, Q("suite:a,b:c,d,x")},
		{Q("suite:a,b:c,d"), []string{"x", "y"}, Q("suite:a,b:c,d,x,y")},
		{Q("suite:a,b:c,d:e"), []string{}, Q("suite:a,b:c,d:e")},
		{Q("suite:a,b:c,d:e"), []string{"x"}, Q("suite:a,b:c,d,x:e")},
		{Q("suite:a,b:c,d:e"), []string{"x", "y"}, Q("suite:a,b:c,d,x,y:e")},
		{Q("suite:a,b:c,d:e;f"), []string{}, Q("suite:a,b:c,d:e;f")},
		{Q("suite:a,b:c,d:e;f"), []string{"x"}, Q("suite:a,b:c,d,x:e;f")},
		{Q("suite:a,b:c,d:e;f"), []string{"x", "y"}, Q("suite:a,b:c,d,x,y:e;f")},
	} {
		got := test.base.AppendTests(test.files...)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("'%v'.AppendTests(%v)\n%v", test.base, test.files, diff)
		}
	}
}

func TestSplitTests(t *testing.T) {
	type Test struct {
		query query.Query
		tests []string
	}

	for _, test := range []Test{
		{Q("suite"), nil},
		{Q("suite:a"), nil},
		{Q("suite:a,b"), nil},
		{Q("suite:a,b:c"), []string{"c"}},
		{Q("suite:a,b:c,d"), []string{"c", "d"}},
		{Q("suite:a,b:c,d:e"), []string{"c", "d"}},
		{Q("suite:a,b:c,d:e;f"), []string{"c", "d"}},
	} {
		got := test.query.SplitTests()
		if diff := cmp.Diff(got, test.tests); diff != "" {
			t.Errorf("'%v'.SplitTests()\n%v", test.query, diff)
		}
	}
}

func TestAppendCases(t *testing.T) {
	type Test struct {
		base   query.Query
		cases  []string
		expect query.Query
	}

	for _, test := range []Test{
		{Q("suite"), []string{}, Q("suite")},
		{Q("suite"), []string{"x"}, Q("suite:::x")},
		{Q("suite"), []string{"x", "y"}, Q("suite:::x;y")},
		{Q("suite:a"), []string{}, Q("suite:a")},
		{Q("suite:a"), []string{"x"}, Q("suite:a::x")},
		{Q("suite:a"), []string{"x", "y"}, Q("suite:a::x;y")},
		{Q("suite:a,b"), []string{}, Q("suite:a,b")},
		{Q("suite:a,b"), []string{"x"}, Q("suite:a,b::x")},
		{Q("suite:a,b"), []string{"x", "y"}, Q("suite:a,b::x;y")},
		{Q("suite:a,b:c"), []string{}, Q("suite:a,b:c")},
		{Q("suite:a,b:c"), []string{"x"}, Q("suite:a,b:c:x")},
		{Q("suite:a,b:c"), []string{"x", "y"}, Q("suite:a,b:c:x;y")},
		{Q("suite:a,b:c,d"), []string{}, Q("suite:a,b:c,d")},
		{Q("suite:a,b:c,d"), []string{"x"}, Q("suite:a,b:c,d:x")},
		{Q("suite:a,b:c,d"), []string{"x", "y"}, Q("suite:a,b:c,d:x;y")},
		{Q("suite:a,b:c,d:e"), []string{}, Q("suite:a,b:c,d:e")},
		{Q("suite:a,b:c,d:e"), []string{"x"}, Q("suite:a,b:c,d:e;x")},
		{Q("suite:a,b:c,d:e"), []string{"x", "y"}, Q("suite:a,b:c,d:e;x;y")},
		{Q("suite:a,b:c,d:e;f"), []string{}, Q("suite:a,b:c,d:e;f")},
		{Q("suite:a,b:c,d:e;f"), []string{"x"}, Q("suite:a,b:c,d:e;f;x")},
		{Q("suite:a,b:c,d:e;f"), []string{"x", "y"}, Q("suite:a,b:c,d:e;f;x;y")},
	} {
		got := test.base.AppendCases(test.cases...)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("'%v'.AppendCases(%v)\n%v", test.base, test.cases, diff)
		}
	}
}

func TestAppend(t *testing.T) {
	type Subtest struct {
		target query.Target
		expect query.Query
	}
	type Test struct {
		base    query.Query
		strings []string
		subtest []Subtest
	}
	for _, test := range []Test{
		{
			Q("suite"), []string{}, []Subtest{
				{query.Files, Q("suite")},
				{query.Tests, Q("suite")},
				{query.Cases, Q("suite")},
			},
		}, {
			Q("suite"), []string{"x"}, []Subtest{
				{query.Files, Q("suite:x")},
				{query.Tests, Q("suite::x")},
				{query.Cases, Q("suite:::x")},
			},
		}, {
			Q("suite"), []string{"x", "y"}, []Subtest{
				{query.Files, Q("suite:x,y")},
				{query.Tests, Q("suite::x,y")},
				{query.Cases, Q("suite:::x;y")},
			},
		}, {
			Q("suite:a"), []string{}, []Subtest{
				{query.Files, Q("suite:a")},
				{query.Tests, Q("suite:a")},
				{query.Cases, Q("suite:a")},
			},
		}, {
			Q("suite:a"), []string{"x"}, []Subtest{
				{query.Files, Q("suite:a,x")},
				{query.Tests, Q("suite:a:x")},
				{query.Cases, Q("suite:a::x")},
			},
		}, {
			Q("suite:a"), []string{"x", "y"}, []Subtest{
				{query.Files, Q("suite:a,x,y")},
				{query.Tests, Q("suite:a:x,y")},
				{query.Cases, Q("suite:a::x;y")},
			},
		}, {
			Q("suite:a,b"), []string{}, []Subtest{
				{query.Files, Q("suite:a,b")},
				{query.Tests, Q("suite:a,b")},
				{query.Cases, Q("suite:a,b")},
			},
		}, {
			Q("suite:a,b"), []string{"x"}, []Subtest{
				{query.Files, Q("suite:a,b,x")},
				{query.Tests, Q("suite:a,b:x")},
				{query.Cases, Q("suite:a,b::x")},
			},
		}, {
			Q("suite:a,b"), []string{"x", "y"}, []Subtest{
				{query.Files, Q("suite:a,b,x,y")},
				{query.Tests, Q("suite:a,b:x,y")},
				{query.Cases, Q("suite:a,b::x;y")},
			},
		}, {
			Q("suite:a,b:c"), []string{}, []Subtest{
				{query.Files, Q("suite:a,b:c")},
				{query.Tests, Q("suite:a,b:c")},
				{query.Cases, Q("suite:a,b:c")},
			},
		}, {
			Q("suite:a,b:c"), []string{"x"}, []Subtest{
				{query.Files, Q("suite:a,b,x:c")},
				{query.Tests, Q("suite:a,b:c,x")},
				{query.Cases, Q("suite:a,b:c:x")},
			},
		}, {
			Q("suite:a,b:c"), []string{"x", "y"}, []Subtest{
				{query.Files, Q("suite:a,b,x,y:c")},
				{query.Tests, Q("suite:a,b:c,x,y")},
				{query.Cases, Q("suite:a,b:c:x;y")},
			},
		}, {
			Q("suite:a,b:c,d"), []string{}, []Subtest{
				{query.Files, Q("suite:a,b:c,d")},
				{query.Tests, Q("suite:a,b:c,d")},
				{query.Cases, Q("suite:a,b:c,d")},
			},
		}, {
			Q("suite:a,b:c,d"), []string{"x"}, []Subtest{
				{query.Files, Q("suite:a,b,x:c,d")},
				{query.Tests, Q("suite:a,b:c,d,x")},
				{query.Cases, Q("suite:a,b:c,d:x")},
			},
		}, {
			Q("suite:a,b:c,d"), []string{"x", "y"}, []Subtest{
				{query.Files, Q("suite:a,b,x,y:c,d")},
				{query.Tests, Q("suite:a,b:c,d,x,y")},
				{query.Cases, Q("suite:a,b:c,d:x;y")},
			},
		}, {
			Q("suite:a,b:c,d:e"), []string{}, []Subtest{
				{query.Files, Q("suite:a,b:c,d:e")},
				{query.Tests, Q("suite:a,b:c,d:e")},
				{query.Cases, Q("suite:a,b:c,d:e")},
			},
		}, {
			Q("suite:a,b:c,d:e"), []string{"x"}, []Subtest{
				{query.Files, Q("suite:a,b,x:c,d:e")},
				{query.Tests, Q("suite:a,b:c,d,x:e")},
				{query.Cases, Q("suite:a,b:c,d:e;x")},
			},
		}, {
			Q("suite:a,b:c,d:e"), []string{"x", "y"}, []Subtest{
				{query.Files, Q("suite:a,b,x,y:c,d:e")},
				{query.Tests, Q("suite:a,b:c,d,x,y:e")},
				{query.Cases, Q("suite:a,b:c,d:e;x;y")},
			},
		}, {
			Q("suite:a,b:c,d:e;f"), []string{}, []Subtest{
				{query.Files, Q("suite:a,b:c,d:e;f")},
				{query.Tests, Q("suite:a,b:c,d:e;f")},
				{query.Cases, Q("suite:a,b:c,d:e;f")},
			},
		}, {
			Q("suite:a,b:c,d:e;f"), []string{"x"}, []Subtest{
				{query.Files, Q("suite:a,b,x:c,d:e;f")},
				{query.Tests, Q("suite:a,b:c,d,x:e;f")},
				{query.Cases, Q("suite:a,b:c,d:e;f;x")},
			},
		}, {
			Q("suite:a,b:c,d:e;f"), []string{"x", "y"}, []Subtest{
				{query.Files, Q("suite:a,b,x,y:c,d:e;f")},
				{query.Tests, Q("suite:a,b:c,d,x,y:e;f")},
				{query.Cases, Q("suite:a,b:c,d:e;f;x;y")},
			},
		},
	} {
		for _, subtest := range test.subtest {
			got := test.base.Append(subtest.target, test.strings...)
			if diff := cmp.Diff(got, subtest.expect); diff != "" {
				t.Errorf("'%v'.Append(%v, %v)\n%v", test.base, subtest.target, test.base.Files, diff)
			}
		}
	}
}

func TestSplitCases(t *testing.T) {
	type Test struct {
		query  query.Query
		expect []string
	}

	for _, test := range []Test{
		{Q("suite"), nil},
		{Q("suite:a"), nil},
		{Q("suite:a,b"), nil},
		{Q("suite:a,b:c"), nil},
		{Q("suite:a,b:c,d"), nil},
		{Q("suite:a,b:c,d:e"), []string{"e"}},
		{Q("suite:a,b:c,d:e;f"), []string{"e", "f"}},
	} {
		got := test.query.SplitCases()
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("'%v'.SplitCases()\n%v", test.query, diff)
		}
	}
}

func TestCaseParameters(t *testing.T) {
	type Test struct {
		query  query.Query
		expect query.CaseParameters
	}

	for _, test := range []Test{
		{Q("suite"), nil},
		{Q("suite:a"), nil},
		{Q("suite:a,b"), nil},
		{Q("suite:a,b:c"), nil},
		{Q("suite:a,b:c,d"), nil},
		{Q("suite:a,b:c,d:e"), query.CaseParameters{"e": ""}},
		{Q("suite:a,b:c,d:e;f"), query.CaseParameters{"e": "", "f": ""}},
		{Q("suite:a,b:c,d:e=f;g=h"), query.CaseParameters{"e": "f", "g": "h"}},
	} {
		got := test.query.CaseParameters()
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("'%v'.CaseParameters()\n%v", test.query, diff)
		}
	}
}

func TestTarget(t *testing.T) {
	type Test struct {
		query  query.Query
		expect query.Target
	}

	for _, test := range []Test{
		{Q("suite"), query.Suite},
		{Q("suite:*"), query.Files},
		{Q("suite:a"), query.Files},
		{Q("suite:a,*"), query.Files},
		{Q("suite:a,b"), query.Files},
		{Q("suite:a,b:*"), query.Tests},
		{Q("suite:a,b:c"), query.Tests},
		{Q("suite:a,b:c,*"), query.Tests},
		{Q("suite:a,b:c,d"), query.Tests},
		{Q("suite:a,b:c,d:*"), query.Cases},
		{Q("suite:a,b:c,d:e"), query.Cases},
		{Q("suite:a,b:c,d:e;*"), query.Cases},
		{Q("suite:a,b:c,d:e;f"), query.Cases},
		{Q("suite:a,b:c,d:e;f;*"), query.Cases},
	} {
		got := test.query.Target()
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("'%v'.Target()\n%v", test.query, diff)
		}
	}
}

func TestIsWildcard(t *testing.T) {
	type Test struct {
		query  query.Query
		expect bool
	}

	for _, test := range []Test{
		{Q("suite"), false},
		{Q("suite:*"), true},
		{Q("suite:a"), false},
		{Q("suite:a,*"), true},
		{Q("suite:a,b"), false},
		{Q("suite:a,b:*"), true},
		{Q("suite:a,b:c"), false},
		{Q("suite:a,b:c,*"), true},
		{Q("suite:a,b:c,d"), false},
		{Q("suite:a,b:c,d:*"), true},
		{Q("suite:a,b:c,d:e"), false},
		{Q("suite:a,b:c,d:e;*"), true},
		{Q("suite:a,b:c,d:e;f"), false},
		{Q("suite:a,b:c,d:e;f;*"), true},
	} {
		got := test.query.IsWildcard()
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("'%v'.IsWildcard()\n%v", test.query, diff)
		}
	}
}

func TestParsePrint(t *testing.T) {
	type Test struct {
		in     string
		expect query.Query
	}

	for _, test := range []Test{
		{
			"a",
			query.Query{
				Suite: "a",
			},
		}, {
			"a:*",
			query.Query{
				Suite: "a",
				Files: "*",
			},
		}, {
			"a:b",
			query.Query{
				Suite: "a",
				Files: "b",
			},
		}, {
			"a:b,*",
			query.Query{
				Suite: "a",
				Files: "b,*",
			},
		}, {
			"a:b:*",
			query.Query{
				Suite: "a",
				Files: "b",
				Tests: "*",
			},
		}, {
			"a:b,c",
			query.Query{
				Suite: "a",
				Files: "b,c",
			},
		}, {
			"a:b,c:*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "*",
			},
		}, {
			"a:b,c:d",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d",
			},
		}, {
			"a:b,c:d,*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,*",
			},
		}, {
			"a:b,c:d,e",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
			},
		}, {
			"a:b,c:d,e,*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e,*",
			},
		}, {
			"a:b,c:d,e:*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: "*",
			},
		}, {
			"a:b,c:d,e:f=g",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: "f=g",
			},
		}, {
			"a:b,c:d,e:f=g;*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: "f=g;*",
			},
		}, {
			"a:b,c:d,e:f=g;h=i",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: "f=g;h=i",
			},
		}, {
			"a:b,c:d,e:f=g;h=i;*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: "f=g;h=i;*",
			},
		}, {
			`a:b,c:d,e:f={"x": 1, "y": 2}`,
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: `f={"x": 1, "y": 2}`,
			},
		}, {
			`a:b,c,d:e,f:g="h";i=[10,20,30];j=40`,
			query.Query{
				Suite: "a",
				Files: "b,c,d",
				Tests: "e,f",
				Cases: `g="h";i=[10,20,30];j=40`,
			},
		},
	} {
		parsed := query.Parse(test.in)
		if diff := cmp.Diff(test.expect, parsed); diff != "" {
			t.Errorf("query.Parse('%v')\n%v", test.in, diff)
		}
		str := test.expect.String()
		if diff := cmp.Diff(test.in, str); diff != "" {
			t.Errorf("query.String('%v')\n%v", test.in, diff)
		}
	}
}

// Essentially the same as TestParsePrint(), but calling ExpectationFileString()
// instead of String().
func TestParsePrintExpectationFileString(t *testing.T) {
	type Test struct {
		input    string
		expected query.Query
	}

	for _, test := range []Test{
		{
			"a",
			query.Query{
				Suite: "a",
			},
		}, {
			"a:*",
			query.Query{
				Suite: "a",
				Files: "*",
			},
		}, {
			"a:b",
			query.Query{
				Suite: "a",
				Files: "b",
			},
		}, {
			"a:b,*",
			query.Query{
				Suite: "a",
				Files: "b,*",
			},
		}, {
			"a:b:*",
			query.Query{
				Suite: "a",
				Files: "b",
				Tests: "*",
			},
		}, {
			"a:b,c",
			query.Query{
				Suite: "a",
				Files: "b,c",
			},
		}, {
			"a:b,c:*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "*",
			},
		}, {
			"a:b,c:d:",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d",
			},
		}, {
			"a:b,c:d,*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,*",
			},
		}, {
			"a:b,c:d,e:",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
			},
		}, {
			"a:b,c:d,e,*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e,*",
			},
		}, {
			"a:b,c:d,e:*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: "*",
			},
		}, {
			"a:b,c:d,e:f=g",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: "f=g",
			},
		}, {
			"a:b,c:d,e:f=g;*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: "f=g;*",
			},
		}, {
			"a:b,c:d,e:f=g;h=i",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: "f=g;h=i",
			},
		}, {
			"a:b,c:d,e:f=g;h=i;*",
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: "f=g;h=i;*",
			},
		}, {
			`a:b,c:d,e:f={"x": 1, "y": 2}`,
			query.Query{
				Suite: "a",
				Files: "b,c",
				Tests: "d,e",
				Cases: `f={"x": 1, "y": 2}`,
			},
		}, {
			`a:b,c,d:e,f:g="h";i=[10,20,30];j=40`,
			query.Query{
				Suite: "a",
				Files: "b,c,d",
				Tests: "e,f",
				Cases: `g="h";i=[10,20,30];j=40`,
			},
		},
	} {
		parsed := query.Parse(test.input)
		assert.Equal(t, test.expected, parsed)
		expectationFileString := test.expected.ExpectationFileString()
		assert.Equal(t, test.input, expectationFileString)
	}
}

func TestCompare(t *testing.T) {
	type Test struct {
		a, b   query.Query
		expect int
	}

	for _, test := range []Test{
		{Q("a"), Q("a"), 0},
		{Q("a:*"), Q("a"), 1},
		{Q("a:*"), Q("a:*"), 0},
		{Q("a:*"), Q("b:*"), -1},
		{Q("a:*"), Q("a:b,*"), -1},
		{Q("a:b,*"), Q("a:b"), 1},
		{Q("a:b,*"), Q("a:b,*"), 0},
		{Q("a:b,*"), Q("a:c,*"), -1},
		{Q("a:b,c,*"), Q("a:b,*"), 1},
		{Q("a:b,c,*"), Q("a:b,c,*"), 0},
		{Q("a:b,c,d,*"), Q("a:b,c,*"), 1},
		{Q("a:b,c,*"), Q("a:b,c:d,*"), 1},
		{Q("a:b,c:*"), Q("a:b,c,d,*"), -1},
		{Q("a:b,c:d,*"), Q("a:b,c:d,*"), 0},
		{Q("a:b,c:d,e,*"), Q("a:b,c:d,*"), 1},
		{Q("a:b,c:d,e,*"), Q("a:b,c:d,e,*"), 0},
		{Q("a:b,c:d,e,*"), Q("a:b,c:e,f,*"), -1},
		{Q("a:b:c:d;*"), Q("a:b:c:d;*"), 0},
		{Q("a:b:c:d;e=1;*"), Q("a:b:c:d;*"), 1},
		{Q("a:b:c:d;e=2;*"), Q("a:b:c:d;e=1;*"), 1},
		{Q("a:b:c:d;e=1;f=2;*"), Q("a:b:c:d;*"), 1},
	} {
		if got, expect := test.a.Compare(test.b), test.expect; got != expect {
			t.Errorf("('%v').Compare('%v')\nexpect: %+v\ngot:    %+v", test.a, test.b, expect, got)
		}
		// Check opposite order
		if got, expect := test.b.Compare(test.a), -test.expect; got != expect {
			t.Errorf("('%v').Compare('%v')\nexpect: %+v\ngot:    %+v", test.b, test.a, expect, got)
		}
	}
}

func TestContains(t *testing.T) {
	type Test struct {
		a, b   query.Query
		expect bool
	}

	for _, test := range []Test{
		{Q("a"), Q("a"), true},
		{Q("a"), Q("b"), false},
		{Q("a:*"), Q("a:*"), true},
		{Q("a:*"), Q("a:b"), true},
		{Q("a:*"), Q("b"), false},
		{Q("a:*"), Q("b:c"), false},
		{Q("a:*"), Q("b:*"), false},
		{Q("a:*"), Q("a:b,*"), true},
		{Q("a:b,*"), Q("a:*"), false},
		{Q("a:b,*"), Q("a:b"), true},
		{Q("a:b,*"), Q("a:c"), false},
		{Q("a:b,*"), Q("a:b,*"), true},
		{Q("a:b,*"), Q("a:c,*"), false},
		{Q("a:b,c"), Q("a:b,c,d"), false},
		{Q("a:b,c"), Q("a:b,c:d"), false},
		{Q("a:b,c,*"), Q("a:b,*"), false},
		{Q("a:b,c,*"), Q("a:b,c"), true},
		{Q("a:b,c,*"), Q("a:b,d"), false},
		{Q("a:b,c,*"), Q("a:b,c,*"), true},
		{Q("a:b,c,*"), Q("a:b,c,d,*"), true},
		{Q("a:b,c,*"), Q("a:b,c:d,*"), true},
		{Q("a:b,c:*"), Q("a:b,c,d,*"), false},
		{Q("a:b,c:d"), Q("a:b,c:d,e"), false},
		{Q("a:b,c:d,*"), Q("a:b,c:d"), true},
		{Q("a:b,c:d,*"), Q("a:b,c:e"), false},
		{Q("a:b,c:d,*"), Q("a:b,c:d,*"), true},
		{Q("a:b,c:d,*"), Q("a:b,c:d,e,*"), true},
		{Q("a:b,c:d,e,*"), Q("a:b,c:d,e"), true},
		{Q("a:b,c:d,e,*"), Q("a:b,c:e,e"), false},
		{Q("a:b,c:d,e,*"), Q("a:b,c:d,f"), false},
		{Q("a:b,c:d,e,*"), Q("a:b,c:d,e,*"), true},
		{Q("a:b,c:d,e,*"), Q("a:b,c:e,f,*"), false},
		{Q("a:b,c:d,e,*"), Q("a:b,c:d,*"), false},
		{Q("a:b:c:d;*"), Q("a:b:c:d;*"), true},
		{Q("a:b:c:d;*"), Q("a:b:c:d,e;*"), true},
		{Q("a:b:c:d;*"), Q("a:b:c:d;e=1;*"), true},
		{Q("a:b:c:d;*"), Q("a:b:c:d;e=1;*"), true},
		{Q("a:b:c:d;*"), Q("a:b:c:d;e=1;f=2;*"), true},
		{Q("a:b:c:d;e=1;*"), Q("a:b:c:d;*"), true},
		{Q("a:b:c:d;e=1;f=2;*"), Q("a:b:c:d;*"), true},
		{Q("a:b:c:d;e=1;*"), Q("a:b:c:d;e=2;*"), false},
		{Q("a:b:c:d;e=2;*"), Q("a:b:c:d;e=1;*"), false},
		{Q("a:b:c:d;e;*"), Q("a:b:c:d;e=1;*"), false},
	} {
		if got := test.a.Contains(test.b); got != test.expect {
			t.Errorf("('%v').Contains('%v')\nexpect: %+v\ngot:    %+v", test.a, test.b, test.expect, got)
		}
	}
}

func TestWalk(t *testing.T) {
	type Segment struct {
		Query  query.Query
		Target query.Target
		Name   string
	}
	type Test struct {
		query  query.Query
		expect []Segment
	}

	for _, test := range []Test{
		{
			Q("suite"), []Segment{
				{Q("suite"), query.Suite, "suite"},
			}},
		{
			Q("suite:*"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:*"), query.Files, "*"},
			}},
		{
			Q("suite:a"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
			}},
		{
			Q("suite:a,*"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
				{Q("suite:a,*"), query.Files, "*"},
			}},
		{
			Q("suite:a,b"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
				{Q("suite:a,b"), query.Files, "b"},
			}},
		{
			Q("suite:a,b:*"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
				{Q("suite:a,b"), query.Files, "b"},
				{Q("suite:a,b:*"), query.Tests, "*"},
			}},
		{
			Q("suite:a,b:c"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
				{Q("suite:a,b"), query.Files, "b"},
				{Q("suite:a,b:c"), query.Tests, "c"},
			}},
		{
			Q("suite:a,b:c,*"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
				{Q("suite:a,b"), query.Files, "b"},
				{Q("suite:a,b:c"), query.Tests, "c"},
				{Q("suite:a,b:c,*"), query.Tests, "*"},
			}},
		{
			Q("suite:a,b:c,d"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
				{Q("suite:a,b"), query.Files, "b"},
				{Q("suite:a,b:c"), query.Tests, "c"},
				{Q("suite:a,b:c,d"), query.Tests, "d"},
			}},
		{
			Q("suite:a,b:c,d:*"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
				{Q("suite:a,b"), query.Files, "b"},
				{Q("suite:a,b:c"), query.Tests, "c"},
				{Q("suite:a,b:c,d"), query.Tests, "d"},
				{Q("suite:a,b:c,d:*"), query.Cases, "*"},
			}},
		{
			Q("suite:a,b:c,d:e"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
				{Q("suite:a,b"), query.Files, "b"},
				{Q("suite:a,b:c"), query.Tests, "c"},
				{Q("suite:a,b:c,d"), query.Tests, "d"},
				{Q("suite:a,b:c,d:e"), query.Cases, "e"},
			}},
		{
			Q("suite:a,b:c,d:e;*"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
				{Q("suite:a,b"), query.Files, "b"},
				{Q("suite:a,b:c"), query.Tests, "c"},
				{Q("suite:a,b:c,d"), query.Tests, "d"},
				{Q("suite:a,b:c,d:e;*"), query.Cases, "e;*"},
			}},
		{
			Q("suite:a,b:c,d:e;f"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
				{Q("suite:a,b"), query.Files, "b"},
				{Q("suite:a,b:c"), query.Tests, "c"},
				{Q("suite:a,b:c,d"), query.Tests, "d"},
				{Q("suite:a,b:c,d:e;f"), query.Cases, "e;f"},
			}},
		{
			Q("suite:a,b:c,d:e;f;*"), []Segment{
				{Q("suite"), query.Suite, "suite"},
				{Q("suite:a"), query.Files, "a"},
				{Q("suite:a,b"), query.Files, "b"},
				{Q("suite:a,b:c"), query.Tests, "c"},
				{Q("suite:a,b:c,d"), query.Tests, "d"},
				{Q("suite:a,b:c,d:e;f;*"), query.Cases, "e;f;*"},
			}},
	} {
		got := []Segment{}
		err := test.query.Walk(func(q query.Query, t query.Target, n string) error {
			got = append(got, Segment{q, t, n})
			return nil
		})
		if err != nil {
			t.Errorf("'%v'.Walk() returned %v", test.query, err)
			continue
		}
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("'%v'.Walk()\n%v", test.query, diff)
		}
	}
}

type TestError struct{}

func (TestError) Error() string { return "test error" }

func TestWalkErrors(t *testing.T) {
	for _, fq := range []query.Query{
		Q("suite"),
		Q("suite:*"),
		Q("suite:a"),
		Q("suite:a,*"),
		Q("suite:a,b"),
		Q("suite:a,b:*"),
		Q("suite:a,b:c"),
		Q("suite:a,b:c,*"),
		Q("suite:a,b:c,d"),
		Q("suite:a,b:c,d:*"),
		Q("suite:a,b:c,d:e"),
		Q("suite:a,b:c,d:e;*"),
		Q("suite:a,b:c,d:e;f"),
		Q("suite:a,b:c,d:e;f;*"),
	} {
		expect := TestError{}
		got := fq.Walk(func(q query.Query, t query.Target, n string) error {
			if q == fq {
				return expect
			}
			return nil
		})
		if diff := cmp.Diff(got, expect); diff != "" {
			t.Errorf("'%v'.Walk()\n%v", fq, diff)
		}
	}
}
