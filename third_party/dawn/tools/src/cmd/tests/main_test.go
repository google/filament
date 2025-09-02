// Copyright 2025 Google LLC
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
	"testing"
	"time"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

// invoke() and things that call it cannot be unittested due to the call to
// exec.CommandContext().

func TestExtractValidationHashes(t *testing.T) {
	tests := []struct {
		name       string
		input      string
		wantString string
		wantHashes []string
	}{
		{
			name:       "No matches",
			input:      "ASDF",
			wantString: "ASDF",
			wantHashes: nil,
		},
		{
			name:       "Single match",
			input:      "Line 1\n<<HASH: ASDF>>\nLine 3",
			wantString: "Line 1\nLine 3",
			wantHashes: []string{"ASDF"},
		},
		{
			name:       "Multiple matches",
			input:      "Line 1\n<<HASH: ASDF>>\nLine 3\n<<HASH: QWER>>\nLine 5",
			wantString: "Line 1\nLine 3\nLine 5",
			wantHashes: []string{"ASDF", "QWER"},
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			out, hashes := extractValidationHashes(testCase.input)
			require.Equal(t, testCase.wantString, out)
			require.Equal(t, testCase.wantHashes, hashes)
		})
	}
}

func TestIndent(t *testing.T) {
	tests := []struct {
		name         string
		input        string
		indentAmount int
		want         string
	}{
		{
			name:         "No indent single line",
			input:        "Foo",
			indentAmount: 0,
			want:         "Foo",
		},
		{
			name:         "Indent single line",
			input:        "Foo",
			indentAmount: 2,
			want:         "  Foo",
		},
		{
			name:         "No indent multi line",
			input:        "Foo\nBar",
			indentAmount: 0,
			want:         "Foo\nBar",
		},
		{
			name:         "Indent multi line",
			input:        "Foo\nBar",
			indentAmount: 2,
			want:         "  Foo\n  Bar",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			require.Equal(t, testCase.want, indent(testCase.input, testCase.indentAmount))
		})
	}
}

func TestAlignLeft(t *testing.T) {
	tests := []struct {
		name  string
		input string
		width int
		want  string
	}{
		{
			name:  "Width smaller than string length",
			input: "Foo",
			width: 2,
			want:  "Foo",
		},
		{
			name:  "Width equal to string length",
			input: "Foo",
			width: 3,
			want:  "Foo",
		},
		{
			name:  "Width greater than string length",
			input: "Foo",
			width: 5,
			want:  "Foo  ",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			require.Equal(t, testCase.want, alignLeft(testCase.input, testCase.width))
		})
	}
}

func TestAlignCenter(t *testing.T) {
	tests := []struct {
		name  string
		input string
		width int
		want  string
	}{
		{
			name:  "Width smaller than string length",
			input: "Foo",
			width: 2,
			want:  "Foo",
		},
		{
			name:  "Width equal to string length",
			input: "Foo",
			width: 3,
			want:  "Foo",
		},
		{
			name:  "Width greater than string length",
			input: "Foo",
			width: 5,
			want:  " Foo ",
		},
		{
			name:  "Width greater than string length non-equal padding",
			input: "Foo",
			width: 6,
			want:  " Foo  ",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			require.Equal(t, testCase.want, alignCenter(testCase.input, testCase.width))
		})
	}
}

func TestAlignRight(t *testing.T) {
	tests := []struct {
		name  string
		input string
		width int
		want  string
	}{
		{
			name:  "Width smaller than string length",
			input: "Foo",
			width: 2,
			want:  "Foo",
		},
		{
			name:  "Width equal to string length",
			input: "Foo",
			width: 3,
			want:  "Foo",
		},
		{
			name:  "Width greater than string length",
			input: "Foo",
			width: 5,
			want:  "  Foo",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			require.Equal(t, testCase.want, alignRight(testCase.input, testCase.width))
		})
	}
}

func TestMaxStringLen(t *testing.T) {
	tests := []struct {
		name  string
		input []string
		want  int
	}{
		{
			name:  "Nil",
			input: nil,
			want:  0,
		},
		{
			name:  "Empty",
			input: []string{},
			want:  0,
		},
		{
			name: "Multiple lines",
			input: []string{
				"Line 1",
				"AAAAAAAAAAAA",
				"Line 3",
			},
			want: 12,
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			require.Equal(t, testCase.want, maxStringLen(testCase.input))
		})
	}
}

