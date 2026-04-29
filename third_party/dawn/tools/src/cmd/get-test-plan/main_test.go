// Copyright 2026 The Dawn & Tint Authors
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

package main

import (
	"fmt"
	"os"
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
	"golang.org/x/net/html"
)

// NOTE: The code under test relies on global state (several variables prefixed
// with "global"), so these tests cannot be run in parallel.

// Helper to create HTML node from string
func createHTMLNode(t *testing.T, data string) *html.Node {
	node, err := html.Parse(strings.NewReader(data))
	require.NoError(t, err)
	return node
}

type mockOSWrapper struct {
	oswrapper.OSWrapper
	writeFileErr error
}

func (m *mockOSWrapper) WriteFile(name string, data []byte, perm os.FileMode) error {
	if m.writeFileErr != nil {
		return m.writeFileErr
	}
	return m.OSWrapper.WriteFile(name, data, perm)
}

func resetGlobals() {
	markedNodesSet = make(map[*html.Node]bool)
	testNamesSet = make(map[string]bool)
	sha1sSet = make(map[string]bool)
	globalSection = ""
	globalPrevSectionX = -1
	globalRuleCounter = 0
}

func TestGetSectionRange(t *testing.T) {
	rules := []rule{
		{SubSection: "1.0"},
		{SubSection: "1.1"},
		{SubSection: "2.0"},
		{SubSection: "1.1.1"},
		{SubSection: "1.2"},
	}

	start, end, err := getSectionRange(rules, []int{1})
	require.NoError(t, err)
	require.Equal(t, 0, start)
	require.Equal(t, 2, end)

	start, end, err = getSectionRange(rules, []int{1, 1})
	require.NoError(t, err)
	require.Equal(t, 0, start)
	require.Equal(t, 1, end)

	start, end, err = getSectionRange(rules, []int{3})
	require.ErrorContains(t, err, "cannot determine section range")
	require.Equal(t, -1, start)
	require.Equal(t, -1, end)

	start, end, err = getSectionRange([]rule{{SubSection: "invalid"}}, []int{1})
	require.ErrorContains(t, err, "cannot parse sections string")
	require.Equal(t, -1, start)
	require.Equal(t, -1, end)
}

func TestParseSection(t *testing.T) {
	res, err := parseSection("1.2.3")
	require.NoError(t, err)
	require.Equal(t, []int{1, 2, 3}, res)

	_, err = parseSection("1.a")
	require.ErrorContains(t, err, "cannot parse sections string")
}

func TestConcatRules(t *testing.T) {
	rules := []rule{
		{
			Number:      1,
			Sha:         "123",
			SubSection:  "1.0",
			URL:         "url",
			Description: "desc",
			TestName:    "test",
			Keyword:     "MUST",
		},
	}

	// Single.
	txt, tsv := concatRules(rules, "")
	expectedTxt := `Rule Number 1:
Unique Id: 123
Section: 1.0
Keyword: MUST
testName: test
URL: url
desc
---------------------------------------------------`
	expectedTsv := "Number\tUniqueId\tSection\tURL\tDescription\tProposed Test Name\tkeyword\n" +
		"1\t123\t1.0\turl\tdesc\tMUST\ttest"
	require.Equal(t, expectedTxt, txt)
	require.Equal(t, tsv, expectedTsv)

	// No matching rule.
	txt, tsv = concatRules(rules, "notfound")
	require.Equal(t, "", txt)
	require.Equal(t, "Number\tUniqueId\tSection\tURL\tDescription\tProposed Test Name\tkeyword", tsv)

	// Multiple rules with a filter that matches some.
	rulesWithFilter := []rule{
		{
			Number:      10,
			Sha:         "sha10",
			SubSection:  "10.0",
			URL:         "url10",
			Description: "first matching rule",
			TestName:    "matching-test",
			Keyword:     "MUST",
		},
		{
			Number:      11,
			Sha:         "sha11",
			SubSection:  "10.1",
			URL:         "url11",
			Description: "non-matching rule",
			TestName:    "other-test",
			Keyword:     "SHOULD",
		},
		{
			Number:      12,
			Sha:         "sha12",
			SubSection:  "10.2",
			URL:         "url12",
			Description: "second matching rule",
			TestName:    "matching-test-2",
			Keyword:     "MAY",
		},
	}

	txt, tsv = concatRules(rulesWithFilter, "matching-test")
	expectedTxt = `Rule Number 10:
Unique Id: sha10
Section: 10.0
Keyword: MUST
testName: matching-test
URL: url10
first matching rule
---------------------------------------------------
Rule Number 12:
Unique Id: sha12
Section: 10.2
Keyword: MAY
testName: matching-test-2
URL: url12
second matching rule
---------------------------------------------------`
	expectedTsv = "Number\tUniqueId\tSection\tURL\tDescription\tProposed Test Name\tkeyword\n" +
		"10\tsha10\t10.0\turl10\tfirst matching rule\tMUST\tmatching-test\n" +
		"12\tsha12\t10.2\turl12\tsecond matching rule\tMAY\tmatching-test-2"

	require.Equal(t, txt, expectedTxt)
	require.NotContains(t, txt, "non-matching rule")

	require.Equal(t, tsv, expectedTsv)
	require.NotContains(t, tsv, "non-matching rule")
}

