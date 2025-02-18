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

// Package resultsdb provides helpers for interfacing with resultsdb
package resultsdb

import (
	"context"
	"fmt"
	"strings"

	"cloud.google.com/go/bigquery"
	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"google.golang.org/api/iterator"
)

type RowHandler = func(*QueryResult) error
type QueryFunc = func(context.Context, []buildbucket.BuildID, string, RowHandler) error

type Querier interface {
	QueryTestResults(ctx context.Context, builds []buildbucket.BuildID, testPrefix string, f RowHandler) error
	QueryUnsuppressedFailingTestResults(ctx context.Context, builds []buildbucket.BuildID, testPrefix string, f RowHandler) error
	QueryRecentUniqueSuppressedTestResults(ctx context.Context, testPrefix string, f RowHandler) error
}

// BigQueryClient is a wrapper around bigquery.Client so that we can define new
// methods.
type BigQueryClient struct {
	client *bigquery.Client
}

// QueryResult contains all of the data for a single test result from a ResultDB
// BigQuery query.
type QueryResult struct {
	TestId   string
	Status   string
	Tags     []TagPair
	Duration float64
}

// TagPair is a key/value pair representing a ResultDB tag.
type TagPair struct {
	Key   string
	Value string
}

// DefaultQueryProject is the default BigQuery project to use when running
// queries.
const DefaultQueryProject string = "chrome-unexpected-pass-data"

// NewBigQueryClient creates a client for running BigQuery queries. The
// intention is for this to be used for querying ResultDB tables, but there is
// nothing ResultDB-specific about the resulting client.
func NewBigQueryClient(ctx context.Context, project string) (*BigQueryClient, error) {
	client, err := bigquery.NewClient(ctx, project)
	if err != nil {
		return nil, err
	}
	// By default, results are retrieved in chunks as they're iterated over, but
	// that results in slow performance. Enabling the Storage API allows us to get
	// all results at once, resulting in a ~8-10x speed increase.
	err = client.EnableStorageReadClient(ctx)
	if err != nil {
		return nil, err
	}
	return &BigQueryClient{client}, nil
}

// CheckIfResultDBCanBeQueried checks whether the BigQueryClient can successfully
// run a query in public ResultDB data. Failure to do so typically indicates a
// permission issue.
func (bq BigQueryClient) CheckIfResultDBCanBeQueried(ctx context.Context) error {
	query := `
    SELECT
      *
    FROM ` + "`chrome-luci-data.chromium.gpu_try_test_results`" + ` tr
    LIMIT 0`

	return bq.runQuery(ctx, query, func(row *QueryResult) error { return nil })
}

// QueryTestResults fetches the test results for the given builds using
// BigQuery.
//
// f is called once per result and is expected to handle any processing or
// storage of results.
func (bq BigQueryClient) QueryTestResults(
	ctx context.Context, builds []buildbucket.BuildID, testPrefix string, f RowHandler) error {
	// test_id gets renamed since the column names need to match the struct names
	// unless we want to get results in a generic bigquery.Value slice and
	// manually copy data over.
	baseQuery := `
		SELECT
			test_id AS testid,
			status,
			tags,
			duration
		FROM ` + "`chrome-luci-data.chromium.gpu_try_test_results`" + ` tr
		WHERE
			exported.id IN UNNEST([%s])
			AND STARTS_WITH(tr.test_id, "%v")`

	return bq.runQueryForBuilds(ctx, baseQuery, builds, testPrefix, f)
}

