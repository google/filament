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

// This tool parses WGSL specification and outputs WGSL rules.
//
// To run from root of tint repo:
//   go get golang.org/x/net/html # Only required once
// Then run
//   ./tools/get-test-plan --spec=<path-to-spec-file-or-url> --output=<path-to-output-file>
// Or run
//   cd tools/src &&  go run cmd/get-spec-rules/main.go --output=<path-to-output-file>
//
// To see help
//   ./tools/get-test-plan --help

package main

import (
	"crypto/sha1"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"golang.org/x/net/html"
)

const (
	toolName        = "get-test-plan"
	specPath        = "https://www.w3.org/TR/WGSL/"
	specVersionUsed = "https://www.w3.org/TR/2021/WD-WGSL-20210929/"
)

var (
	errInvalidArg  = errors.New("invalid arguments")
	headURL        = specVersionUsed
	markedNodesSet = make(map[*html.Node]bool)
	testNamesSet   = make(map[string]bool)
	sha1sSet       = make(map[string]bool)
	keywords       = []string{
		"MUST ", "MUST NOT ", "REQUIRED ", "SHALL ",
		"SHALL NOT ", "SHOULD ", "SHOULD NOT ",
		"RECOMMENDED ", "MAY ", "OPTIONAL ",
	}
	globalSection      = ""
	globalPrevSectionX = -1
	globalRuleCounter  = 0
)

// Holds all the information about a WGSL rule
type rule struct {
	Number      int    // The index of this obj in an array of 'rules'
	Section     int    // The section this rule belongs to
	SubSection  string // The section this rule belongs to
	URL         string // The section's URL of this rule
	Description string // The rule's description
	TestName    string // The suggested test name to use when writing CTS
	Keyword     string // The keyword e.g. MUST, ALGORITHM, ..., i.e. Indicating why the rule is added
	Desc        []string
	Sha         string
}

func main() {
	flag.Usage = func() {
		out := flag.CommandLine.Output()
		fmt.Fprintf(out, "%v parses WGSL spec and outputs a test plan\n", toolName)
		fmt.Fprintf(out, "\n")
		fmt.Fprintf(out, "Usage:\n")
		fmt.Fprintf(out, "  %s [spec] [flags]\n", toolName)
		fmt.Fprintf(out, "\n")
		fmt.Fprintf(out, "spec is an optional local file or a URL to the WGSL specification.\n")
		fmt.Fprintf(out, "If spec is omitted then the specification is fetched from %v\n\n", specPath)

		fmt.Fprintf(out, "this tools is developed based on: %v\n", specVersionUsed)
		fmt.Fprintf(out, "flags may be any combination of:\n")
		flag.PrintDefaults()
	}

	err := run()
	switch err {
	case nil:
		return
	case errInvalidArg:
		fmt.Fprintf(os.Stderr, "Error: %v\n\n", err)
		flag.Usage()
	default:
		fmt.Fprintf(os.Stderr, "%v\n", err)
	}
	os.Exit(1)
}

