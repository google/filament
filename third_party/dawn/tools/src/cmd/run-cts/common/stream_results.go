// Copyright 2023 The Dawn & Tint Authors
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
	"bytes"
	"context"
	"fmt"
	"io"
	"os"
	"strings"
	"sync"
	"time"
	"unicode/utf8"

	"dawn.googlesource.com/dawn/tools/src/browser"
	"dawn.googlesource.com/dawn/tools/src/cov"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/git"
	"dawn.googlesource.com/dawn/tools/src/progressbar"
	"dawn.googlesource.com/dawn/tools/src/term"
)

var statusColor = map[Status]string{
	Pass:    term.Green,
	Warn:    term.Yellow,
	Skip:    term.Cyan,
	Timeout: term.Yellow,
	Fail:    term.Red,
}

var pbStatusColor = map[Status]progressbar.Color{
	Pass:    progressbar.Green,
	Warn:    progressbar.Yellow,
	Skip:    progressbar.Cyan,
	Timeout: progressbar.Yellow,
	Fail:    progressbar.Red,
}

// Create another goroutine to close the results chan when all the runner
// goroutines have finished.
func CloseChanOnWaitGroupDone[T any](wg *sync.WaitGroup, c chan<- T) {
	go func() {
		wg.Wait()
		close(c)
	}()
}

// Coverage holds information about coverage gathering. Used by StreamResults.
type Coverage struct {
	Env        *cov.Env
	OutputFile string
}

// StreamResults reads from the chan 'results', printing the results in test-id
// sequential order.
// Once all the results have been printed, a summary will be printed and the
// function will return.
func StreamResults(
	ctx context.Context,
	colors bool,
	state State,
	verbose bool,
	coverage *Coverage, // Optional coverage generation info
	numTestCases int, // Total number of test cases
	stream <-chan Result) (Results, error) {
	// If the context was already cancelled then just return
	if err := ctx.Err(); err != nil {
		return nil, err
	}

	stdout := state.Stdout
	results := Results{}

	// Total number of tests, test counts binned by status
	numByExpectedStatus := map[expectedStatus]int{}

	// Helper function for printing a progress bar.
	lastStatusUpdate, animFrame := time.Now(), 0
	updateProgress := func() {
		drawProgressBar(stdout, animFrame, numTestCases, colors, numByExpectedStatus)
		animFrame++
		lastStatusUpdate = time.Now()
	}

	// Pull test results as they become available.
	// Update the status counts, and print any failures (or all test results if --verbose)
	progressUpdateRate := time.Millisecond * 10
	if !colors {
		// No colors == no cursor control. Reduce progress updates so that
		// we're not printing endless progress bars.
		progressUpdateRate = time.Second
	}

	var covTree *cov.Tree
	if coverage != nil {
		covTree = &cov.Tree{}
	}

	start := time.Now()
	for res := range stream {
		state.Log.logResults(res)
		results[res.TestCase] = res
		expected := state.Expectations[res.TestCase]
		exStatus := expectedStatus{
			status:   res.Status,
			expected: expected == res.Status,
		}
		numByExpectedStatus[exStatus] = numByExpectedStatus[exStatus] + 1
		name := res.TestCase
		if verbose ||
			res.Error != nil ||
			(exStatus.status != Pass && exStatus.status != Skip && !exStatus.expected) {
			buf := &bytes.Buffer{}
			fmt.Fprint(buf, statusColor[res.Status])
			if res.Message != "" {
				fmt.Fprintf(buf, "%v - %v:\n", name, res.Status)
				fmt.Fprintf(buf, term.Reset)
				fmt.Fprintf(buf, "%v", res.Message)
			} else {
				fmt.Fprintf(buf, "%v - %v", name, res.Status)
			}
			if expected != "" {
				fmt.Fprintf(buf, " [%v -> %v]", expected, res.Status)
			}
			fmt.Fprintln(buf)
			if res.Error != nil {
				fmt.Fprintln(buf, res.Error)
			}
			fmt.Fprint(buf, term.Reset)
			fmt.Fprint(stdout, buf.String())
			updateProgress()
		}
		if time.Since(lastStatusUpdate) > progressUpdateRate {
			updateProgress()
		}

		if res.Coverage != nil {
			covTree.Add(splitCTSQuery(res.TestCase), res.Coverage)
		}
	}
	timeTaken := time.Since(start)
	drawProgressBar(stdout, animFrame, numTestCases, colors, numByExpectedStatus)

	// All done. Print final stats.
	fmt.Fprintf(stdout, "\nCompleted in %v\n", timeTaken)

	var numExpectedByStatus map[Status]int
	if state.Expectations != nil {
		// The status of each testcase that was run
		numExpectedByStatus = map[Status]int{}
		for t, s := range state.Expectations {
			if _, wasTested := results[t]; wasTested {
				numExpectedByStatus[s] = numExpectedByStatus[s] + 1
			}
		}
	}

	for _, s := range AllStatuses {
		// number of tests, just run, that resulted in the given status
		numByStatus := numByExpectedStatus[expectedStatus{s, true}] +
			numByExpectedStatus[expectedStatus{s, false}]
		// difference in number of tests that had the given status from the
		// expected number (taken from the expectations file)
		diffFromExpected := 0
		if numExpectedByStatus != nil {
			diffFromExpected = numByStatus - numExpectedByStatus[s]
		}
		if numByStatus == 0 && diffFromExpected == 0 {
			continue
		}

		fmt.Fprint(stdout, term.Bold, statusColor[s])
		fmt.Fprint(stdout, alignRight(strings.ToUpper(string(s))+": ", 10))
		fmt.Fprint(stdout, term.Reset)
		if numByStatus > 0 {
			fmt.Fprint(stdout, term.Bold)
		}
		fmt.Fprint(stdout, alignLeft(numByStatus, 10))
		fmt.Fprint(stdout, term.Reset)
		fmt.Fprint(stdout, alignRight("("+percentage(numByStatus, numTestCases)+")", 6))

		if diffFromExpected != 0 {
			fmt.Fprint(stdout, term.Bold, " [")
			fmt.Fprintf(stdout, "%+d", diffFromExpected)
			fmt.Fprint(stdout, term.Reset, "]")
		}
		fmt.Fprintln(stdout)
	}

	if coverage != nil {
		// Obtain the current git revision
		revision := "HEAD"
		if g, err := git.New(""); err == nil {
			if r, err := g.Open(fileutils.DawnRoot()); err == nil {
				if l, err := r.Log(&git.LogOptions{From: "HEAD", To: "HEAD"}); err == nil {
					revision = l[0].Hash.String()
				}
			}
		}

		if coverage.OutputFile != "" {
			file, err := os.Create(coverage.OutputFile)
			if err != nil {
				return nil, fmt.Errorf("failed to create the coverage file: %w", err)
			}
			defer file.Close()
			if err := covTree.Encode(revision, file); err != nil {
				return nil, fmt.Errorf("failed to encode coverage file: %w", err)
			}

			fmt.Fprintln(stdout)
			fmt.Fprintln(stdout, "Coverage data written to "+coverage.OutputFile)
		} else {
			covData := &bytes.Buffer{}
			if err := covTree.Encode(revision, covData); err != nil {
				return nil, fmt.Errorf("failed to encode coverage file: %w", err)
			}

			const port = 9392
			url := fmt.Sprintf("http://localhost:%v/index.html", port)

			err := cov.StartServer(ctx, port, covData.Bytes(), func() error {
				fmt.Fprintln(stdout)
				fmt.Fprintln(stdout, term.Blue+"Serving coverage view at "+url+term.Reset)
				return browser.Open(url)
			})
			if err != nil {
				return nil, err
			}
		}
	}

	return results, nil
}

