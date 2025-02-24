// Copyright 2024 The Dawn & Tint Authors
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
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"

	"github.com/google/go-cmp/cmp"
	"github.com/stretchr/testify/require"
)

/*******************************************************************************
 * Content tests
 ******************************************************************************/

// Tests behavior of Content.Format()
func TestContentFormat(t *testing.T) {
	// Intentionally includes extra whitespace since that should be removed.
	content := `# OS
# tags: [ linux mac win ]
# GPU
# tags: [ amd intel nvidia ]

################################################################################
# Bug order
################################################################################
crbug.com/3 [ linux ] a1 [ Failure ]
crbug.com/2 [ mac ] b1 [ Failure ]
crbug.com/1 [ win ] c1 [ Failure ]





################################################################################
# Query order
################################################################################
crbug.com/4 [ linux ] c2 [ Failure ]
crbug.com/4 [ linux ] b2 [ Failure ]
crbug.com/4 [ linux ] a2 [ Failure ]

################################################################################
# Tag order
################################################################################
crbug.com/5 [ intel ] a3 [ Failure ]
crbug.com/5 [ amd ] a3 [ Failure ]
crbug.com/5 [ nvidia ] a3 [ Failure ]

################################################################################
# Order priority
# Bug > Tags > Query
################################################################################
crbug.com/1 [ win ] z [ Failure ]
crbug.com/3 [ intel ] c [ Failure ]
crbug.com/2 [ intel ] a [ Failure ]
crbug.com/3 [ intel ] d [ Failure ]
crbug.com/2 [ amd ] b [ Failure ]
`

	expected_content := `# OS
# tags: [ linux mac win ]
# GPU
# tags: [ amd intel nvidia ]

################################################################################
# Bug order
################################################################################
crbug.com/1 [ win ] c1 [ Failure ]
crbug.com/2 [ mac ] b1 [ Failure ]
crbug.com/3 [ linux ] a1 [ Failure ]

################################################################################
# Query order
################################################################################
crbug.com/4 [ linux ] a2 [ Failure ]
crbug.com/4 [ linux ] b2 [ Failure ]
crbug.com/4 [ linux ] c2 [ Failure ]

################################################################################
# Tag order
################################################################################
crbug.com/5 [ amd ] a3 [ Failure ]
crbug.com/5 [ intel ] a3 [ Failure ]
crbug.com/5 [ nvidia ] a3 [ Failure ]

################################################################################
# Order priority
# Bug > Tags > Query
################################################################################
crbug.com/1 [ win ] z [ Failure ]
crbug.com/2 [ amd ] b [ Failure ]
crbug.com/2 [ intel ] a [ Failure ]
crbug.com/3 [ intel ] c [ Failure ]
crbug.com/3 [ intel ] d [ Failure ]
`
	expectations, err := Parse("", content)
	if err != nil {
		t.Errorf("Parsing content failed: %s", err.Error())
	}
	expectations.Format()

	if diff := cmp.Diff(expectations.String(), expected_content); diff != "" {
		t.Errorf("Format produced unexpected output: %v", diff)
	}
}

// Tests that expectations for unknown tests are properly removed, even if they
// are in an immutable chunk.
func TestRemoveExpectationsForUnknownTests(t *testing.T) {
	startingContent := `
# BEGIN TAG HEADER
# OS
# tags: [ android linux mac win10 ]
# END TAG HEADER

# Partially removed, immutable.
[ android ] * [ Failure ]
[ mac ] a:* [ Failure ]
[ linux ] a:b,c:d,e:f=* [ Failure ]
[ linux ] valid_test1 [ Failure ]
[ linux ] invalid_test [ Failure ]
[ linux ] valid_test2 [ Failure ]

# Fully removed, immutable.
[ android ] invalid_test [ Failure ]
[ android ] c:* [ Failure ]

# Partially removed, mutable.
# ##ROLLER_AUTOGENERATED_FAILURES##
[ win10 ] valid_test1 [ Failure ]
[ win10 ] invalid_test [ Failure ]
[ win10 ] valid_test2 [ Failure ]
`

	content, err := Parse("expectations.txt", startingContent)
	require.NoErrorf(t, err, "Failed to parse expectations: %v", err)

	knownTests := []query.Query{
		query.Parse("valid_test1"),
		query.Parse("valid_test2"),
		query.Parse("a:b"),
		query.Parse("a:b,c:d,e:f=g"),
	}

	content.RemoveExpectationsForUnknownTests(&knownTests)

	expectedContent := `# BEGIN TAG HEADER
# OS
# tags: [ android linux mac win10 ]
# END TAG HEADER

# Partially removed, immutable.
[ android ] * [ Failure ]
[ mac ] a:* [ Failure ]
[ linux ] a:b,c:d,e:f=* [ Failure ]
[ linux ] valid_test1 [ Failure ]
[ linux ] valid_test2 [ Failure ]

# Partially removed, mutable.
# ##ROLLER_AUTOGENERATED_FAILURES##
[ win10 ] valid_test1 [ Failure ]
[ win10 ] valid_test2 [ Failure ]
`

	require.Equal(t, expectedContent, content.String())
}

