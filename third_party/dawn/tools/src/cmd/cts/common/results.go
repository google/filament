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

package common

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"log"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/resultsdb"
	"dawn.googlesource.com/dawn/tools/src/subcmd"
	"go.chromium.org/luci/auth"
	rdbpb "go.chromium.org/luci/resultdb/proto/v1"
)

// These function signatures are to facilitate code reuse between the code paths
// for getting all results and for only getting unsuppressed failures.
// For MostRecentResultsForChange/MostRecentUnsuppressedFailingResultsForChange.
type mostRecentResultsFunc = func(context.Context, Config, string, *gerrit.Gerrit, *buildbucket.Buildbucket, resultsdb.Querier, int) (result.ResultsByExecutionMode, gerrit.Patchset, error)

// For CacheResults/CacheUnsuppressedFailingResults.
type cacheResultsFunc = func(context.Context, Config, gerrit.Patchset, string, resultsdb.Querier, BuildsByName) (result.ResultsByExecutionMode, error)

// For GetRawResults/GetRawUnsuppressedFailingResults.
type getRawResultsFunc = func(context.Context, Config, resultsdb.Querier, BuildsByName) (result.ResultsByExecutionMode, error)

// ResultSource describes the source of CTS test results.
// ResultSource is commonly used by command line flags for specifying from
// where the results should be loaded / fetched.
// If neither File or Patchset are specified, then results will be fetched from
// the last successful CTS roll.
type ResultSource struct {
	// The directory used to cache results fetched from ResultDB
	CacheDir string
	// If specified, results will be loaded from this file path
	// Must not be specified if Patchset is also specified.
	File string
	// If specified, results will be fetched from this gerrit patchset
	// Must not be specified if File is also specified.
	Patchset gerrit.Patchset
}

// RegisterFlags registers the ResultSource fields as commandline flags for use
// by command line tools.
func (r *ResultSource) RegisterFlags(cfg Config) {
	flag.StringVar(&r.CacheDir, "cache", DefaultCacheDir, "path to the results cache")
	flag.StringVar(&r.File, "results", "", "local results.txt file (mutually exclusive with --cl)")
	r.Patchset.RegisterFlags(cfg.Gerrit.Host, cfg.Gerrit.Project)
}

// GetResults loads or fetches the results, based on the values of r.
// GetResults will update the ResultSource with the inferred patchset, if a file
// and specific patchset was not specified.
func (r *ResultSource) GetResults(ctx context.Context, cfg Config, auth auth.Options) (result.ResultsByExecutionMode, error) {
	return r.getResultsImpl(ctx, cfg, auth, MostRecentResultsForChange, CacheResults)
}

// GetUnsuppressedFailingResults loads or fetches the unsuppressed failing
// results, based on the values of r.
// GetUnsuppressedFailingResults will update the ResultSource with the inferred
// patchset if a file and specific patchset was not specified.
func (r *ResultSource) GetUnsuppressedFailingResults(ctx context.Context, cfg Config, auth auth.Options) (result.ResultsByExecutionMode, error) {
	return r.getResultsImpl(ctx, cfg, auth, MostRecentUnsuppressedFailingResultsForChange, CacheUnsuppressedFailingResults)
}

