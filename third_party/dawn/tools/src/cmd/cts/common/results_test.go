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

package common

import (
	"context"
	"fmt"
	"path/filepath"
	"testing"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/resultsdb"
	"github.com/stretchr/testify/require"
)

// TODO(crbug.com/342554800): Add test coverage for:
//   ResultSource.GetResults (requires breaking out into helper functions)
//   ResultSource.GetUnsuppressedFailingResults (ditto)
//   LatestCTSRoll
//   LatestPatchset
//   MostRecentResultsForChange (requires os abstraction crbug.com/344014313)
//   MostRecentUnsuppressedFailingResultsForChange (ditto)

/*******************************************************************************
 * CacheResults tests
 ******************************************************************************/

func getCacheResultsSharedSetupData() (
	context.Context, Config, oswrapper.MemMapOSWrapper, gerrit.Patchset, string) {

	ctx := context.Background()
	wrapper := oswrapper.CreateMemMapOSWrapper()
	patchset := gerrit.Patchset{
		Change:   1,
		Patchset: 2,
	}
	cacheDir := "/cache"
	cfg := Config{
		Tests: []TestConfig{
			{
				ExecutionMode: result.ExecutionMode("execution_mode"),
				Prefixes:      []string{"prefix"},
			},
		},
		OsWrapper: wrapper,
	}
	cfg.Tag.Remove = []string{"tag_to_remove"}

	return ctx, cfg, wrapper, patchset, cacheDir
}

func TestCacheResults_CacheHit(t *testing.T) {
	testCacheResults_CacheHit_Impl(t, false)
}

func TestCacheUnsuppressedFailingResults_CacheHit(t *testing.T) {
	testCacheResults_CacheHit_Impl(t, true)
}

func testCacheResults_CacheHit_Impl(t *testing.T, unsuppressedOnly bool) {
	client := resultsdb.MockBigQueryClient{}
	var testedFunc cacheResultsFunc
	var clientDataField *resultsdb.PrefixGroupedQueryResults
	var partialCacheFilePath string
	if unsuppressedOnly {
		testedFunc = CacheUnsuppressedFailingResults
		clientDataField = &client.UnsuppressedFailureReturnValues
		partialCacheFilePath = filepath.Join("1", "ps-2-unsuppressed-failures.txt")
	} else {
		testedFunc = CacheResults
		clientDataField = &client.ReturnValues
		partialCacheFilePath = filepath.Join("1", "ps-2.txt")
	}

	ctx, cfg, wrapper, patchset, cacheDir := getCacheResultsSharedSetupData()

	// Return bad data to ensure that no querying occurs when the cache is hit.
	*clientDataField = resultsdb.PrefixGroupedQueryResults{
		"prefix": []resultsdb.QueryResult{
			{
				TestId:   "bad_test",
				Status:   "FAIL",
				Tags:     []resultsdb.TagPair{},
				Duration: 1.0,
			},
		},
	}

	cachedResults := result.ResultsByExecutionMode{
		"execution_mode": result.List{
			{
				Query:        query.Parse("_test_1"),
				Tags:         result.NewTags("tag_1"),
				Status:       result.Failure,
				Duration:     0,
				MayExonerate: false,
			},
		},
	}

	cachePath := filepath.Join(cacheDir, partialCacheFilePath)
	err := result.SaveWithWrapper(cachePath, cachedResults, wrapper)
	require.NoErrorf(t, err, "Got error writing results: %v", err)

	resultsByExecutionMode, err := testedFunc(ctx, cfg, patchset, cacheDir, client, BuildsByName{})
	require.NoErrorf(t, err, "Got error caching results: %v", err)
	require.Equal(t, cachedResults, resultsByExecutionMode)
}

func TestCacheResults_GetRawResultsError(t *testing.T) {
	testCacheResults_GetRawResultsError_Impl(t, false)
}

func TestCacheUnsuppressedFailingResults_GetRawResultsError(t *testing.T) {
	testCacheResults_GetRawResultsError_Impl(t, true)
}

func testCacheResults_GetRawResultsError_Impl(t *testing.T, unsuppressedOnly bool) {
	client := resultsdb.MockBigQueryClient{}
	var testedFunc cacheResultsFunc
	var clientDataField *resultsdb.PrefixGroupedQueryResults
	if unsuppressedOnly {
		testedFunc = CacheUnsuppressedFailingResults
		clientDataField = &client.UnsuppressedFailureReturnValues
	} else {
		testedFunc = CacheResults
		clientDataField = &client.ReturnValues
	}

	ctx, cfg, _, patchset, cacheDir := getCacheResultsSharedSetupData()

	*clientDataField = resultsdb.PrefixGroupedQueryResults{
		"prefix": []resultsdb.QueryResult{
			{
				TestId:   "bad_test",
				Status:   "FAIL",
				Tags:     []resultsdb.TagPair{},
				Duration: 1.0,
			},
		},
	}

	resultsByExecutionMode, err := testedFunc(ctx, cfg, patchset, cacheDir, client, BuildsByName{})
	require.Nil(t, resultsByExecutionMode)
	require.ErrorContains(t, err,
		"Test ID bad_test did not start with prefix even though query should have filtered.")
}

func TestCacheResults_Success(t *testing.T) {
	testCacheResults_Success_Impl(t, false)
}

func TestCacheUnsuppressedFailingResults_Success(t *testing.T) {
	testCacheResults_Success_Impl(t, true)
}