/*******************************************************************************
 * Expectation tests
 ******************************************************************************/

func TestExpectationAsExpectationFileString(t *testing.T) {
	// Full expectation.
	e := Expectation{
		Bug:     "crbug.com/1234",
		Tags:    result.NewTags("linux", "nvidia"),
		Query:   "query",
		Status:  []string{"Failure", "Slow"},
		Comment: "# comment",
	}
	require.Equal(t, e.AsExpectationFileString(), "crbug.com/1234 [ linux nvidia ] query [ Failure Slow ] # comment")

	// No bug.
	e = Expectation{
		Tags:    result.NewTags("linux", "nvidia"),
		Query:   "query",
		Status:  []string{"Failure", "Slow"},
		Comment: "# comment",
	}
	require.Equal(t, e.AsExpectationFileString(), "[ linux nvidia ] query [ Failure Slow ] # comment")

	// No tags.
	e = Expectation{
		Bug:     "crbug.com/1234",
		Tags:    result.NewTags(),
		Query:   "query",
		Status:  []string{"Failure", "Slow"},
		Comment: "# comment",
	}
	require.Equal(t, e.AsExpectationFileString(), "crbug.com/1234 query [ Failure Slow ] # comment")

	// No comment.
	e = Expectation{
		Bug:    "crbug.com/1234",
		Tags:   result.NewTags("linux", "nvidia"),
		Query:  "query",
		Status: []string{"Failure", "Slow"},
	}
	require.Equal(t, e.AsExpectationFileString(), "crbug.com/1234 [ linux nvidia ] query [ Failure Slow ]")

	// Minimal expectation.
	e = Expectation{
		Query:  "query",
		Status: []string{"Failure", "Slow"},
	}
	require.Equal(t, e.AsExpectationFileString(), "query [ Failure Slow ]")
}

func TestSort(t *testing.T) {
	firstAndroidOne := Expectation{
		Bug:    "crbug.com/1",
		Tags:   result.NewTags("android"),
		Query:  "first_query",
		Status: []string{"Failure"},
	}

	firstAndroidTwo := Expectation{
		Bug:    "crbug.com/2",
		Tags:   result.NewTags("android"),
		Query:  "first_query",
		Status: []string{"Failure"},
	}

	firstLinuxOne := Expectation{
		Bug:    "crbug.com/1",
		Tags:   result.NewTags("linux"),
		Query:  "first_query",
		Status: []string{"Failure"},
	}

	firstLinuxTwo := Expectation{
		Bug:    "crbug.com/2",
		Tags:   result.NewTags("linux"),
		Query:  "first_query",
		Status: []string{"Failure"},
	}

	secondAndroidOne := Expectation{
		Bug:    "crbug.com/1",
		Tags:   result.NewTags("android"),
		Query:  "second_query",
		Status: []string{"Failure"},
	}

	secondAndroidTwo := Expectation{
		Bug:    "crbug.com/2",
		Tags:   result.NewTags("android"),
		Query:  "second_query",
		Status: []string{"Failure"},
	}

	secondLinuxOne := Expectation{
		Bug:    "crbug.com/1",
		Tags:   result.NewTags("linux"),
		Query:  "second_query",
		Status: []string{"Failure"},
	}

	secondLinuxTwo := Expectation{
		Bug:    "crbug.com/2",
		Tags:   result.NewTags("linux"),
		Query:  "second_query",
		Status: []string{"Failure"},
	}

	expectationsList := Expectations{
		firstAndroidOne,
		firstAndroidTwo,
		firstLinuxOne,
		firstLinuxTwo,
		secondAndroidOne,
		secondAndroidTwo,
		secondLinuxOne,
		secondLinuxTwo,
	}

	expectationsList.Sort()

	expectedList := Expectations{
		firstAndroidOne,
		secondAndroidOne,
		firstLinuxOne,
		secondLinuxOne,
		firstAndroidTwo,
		secondAndroidTwo,
		firstLinuxTwo,
		secondLinuxTwo,
	}

	require.Equal(t, expectationsList, expectedList)
}

