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

package treemap

import (
	"bufio"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/auth"
	"dawn.googlesource.com/dawn/tools/src/browser"
	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/subcmd"
	luciauth "go.chromium.org/luci/auth"
	"go.chromium.org/luci/auth/client/authcli"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		source    common.ResultSource
		auth      authcli.Flags
		keepAlive bool
		testQuery string
	}
}

func (cmd) Name() string {
	return "treemap"
}

func (cmd) Desc() string {
	return `displays a treemap visualization of the CTS tests cases

mode:

  cases  - displays a treemap of the test cases, visualizing both the
           distribution of cases (spatially) and the number of parameterized
		   cases per test (color).
  timing - displays a treemap of the time taken by the CTS bots, visualizing
           both the total c of cases (spatially) and the number of parameterized
		   cases per test (color).`
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	c.flags.source.RegisterFlags(cfg)
	c.flags.auth.Register(flag.CommandLine, auth.DefaultAuthOptions(cfg.OsWrapper))
	flag.BoolVar(&c.flags.keepAlive, "keep-alive", false, "keep the server alive after the page has been closed")
	flag.StringVar(&c.flags.testQuery, "test-query", "webgpu:*", "cts test query to generate test list")
	return []string{"[cases | timing]"}, nil
}

// TODO(crbug.com/344014313): Split this up into testable helper functions and
// add coverage (browser.Open() likely prevents testing of Run() itself).
func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	ctx, stop := context.WithCancel(context.Background())
	defer stop()

	// Are we visualizing cases, or timings?
	var data string
	var err error
	switch flag.Arg(0) {
	case "case", "cases":
		data, err = loadCasesData(c.flags.testQuery, cfg.OsWrapper)

	case "time", "times", "timing":
		// Validate command line arguments
		auth, err := c.flags.auth.Options()
		if err != nil {
			return fmt.Errorf("failed to obtain authentication options: %w", err)
		}

		data, err = loadTimingData(ctx, c.flags.source, cfg, auth)

	default:
		err = subcmd.ErrInvalidCLA
	}
	if err != nil {
		return err
	}

	// Kick the server
	handler := http.NewServeMux()
	handler.HandleFunc("/index.html", func(w http.ResponseWriter, r *http.Request) {
		f, err := os.Open(filepath.Join(fileutils.ThisDir(), "treemap.html"))
		if err != nil {
			fmt.Fprint(w, "file not found")
			w.WriteHeader(http.StatusNotFound)
			return
		}
		defer f.Close()
		io.Copy(w, f)
	})
	handler.HandleFunc("/data.json", func(w http.ResponseWriter, r *http.Request) {
		io.Copy(w, strings.NewReader(data))
	})
	handler.HandleFunc("/viewer.closed", func(w http.ResponseWriter, r *http.Request) {
		if !c.flags.keepAlive {
			stop()
		}
	})

	const port = 9393
	url := fmt.Sprintf("http://localhost:%v/index.html", port)

	server := &http.Server{Addr: fmt.Sprint(":", port), Handler: handler}
	go server.ListenAndServe()

	browser.Open(url)

	<-ctx.Done()
	err = server.Shutdown(ctx)
	switch err {
	case nil, context.Canceled:
		return nil
	default:
		return err
	}
}

// TODO(crbug.com/344014313): Add unittests for this function.
// loadCasesData creates the JSON payload for a cases visualization
func loadCasesData(testQuery string, fsReader oswrapper.FilesystemReader) (string, error) {
	testlist, err := common.GenTestList(context.Background(), common.DefaultCTSPath(fsReader), fileutils.NodePath(fsReader), testQuery)
	if err != nil {
		return "", err
	}
	queryCounts := container.NewMap[string, int]()

	scanner := bufio.NewScanner(strings.NewReader(testlist))
	for scanner.Scan() {
		if name := strings.TrimSpace(scanner.Text()); name != "" {
			q := query.Parse(name)
			q.Cases = "" // Remove parameterization
			for name = q.String(); name != ""; name = parentOf(name) {
				queryCounts[name] = queryCounts[name] + 1
			}
		}
	}

	if err := scanner.Err(); err != nil {
		return "", fmt.Errorf("failed to parse test list: %w", err)
	}

	// https://developers.google.com/chart/interactive/docs/gallery/treemap#data-format
	data := &strings.Builder{}
	fmt.Fprint(data, `{`)
	fmt.Fprint(data, `"desc":"Treemap visualization of the CTS test cases.<br>Area represents total number of test cases.<br>Color represents the number of parameterized test cases for a single test.",`)
	fmt.Fprintf(data, `"limit": 1000,`)
	fmt.Fprint(data, `"data":[`)
	fmt.Fprint(data, `["Query", "Parent", "Number of tests", "Color"],`)
	fmt.Fprint(data, `["root", null, 0, 0]`)
	for _, query := range queryCounts.Keys() {
		fmt.Fprint(data, ",")
		count := queryCounts[query]
		if err := json.NewEncoder(data).Encode([]any{query, parentOfOrRoot(query), count, count}); err != nil {
			return "", err
		}
	}
	fmt.Fprintln(data, `]}`)

	return data.String(), nil
}

type durations struct {
	sum   time.Duration
	count int
}

func (d durations) add(n time.Duration) durations {
	return durations{d.sum + n, d.count + 1}
}

func (d durations) average() time.Duration {
	if d.count == 0 {
		return 0
	}
	return time.Duration(float64(d.sum) / float64(d.count))
}

// loadTimingData creates the JSON payload for timing visualization
func loadTimingData(ctx context.Context, source common.ResultSource, cfg common.Config, auth luciauth.Options) (string, error) {
	// Obtain the results
	results, err := source.GetResults(ctx, cfg, auth)
	if err != nil {
		return "", err
	}

	queryTimes := container.NewMap[string, durations]()

	for _, mode := range results {
		for _, result := range mode {
			q := result.Query
			q.Cases = "" // Remove parameterization
			for name := q.String(); name != ""; name = parentOf(name) {
				queryTimes[name] = queryTimes[name].add(result.Duration)
			}
		}
	}

	// https://developers.google.com/chart/interactive/docs/gallery/treemap#data-format
	data := &strings.Builder{}
	fmt.Fprint(data, `{`)
	fmt.Fprint(data, `"desc":"Treemap visualization of the CTS timings.<br>Area represents total time taken by the test cases.<br>Color represents average time take by the non-parameterized test case.",`)
	fmt.Fprint(data, `"limit": 1000,`)
	fmt.Fprint(data, `"data":[`)
	fmt.Fprint(data, `["Query", "Parent", "Time (ms)", "Color"],`)
	fmt.Fprint(data, `["root", null, 0, 0]`)
	for _, query := range queryTimes.Keys() {
		fmt.Fprint(data, ",")
		d := queryTimes[query].average()
		if err := json.NewEncoder(data).Encode([]any{query, parentOfOrRoot(query), d.Milliseconds(), d.Milliseconds()}); err != nil {
			return "", err
		}
	}
	fmt.Fprintln(data, `]}`)

	return data.String(), nil
}

func parentOf(query string) string {
	if n := strings.LastIndexAny(query, ",:"); n > 0 {
		return query[:n]
	}
	return ""
}

func parentOfOrRoot(query string) string {
	if parent := parentOf(query); parent != "" {
		return parent
	}
	return "root"
}
