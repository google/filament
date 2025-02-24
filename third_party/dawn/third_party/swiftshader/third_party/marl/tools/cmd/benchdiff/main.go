// Copyright 2020 The Marl Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// benchdiff is a tool that compares two Google benchmark results and displays
// sorted performance differences.
package main

import (
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"sort"
	"text/tabwriter"
	"time"

	"github.com/google/marl/tools/bench"
)

var (
	minDiff    = flag.Duration("min-diff", time.Microsecond*10, "Filter away time diffs less than this duration")
	minRelDiff = flag.Float64("min-rel-diff", 0.01, "Filter away absolute relative diffs between [1, 1+x]")
)

func main() {
	flag.ErrHelp = errors.New("benchdiff is a tool to compare two benchmark results")
	flag.Parse()
	flag.Usage = func() {
		fmt.Fprintln(os.Stderr, "benchdiff <benchmark-a> <benchmark-b>")
		flag.PrintDefaults()
	}

	args := flag.Args()
	if len(args) < 2 {
		flag.Usage()
		os.Exit(1)
	}

	pathA, pathB := args[0], args[1]

	if err := run(pathA, pathB); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(-1)
	}
}

func run(pathA, pathB string) error {
	fileA, err := ioutil.ReadFile(pathA)
	if err != nil {
		return err
	}
	benchA, err := bench.Parse(string(fileA))
	if err != nil {
		return err
	}

	fileB, err := ioutil.ReadFile(pathB)
	if err != nil {
		return err
	}
	benchB, err := bench.Parse(string(fileB))
	if err != nil {
		return err
	}

	compare(benchA, benchB, fileName(pathA), fileName(pathB))

	return nil
}

func fileName(path string) string {
	_, name := filepath.Split(path)
	return name
}

func compare(benchA, benchB bench.Benchmark, nameA, nameB string) {
	type times struct {
		a time.Duration
		b time.Duration
	}
	byName := map[string]times{}
	for _, test := range benchA.Tests {
		byName[test.Name] = times{a: test.Duration}
	}
	for _, test := range benchB.Tests {
		t := byName[test.Name]
		t.b = test.Duration
		byName[test.Name] = t
	}

	type delta struct {
		name       string
		times      times
		relDiff    float64
		absRelDiff float64
	}
	deltas := []delta{}
	for name, times := range byName {
		if times.a == 0 || times.b == 0 {
			continue // Assuming test was missing from a or b
		}
		diff := times.b - times.a
		absDiff := diff
		if absDiff < 0 {
			absDiff = -absDiff
		}
		if absDiff < *minDiff {
			continue
		}

		relDiff := float64(times.b) / float64(times.a)
		absRelDiff := relDiff
		if absRelDiff < 1 {
			absRelDiff = 1.0 / absRelDiff
		}
		if absRelDiff < (1.0 + *minRelDiff) {
			continue
		}

		d := delta{
			name:       name,
			times:      times,
			relDiff:    relDiff,
			absRelDiff: absRelDiff,
		}
		deltas = append(deltas, d)
	}

	sort.Slice(deltas, func(i, j int) bool { return deltas[j].relDiff < deltas[i].relDiff })

	w := tabwriter.NewWriter(os.Stdout, 1, 1, 0, ' ', 0)
	fmt.Fprintf(w, "Delta\t | Test name\t | (A) %v\t | (B) %v\n", nameA, nameB)
	for _, delta := range deltas {
		sign, diff := "+", delta.times.b-delta.times.a
		if diff < 0 {
			sign, diff = "-", -diff
		}
		fmt.Fprintf(w, "%v%.2fx %v%+v\t | %v\t | %v\t | %v\n", sign, delta.absRelDiff, sign, diff, delta.name, delta.times.a, delta.times.b)
	}
	w.Flush()
}