func TestFormatWidth(t *testing.T) {
	tests := []struct {
		name  string
		input outputFormat
		want  int
	}{
		{
			name:  "Less than minimum",
			input: "",
			want:  6,
		},
		{
			name:  "Equal to minimum",
			input: "123456",
			want:  6,
		},
		{
			name:  "Greater than minimum",
			input: "1234567890",
			want:  10,
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			require.Equal(t, testCase.want, formatWidth(testCase.input))
		})
	}
}

func TestPercentage(t *testing.T) {
	tests := []struct {
		name       string
		inputN     int
		inputTotal int
		want       string
	}{
		{
			name:       "Zero total",
			inputN:     10,
			inputTotal: 0,
			want:       "-",
		},
		{
			name:       "Zero N",
			inputN:     0,
			inputTotal: 10,
			want:       "0.0%",
		},
		{
			name:       "Both positive",
			inputN:     1,
			inputTotal: 10,
			want:       "10.0%",
		},
		{
			name:       "Negative N",
			inputN:     -1,
			inputTotal: 10,
			want:       "-10.0%",
		},
		{
			name:       "Negative total",
			inputN:     1,
			inputTotal: -10,
			want:       "-10.0%",
		},
		{
			name:       "Both negative",
			inputN:     -1,
			inputTotal: -10,
			want:       "10.0%",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			require.Equal(t, testCase.want, percentage(testCase.inputN, testCase.inputTotal))
		})
	}
}

