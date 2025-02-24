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

// Package bench provides types and methods for parsing Google benchmark results.
package bench

import (
	"encoding/json"
	"errors"
	"fmt"
	"regexp"
	"strconv"
	"strings"
	"time"
)

// Test holds the results of a single benchmark test.
type Test struct {
	Name       string
	NumTasks   uint
	NumThreads uint
	Duration   time.Duration
	Iterations uint
}

var testVarRE = regexp.MustCompile(`([\w])+:([0-9]+)`)

func (t *Test) parseName() {
	for _, match := range testVarRE.FindAllStringSubmatch(t.Name, -1) {
		if len(match) != 3 {
			continue
		}
		n, err := strconv.Atoi(match[2])
		if err != nil {
			continue
		}
		switch match[1] {
		case "threads":
			t.NumThreads = uint(n)
		case "tasks":
			t.NumTasks = uint(n)
		}
	}
}

// Benchmark holds a set of benchmark test results.
type Benchmark struct {
	Tests []Test
}

// Parse parses the benchmark results from the string s.
// Parse will handle the json and 'console' formats.
func Parse(s string) (Benchmark, error) {
	type Parser = func(s string) (Benchmark, error)
	for _, parser := range []Parser{parseConsole, parseJSON} {
		b, err := parser(s)
		switch err {
		case nil:
			return b, nil
		case errWrongFormat:
		default:
			return Benchmark{}, err
		}
	}

	return Benchmark{}, errors.New("Unrecognised file format")
}

var errWrongFormat = errors.New("Wrong format")
var consoleLineRE = regexp.MustCompile(`([\w/:]+)\s+([0-9]+(?:.[0-9e+]+)?) ns\s+[0-9]+(?:.[0-9e+]+) ns\s+([0-9]+)`)

func parseConsole(s string) (Benchmark, error) {
	blocks := strings.Split(s, "--------------------------------------------------------------------------------------------------------")
	if len(blocks) != 3 {
		return Benchmark{}, errWrongFormat
	}

	lines := strings.Split(blocks[2], "\n")
	b := Benchmark{
		Tests: make([]Test, 0, len(lines)),
	}
	for _, line := range lines {
		if len(line) == 0 {
			continue
		}
		matches := consoleLineRE.FindStringSubmatch(line)
		if len(matches) != 4 {
			return Benchmark{}, fmt.Errorf("Unable to parse the line:\n" + line)
		}
		ns, err := strconv.ParseFloat(matches[2], 64)
		if err != nil {
			return Benchmark{}, fmt.Errorf("Unable to parse the duration: " + matches[2])
		}
		iterations, err := strconv.Atoi(matches[3])
		if err != nil {
			return Benchmark{}, fmt.Errorf("Unable to parse the number of iterations: " + matches[3])
		}

		t := Test{
			Name:       matches[1],
			Duration:   time.Nanosecond * time.Duration(ns),
			Iterations: uint(iterations),
		}
		t.parseName()
		b.Tests = append(b.Tests, t)
	}
	return b, nil
}

func parseJSON(s string) (Benchmark, error) {
	type T struct {
		Name       string  `json:"name"`
		Iterations uint    `json:"iterations"`
		Time       float64 `json:"real_time"`
	}
	type B struct {
		Tests []T `json:"benchmarks"`
	}
	b := B{}
	d := json.NewDecoder(strings.NewReader(s))
	if err := d.Decode(&b); err != nil {
		return Benchmark{}, err
	}

	out := Benchmark{
		Tests: make([]Test, len(b.Tests)),
	}
	for i, test := range b.Tests {
		t := Test{
			Name:       test.Name,
			Duration:   time.Nanosecond * time.Duration(int64(test.Time)),
			Iterations: test.Iterations,
		}
		t.parseName()
		out.Tests[i] = t
	}

	return out, nil
}