func run() error {
	// Parse flags
	keyword := flag.String("keyword", "",
		`if provided, it will be used as the keyword to search WGSL spec for rules
if omitted, the keywords indicated in RFC 2119 requirement are used,
in addition to nodes containing a nowrap or an algorithm tag eg. <tr algorithm=...>`)

	ctsDir := flag.String("cts-directory", "",
		`if provided:
    validation cts test plan will be written to: '<cts-directory>/validation/'
    builtin functions cts test plan will be written to: '<cts-directory>/execution/builtin'`)

	output := flag.String("output", "",
		`if file extension is 'txt' the output format will be a human readable text
if file extension is 'tsv' the output format will be a tab separated file
if file extension is 'json' the output format will be json
if omitted, a human readable version of the rules is written to stdout`)

	testNameFilter := flag.String("test-name-filter", "",
		`if provided will be used to filter reported rules based on if their name
contains the provided string`)

	flag.Parse()

	args := flag.Args()

	// Parse spec
	spec, err := parseSpec(args)
	if err != nil {
		return err
	}

	// Set keywords
	if *keyword != "" {
		keywords = []string{*keyword}
	}

	parser, err := Parse(spec)
	if err != nil {
		return err
	}
	rules := parser.rules

	if *ctsDir != "" {
		err := getUnimplementedTestPlan(*parser, *ctsDir)
		if err != nil {
			return err
		}
	}

	txt, tsv := concatRules(rules, *testNameFilter)
	// if no output then write rules to stdout
	if *output == "" {
		fmt.Println(txt)
		// write concatenated rules to file
	} else if strings.HasSuffix(*output, ".json") {
		j, err := json.Marshal(rules)
		if err != nil {
			return err
		}
		return writeFile(*output, string(j))
	} else if strings.HasSuffix(*output, ".txt") {
		return writeFile(*output, txt)
	} else if strings.HasSuffix(*output, ".tsv") {
		return writeFile(*output, tsv)
	} else {
		return fmt.Errorf("unsupported output file extension: %v", *output)
	}
	return nil
}

// getSectionRange scans all the rules and returns the rule index interval of a given section.
// The sections range is the interval: rules[start:end].
// example: section = [x, y, z] i.e. x.y.z(.w)* it returns (start = min(w),end = max(w))
// if there are no rules extracted from x.y.z it returns (-1, -1)
func getSectionRange(rules []rule, s []int) (start, end int, err error) {
	start = -1
	end = -1
	for _, r := range rules {
		sectionDims, err := parseSection(r.SubSection)
		if err != nil {
			return -1, -1, err
		}

		ruleIsInSection := true
		for i := range s {
			if sectionDims[i] != s[i] {
				ruleIsInSection = false
				break
			}
		}
		if !ruleIsInSection {
			continue
		}

		dim := -1
		if len(sectionDims) == len(s) {
			//x.y is the same as x.y.0
			dim = 0
		} else if len(sectionDims) > len(s) {
			dim = sectionDims[len(s)]
		} else {
			continue
		}

		if start == -1 {
			start = dim
		}
		if dim > end {
			end = dim
		}
	}

	if start == -1 || end == -1 {
		return -1, -1, fmt.Errorf("cannot determine section range")
	}

	return start, end, nil
}

// parseSection return the numbers for any dot-separated string of numbers
// example: x.y.z.w returns [x, y, z, w]
// returns an error if the string does not match "^\d(.\d)*$"
func parseSection(in string) ([]int, error) {
	parts := strings.Split(in, ".")
	out := make([]int, len(parts))
	for i, part := range parts {
		var err error
		out[i], err = strconv.Atoi(part)
		if err != nil {
			return nil, fmt.Errorf(`cannot parse sections string "%v": %w`, in, err)
		}
	}
	return out, nil
}

// concatRules concatenate rules slice to make two string outputs;
//
//	txt, a human-readable string
//	tsv, a tab separated string
//
// If testNameFilter is a non-empty string, then only rules whose TestName
// contains the string are included
func concatRules(rules []rule, testNameFilter string) (string, string) {
	txtLines := []string{}
	tsvLines := []string{"Number\tUniqueId\tSection\tURL\tDescription\tProposed Test Name\tkeyword"}

	for _, r := range rules {
		if testNameFilter != "" && !strings.Contains(r.TestName, testNameFilter) {
			continue
		}

		txtLines = append(txtLines, strings.Join([]string{
			"Rule Number " + strconv.Itoa(r.Number) + ":",
			"Unique Id: " + r.Sha,
			"Section: " + r.SubSection,
			"Keyword: " + r.Keyword,
			"testName: " + r.TestName,
			"URL: " + r.URL,
			r.Description,
			"---------------------------------------------------"}, "\n"))

		tsvLines = append(tsvLines, strings.Join([]string{
			strconv.Itoa(r.Number),
			r.Sha,
			r.SubSection,
			r.URL,
			strings.Trim(r.Description, "\n\t "),
			r.Keyword,
			r.TestName}, "\t"))
	}
	txt := strings.Join(txtLines, "\n")
	tsv := strings.Join(tsvLines, "\n")
	return txt, tsv
}

