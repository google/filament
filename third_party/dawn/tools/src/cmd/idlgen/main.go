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

// idlgen is a tool used to generate code from WebIDL files and a golang
// template file
package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"reflect"
	"strings"
	"text/template"
	"unicode"

	"github.com/ben-clayton/webidlparser/ast"
	"github.com/ben-clayton/webidlparser/parser"
)

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
idlgen is a tool used to generate code from WebIDL files and a golang
template file

Usage:
  idlgen --template=<template-path> --output=<output-path> <idl-file> [<idl-file>...]`)
	os.Exit(1)
}

func run() error {
	var templatePath string
	var outputPath string
	flag.StringVar(&templatePath, "template", "", "the template file run with the parsed WebIDL files")
	flag.StringVar(&outputPath, "output", "", "the output file")
	flag.Parse()

	idlFiles := flag.Args()

	// Check all required arguments are provided
	if templatePath == "" || outputPath == "" || len(idlFiles) == 0 {
		showUsage()
	}

	// Open up the output file
	out := os.Stdout
	if outputPath != "" {
		dir := filepath.Dir(outputPath)
		if err := os.MkdirAll(dir, 0777); err != nil {
			return fmt.Errorf("failed to create output directory '%v'", dir)
		}
		file, err := os.Create(outputPath)
		if err != nil {
			return fmt.Errorf("failed to open output file '%v'", outputPath)
		}
		out = file
		defer file.Close()
	}

	// Read the template file
	tmpl, err := ioutil.ReadFile(templatePath)
	if err != nil {
		return fmt.Errorf("failed to open template file '%v'", templatePath)
	}

	// idl is the combination of the parsed idlFiles
	idl := &ast.File{}

	// Parse each of the WebIDL files and add the declarations to idl
	for _, path := range idlFiles {
		content, err := ioutil.ReadFile(path)
		if err != nil {
			return fmt.Errorf("failed to open file '%v'", path)
		}
		fileIDL := parser.Parse(string(content))
		if numErrs := len(fileIDL.Errors); numErrs != 0 {
			errs := make([]string, numErrs)
			for i, e := range fileIDL.Errors {
				errs[i] = e.Message
			}
			return fmt.Errorf("errors found while parsing %v:\n%v", path, strings.Join(errs, "\n"))
		}
		idl.Declarations = append(idl.Declarations, fileIDL.Declarations...)
	}

	// Initialize the generator
	g := generator{t: template.New(templatePath)}
	g.workingDir = filepath.Dir(templatePath)
	g.funcs = map[string]interface{}{
		// Functions exposed to the template
		"AttributesOf":               attributesOf,
		"ConstantsOf":                constantsOf,
		"EnumEntryName":              enumEntryName,
		"Eval":                       g.eval,
		"HasAnnotation":              hasAnnotation,
		"FlattenedAttributesOf":      g.flattenedAttributesOf,
		"FlattenedConstantsOf":       g.flattenedConstantsOf,
		"FlattenedMethodsOf":         g.flattenedMethodsOf,
		"Include":                    g.include,
		"IsBasicLiteral":             is(ast.BasicLiteral{}),
		"IsInitializer":              isInitializer,
		"IsDefaultDictionaryLiteral": is(ast.DefaultDictionaryLiteral{}),
		"IsDictionary":               is(ast.Dictionary{}),
		"IsEnum":                     is(ast.Enum{}),
		"IsInterface":                is(ast.Interface{}),
		"IsInterfaceOrNamespace":     is(ast.Interface{}, ast.Namespace{}),
		"IsMember":                   is(ast.Member{}),
		"IsNamespace":                is(ast.Namespace{}),
		"IsNullableType":             is(ast.NullableType{}),
		"IsParametrizedType":         is(ast.ParametrizedType{}),
		"IsRecordType":               is(ast.RecordType{}),
		"IsSequenceType":             is(ast.SequenceType{}),
		"IsTypedef":                  is(ast.Typedef{}),
		"IsTypeName":                 is(ast.TypeName{}),
		"IsUndefinedType":            isUndefinedType,
		"IsUnionType":                is(ast.UnionType{}),
		"Lookup":                     g.lookup,
		"MethodsOf":                  methodsOf,
		"ReturnsPromise":             returnsPromise,
		"SetlikeOf":                  setlikeOf,
		"Title":                      strings.Title,
	}
	t, err := g.t.
		Option("missingkey=invalid").
		Funcs(g.funcs).
		Parse(string(tmpl))
	if err != nil {
		return fmt.Errorf("failed to parse template file '%v': %w", templatePath, err)
	}

	// simplify the definitions in the WebIDL before passing this to the template
	idl, declarations := simplify(idl)

	// Patch the IDL for the differences we need compared to the upstream IDL.
	patch(idl, declarations)
	g.declarations = declarations

	// Write the file header
	fmt.Fprintf(out, header, strings.Join(os.Args[1:], "\n//   "))

	// Execute the template
	return t.Execute(out, idl)
}

// declarations is a map of WebIDL declaration name to its AST node.
type declarations map[string]ast.Decl

// nameOf returns the name of the AST node n.
// Returns an empty string if the node is not named.
func nameOf(n ast.Node) string {
	switch n := n.(type) {
	case *ast.Namespace:
		return n.Name
	case *ast.Interface:
		return n.Name
	case *ast.Dictionary:
		return n.Name
	case *ast.Enum:
		return n.Name
	case *ast.Typedef:
		return n.Name
	case *ast.Mixin:
		return n.Name
	case *ast.Includes:
		return ""
	default:
		panic(fmt.Errorf("unhandled AST declaration %T", n))
	}
}

// simplify processes the AST 'in', returning a new AST that:
// * Has all partial interfaces merged into a single interface.
// * Has all mixins flattened into their place of use.
// * Has all the declarations ordered in dependency order (leaf first)
// simplify also returns the map of declarations in the AST.
func simplify(in *ast.File) (*ast.File, declarations) {
	s := simplifier{
		declarations: declarations{},
		registered:   map[string]bool{},
		out:          &ast.File{},
	}

	// Walk the IDL declarations to merge together partial interfaces and embed
	// mixins into their uses.
	{
		interfaces := map[string]*ast.Interface{}
		mixins := map[string]*ast.Mixin{}
		includes := []*ast.Includes{}
		enums := map[string]*ast.Enum{}
		dicts := map[string]*ast.Dictionary{}
		for _, d := range in.Declarations {
			switch d := d.(type) {
			case *ast.Interface:
				if i, ok := interfaces[d.Name]; ok {
					// Merge partial body into one interface
					i.Members = append(i.Members, d.Members...)
				} else {
					clone := *d
					d := &clone
					interfaces[d.Name] = d
					s.declarations[d.Name] = d
				}
			case *ast.Mixin:
				mixins[d.Name] = d
				s.declarations[d.Name] = d
			case *ast.Includes:
				includes = append(includes, d)
			case *ast.Enum:
				if e, ok := enums[d.Name]; ok {
					// Merge partial enums into one enum
					e.Values = append(e.Values, d.Values...)
				} else {
					clone := *d
					d := &clone
					enums[d.Name] = d
					s.declarations[d.Name] = d
				}
			case *ast.Dictionary:
				if e, ok := dicts[d.Name]; ok {
					// Merge partial dictionaries into one dictionary
					e.Members = append(e.Members, d.Members...)
				} else {
					clone := *d
					d := &clone
					dicts[d.Name] = d
					s.declarations[d.Name] = d
				}
			default:
				if name := nameOf(d); name != "" {
					s.declarations[nameOf(d)] = d
				}
			}
		}

		// Merge mixin into interface
		for _, include := range includes {
			i, ok := interfaces[include.Name]
			if !ok {
				panic(fmt.Errorf("%v includes %v, but %v is not an interface", include.Name, include.Source, include.Name))
			}
			m, ok := mixins[include.Source]
			if !ok {
				panic(fmt.Errorf("%v includes %v, but %v is not an mixin", include.Name, include.Source, include.Source))
			}
			// Merge mixin into the interface
			for _, member := range m.Members {
				if member, ok := member.(*ast.Member); ok {
					i.Members = append(i.Members, member)
				}
			}
		}
	}

	// Now traverse the declarations in to produce the dependency-ordered
	// output `s.out`.
	for _, d := range in.Declarations {
		if name := nameOf(d); name != "" {
			s.visit(s.declarations[nameOf(d)])
		}
	}

	return s.out, s.declarations
}

// simplifier holds internal state for simplify()
type simplifier struct {
	// all AST declarations
	declarations declarations
	// set of visited declarations
	registered map[string]bool
	// the dependency-ordered output
	out *ast.File
}

// visit traverses the AST declaration 'd' adding all dependent declarations to
// s.out.
func (s *simplifier) visit(d ast.Decl) {
	register := func(name string) bool {
		if s.registered[name] {
			return true
		}
		s.registered[name] = true
		return false
	}
	switch d := d.(type) {
	case *ast.Namespace:
		if register(d.Name) {
			return
		}
		for _, m := range d.Members {
			if m, ok := m.(*ast.Member); ok {
				s.visitType(m.Type)
				for _, p := range m.Parameters {
					s.visitType(p.Type)
				}
			}
		}
	case *ast.Interface:
		if register(d.Name) {
			return
		}
		if d, ok := s.declarations[d.Inherits]; ok {
			s.visit(d)
		}
		for _, m := range d.Members {
			if m, ok := m.(*ast.Member); ok {
				s.visitType(m.Type)
				for _, p := range m.Parameters {
					s.visitType(p.Type)
				}
			}
		}
	case *ast.Dictionary:
		if register(d.Name) {
			return
		}
		if d, ok := s.declarations[d.Inherits]; ok {
			s.visit(d)
		}
		for _, m := range d.Members {
			s.visitType(m.Type)
			for _, p := range m.Parameters {
				s.visitType(p.Type)
			}
		}
	case *ast.Typedef:
		if register(d.Name) {
			return
		}
		s.visitType(d.Type)
	case *ast.Mixin:
		if register(d.Name) {
			return
		}
		for _, m := range d.Members {
			if m, ok := m.(*ast.Member); ok {
				s.visitType(m.Type)
				for _, p := range m.Parameters {
					s.visitType(p.Type)
				}
			}
		}
	case *ast.Enum:
		if register(d.Name) {
			return
		}
	case *ast.Includes:
		if register(d.Name) {
			return
		}
	default:
		panic(fmt.Errorf("unhandled AST declaration %T", d))
	}

	s.out.Declarations = append(s.out.Declarations, d)
}

// visitType traverses the AST type 't' adding all dependent declarations to
// s.out.
func (s *simplifier) visitType(t ast.Type) {
	switch t := t.(type) {
	case *ast.TypeName:
		if d, ok := s.declarations[t.Name]; ok {
			s.visit(d)
		}
	case *ast.UnionType:
		for _, t := range t.Types {
			s.visitType(t)
		}
	case *ast.ParametrizedType:
		for _, t := range t.Elems {
			s.visitType(t)
		}
	case *ast.NullableType:
		s.visitType(t.Type)
	case *ast.SequenceType:
		s.visitType(t.Elem)
	case *ast.RecordType:
		s.visitType(t.Elem)
	default:
		panic(fmt.Errorf("unhandled AST type %T", t))
	}
}

func patch(idl *ast.File, decl declarations) {
	// Add [SameObject] to GPUDevice.lost
	for _, member := range decl["GPUDevice"].(*ast.Interface).Members {
		if m := member.(*ast.Member); m != nil && m.Name == "lost" {
			annotation := &ast.Annotation{}
			annotation.Name = "SameObject"
			m.Annotations = append(m.Annotations, annotation)
		}
	}
}

// generator holds the template generator state
type generator struct {
	// the root template
	t *template.Template
	// the working directory
	workingDir string
	// map of function name to function exposed to the template executor
	funcs map[string]interface{}
	// dependency-sorted declarations
	declarations declarations
}

// eval executes the sub-template with the given name and arguments, returning
// the generated output
// args can be a single argument:
//
//	arg[0]
//
// or a list of name-value pairs:
//
//	(args[0]: name, args[1]: value), (args[2]: name, args[3]: value)...
func (g *generator) eval(template string, args ...interface{}) (string, error) {
	target := g.t.Lookup(template)
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
		return "", fmt.Errorf("while evaluating '%v': %v", template, err)
	}
	return sb.String(), nil
}

// lookup returns the declaration with the given name, or nil if not found.
func (g *generator) lookup(name string) ast.Decl {
	return g.declarations[name]
}

// include loads the template with the given path, importing the declarations
// into the scope of the current template.
func (g *generator) include(path string) (string, error) {
	t, err := g.t.
		Option("missingkey=invalid").
		Funcs(g.funcs).
		ParseFiles(filepath.Join(g.workingDir, path))
	if err != nil {
		return "", err
	}
	g.t.AddParseTree(path, t.Tree)
	return "", nil
}

// Map is a simple generic key-value map, which can be used in the template
type Map map[interface{}]interface{}

func newMap() Map { return Map{} }

// Put adds the key-value pair into the map.
// Put always returns an empty string so nothing is printed in the template.
func (m Map) Put(key, value interface{}) string {
	m[key] = value
	return ""
}

// Get looks up and returns the value with the given key. If the map does not
// contain the given key, then nil is returned.
func (m Map) Get(key interface{}) interface{} {
	return m[key]
}

// is returns a function that returns true if the value passed to the function
// matches any of the types of the objects in 'prototypes'.
func is(prototypes ...interface{}) func(interface{}) bool {
	types := make([]reflect.Type, len(prototypes))
	for i, p := range prototypes {
		types[i] = reflect.TypeOf(p)
	}
	return func(v interface{}) bool {
		ty := reflect.TypeOf(v)
		for _, rty := range types {
			if ty == rty || ty == reflect.PtrTo(rty) {
				return true
			}
		}
		return false
	}
}

// isInitializer returns true if the object is a constructor ast.Member.
func isInitializer(v interface{}) bool {
	if member, ok := v.(*ast.Member); ok {
		if ty, ok := member.Type.(*ast.TypeName); ok {
			return ty.Name == "constructor"
		}
	}
	return false
}

// isUndefinedType returns true if the type is 'undefined'
func isUndefinedType(ty ast.Type) bool {
	if ty, ok := ty.(*ast.TypeName); ok {
		return ty.Name == "undefined"
	}
	return false
}

// enumEntryName formats the enum entry name 's' for use in a C++ enum.
func enumEntryName(s string) string {
	return "k" + strings.ReplaceAll(pascalCase(strings.Trim(s, `"`)), "-", "")
}

