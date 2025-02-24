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

package result_test

import (
	"bytes"
	"path/filepath"
	"testing"
	"time"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/google/go-cmp/cmp"
	"github.com/stretchr/testify/require"
)

var Q = query.Parse

func T(tags ...string) result.Tags {
	return result.NewTags(tags...)
}

func TestStringAndParse(t *testing.T) {
	type Test struct {
		result result.Result
		expect string
	}
	for _, test := range []Test{
		{
			result.Result{
				Query:        Q(`a`),
				Status:       result.Failure,
				Duration:     time.Second * 42,
				MayExonerate: false,
			},
			`a Failure 42s false`,
		}, {
			result.Result{
				Query:        Q(`a:b,c,*`),
				Tags:         T("x"),
				Status:       result.Pass,
				Duration:     time.Second * 42,
				MayExonerate: true,
			},
			`a:b,c,* x Pass 42s true`,
		},
		{
			result.Result{
				Query:        Q(`a:b,c:d,*`),
				Tags:         T("zzz", "x", "yy"),
				Status:       result.Failure,
				Duration:     time.Second * 42,
				MayExonerate: false,
			},
			`a:b,c:d,* x,yy,zzz Failure 42s false`,
		},
	} {
		if diff := cmp.Diff(test.result.String(), test.expect); diff != "" {
			t.Errorf("'%v'.String() was not as expected:\n%v", test.result, diff)
			continue
		}
		_, parsed, err := result.Parse(test.expect)
		if err != nil {
			t.Errorf("Parse('%v') returned %v", test.expect, err)
			continue
		}
		if diff := cmp.Diff(parsed, test.result); diff != "" {
			t.Errorf("Parse('%v') was not as expected:\n%v", test.expect, diff)
		}
	}
}

func TestParseError(t *testing.T) {
	for _, test := range []struct {
		in, expect string
	}{
		{``, `unable to parse result ''`},
		{`a b`, `unable to parse result 'a b'`},
		{`a b c d e`, `unable to parse result 'a b c d e': time: invalid duration "d"`},
		{`a b c 10s e`, `unable to parse result 'a b c 10s e': strconv.ParseBool: parsing "e": invalid syntax`},
	} {
		_, _, err := result.Parse(test.in)
		got := ""
		if err != nil {
			got = err.Error()
		}
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Parse('%v'): %v", test, diff)
			continue
		}
	}
}

func TestTransformTags(t *testing.T) {
	type Test struct {
		results   result.List
		transform func(result.Tags) result.Tags
		expect    result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags(),
				},
			},
			transform: func(t result.Tags) result.Tags { return t },
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags(),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
			},
			transform: func(got result.Tags) result.Tags {
				expect := result.NewTags("x")
				if diff := cmp.Diff(got, expect); diff != "" {
					t.Errorf("transform function's parameter was not as expected:\n%v", diff)
				}
				return got
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Pass,
					Tags:   result.NewTags("y", "z"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("z", "x"),
				},
			},
			transform: func(l result.Tags) result.Tags {
				l = l.Clone()
				if l.Contains("x") {
					l.Remove("x")
					l.Add("X")
				}
				return l
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("X", "y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Pass,
					Tags:   result.NewTags("y", "z"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("z", "X"),
				},
			},
		},
	} {
		got := test.results.TransformTags(test.transform)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nTransformTags() was not as expected:\n%v", test.results, diff)
		}
	}
}

