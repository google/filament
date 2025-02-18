// Copyright 2025 The Dawn & Tint Authors
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

package expectationcoverage

import (
	"bytes"
	"context"
	"fmt"
	"reflect"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/resultsdb"

	"github.com/stretchr/testify/require"
)

/*******************************************************************************
 * Run tests
 ******************************************************************************/

func createConfig(wrapper oswrapper.OSWrapper, client resultsdb.Querier) common.Config {
	return common.Config{
		Tests: []common.TestConfig{
			{
				ExecutionMode: "core",
				Prefixes:      []string{"core_prefix"},
			},
			{
				ExecutionMode: "compat",
				Prefixes:      []string{"compat_prefix"},
			},
		},
		OsWrapper: wrapper,
		Querier:   client,
	}
}

func createRunSampleQueryResults() resultsdb.PrefixGroupedQueryResults {
	return resultsdb.PrefixGroupedQueryResults{
		"core_prefix": []resultsdb.QueryResult{
			{
				TestId: "core_prefix_foo",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					{
						Key:   "typ_tag",
						Value: "linux",
					},
				},
			},
			{
				TestId: "core_prefix_bar",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					{
						Key:   "typ_tag",
						Value: "mac",
					},
				},
			},
		},
		"compat_prefix": []resultsdb.QueryResult{
			{
				TestId: "compat_prefix_bar",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					{
						Key:   "typ_tag",
						Value: "linux",
					},
				},
			},
			{
				TestId: "compat_prefix_foo",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					{
						Key:   "typ_tag",
						Value: "win10",
					},
				},
			},
		},
	}
}

func TestRun_GetTrimmedContentFailure(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	client := resultsdb.MockBigQueryClient{}

	expectationFileContent := `# BEGIN TAG HEADER
# OS
# tags: [ android linux mac win10 ]
# END TAG HEADER

crbug.com/0000 [ android ] foo
`
	wrapper.WriteFile(common.DefaultExpectationsPath(), []byte(expectationFileContent), 0o700)

	ctx := context.Background()
	cfg := createConfig(wrapper, client)
	c := cmd{}
	err := c.Run(ctx, cfg)
	require.ErrorContains(t, err, "/expectations.txt:6:31 error: expected status")
}

func TestRun_GetResultsFailure(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	client := resultsdb.MockBigQueryClient{}

	expectationFileContent := getExpectationContentForTrimmedContentTest()
	wrapper.WriteFile(common.DefaultExpectationsPath(), []byte(expectationFileContent), 0o700)

	client.RecentUniqueSuppressedReturnValues = resultsdb.PrefixGroupedQueryResults{
		"core_prefix": []resultsdb.QueryResult{
			{
				TestId: "invalid_prefix_test",
				Status: "FAIL",
			},
		},
	}

	ctx := context.Background()
	cfg := createConfig(wrapper, client)
	c := cmd{}
	err := c.Run(ctx, cfg)
	require.ErrorContains(t, err,
		"Test ID invalid_prefix_test did not start with core_prefix even though query should have filtered.")
}

func TestRun_SuccessCore(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	client := resultsdb.MockBigQueryClient{}

	expectationFileContent := getExpectationContentForTrimmedContentTest()
	wrapper.WriteFile(common.DefaultExpectationsPath(), []byte(expectationFileContent), 0o700)

	client.RecentUniqueSuppressedReturnValues = createRunSampleQueryResults()

	ctx := context.Background()
	cfg := createConfig(wrapper, client)
	c := cmd{}
	err := c.Run(ctx, cfg)
	require.NoErrorf(t, err, "Got error: %v", err)
}

func TestRun_SuccessCompat(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	client := resultsdb.MockBigQueryClient{}

	expectationFileContent := getExpectationContentForTrimmedContentTest()
	wrapper.WriteFile(common.DefaultCompatExpectationsPath(), []byte(expectationFileContent), 0o700)

	client.RecentUniqueSuppressedReturnValues = createRunSampleQueryResults()

	ctx := context.Background()
	cfg := createConfig(wrapper, client)
	c := cmd{}
	c.flags.checkCompatExpectations = true
	err := c.Run(ctx, cfg)
	require.NoErrorf(t, err, "Got error: %v", err)
}