func findAnnotation(list []*ast.Annotation, name string) *ast.Annotation {
	for _, annotation := range list {
		if annotation.Name == name {
			return annotation
		}
	}
	return nil
}

func hasAnnotation(obj interface{}, name string) bool {
	switch obj := obj.(type) {
	case *ast.Interface:
		return findAnnotation(obj.Annotations, name) != nil
	case *ast.Member:
		return findAnnotation(obj.Annotations, name) != nil
	case *ast.Namespace:
		return findAnnotation(obj.Annotations, name) != nil
	case *ast.Parameter:
		return findAnnotation(obj.Annotations, name) != nil
	case *ast.Typedef:
		return findAnnotation(obj.Annotations, name) != nil || findAnnotation(obj.TypeAnnotations, name) != nil
	}
	panic("Unhandled AST node type in hasAnnotation")
}

// Method describes a WebIDL interface method
type Method struct {
	// Name of the method
	Name string
	// The list of overloads of the method
	Overloads []*ast.Member
}

// methodsOf returns all the methods of the given WebIDL interface.
func methodsOf(obj interface{}) []*Method {
	iface, ok := obj.(*ast.Interface)
	if !ok {
		return nil
	}
	byName := map[string]*Method{}
	out := []*Method{}
	for _, member := range iface.Members {
		member := member.(*ast.Member)
		if !member.Const && !member.Attribute && !isInitializer(member) {
			if method, ok := byName[member.Name]; ok {
				method.Overloads = append(method.Overloads, member)
			} else {
				method = &Method{
					Name:      member.Name,
					Overloads: []*ast.Member{member},
				}
				byName[member.Name] = method
				out = append(out, method)
			}
		}
	}
	return out
}