func testCacheResults_Success_Impl(t *testing.T, unsuppressedOnly bool) {
	client := resultsdb.MockBigQueryClient{}
	var testedFunc cacheResultsFunc
	var clientDataField *resultsdb.PrefixGroupedQueryResults
	if unsuppressedOnly {
		testedFunc = CacheUnsuppressedFailingResults
		clientDataField = &client.UnsuppressedFailureReturnValues
	} else {
		testedFunc = CacheResults
		clientDataField = &client.ReturnValues
	}

	ctx, cfg, _, patchset, cacheDir := getCacheResultsSharedSetupData()

	*clientDataField = resultsdb.PrefixGroupedQueryResults{
		"prefix": []resultsdb.QueryResult{
			{
				TestId: "prefix_test_2",
				Status: "CRASH",
				Tags: []resultsdb.TagPair{
					{
						Key:   "typ_tag",
						Value: "tag_2",
					},
				},
				Duration: 2,
			},
			{
				TestId: "prefix_test_1",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					{
						Key:   "typ_tag",
						Value: "tag_1",
					},
					// This should be removed by CleanResults().
					{
						Key:   "typ_tag",
						Value: "tag_to_remove",
					},
				},
				Duration: 5,
			},
			// Should be merged into the above result by CleanResults()
			{
				TestId: "prefix_test_1",
				Status: "PASS",
				Tags: []resultsdb.TagPair{
					{
						Key:   "typ_tag",
						Value: "tag_1",
					},
					// This should be removed by CleanResults().
					{
						Key:   "typ_tag",
						Value: "tag_to_remove",
					},
				},
				Duration: 1,
			},
		},
	}

	expectedResults := result.ResultsByExecutionMode{
		"execution_mode": result.List{
			{
				Query:        query.Parse("_test_1"),
				Tags:         result.NewTags("tag_1"),
				Status:       result.Failure,
				Duration:     3000000000,
				MayExonerate: false,
			},
			{
				Query:        query.Parse("_test_2"),
				Tags:         result.NewTags("tag_2"),
				Status:       result.Crash,
				Duration:     2000000000,
				MayExonerate: false,
			},
		},
	}

	// Check that the initial results are retrieved and cleaned properly.
	resultsByExecutionMode, err := testedFunc(
		ctx, cfg, patchset, cacheDir, client, BuildsByName{})
	require.NoErrorf(t, err, "Got error caching results: %v", err)
	require.Equal(t, expectedResults, resultsByExecutionMode)

	// Check that the results were cached and that hitting the cache still results
	// in cleaned data.
	client = resultsdb.MockBigQueryClient{}
	resultsByExecutionMode, err = testedFunc(
		ctx, cfg, patchset, cacheDir, client, BuildsByName{})
	require.NoErrorf(t, err, "Got error caching results: %v", err)
	require.Equal(t, expectedResults, resultsByExecutionMode)
}

/*******************************************************************************
 * GetResults tests
 ******************************************************************************/

func generateGoodGetResultsInputs() (
	context.Context, Config, *resultsdb.MockBigQueryClient, BuildsByName) {

	ctx := context.Background()

	cfg := Config{
		Tests: []TestConfig{
			TestConfig{
				ExecutionMode: result.ExecutionMode("execution_mode"),
				Prefixes:      []string{"prefix"},
			},
		},
	}

	client := &resultsdb.MockBigQueryClient{
		ReturnValues: resultsdb.PrefixGroupedQueryResults{
			"prefix": []resultsdb.QueryResult{
				resultsdb.QueryResult{
					TestId:   "prefix_test",
					Status:   "PASS",
					Tags:     []resultsdb.TagPair{},
					Duration: 1.0,
				},
			},
		},
	}

	builds := make(BuildsByName)

	return ctx, cfg, client, builds
}

// Tests that valid results are properly cleaned, sorted, and returned.
func TestGetResultsHappyPath(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetResultsInputs()

	cfg.Tag.Remove = []string{
		"remove_me",
	}

	client.ReturnValues = resultsdb.PrefixGroupedQueryResults{
		"prefix": []resultsdb.QueryResult{
			resultsdb.QueryResult{
				TestId: "prefix_test_2",
				Status: "PASS",
				Tags: []resultsdb.TagPair{
					resultsdb.TagPair{
						Key:   "typ_tag",
						Value: "remove_me",
					},
					resultsdb.TagPair{
						Key:   "typ_tag",
						Value: "win",
					},
				},
				Duration: 2.0,
			},
			resultsdb.QueryResult{
				TestId: "prefix_test_1",
				Status: "PASS",
				Tags: []resultsdb.TagPair{
					resultsdb.TagPair{
						Key:   "typ_tag",
						Value: "linux",
					},
				},
				Duration: 1.0,
			},
		},
	}

	expectedResultsList := result.List{
		result.Result{
			Query:        query.Parse("_test_1"),
			Status:       result.Pass,
			Tags:         result.NewTags("linux"),
			Duration:     time.Second,
			MayExonerate: false,
		},
		result.Result{
			Query:        query.Parse("_test_2"),
			Status:       result.Pass,
			Tags:         result.NewTags("win"),
			Duration:     2 * time.Second,
			MayExonerate: false,
		},
	}

	expectedResults := make(result.ResultsByExecutionMode)
	expectedResults["execution_mode"] = expectedResultsList

	results, err := GetResults(ctx, cfg, client, builds)
	require.Nil(t, err)
	require.Equal(t, results, expectedResults)
}

