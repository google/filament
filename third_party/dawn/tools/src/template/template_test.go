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

package template_test

import (
	"bytes"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/template"
	"github.com/google/go-cmp/cmp"
)

func check(t *testing.T, content, expected string, fns template.Functions) {
	t.Helper()
	w := &bytes.Buffer{}
	if err := template.FromString("template", content).Run(w, nil, fns); err != nil {
		t.Errorf("Template.Run() failed with %v", err)
		return
	}
	got := w.String()
	if diff := cmp.Diff(expected, got); diff != "" {
		t.Errorf("output was not as expected. Diff:\n%v", diff)
	}
}

func TestContains(t *testing.T) {
	tmpl := `
{{ Contains "hello world" "hello"}}
{{ Contains "hello world" "fish"}}
`
	expected := `
true
false
`
	check(t, tmpl, expected, nil)
}

func TestEvalSingleParameter(t *testing.T) {
	tmpl := `
pre-eval
{{ Eval "T" 123 }}
{{ Eval "T" "cat" }}
post-eval

pre-define
{{- define "T"}}
  . is {{.}}
{{- end }}
post-define
`
	expected := `
pre-eval

  . is 123

  . is cat
post-eval

pre-define
post-define
`
	check(t, tmpl, expected, nil)
}

func TestEvalParameterPairs(t *testing.T) {
	tmpl := `
pre-eval
{{ Eval "T" "number" 123 "animal" "cat" }}
post-eval

pre-define
{{- define "T"}}
  .number is {{.number}}
  .animal is {{.animal}}
{{- end }}
post-define
`
	expected := `
pre-eval

  .number is 123
  .animal is cat
post-eval

pre-define
post-define
`
	check(t, tmpl, expected, nil)
}

func TestHasPrefix(t *testing.T) {
	tmpl := `
{{ HasPrefix "hello world" "hello"}}
{{ HasPrefix "hello world" "world"}}
`
	expected := `
true
false
`
	check(t, tmpl, expected, nil)
}

func TestIterate(t *testing.T) {
	tmpl := `
{{- range $i := Iterate 5}}
  {{$i}}
{{- end}}
`
	expected := `
  0
  1
  2
  3
  4
`
	check(t, tmpl, expected, nil)
}

func TestMap(t *testing.T) {
	tmpl := `
	{{- $m := Map }}
	{{- $m.Put "one" 1 }}
	{{- $m.Put "two" 2 }}
	one: {{ $m.Get "one" }}
	two: {{ $m.Get "two" }}
`
	expected := `
	one: 1
	two: 2
`
	check(t, tmpl, expected, nil)
}

func TestPascalCase(t *testing.T) {
	tmpl := `
{{ PascalCase "hello world" }}
{{ PascalCase "hello_world" }}
`
	expected := `
HelloWorld
HelloWorld
`
	check(t, tmpl, expected, nil)
}

func TestSplit(t *testing.T) {
	tmpl := `
{{- range $i, $s := Split "cat_says_meow" "_" }}
  {{$i}}: '{{$s}}'
{{- end }}
`
	expected := `
  0: 'cat'
  1: 'says'
  2: 'meow'
`
	check(t, tmpl, expected, nil)
}

func TestTitle(t *testing.T) {
	tmpl := `
{{Title "hello world"}}
`
	expected := `
Hello World
`
	check(t, tmpl, expected, nil)
}

func TrimLeft(t *testing.T) {
	tmpl := `
'{{TrimLeft "hello world", "hel"}}'
`
	expected := `
'o world'
`
	check(t, tmpl, expected, nil)
}

func TrimPrefix(t *testing.T) {
	tmpl := `
'{{TrimLeft "hello world", "hel"}}'
'{{TrimLeft "hello world", "heo"}}'
`
	expected := `
'o world'
'hello world'
`
	check(t, tmpl, expected, nil)
}

func TrimRight(t *testing.T) {
	tmpl := `
'{{TrimRight "hello world", "wld"}}'
`
	expected := `
'hello wor'
`
	check(t, tmpl, expected, nil)
}

func TrimSuffix(t *testing.T) {
	tmpl := `
'{{TrimRight "hello world", "rld"}}'
'{{TrimRight "hello world", "wld"}}'
`
	expected := `
'hello wo'
'hello world'
`
	check(t, tmpl, expected, nil)
}