// writeFile writes content to path
// the existing content will be overwritten
func writeFile(path, content string) error {
	if err := os.MkdirAll(filepath.Dir(path), 0777); err != nil {
		return fmt.Errorf("failed to create directory for '%v': %w", path, err)
	}
	if err := ioutil.WriteFile(path, []byte(content), 0666); err != nil {
		return fmt.Errorf("failed to write file '%v': %w", path, err)
	}
	return nil
}

// parseSpec reads the spec from a local file, or the URL to WGSL spec
func parseSpec(args []string) (*html.Node, error) {
	// Check for explicit WGSL spec path
	specURL, _ := url.Parse(specPath)
	switch len(args) {
	case 0:
	case 1:
		var err error
		specURL, err = url.Parse(args[0])
		if err != nil {
			return nil, err
		}
	default:
		if len(args) > 1 {
			return nil, errInvalidArg
		}
	}

	// The specURL might just be a local file path, in which case automatically
	// add the 'file' URL scheme
	if specURL.Scheme == "" {
		specURL.Scheme = "file"
	}

	// Open the spec from HTTP(S) or from a local file
	var specContent io.ReadCloser
	switch specURL.Scheme {
	case "http", "https":
		response, err := http.Get(specURL.String())
		if err != nil {
			return nil, fmt.Errorf("failed to load the WGSL spec from '%v': %w", specURL, err)
		}
		specContent = response.Body
	case "file":
		path, err := filepath.Abs(specURL.Path)
		if err != nil {
			return nil, fmt.Errorf("failed to load the WGSL spec from '%v': %w", specURL, err)
		}

		file, err := os.Open(path)
		if err != nil {
			return nil, fmt.Errorf("failed to load the WGSL spec from '%v': %w", specURL, err)
		}
		specContent = file
	default:
		return nil, fmt.Errorf("unsupported URL scheme: %v", specURL.Scheme)
	}
	defer specContent.Close()

	// Open the spec from HTTP(S) or from a local file
	switch specURL.Scheme {
	case "http", "https":
		response, err := http.Get(specURL.String())
		if err != nil {
			return nil, fmt.Errorf("failed to load the WGSL spec from '%v': %w", specURL, err)
		}
		specContent = response.Body

	case "file":
		path, err := filepath.Abs(specURL.Path)
		if err != nil {
			return nil, fmt.Errorf("failed to load the WGSL spec from '%v': %w", specURL, err)
		}
		file, err := os.Open(path)
		if err != nil {
			return nil, fmt.Errorf("failed to load the WGSL spec from '%v': %w", specURL, err)
		}
		specContent = file

	default:
		return nil, fmt.Errorf("unsupported URL scheme: %v", specURL.Scheme)
	}
	defer specContent.Close()

	// Parse spec
	spec, err := html.Parse(specContent)
	if err != nil {
		return spec, err
	}
	return spec, nil
}

// containsKeyword returns (true, 'kw'), if input string 'data' contains an
// element of the string list, otherwise it returns (false, "")
// search is not case-sensitive
func containsKeyword(data string, list []string) (bool, string) {
	for _, kw := range list {
		if strings.Contains(
			strings.ToLower(data),
			strings.ToLower(kw),
		) {
			return true, kw
		}
	}
	return false, ""
}

// Parser holds the information extracted from the spec
// TODO(sarahM0): https://bugs.c/tint/1149/ clean up the vars holding section information
type Parser struct {
	rules                      []rule // a slice to store the rules extracted from the spec
	firstSectionContainingRule int    // the first section a rules is extracted from
	lastSectionContainingRule  int    // the last section a rules is extracted form
}