func TestWriteFile(t *testing.T) {
	w := oswrapper.CreateFSTestOSWrapper()
	path := "/dir/file.txt"
	content := "hello"

	err := writeFile(path, content, w)
	require.NoError(t, err)

	readContent, err := w.ReadFile(path)
	require.NoError(t, err)
	require.Equal(t, content, string(readContent))

	// Test mkdir failure case (by making a file at dir location)
	err = w.WriteFile("/dir2", []byte("file"), 0666)
	require.NoError(t, err)
	err = writeFile("/dir2/file.txt", content, w)
	require.ErrorContains(t, err, "failed to create directory for")

	// Test WriteFile failure
	mockW := &mockOSWrapper{
		OSWrapper:    oswrapper.CreateFSTestOSWrapper(),
		writeFileErr: fmt.Errorf("write error"),
	}
	err = writeFile("/dir/fail.txt", content, mockW)
	require.ErrorContains(t, err, "failed to write file")
	require.ErrorContains(t, err, "write error")
}

func TestContainsKeyword(t *testing.T) {
	list := []string{"foo", "bar"}

	found, kw := containsKeyword("something FOO", list)
	require.True(t, found)
	require.Equal(t, "foo", kw)

	found, _ = containsKeyword("nothing", list)
	require.False(t, found)
}

func TestParseAndGetRules(t *testing.T) {
	resetGlobals()

	htmlStr := `
<div id="section3"></div>
<section data-level="3.0">
	<p>MUST do something</p>
	<p class="algorithm" data-algorithm="algo1">algo description</p>
	<table><tr><td class="nowrap">nowrap desc</td></tr></table>
</section>
`
	node := createHTMLNode(t, htmlStr)
	parser, err := Parse(node)
	require.NoError(t, err)

	require.Len(t, parser.rules, 3)

	var mustRule, algoRule, nowrapRule rule
	foundMust := false
	foundAlgo := false
	foundNowrap := false

	for _, r := range parser.rules {
		if strings.TrimSpace(r.Keyword) == "MUST" {
			mustRule = r
			foundMust = true
		} else if r.Keyword == "ALGORITHM" {
			algoRule = r
			foundAlgo = true
		} else if r.Keyword == "Nowrap" {
			nowrapRule = r
			foundNowrap = true
		}
	}

	require.True(t, foundMust, "MUST rule not found")
	require.True(t, foundAlgo, "ALGORITHM rule not found")
	require.True(t, foundNowrap, "Nowrap rule not found")

	require.Equal(t, "3.0", mustRule.SubSection)
	require.Equal(t, "algo1:\nalgo description", algoRule.Description)
	require.Equal(t, "nowrap desc", nowrapRule.Description)
}

func TestGetSectionInfo(t *testing.T) {
	resetGlobals()

	// Case 1: Node has data-level
	node1 := &html.Node{
		Type: html.ElementNode,
		Attr: []html.Attribute{{Key: "data-level", Val: "1.2"}},
	}
	sect, sub, err := getSectionInfo(node1)
	require.NoError(t, err)
	require.Equal(t, 1, sect)
	require.Equal(t, "1.2", sub)

	// Case 2: Parent has data-level
	node2 := &html.Node{Type: html.ElementNode}
	node1.AppendChild(node2)
	sect, sub, err = getSectionInfo(node2)
	require.NoError(t, err)
	require.Equal(t, 1, sect)
	require.Equal(t, "1.2", sub)

	// Case 3: Sibling has data-level
	parent := &html.Node{Type: html.ElementNode}
	sibling := &html.Node{
		Type: html.ElementNode,
		Attr: []html.Attribute{{Key: "data-level", Val: "2.0"}},
	}
	target := &html.Node{Type: html.ElementNode}
	parent.AppendChild(sibling)
	parent.AppendChild(target)

	sect, sub, err = getSectionInfo(target)
	require.NoError(t, err)
	require.Equal(t, 2, sect)
	require.Equal(t, "2.0", sub)

	// Case 4: Global fallback
	// globalSection is set by previous calls if they find something.
	// Reset it first.
	globalSection = "5.5"
	orphan := &html.Node{Type: html.ElementNode}
	sect, sub, err = getSectionInfo(orphan)
	require.NoError(t, err)
	require.Equal(t, 5, sect)
	require.Equal(t, "5.5", sub)
}

