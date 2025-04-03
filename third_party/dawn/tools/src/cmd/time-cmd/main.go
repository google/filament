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

// time-cmd runs a given command for each file found in a glob, returning the
// sorted run times.
package main

import (
	"flag"
	"fmt"
	"math/rand"
	"os"
	"os/exec"
	"sort"
	"strings"
	"text/tabwriter"
	"time"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/progressbar"
)

func main() {
	if err := run(oswrapper.GetRealOSWrapper()); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

// TODO(crbug.com/344014313): Split this into multiple helper functions so that
// it can be unittested. Testing is not currently possible due to the call to
// exec.LookPath.
func run(osWrapper oswrapper.OSWrapper) error {
	flag.Usage = func() {
		out := flag.CommandLine.Output()
		fmt.Fprintf(out, "time-cmd runs a given command for each file found in a glob, returning the sorted run times..\n")
		fmt.Fprintf(out, "\n")
		fmt.Fprintf(out, "Usage:\n")
		fmt.Fprintf(out, "  time-cmd [flags] <cmd [args...]>\n")
		fmt.Fprintf(out, "\n")
		fmt.Fprintf(out, "cmd is the path to the command to run\n")
		fmt.Fprintf(out, "args are optional command line arguments to 'cmd'\n")
		fmt.Fprintf(out, "args should include '%%F' for the globbed file path\n")
		fmt.Fprintf(out, "\n")
		fmt.Fprintf(out, "flags may be any combination of:\n")
		flag.PrintDefaults()
	}
	var fileGlob string
	var top, runs int
	flag.StringVar(&fileGlob, "f", "", "the files to glob. Paths are relative to the dawn root directory")
	flag.IntVar(&top, "top", 0, "displays the longest N results")
	flag.IntVar(&runs, "runs", 1, "takes the average of N runs")
	flag.Parse()

	args := flag.Args()
	if fileGlob == "" {
		flag.Usage()
		return fmt.Errorf("Missing --f flag")
	}
	if len(args) < 1 {
		flag.Usage()
		return fmt.Errorf("Missing command")
	}

	fileGlob = fileutils.ExpandHome(fileGlob, osWrapper)

	exe, err := exec.LookPath(fileutils.ExpandHome(args[0], osWrapper))
	if err != nil {
		return fmt.Errorf("could not find executable '%v'", args[0])
	}

	files, err := glob.Glob(fileGlob, osWrapper)
	if err != nil {
		return err
	}

	if len(files) == 0 {
		fmt.Println("no files found with glob '" + fileGlob + "'")
		return nil
	}

	pb := progressbar.New(os.Stdout, nil)
	defer pb.Stop()

	type Result struct {
		file string
		time time.Duration
		errs []error
	}
	status := progressbar.Status{
		Total:    len(files) * runs,
		Segments: []progressbar.Segment{{Color: progressbar.Green}, {Color: progressbar.Red}},
	}
	segSuccess := &status.Segments[0]
	segError := &status.Segments[1]

	results := make([]Result, len(files))
	for i, f := range files {
		results[i].file = f
	}

	for run := 0; run < runs; run++ {
		rand.Shuffle(len(results), func(i, j int) { results[i], results[j] = results[j], results[i] })
		for i := range results {
			result := &results[i]
			exeArgs := make([]string, len(args[1:]))
			for i, arg := range args[1:] {
				exeArgs[i] = strings.ReplaceAll(arg, "%F", result.file)
			}
			cmd := exec.Command(exe, exeArgs...)

			start := time.Now()
			out, err := cmd.CombinedOutput()
			time := time.Since(start)

			result.time += time
			if err != nil {
				result.errs = append(result.errs, fmt.Errorf("%v", string(out)))
				segError.Count++
			} else {
				segSuccess.Count++
			}
			pb.Update(status)
		}
	}

	sort.Slice(results, func(i, j int) bool { return results[i].time > results[j].time })

	if top > 0 {
		if top > len(results) {
			top = len(results)
		}
		results = results[:top]
	}

	tw := tabwriter.NewWriter(os.Stdout, 1, 0, 1, ' ', 0)
	defer tw.Flush()
	for i, r := range results {
		s := ""
		if len(r.errs) > 0 {
			s = fmt.Sprint(r.errs)
		}
		fmt.Fprintf(tw, "%v\t%v\t%v\t%v\n", i, r.time/time.Duration(runs), r.file, s)
	}
	return nil
}