// Tests that errors from GetRawResults are properly surfaced.
func TestGetResultsGetRawResultsErrorSurfaced(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetResultsInputs()
	client.ReturnValues["prefix"][0].TestId = "bad_test"

	results, err := GetResults(ctx, cfg, client, builds)
	require.Nil(t, results)
	require.ErrorContains(t, err, "Test ID bad_test did not start with prefix even though query should have filtered.")
}

/*******************************************************************************
 * GetUnsuppressedFailingResults tests
 ******************************************************************************/

func generateGoodGetUnsuppressedFailingResultsInputs() (
	context.Context, Config, *resultsdb.MockBigQueryClient, BuildsByName) {

	ctx := context.Background()

	cfg := Config{
		Tests: []TestConfig{
			TestConfig{
				ExecutionMode: result.ExecutionMode("execution_mode"),
				Prefixes:      []string{"prefix"},
			},
		},
	}

	client := &resultsdb.MockBigQueryClient{
		UnsuppressedFailureReturnValues: resultsdb.PrefixGroupedQueryResults{
			"prefix": []resultsdb.QueryResult{
				resultsdb.QueryResult{
					TestId:   "prefix_test",
					Status:   "FAIL",
					Tags:     []resultsdb.TagPair{},
					Duration: 1.0,
				},
			},
		},
	}

	builds := make(BuildsByName)

	return ctx, cfg, client, builds
}

// Tests that valid results are properly cleaned, sorted, and returned.
func TestGetUnsuppressedFailingResultsHappyPath(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetUnsuppressedFailingResultsInputs()

	cfg.Tag.Remove = []string{
		"remove_me",
	}

	client.UnsuppressedFailureReturnValues = resultsdb.PrefixGroupedQueryResults{
		"prefix": []resultsdb.QueryResult{
			resultsdb.QueryResult{
				TestId: "prefix_test_2",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					resultsdb.TagPair{
						Key:   "typ_tag",
						Value: "remove_me",
					},
					resultsdb.TagPair{
						Key:   "typ_tag",
						Value: "win",
					},
				},
				Duration: 2.0,
			},
			resultsdb.QueryResult{
				TestId: "prefix_test_1",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					resultsdb.TagPair{
						Key:   "typ_tag",
						Value: "linux",
					},
				},
				Duration: 1.0,
			},
		},
	}

	expectedResultsList := result.List{
		result.Result{
			Query:        query.Parse("_test_1"),
			Status:       result.Failure,
			Tags:         result.NewTags("linux"),
			Duration:     time.Second,
			MayExonerate: false,
		},
		result.Result{
			Query:        query.Parse("_test_2"),
			Status:       result.Failure,
			Tags:         result.NewTags("win"),
			Duration:     2 * time.Second,
			MayExonerate: false,
		},
	}

	expectedResults := make(result.ResultsByExecutionMode)
	expectedResults["execution_mode"] = expectedResultsList

	results, err := GetUnsuppressedFailingResults(ctx, cfg, client, builds)
	require.Nil(t, err)
	require.Equal(t, results, expectedResults)
}

// Tests that errors from GetRawResults are properly surfaced.
func TestGetUnsuppressedFailingResultsGetRawResultsErrorSurfaced(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetUnsuppressedFailingResultsInputs()
	client.UnsuppressedFailureReturnValues["prefix"][0].TestId = "bad_test"

	results, err := GetUnsuppressedFailingResults(ctx, cfg, client, builds)
	require.Nil(t, results)
	require.ErrorContains(t, err, "Test ID bad_test did not start with prefix even though query should have filtered.")
}

/*******************************************************************************
 * GetRawResults tests
 ******************************************************************************/

// Tests that valid results are properly parsed and returned.
func TestGetRawResultsHappyPath(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetResultsInputs()
	client.ReturnValues = resultsdb.PrefixGroupedQueryResults{
		"prefix": []resultsdb.QueryResult{
			resultsdb.QueryResult{
				TestId:   "prefix_test_1",
				Status:   "PASS",
				Tags:     []resultsdb.TagPair{},
				Duration: 1.0,
			},
			resultsdb.QueryResult{
				TestId: "prefix_test_2",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					resultsdb.TagPair{
						Key:   "javascript_duration",
						Value: "0.5s",
					},
				},
				Duration: 2.0,
			},
			resultsdb.QueryResult{
				TestId: "prefix_test_3",
				Status: "SKIP",
				Tags: []resultsdb.TagPair{
					resultsdb.TagPair{
						Key:   "may_exonerate",
						Value: "true",
					},
				},
				Duration: 3.0,
			},
			resultsdb.QueryResult{
				TestId: "prefix_test_4",
				Status: "SomeStatus",
				Tags: []resultsdb.TagPair{
					resultsdb.TagPair{
						Key:   "typ_tag",
						Value: "linux",
					},
					resultsdb.TagPair{
						Key:   "typ_tag",
						Value: "intel",
					},
				},
				Duration: 4.0,
			},
		},
	}

	expectedResultsList := result.List{
		result.Result{
			Query:        query.Parse("_test_1"),
			Status:       result.Pass,
			Tags:         result.NewTags(),
			Duration:     time.Second,
			MayExonerate: false,
		},
		result.Result{
			Query:        query.Parse("_test_2"),
			Status:       result.Failure,
			Tags:         result.NewTags(),
			Duration:     500 * time.Millisecond,
			MayExonerate: false,
		},
		result.Result{
			Query:        query.Parse("_test_3"),
			Status:       result.Skip,
			Tags:         result.NewTags(),
			Duration:     3 * time.Second,
			MayExonerate: true,
		},
		result.Result{
			Query:        query.Parse("_test_4"),
			Status:       result.Unknown,
			Tags:         result.NewTags("linux", "intel"),
			Duration:     4 * time.Second,
			MayExonerate: false,
		},
	}

	expectedResults := make(result.ResultsByExecutionMode)
	expectedResults["execution_mode"] = expectedResultsList

	results, err := GetRawResults(ctx, cfg, client, builds)
	require.Nil(t, err)
	require.Equal(t, results, expectedResults)
}

