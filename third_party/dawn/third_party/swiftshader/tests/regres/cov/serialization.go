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

package cov

import (
	"bufio"
	"compress/zlib"
	"fmt"
	"io"
	"runtime/debug"
	"sort"
	"strconv"
	"strings"
)

// ReadJSON parses the JSON Tree from r.
func ReadJSON(r io.Reader) (*Tree, string, error) {
	p := parser{r: bufio.NewReader(r)}
	return p.parse()
}

// Encode zlib encodes the JSON coverage tree to w.
func (t *Tree) Encode(revision string, w io.Writer) error {
	t.Optimize()

	zw := zlib.NewWriter(w)

	_, err := zw.Write([]byte(t.JSON(revision)))
	if err != nil {
		return err
	}

	return zw.Close()
}

// JSON returns the full test tree serialized to JSON.
func (t *Tree) JSON(revision string) string {
	sb := &strings.Builder{}
	sb.WriteString(`{`)

	spansByID := map[SpanID]Span{}
	for span, id := range t.spans {
		spansByID[id] = span
	}

	// write the revision
	sb.WriteString(`"r":"` + revision + `"`)

	// write the strings
	sb.WriteString(`,"n":[`)
	for i, s := range t.strings.s {
		if i > 0 {
			sb.WriteString(`,`)
		}
		sb.WriteString(`"`)
		sb.WriteString(s)
		sb.WriteString(`"`)
	}
	sb.WriteString(`]`)

	// write the tests
	sb.WriteString(`,"t":`)
	t.writeTestJSON(&t.testRoot, sb)

	// write the spans
	sb.WriteString(`,"s":`)
	t.writeSpansJSON(sb)

	// write the files
	sb.WriteString(`,"f":`)
	t.writeFilesJSON(spansByID, sb)

	sb.WriteString(`}`)
	return sb.String()
}

func (t *Tree) writeTestJSON(test *Test, sb *strings.Builder) {
	names := map[int]StringID{}
	for name, idx := range test.indices {
		names[int(idx)] = name
	}

	sb.WriteString(`[`)
	for i, child := range test.children {
		if i > 0 {
			sb.WriteString(`,`)
		}
		sb.WriteString(`[`)
		sb.WriteString(fmt.Sprintf("%v,", names[i]))
		t.writeTestJSON(&child, sb)
		sb.WriteString(`]`)
	}

	sb.WriteString(`]`)
}

func (t *Tree) writeSpansJSON(sb *strings.Builder) {
	type spanAndID struct {
		span Span
		id   SpanID
	}
	spans := make([]spanAndID, 0, len(t.spans))
	for span, id := range t.spans {
		spans = append(spans, spanAndID{span, id})
	}
	sort.Slice(spans, func(i, j int) bool { return spans[i].id < spans[j].id })

	sb.WriteString(`[`)
	for i, s := range spans {
		if i > 0 {
			sb.WriteString(`,`)
		}
		sb.WriteString(fmt.Sprintf("[%v,%v,%v,%v]",
			s.span.Start.Line, s.span.Start.Column,
			s.span.End.Line, s.span.End.Column))
	}

	sb.WriteString(`]`)
}

func (t *Tree) writeSpanJSON(span Span, sb *strings.Builder) {
	sb.WriteString(fmt.Sprintf("[%v,%v,%v,%v]",
		span.Start.Line, span.Start.Column,
		span.End.Line, span.End.Column))
}

func (t *Tree) writeFilesJSON(spansByID map[SpanID]Span, sb *strings.Builder) {
	paths := make([]string, 0, len(t.files))
	for path := range t.files {
		paths = append(paths, path)
	}
	sort.Strings(paths)

	sb.WriteString(`{`)
	for i, path := range paths {
		file := t.files[path]

		uncovered := append(SpanList{}, file.allSpans...)
		for id := range t.allSpans(file, file.tcm) {
			uncovered.Remove(spansByID[id])
		}

		if i > 0 {
			sb.WriteString(`,`)
		}
		sb.WriteString(`"`)
		sb.WriteString(path)
		sb.WriteString(`":`)
		sb.WriteString(`{`)
		if totalLines := file.allSpans.NumLines(); totalLines > 0 {
			uncoveredLines := uncovered.NumLines()
			percentage := 1.0 - (float64(uncoveredLines) / float64(totalLines))
			sb.WriteString(`"p":`)
			sb.WriteString(fmt.Sprintf("%v", percentage))
			sb.WriteString(`,`)
		}
		sb.WriteString(`"g":`)
		t.writeSpanGroupsJSON(file.spangroups, sb)
		sb.WriteString(`,"u":`)
		t.writeUncoveredJSON(file, uncovered, sb)
		sb.WriteString(`,"c":`)
		t.writeCoverageMapJSON(file.tcm, sb)
		sb.WriteString(`}`)
	}

	sb.WriteString(`}`)
}