/*******************************************************************************
 * getTrimmedContent tests
 ******************************************************************************/

func TestGetTrimmedContent_NonExistentFile(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	content, err := getTrimmedContent(
		"/expectations.txt",
		/*individualExpectations=*/ false,
		/*ignoreSkipExpectations=*/ false,
		/*verbose=*/ false,
		wrapper)

	require.Equal(t, content, expectations.Content{})
	require.ErrorContains(t, err, "open /expectations.txt: file does not exist")
}

func TestGetTrimmedContent_InvalidFile(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	expectationFileContent := `# BEGIN TAG HEADER
# OS
# tags: [ android linux mac win10 ]
# END TAG HEADER

crbug.com/0000 [ android ] foo
`
	wrapper.WriteFile("/expectations.txt", []byte(expectationFileContent), 0o700)

	content, err := getTrimmedContent(
		"/expectations.txt",
		/*individualExpectations=*/ false,
		/*ignoreSkipExpectations=*/ false,
		/*verbose=*/ false,
		wrapper)

	require.Equal(t, content, expectations.Content{})
	require.ErrorContains(t, err, "/expectations.txt:6:31 error: expected status")
}

// Error from getPermanentSkipContent not tested since there is no way to
// trigger an error there without triggering an error when parsing in
// getTrimmedContent.

func getExpectationContentForTrimmedContentTest() string {
	return `# BEGIN TAG HEADER
# OS
# tags: [ android linux mac win10 ]
# END TAG HEADER

################################################################################
# Permanent Skip Expectations
################################################################################

# Permanent skips
crbug.com/0000 [ android ] foo [ Skip ]
crbug.com/0000 [ android ] bar [ Skip ]

################################################################################
# Temporary Skip Expectations
################################################################################

# Temporary skips
crbug.com/0000 [ linux ] foo [ Skip ]
crbug.com/0000 [ linux ] bar [ Skip ]

################################################################################
# Triaged/Manually Added Failures
################################################################################

# Failures 1
crbug.com/0000 [ mac ] foo [ Failure ]
crbug.com/0000 [ mac ] bar [ RetryOnFailure ]

# Failures 2
# Second line
crbug.com/0000 [ win10 ] foo [ Failure ]
crbug.com/0000 [ win10 ] bar [ Failure ]
`
}

func TestGetTrimmedContent_GroupedWithSkips(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	expectationFileContent := getExpectationContentForTrimmedContentTest()
	wrapper.WriteFile("/expectations.txt", []byte(expectationFileContent), 0o700)

	content, err := getTrimmedContent(
		"/expectations.txt",
		/*individualExpectations=*/ false,
		/*ignoreSkipExpectations=*/ false,
		/*verbose=*/ false,
		wrapper)

	expectedChunks := []expectations.Chunk{
		{
			Comments: []string{"# Temporary skips"},
			Expectations: expectations.Expectations{
				{
					Line:   19,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("linux"),
					Query:  "foo",
					Status: []string{"Skip"},
				},
				{
					Line:   20,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("linux"),
					Query:  "bar",
					Status: []string{"Skip"},
				},
			},
		},
		{
			Comments: []string{"# Failures 1"},
			Expectations: expectations.Expectations{
				{
					Line:   27,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("mac"),
					Query:  "foo",
					Status: []string{"Failure"},
				},
				{
					Line:   28,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("mac"),
					Query:  "bar",
					Status: []string{"RetryOnFailure"},
				},
			},
		},
		{
			Comments: []string{"# Failures 2", "# Second line"},
			Expectations: expectations.Expectations{
				{
					Line:   32,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("win10"),
					Query:  "foo",
					Status: []string{"Failure"},
				},
				{
					Line:   33,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("win10"),
					Query:  "bar",
					Status: []string{"Failure"},
				},
			},
		},
	}

	require.NoErrorf(t, err, "Got error getting trimmed content: %v", err)
	require.Equal(t, expectedChunks, content.Chunks)
}