// flattenedMethodsOf returns all the methods of the given WebIDL
// interface or namespace, as well as all the methods of the full inheritance
// chain
func (g *generator) flattenedMethodsOf(obj interface{}) []*Method {
	switch obj := obj.(type) {
	case *ast.Interface:
		out := methodsOf(obj)
		if base := g.lookup(obj.Inherits); base != nil {
			out = append(out, g.flattenedMethodsOf(base)...)
		}
		return out
	default:
		return methodsOf(obj)
	}
}

// attributesOf returns all the attributes of the given WebIDL interface or
// namespace.
func attributesOf(obj interface{}) []*ast.Member {
	out := []*ast.Member{}
	add := func(m interface{}) {
		if m := m.(*ast.Member); m.Attribute {
			out = append(out, m)
		}
	}

	switch obj := obj.(type) {
	case *ast.Interface:
		for _, m := range obj.Members {
			add(m)
		}
	case *ast.Namespace:
		for _, m := range obj.Members {
			add(m)
		}
	default:
		return nil
	}
	return out
}

// flattenedAttributesOf returns all the attributes of the given WebIDL
// interface or namespace, as well as all the attributes of the full inheritance
// chain
func (g *generator) flattenedAttributesOf(obj interface{}) []*ast.Member {
	switch obj := obj.(type) {
	case *ast.Interface:
		out := attributesOf(obj)
		if base := g.lookup(obj.Inherits); base != nil {
			out = append(out, g.flattenedAttributesOf(base)...)
		}
		return out
	default:
		return attributesOf(obj)
	}
}

