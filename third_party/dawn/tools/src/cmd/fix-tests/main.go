// Copyright 2021 The Dawn & Tint Authors
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

// fix-tests is a tool to update tests with new expected output.
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/substr"
)

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
fix-tests is a tool to update tests with new expected output.

fix-tests performs string matching and heuristics to fix up expected results of
tests that use EXPECT_EQ(a, b) and EXPECT_THAT(a, HasSubstr(b))

WARNING: Always thoroughly check the generated output for mistakes.
This may produce incorrect output

Usage:
  fix-tests <executable>

  executable         - the path to the test executable to run.`)
	os.Exit(1)
}

func run() error {
	flag.Parse()
	args := flag.Args()
	if len(args) < 1 {
		showUsage()
	}

	exe := args[0]          // The path to the test executable
	wd := filepath.Dir(exe) // The directory holding the test exe

	// Create a temporary directory to hold the 'test-results.json' file
	tmpDir, err := ioutil.TempDir("", "fix-tests")
	if err != nil {
		return err
	}
	if err := os.MkdirAll(tmpDir, 0666); err != nil {
		return fmt.Errorf("Failed to create temporary directory: %w", err)
	}
	defer os.RemoveAll(tmpDir)

	// Full path to the 'test-results.json' in the temporary directory
	testResultsPath := filepath.Join(tmpDir, "test-results.json")

	// Run the tests
	testArgs := []string{"--gtest_output=json:" + testResultsPath}
	if len(args) > 1 {
		testArgs = append(testArgs, args[1:]...)
	}
	switch err := exec.Command(exe, testArgs...).Run().(type) {
	default:
		return err
	case nil:
		fmt.Println("All tests passed")
	case *exec.ExitError:
	}

	// Read the 'test-results.json' file
	testResultsFile, err := os.Open(testResultsPath)
	if err != nil {
		return err
	}

	var testResults Results
	if err := json.NewDecoder(testResultsFile).Decode(&testResults); err != nil {
		return err
	}

	// For each failing test...
	seen := map[string]bool{}
	numFixed, numFailed := 0, 0
	for _, group := range testResults.Groups {
		for _, suite := range group.Testsuites {
			for _, failure := range suite.Failures {
				// .. attempt to fix the problem
				test := testName(group, suite)
				if seen[test] {
					continue
				}
				seen[test] = true

				if err := processFailure(test, wd, failure.Failure); err != nil {
					fmt.Println(fmt.Errorf("%v: %w", test, err))
					numFailed++
				} else {
					numFixed++
				}
			}
		}
	}

	fmt.Println()

	if numFailed > 0 {
		fmt.Println(numFailed, "tests could not be fixed")
	}
	if numFixed > 0 {
		fmt.Println(numFixed, "tests fixed")
	}
	return nil
}

func testName(group TestsuiteGroup, suite Testsuite) string {
	groupParts := strings.Split(group.Name, "/")
	suiteParts := strings.Split(suite.Name, "/")
	return groupParts[len(groupParts)-1] + "." + suiteParts[0]
}

var (
	// Regular expression to match a test declaration
	reTests = regexp.MustCompile(`TEST(?:_[FP])?\([ \n]*(\w+),[ \n]*(\w+)\)`)
	// Regular expression to match a `EXPECT_EQ(a, b)` failure for strings
	reExpectEq = regexp.MustCompile(`([./\\\w_\-:]*):(\d+).*\nExpected equality of these values:\n(?:.|\n)*?(?:Which is: |  )"((?:.|\n)*?[^\\])"\n(?:.|\n)*?(?:Which is: |  )"((?:.|\n)*?[^\\])"`)
	// Regular expression to match a `EXPECT_THAT(a, HasSubstr(b))` failure for strings
	reExpectHasSubstr = regexp.MustCompile(`([./\\\w_\-:]*):(\d+).*\nValue of: .*\nExpected: has substring "((?:.|\n)*?[^\\])"\n  Actual: "((?:.|\n)*?[^\\])"`)
)

var reHexCode = regexp.MustCompile(`\\x([0-9A-Z]{2})`)

type MatchRange struct {
	start int
	end   int // exclusive
}

func Size(mr MatchRange) int {
	return mr.end - mr.start
}

// Look for 'search_str' in the 'doc_string' and return the longest matching substring of 'search_str' as start/end indices
func longestSubstringMatch(search_str string, doc_string string) MatchRange {
	// Brute force algorithm is n*m for string sizes n and m
	best_idx_start := 0
	best_idx_end := 0 // exclusive
	// Scan the document with the search string from the highest offset to the lowest. This will be out of bound for search string but that is because we are looking of a substring.
	for doc_offset := -len(search_str); doc_offset < len(doc_string)+len(search_str); doc_offset++ {
		curr_idx_start := 0
		curr_idx_end := 0 // exclusive
		for search_idx := 0; search_idx < len(search_str); search_idx++ {
			doc_offset := search_idx + doc_offset
			is_match := false

			// basic range checking for the doc
			if doc_offset >= 0 && doc_offset < len(doc_string) {
				if search_str[search_idx] == doc_string[doc_offset] {
					is_match = true
				}
			}

			if is_match {
				if curr_idx_end == 0 {
					// first time matching
					curr_idx_start = search_idx
					curr_idx_end = curr_idx_start + 1 // exclusive
				} else {
					// continue current matching.
					curr_idx_end++
				}
			}

			// check if our match is the best
			best_size := best_idx_end - best_idx_start
			curr_size := curr_idx_end - curr_idx_start

			if best_size < curr_size {
				best_idx_start = curr_idx_start
				best_idx_end = curr_idx_end
			}

			if !is_match {
				// reset
				curr_idx_start = 0
				curr_idx_end = 0
			}
		}
	}

	return MatchRange{best_idx_start, best_idx_end}
}

func processFailure(test, wd, failure string) error {
	// Start by un-escaping newlines in the failure message
	failure = strings.ReplaceAll(failure, "\\n", "\n")
	// Matched regex strings will also need to be un-escaped, but do this after
	// the match, as unescaped quotes may upset the regex patterns
	unescape := func(s string) string {
		s = strings.ReplaceAll(s, `\"`, `"`)
		s = strings.ReplaceAll(s, `\\`, `\`)
		s = reHexCode.ReplaceAllStringFunc(s, func(match string) string {
			i, err := strconv.ParseInt(match[2:], 16, 32)
			if err != nil {
				panic(err)
			}
			return string([]byte{byte(i)})
		})
		return s
	}
	escape := func(s string) string {
		s = strings.ReplaceAll(s, "\n", `\n`)
		s = strings.ReplaceAll(s, "\"", `\"`)
		return s
	}

	// Look for a EXPECT_EQ failure pattern
	var file string
	var fix func(testSource string) (string, error)
	if parts := reExpectEq.FindStringSubmatch(failure); len(parts) == 5 {
		a, b := unescape(parts[3]), unescape(parts[4])
		file = parts[1]
		fix = func(testSource string) (string, error) {
			// This code is written so it can insert changes as fixes for expects that only match a substring.
			// An example of where this is required is the "glsl/writer/builtin_test.cc" due to the programmatic header (GlslHeader())
			// We don't know if a or b is the expected. We also dont know if it should be escaped for R"(...)" strings
			a_esc, b_esc := escape(a), escape(b)

			// Find the longest match. We have (unfortunately) 4 options.
			mr_a := longestSubstringMatch(a, testSource)
			mr_b := longestSubstringMatch(b, testSource)
			mr_a_esc := longestSubstringMatch(a_esc, testSource)
			mr_b_esc := longestSubstringMatch(b_esc, testSource)

			// Perfect matches are prioritized for the cases where there may be more than matchable string in test function.
			// This is common in tint where there is both the 'src' and 'expect' strings of non-failing transform test.
			var a_perfect = len(a) == Size(mr_a)
			var b_perfect = len(b) == Size(mr_b)
			var a_esc_perfect = len(a_esc) == Size(mr_a_esc)
			var b_esc_perfect = len(b_esc) == Size(mr_b_esc)
			var has_perfect = a_perfect || b_perfect || a_esc_perfect || b_esc_perfect

			use_largest := func(mr MatchRange) bool {
				return Size(mr) >= Size(mr_a) && Size(mr) >= Size(mr_b) &&
					Size(mr) >= Size(mr_a_esc) && Size(mr) >= Size(mr_b_esc) && !has_perfect
			}

			// assumed mr_b_esc is best match
			expected_str := b_esc
			replace_str := a_esc
			mr_largest := mr_b_esc

			if use_largest(mr_a) || a_perfect {
				expected_str = a
				replace_str = b
				mr_largest = mr_a
			} else if use_largest(mr_b) || b_perfect {
				expected_str = b
				replace_str = a
				mr_largest = mr_b
			} else if use_largest(mr_a_esc) || a_esc_perfect {
				expected_str = a_esc
				replace_str = b_esc
				mr_largest = mr_a_esc
			}

			// trim away the number of unmatched characters from the end of expected to the end of the replacement.
			replace_str_end := len(replace_str) - (len(expected_str) - mr_largest.end)
			if replace_str_end >= mr_largest.start && replace_str_end <= len(replace_str) {
				replace_str = replace_str[mr_largest.start:replace_str_end]
				expected_str = expected_str[mr_largest.start:mr_largest.end]
			} else {
				// It is not safe to attempt a replace if the replacement string would have negative (nonsense) size.
				expected_str = ""
			}

			// Do not try to replace on empty strings.
			if len(expected_str) <= 0 {
				return "", fmt.Errorf("could not fix 'EXPECT_EQ' pattern in '%v'\n\nA: '%v'\n\nB: '%v'", file, a, b)
			}

			testSource = strings.ReplaceAll(testSource, expected_str, replace_str)
			return testSource, nil
		}
	} else if parts := reExpectHasSubstr.FindStringSubmatch(failure); len(parts) == 5 {
		// EXPECT_THAT(a, HasSubstr(b))
		a, b := unescape(parts[4]), unescape(parts[3])
		file = parts[1]
		fix = func(testSource string) (string, error) {
			if fix := substr.Fix(a, b); fix != "" {
				if !strings.Contains(testSource, b) {
					// Try escaping for R"(...)" strings
					b, fix = escape(b), escape(fix)
				}
				if strings.Contains(testSource, b) {
					testSource = strings.Replace(testSource, b, fix, -1)
					return testSource, nil
				}
				return "", fmt.Errorf("could apply fix for 'HasSubstr' pattern in '%v'", file)
			}

			return "", fmt.Errorf("could find fix for 'HasSubstr' pattern in '%v'", file)
		}
	} else {
		return fmt.Errorf("cannot fix this type of failure")
	}

	// Get the absolute source path
	sourcePath := file
	if !filepath.IsAbs(sourcePath) {
		sourcePath = filepath.Join(wd, file)
	}

	// Parse the source file, split into tests
	sourceFile, err := parseSourceFile(sourcePath)
	if err != nil {
		return fmt.Errorf("Couldn't parse tests from file '%v': %w", file, err)
	}

	// Find the test
	testIdx, ok := sourceFile.tests[test]
	if !ok {
		return fmt.Errorf("Test not found in '%v'", file)
	}

	// Grab the source for the particular test
	testSource := sourceFile.parts[testIdx]

	if testSource, err = fix(testSource); err != nil {
		return err
	}

	// Replace the part of the source file
	sourceFile.parts[testIdx] = testSource

	// Write out the source file
	return writeSourceFile(sourcePath, sourceFile)
}