func TestGetTrimmedContent_GroupedWithoutSkips(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	expectationFileContent := getExpectationContentForTrimmedContentTest()
	wrapper.WriteFile("/expectations.txt", []byte(expectationFileContent), 0o700)

	content, err := getTrimmedContent(
		"/expectations.txt",
		/*individualExpectations=*/ false,
		/*ignoreSkipExpectations=*/ true,
		/*verbose=*/ false,
		wrapper)

	expectedChunks := []expectations.Chunk{
		{
			Comments: []string{"# Failures 1"},
			Expectations: expectations.Expectations{
				{
					Line:   27,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("mac"),
					Query:  "foo",
					Status: []string{"Failure"},
				},
				{
					Line:   28,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("mac"),
					Query:  "bar",
					Status: []string{"RetryOnFailure"},
				},
			},
		},
		{
			Comments: []string{"# Failures 2", "# Second line"},
			Expectations: expectations.Expectations{
				{
					Line:   32,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("win10"),
					Query:  "foo",
					Status: []string{"Failure"},
				},
				{
					Line:   33,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("win10"),
					Query:  "bar",
					Status: []string{"Failure"},
				},
			},
		},
	}

	require.NoErrorf(t, err, "Got error getting trimmed content: %v", err)
	require.Equal(t, expectedChunks, content.Chunks)
}

func TestGetTrimmedContent_IndividualWithSkips(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	expectationFileContent := getExpectationContentForTrimmedContentTest()
	wrapper.WriteFile("/expectations.txt", []byte(expectationFileContent), 0o700)

	content, err := getTrimmedContent(
		"/expectations.txt",
		/*individualExpectations=*/ true,
		/*ignoreSkipExpectations=*/ false,
		/*verbose=*/ false,
		wrapper)

	expectedChunks := []expectations.Chunk{
		{
			Comments: []string{"# Temporary skips"},
			Expectations: expectations.Expectations{
				{
					Line:   19,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("linux"),
					Query:  "foo",
					Status: []string{"Skip"},
				},
			},
		},
		{
			Comments: []string{"# Temporary skips"},
			Expectations: expectations.Expectations{
				{
					Line:   20,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("linux"),
					Query:  "bar",
					Status: []string{"Skip"},
				},
			},
		},
		{
			Comments: []string{"# Failures 1"},
			Expectations: expectations.Expectations{
				{
					Line:   27,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("mac"),
					Query:  "foo",
					Status: []string{"Failure"},
				},
			},
		},
		{
			Comments: []string{"# Failures 1"},
			Expectations: expectations.Expectations{
				{
					Line:   28,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("mac"),
					Query:  "bar",
					Status: []string{"RetryOnFailure"},
				},
			},
		},
		{
			Comments: []string{"# Failures 2", "# Second line"},
			Expectations: expectations.Expectations{
				{
					Line:   32,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("win10"),
					Query:  "foo",
					Status: []string{"Failure"},
				},
			},
		},
		{
			Comments: []string{"# Failures 2", "# Second line"},
			Expectations: expectations.Expectations{
				{
					Line:   33,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("win10"),
					Query:  "bar",
					Status: []string{"Failure"},
				},
			},
		},
	}

	require.NoErrorf(t, err, "Got error getting trimmed content: %v", err)
	require.Equal(t, expectedChunks, content.Chunks)
}