func TestParseFlags(t *testing.T) {
	tests := []struct {
		name          string
		path          string
		contents      string
		skipFileWrite bool
		want          []cmdLineFlags
		wantErr       bool
		wantErrMsg    string
	}{
		{
			name:          "Non-existent file",
			path:          "/foo.txt",
			skipFileWrite: true,
			want:          nil,
			wantErr:       true,
			wantErrMsg:    "open /foo.txt: file does not exist",
		},
		{
			name: "Single entry single flag no format",
			path: "/foo.txt",
			contents: `// Comment
// flags: --hlsl-shader-model 60
// Another comment
`,
			want: []cmdLineFlags{
				{
					formats: container.NewSet(allOutputFormats...),
					flags:   []string{"", "--hlsl-shader-model", "60"},
				},
			},
		},
		{
			name: "Single entry multiple flags no format",
			path: "/foo.txt",
			contents: `// Comment
// flags: --hlsl-shader-model 60 --overrides wgsize=10
// Another comment
`,
			want: []cmdLineFlags{
				{
					formats: container.NewSet(allOutputFormats...),
					flags:   []string{"", "--hlsl-shader-model", "60", "--overrides", "wgsize=10"},
				},
			},
		},
		{
			name: "Single entry single flag with format",
			path: "/foo.txt",
			contents: `// Comment
// [wgsl] flags: --hlsl-shader-model 60
// Another comment
`,
			want: []cmdLineFlags{
				{
					formats: container.NewSet(wgsl),
					flags:   []string{"", "--hlsl-shader-model", "60"},
				},
			},
		},
		{
			name: "Single entry multiple flags with format",
			path: "/foo.txt",
			contents: `// Comment
// [wgsl] flags: --hlsl-shader-model 60 --overrides wgsize=10
// Another comment
`,
			want: []cmdLineFlags{
				{
					formats: container.NewSet(wgsl),
					flags:   []string{"", "--hlsl-shader-model", "60", "--overrides", "wgsize=10"},
				},
			},
		},
		{
			name: "Multiple entries single flag no format",
			path: "/foo.txt",
			contents: `// Comment
// flags: --hlsl-shader-model 60
// Another comment
// flags: --overrides wgsize=10
`,
			want: []cmdLineFlags{
				{
					formats: container.NewSet(allOutputFormats...),
					flags:   []string{"", "--hlsl-shader-model", "60"},
				},
				{
					formats: container.NewSet(allOutputFormats...),
					flags:   []string{"", "--overrides", "wgsize=10"},
				},
			},
		},
		{
			name: "Multiple entries multiple flags no format",
			path: "/foo.txt",
			contents: `// Comment
// flags: --hlsl-shader-model 60 --foo
// Another comment
// flags: --overrides wgsize=10 --bar
`,
			want: []cmdLineFlags{
				{
					formats: container.NewSet(allOutputFormats...),
					flags:   []string{"", "--hlsl-shader-model", "60", "--foo"},
				},
				{
					formats: container.NewSet(allOutputFormats...),
					flags:   []string{"", "--overrides", "wgsize=10", "--bar"},
				},
			},
		},
		{
			name: "Multiple entries single flag with format",
			path: "/foo.txt",
			contents: `// Comment
// [wgsl] flags: --hlsl-shader-model 60
// Another comment
// [glsl] flags: --overrides wgsize=10
`,
			want: []cmdLineFlags{
				{
					formats: container.NewSet(wgsl),
					flags:   []string{"", "--hlsl-shader-model", "60"},
				},
				{
					formats: container.NewSet(glsl),
					flags:   []string{"", "--overrides", "wgsize=10"},
				},
			},
		},
		{
			name: "Multiple entries multiple flags with format",
			path: "/foo.txt",
			contents: `// Comment
// [wgsl] flags: --hlsl-shader-model 60 --foo
// Another comment
// [glsl] flags: --overrides wgsize=10 --bar
`,
			want: []cmdLineFlags{
				{
					formats: container.NewSet(wgsl),
					flags:   []string{"", "--hlsl-shader-model", "60", "--foo"},
				},
				{
					formats: container.NewSet(glsl),
					flags:   []string{"", "--overrides", "wgsize=10", "--bar"},
				},
			},
		},
		{
			name: "Multiple entries multiple flags mixed formats",
			path: "/foo.txt",
			contents: `// Comment
// flags: --hlsl-shader-model 60 --foo
// Another comment
// [glsl] flags: --overrides wgsize=10 --bar
`,
			want: []cmdLineFlags{
				{
					formats: container.NewSet(allOutputFormats...),
					flags:   []string{"", "--hlsl-shader-model", "60", "--foo"},
				},
				{
					formats: container.NewSet(glsl),
					flags:   []string{"", "--overrides", "wgsize=10", "--bar"},
				},
			},
		},
		{
			name: "Invalid format",
			path: "/foo.txt",
			contents: `// Comment
// [invalid] flags: --hlsl-shader-model 60
// Another comment
`,
			want:       nil,
			wantErr:    true,
			wantErrMsg: "unknown format 'invalid'",
		},
		{
			name: "-ir formats automatically included",
			path: "/foo.txt",
			contents: `// Comment
// [hlsl-dxc] flags: --hlsl-shader-model 60
// Another comment
`,
			want: []cmdLineFlags{
				{
					formats: container.NewSet(hlslDXC),
					flags:   []string{"", "--hlsl-shader-model", "60"},
				},
			},
		},
		{
			name: "Ignored after first comment block",
			path: "/foo.txt",
			contents: `// Comment
// [wgsl] flags: --hlsl-shader-model 60
// Another comment
foo
// [glsl] flags: --hlsl-shader-model 60
`,
			want: []cmdLineFlags{
				{
					formats: container.NewSet(wgsl),
					flags:   []string{"", "--hlsl-shader-model", "60"},
				},
			},
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			if !testCase.skipFileWrite {
				wrapper.WriteFile(testCase.path, []byte(testCase.contents), 0o700)
			}

			flags, err := parseFlags(testCase.path, wrapper)
			if testCase.wantErr {
				require.ErrorContains(t, err, testCase.wantErrMsg)
			} else {
				require.NoErrorf(t, err, "Got error parsing flags: %v", err)
			}
			require.Equal(t, testCase.want, flags)
		})
	}
}

func TestPrintDuration(t *testing.T) {
	tests := []struct {
		name         string
		inputToParse string
		want         string
	}{
		{
			name:         "Seconds",
			inputToParse: "5s",
			want:         "5s",
		},
		{
			name:         "Minutes",
			inputToParse: "5m",
			want:         "5m",
		},
		{
			name:         "Minutes and Seconds",
			inputToParse: "5m5s",
			want:         "5m5s",
		},
		{
			name:         "Hours",
			inputToParse: "5h",
			want:         "5h",
		},
		{
			name:         "Hours and Minutes",
			inputToParse: "5h5m",
			want:         "5h5m",
		},
		{
			name:         "Hours and Seconds",
			inputToParse: "5h5s",
			want:         "5h5s",
		},
		{
			name:         "Hours, Minutes, and Seconds",
			inputToParse: "5h5m5s",
			want:         "5h5m5s",
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			input, err := time.ParseDuration(testCase.inputToParse)
			require.NoErrorf(t, err, "Failed to parse duration: %v", err)
			require.Equal(t, testCase.want, printDuration(input))
		})
	}
}