// Tests that a mismatched prefix results in an error.
func TestGetRawResultsPrefixMismatch(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetResultsInputs()
	client.ReturnValues["prefix"][0].TestId = "bad_test"

	results, err := GetRawResults(ctx, cfg, client, builds)
	require.Nil(t, results)
	require.ErrorContains(t, err, "Test ID bad_test did not start with prefix even though query should have filtered.")
}

// Tests that a JavaScript duration that cannot be parsed results in an error.
func TestGetRawResultsBadJavaScriptDuration(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetResultsInputs()
	client.ReturnValues["prefix"][0].Tags = []resultsdb.TagPair{
		resultsdb.TagPair{
			Key:   "javascript_duration",
			Value: "1000foo",
		},
	}

	results, err := GetRawResults(ctx, cfg, client, builds)
	require.Nil(t, results)
	require.ErrorContains(t, err, `time: unknown unit "foo" in duration "1000foo"`)
}

// Tests that a non-boolean may_exonerate value results in an error.
func TestGetRawResultsBadMayExonerate(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetResultsInputs()
	client.ReturnValues["prefix"][0].Tags = []resultsdb.TagPair{
		resultsdb.TagPair{
			Key:   "may_exonerate",
			Value: "yesnt",
		},
	}

	results, err := GetRawResults(ctx, cfg, client, builds)
	require.Nil(t, results)
	require.ErrorContains(t, err, `strconv.ParseBool: parsing "yesnt": invalid syntax`)
}

/*******************************************************************************
 * convertRdbStatus tests
 ******************************************************************************/

func TestConvertRdbStatus(t *testing.T) {
	tests := []struct {
		name  string
		input string
		want  result.Status
	}{
		{
			name:  "Unknown",
			input: "asdf",
			want:  result.Unknown,
		},
		{
			name:  "Pass",
			input: "PASS",
			want:  result.Pass,
		},
		{
			name:  "Failure",
			input: "FAIL",
			want:  result.Failure,
		},
		{
			name:  "Crash",
			input: "CRASH",
			want:  result.Crash,
		},
		{
			name:  "Abort",
			input: "ABORT",
			want:  result.Abort,
		},
		{
			name:  "Skip",
			input: "SKIP",
			want:  result.Skip,
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			status := convertRdbStatus(testCase.input)
			require.Equal(t, testCase.want, status)
		})
	}
}

/*******************************************************************************
 * GetRawUnsuppressedFailingResults tests
 ******************************************************************************/

// Tests that valid results are properly parsed and returned.
func TestGetRawUnsuppressedFailingResultsHappyPath(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetUnsuppressedFailingResultsInputs()
	client.UnsuppressedFailureReturnValues = resultsdb.PrefixGroupedQueryResults{
		"prefix": []resultsdb.QueryResult{
			resultsdb.QueryResult{
				TestId:   "prefix_test_1",
				Status:   "FAIL",
				Tags:     []resultsdb.TagPair{},
				Duration: 1.0,
			},
			resultsdb.QueryResult{
				TestId: "prefix_test_2",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					resultsdb.TagPair{
						Key:   "javascript_duration",
						Value: "0.5s",
					},
				},
				Duration: 2.0,
			},
			resultsdb.QueryResult{
				TestId: "prefix_test_3",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					resultsdb.TagPair{
						Key:   "may_exonerate",
						Value: "true",
					},
				},
				Duration: 3.0,
			},
			resultsdb.QueryResult{
				TestId: "prefix_test_4",
				Status: "SomeStatus",
				Tags: []resultsdb.TagPair{
					resultsdb.TagPair{
						Key:   "typ_tag",
						Value: "linux",
					},
					resultsdb.TagPair{
						Key:   "typ_tag",
						Value: "intel",
					},
				},
				Duration: 4.0,
			},
		},
	}

	expectedResultsList := result.List{
		result.Result{
			Query:        query.Parse("_test_1"),
			Status:       result.Failure,
			Tags:         result.NewTags(),
			Duration:     time.Second,
			MayExonerate: false,
		},
		result.Result{
			Query:        query.Parse("_test_2"),
			Status:       result.Failure,
			Tags:         result.NewTags(),
			Duration:     500 * time.Millisecond,
			MayExonerate: false,
		},
		result.Result{
			Query:        query.Parse("_test_3"),
			Status:       result.Failure,
			Tags:         result.NewTags(),
			Duration:     3 * time.Second,
			MayExonerate: true,
		},
		result.Result{
			Query:        query.Parse("_test_4"),
			Status:       result.Unknown,
			Tags:         result.NewTags("linux", "intel"),
			Duration:     4 * time.Second,
			MayExonerate: false,
		},
	}

	expectedResults := make(result.ResultsByExecutionMode)
	expectedResults["execution_mode"] = expectedResultsList

	results, err := GetRawUnsuppressedFailingResults(ctx, cfg, client, builds)
	require.Nil(t, err)
	require.Equal(t, results, expectedResults)
}