func TestGetTrimmedContent_IndividualWithoutSkips(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	expectationFileContent := getExpectationContentForTrimmedContentTest()
	wrapper.WriteFile("/expectations.txt", []byte(expectationFileContent), 0o700)

	content, err := getTrimmedContent(
		"/expectations.txt",
		/*individualExpectations=*/ true,
		/*ignoreSkipExpectations=*/ true,
		/*verbose=*/ false,
		wrapper)

	expectedChunks := []expectations.Chunk{
		{
			Comments: []string{"# Failures 1"},
			Expectations: expectations.Expectations{
				{
					Line:   27,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("mac"),
					Query:  "foo",
					Status: []string{"Failure"},
				},
			},
		},
		{
			Comments: []string{"# Failures 1"},
			Expectations: expectations.Expectations{
				{
					Line:   28,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("mac"),
					Query:  "bar",
					Status: []string{"RetryOnFailure"},
				},
			},
		},
		{
			Comments: []string{"# Failures 2", "# Second line"},
			Expectations: expectations.Expectations{
				{
					Line:   32,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("win10"),
					Query:  "foo",
					Status: []string{"Failure"},
				},
			},
		},
		{
			Comments: []string{"# Failures 2", "# Second line"},
			Expectations: expectations.Expectations{
				{
					Line:   33,
					Bug:    "crbug.com/0000",
					Tags:   result.NewTags("win10"),
					Query:  "bar",
					Status: []string{"Failure"},
				},
			},
		},
	}

	require.NoErrorf(t, err, "Got error getting trimmed content: %v", err)
	require.Equal(t, expectedChunks, content.Chunks)
}

/*******************************************************************************
 * getPermanentSkipContent tests
 ******************************************************************************/

func TestGetPermanentSkipContent(t *testing.T) {
	tests := []struct {
		name       string
		content    string
		want       []expectations.Chunk
		wantErr    bool
		wantErrMsg string
	}{
		{ /////////////////////////////////////////////////////////////////////////
			name: "Success",
			content: `# BEGIN TAG HEADER
# OS
# tags: [ android linux mac win10 ]
# END TAG HEADER

# Comment
crbug.com/1234 [ android ] foo [ Skip ]

################################################################################
# Temporary Skip Expectations
################################################################################

crbug.com/2345 [ linux ] bar [ Skip ]
`,
			want: []expectations.Chunk{
				expectations.Chunk{
					Comments: []string{
						"# Comment",
					},
					Expectations: []expectations.Expectation{
						expectations.Expectation{
							Line:  7,
							Bug:   "crbug.com/1234",
							Tags:  result.NewTags("android"),
							Query: "foo",
							Status: []string{
								"Skip",
							},
						},
					},
				},
			},
			wantErr:    false,
			wantErrMsg: "",
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "No temporary skip header",
			content: `# BEGIN TAG HEADER
# OS
# tags: [ android linux mac win10 ]
# END TAG HEADER

# Comment
crbug.com/1234 [ android ] foo [ Skip ]

crbug.com/2345 [ linux ] bar [ Skip ]
`,
			want:       nil,
			wantErr:    false,
			wantErrMsg: "",
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Parse error",
			content: `# BEGIN TAG HEADER
# OS
# tags: [ android linux mac win10 ]
# END TAG HEADER

# Comment
crbug.com/1234 [ android ] foo

################################################################################
# Temporary Skip Expectations
################################################################################

crbug.com/2345 [ linux ] bar [ Skip ]
`,
			want:       nil,
			wantErr:    true,
			wantErrMsg: "expectations.txt:7:31 error: expected status",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			content, err := getPermanentSkipContent("expectations.txt", testCase.content)
			if (err != nil) != testCase.wantErr {
				t.Errorf("getPermanentSkipContent() error = %v, wantErr %v", err, testCase.wantErr)
			}
			if testCase.wantErr && err.Error() != testCase.wantErrMsg {
				t.Errorf("getPermanentSkipContent() errorMsg = %v, wantErrMsg %v", err.Error(), testCase.wantErrMsg)
				return
			}
			if !reflect.DeepEqual(content.Chunks, testCase.want) {
				t.Errorf("getPermanentSkipContent() = %v, want %v", content.Chunks, testCase.want)
				return
			}
		})
	}
}

/*******************************************************************************
 * getChunksOrderedByCoverageLoss tests
 ******************************************************************************/