func TestSortPrioritizeQuery(t *testing.T) {
	firstAndroidOne := Expectation{
		Bug:    "crbug.com/1",
		Tags:   result.NewTags("android"),
		Query:  "first_query",
		Status: []string{"Failure"},
	}

	firstAndroidTwo := Expectation{
		Bug:    "crbug.com/2",
		Tags:   result.NewTags("android"),
		Query:  "first_query",
		Status: []string{"Failure"},
	}

	firstLinuxOne := Expectation{
		Bug:    "crbug.com/1",
		Tags:   result.NewTags("linux"),
		Query:  "first_query",
		Status: []string{"Failure"},
	}

	firstLinuxTwo := Expectation{
		Bug:    "crbug.com/2",
		Tags:   result.NewTags("linux"),
		Query:  "first_query",
		Status: []string{"Failure"},
	}

	secondAndroidOne := Expectation{
		Bug:    "crbug.com/1",
		Tags:   result.NewTags("android"),
		Query:  "second_query",
		Status: []string{"Failure"},
	}

	secondAndroidTwo := Expectation{
		Bug:    "crbug.com/2",
		Tags:   result.NewTags("android"),
		Query:  "second_query",
		Status: []string{"Failure"},
	}

	secondLinuxOne := Expectation{
		Bug:    "crbug.com/1",
		Tags:   result.NewTags("linux"),
		Query:  "second_query",
		Status: []string{"Failure"},
	}

	secondLinuxTwo := Expectation{
		Bug:    "crbug.com/2",
		Tags:   result.NewTags("linux"),
		Query:  "second_query",
		Status: []string{"Failure"},
	}

	expectationsList := Expectations{
		firstAndroidOne,
		secondAndroidOne,
		firstLinuxOne,
		secondLinuxOne,
		firstAndroidTwo,
		secondAndroidTwo,
		firstLinuxTwo,
		secondLinuxTwo,
	}

	expectationsList.SortPrioritizeQuery()

	expectedList := Expectations{
		firstAndroidOne,
		firstAndroidTwo,
		firstLinuxOne,
		firstLinuxTwo,
		secondAndroidOne,
		secondAndroidTwo,
		secondLinuxOne,
		secondLinuxTwo,
	}

	require.Equal(t, expectationsList, expectedList)
}

func TestAppliesToResult(t *testing.T) {
	tests := []struct {
		name        string
		e           Expectation
		r           result.Result
		shouldMatch bool
	}{
		{ /////////////////////////////////////////////////////////////////////////
			name: "Exact match",
			e: Expectation{
				Query: "foo",
				Tags:  result.NewTags("android"),
			},
			r: result.Result{
				Query: query.Parse("foo"),
				Tags:  result.NewTags("android", "release"),
			},
			shouldMatch: true,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Wildcard match",
			e: Expectation{
				Query: "foo*",
				Tags:  result.NewTags("android"),
			},
			r: result.Result{
				Query: query.Parse("foobar"),
				Tags:  result.NewTags("android", "release"),
			},
			shouldMatch: true,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Wildcard match everything",
			e: Expectation{
				Query: "*",
				Tags:  result.NewTags("android"),
			},
			r: result.Result{
				Query: query.Parse("foobar"),
				Tags:  result.NewTags("android", "release"),
			},
			shouldMatch: true,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Name mismatch",
			e: Expectation{
				Query: "foo",
				Tags:  result.NewTags("android"),
			},
			r: result.Result{
				Query: query.Parse("bar"),
				Tags:  result.NewTags("android", "release"),
			},
			shouldMatch: false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Wildcard mismatch",
			e: Expectation{
				Query: "foo*",
				Tags:  result.NewTags("android"),
			},
			r: result.Result{
				Query: query.Parse("bar"),
				Tags:  result.NewTags("android", "release"),
			},
			shouldMatch: false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Escaped wildcard mismatch",
			e: Expectation{
				Query: "foo\\*",
				Tags:  result.NewTags("android"),
			},
			r: result.Result{
				Query: query.Parse("foobar"),
				Tags:  result.NewTags("android", "release"),
			},
			shouldMatch: false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Tag mismatch",
			e: Expectation{
				Query: "foo",
				Tags:  result.NewTags("android"),
			},
			r: result.Result{
				Query: query.Parse("foo"),
				Tags:  result.NewTags("linux", "release"),
			},
			shouldMatch: false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Tag not subset",
			e: Expectation{
				Query: "foo",
				Tags:  result.NewTags("android", "release"),
			},
			r: result.Result{
				Query: query.Parse("foo"),
				Tags:  result.NewTags("linux", "release"),
			},
			shouldMatch: false,
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			require.Equal(t, testCase.shouldMatch, testCase.e.AppliesToResult(testCase.r))
		})
	}
}