// QueryUnsuppressedFailingTestResults fetches the test results for the given
// builds using BigQuery that both:
//  1. Failed in some way
//  2. Did not have an existing failure suppression in place
//
// f is called once per result and is expected to handle any processing or
// sorage of results.
func (bq BigQueryClient) QueryUnsuppressedFailingTestResults(
	ctx context.Context, builds []buildbucket.BuildID, testPrefix string, f RowHandler) error {
	// We use a subquery since we need to calculate the typ expectations for
	// filtering, but don't actually need them in the end results.
	// test_id gets renamed since the column names need to match the struct names
	// unless we want to get results as a generic bigquery.Value slice and
	// manually copy data over.
	baseQuery := `
		WITH
			failing_results AS (
				SELECT
					test_id AS testid,
					status,
					tags,
					duration,
					ARRAY(
						SELECT value
						FROM tr.tags
						WHERE key = "raw_typ_expectation") as typ_expectations
				FROM ` + "`chrome-luci-data.chromium.gpu_try_test_results`" + ` tr
				WHERE
					exported.id IN UNNEST([%s])
					AND STARTS_WITH(tr.test_id, "%v")
					AND status != "SKIP"
					AND status != "PASS"
			)
		SELECT
			*
		EXCEPT
			(typ_expectations)
		FROM
			failing_results
		WHERE
			ARRAY_LENGTH(typ_expectations) = 1
			AND typ_expectations[0] = "Pass"`

	return bq.runQueryForBuilds(ctx, baseQuery, builds, testPrefix, f)
}

// QueryRecentUniqueSuppressedTestResults fetches the test results for the given
// 'testPrefix' that:
//  1. Were produced within the last 6 hours
//  2. Had some sort of test suppression in place, regardless of whether the
//     test passed or not.
//
// Results are grouped by unique test ID and typ tags, with other information
// removed.
//
// f is called once per result and is expected to handle any processing or
// storage of results.
func (bq BigQueryClient) QueryRecentUniqueSuppressedTestResults(
	ctx context.Context,
	testPrefix string,
	f RowHandler) error {

	baseQuery := `
		WITH
			recent_results AS (
				SELECT
					test_id AS testid,
          ARRAY(
            SELECT t
            FROM tr.tags t
            WHERE key = "typ_tag"
          ) as tags,
					ARRAY(
						SELECT value
						FROM tr.tags
						WHERE key = "raw_typ_expectation") as typ_expectations
				FROM ` + "`chrome-luci-data.chromium.gpu_ci_test_results`" + ` tr
				WHERE
					TIME(partition_time) > TIME_SUB(CURRENT_TIME(), INTERVAL 6 HOUR)
					AND STARTS_WITH(tr.test_id, "%v")
			)
		SELECT
		  *
		EXCEPT
		  (typ_expectations)
		FROM
		  recent_results
		WHERE
		  (
		  	ARRAY_LENGTH(typ_expectations) = 1
		  	AND typ_expectations[0] != "Pass"
		  )
		  OR
		  (
		  	ARRAY_LENGTH(typ_expectations) > 1
		  )
    GROUP BY testid, tags
`

	query := fmt.Sprintf(baseQuery, testPrefix)

	return bq.runQuery(ctx, query, f)
}

// runQueryForBuilds is a helper function for running queries limited to a set
// of builds and prefix. See callers of this function for additional information.
func (bq BigQueryClient) runQueryForBuilds(
	ctx context.Context, baseQuery string, builds []buildbucket.BuildID, testPrefix string, f RowHandler) error {
	var buildIds []string
	for _, id := range builds {
		buildIds = append(buildIds, fmt.Sprintf(`"build-%v"`, id))
	}
	query := fmt.Sprintf(baseQuery, strings.Join(buildIds, ","), testPrefix)

	return bq.runQuery(ctx, query, f)
}

// runQuery is a helper function to run the provided 'query' and call 'f' for
// each resulting row.
func (bq BigQueryClient) runQuery(ctx context.Context, query string, f RowHandler) error {
	q := bq.client.Query(query)
	iter, err := q.Read(ctx)
	if err != nil {
		return err
	}

	var row QueryResult
	for {
		err := iter.Next(&row)
		if err == iterator.Done {
			break
		}
		if err != nil {
			return err
		}

		if err := f(&row); err != nil {
			return err
		}
	}

	return nil
}