// Helper function to share the implementation between GetResults and GetUnsuppressedFailingResults.
func (r *ResultSource) getResultsImpl(ctx context.Context, cfg Config, auth auth.Options,
	getRecentResults mostRecentResultsFunc, cacheResults cacheResultsFunc) (result.ResultsByExecutionMode, error) {
	// Check that File and Patchset weren't both specified
	ps := &r.Patchset
	if r.File != "" && ps.Change != 0 {
		fmt.Fprintln(flag.CommandLine.Output(), "only one of --results and --cl can be specified")
		return nil, subcmd.ErrInvalidCLA
	}

	// If a file was specified, then load that.
	if r.File != "" {
		return result.Load(r.File)
	}

	// Initialize the buildbucket and resultdb clients
	bb, err := buildbucket.New(ctx, auth)
	if err != nil {
		return nil, err
	}
	client, err := resultsdb.NewBigQueryClient(ctx, resultsdb.DefaultQueryProject)
	if err != nil {
		return nil, err
	}

	// If no change was specified, then pull the results from the most recent
	// CTS roll.
	if ps.Change == 0 {
		fmt.Println("no change specified, scanning gerrit for last CTS roll...")
		gerrit, err := gerrit.New(ctx, auth, cfg.Gerrit.Host)
		if err != nil {
			return nil, err
		}
		latest, err := LatestCTSRoll(gerrit)
		if err != nil {
			return nil, err
		}
		fmt.Printf("scanning for latest patchset of %v...\n", latest.Number)
		var resultsByExecutionMode result.ResultsByExecutionMode
		resultsByExecutionMode, *ps, err = getRecentResults(ctx, cfg, r.CacheDir, gerrit, bb, client, latest.Number)
		if err != nil {
			return nil, err
		}
		fmt.Printf("using results from cl %v ps %v...\n", ps.Change, ps.Patchset)
		return resultsByExecutionMode, nil
	}

	// If a change, but no patchset was specified, then query the most recent
	// patchset.
	if ps.Patchset == 0 {
		gerrit, err := gerrit.New(ctx, auth, cfg.Gerrit.Host)
		if err != nil {
			return nil, err
		}
		*ps, err = gerrit.LatestPatchset(strconv.Itoa(ps.Change))
		if err != nil {
			err := fmt.Errorf("failed to find latest patchset of change %v: %w",
				ps.Change, err)
			return nil, err
		}
	}

	// Obtain the patchset's results, kicking a build if there are no results
	// already available.
	builds, err := GetOrStartBuildsAndWait(ctx, cfg, *ps, bb, "", false)
	if err != nil {
		return nil, err
	}

	resultsByExecutionMode, err := cacheResults(ctx, cfg, *ps, r.CacheDir, client, builds)
	if err != nil {
		return nil, err
	}

	return resultsByExecutionMode, nil
}

// TODO(crbug.com/344014313): Switch to using the resultsdb.Querier contained
// within the provided Config instead of taking a separate argument.

// CacheResults looks in the cache at 'cacheDir' for the results for the given patchset.
// If the cache contains the results, then these are loaded, transformed with CleanResults() and
// returned.
// If the cache does not contain the results, then they are fetched using GetRawResults(), saved to
// the cache directory, transformed with CleanResults() and then returned.
func CacheResults(
	ctx context.Context,
	cfg Config,
	ps gerrit.Patchset,
	cacheDir string,
	client resultsdb.Querier,
	builds BuildsByName) (result.ResultsByExecutionMode, error) {

	return cacheResultsImpl(ctx, cfg, ps, cacheDir, "", client, builds, GetRawResults)
}

// CacheUnsuppressedFailingResults looks in the cache at 'cacheDir' for the
// unsuppressed failing results for the given patchset. If the cache contains
// the results, then these are loaded, transformed with CleanResults(), and
// returned.
// If the cache does not contain the results, then they are fetched using
// GetRawUnsuppressedFailingResults(), saved to the cache directory, transformed
// with CleanResults(), and then returned.
func CacheUnsuppressedFailingResults(
	ctx context.Context,
	cfg Config,
	ps gerrit.Patchset,
	cacheDir string,
	client resultsdb.Querier,
	builds BuildsByName) (result.ResultsByExecutionMode, error) {

	return cacheResultsImpl(ctx, cfg, ps, cacheDir, "-unsuppressed-failures", client, builds, GetRawUnsuppressedFailingResults)
}