// constantsOf returns all the constant values of the given WebIDL interface or
// namespace.
func constantsOf(obj interface{}) []*ast.Member {
	out := []*ast.Member{}
	add := func(m interface{}) {
		if m := m.(*ast.Member); m.Const {
			out = append(out, m)
		}
	}
	switch obj := obj.(type) {
	case *ast.Interface:
		for _, m := range obj.Members {
			add(m)
		}
	case *ast.Namespace:
		for _, m := range obj.Members {
			add(m)
		}
	default:
		return nil
	}
	return out
}

// flattenedConstantsOf returns all the constants of the given WebIDL
// interface or namespace, as well as all the constants of the full inheritance
// chain
func (g *generator) flattenedConstantsOf(obj interface{}) []*ast.Member {
	switch obj := obj.(type) {
	case *ast.Interface:
		out := constantsOf(obj)
		if base := g.lookup(obj.Inherits); base != nil {
			out = append(out, g.flattenedConstantsOf(base)...)
		}
		return out
	default:
		return constantsOf(obj)
	}
}

// isPromiseType returns true if the type is 'Promise<T>'
func isPromiseType(ty ast.Type) bool {
	if ty, ok := ty.(*ast.ParametrizedType); ok {
		return ty.Name == "Promise"
	}
	return false
}