func TestGetChunksOrderedByCoverageLoss_FewResults(t *testing.T) {
	content := expectations.Content{
		Chunks: []expectations.Chunk{
			// Should not apply to anything.
			{
				Expectations: expectations.Expectations{
					{
						Query: "fake_test",
						Tags:  result.NewTags("android"),
					},
					{
						Query: "real_test",
						Tags:  result.NewTags("fake_tag"),
					},
				},
			},
			// Should apply to everything due to the first expectation.
			{
				Expectations: expectations.Expectations{
					{
						Query: "*",
						Tags:  result.NewTags(),
					},
					{
						Query: "fake_test",
						Tags:  result.NewTags("android"),
					},
				},
			},
			// Should apply to everything due to the second expectation.
			{
				Expectations: expectations.Expectations{
					{
						Query: "fake_test",
						Tags:  result.NewTags("android"),
					},
					{
						Query: "*",
						Tags:  result.NewTags(),
					},
				},
			},
			// Should only apply to a single result.
			{
				Expectations: expectations.Expectations{
					{
						Query: "real_test_2",
						Tags:  result.NewTags("android"),
					},
				},
			},
			// Should apply to all Android results, and thus be fairly high up.
			{
				Expectations: expectations.Expectations{
					{
						Query: "*",
						Tags:  result.NewTags("android"),
					},
				},
			},
			// Should apply to all Linux results, and thus should be below the
			// Android one.
			{
				Expectations: expectations.Expectations{
					{
						Query: "*",
						Tags:  result.NewTags("linux"),
					},
				},
			},
			// Should also apply to all Linux results, but should not end up double
			// counting.
			{
				Expectations: expectations.Expectations{
					{
						Query: "real_*",
						Tags:  result.NewTags("linux"),
					},
					{
						Query: "real_test_1",
						Tags:  result.NewTags("linux"),
					},
					{
						Query: "real_test_2",
						Tags:  result.NewTags("linux"),
					},
				},
			},
		},
	}

	uniqueResults := result.List{
		{
			Query: query.Parse("real_test_1"),
			Tags:  result.NewTags("android"),
		},
		{
			Query: query.Parse("real_test_2"),
			Tags:  result.NewTags("android"),
		},
		{
			Query: query.Parse("real_test_3"),
			Tags:  result.NewTags("android"),
		},
		{
			Query: query.Parse("real_test_1"),
			Tags:  result.NewTags("linux"),
		},
		{
			Query: query.Parse("real_test_2"),
			Tags:  result.NewTags("linux"),
		},
	}

	expectedChunkCounts := []ChunkWithCounter{
		{
			Chunk: &content.Chunks[1],
			Count: 5,
		},
		{
			Chunk: &content.Chunks[2],
			Count: 5,
		},
		{
			Chunk: &content.Chunks[4],
			Count: 3,
		},
		{
			Chunk: &content.Chunks[5],
			Count: 2,
		},
		{
			Chunk: &content.Chunks[6],
			Count: 2,
		},
		{
			Chunk: &content.Chunks[3],
			Count: 1,
		},
		{
			Chunk: &content.Chunks[0],
			Count: 0,
		},
	}

	actualChunkCounts := getChunksOrderedByCoverageLoss(&content, &uniqueResults)
	require.Equal(t, expectedChunkCounts, actualChunkCounts)
}

func TestGetChunksOrderedByCoverageLoss_ManyResults(t *testing.T) {
	// This is primarily to exercise code when there are multiple subworkers,
	// which requires a large number of results.
	content := expectations.Content{
		Chunks: []expectations.Chunk{
			{
				Expectations: expectations.Expectations{
					{
						Query: "*",
						Tags:  result.NewTags("android"),
					},
				},
			},
			{
				Expectations: expectations.Expectations{
					{
						Query: "*",
						Tags:  result.NewTags("linux"),
					},
				},
			},
		},
	}

	uniqueResults := result.List{}
	for i := 0; i < maxResultsPerWorker*2; i++ {
		r := result.Result{
			Query: query.Parse(fmt.Sprintf("real_test_%d", i)),
			Tags:  result.NewTags("android"),
		}
		uniqueResults = append(uniqueResults, r)

		if i%2 == 0 {
			r = result.Result{
				Query: query.Parse(fmt.Sprintf("real_test_%d", i)),
				Tags:  result.NewTags("linux"),
			}
			uniqueResults = append(uniqueResults, r)
		}
	}

	expectedChunkCounts := []ChunkWithCounter{
		{
			Chunk: &content.Chunks[0],
			Count: maxResultsPerWorker * 2,
		},
		{
			Chunk: &content.Chunks[1],
			Count: maxResultsPerWorker,
		},
	}

	actualChunkCounts := getChunksOrderedByCoverageLoss(&content, &uniqueResults)
	require.Equal(t, expectedChunkCounts, actualChunkCounts)
}