// Helper function to share the implementation between CacheResults and CacheUnsuppressedFailingResults.
func cacheResultsImpl(
	ctx context.Context,
	cfg Config,
	ps gerrit.Patchset,
	cacheDir string,
	fileSuffix string,
	client resultsdb.Querier,
	builds BuildsByName,
	getRawResults getRawResultsFunc) (result.ResultsByExecutionMode, error) {

	var cachePath string
	if cacheDir != "" {
		dir := fileutils.ExpandHome(cacheDir, cfg.OsWrapper)
		path := filepath.Join(dir, strconv.Itoa(ps.Change), fmt.Sprintf("ps-%v%v.txt", ps.Patchset, fileSuffix))
		if _, err := cfg.OsWrapper.Stat(path); err == nil {
			log.Printf("loading cached results from cl %v ps %v...", ps.Change, ps.Patchset)
			return result.LoadWithWrapper(path, cfg.OsWrapper)
		}
		cachePath = path
	}

	log.Printf("fetching results from cl %v ps %v...", ps.Change, ps.Patchset)
	resultsByExecutionMode, err := getRawResults(ctx, cfg, client, builds)
	if err != nil {
		return nil, err
	}

	// Expand aliased tags, remove specific tags
	for i, results := range resultsByExecutionMode {
		CleanResults(cfg, &results)
		results.Sort()
		resultsByExecutionMode[i] = results
	}

	if err := result.SaveWithWrapper(cachePath, resultsByExecutionMode, cfg.OsWrapper); err != nil {
		log.Println("failed to save results to cache: %w", err)
	}

	return resultsByExecutionMode, nil
}

// GetResults calls GetRawResults() to fetch the build results from ResultDB and applies CleanResults().
// GetResults does not trigger new builds.
func GetResults(
	ctx context.Context,
	cfg Config,
	client resultsdb.Querier,
	builds BuildsByName) (result.ResultsByExecutionMode, error) {

	return getResultsImpl(ctx, cfg, client, builds, GetRawResults)
}

// GetUnsuppressedFailingResults calls GetRawUnsuppressedFailingResults() to
// fetch the unsuppressed failing results from ResultDB and applies
// CleanResults(). GetUnsuppressedFailingResults() does not trigger new builds.
func GetUnsuppressedFailingResults(
	ctx context.Context,
	cfg Config,
	client resultsdb.Querier,
	builds BuildsByName) (result.ResultsByExecutionMode, error) {

	return getResultsImpl(ctx, cfg, client, builds, GetRawUnsuppressedFailingResults)
}

// Helper function to share the implementation between GetResults and GetUnsuppressedFailingResults.
func getResultsImpl(
	ctx context.Context,
	cfg Config,
	client resultsdb.Querier,
	builds BuildsByName,
	getRawResults getRawResultsFunc) (result.ResultsByExecutionMode, error) {

	resultsByExecutionMode, err := getRawResults(ctx, cfg, client, builds)
	if err != nil {
		return nil, err
	}

	// Expand aliased tags, remove specific tags
	for i, results := range resultsByExecutionMode {
		CleanResults(cfg, &results)
		results.Sort()
		resultsByExecutionMode[i] = results
	}

	return resultsByExecutionMode, err
}

// GetRawResults fetches the build results from ResultDB, without applying CleanResults().
// GetRawResults does not trigger new builds.
func GetRawResults(
	ctx context.Context,
	cfg Config,
	client resultsdb.Querier,
	builds BuildsByName) (result.ResultsByExecutionMode, error) {

	return getRawResultsImpl(ctx, cfg, client, builds, client.QueryTestResults)
}

// GetRawUnsuppressedFailingResults fetches the unsuppressed failing results
// from ResultDB, without applying CleanResults().
// GetRawUnsuppressedFailingResults does not trigger new builds.
func GetRawUnsuppressedFailingResults(
	ctx context.Context,
	cfg Config,
	client resultsdb.Querier,
	builds BuildsByName) (result.ResultsByExecutionMode, error) {

	return getRawResultsImpl(ctx, cfg, client, builds, client.QueryUnsuppressedFailingTestResults)
}

