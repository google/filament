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

// Package bench provides types and methods for parsing Google benchmark results.
package bench

import (
	"encoding/json"
	"errors"
	"fmt"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"time"
	"unicode/utf8"
)

// Run holds all the benchmark results for a run, along with the context
// information for the run.
type Run struct {
	Benchmarks []Benchmark
	Context    *Context
}

// Context provides information about the environment used to perform the
// benchmark.
type Context struct {
	Date              time.Time
	HostName          string
	Executable        string
	NumCPUs           int
	MhzPerCPU         int
	CPUScalingEnabled bool
	Caches            []ContextCache
	LoadAvg           []float32
	LibraryBuildType  string
}

// ContextCache holds information about one of the system caches.
type ContextCache struct {
	Type       string
	Level      int
	Size       int
	NumSharing int
}

// Benchmark holds the results of a single benchmark test.
type Benchmark struct {
	Name          string
	Duration      time.Duration
	AggregateType AggregateType
}

// AggregateType is an enumerator of benchmark aggregate types.
type AggregateType string

// Enumerator values of AggregateType
const (
	NonAggregate AggregateType = "NonAggregate"
	Mean         AggregateType = "mean"
	Median       AggregateType = "median"
	Stddev       AggregateType = "stddev"
)

// Parse parses the benchmark results from the string s.
// Parse will handle the json and 'console' formats.
func Parse(s string) (Run, error) {
	type Parser = func(s string) (Run, error)
	for _, parser := range []Parser{parseConsole, parseJSON} {
		r, err := parser(s)
		switch err {
		case nil:
			return r, nil
		case errWrongFormat:
		default:
			return Run{}, err
		}
	}

	return Run{}, errors.New("Unrecognised file format")
}

var errWrongFormat = errors.New("Wrong format")
var consoleLineRE = regexp.MustCompile(`([\w/:]+)\s+([0-9]+(?:.[0-9]+)?) ns\s+[0-9]+(?:.[0-9]+) ns\s+([0-9]+)`)

func parseConsole(s string) (Run, error) {
	blocks := strings.Split(s, "------------------------------------------------------------------------------------------")
	if len(blocks) != 3 {
		return Run{}, errWrongFormat
	}

	lines := strings.Split(blocks[2], "\n")
	b := make([]Benchmark, 0, len(lines))

	for _, line := range lines {
		if len(line) == 0 {
			continue
		}
		matches := consoleLineRE.FindStringSubmatch(line)
		if len(matches) != 4 {
			return Run{}, fmt.Errorf("Unable to parse the line:\n%s", line)
		}
		ns, err := strconv.ParseFloat(matches[2], 64)
		if err != nil {
			return Run{}, fmt.Errorf("Unable to parse the duration: %s", matches[2])
		}

		b = append(b, Benchmark{
			Name:     trimAggregateSuffix(matches[1]),
			Duration: time.Nanosecond * time.Duration(ns),
		})
	}
	return Run{Benchmarks: b}, nil
}

func parseJSON(s string) (Run, error) {
	type Data struct {
		Context struct {
			Date              time.Time `json:"date"`
			HostName          string    `json:"host_name"`
			Executable        string    `json:"executable"`
			NumCPUs           int       `json:"num_cpus"`
			MhzPerCPU         int       `json:"mhz_per_cpu"`
			CPUScalingEnabled bool      `json:"cpu_scaling_enabled"`
			LoadAvg           []float32 `json:"load_avg"`
			LibraryBuildType  string    `json:"library_build_type"`
			Caches            []struct {
				Type       string `json:"type"`
				Level      int    `json:"level"`
				Size       int    `json:"size"`
				NumSharing int    `json:"num_sharing"`
			} `json:"caches"`
		} `json:"context"`
		Benchmarks []struct {
			Name          string        `json:"name"`
			Time          float64       `json:"cpu_time"`
			AggregateType AggregateType `json:"aggregate_name"`
		} `json:"benchmarks"`
	}
	data := Data{}
	d := json.NewDecoder(strings.NewReader(s))
	if err := d.Decode(&data); err != nil {
		return Run{}, err
	}

	out := Run{
		Benchmarks: make([]Benchmark, len(data.Benchmarks)),
		Context: &Context{
			Date:              data.Context.Date,
			HostName:          data.Context.HostName,
			Executable:        data.Context.Executable,
			NumCPUs:           data.Context.NumCPUs,
			MhzPerCPU:         data.Context.MhzPerCPU,
			CPUScalingEnabled: data.Context.CPUScalingEnabled,
			LoadAvg:           data.Context.LoadAvg,
			LibraryBuildType:  data.Context.LibraryBuildType,
			Caches:            make([]ContextCache, len(data.Context.Caches)),
		},
	}
	for i, c := range data.Context.Caches {
		out.Context.Caches[i] = ContextCache{
			Type:       c.Type,
			Level:      c.Level,
			Size:       c.Size,
			NumSharing: c.NumSharing,
		}
	}
	for i, b := range data.Benchmarks {
		out.Benchmarks[i] = Benchmark{
			Name:          trimAggregateSuffix(b.Name),
			Duration:      time.Nanosecond * time.Duration(int64(b.Time)),
			AggregateType: b.AggregateType,
		}
	}

	return out, nil
}