func Parse(node *html.Node) (*Parser, error) {
	var p *Parser = new(Parser)
	p.firstSectionContainingRule = -1
	p.lastSectionContainingRule = -1
	return p, p.getRules(node)
}

// getRules populates the rule slice by scanning HTML node and its children
func (p *Parser) getRules(node *html.Node) error {
	section, subSection, err := getSectionInfo(node)

	if err != nil {
		// skip this node and move on to its children
	} else {
		// Do not generate rules for introductory sections
		if section > 2 {
			// Check if this node is visited before. This is necessary since
			// sometimes to create rule description we visit siblings or children
			if marked := markedNodesSet[node]; marked {
				return nil
			}

			// update parser's section info
			if p.firstSectionContainingRule == -1 {
				p.firstSectionContainingRule = section
			}
			p.lastSectionContainingRule = section

			// extract rules from the node
			if err := p.getAlgorithmRule(node, section, subSection); err != nil {
				return err
			}
			if err := p.getNowrapRule(node, section, subSection); err != nil {
				return err
			}
			if err := p.getKeywordRule(node, section, subSection); err != nil {
				return err
			}
		}
	}

	for child := node.FirstChild; child != nil; child = child.NextSibling {
		if err := p.getRules(child); err != nil {
			return err
		}
	}
	return nil
}

// gatherKeywordRules scans the HTML node data, adds a new rules if it contains one
// of the keywords
func (p *Parser) getKeywordRule(node *html.Node, section int, subSection string) error {
	if node.Type != html.TextNode {
		return nil
	}

	hasKeyword, keyword := containsKeyword(node.Data, keywords)
	if !hasKeyword {
		return nil
	}

	// TODO(sarah): create a list of rule.sha1 for unwanted rules
	if strings.HasPrefix(node.Data, "/*") ||
		strings.Contains(node.Data, "reference must load and store from the same") ||
		strings.Contains(node.Data, " to an invalid reference may either: ") ||
		// Do not add Issues
		strings.Contains(node.Data, "Issue: ") ||
		strings.Contains(node.Data, "WebGPU issue") ||
		strings.Contains(node.Data, "/issues/") {
		return nil
	}

	id := getID(node)
	desc := cleanUpString(getNodeData(node))

	t, _, err := testName(id, desc, subSection)
	if err != nil {
		return err
	}

	sha, err := getSha1(desc, id)
	if err != nil {
		return err
	}

	r := rule{
		Sha:         sha,
		Number:      len(p.rules) + 1,
		Section:     section,
		SubSection:  subSection,
		URL:         headURL + "#" + id,
		Description: desc,
		TestName:    t,
		Keyword:     keyword,
	}
	p.rules = append(p.rules, r)

	return nil
}

// getNodeData builds the rule's description from the HTML node's data and all of its siblings.
// the node data is a usually a partial sentence, build the description from the node's data and
// all it's siblings to get a full context of the rule.
func getNodeData(node *html.Node) string {
	sb := strings.Builder{}
	if node.Parent != nil {
		for n := node.Parent.FirstChild; n != nil; n = n.NextSibling {
			printNodeText(n, &sb)
		}
	} else {
		printNodeText(node, &sb)
	}
	return sb.String()
}

// getAlgorithmRules scans the HTML node for blocks that
// contain an 'algorithm' class, populating the rule slice.
// i.e. <tr algorithm=...> and <p algorithm=...>
func (p *Parser) getAlgorithmRule(node *html.Node, section int, subSection string) error {
	if !hasClass(node, "algorithm") {
		return nil
	}
	// mark this node as seen
	markedNodesSet[node] = true

	sb := strings.Builder{}
	printNodeText(node, &sb)
	title := cleanUpStartEnd(getNodeAttrValue(node, "data-algorithm"))
	desc := title + ":\n" + cleanUpString(sb.String())
	id := getID(node)
	testName, _, err := testName(id, desc, subSection)
	if err != nil {
		return err
	}

	sha, err := getSha1(desc, id)
	if err != nil {
		return err
	}

	r := rule{
		Sha:         sha,
		Number:      len(p.rules) + 1,
		Section:     section,
		SubSection:  subSection,
		URL:         headURL + "#" + id,
		Description: desc,
		TestName:    testName,
		Keyword:     "ALGORITHM",
	}
	p.rules = append(p.rules, r)
	return nil
}