// returnsPromise returns true if the ast.Method returns a Promise.
func returnsPromise(obj interface{}) bool {
	method, ok := obj.(*Method)
	if !ok {
		panic("Unhandled AST node type in hasAnnotation")
	}

	firstIsPromise := isPromiseType(method.Overloads[0].Type)
	for _, o := range method.Overloads {
		if isPromiseType(o.Type) != firstIsPromise {
			panic("IDL has overloads that are not consistently returning Promises")
		}
	}

	return firstIsPromise
}

// setlikeOf returns the setlike ast.Pattern, if obj is a setlike interface.
func setlikeOf(obj interface{}) *ast.Pattern {
	iface, ok := obj.(*ast.Interface)
	if !ok {
		return nil
	}
	for _, pattern := range iface.Patterns {
		if pattern.Type == ast.Setlike {
			return pattern
		}
	}
	return nil
}

// pascalCase returns the snake-case string s transformed into 'PascalCase',
// Rules:
// * The first letter of the string is capitalized
// * Characters following an underscore, hyphen or number are capitalized
// * Underscores are removed from the returned string
// See: https://en.wikipedia.org/wiki/Camel_case
func pascalCase(s string) string {
	b := strings.Builder{}
	upper := true
	for _, r := range s {
		if r == '_' || r == '-' {
			upper = true
			continue
		}
		if upper {
			b.WriteRune(unicode.ToUpper(r))
			upper = false
		} else {
			b.WriteRune(r)
		}
		if unicode.IsNumber(r) {
			upper = true
		}
	}
	return b.String()
}

const header = `// Copyright 2021 The Dawn & Tint Authors
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

////////////////////////////////////////////////////////////////////////////////
// File generated by tools/cmd/idlgen.go, with the arguments:
//   %v
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

`