// Helper function to share the implementation between GetRawResults and
// GetRawUnsuppressedFailingResults.
func getRawResultsImpl(
	ctx context.Context,
	cfg Config,
	client resultsdb.Querier,
	builds BuildsByName,
	queryFunc resultsdb.QueryFunc) (result.ResultsByExecutionMode, error) {

	fmt.Printf("fetching results from resultdb...")

	lastPrintedDot := time.Now()

	resultsByExecutionMode := result.ResultsByExecutionMode{}
	for _, test := range cfg.Tests {
		results := result.List{}
		for _, prefix := range test.Prefixes {

			err := queryFunc(ctx, builds.ids(), prefix, func(r *resultsdb.QueryResult) error {
				if time.Since(lastPrintedDot) > 5*time.Second {
					lastPrintedDot = time.Now()
					fmt.Printf(".")
				}

				if !strings.HasPrefix(r.TestId, prefix) {
					return errors.New(fmt.Sprintf("Test ID %s did not start with %s even though query should have filtered.", r.TestId, prefix))
				}

				testName := r.TestId[len(prefix):]
				status := convertRdbStatus(r.Status)
				tags := result.NewTags()
				duration := time.Duration(r.Duration * float64(time.Second))
				mayExonerate := false

				for _, tagPair := range r.Tags {
					if tagPair.Key == "typ_tag" {
						tags.Add(tagPair.Value)
					} else if tagPair.Key == "javascript_duration" {
						var err error
						if duration, err = time.ParseDuration(tagPair.Value); err != nil {
							return err
						}
					} else if tagPair.Key == "may_exonerate" {
						var err error
						if mayExonerate, err = strconv.ParseBool(tagPair.Value); err != nil {
							return err
						}
					}
				}

				results = append(results, result.Result{
					Query:        query.Parse(testName),
					Status:       status,
					Tags:         tags,
					Duration:     duration,
					MayExonerate: mayExonerate,
				})

				return nil
			})

			if err != nil {
				return nil, err
			}

			results.Sort()
		}
		resultsByExecutionMode[test.ExecutionMode] = results
	}

	fmt.Println(" done")

	return resultsByExecutionMode, nil
}

func convertRdbStatus(rdbStatus string) result.Status {
	switch rdbStatus {
	default:
		return result.Unknown
	case rdbpb.TestStatus_PASS.String():
		return result.Pass
	case rdbpb.TestStatus_FAIL.String():
		return result.Failure
	case rdbpb.TestStatus_CRASH.String():
		return result.Crash
	case rdbpb.TestStatus_ABORT.String():
		return result.Abort
	case rdbpb.TestStatus_SKIP.String():
		return result.Skip
	}
}

// LatestCTSRoll returns for the latest merged CTS roll that landed in the past
// month. If no roll can be found, then an error is returned.
func LatestCTSRoll(g *gerrit.Gerrit) (gerrit.ChangeInfo, error) {
	changes, _, err := g.QueryChanges(
		`status:merged`,
		`-age:1month`,
		fmt.Sprintf(`message:"%v"`, RollSubjectPrefix))
	if err != nil {
		return gerrit.ChangeInfo{}, err
	}
	if len(changes) == 0 {
		return gerrit.ChangeInfo{}, fmt.Errorf("no change found")
	}
	sort.Slice(changes, func(i, j int) bool {
		return changes[i].Submitted.Time.After(changes[j].Submitted.Time)
	})
	return changes[0], nil
}

// LatestPatchset returns the most recent patchset for the given change.
func LatestPatchset(g *gerrit.Gerrit, change int) (gerrit.Patchset, error) {
	ps, err := g.LatestPatchset(strconv.Itoa(change))
	if err != nil {
		err := fmt.Errorf("failed to find latest patchset of change %v: %w",
			ps.Change, err)
		return gerrit.Patchset{}, err
	}
	return ps, nil
}

// MostRecentResultsForChange returns the results from the most recent patchset
// that has build results. If no results can be found for the entire change,
// then an error is returned.
func MostRecentResultsForChange(
	ctx context.Context,
	cfg Config,
	cacheDir string,
	g *gerrit.Gerrit,
	bb *buildbucket.Buildbucket,
	client resultsdb.Querier,
	change int) (result.ResultsByExecutionMode, gerrit.Patchset, error) {

	return mostRecentResultsForChangeImpl(ctx, cfg, cacheDir, g, bb, client, change, CacheResults)
}