// getNowrapRules scans the HTML node for blocks that contain a
// 'nowrap' class , populating the rule slice.
// ie. <td class="nowrap">
// TODO(https://crbug.com/tint/1157)
// remove this when https://github.com/gpuweb/gpuweb/pull/2084 is closed
// and make sure Derivative built-in functions are added to the rules
func (p *Parser) getNowrapRule(node *html.Node, section int, subSection string) error {
	if !hasClass(node, "nowrap") {
		return nil
	}
	// mark this node as seen
	markedNodesSet[node] = true
	desc := cleanUpStartEnd(getNodeData(node))
	id := getID(node)

	t, _, err := testName(id, desc, subSection)
	if err != nil {
		return err
	}

	sha, err := getSha1(desc, id)
	if err != nil {
		return err
	}

	r := rule{
		Sha:         sha,
		Number:      len(p.rules) + 1,
		SubSection:  subSection,
		Section:     section,
		URL:         headURL + "#" + id,
		Description: desc,
		TestName:    t,
		Keyword:     "Nowrap",
	}
	p.rules = append(p.rules, r)

	return nil
}

// hasClass returns true if node is has the given "class" attribute.
func hasClass(node *html.Node, class string) bool {
	for _, attr := range node.Attr {
		if attr.Key == "class" {
			classes := strings.Split(attr.Val, " ")
			for _, c := range classes {
				if c == class {
					return true
				}
			}
		}
	}
	return false
}

// getSectionInfo returns the section this node belongs to
func getSectionInfo(node *html.Node) (int, string, error) {
	sub := getNodeAttrValue(node, "data-level")
	for p := node; sub == "" && p != nil; p = p.Parent {
		sub = getSiblingSectionInfo(p)
	}
	// when there is and ISSUE in HTML section cannot be set
	// use the previously set section
	if sub == "" && globalSection == "" {
		// for the section Abstract no section can be found
		// return -1 to skip this node
		return -1, "", fmt.Errorf("cannot get section info")
	}
	if sub == "" {
		sub = globalSection
	}
	globalSection = sub
	sectionDims, err := parseSection(sub)
	if len(sectionDims) > -1 {
		return sectionDims[0], sub, err
	}
	return -1, sub, err
}

// getSection return the section of this node's sibling
// iterates over all siblings and return the first one it can determine
func getSiblingSectionInfo(node *html.Node) string {
	for sp := node.PrevSibling; sp != nil; sp = sp.PrevSibling {
		section := getNodeAttrValue(sp, "data-level")
		if section != "" {
			return section
		}
	}
	return ""
}

// GetSiblingSectionInfo determines if the node's id refers to an example
func isExampleNode(node *html.Node) string {
	for sp := node.PrevSibling; sp != nil; sp = sp.PrevSibling {
		id := getNodeAttrValue(sp, "id")
		if id != "" && !strings.Contains(id, "example-") {
			return id
		}
	}
	return ""
}

// getID returns the id of the section this node belongs to
func getID(node *html.Node) string {
	id := getNodeAttrValue(node, "id")
	for p := node; id == "" && p != nil; p = p.Parent {
		id = isExampleNode(p)
	}
	return id
}

var (
	reCleanUpString       = regexp.MustCompile(`\n(\n|\s|\t)+|(\s|\t)+\n`)
	reSpacePlusTwo        = regexp.MustCompile(`\t|\s{2,}`)
	reBeginOrEndWithSpace = regexp.MustCompile(`^\s|\s$`)
	reIrregularWhiteSpace = regexp.MustCompile(`§.`)
)

