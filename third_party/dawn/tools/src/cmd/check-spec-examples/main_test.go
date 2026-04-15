// Copyright 2025 The Dawn & Tint Authors
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
	"strings"
	"testing"

	"github.com/stretchr/testify/require"
	"golang.org/x/net/html"
)

func TestGatherExamples(t *testing.T) {
	tests := []struct {
		name     string
		html     string
		expected []example
	}{
		{
			name:     "empty html",
			html:     "",
			expected: []example{},
		},
		{
			name:     "no examples",
			html:     "<html><body><p>Hello</p></body></html>",
			expected: []example{},
		},
		{
			name: "single global-scope example",
			html: `
				<div class="example wgsl global-scope" id="ex1">
					<pre>var x : i32;</pre>
				</div>
			`,
			expected: []example{
				{
					name:          "ex1",
					code:          "var x : i32;",
					globalScope:   true,
					functionScope: false,
					expectError:   false,
				},
			},
		},
		{
			name: "single function-scope example",
			html: `
				<div class="example wgsl function-scope" id="ex2">
					<pre>let y = 1;</pre>
				</div>
			`,
			expected: []example{
				{
					name:          "ex2",
					code:          "let y = 1;",
					globalScope:   false,
					functionScope: true,
					expectError:   false,
				},
			},
		},
		{
			name: "expect-error example",
			html: `
				<div class="example wgsl global-scope expect-error" id="ex3">
					<pre>error</pre>
				</div>
			`,
			expected: []example{
				{
					name:          "ex3",
					code:          "error",
					globalScope:   true,
					functionScope: false,
					expectError:   true,
				},
			},
		},
		{
			name: "ignored example (no scope)",
			html: `
				<div class="example wgsl" id="ignored">
					<pre>ignore me</pre>
				</div>
			`,
			expected: []example{},
		},
		{
			name: "ignored example (not wgsl)",
			html: `
				<div class="example cpp global-scope" id="cpp">
					<pre>int main() {}</pre>
				</div>
			`,
			expected: []example{},
		},
		{
			name: "ignored example (not example class)",
			html: `
				<div class="code wgsl global-scope" id="not-ex">
					<pre>code</pre>
				</div>
			`,
			expected: []example{},
		},
		{
			name: "multiple examples",
			html: `
				<div class="example wgsl global-scope" id="ex1">
					<pre>ex1</pre>
				</div>
				<div>
					<div class="example wgsl function-scope" id="ex2">
						<pre>ex2</pre>
					</div>
				</div>
			`,
			expected: []example{
				{
					name:          "ex1",
					code:          "ex1",
					globalScope:   true,
					functionScope: false,
					expectError:   false,
				},
				{
					name:          "ex2",
					code:          "ex2",
					globalScope:   false,
					functionScope: true,
					expectError:   false,
				},
			},
		},
		{
			name: "complex content",
			html: `
				<div class="example wgsl global-scope" id="ex_complex">
					<pre>part1 <span>part2</span> part3</pre>
				</div>
			`,
			expected: []example{
				{
					name:          "ex_complex",
					code:          "part1 part2 part3",
					globalScope:   true,
					functionScope: false,
					expectError:   false,
				},
			},
		},
		{
			name: "multiple pre blocks",
			html: `
				<div class="example wgsl global-scope" id="ex_multi_pre">
					<pre>part1</pre>
					<pre>part2</pre>
				</div>
			`,
			expected: []example{
				{
					name:          "ex_multi_pre",
					code:          "part1part2",
					globalScope:   true,
					functionScope: false,
					expectError:   false,
				},
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			doc, err := html.Parse(strings.NewReader(tc.html))
			require.NoError(t, err)

			examples := []example{}
			err = gatherExamples(doc, &examples)
			require.NoError(t, err)
			require.Equal(t, tc.expected, examples)
		})
	}
}

func TestPrintNodeText(t *testing.T) {
	tests := []struct {
		name     string
		node     *html.Node
		expected string
	}{
		{
			name:     "single text node",
			node:     &html.Node{Type: html.TextNode, Data: "Hello"},
			expected: "Hello",
		},
		{
			name: "nested text nodes",
			node: &html.Node{
				Type: html.ElementNode, Data: "div",
				FirstChild: &html.Node{Type: html.TextNode, Data: "Part1",
					NextSibling: &html.Node{
						Type: html.ElementNode, Data: "span",
						FirstChild:  &html.Node{Type: html.TextNode, Data: "Part2"},
						NextSibling: &html.Node{Type: html.TextNode, Data: "Part3"},
					},
				},
			},
			expected: "Part1Part2Part3",
		},
		{
			name:     "empty node",
			node:     &html.Node{Type: html.ElementNode, Data: "div"},
			expected: "",
		},
		{
			name:     "node with no text children",
			node:     &html.Node{Type: html.ElementNode, Data: "div", FirstChild: &html.Node{Type: html.ElementNode, Data: "span"}},
			expected: "",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			sb := &strings.Builder{}
			printNodeText(tc.node, sb)
			require.Equal(t, tc.expected, sb.String())
		})
	}
}

func TestHasClass(t *testing.T) {
	node := &html.Node{
		Attr: []html.Attribute{
			{Key: "class", Val: "foo bar baz"},
		},
	}
	require.True(t, hasClass(node, "foo"))
	require.True(t, hasClass(node, "bar"))
	require.True(t, hasClass(node, "baz"))
	require.False(t, hasClass(node, "fo"))
	require.False(t, hasClass(node, "ba"))
}

func TestNodeID(t *testing.T) {
	nodeWithID := &html.Node{
		Attr: []html.Attribute{
			{Key: "id", Val: "my-id"},
		},
	}
	require.Equal(t, "my-id", nodeID(nodeWithID))

	nodeWithoutID := &html.Node{
		Attr: []html.Attribute{
			{Key: "class", Val: "foo"},
		},
	}
	require.Equal(t, "", nodeID(nodeWithoutID))
}