// Diff describes the difference between two benchmarks
type Diff struct {
	TestName           string
	Delta              time.Duration // Δ (A → B)
	PercentChangeAB    float64       // % (A → B)
	PercentChangeBA    float64       // % (B → A)
	MultiplierChangeAB float64       // × (A → B)
	MultiplierChangeBA float64       // × (B → A)
	TimeA              time.Duration // A
	TimeB              time.Duration // B
}

// Diffs is a list of Diff
type Diffs []Diff

// DiffFormat describes how a list of diffs should be formatted
type DiffFormat struct {
	TestName           bool
	Delta              bool
	PercentChangeAB    bool
	PercentChangeBA    bool
	MultiplierChangeAB bool
	MultiplierChangeBA bool
	TimeA              bool
	TimeB              bool
}

func (diffs Diffs) Format(f DiffFormat) string {
	if len(diffs) == 0 {
		return "<no changes>"
	}

	type row []string

	header := row{}
	if f.TestName {
		header = append(header, "Test name")
	}
	if f.Delta {
		header = append(header, "Δ (A → B)")
	}
	if f.PercentChangeAB {
		header = append(header, "% (A → B)")
	}
	if f.PercentChangeBA {
		header = append(header, "% (B → A)")
	}
	if f.MultiplierChangeAB {
		header = append(header, "× (A → B)")
	}
	if f.MultiplierChangeBA {
		header = append(header, "× (B → A)")
	}
	if f.TimeA {
		header = append(header, "A")
	}
	if f.TimeB {
		header = append(header, "B")
	}
	if len(header) == 0 {
		return ""
	}

	columns := []row{}
	for _, d := range diffs {
		r := make(row, 0, len(header))
		if f.TestName {
			r = append(r, d.TestName)
		}
		if f.Delta {
			r = append(r, fmt.Sprintf("%v", d.Delta))
		}
		if f.PercentChangeAB {
			r = append(r, fmt.Sprintf("%+2.1f%%", d.PercentChangeAB))
		}
		if f.PercentChangeBA {
			r = append(r, fmt.Sprintf("%+2.1f%%", d.PercentChangeBA))
		}
		if f.MultiplierChangeAB {
			r = append(r, fmt.Sprintf("%+.4f", d.MultiplierChangeAB))
		}
		if f.MultiplierChangeBA {
			r = append(r, fmt.Sprintf("%+.4f", d.MultiplierChangeBA))
		}
		if f.TimeA {
			r = append(r, fmt.Sprintf("%v", d.TimeA))
		}
		if f.TimeB {
			r = append(r, fmt.Sprintf("%v", d.TimeB))
		}
		columns = append(columns, r)
	}

	// measure
	widths := make([]int, len(header))
	for i, h := range header {
		widths[i] = utf8.RuneCountInString(h)
	}
	for _, row := range columns {
		for i, cell := range row {
			l := utf8.RuneCountInString(cell)
			if widths[i] < l {
				widths[i] = l
			}
		}
	}

	pad := func(s string, i int) string {
		if n := i - utf8.RuneCountInString(s); n > 0 {
			return s + strings.Repeat(" ", n)
		}
		return s
	}

	// Draw table
	b := &strings.Builder{}

	horizontal_bar := func() {
		for i := range header {
			fmt.Fprintf(b, "+%v", strings.Repeat("-", 2+widths[i]))
		}
		fmt.Fprintln(b, "+")
	}

	horizontal_bar()

	for i, h := range header {
		fmt.Fprintf(b, "| %v ", pad(h, widths[i]))
	}
	fmt.Fprintln(b, "|")

	horizontal_bar()

	for _, row := range columns {
		for i, cell := range row {
			fmt.Fprintf(b, "| %v ", pad(cell, widths[i]))
		}
		fmt.Fprintln(b, "|")
	}

	horizontal_bar()

	return b.String()
}

// Compare returns a string describing differences in the two benchmarks
// Absolute benchmark differences less than minDiff are omitted
// Absolute relative differences between [1, 1+x] are omitted
func Compare(a, b []Benchmark, minDiff time.Duration, minRelDiff float64) Diffs {
	type times struct {
		a time.Duration
		b time.Duration
	}
	byName := map[string]times{}
	for _, test := range a {
		byName[test.Name] = times{a: test.Duration}
	}
	for _, test := range b {
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
		if absDiff < minDiff {
			continue
		}

		relDiff := float64(times.b) / float64(times.a)
		absRelDiff := relDiff
		if absRelDiff < 1 {
			absRelDiff = 1.0 / absRelDiff
		}
		if absRelDiff < (1.0 + minRelDiff) {
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

	out := make(Diffs, len(deltas))

	for i, delta := range deltas {
		a2b := delta.times.b - delta.times.a
		out[i] = Diff{
			TestName:           delta.name,
			Delta:              a2b,
			PercentChangeAB:    100 * float64(a2b) / float64(delta.times.a),
			PercentChangeBA:    100 * float64(-a2b) / float64(delta.times.b),
			MultiplierChangeAB: float64(delta.times.b) / float64(delta.times.a),
			MultiplierChangeBA: float64(delta.times.a) / float64(delta.times.b),
			TimeA:              delta.times.a,
			TimeB:              delta.times.b,
		}
	}
	return out
}

func trimAggregateSuffix(name string) string {
	name = strings.TrimSuffix(name, "_stddev")
	name = strings.TrimSuffix(name, "_mean")
	name = strings.TrimSuffix(name, "_median")
	return name
}
