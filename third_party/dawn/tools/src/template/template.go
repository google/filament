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

// Package template wraps the golang "text/template" package to provide an
// enhanced template generator.
package template

import (
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"reflect"
	"strings"
	"text/template"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/text"
	"dawn.googlesource.com/dawn/tools/src/transform"
)

// The template function binding table
type Functions = template.FuncMap

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

	globals := newMap()

	// Add a bunch of generic useful functions
	g.funcs = Functions{
		"Append":     listAppend,
		"Concat":     listConcat,
		"Contains":   strings.Contains,
		"Eval":       g.eval,
		"Globals":    func() Map { return globals },
		"HasPrefix":  strings.HasPrefix,
		"HasSuffix":  strings.HasSuffix,
		"Import":     g.importTmpl,
		"Index":      index,
		"Is":         is,
		"Iterate":    iterate,
		"Join":       strings.Join,
		"List":       list,
		"Map":        newMap,
		"PascalCase": text.PascalCase,
		"Repeat":     strings.Repeat,
		"Replace":    replace,
		"SortUnique": listSortUnique,
		"Split":      strings.Split,
		"Sum":        sum,
		"Title":      strings.Title,
		"ToLower":    strings.ToLower,
		"ToUpper":    strings.ToUpper,
		"TrimLeft":   strings.TrimLeft,
		"TrimPrefix": strings.TrimPrefix,
		"TrimRight":  strings.TrimRight,
		"TrimSuffix": strings.TrimSuffix,
		"Error":      func(err string, args ...any) string { panic(fmt.Errorf(err, args...)) },
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
		m := newMap()
		if len(args)%2 != 0 {
			return "", fmt.Errorf("Eval expects a single argument or list name-value pairs")
		}
		for i := 0; i < len(args); i += 2 {
			name, ok := args[i].(string)
			if !ok {
				return "", fmt.Errorf("Eval argument %v is not a string", i)
			}
			m.Put(name, args[i+1])
		}
		err = target.Execute(&sb, m)
	}

	if err != nil {
		return "", fmt.Errorf("while evaluating '%v' with args '%v'\n%v", template, args, err)
	}
	return sb.String(), nil
}

// TODO(crbug.com/344014313): Add unittest coverage.
// importTmpl parses the template at the given project-relative path, merging
// the template definitions into the global namespace.
// Note: The body of the template is not executed.
func (g *generator) importTmpl(path string) (string, error) {
	// TODO(crbug.com/344014313): Use a properly injected interface instead of
	// hard-coding the real wrapper. We cannot easily take it as an argument due
	// to "tools/src/cmd/gen/main.go --check-stale" complaining about a
	// mismatched number of arguments despite all relevant Go code being updated.
	fsReader := oswrapper.GetRealOSWrapper()
	if strings.Contains(path, "..") {
		return "", fmt.Errorf("import path must not contain '..'")
	}
	path = filepath.Join(fileutils.DawnRoot(fsReader), path)
	data, err := fsReader.ReadFile(path)
	if err != nil {
		return "", fmt.Errorf("failed to open '%v': %w", path, err)
	}
	t := g.template.New("")
	if err := g.bindAndParse(t, string(data)); err != nil {
		return "", fmt.Errorf("failed to parse '%v': %w", path, err)
	}
	if err := t.Execute(ioutil.Discard, nil); err != nil {
		return "", fmt.Errorf("failed to execute '%v': %w", path, err)
	}
	return "", nil
}

// Map is a simple generic key-value map, which can be used in the template
type Map map[any]any

func newMap() Map { return Map{} }

// Put adds the key-value pair into the map.
// Put always returns an empty string so nothing is printed in the template.
func (m Map) Put(key, value any) string {
	m[key] = value
	return ""
}

// Get looks up and returns the value with the given key. If the map does not
// contain the given key, then nil is returned.
func (m Map) Get(key any) any {
	return m[key]
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

// sum returns the sum of provided values
func sum(numbers ...int) int {
	n := 0
	for _, i := range numbers {
		n += i
	}
	return n
}

// iterate returns a slice of length 'n', with each element equal to its index.
// Useful for: {{- range Iterate $n -}}<this will be looped $n times>{{end}}
func iterate(n int) []int {
	out := make([]int, n)
	for i := range out {
		out[i] = i
	}
	return out
}

// listAppend returns the slice list with items appended
func listAppend(list any, items ...any) any {
	itemValues := transform.SliceNoErr(items, reflect.ValueOf)
	return reflect.Append(reflect.ValueOf(list), itemValues...).Interface()
}

// listConcat returns a slice formed from concatenating all the elements of all
// the slice arguments
func listConcat(firstList any, otherLists ...any) any {
	out := reflect.ValueOf(firstList)
	for _, list := range otherLists {
		out = reflect.AppendSlice(out, reflect.ValueOf(list))
	}
	return out.Interface()
}

// listSortUnique returns items sorted by the string-formatted value of each element, with
// items with the same strings deduplicated.
func listSortUnique(items []any) []any {
	m := make(container.Map[string, any], len(items))
	for _, item := range items {
		m.Add(fmt.Sprint(item), item)
	}
	return m.Values()
}

// list returns a new slice of elements from the argument list
// Useful for: {{- range List "a" "b" "c" -}}{{.}}{{end}}
func list(elements ...any) []any { return elements }

func index(obj any, indices ...any) (any, error) {
	v := reflect.ValueOf(obj)
	for _, idx := range indices {
		for v.Kind() == reflect.Interface || v.Kind() == reflect.Pointer {
			v = v.Elem()
		}
		if !v.IsValid() || v.IsZero() || v.IsNil() {
			return nil, nil
		}
		switch v.Kind() {
		case reflect.Array, reflect.Slice:
			v = v.Index(idx.(int))
		case reflect.Map:
			v = v.MapIndex(reflect.ValueOf(idx))
		default:
			return nil, fmt.Errorf("cannot index %T (%v)", obj, v.Kind())
		}
	}
	if !v.IsValid() || v.IsZero() || v.IsNil() {
		return nil, nil
	}
	return v.Interface(), nil
}

func replace(s string, oldNew ...string) string {
	return strings.NewReplacer(oldNew...).Replace(s)
}