// MostRecentUnsuppressedFailingResultsForChange returns the unsuppressed
// failing results from the most recent patchset that has build results. If no
// results can be found for the entire change, then an error is returned.
func MostRecentUnsuppressedFailingResultsForChange(
	ctx context.Context,
	cfg Config,
	cacheDir string,
	g *gerrit.Gerrit,
	bb *buildbucket.Buildbucket,
	client resultsdb.Querier,
	change int) (result.ResultsByExecutionMode, gerrit.Patchset, error) {

	return mostRecentResultsForChangeImpl(ctx, cfg, cacheDir, g, bb, client, change, CacheUnsuppressedFailingResults)
}

// Helper function to share the implementation between MostRecentResultsForChange
// and MostRecentUnsuppressedFailingResultsForChange.
func mostRecentResultsForChangeImpl(
	ctx context.Context,
	cfg Config,
	cacheDir string,
	g *gerrit.Gerrit,
	bb *buildbucket.Buildbucket,
	client resultsdb.Querier,
	change int,
	cacheResults cacheResultsFunc) (result.ResultsByExecutionMode, gerrit.Patchset, error) {

	ps, err := LatestPatchset(g, change)
	if err != nil {
		return nil, gerrit.Patchset{}, nil
	}

	for ps.Patchset > 0 {
		builds, err := GetBuilds(ctx, cfg, ps, bb)
		if err != nil {
			return nil, gerrit.Patchset{}, err
		}
		if len(builds) > 0 {
			if err := WaitForBuildsToComplete(ctx, cfg, ps, bb, builds); err != nil {
				return nil, gerrit.Patchset{}, err
			}

			results, err := cacheResults(ctx, cfg, ps, cacheDir, client, builds)
			if err != nil {
				return nil, gerrit.Patchset{}, err
			}

			if len(results) > 0 {
				return results, ps, nil
			}
		}
		ps.Patchset--
	}

	return nil, gerrit.Patchset{}, fmt.Errorf("no builds found for change %v", change)
}

// CleanResults modifies each result so that tags in cfg.Tag.Remove are removed and
// duplicate results are removed by erring towards Failure.
// See: crbug.com/dawn/1387, crbug.com/dawn/1401
func CleanResults(cfg Config, results *result.List) {
	// Remove any tags found in cfg.Tag.Remove
	remove := result.NewTags(cfg.Tag.Remove...)
	for _, r := range *results {
		r.Tags.RemoveAll(remove)
	}
	// Clean up duplicate results
	*results = results.ReplaceDuplicates(func(s result.Statuses) result.Status {
		// If all results have the same status, then use that.
		if len(s) == 1 {
			return s.One()
		}
		// Mixed statuses. Replace with something appropriate.
		switch {
		case s.Contains(result.Crash):
			return result.Crash
		case s.Contains(result.Abort):
			return result.Abort
		case s.Contains(result.Failure):
			return result.Failure
		case s.Contains(result.Slow):
			return result.Slow
		}
		return result.Failure
	})
}

// CacheRecentUniqueSuppressedCoreResults is a helper function to only get the
// core results from CacheRecentUniqueSuppressedResults. This allows other
// unused results to be garbage collected.
func CacheRecentUniqueSuppressedCoreResults(
	ctx context.Context,
	cfg Config,
	cacheDir string,
	client resultsdb.Querier,
	osWrapper oswrapper.OSWrapper) (result.List, error) {

	resultsByExecutionMode, err := CacheRecentUniqueSuppressedResults(
		ctx, cfg, cacheDir, client, osWrapper)
	if err != nil {
		return nil, err
	}

	return resultsByExecutionMode["core"], nil
}