func (t *Tree) writeSpanGroupsJSON(spangroups map[SpanGroupID]SpanGroup, sb *strings.Builder) {
	type groupAndID struct {
		group SpanGroup
		id    SpanGroupID
	}
	groups := make([]groupAndID, 0, len(spangroups))
	for id, group := range spangroups {
		groups = append(groups, groupAndID{group, id})
	}
	sort.Slice(groups, func(i, j int) bool { return groups[i].id < groups[j].id })

	sb.WriteString(`[`)
	for i, g := range groups {
		if i > 0 {
			sb.WriteString(`,`)
		}
		t.writeSpanGroupJSON(g.group, sb)
	}
	sb.WriteString(`]`)
}

func (t *Tree) writeSpanGroupJSON(group SpanGroup, sb *strings.Builder) {
	sb.WriteString(`{`)
	sb.WriteString(`"s":[`)
	for i, spanID := range group.Spans.List() {
		if i > 0 {
			sb.WriteString(`,`)
		}
		sb.WriteString(fmt.Sprintf("%v", spanID))
	}
	sb.WriteString(`]`)
	if group.Extend != nil {
		sb.WriteString(`,"e":`)
		sb.WriteString(fmt.Sprintf("%v", *group.Extend))
	}
	sb.WriteString(`}`)
}

func (t *Tree) writeUncoveredJSON(tf *treeFile, uncovered SpanList, sb *strings.Builder) {
	sb.WriteString(`[`)
	for i, span := range uncovered {
		if i > 0 {
			sb.WriteString(`,`)
		}
		t.writeSpanJSON(span, sb)
	}
	sb.WriteString(`]`)
}

func (t *Tree) writeCoverageMapJSON(c TestCoverageMap, sb *strings.Builder) {
	ids := make([]TestIndex, 0, len(c))
	for id := range c {
		ids = append(ids, id)
	}
	sort.Slice(ids, func(i, j int) bool { return ids[i] < ids[j] })

	sb.WriteString(`[`)
	for i, id := range ids {
		if i > 0 {
			sb.WriteString(`,`)
		}

		sb.WriteString(`[`)
		sb.WriteString(fmt.Sprintf("%v", id))
		sb.WriteString(`,`)
		t.writeCoverageJSON(c[id], sb)
		sb.WriteString(`]`)
	}
	sb.WriteString(`]`)
}

func (t *Tree) writeCoverageJSON(c *TestCoverage, sb *strings.Builder) {
	sb.WriteString(`{`)
	comma := false
	if len(c.Spans) > 0 {
		sb.WriteString(`"s":[`)
		for i, spanID := range c.Spans.List() {
			if i > 0 {
				sb.WriteString(`,`)
			}
			sb.WriteString(fmt.Sprintf("%v", spanID))
		}
		sb.WriteString(`]`)
		comma = true
	}
	if c.Group != nil {
		sb.WriteString(`"g":`)
		sb.WriteString(fmt.Sprintf("%v", *c.Group))
		comma = true
	}
	if len(c.Children) > 0 {
		if comma {
			sb.WriteString(`,`)
		}
		sb.WriteString(`"c":`)
		t.writeCoverageMapJSON(c.Children, sb)
	}
	sb.WriteString(`}`)
}

type parser struct {
	r   *bufio.Reader
	err error

	revision string
	tree     Tree
}

func (p *parser) parse() (*Tree, string, error) {
	p.tree.init()
	p.dict(func(key string) {
		switch key {
		case "r":
			p.revision = p.str()
		case "n":
			p.parseStrings()
		case "t":
			p.parseTests(&p.tree.testRoot)
		case "s":
			p.parseSpans()
		case "g":
			p.parseSpanGroups()
		case "f":
			p.parseFiles()
		default:
			p.fail("Unknown root key '%v'", key)
		}
	})
	if p.err != nil {
		return nil, "", p.err
	}

	p.populateAllSpans(&p.tree)

	return &p.tree, p.revision, nil
}

// populateAllSpans() adds all the coverage spans to each treeFile.allSpans.
func (p *parser) populateAllSpans(tree *Tree) {
	spansByID := map[SpanID]Span{}
	for span, id := range tree.spans {
		spansByID[id] = span
	}
	for _, file := range tree.files {
		for spanID := range tree.allSpans(file, file.tcm) {
			span := spansByID[spanID]
			file.allSpans.Add(span)
		}
	}
}

func (p *parser) parseStrings() {
	p.array(func(idx int) {
		id := StringID(idx)
		s := p.str()
		p.tree.strings.m[s] = id
		p.tree.strings.s = append(p.tree.strings.s, s)
	})
}

func (p *parser) parseTests(t *Test) {
	p.array(func(idx int) {
		p.expect("[")
		name := StringID(p.integer())
		child, _ := t.index(name)
		p.expect(",")
		p.parseTests(child)
		p.expect("]")
	})
}

func (p *parser) parseSpans() {
	p.array(func(idx int) {
		p.tree.spans[p.parseSpan()] = SpanID(idx)
	})
}

func (p *parser) parseSpan() Span {
	p.expect("[")
	s := Span{}
	s.Start.Line = p.integer()
	p.expect(",")
	s.Start.Column = p.integer()
	p.expect(",")
	s.End.Line = p.integer()
	p.expect(",")
	s.End.Column = p.integer()
	p.expect("]")
	return s
}

