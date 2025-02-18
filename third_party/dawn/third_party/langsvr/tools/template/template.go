// Copyright 2024 The langsvr Authors
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

// Package template wraps the golang "text/template" package to provide an
// enhanced template generator.
package template

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"reflect"
	"strings"
	"text/template"

	"golang.org/x/text/cases"
	"golang.org/x/text/language"

	"github.com/google/langsvr/tools/fileutils"
)

// Functions is an alias to the template function binding table
type Functions = template.FuncMap

// Template holds an executable go template.
// See https://golang.org/pkg/text/template/
type Template struct {
	name    string
	content string
}

// FromFile loads the template file at path and builds and returns a Template
// using the file content
func FromFile(path string) (*Template, error) {
	content, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	return FromString(path, string(content)), nil
}

// FromString returns a Template with the given name from content
func FromString(name, content string) *Template {
	return &Template{name: name, content: content}
}

// Run executes the template tmpl, writing the output to w.
// funcs are the functions provided to the template.
// See https://golang.org/pkg/text/template/ for documentation on the template
// syntax.
func (t *Template) Run(w io.Writer, data any, funcs Functions) error {
	g := generator{
		template: template.New(t.name),
	}

	// Add a bunch of generic useful functions
	g.funcs = Functions{
		"Eval":   g.eval,
		"Import": g.importTmpl,
		"Is":     is,
		"Split":  strings.Split,
		"Join":   strings.Join,
		"Title":  cases.Title(language.Und, cases.NoLower).String,
		"Sum":    sum,
		"Error":  func(err string, args ...any) string { panic(fmt.Errorf(err, args...)) },
	}

	// Append custom functions
	for name, fn := range funcs {
		g.funcs[name] = fn
	}

	if err := g.bindAndParse(g.template, t.content); err != nil {
		return err
	}

	return g.template.Execute(w, data)
}

type generator struct {
	template *template.Template
	funcs    Functions
}

func (g *generator) bindAndParse(t *template.Template, tmpl string) error {
	_, err := t.
		Funcs(map[string]any(g.funcs)).
		Option("missingkey=error").
		Parse(tmpl)
	return err
}

// eval executes the sub-template with the given name and argument, returning
// the generated output
func (g *generator) eval(template string, args ...any) (string, error) {
	target := g.template.Lookup(template)
	if target == nil {
		return "", fmt.Errorf("template '%v' not found", template)
	}
	sb := strings.Builder{}

	var err error
	if len(args) == 1 {
		err = target.Execute(&sb, args[0])
	} else {
		m := map[string]any{}
		if len(args)%2 != 0 {
			return "", fmt.Errorf("Eval expects a single argument or list name-value pairs")
		}
		for i := 0; i < len(args); i += 2 {
			name, ok := args[i].(string)
			if !ok {
				return "", fmt.Errorf("Eval argument %v is not a string", i)
			}
			m[name] = args[i+1]
		}
		err = target.Execute(&sb, m)
	}

	if err != nil {
		return "", fmt.Errorf("while evaluating '%v': %v", template, err)
	}
	return sb.String(), nil
}

// importTmpl parses and executes the template at the given project-relative
// path, merging the template definitions into the global namespace.
// Note: The body of the template is not emitted.
func (g *generator) importTmpl(path string) (string, error) {
	if strings.Contains(path, "..") {
		return "", fmt.Errorf("import path must not contain '..'")
	}
	path = filepath.Join(fileutils.ProjectRoot(), path)
	data, err := os.ReadFile(path)
	if err != nil {
		return "", fmt.Errorf("failed to open '%v': %w", path, err)
	}
	t := g.template.New("")
	if err := g.bindAndParse(t, string(data)); err != nil {
		return "", fmt.Errorf("failed to parse '%v': %w", path, err)
	}
	if err := t.Execute(io.Discard, nil); err != nil {
		return "", fmt.Errorf("failed to execute '%v': %w", path, err)
	}
	return "", nil
}

// is returns true if the type of object is ty
func is(object any, ty string) bool {
	if object == nil {
		return false
	}
	val := reflect.ValueOf(object)
	for val.Kind() == reflect.Pointer {
		val = val.Elem()
	}
	return ty == val.Type().Name()
}

// sum returns the sum of all the input integers
func sum(numbers ...int) int {
	n := 0
	for _, i := range numbers {
		n += i
	}
	return n
}