// expectedStatus is a test status, along with a boolean to indicate whether the
// status matches the test expectations
type expectedStatus struct {
	status   Status
	expected bool
}

// percentage returns the percentage of n out of total as a string
func percentage(n, total int) string {
	if total == 0 {
		return "-"
	}
	f := float64(n) / float64(total)
	return fmt.Sprintf("%.1f%c", f*100.0, '%')
}

// alignLeft returns the string of 'val' padded so that it is aligned left in
// a column of the given width
func alignLeft(val interface{}, width int) string {
	s := fmt.Sprint(val)
	padding := width - utf8.RuneCountInString(s)
	if padding < 0 {
		return s
	}
	return s + strings.Repeat(" ", padding)
}

// alignRight returns the string of 'val' padded so that it is aligned right in
// a column of the given width
func alignRight(val interface{}, width int) string {
	s := fmt.Sprint(val)
	padding := width - utf8.RuneCountInString(s)
	if padding < 0 {
		return s
	}
	return strings.Repeat(" ", padding) + s
}

// drawProgressBar draws an ANSI-colored progress bar, providing realtime
// information about the status of the CTS run.
// Note: We'll want to skip this if !isatty or if we're running on windows.
func drawProgressBar(stdout io.Writer, animFrame int, numTestCases int, colors bool, numByExpectedStatus map[expectedStatus]int) {
	bar := progressbar.Status{Total: numTestCases}
	for _, status := range AllStatuses {
		for _, expected := range []bool{true, false} {
			if num := numByExpectedStatus[expectedStatus{status, expected}]; num > 0 {
				bar.Segments = append(bar.Segments,
					progressbar.Segment{
						Count:       num,
						Color:       pbStatusColor[status],
						Bold:        expected,
						Transparent: expected,
					})
			}
		}
	}
	const width = 50
	bar.Draw(stdout, width, colors, animFrame)
}

// splitCTSQuery splits a CTS query into a cov.Path
//
// Each WebGPU CTS test is uniquely identified by a test query.
// See https://github.com/gpuweb/cts/blob/main/docs/terms.md#queries about how a
// query is officially structured.
//
// A Path is a simplified form of a CTS Query, where all colons ':' and comma
// ',' denote a split point in the tree. These delimiters are included in the
// parent node's string.
//
// For example, the query string for the single test case:
//
//	webgpu:shader,execution,expression,call,builtin,acos:f32:inputSource="storage_r";vectorize=4
//
// Is broken down into the following strings:
//
//	'webgpu:'
//	'shader,'
//	'execution,'
//	'expression,'
//	'call,'
//	'builtin,'
//	'acos:'
//	'f32:'
//	'inputSource="storage_r";vectorize=4'
func splitCTSQuery(testcase TestCase) cov.Path {
	out := []string{}
	s := 0
	for e, r := range testcase {
		switch r {
		case ':', '.':
			out = append(out, string(testcase[s:e+1]))
			s = e + 1
		}
	}
	if end := testcase[s:]; end != "" {
		out = append(out, string(end))
	}
	return out
}
