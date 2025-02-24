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

// tint-bench repeatedly emits a WGSL file from a template, then times how long
// it takes to execute the tint executable with that WGSL file.
package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"sort"
	"time"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/template"
)

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

// [from .. to]
type Range struct {
	from int
	to   int
}

func run() error {
	alphaRange := Range{}

	iterations := 0
	tmplPath := ""
	flag.StringVar(&tmplPath, "tmpl", "tint-bench.tmpl", `the WGSL template to benchmark.
Searches in order: absolute, relative to CWD, then relative to `+fileutils.ThisDir())
	flag.IntVar(&alphaRange.from, "alpha-from", 0, "the start value for 'Alpha'")
	flag.IntVar(&alphaRange.to, "alpha-to", 10, "the end value for 'Alpha'")
	flag.IntVar(&iterations, "iterations", 10, "number of times to benchmark tint")
	flag.Usage = func() {
		fmt.Println("tint-bench repeatedly emits a WGSL file from a template, then times how long")
		fmt.Println("it takes to execute the tint executable with that WGSL file.")
		fmt.Println("")
		fmt.Println("usage:")
		fmt.Println("  tint-bench <bench-flags> [tint-exe] <tint-flags>")
		fmt.Println("")
		fmt.Println("bench-flags:")
		flag.PrintDefaults()
		os.Exit(1)
	}

	flag.Parse()

	if tmplPath == "" {
		return fmt.Errorf("missing template path")
	}

	tmpl, err := template.FromFile(tmplPath)
	if err != nil {
		if !filepath.IsAbs(tmplPath) {
			// Try relative to this .go file
			tmplPath = filepath.Join(fileutils.ThisDir(), tmplPath)
			tmpl, err = template.FromFile(tmplPath)
		}
	}
	if err != nil {
		return fmt.Errorf("failed to load template: %v", err)
	}

	args := flag.Args()
	if len(args) < 1 {
		flag.Usage()
	}
	tintExe := args[0]

	fmt.Println(" alpha | Time (μs)")
	fmt.Println("-------+-----------------")

	for alpha := alphaRange.from; alpha < alphaRange.to; alpha++ {
		alpha := alpha
		funcs := template.Functions{
			"Alpha": func() int { return alpha },
		}
		wgslPath, err := writeWGSLFile(tmpl, funcs)
		if err != nil {
			return err
		}

		tintArgs := []string{wgslPath}
		tintArgs = append(tintArgs, args[1:]...)

		durations := []time.Duration{}
		for i := 0; i < iterations; i++ {
			tint := exec.Command(tintExe, tintArgs...)
			start := time.Now()
			if out, err := tint.CombinedOutput(); err != nil {
				return fmt.Errorf("tint failed with error: %w\n%v\n\nwith: Alpha=%v", err, string(out), alpha)
			}
			duration := time.Since(start)
			durations = append(durations, duration)
		}
		sort.Slice(durations, func(i, j int) bool { return durations[i] < durations[j] })

		median := durations[len(durations)/2]
		fmt.Printf("%6.v | %v\n", alpha, median.Microseconds())
	}
	return nil
}

func writeWGSLFile(tmpl *template.Template, funcs template.Functions) (string, error) {
	const path = "tint-bench.wgsl"
	wgslFile, err := os.Create(path)
	if err != nil {
		return "", fmt.Errorf("failed to create benchmark WGSL test file: %w", err)
	}
	defer wgslFile.Close()
	if err := tmpl.Run(wgslFile, nil, funcs); err != nil {
		return "", fmt.Errorf("template error:\n%w", err)
	}
	return path, nil
}