func (p *parser) parseFiles() {
	p.dict(func(path string) {
		p.tree.files[path] = p.parseFile()
	})
}

func (p *parser) parseFile() *treeFile {
	file := newTreeFile()
	if p.peek() == '{' {
		p.dict(func(key string) {
			switch key {
			case "p":
				p.double()
			case "g":
				file.spangroups = p.parseSpanGroups()
			case "c":
				p.parseCoverageMap(file.tcm)
			case "u":
				p.parseUncovered(file)
			default:
				p.fail("Unknown file key: '%s'", key)
			}
		})
	} else { // backwards compatibility
		p.parseCoverageMap(file.tcm)
	}
	return file
}

func (p *parser) parseSpanGroups() map[SpanGroupID]SpanGroup {
	spangroups := map[SpanGroupID]SpanGroup{}
	p.array(func(groupIdx int) {
		g := newSpanGroup()
		p.dict(func(key string) {
			switch key {
			case "s":
				p.array(func(spanIdx int) {
					id := SpanID(p.integer())
					g.Spans[id] = struct{}{}
				})
			case "e":
				extend := SpanGroupID(p.integer())
				g.Extend = &extend
			}
		})
		spangroups[SpanGroupID(groupIdx)] = g
	})
	return spangroups
}

func (p *parser) parseCoverageMap(tcm TestCoverageMap) {
	p.array(func(int) {
		p.expect("[")
		idx := TestIndex(p.integer())
		p.expect(",")
		p.parseCoverage(tcm.index(idx))
		p.expect("]")
	})
}

func (p *parser) parseUncovered(tf *treeFile) {
	p.array(func(int) {
		tf.allSpans.Add(p.parseSpan())
	})
}

func (p *parser) parseCoverage(tc *TestCoverage) {
	p.dict(func(key string) {
		switch key {
		case "s":
			p.array(func(int) {
				id := SpanID(p.integer())
				tc.Spans[id] = struct{}{}
			})
		case "g":
			groupID := SpanGroupID(p.integer())
			tc.Group = &groupID
		case "c":
			p.parseCoverageMap(tc.Children)
		default:
			p.fail("Unknown test key: '%s'", key)
		}
	})
}

func (p *parser) array(f func(idx int)) {
	p.expect("[")
	if p.match("]") {
		return
	}
	idx := 0
	for p.err == nil {
		f(idx)
		if !p.match(",") {
			p.expect("]")
			return
		}
		idx++
	}
	p.expect("]")
}

func (p *parser) dict(f func(key string)) {
	p.expect("{")
	if p.match("}") {
		return
	}
	for p.err == nil {
		key := p.str()
		p.expect(`:`)
		f(key)
		if !p.match(",") {
			p.expect("}")
			return
		}
	}
	p.expect("}")
}

func (p *parser) next() byte {
	d := make([]byte, 1)
	n, err := p.r.Read(d)
	if err != nil || n != 1 {
		p.err = err
		return 0
	}
	return d[0]
}

func (p *parser) peek() byte {
	d, err := p.r.Peek(1)
	if err != nil {
		p.err = err
		return 0
	}
	return d[0]
}

func (p *parser) expect(s string) {
	if p.err != nil {
		return
	}
	d := make([]byte, len(s))
	n, err := p.r.Read(d)
	if err != nil {
		p.err = err
		return
	}
	got := string(d[:n])
	if got != s {
		p.fail("Expected '%v', got '%v'", s, got)
		return
	}
}

func (p *parser) match(s string) bool {
	got, err := p.r.Peek(len(s))
	if err != nil {
		return false
	}
	if string(got) != s {
		return false
	}
	p.r.Discard(len(s))
	return true
}

func (p *parser) str() string {
	p.expect(`"`)
	sb := strings.Builder{}
	for p.err == nil {
		c := p.next()
		if c == '"' {
			return sb.String()
		}
		sb.WriteByte(c)
	}
	return ""
}

func (p *parser) integer() int {
	sb := strings.Builder{}
	for {
		if c := p.peek(); c < '0' || c > '9' {
			break
		}
		sb.WriteByte(p.next())
	}
	if sb.Len() == 0 {
		p.fail("Expected integer, got '%c'", p.peek())
		return 0
	}
	i, err := strconv.Atoi(sb.String())
	if err != nil {
		p.fail("Failed to parse integer: %v", err)
		return 0
	}
	return i
}

func (p *parser) double() float64 {
	sb := strings.Builder{}
	for {
		if c := p.peek(); c != '.' && (c < '0' || c > '9') {
			break
		}
		sb.WriteByte(p.next())
	}
	if sb.Len() == 0 {
		p.fail("Expected double, got '%c'", p.peek())
		return 0
	}
	f, err := strconv.ParseFloat(sb.String(), 64)
	if err != nil {
		p.fail("Failed to parse double: %v", err)
		return 0
	}
	return f
}

func (p *parser) fail(msg string, args ...interface{}) {
	if p.err == nil {
		msg = fmt.Sprintf(msg, args...)
		stack := string(debug.Stack())
		p.err = fmt.Errorf("%v\nCallstack:\n%v", msg, stack)
	}
}
