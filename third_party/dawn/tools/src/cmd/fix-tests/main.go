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
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/substr"
)

func main() {
	if err := run(oswrapper.GetRealOSWrapper()); err != nil {
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

// TODO(crbug.com/416755658): Add unittest coverage once exec is handled via
// dependency injection.
func run(osWrapper oswrapper.OSWrapper) error {
	flag.Parse()
	args := flag.Args()
	if len(args) < 1 {
		showUsage()
	}

	exe := args[0]          // The path to the test executable
	wd := filepath.Dir(exe) // The directory holding the test exe

	// Create a temporary directory to hold the 'test-results.json' file
	tmpDir, err := osWrapper.MkdirTemp("", "fix-tests")
	if err != nil {
		return err
	}
	if err := osWrapper.MkdirAll(tmpDir, 0666); err != nil {
		return fmt.Errorf("Failed to create temporary directory: %w", err)
	}
	defer osWrapper.RemoveAll(tmpDir)

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
	testResultsFile, err := osWrapper.Open(testResultsPath)
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

				if err := processFailure(test, wd, failure.Failure, osWrapper); err != nil {
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

// longestSubstringMatch finds the longest matching substring of searchStr in docString and
// returns the start/end indices
func longestSubstringMatch(searchStr string, docString string) MatchRange {
	// Brute force algorithm is n*m for string sizes n and m
	bestIdxStart := 0
	bestIdxEnd := 0 // exclusive
	// Scan the document with the search string from the highest offset to the lowest. This will be out of bound for search string but that is because we are looking of a substring.
	for docOffset := -len(searchStr); docOffset < len(docString)+len(searchStr); docOffset++ {
		currIdxStart := 0
		currIdxEnd := 0 // exclusive
		for searchIdx := 0; searchIdx < len(searchStr); searchIdx++ {
			docOffset := searchIdx + docOffset
			isMatch := false

			// basic range checking for the doc
			if docOffset >= 0 && docOffset < len(docString) {
				if searchStr[searchIdx] == docString[docOffset] {
					isMatch = true
				}
			}

			if isMatch {
				if currIdxEnd == 0 {
					// first time matching
					currIdxStart = searchIdx
					currIdxEnd = currIdxStart + 1 // exclusive
				} else {
					// continue current matching.
					currIdxEnd++
				}
			}

			// check if our match is the best
			bestSize := bestIdxEnd - bestIdxStart
			currSize := currIdxEnd - currIdxStart

			if bestSize < currSize {
				bestIdxStart = currIdxStart
				bestIdxEnd = currIdxEnd
			}

			if !isMatch {
				// reset
				currIdxStart = 0
				currIdxEnd = 0
			}
		}
	}

	return MatchRange{bestIdxStart, bestIdxEnd}
}

func processFailure(test, wd, failure string, osWrapper oswrapper.OSWrapper) error {
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
			// We don't know if a or b is the expected. We also don't know if it should be escaped for R"(...)" strings
			aEsc, bEsc := escape(a), escape(b)

			// Find the longest match. We have (unfortunately) 4 options.
			mrA := longestSubstringMatch(a, testSource)
			mrB := longestSubstringMatch(b, testSource)
			mrAEsc := longestSubstringMatch(aEsc, testSource)
			mrBEsc := longestSubstringMatch(bEsc, testSource)

			// Perfect matches are prioritized for the cases where there may be more than matchable string in test function.
			// This is common in tint where there is both the 'src' and 'expect' strings of non-failing transform test.
			var aPerfect = len(a) == Size(mrA)
			var bPerfect = len(b) == Size(mrB)
			var aEscPerfect = len(aEsc) == Size(mrAEsc)
			var bEscPerfect = len(bEsc) == Size(mrBEsc)
			var hasPerfect = aPerfect || bPerfect || aEscPerfect || bEscPerfect

			useLargest := func(mr MatchRange) bool {
				return Size(mr) >= Size(mrA) && Size(mr) >= Size(mrB) &&
					Size(mr) >= Size(mrAEsc) && Size(mr) >= Size(mrBEsc) && !hasPerfect
			}

			// assumed mr_b_esc is best match
			expectedStr := bEsc
			replaceStr := aEsc
			mrLargest := mrBEsc

			if useLargest(mrA) || aPerfect {
				expectedStr = a
				replaceStr = b
				mrLargest = mrA
			} else if useLargest(mrB) || bPerfect {
				expectedStr = b
				replaceStr = a
				mrLargest = mrB
			} else if useLargest(mrAEsc) || aEscPerfect {
				expectedStr = aEsc
				replaceStr = bEsc
				mrLargest = mrAEsc
			}

			// trim away the number of unmatched characters from the end of expected to the end of the replacement.
			replaceStrEnd := len(replaceStr) - (len(expectedStr) - mrLargest.end)
			if replaceStrEnd >= mrLargest.start && replaceStrEnd <= len(replaceStr) {
				replaceStr = replaceStr[mrLargest.start:replaceStrEnd]
				expectedStr = expectedStr[mrLargest.start:mrLargest.end]
			} else {
				// It is not safe to attempt a replacement if the replacement string would have negative (nonsense) size.
				expectedStr = ""
			}

			// Do not try to replace on empty strings.
			if len(expectedStr) <= 0 {
				return "", fmt.Errorf("could not fix 'EXPECT_EQ' pattern in '%v'\n\nA: '%v'\n\nB: '%v'", file, a, b)
			}

			testSource = strings.ReplaceAll(testSource, expectedStr, replaceStr)
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
	sourceFile, err := parseSourceFile(sourcePath, osWrapper)
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
	return writeSourceFile(sourcePath, sourceFile, osWrapper)
}

// parseSourceFile() reads the file at path, splitting the content into chunks
// for each TEST.
func parseSourceFile(path string, fsReader oswrapper.FilesystemReader) (sourceFile, error) {
	fileBytes, err := fsReader.ReadFile(path)
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
func writeSourceFile(path string, file sourceFile, fsWriter oswrapper.FilesystemWriter) error {
	body := strings.Join(file.parts, "")
	return fsWriter.WriteFile(path, []byte(body), 0666)
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