// Tests that a mismatched prefix results in an error.
func TestGetRawUnsuppressedFailingResultsPrefixMismatch(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetUnsuppressedFailingResultsInputs()
	client.UnsuppressedFailureReturnValues["prefix"][0].TestId = "bad_test"

	results, err := GetRawUnsuppressedFailingResults(ctx, cfg, client, builds)
	require.Nil(t, results)
	require.ErrorContains(t, err, "Test ID bad_test did not start with prefix even though query should have filtered.")
}

// Tests that a JavaScript duration that cannot be parsed results in an error.
func TestGetRawUnsuppressedFailingResultsBadJavaScriptDuration(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetUnsuppressedFailingResultsInputs()
	client.UnsuppressedFailureReturnValues["prefix"][0].Tags = []resultsdb.TagPair{
		resultsdb.TagPair{
			Key:   "javascript_duration",
			Value: "1000foo",
		},
	}

	results, err := GetRawUnsuppressedFailingResults(ctx, cfg, client, builds)
	require.Nil(t, results)
	require.ErrorContains(t, err, `time: unknown unit "foo" in duration "1000foo"`)
}

// Tests that a non-boolean may_exonerate value results in an error.
func TestGetRawUnsuppressedFailingResultsBadMayExonerate(t *testing.T) {
	ctx, cfg, client, builds := generateGoodGetUnsuppressedFailingResultsInputs()
	client.UnsuppressedFailureReturnValues["prefix"][0].Tags = []resultsdb.TagPair{
		resultsdb.TagPair{
			Key:   "may_exonerate",
			Value: "yesnt",
		},
	}

	results, err := GetRawUnsuppressedFailingResults(ctx, cfg, client, builds)
	require.Nil(t, results)
	require.ErrorContains(t, err, `strconv.ParseBool: parsing "yesnt": invalid syntax`)
}

/*******************************************************************************
 * CleanResults tests
 ******************************************************************************/

// Tests that tags specified for removal are properly removed.
func TestCleanResultsTagRemoval(t *testing.T) {
	cfg := Config{}
	cfg.Tag.Remove = []string{
		"remove_1",
		"remove_2",
		"remove_3",
		"missing",
	}

	results := result.List{
		result.Result{
			Query:        query.Parse("test_1"),
			Status:       result.Pass,
			Tags:         result.NewTags("remove_1", "remove_2", "linux"),
			Duration:     time.Second,
			MayExonerate: false,
		},
		result.Result{
			Query:        query.Parse("test_2"),
			Status:       result.Pass,
			Tags:         result.NewTags("remove_2", "remove_3"),
			Duration:     time.Second,
			MayExonerate: false,
		},
	}

	expectedResults := result.List{
		result.Result{
			Query:        query.Parse("test_1"),
			Status:       result.Pass,
			Tags:         result.NewTags("linux"),
			Duration:     time.Second,
			MayExonerate: false,
		},
		result.Result{
			Query:        query.Parse("test_2"),
			Status:       result.Pass,
			Tags:         result.NewTags(),
			Duration:     time.Second,
			MayExonerate: false,
		},
	}

	CleanResults(cfg, &results)
	require.Equal(t, results, expectedResults)
}

// Tests that duplicate results with the same status always use that status.
func TestCleanResultsDuplicateResultSameStatus(t *testing.T) {
	cfg := Config{}
	for _, status := range []result.Status{result.Abort, result.Crash,
		result.Failure, result.Pass, result.RetryOnFailure, result.Skip, result.Slow, result.Unknown} {
		results := result.List{
			result.Result{
				Query:        query.Parse("test_1"),
				Status:       status,
				Tags:         result.NewTags(),
				Duration:     time.Second,
				MayExonerate: false,
			},
			result.Result{
				Query:        query.Parse("test_1"),
				Status:       status,
				Tags:         result.NewTags(),
				Duration:     3 * time.Second,
				MayExonerate: false,
			},
		}

		expectedResults := result.List{
			result.Result{
				Query:        query.Parse("test_1"),
				Status:       status,
				Tags:         result.NewTags(),
				Duration:     2 * time.Second,
				MayExonerate: false,
			},
		}

		CleanResults(cfg, &results)
		require.Equal(t, results, expectedResults)
	}
}

func runPriorityTest(t *testing.T, testedStatus result.Status, lowerPriorityStatuses *[]result.Status) {
	cfg := Config{}
	for _, status := range *lowerPriorityStatuses {
		results := result.List{
			result.Result{
				Query:        query.Parse("test_1"),
				Status:       status,
				Tags:         result.NewTags(),
				Duration:     time.Second,
				MayExonerate: false,
			},
			result.Result{
				Query:        query.Parse("test_1"),
				Status:       testedStatus,
				Tags:         result.NewTags(),
				Duration:     3 * time.Second,
				MayExonerate: false,
			},
		}

		expectedResults := result.List{
			result.Result{
				Query:        query.Parse("test_1"),
				Status:       testedStatus,
				Tags:         result.NewTags(),
				Duration:     2 * time.Second,
				MayExonerate: false,
			},
		}

		CleanResults(cfg, &results)
		require.Equal(t, results, expectedResults)
	}
}