// parseSourceFile() reads the file at path, splitting the content into chunks
// for each TEST.
func parseSourceFile(path string) (sourceFile, error) {
	fileBytes, err := ioutil.ReadFile(path)
	if err != nil {
		return sourceFile{}, err
	}
	fileContent := string(fileBytes)

	out := sourceFile{
		tests: map[string]int{},
	}

	pos := 0
	for _, span := range reTests.FindAllStringIndex(fileContent, -1) {
		out.parts = append(out.parts, fileContent[pos:span[0]])
		pos = span[0]

		match := reTests.FindStringSubmatch(fileContent[span[0]:span[1]])
		group := match[1]
		suite := match[2]
		out.tests[group+"."+suite] = len(out.parts)
	}
	out.parts = append(out.parts, fileContent[pos:])

	return out, nil
}

// writeSourceFile() joins the chunks of the file, and writes the content out to
// path.
func writeSourceFile(path string, file sourceFile) error {
	body := strings.Join(file.parts, "")
	return ioutil.WriteFile(path, []byte(body), 0666)
}

type sourceFile struct {
	parts []string
	tests map[string]int // "X.Y" -> part index
}

// Results is the root JSON structure of the JSON --gtest_output file .
type Results struct {
	Tests     int              `json:"tests"`
	Failures  int              `json:"failures"`
	Disabled  int              `json:"disabled"`
	Errors    int              `json:"errors"`
	Timestamp string           `json:"timestamp"`
	Time      string           `json:"time"`
	Name      string           `json:"name"`
	Groups    []TestsuiteGroup `json:"testsuites"`
}

// TestsuiteGroup is a group of test suites in the JSON --gtest_output file .
type TestsuiteGroup struct {
	Name       string      `json:"name"`
	Tests      int         `json:"tests"`
	Failures   int         `json:"failures"`
	Disabled   int         `json:"disabled"`
	Errors     int         `json:"errors"`
	Timestamp  string      `json:"timestamp"`
	Time       string      `json:"time"`
	Testsuites []Testsuite `json:"testsuite"`
}

// Testsuite is a suite of tests in the JSON --gtest_output file.
type Testsuite struct {
	Name       string    `json:"name"`
	ValueParam string    `json:"value_param,omitempty"`
	Status     Status    `json:"status"`
	Result     Result    `json:"result"`
	Timestamp  string    `json:"timestamp"`
	Time       string    `json:"time"`
	Classname  string    `json:"classname"`
	Failures   []Failure `json:"failures,omitempty"`
}

// Failure is a reported test failure in the JSON --gtest_output file.
type Failure struct {
	Failure string `json:"failure"`
	Type    string `json:"type"`
}

// Status is a status code in the JSON --gtest_output file.
type Status string

// Result is a result code in the JSON --gtest_output file.
type Result string