func TestReplaceDuplicates(t *testing.T) {
	type Test struct {
		location string
		results  result.List
		fn       func(result.Statuses) result.Status
		expect   result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass, Duration: 1},
			},
			fn: func(result.Statuses) result.Status {
				return result.Abort
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass, Duration: 1},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass, Duration: 1},
				result.Result{Query: Q(`a`), Status: result.Pass, Duration: 3},
			},
			fn: func(result.Statuses) result.Status {
				return result.Abort
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass, Duration: 2},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
			fn: func(result.Statuses) result.Status {
				return result.Abort
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
				result.Result{Query: Q(`a`), Status: result.Skip},
			},
			fn: func(got result.Statuses) result.Status {
				expect := result.NewStatuses(result.Pass, result.Skip)
				if diff := cmp.Diff(got, expect); diff != "" {
					t.Errorf("function's parameter was not as expected:\n%v", diff)
				}
				return result.Abort
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Abort},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Failure, Duration: 1, MayExonerate: true},
				result.Result{Query: Q(`a`), Status: result.Failure, Duration: 3, MayExonerate: true},
				result.Result{Query: Q(`b`), Status: result.Failure, Duration: 1, MayExonerate: false},
				result.Result{Query: Q(`b`), Status: result.Failure, Duration: 3, MayExonerate: false},
				result.Result{Query: Q(`c`), Status: result.Pass, Duration: 1, MayExonerate: false},
				result.Result{Query: Q(`c`), Status: result.Pass, Duration: 3, MayExonerate: false},
				result.Result{Query: Q(`d`), Status: result.Failure, Duration: 1, MayExonerate: true},
				result.Result{Query: Q(`d`), Status: result.Pass, Duration: 3, MayExonerate: false},
				result.Result{Query: Q(`e`), Status: result.Failure, Duration: 1, MayExonerate: false},
				result.Result{Query: Q(`e`), Status: result.Pass, Duration: 3, MayExonerate: true},
			},
			fn: func(result.Statuses) result.Status {
				return result.Abort
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Failure, Duration: 2, MayExonerate: true},
				result.Result{Query: Q(`b`), Status: result.Failure, Duration: 2, MayExonerate: false},
				result.Result{Query: Q(`c`), Status: result.Pass, Duration: 2, MayExonerate: false},
				result.Result{Query: Q(`d`), Status: result.Pass, Duration: 3, MayExonerate: false},
				result.Result{Query: Q(`e`), Status: result.Failure, Duration: 1, MayExonerate: false},
			},
		},
	} {
		got := test.results.ReplaceDuplicates(test.fn)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("\n%v ReplaceDuplicates() was not as expected:\n%v", test.location, diff)
		}
	}
}

func TestSort(t *testing.T) {
	type Test struct {
		results result.List
		expect  result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`b`), Status: result.Pass},
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`a`), Status: result.Skip},
			},
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`a`), Status: result.Skip},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Skip,
					Tags:   result.NewTags(),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags(),
				},
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags(),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Skip,
					Tags:   result.NewTags(),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("a"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("b"),
				},
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("a"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("b"),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("b"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("a"),
				},
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("a"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("b"),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`c`),
					Status: result.RetryOnFailure,
					Tags:   result.NewTags("z"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Crash,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Slow,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Skip,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Crash,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`aa`),
					Status: result.Crash,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Abort,
					Tags:   result.NewTags("z"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
			},
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Crash,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`a`),
					Status: result.Crash,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`aa`),
					Status: result.Crash,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Skip,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Slow,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Abort,
					Tags:   result.NewTags("z"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.RetryOnFailure,
					Tags:   result.NewTags("z"),
				},
			},
		},
	} {
		got := make(result.List, len(test.results))
		copy(got, test.results)
		got.Sort()
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nSort() was not as expected:\n%v", test.results, diff)
		}
	}
}

func TestFilter(t *testing.T) {
	type Test struct {
		results result.List
		f       func(result.Result) bool
		expect  result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
			f: func(result.Result) bool { return true },
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Failure},
			},
			f: func(r result.Result) bool { return r.Query == Q("b") },
			expect: result.List{
				result.Result{Query: Q(`b`), Status: result.Failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Skip},
				result.Result{Query: Q(`c`), Status: result.Pass},
			},
			f: func(r result.Result) bool { return r.Status == result.Pass },
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`c`), Status: result.Pass},
			},
		},
	} {
		got := test.results.Filter(test.f)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nFilter() was not as expected:\n%v", test.results, diff)
		}
	}
}

func TestFilterByStatus(t *testing.T) {
	type Test struct {
		results result.List
		status  result.Status
		expect  result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Failure},
				result.Result{Query: Q(`c`), Status: result.Pass},
			},
			status: result.Pass,
			expect: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`c`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Failure},
			},
			status: result.Failure,
			expect: result.List{
				result.Result{Query: Q(`b`), Status: result.Failure},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{Query: Q(`a`), Status: result.Pass},
				result.Result{Query: Q(`b`), Status: result.Failure},
			},
			status: result.RetryOnFailure,
			expect: result.List{},
		},
	} {
		got := test.results.FilterByStatus(test.status)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nFilterByStatus(%v) was not as expected:\n%v", test.results, test.status, diff)
		}
	}
}