// Tests that Crash has the highest priority among statuses.
func TestCleanResultsReplaceResultCrash(t *testing.T) {
	lowerPriorityStatuses := []result.Status{result.Abort, result.Failure, result.Slow, result.Pass, result.RetryOnFailure, result.Skip, result.Unknown}
	runPriorityTest(t, result.Crash, &lowerPriorityStatuses)
}

// Tests that Abort has the second highest priority among statuses.
func TestCleanResultsReplaceResultAbort(t *testing.T) {
	lowerPriorityStatuses := []result.Status{result.Failure, result.Slow, result.Pass, result.RetryOnFailure, result.Skip, result.Unknown}
	runPriorityTest(t, result.Abort, &lowerPriorityStatuses)
}

// Tests that Failure has the third highest priority among statuses.
func TestCleanResultsReplaceResultFailure(t *testing.T) {
	lowerPriorityStatuses := []result.Status{result.Slow, result.Pass, result.RetryOnFailure, result.Skip, result.Unknown}
	runPriorityTest(t, result.Failure, &lowerPriorityStatuses)
}

// Tests that Slow has the fourth highest priority among statuses.
func TestCleanResultsReplaceResultSlow(t *testing.T) {
	lowerPriorityStatuses := []result.Status{result.Pass, result.RetryOnFailure, result.Skip, result.Unknown}
	runPriorityTest(t, result.Slow, &lowerPriorityStatuses)
}

// Tests that statuses without explicit priority default to Failure.
func TestCleanResultsReplaceResultDefault(t *testing.T) {
	cfg := Config{}
	statuses := []result.Status{result.Pass, result.RetryOnFailure, result.Skip, result.Unknown}
	for i, leftStatus := range statuses {
		for j, rightStatus := range statuses {
			if i == j {
				continue
			}

			results := result.List{
				result.Result{
					Query:        query.Parse("test_1"),
					Status:       leftStatus,
					Tags:         result.NewTags(),
					Duration:     time.Second,
					MayExonerate: false,
				},
				result.Result{
					Query:        query.Parse("test_1"),
					Status:       rightStatus,
					Tags:         result.NewTags(),
					Duration:     3 * time.Second,
					MayExonerate: false,
				},
			}

			expectedResults := result.List{
				result.Result{
					Query:        query.Parse("test_1"),
					Status:       result.Failure,
					Tags:         result.NewTags(),
					Duration:     2 * time.Second,
					MayExonerate: false,
				},
			}

			CleanResults(cfg, &results)
			require.Equal(t, results, expectedResults)
		}
	}
}

/*******************************************************************************
 * CacheRecentUniqueSuppressed tests
 ******************************************************************************/

func getMultiPrefixConfig() Config {
	return Config{
		Tests: []TestConfig{
			TestConfig{
				ExecutionMode: result.ExecutionMode("core"),
				Prefixes:      []string{"core_prefix"},
			},
			TestConfig{
				ExecutionMode: result.ExecutionMode("compat"),
				Prefixes:      []string{"compat_prefix"},
			},
		},
	}
}

func getMultiPrefixQueryResults() resultsdb.PrefixGroupedQueryResults {
	return resultsdb.PrefixGroupedQueryResults{
		"core_prefix": []resultsdb.QueryResult{
			{
				TestId: "core_prefix_test_1",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					{
						Key:   "typ_tag",
						Value: "tag_1",
					},
				},
				Duration: 1.0,
			},
			{
				TestId: "core_prefix_test_2",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					{
						Key:   "typ_tag",
						Value: "tag_2",
					},
				},
				Duration: 1.0,
			},
		},
		"compat_prefix": []resultsdb.QueryResult{
			{
				TestId: "compat_prefix_test_3",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					{
						Key:   "typ_tag",
						Value: "tag_3",
					},
				},
				Duration: 1.0,
			},
			{
				TestId: "compat_prefix_test_4",
				Status: "FAIL",
				Tags: []resultsdb.TagPair{
					{
						Key:   "typ_tag",
						Value: "tag_4",
					},
				},
				Duration: 1.0,
			},
		},
	}
}

func getExpectedMultiPrefixResults() result.ResultsByExecutionMode {
	return result.ResultsByExecutionMode{
		"core": result.List{
			{
				Query:        query.Parse("_test_1"),
				Tags:         result.NewTags("tag_1"),
				Status:       result.Pass,
				Duration:     0,
				MayExonerate: false,
			},
			{
				Query:        query.Parse("_test_2"),
				Tags:         result.NewTags("tag_2"),
				Status:       result.Pass,
				Duration:     0,
				MayExonerate: false,
			},
		},
		"compat": result.List{
			{
				Query:        query.Parse("_test_3"),
				Tags:         result.NewTags("tag_3"),
				Status:       result.Pass,
				Duration:     0,
				MayExonerate: false,
			},
			{
				Query:        query.Parse("_test_4"),
				Tags:         result.NewTags("tag_4"),
				Status:       result.Pass,
				Duration:     0,
				MayExonerate: false,
			},
		},
	}
}

func TestCacheRecentUniqueSuppressedCoreResults_ErrorSurfaced(t *testing.T) {
	ctx := context.Background()
	cfg := getMultiPrefixConfig()
	wrapper := oswrapper.CreateMemMapOSWrapper()

	results := getMultiPrefixQueryResults()
	results["core_prefix"][0].TestId = "bad_test"
	client := resultsdb.MockBigQueryClient{
		RecentUniqueSuppressedReturnValues: results,
	}

	resultsList, err := CacheRecentUniqueSuppressedCoreResults(
		ctx, cfg, fileutils.ThisDir(), client, wrapper)
	require.Nil(t, resultsList)
	require.ErrorContains(t, err,
		"Test ID bad_test did not start with core_prefix even though query should have filtered.")
}