// cleanUpString creates a string by removing  all extra spaces, newlines and tabs
// form input string 'in' and returns it
// This is done so that the uniqueID does not change because of a change in white spaces
//
// example in:
// ` float abs:
// T is f32 or vecN<f32>
//
//			abs(e: T ) -> 	T
//		  Returns the absolute value of e (e.g. e  with a positive sign bit). Component-wise when T is a vector.
//	  (GLSLstd450Fabs)`
//
// example out:
// `float abs:
// T is f32 or vecN<f32> abs(e: T ) -> T Returns the absolute value of e (e.g. e with a positive sign bit). Component-wise when T is a vector. (GLSLstd450Fabs)`
func cleanUpString(in string) string {
	out := reCleanUpString.ReplaceAllString(in, " ")
	out = reSpacePlusTwo.ReplaceAllString(out, " ")
	//`§.` is not a valid character for a cts description
	// ie. this is invalid: g.test().desc(`§.`)
	out = reIrregularWhiteSpace.ReplaceAllString(out, "section ")
	out = reBeginOrEndWithSpace.ReplaceAllString(out, "")
	return out
}

var (
	reCleanUpStartEnd = regexp.MustCompile(`^\s+|\s+$|^\t+|\t+$|^\n+|\n+$`)
)

// cleanUpStartEnd creates a string by removing all extra spaces,
// newlines and tabs form the start and end of the input string.
// Example:
//
//		input: "\s\t\nHello\s\n\t\Bye\s\s\s\t\n\n\n"
//	 output: "Hello\s\n\tBye"
//	 input2: "\nbye\n\n"
//	 output2: "\nbye"
func cleanUpStartEnd(in string) string {
	out := reCleanUpStartEnd.ReplaceAllString(in, "")
	return out
}

var (
	name         = "^[a-zA-Z0-9_]+$"
	reName       = regexp.MustCompile(`[^a-zA-Z0-9_]`)
	reUnderScore = regexp.MustCompile(`[_]+`)
	reDoNotBegin = regexp.MustCompile(`^[0-9_]+|[_]$`)
)

// testName creates a test name given a rule id (ie. section name), description and section
// returns for a builtin rule:
//
//	testName:${section name} + "," + ${builtin name}
//	builtinName: ${builtin name}
//	err: nil
//
// returns for a other rules:
//
//	testName: ${section name} + "_rule_ + " + ${string(counter)}
//	builtinName: ""
//	err: nil
//
// if it cannot create a unique name it returns "", "", err.
func testName(id string, desc string, section string) (testName, builtinName string, err error) {
	// regex for every thing other than letters and numbers
	if desc == "" || section == "" || id == "" {
		return "", "", fmt.Errorf("cannot generate test name")
	}
	// avoid any characters other than letters, numbers and underscore
	id = reName.ReplaceAllString(id, "_")
	// avoid underscore repeats
	id = reUnderScore.ReplaceAllString(id, "_")
	// test name must not start with underscore or a number
	// nor end with and underscore
	id = reDoNotBegin.ReplaceAllString(id, "")

	sectionX, err := parseSection(section)
	if err != nil {
		return "", "", err
	}

	builtinName = ""
	index := strings.Index(desc, ":")
	if strings.Contains(id, "builtin_functions") && index > -1 {
		builtinName = reName.ReplaceAllString(desc[:index], "_")
		builtinName = reDoNotBegin.ReplaceAllString(builtinName, "")
		builtinName = reUnderScore.ReplaceAllString(builtinName, "_")
		match, _ := regexp.MatchString(name, builtinName)
		if match {
			testName = id + "," + builtinName
			// in case there is more than one builtin functions
			// with the same name in one section:
			// "id,builtin", "id,builtin2", "id,builtin3", ...
			for i := 2; testNamesSet[testName]; i++ {
				testName = id + "," + builtinName + strconv.Itoa(i)
			}
			testNamesSet[testName] = true
			return testName, builtinName, nil
		}

	}

	if sectionX[0] == globalPrevSectionX {
		globalRuleCounter++
	} else {
		globalRuleCounter = 0
		globalPrevSectionX = sectionX[0]
	}
	testName = id + ",rule" + strconv.Itoa(globalRuleCounter)
	if testNamesSet[testName] {
		testName = "error-unable-to-generate-unique-file-name"
		return testName, "", fmt.Errorf("unable to generate unique test name\n%s", desc)
	}
	testNamesSet[testName] = true
	return testName, "", nil
}