func TestFilterByTags(t *testing.T) {
	type Test struct {
		results result.List
		tags    result.Tags
		expect  result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Failure,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
			tags: result.NewTags("x", "y"),
			expect: result.List{
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Failure,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
			tags: result.NewTags("x"),
			expect: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
					Tags:   result.NewTags("x"),
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Failure,
					Tags:   result.NewTags("y"),
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Pass,
					Tags:   result.NewTags("x", "y"),
				},
			},
			tags:   result.NewTags("q"),
			expect: result.List{},
		},
	} {
		got := test.results.FilterByTags(test.tags)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nFilterByTags(%v) was not as expected:\n%v", test.results, test.tags, diff)
		}
	}
}

func TestStatuses(t *testing.T) {
	type Test struct {
		results result.List
		expect  container.Set[result.Status]
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{},
			expect:  container.NewSet[result.Status](),
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
				},
			},
			expect: container.NewSet(result.Pass),
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Pass,
				},
			},
			expect: container.NewSet(result.Pass),
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Skip,
				},
			},
			expect: container.NewSet(result.Pass, result.Skip),
		},
		{ //////////////////////////////////////////////////////////////////////
			results: result.List{
				result.Result{
					Query:  Q(`a`),
					Status: result.Pass,
				},
				result.Result{
					Query:  Q(`b`),
					Status: result.Skip,
				},
				result.Result{
					Query:  Q(`c`),
					Status: result.Failure,
				},
			},
			expect: container.NewSet(result.Pass, result.Skip, result.Failure),
		},
	} {
		got := test.results.Statuses()
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("Results:\n%v\nStatuses() was not as expected:\n%v", test.results, diff)
		}
	}
}