func TestCacheRecentUniqueSuppressedCoreResults_Success(t *testing.T) {
	ctx := context.Background()
	cfg := getMultiPrefixConfig()
	wrapper := oswrapper.CreateMemMapOSWrapper()

	client := resultsdb.MockBigQueryClient{
		RecentUniqueSuppressedReturnValues: getMultiPrefixQueryResults(),
	}

	resultsList, err := CacheRecentUniqueSuppressedCoreResults(
		ctx, cfg, fileutils.ThisDir(), client, wrapper)
	require.NoErrorf(t, err, "Error getting results: %v", err)
	require.Equal(t, resultsList, getExpectedMultiPrefixResults()["core"])
}

func TestCAcheRecentUniqueSuppressedCompatResults_ErrorSurfaced(t *testing.T) {
	ctx := context.Background()
	cfg := getMultiPrefixConfig()
	wrapper := oswrapper.CreateMemMapOSWrapper()

	results := getMultiPrefixQueryResults()
	results["compat_prefix"][0].TestId = "bad_test"
	client := resultsdb.MockBigQueryClient{
		RecentUniqueSuppressedReturnValues: results,
	}

	resultsList, err := CacheRecentUniqueSuppressedCoreResults(
		ctx, cfg, fileutils.ThisDir(), client, wrapper)
	require.Nil(t, resultsList)
	require.ErrorContains(t, err,
		"Test ID bad_test did not start with compat_prefix even though query should have filtered.")
}

func TestCacheRecentUniqueSuppressedCompatResults_Success(t *testing.T) {
	ctx := context.Background()
	cfg := getMultiPrefixConfig()
	wrapper := oswrapper.CreateMemMapOSWrapper()

	client := resultsdb.MockBigQueryClient{
		RecentUniqueSuppressedReturnValues: getMultiPrefixQueryResults(),
	}

	resultsList, err := CacheRecentUniqueSuppressedCompatResults(
		ctx, cfg, fileutils.ThisDir(), client, wrapper)
	require.NoErrorf(t, err, "Error getting results: %v", err)
	require.Equal(t, resultsList, getExpectedMultiPrefixResults()["compat"])
}

func TestCacheRecentUniqueSuppressedResults_CacheHit(t *testing.T) {
	ctx := context.Background()
	cfg := getMultiPrefixConfig()
	wrapper := oswrapper.CreateMemMapOSWrapper()

	// Technically this could run into a race condition if we run this test at the
	// exact time the day changes so the file is created on a different day than
	// it's read, but that seems exceedingly unlikely in practice.
	year, month, day := time.Now().Date()
	result.SaveWithWrapper(
		filepath.Join(
			fileutils.ThisDir(),
			"expectation-affected-ci-results",
			fmt.Sprintf("%d-%d-%d.txt", year, month, day)),
		getExpectedMultiPrefixResults(),
		wrapper)

	client := resultsdb.MockBigQueryClient{}

	resultsByExecutionMode, err := CacheRecentUniqueSuppressedResults(
		ctx, cfg, fileutils.ThisDir(), client, wrapper)
	require.NoErrorf(t, err, "Error getting results: %v", err)
	require.Equal(t, getExpectedMultiPrefixResults(), resultsByExecutionMode)
}

func TestCacheRecentUniqueSuppressedResults_CacheSkippedIfUnspecified(t *testing.T) {
	ctx := context.Background()
	cfg := getMultiPrefixConfig()
	wrapper := oswrapper.CreateMemMapOSWrapper()

	modifiedResults := getExpectedMultiPrefixResults()
	modifiedResults["core"] = append(modifiedResults["core"], result.Result{
		Query:        query.Parse("_test_5"),
		Tags:         result.NewTags("tag_5"),
		Status:       result.Pass,
		Duration:     0,
		MayExonerate: false,
	})

	year, month, day := time.Now().Date()
	result.SaveWithWrapper(
		filepath.Join(
			"expectation-affected-ci-results",
			fmt.Sprintf("%d-%d-%d.txt", year, month, day)),
		modifiedResults,
		wrapper)

	client := resultsdb.MockBigQueryClient{
		RecentUniqueSuppressedReturnValues: getMultiPrefixQueryResults(),
	}

	resultsByExecutionMode, err := CacheRecentUniqueSuppressedResults(
		ctx, cfg, "", client, wrapper)
	require.NoErrorf(t, err, "Error getting results: %v", err)
	require.Equal(t, getExpectedMultiPrefixResults(), resultsByExecutionMode)
}

func TestCacheRecentUniqueSuppressedResults_GetResultsError(t *testing.T) {
	ctx := context.Background()
	cfg := getMultiPrefixConfig()
	wrapper := oswrapper.CreateMemMapOSWrapper()

	modifiedQueryResults := getMultiPrefixQueryResults()
	modifiedQueryResults["core_prefix"][0].Tags[0].Key = "non_typ_tag"
	client := resultsdb.MockBigQueryClient{
		RecentUniqueSuppressedReturnValues: modifiedQueryResults,
	}

	resultsByExecutionMode, err := CacheRecentUniqueSuppressedResults(
		ctx, cfg, fileutils.ThisDir(), client, wrapper)
	require.Nil(t, resultsByExecutionMode)
	require.ErrorContains(t, err,
		"Got tag key non_typ_tag when only typ_tag should be present")
}