func TestGetID(t *testing.T) {
	resetGlobals()

	// Case 1: Node has ID
	idNode := &html.Node{
		Type: html.ElementNode,
		Attr: []html.Attribute{{Key: "id", Val: "my-id"}},
	}
	require.Equal(t, "my-id", getID(idNode))

	// Case 2: Node has previous sibling with ID
	prevSibling := &html.Node{
		Type: html.ElementNode,
		Attr: []html.Attribute{{Key: "id", Val: "sibling-id"}},
	}
	targetNode := &html.Node{Type: html.ElementNode}
	parentNode := &html.Node{Type: html.ElementNode}
	parentNode.AppendChild(prevSibling)
	parentNode.AppendChild(targetNode)

	require.Equal(t, "sibling-id", getID(targetNode))

	// Case 3: Ancestor has previous sibling with ID
	container := &html.Node{Type: html.ElementNode}
	ancSibling := &html.Node{
		Type: html.ElementNode,
		Attr: []html.Attribute{{Key: "id", Val: "ancestor-sibling-id"}},
	}
	child := &html.Node{Type: html.ElementNode}

	grandParent := &html.Node{Type: html.ElementNode}
	grandParent.AppendChild(ancSibling)
	grandParent.AppendChild(container)
	container.AppendChild(child)

	require.Equal(t, "ancestor-sibling-id", getID(child))
}

func TestCleanUpString(t *testing.T) {
	in := "  foo  \n  bar  "
	out := cleanUpString(in)
	require.Equal(t, "foo bar", out)

	require.Equal(t, "section 1", cleanUpString("§.1"))
}

func TestCleanUpStartEnd(t *testing.T) {
	require.Equal(t, "foo", cleanUpStartEnd("  foo  "))
	require.Equal(t, "foo", cleanUpStartEnd("\tfoo\n"))
}

func TestTestName(t *testing.T) {
	resetGlobals()

	name, builtin, err := testName("id", "desc", "1.0")
	require.NoError(t, err)
	require.Equal(t, "id,rule0", name)
	require.Equal(t, "", builtin)

	// Test builtin
	name, builtin, err = testName("builtin_functions", "abs: desc", "1.0")
	require.NoError(t, err)
	require.Equal(t, "builtin_functions,abs", name)
	require.Equal(t, "abs", builtin)

	// Test duplicate builtin
	name, builtin, err = testName("builtin_functions", "abs: desc", "1.0")
	require.NoError(t, err)
	require.Equal(t, "builtin_functions,abs2", name)

	// Test error cases
	_, _, err = testName("", "desc", "1.0")
	require.ErrorContains(t, err, "cannot generate test name")
	_, _, err = testName("id", "", "1.0")
	require.ErrorContains(t, err, "cannot generate test name")
	_, _, err = testName("id", "desc", "")
	require.ErrorContains(t, err, "cannot generate test name")
}

func TestGetSha1(t *testing.T) {
	resetGlobals()
	s1, err := getSha1("a", "b")
	require.NoError(t, err)
	require.NotEmpty(t, s1)

	s2, err := getSha1("a", "b")
	require.ErrorContains(t, err, "sha1 is not unique")
	require.Empty(t, s2)
}

func TestGetUnimplementedTestPlan(t *testing.T) {
	resetGlobals()

	w := oswrapper.CreateFSTestOSWrapper()

	htmlStr := `
<div id="builtin-functions"></div>
<section data-level="3.0">
	<p>MUST do something</p>
</section>
`
	node := createHTMLNode(t, htmlStr)
	parser, err := Parse(node)
	require.NoError(t, err)
	require.NotEmpty(t, parser.rules)

	err = getUnimplementedTestPlan(*parser, "/cts", w)
	require.NoError(t, err)

	// Verify files created
	// validation/section3.spec.ts should exist
	_, err = w.ReadFile("/cts/validation/section3.spec.ts")
	require.NoError(t, err)
}

func TestGetTestPlanFilePath(t *testing.T) {
	path, err := getTestPlanFilePath("root", 1, -1, 2)
	require.NoError(t, err)
	require.Equal(t, "root/section01.spec.ts", path)

	path, err = getTestPlanFilePath("root", 1, 2, 2)
	require.NoError(t, err)
	require.Equal(t, "root/section1_02.spec.ts", path)
}

