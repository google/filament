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
	"time"

	"dawn.googlesource.com/dawn/tools/src/bench"
)

var (
	minDiff    = flag.Duration("min-diff", time.Microsecond*10, "Filter away time diffs less than this duration")
	minRelDiff = flag.Float64("min-rel-diff", 0.01, "Filter away absolute relative diffs between [1, 1+x]")
)

func main() {
	flag.ErrHelp = errors.New("benchdiff is a tool to compare two benchmark results")
	flag.Usage = func() {
		fmt.Fprintln(os.Stderr, "benchdiff <benchmark-a> <benchmark-b>")
		flag.PrintDefaults()
	}
	flag.Parse()

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

	cmp := bench.Compare(benchA.Benchmarks, benchB.Benchmarks, *minDiff, *minRelDiff)
	diff := cmp.Format(bench.DiffFormat{
		TestName:        true,
		Delta:           true,
		PercentChangeAB: true,
		TimeA:           true,
		TimeB:           true,
	})

	fmt.Println("A:", pathA, "  B:", pathB)
	fmt.Println()
	fmt.Println(diff)

	return nil
}

func fileName(path string) string {
	_, name := filepath.Split(path)
	return name
}