func TestSaveLoad(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()

	in := result.ResultsByExecutionMode{
		"bar": result.List{
			{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			{Query: Q(`suite:b,*`), Tags: T(`y`), Status: result.Failure},
			{Query: Q(`suite:a:b:*`), Tags: T(`x`, `y`), Status: result.Skip},
			{Query: Q(`suite:a:c,*`), Tags: T(`y`, `x`), Status: result.Failure},
			{Query: Q(`suite:a,b:c,*`), Tags: T(`y`, `x`), Status: result.Crash},
			{Query: Q(`suite:a,b:c:*`), Status: result.Slow},
		},
		"foo": result.List{
			{Query: Q(`suite:d:*`), Tags: T(`x`), Status: result.Pass},
			{Query: Q(`suite:e,*`), Tags: T(`y`), Status: result.Failure},
		},
	}

	saveLocation := filepath.Join(fileutils.ThisDir(), "cache.txt")
	err := result.SaveWithWrapper(saveLocation, in, wrapper)
	require.NoErrorf(t, err, "Error saving results: %v", err)

	out, err := result.LoadWithWrapper(saveLocation, wrapper)
	require.NoErrorf(t, err, "Error loading results: %v", err)
	require.Equal(t, in, out)
}

func TestReadWrite(t *testing.T) {
	in := result.ResultsByExecutionMode{
		"bar": result.List{
			{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			{Query: Q(`suite:b,*`), Tags: T(`y`), Status: result.Failure},
			{Query: Q(`suite:a:b:*`), Tags: T(`x`, `y`), Status: result.Skip},
			{Query: Q(`suite:a:c,*`), Tags: T(`y`, `x`), Status: result.Failure},
			{Query: Q(`suite:a,b:c,*`), Tags: T(`y`, `x`), Status: result.Crash},
			{Query: Q(`suite:a,b:c:*`), Status: result.Slow},
		},
		"foo": result.List{
			{Query: Q(`suite:d:*`), Tags: T(`x`), Status: result.Pass},
			{Query: Q(`suite:e,*`), Tags: T(`y`), Status: result.Failure},
		},
	}
	buf := &bytes.Buffer{}
	if err := result.Write(buf, in); err != nil {
		t.Fatalf("Write(): %v", err)
	}
	got, err := result.Read(buf)
	if err != nil {
		t.Fatalf("Read(): %v", err)
	}
	if diff := cmp.Diff(got, in); diff != "" {
		t.Errorf("Read() was not as expected:\n%v", diff)
	}
}

func TestMerge(t *testing.T) {
	type Test struct {
		location string
		a, b     result.List
		expect   result.List
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			a:        result.List{},
			b:        result.List{},
			expect:   result.List{},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			a: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			},
			b: result.List{},
			expect: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			a:        result.List{},
			b: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			},
			expect: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			a: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			},
			b: result.List{
				{Query: Q(`suite:b:*`), Tags: T(`x`), Status: result.Pass},
			},
			expect: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
				{Query: Q(`suite:b:*`), Tags: T(`x`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			a: result.List{
				{Query: Q(`suite:b:*`), Tags: T(`x`), Status: result.Pass},
			},
			b: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			},
			expect: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
				{Query: Q(`suite:b:*`), Tags: T(`x`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			a: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			},
			b: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`y`), Status: result.Pass},
			},
			expect: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
				{Query: Q(`suite:a:*`), Tags: T(`y`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			a: result.List{
				{Query: Q(`suite:a:*`), Status: result.Pass},
			},
			b: result.List{
				{Query: Q(`suite:a:*`), Status: result.Pass},
			},
			expect: result.List{
				{Query: Q(`suite:a:*`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			a: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			},
			b: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			},
			expect: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			a: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Crash},
			},
			b: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Crash},
			},
			expect: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Crash},
			},
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			a: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Pass},
				{Query: Q(`suite:b:*`), Tags: T(`x`), Status: result.Pass},
				{Query: Q(`suite:c:*`), Tags: T(`x`), Status: result.Failure},
				{Query: Q(`suite:d:*`), Tags: T(`x`), Status: result.Failure},
				{Query: Q(`suite:e:*`), Tags: T(`x`), Status: result.Crash},
			},
			b: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.Failure},
				{Query: Q(`suite:b:*`), Tags: T(`x`), Status: result.Pass},
				{Query: Q(`suite:c:*`), Tags: T(`x`), Status: result.Pass},
				{Query: Q(`suite:d:*`), Tags: T(`y`), Status: result.Pass},
				{Query: Q(`suite:e:*`), Tags: T(`x`), Status: result.Pass},
			},
			expect: result.List{
				{Query: Q(`suite:a:*`), Tags: T(`x`), Status: result.RetryOnFailure},
				{Query: Q(`suite:b:*`), Tags: T(`x`), Status: result.Pass},
				{Query: Q(`suite:c:*`), Tags: T(`x`), Status: result.RetryOnFailure},
				{Query: Q(`suite:d:*`), Tags: T(`x`), Status: result.Failure},
				{Query: Q(`suite:d:*`), Tags: T(`y`), Status: result.Pass},
				{Query: Q(`suite:e:*`), Tags: T(`x`), Status: result.Crash},
			},
		},
	} {
		got := result.Merge(test.a, test.b)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("%v\nStatusTree() was not as expected:\n%v", test.location, diff)
		}
	}
}

func TestDeduplicate(t *testing.T) {
	type Test struct {
		location string
		statuses result.Statuses
		expect   result.Status
	}
	for _, test := range []Test{
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Pass),
			expect:   result.Pass,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Abort),
			expect:   result.Abort,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Failure),
			expect:   result.Failure,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Skip),
			expect:   result.Skip,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Crash),
			expect:   result.Crash,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Slow),
			expect:   result.Slow,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Unknown),
			expect:   result.Unknown,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.RetryOnFailure),
			expect:   result.RetryOnFailure,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Pass, result.Failure),
			expect:   result.RetryOnFailure,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Pass, result.Abort),
			expect:   result.Abort,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Pass, result.Skip),
			expect:   result.RetryOnFailure,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Pass, result.Crash),
			expect:   result.Crash,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Pass, result.Slow),
			expect:   result.RetryOnFailure,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Pass, result.Unknown),
			expect:   result.Unknown,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Pass, result.RetryOnFailure),
			expect:   result.RetryOnFailure,
		},
		{ //////////////////////////////////////////////////////////////////////
			location: fileutils.ThisLine(),
			statuses: result.NewStatuses(result.Status("??"), result.Status("?!")),
			expect:   result.Unknown,
		},
	} {
		got := result.Deduplicate(test.statuses)
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf("\n%v Deduplicate() was not as expected:\n%v", test.location, diff)
		}
	}
}