/*******************************************************************************
 * Chunk tests
 ******************************************************************************/

func TestChunk_ContainedWithinList(t *testing.T) {
	tests := []struct {
		name      string
		chunk     Chunk
		chunkList []Chunk
		want      bool
	}{
		{ /////////////////////////////////////////////////////////////////////////
			name:      "Empty list",
			chunk:     Chunk{},
			chunkList: []Chunk{},
			want:      false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name:  "Empty chunk",
			chunk: Chunk{},
			chunkList: []Chunk{
				Chunk{},
			},
			want: true,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Complex chunk success",
			chunk: Chunk{
				Comments: []string{"c1", "c2"},
				Expectations: []Expectation{
					{
						Line:    2,
						Bug:     "bug",
						Tags:    result.NewTags("t1", "t2"),
						Query:   "q",
						Status:  []string{"status"},
						Comment: "comment",
					},
				},
			},
			chunkList: []Chunk{
				Chunk{
					Comments: []string{"c1", "c2"},
					Expectations: []Expectation{
						{
							Line:    2,
							Bug:     "bug",
							Tags:    result.NewTags("t1", "t2"),
							Query:   "q",
							Status:  []string{"status"},
							Comment: "comment",
						},
					},
				},
			},
			want: true,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Complex chunk success multiple in list",
			chunk: Chunk{
				Comments: []string{"c1", "c2"},
				Expectations: []Expectation{
					{
						Line:    2,
						Bug:     "bug",
						Tags:    result.NewTags("t1", "t2"),
						Query:   "q",
						Status:  []string{"status"},
						Comment: "comment",
					},
				},
			},
			chunkList: []Chunk{
				Chunk{
					Comments: []string{"c3"},
					Expectations: []Expectation{
						{
							Line:    2,
							Bug:     "bug",
							Tags:    result.NewTags("t1", "t2"),
							Query:   "q",
							Status:  []string{"status"},
							Comment: "comment",
						},
					},
				},
				Chunk{
					Comments: []string{"c1", "c2"},
					Expectations: []Expectation{
						{
							Line:    2,
							Bug:     "bug",
							Tags:    result.NewTags("t1", "t2"),
							Query:   "q",
							Status:  []string{"status"},
							Comment: "comment",
						},
					},
				},
			},
			want: true,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Complex chunk comments differ",
			chunk: Chunk{
				Comments: []string{"c1", "c2"},
				Expectations: []Expectation{
					{
						Line:    2,
						Bug:     "bug",
						Tags:    result.NewTags("t1", "t2"),
						Query:   "q",
						Status:  []string{"status"},
						Comment: "comment",
					},
				},
			},
			chunkList: []Chunk{
				Chunk{
					Comments: []string{"c2"},
					Expectations: []Expectation{
						{
							Line:    2,
							Bug:     "bug",
							Tags:    result.NewTags("t1", "t2"),
							Query:   "q",
							Status:  []string{"status"},
							Comment: "comment",
						},
					},
				},
			},
			want: false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Complex chunk success expectations differ",
			chunk: Chunk{
				Comments: []string{"c1", "c2"},
				Expectations: []Expectation{
					{
						Line:    2,
						Bug:     "bug",
						Tags:    result.NewTags("t1", "t2"),
						Query:   "q",
						Status:  []string{"status"},
						Comment: "comment",
					},
				},
			},
			chunkList: []Chunk{
				Chunk{
					Comments: []string{"c1", "c2"},
					Expectations: []Expectation{
						{
							Line:    2,
							Bug:     "bug",
							Tags:    result.NewTags("t1", "t2"),
							Query:   "q",
							Status:  []string{"status"},
							Comment: "comment",
						},
						{
							Line:    3,
							Bug:     "bug",
							Tags:    result.NewTags("t1", "t2"),
							Query:   "q",
							Status:  []string{"status"},
							Comment: "comment",
						},
					},
				},
			},
			want: false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Complex chunk expectation line differs",
			chunk: Chunk{
				Comments: []string{"c1", "c2"},
				Expectations: []Expectation{
					{
						Line:    2,
						Bug:     "bug",
						Tags:    result.NewTags("t1", "t2"),
						Query:   "q",
						Status:  []string{"status"},
						Comment: "comment",
					},
				},
			},
			chunkList: []Chunk{
				Chunk{
					Comments: []string{"c1", "c2"},
					Expectations: []Expectation{
						{
							Line:    3,
							Bug:     "bug",
							Tags:    result.NewTags("t1", "t2"),
							Query:   "q",
							Status:  []string{"status"},
							Comment: "comment",
						},
					},
				},
			},
			want: false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Complex chunk expectation bug differs",
			chunk: Chunk{
				Comments: []string{"c1", "c2"},
				Expectations: []Expectation{
					{
						Line:    2,
						Bug:     "bug",
						Tags:    result.NewTags("t1", "t2"),
						Query:   "q",
						Status:  []string{"status"},
						Comment: "comment",
					},
				},
			},
			chunkList: []Chunk{
				Chunk{
					Comments: []string{"c1", "c2"},
					Expectations: []Expectation{
						{
							Line:    2,
							Bug:     "bug2",
							Tags:    result.NewTags("t1", "t2"),
							Query:   "q",
							Status:  []string{"status"},
							Comment: "comment",
						},
					},
				},
			},
			want: false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Complex chunk expectation tags differ",
			chunk: Chunk{
				Comments: []string{"c1", "c2"},
				Expectations: []Expectation{
					{
						Line:    2,
						Bug:     "bug",
						Tags:    result.NewTags("t1", "t2"),
						Query:   "q",
						Status:  []string{"status"},
						Comment: "comment",
					},
				},
			},
			chunkList: []Chunk{
				Chunk{
					Comments: []string{"c1", "c2"},
					Expectations: []Expectation{
						{
							Line:    2,
							Bug:     "bug",
							Tags:    result.NewTags("t1", "t2", "t3"),
							Query:   "q",
							Status:  []string{"status"},
							Comment: "comment",
						},
					},
				},
			},
			want: false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Complex chunk expectation query differs",
			chunk: Chunk{
				Comments: []string{"c1", "c2"},
				Expectations: []Expectation{
					{
						Line:    2,
						Bug:     "bug",
						Tags:    result.NewTags("t1", "t2"),
						Query:   "q",
						Status:  []string{"status"},
						Comment: "comment",
					},
				},
			},
			chunkList: []Chunk{
				Chunk{
					Comments: []string{"c1", "c2"},
					Expectations: []Expectation{
						{
							Line:    2,
							Bug:     "bug",
							Tags:    result.NewTags("t1", "t2"),
							Query:   "q2",
							Status:  []string{"status"},
							Comment: "comment",
						},
					},
				},
			},
			want: false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Complex chunk expectation status differs",
			chunk: Chunk{
				Comments: []string{"c1", "c2"},
				Expectations: []Expectation{
					{
						Line:    2,
						Bug:     "bug",
						Tags:    result.NewTags("t1", "t2"),
						Query:   "q",
						Status:  []string{"status"},
						Comment: "comment",
					},
				},
			},
			chunkList: []Chunk{
				Chunk{
					Comments: []string{"c1", "c2"},
					Expectations: []Expectation{
						{
							Line:    2,
							Bug:     "bug",
							Tags:    result.NewTags("t1", "t2"),
							Query:   "q",
							Status:  []string{"status2"},
							Comment: "comment",
						},
					},
				},
			},
			want: false,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Complex chunk expectation comment differs",
			chunk: Chunk{
				Comments: []string{"c1", "c2"},
				Expectations: []Expectation{
					{
						Line:    2,
						Bug:     "bug",
						Tags:    result.NewTags("t1", "t2"),
						Query:   "q",
						Status:  []string{"status"},
						Comment: "comment",
					},
				},
			},
			chunkList: []Chunk{
				Chunk{
					Comments: []string{"c1", "c2"},
					Expectations: []Expectation{
						{
							Line:    2,
							Bug:     "bug",
							Tags:    result.NewTags("t1", "t2"),
							Query:   "q",
							Status:  []string{"status"},
							Comment: "comment2",
						},
					},
				},
			},
			want: false,
		},
	}
	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			got := testCase.chunk.ContainedWithinList(&testCase.chunkList)
			require.Equal(t, testCase.want, got)
		})
	}
}