// printNodeText traverses node and its children, writing the Data of all TextNodes to sb.
func printNodeText(node *html.Node, sb *strings.Builder) {
	// mark this node as seen
	markedNodesSet[node] = true
	if node.Type == html.TextNode {
		sb.WriteString(node.Data)
	}

	for child := node.FirstChild; child != nil; child = child.NextSibling {
		printNodeText(child, sb)
	}
}

// getNodeAttrValue scans attributes of 'node' and returns the value of attribute 'key'
// or an empty string if 'node' doesn't have an attribute 'key'
func getNodeAttrValue(node *html.Node, key string) string {
	for _, attr := range node.Attr {
		if attr.Key == key {
			return attr.Val
		}
	}
	return ""
}

// getSha1 returns the first 8 byte of sha1(a+b)
func getSha1(a string, b string) (string, error) {
	sum := sha1.Sum([]byte(a + b))
	sha := fmt.Sprintf("%x", sum[0:8])
	if sha1sSet[sha] {
		return "", fmt.Errorf("sha1 is not unique")
	}
	sha1sSet[sha] = true
	return sha, nil
}

// getUnimplementedPlan generate the typescript code of a test plan for rules in sections[start, end]
// then it writes the generated test plans in the given 'path'
func getUnimplementedTestPlan(p Parser, path string) error {
	rules := p.rules
	start := p.firstSectionContainingRule
	end := p.lastSectionContainingRule
	validationPath := filepath.Join(path, "validation")
	if err := validationTestPlan(rules, validationPath, start, end); err != nil {
		return err
	}

	executionPath := filepath.Join(path, "execution", "builtin")
	if err := executionTestPlan(rules, executionPath); err != nil {
		return err
	}
	return nil
}

// getTestPlanFilePath returns a sort friendly path
// example: if we have 10 sections, and generate filenames naively, this will be the sorted result:
//
//		section1.spec.ts -> section10.spec.ts -> section2.spec.ts -> ...
//	    if we make all the section numbers have the same number of digits, we will get:
//		section01.spec.ts -> section02.spec.ts -> ... -> section10.spec.ts
func getTestPlanFilePath(path string, x, y, digits int) (string, error) {
	fileName := ""
	if y != -1 {
		// section16.01.spec.ts, ...
		sectionFmt := fmt.Sprintf("section%%d_%%.%dd.spec.ts", digits)
		fileName = fmt.Sprintf(sectionFmt, x, y)
	} else {
		// section01.spec.ts, ...
		sectionFmt := fmt.Sprintf("section%%.%dd.spec.ts", digits)
		fileName = fmt.Sprintf(sectionFmt, x)
	}
	return filepath.Join(path, fileName), nil

}

// validationTestPlan generates the typescript code of a test plan for rules in sections[start, end]
func validationTestPlan(rules []rule, path string, start int, end int) error {
	content := [][]string{}
	filePath := []string{}
	for section := 0; section <= end; section++ {
		sb := strings.Builder{}
		sectionStr := strconv.Itoa(section)
		testDescription := "`WGSL Section " + sectionStr + " Test Plan`"
		sb.WriteString(fmt.Sprintf(validationTestHeader, testDescription))
		content = append(content, []string{sb.String()})
		f, err := getTestPlanFilePath(path, section, -1, len(strconv.Itoa(end)))
		if err != nil {
			return nil
		}
		filePath = append(filePath, f)
	}

	for _, r := range rules {
		sectionDims, err := parseSection(r.SubSection)
		if err != nil || len(sectionDims) == 0 {
			return err
		}
		section := sectionDims[0]
		if section < start || section >= end {
			continue
		}
		content[section] = append(content[section], testPlan(r))
	}

	for i := start; i <= end; i++ {
		if len(content[i]) > 1 {
			if err := writeFile(filePath[i], strings.Join(content[i], "\n")); err != nil {
				return err
			}
		}
	}
	return nil

}