func TestValidationTestPlan(t *testing.T) {
	resetGlobals()
	w := oswrapper.CreateFSTestOSWrapper()

	rules := []rule{
		{
			Section:     1,
			SubSection:  "1.0",
			URL:         "url",
			Description: "desc",
			TestName:    "test",
			Sha:         "sha",
		},
	}

	// Setup parser state for start/end sections
	// This function uses start/end passed to it, usually from parser state.

	err := validationTestPlan(rules, "/val", 0, 2, w)
	require.NoError(t, err)

	// Check file existence
	_, err = w.ReadFile("/val/section1.spec.ts")
	require.NoError(t, err)

	// Test write failure
	mockW := &mockOSWrapper{
		OSWrapper:    oswrapper.CreateFSTestOSWrapper(),
		writeFileErr: fmt.Errorf("write error"),
	}
	err = validationTestPlan(rules, "/val", 0, 2, mockW)
	require.ErrorContains(t, err, "write error")
}

func TestExecutionTestPlan(t *testing.T) {
	resetGlobals()

	w := oswrapper.CreateFSTestOSWrapper()

	rules := []rule{
		{
			Section:     1,
			SubSection:  "1.0",
			URL:         "builtin-functions",
			Description: "abs: desc",
			TestName:    "builtin_functions,abs",
		},
	}

	err := executionTestPlan(rules, "/execution", w)
	require.NoError(t, err)

	_, err = w.ReadFile("/execution/section1_0.spec.ts")
	require.NoError(t, err)
}

func TestExecutionTestPlan_Extended(t *testing.T) {
	resetGlobals()
	w := oswrapper.CreateFSTestOSWrapper()

	// Case 1: No builtin section
	rulesNoBuiltin := []rule{{Section: 1, URL: "other", SubSection: "1.0"}}
	err := executionTestPlan(rulesNoBuiltin, "/exec", w)
	require.ErrorContains(t, err, "unable to find the built-in function section")

	// Case 2: Write failure
	mockW := &mockOSWrapper{
		OSWrapper:    oswrapper.CreateFSTestOSWrapper(),
		writeFileErr: fmt.Errorf("write error"),
	}
	rules := []rule{
		{
			Section:     2,
			SubSection:  "2.0",
			URL:         "builtin-functions",
			Description: "abs: desc",
			TestName:    "builtin_functions,abs",
		},
	}
	err = executionTestPlan(rules, "/exec", mockW)
	require.ErrorContains(t, err, "write error")

	// Case 3: Index out of bounds (SubSection with negative index)
	// Need a valid rule to establish the range (so getSectionRange doesn't fail),
	// and an invalid rule to trigger the index check failure.
	rulesOutOfBounds := []rule{
		{Section: 2, SubSection: "2.0", URL: "builtin-functions"},
		{Section: 2, SubSection: "2.-1", URL: "builtin-functions"},
	}
	err = executionTestPlan(rulesOutOfBounds, "/exec", w)
	require.ErrorContains(t, err, "index -1 out of range")
}

func TestGetBuiltinSectionNum(t *testing.T) {
	resetGlobals()

	// Test getBuiltinSectionNum
	rules := []rule{
		{Section: 1, URL: "other"},
		{Section: 2, URL: "builtin-functions"},
	}
	sect, err := getBuiltinSectionNum(rules)
	require.NoError(t, err)
	require.Equal(t, 2, sect)

	rulesNoBuiltin := []rule{
		{Section: 1, URL: "other"},
	}
	_, err = getBuiltinSectionNum(rulesNoBuiltin)
	require.Error(t, err)
}

func TestIsBuiltInFunctionRule(t *testing.T) {
	resetGlobals()

	rBuiltin := rule{
		URL: "builtin-functions",
	}
	require.True(t, isBuiltinFunctionRule(rBuiltin))

	rNotBuiltin := rule{
		URL: "other",
	}
	require.False(t, isBuiltinFunctionRule(rNotBuiltin))
}

func TestTestPlan(t *testing.T) {
	resetGlobals()

	r := rule{
		TestName:    "test/name",
		Sha:         "12345",
		URL:         "http://example.com",
		Description: "description",
	}
	plan := testPlan(r)
	require.Contains(t, plan, "g.test('test/name')")
	require.Contains(t, plan, ".uniqueId('12345')")
	require.Contains(t, plan, ".specURL('http://example.com')")
	require.Contains(t, plan, "description")
}
