// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// check_build_files scans all the .bp, .gn and .bazel files for source
// references to non-existent files.

package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"regexp"
	"strings"
)

func cwd() string {
	wd, err := os.Getwd()
	if err != nil {
		return ""
	}
	return wd
}

var root = flag.String("root", cwd(), "root project directory")

func main() {
	flag.Parse()

	if err := run(); err != nil {
		fmt.Fprintf(os.Stderr, "%v", err)
		os.Exit(1)
	}
	fmt.Printf("Build file check completed with no errors\n")
}

func run() error {
	wd := *root

	errs := []error{}

	filepath.Walk(wd, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		rel, err := filepath.Rel(wd, path)
		if err != nil {
			return filepath.SkipDir
		}

		switch rel {
		case ".git", "cache", "build", "out", "third_party":
			return filepath.SkipDir
		}

		if info.IsDir() {
			return nil
		}

		content, err := ioutil.ReadFile(path)
		if err != nil {
			errs = append(errs, err)
			return nil // Continue walking files
		}

		switch filepath.Ext(path) {
		case ".bp":
			errs = append(errs, checkBlueprint(path, string(content))...)
		case ".gn":
			errs = append(errs, checkGn(path, string(content))...)
		case ".bazel":
			errs = append(errs, checkBazel(path, string(content))...)
		}

		return nil
	})

	sb := strings.Builder{}
	for _, err := range errs {
		sb.WriteString(err.Error())
		sb.WriteString("\n")
	}
	if sb.Len() > 0 {
		return fmt.Errorf("%v", sb.String())
	}
	return nil
}

var (
	reSources = regexp.MustCompile(`sources\s*=\s*\[([^\]]*)\]`)
	reSrc     = regexp.MustCompile(`srcs\s*[:=]\s*\[([^\]]*)\]`)
	reQuoted  = regexp.MustCompile(`"([^\"]*)"`)
)

func checkBlueprint(path, content string) []error {
	errs := []error{}
	for _, sources := range matchRE(reSrc, content) {
		for _, source := range matchRE(reQuoted, sources) {
			if strings.HasPrefix(source, ":") {
				continue // Build target, we can't resolve.
			}
			if err := checkSource(path, source); err != nil {
				errs = append(errs, err)
			}
		}
	}
	return errs
}

func checkGn(path, content string) []error {
	errs := []error{}
	for _, sources := range matchRE(reSources, content) {
		for _, source := range matchRE(reQuoted, sources) {
			if strings.ContainsAny(source, "$") {
				return nil // Env vars we can't resolve
			}
			if strings.HasPrefix(source, "//") {
				continue // Build target, we can't resolve.
			}
			if err := checkSource(path, source); err != nil {
				errs = append(errs, err)
			}
		}
	}
	return errs
}

func checkBazel(path, content string) []error {
	errs := []error{}
	for _, sources := range matchRE(reSrc, content) {
		for _, source := range matchRE(reQuoted, sources) {
			if strings.HasPrefix(source, "@") || strings.HasPrefix(source, ":") {
				continue // Build target, we can't resolve.
			}
			if err := checkSource(path, source); err != nil {
				errs = append(errs, err)
			}
		}
	}
	return errs
}

func checkSource(path, source string) error {
	source = filepath.Join(filepath.Dir(path), source)

	if strings.Contains(source, "*") {
		sources, err := filepath.Glob(source)
		if err != nil {
			return fmt.Errorf("In '%v': %w", path, err)
		}
		if len(sources) == 0 {
			return fmt.Errorf("In '%v': Glob '%v' does not reference any files", path, source)
		}
		return nil
	}

	stat, err := os.Stat(source)
	if err != nil {
		return fmt.Errorf("In '%v': %w", path, err)
	}
	if stat.IsDir() {
		return fmt.Errorf("In '%v': '%v' refers to a directory, not a file", path, source)
	}
	return nil
}

func matchRE(re *regexp.Regexp, text string) []string {
	out := []string{}
	for _, match := range re.FindAllStringSubmatch(text, -1) {
		if len(match) < 2 {
			return nil
		}
		out = append(out, match[1])
	}
	return out
}