// executionTestPlan generates the typescript code of a test plan for rules in the given section
// the rules in section X.Y.* will be written to path/sectionX_Y.spec.ts
func executionTestPlan(rules []rule, path string) error {
	// TODO(SarahM) This generates execution tests for builtin function tests. Add other executions tests.
	section, err := getBuiltinSectionNum(rules)
	if err != nil {
		return err
	}

	content := [][]string{}
	filePath := []string{}

	start, end, err := getSectionRange(rules, []int{section})
	if err != nil || start == -1 || end == -1 {
		return err
	}
	for y := 0; y <= end; y++ {
		fileName, err := getTestPlanFilePath(path, section, y, len(strconv.Itoa(end)))
		if err != nil {
			return err
		}
		filePath = append(filePath, fileName)

		sb := strings.Builder{}
		testDescription := fmt.Sprintf("`WGSL section %v.%v execution test`", section, y)
		sb.WriteString(fmt.Sprintf(executionTestHeader, testDescription))
		content = append(content, []string{sb.String()})
	}

	for _, r := range rules {
		if r.Section != section || !isBuiltinFunctionRule(r) {
			continue
		}

		index := -1
		sectionDims, err := parseSection(r.SubSection)
		if err != nil || len(sectionDims) == 0 {
			return err
		}

		if len(sectionDims) == 1 {
			// section = x
			index = 0
		} else {
			// section = x.y(.z)*
			index = sectionDims[1]
		}

		if index < 0 && index >= len(content) {
			return fmt.Errorf("cannot append to content, index %v out of range 0..%v",
				index, len(content)-1)
		}
		content[index] = append(content[index], testPlan(r))
	}

	for i := start; i <= end; i++ {
		// Write the file if there is a test in there
		// compared with >1 because content has at least the test description
		if len(content[i]) > 1 {
			if err := writeFile(filePath[i], strings.Join(content[i], "\n")); err != nil {
				return err
			}
		}
	}
	return nil
}

func getBuiltinSectionNum(rules []rule) (int, error) {
	for _, r := range rules {
		if strings.Contains(r.URL, "builtin-functions") {
			return r.Section, nil
		}
	}
	return -1, fmt.Errorf("unable to find the built-in function section")
}

func isBuiltinFunctionRule(r rule) bool {
	_, builtinName, _ := testName(r.URL, r.Description, r.SubSection)
	return builtinName != "" || strings.Contains(r.URL, "builtin-functions")
}

func testPlan(r rule) string {
	sb := strings.Builder{}
	sb.WriteString(fmt.Sprintf(unImplementedTestTemplate, r.TestName, r.Sha, r.URL,
		"`\n"+r.Description+"\n"+howToContribute+"\n`"))

	return sb.String()
}

const (
	validationTestHeader = `export const description = %v;

import { makeTestGroup } from '../../../common/framework/test_group.js';

import { ShaderValidationTest } from './shader_validation_test.js';

export const g = makeTestGroup(ShaderValidationTest);
`
	executionTestHeader = `export const description = %v;

import { makeTestGroup } from '../../../../common/framework/test_group.js';
import { GPUTest } from '../../../gpu_test.js';

export const g = makeTestGroup(GPUTest);
`
	unImplementedTestTemplate = `g.test('%v')
  .uniqueId('%v')
  .specURL('%v')
  .desc(
    %v
  )
  .params(u => u.combine('placeHolder1', ['placeHolder2', 'placeHolder3']))
  .unimplemented();
`
	howToContribute = `
Please read the following guidelines before contributing:
https://github.com/gpuweb/cts/blob/main/docs/plan_autogen.md`
)