// CacheRecentUniqueSuppressedCompatResults is a helper function to only get the
// compat results from CacheRecentUniqueSuppressedResults. This allows other
// unused results to be garbage collected.
func CacheRecentUniqueSuppressedCompatResults(
	ctx context.Context,
	cfg Config,
	cacheDir string,
	client resultsdb.Querier,
	osWrapper oswrapper.OSWrapper) (result.List, error) {

	resultsByExecutionMode, err := CacheRecentUniqueSuppressedResults(
		ctx, cfg, cacheDir, client, osWrapper)
	if err != nil {
		return nil, err
	}

	return resultsByExecutionMode["compat"], nil
}

// CacheRecentRexpectationAffectedCiResults looks in the cache at 'cacheDir' for
// CI results from the recent history. If the cache contains the results, they
// are loaded and returned. If the cache does not contain the results, they are
// fetched, cleaned with CleanResults(), saved to the cache directory, and
// returned.
// TODO(crbug.com/390105593): Consider sharing code with the other cache
// functions.
func CacheRecentUniqueSuppressedResults(
	ctx context.Context,
	cfg Config,
	cacheDir string,
	client resultsdb.Querier,
	osWrapper oswrapper.OSWrapper) (result.ResultsByExecutionMode, error) {

	// Load cached results if they are available.
	var cachePath string
	if cacheDir != "" {
		dir := fileutils.ExpandHome(cacheDir, osWrapper)
		year, month, day := time.Now().Date()
		path := filepath.Join(dir, "expectation-affected-ci-results", fmt.Sprintf("%d-%d-%d.txt", year, month, day))
		if _, err := osWrapper.Stat(path); err == nil {
			log.Println("loading cached results for today")
			return result.LoadWithWrapper(path, osWrapper)
		}
		cachePath = path
	}

	// Retrieve, clean, and cache results.
	log.Println("fetching results from recent CI builds")
	resultsByExecutionMode, err := getRecentUniqueSuppressedResults(ctx, cfg, client)
	if err != nil {
		return nil, err
	}

	for i, results := range resultsByExecutionMode {
		CleanResults(cfg, &results)
		results.Sort()
		resultsByExecutionMode[i] = results
	}

	if err := result.SaveWithWrapper(cachePath, resultsByExecutionMode, osWrapper); err != nil {
		log.Printf("failed to save results to cache: %v", err)
	}

	return resultsByExecutionMode, nil
}

// getRecentUniqueSuppressedResults fetches recent results from CI via ResultDB
// which were affected by an expectation, without applying CleanResults(). The
// returned results should be unique for each combination of test name and typ
// tags, with other fields not containing real data.
// TODO(crbug.com/390105593): Consider sharing code with the other
// GetRawResults functions.
func getRecentUniqueSuppressedResults(
	ctx context.Context,
	cfg Config,
	client resultsdb.Querier) (result.ResultsByExecutionMode, error) {

	log.Println("fetching results from resultdb...")

	resultsByExecutionMode := result.ResultsByExecutionMode{}
	for _, test := range cfg.Tests {
		results := result.List{}
		for _, prefix := range test.Prefixes {

			rowHandler := func(r *resultsdb.QueryResult) error {
				if !strings.HasPrefix(r.TestId, prefix) {
					return fmt.Errorf(
						"Test ID %s did not start with %s even though query should have filtered.",
						r.TestId, prefix)
				}

				testName := r.TestId[len(prefix):]
				tags := result.NewTags()

				for _, tagPair := range r.Tags {
					if tagPair.Key != "typ_tag" {
						return fmt.Errorf("Got tag key %v when only typ_tag should be present", tagPair.Key)
					}
					tags.Add(tagPair.Value)
				}

				results = append(results, result.Result{
					// We don't actually care about anything other than the query and
					// tags.
					Query:        query.Parse(testName),
					Status:       result.Pass,
					Tags:         tags,
					Duration:     0,
					MayExonerate: false,
				})

				return nil
			}

			err := client.QueryRecentUniqueSuppressedTestResults(ctx, prefix, rowHandler)
			if err != nil {
				return nil, err
			}

			results.Sort()
		}
		resultsByExecutionMode[test.ExecutionMode] = results
	}

	fmt.Println(" done")

	return resultsByExecutionMode, nil
}