/*******************************************************************************
 * outputResults tests
 ******************************************************************************/

func getOutputResultsOrderedChunks() []ChunkWithCounter {
	chunks := []expectations.Chunk{
		{
			Comments: []string{"# Linux"},
			Expectations: expectations.Expectations{
				{
					Line:    5,
					Bug:     "crbug.com/1234",
					Tags:    result.NewTags("linux"),
					Query:   "real_test_1",
					Status:  []string{"Failure"},
					Comment: "# trailing comment",
				},
				{
					Line:   6,
					Bug:    "crbug.com/2345",
					Tags:   result.NewTags("linux"),
					Query:  "real_test_2",
					Status: []string{"Failure"},
				},
			},
		},
		{
			Comments: []string{"# Android", "# Second comment line"},
			Expectations: expectations.Expectations{
				{
					Line:   10,
					Bug:    "crbug.com/3456",
					Tags:   result.NewTags("android"),
					Query:  "real_test_*",
					Status: []string{"Failure"},
				},
			},
		},
	}

	return []ChunkWithCounter{
		{
			Chunk: &chunks[1],
			Count: 10,
		},
		{
			Chunk: &chunks[0],
			Count: 5,
		},
	}
}

func TestOutputResults(t *testing.T) {
	tests := []struct {
		name                   string
		maxChunksToOutput      int
		individualExpectations bool
		expectedOutput         string
	}{
		{
			name:                   "Output all as chunks",
			maxChunksToOutput:      0,
			individualExpectations: false,
			expectedOutput: `
Complete output:

Comment: # Android
# Second comment line
First expectation: crbug.com/3456 [ android ] real_test_* [ Failure ]
Line number: 10
Affected 10 test results

Comment: # Linux
First expectation: crbug.com/1234 [ linux ] real_test_1 [ Failure ] # trailing comment
Line number: 5
Affected 5 test results
`,
		},
		{
			name:                   "Output all as individual expectations",
			maxChunksToOutput:      0,
			individualExpectations: true,
			expectedOutput: `
Complete output:

Comment: # Android
# Second comment line
Expectation: crbug.com/3456 [ android ] real_test_* [ Failure ]
Line number: 10
Affected 10 test results

Comment: # Linux
Expectation: crbug.com/1234 [ linux ] real_test_1 [ Failure ] # trailing comment
Line number: 5
Affected 5 test results
`,
		},
		{
			name:                   "Output one as chunk",
			maxChunksToOutput:      1,
			individualExpectations: false,
			expectedOutput: `
Top 1 chunks contributing to test coverage loss:

Comment: # Android
# Second comment line
First expectation: crbug.com/3456 [ android ] real_test_* [ Failure ]
Line number: 10
Affected 10 test results
`,
		},
		{
			name:                   "Output one as individual expectations",
			maxChunksToOutput:      1,
			individualExpectations: true,
			expectedOutput: `
Top 1 individual expectations contributing to test coverage loss:

Comment: # Android
# Second comment line
Expectation: crbug.com/3456 [ android ] real_test_* [ Failure ]
Line number: 10
Affected 10 test results
`,
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			buffer := bytes.Buffer{}
			outputResults(
				getOutputResultsOrderedChunks(),
				testCase.maxChunksToOutput,
				testCase.individualExpectations,
				&buffer)
			require.Equal(t, testCase.expectedOutput, buffer.String())
		})
	}
}