func TestCacheRecentUniqueSuppressedResults_Success(t *testing.T) {
	ctx := context.Background()
	cfg := getMultiPrefixConfig()
	wrapper := oswrapper.CreateMemMapOSWrapper()
	client := resultsdb.MockBigQueryClient{
		RecentUniqueSuppressedReturnValues: getMultiPrefixQueryResults(),
	}

	resultsByExecutionMode, err := CacheRecentUniqueSuppressedResults(
		ctx, cfg, fileutils.ThisDir(), client, wrapper)
	require.NoErrorf(t, err, "Error getting results: %v", err)
	require.Equal(t, getExpectedMultiPrefixResults(), resultsByExecutionMode)
}

/*******************************************************************************
 * getRecentUniqueSuppressedResults tests
 ******************************************************************************/

func TestGetRecentUniqueSuppressedResults_PrefixMismatch(t *testing.T) {
	ctx := context.Background()

	cfg := Config{
		Tests: []TestConfig{
			TestConfig{
				ExecutionMode: result.ExecutionMode("execution_mode"),
				Prefixes:      []string{"prefix"},
			},
		},
	}

	client := resultsdb.MockBigQueryClient{
		RecentUniqueSuppressedReturnValues: resultsdb.PrefixGroupedQueryResults{
			"prefix": []resultsdb.QueryResult{
				{
					TestId:   "bad_test",
					Status:   "FAIL",
					Tags:     []resultsdb.TagPair{},
					Duration: 1.0,
				},
			},
		},
	}

	resultsByExecutionMode, err := getRecentUniqueSuppressedResults(ctx, cfg, client)
	require.Nil(t, resultsByExecutionMode)
	require.ErrorContains(t, err,
		"Test ID bad_test did not start with prefix even though query should have filtered.")
}

func TestGetRecentUniqueSuppressedResults_NonTypTag(t *testing.T) {
	ctx := context.Background()

	cfg := Config{
		Tests: []TestConfig{
			TestConfig{
				ExecutionMode: result.ExecutionMode("execution_mode"),
				Prefixes:      []string{"prefix"},
			},
		},
	}

	client := resultsdb.MockBigQueryClient{
		RecentUniqueSuppressedReturnValues: resultsdb.PrefixGroupedQueryResults{
			"prefix": []resultsdb.QueryResult{
				{
					TestId: "prefix_test",
					Status: "FAIL",
					Tags: []resultsdb.TagPair{
						{
							Key:   "non_typ_tag",
							Value: "value",
						},
					},
					Duration: 1.0,
				},
			},
		},
	}

	resultsByExecutionMode, err := getRecentUniqueSuppressedResults(ctx, cfg, client)
	require.Nil(t, resultsByExecutionMode)
	require.ErrorContains(t, err,
		"Got tag key non_typ_tag when only typ_tag should be present")
}

func TestGetRecentUniqueSuppressedResults_Success(t *testing.T) {
	ctx := context.Background()

	cfg := Config{
		Tests: []TestConfig{
			TestConfig{
				ExecutionMode: result.ExecutionMode("execution_mode"),
				Prefixes:      []string{"prefix"},
			},
		},
	}

	client := resultsdb.MockBigQueryClient{
		RecentUniqueSuppressedReturnValues: resultsdb.PrefixGroupedQueryResults{
			"prefix": []resultsdb.QueryResult{
				{
					TestId: "prefix_test_1",
					Status: "FAIL",
					Tags: []resultsdb.TagPair{
						{
							Key:   "typ_tag",
							Value: "tag_1",
						},
					},
					Duration: 1.0,
				},
				{
					TestId: "prefix_test_2",
					Status: "FAIL",
					Tags: []resultsdb.TagPair{
						{
							Key:   "typ_tag",
							Value: "tag_1",
						},
					},
					Duration: 1.0,
				},
				{
					TestId: "prefix_test_1",
					Status: "FAIL",
					Tags: []resultsdb.TagPair{
						{
							Key:   "typ_tag",
							Value: "tag_2",
						},
					},
					Duration: 1.0,
				},
				{
					TestId: "prefix_test_2",
					Status: "FAIL",
					Tags: []resultsdb.TagPair{
						{
							Key:   "typ_tag",
							Value: "tag_2",
						},
					},
					Duration: 1.0,
				},
			},
		},
	}

	expectedResults := result.ResultsByExecutionMode{
		"execution_mode": result.List{
			{
				Query:        query.Parse("_test_1"),
				Tags:         result.NewTags("tag_1"),
				Status:       result.Pass,
				Duration:     0,
				MayExonerate: false,
			},
			{
				Query:        query.Parse("_test_1"),
				Tags:         result.NewTags("tag_2"),
				Status:       result.Pass,
				Duration:     0,
				MayExonerate: false,
			},
			{
				Query:        query.Parse("_test_2"),
				Tags:         result.NewTags("tag_1"),
				Status:       result.Pass,
				Duration:     0,
				MayExonerate: false,
			},
			{
				Query:        query.Parse("_test_2"),
				Tags:         result.NewTags("tag_2"),
				Status:       result.Pass,
				Duration:     0,
				MayExonerate: false,
			},
		},
	}

	resultsByExecutionMode, err := getRecentUniqueSuppressedResults(ctx, cfg, client)
	require.NoErrorf(t, err, "Got error getting results: %v", err)
	require.Equal(t, resultsByExecutionMode, expectedResults)
}
