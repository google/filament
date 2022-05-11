/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package main

import (
	"bufio"
	"log"
	"os"
	"regexp"
	"strings"
)

func Parse(sourcePath string) []Scope {
	sourceFile, err := os.Open(sourcePath)
	if err != nil {
		log.Fatal(err)
	}
	defer sourceFile.Close()
	context := parserContext{}
	lineScanner := bufio.NewScanner(sourceFile)
	for lineNumber := 1; lineScanner.Scan(); lineNumber++ {
		context.scanCppCodeline(lineScanner.Text(), lineNumber)
	}
	if err := lineScanner.Err(); err != nil {
		log.Fatal(err)
	}
	return context.definitions
}

type Scope interface {
	BaseName() string
	QualifiedName() string
}

type StructField struct {
	Type           string
	Name           string
	DefaultValue   string
	SkipJson       bool
	SkipJavaScript bool
}

type StructDefinition struct {
	StructName string
	Qualifier  string
	Fields     []StructField
}

type EnumDefinition struct {
	EnumName  string
	Qualifier string
	Values    []string
}

type generalScope struct{}

func (defn StructDefinition) BaseName() string      { return defn.StructName }
func (defn StructDefinition) QualifiedName() string { return defn.Qualifier + defn.StructName }

func (defn EnumDefinition) BaseName() string      { return defn.EnumName }
func (defn EnumDefinition) QualifiedName() string { return defn.Qualifier + defn.EnumName }

func (defn generalScope) BaseName() string      { return "" }
func (defn generalScope) QualifiedName() string { return "" }

type parserContext struct {
	stack             []Scope
	definitions       []Scope
	insideComment     bool
	cppTokenizer      *regexp.Regexp
	assignmentMatcher *regexp.Regexp
	floatMatcher      *regexp.Regexp
	vecMatcher        *regexp.Regexp
}

func (context parserContext) generateQualifier() string {
	qualifier := ""
	for _, defn := range context.stack {
		if defn.BaseName() != "" {
			qualifier = qualifier + defn.BaseName() + "::"
		}
	}
	return qualifier
}

// Validate and transform the RHS of an assignment.
// For vectors, this converts curly braces into square brackets.
func (context parserContext) distillValue(cppvalue string, lineNumber int) string {
	cppvalue = strings.TrimSpace(cppvalue)

	// Remove trailing "f" from floats, which isn't allowed in JavaScript.
	if context.floatMatcher.MatchString(cppvalue) {
		cppvalue = context.floatMatcher.ReplaceAllString(cppvalue, "$1")
	}

	// There are many ways to declare vector values (multi-arg constructor, single-arg
	// constructor, curly braces with type, curly braces without type), so just poop out if
	// the syntax is anything other than "curly braces without type".
	if strings.Contains(cppvalue, "math::") || strings.Contains(cppvalue, "Color") {
		log.Fatalf("%d: vectors must have the form {x, y ...}", lineNumber)
	}

	// Assume it's a vector if there's a curly brace.
	if strings.Contains(cppvalue, "{") {
		if context.vecMatcher.MatchString(cppvalue) {
			cppvalue = context.vecMatcher.ReplaceAllString(cppvalue, "[$1]")
		} else {
			log.Fatalf("%d: vectors must have the form {x, y ...}", lineNumber)
		}
	}

	if len(cppvalue) == 0 {
		log.Fatalf("%d: empty value specified", lineNumber)
	}

	return cppvalue
}

func (context *parserContext) scanCppCodeline(codeline string, lineNumber int) {
	if context.cppTokenizer == nil {
		// https://github.com/google/re2/wiki/Syntax
		context.cppTokenizer = regexp.MustCompile(`((?:/\*)|(?:\*/)|(?:;)|(?://)|(?:\})|(?:\{))`)
		context.assignmentMatcher = regexp.MustCompile(`=(.*);`)
		context.floatMatcher = regexp.MustCompile(`(\-?[0-9]+\.[0-9]*)f?`)
		context.vecMatcher = regexp.MustCompile(`\{(\s*\-?[0-9\.]+\s*(,\s*\-?[0-9\.]+\s*){1,})\}`)
	}
	codeline = context.cppTokenizer.ReplaceAllString(codeline, " $1 ")
	scanner := bufio.NewScanner(strings.NewReader(codeline))
	scanner.Split(bufio.ScanWords)
	inPlaceDefinition := ""
	for scanner.Scan() {
		token := scanner.Text()
		switch {
		case token == "//":
			return
		case token == "/*":
			context.insideComment = true
		case token == "*/":
			context.insideComment = false
		case context.insideComment:
			// Do nothing inside a comment.
		case token == ";":
			// Do nothing.
		case token == "{":
			context.stack = append(context.stack, &generalScope{})
		case token == "}":
			depth := len(context.stack) - 1
			if depth < 0 {
				log.Fatalf("%d: bizarre nesting", lineNumber)
			}
			switch defn := context.stack[depth].(type) {
			case *generalScope:
				// Do nothing here since this scope is neither a struct definition
				// nor an enum definition. (e.g. it could be a namespace or initializer)
			default:
				inPlaceDefinition = defn.BaseName()
				context.definitions = append(context.definitions, defn)
			}
			context.stack = context.stack[:depth]
		case token == "struct":
			if !scanner.Scan() {
				log.Fatalf("%d: bizarre struct", lineNumber)
			}
			if !strings.Contains(codeline, "{") || strings.Contains(codeline, "}") {
				log.Fatalf("%d: bad formatting", lineNumber)
			}
			stackEntry := StructDefinition{scanner.Text(), context.generateQualifier(), nil}
			context.stack = append(context.stack, &stackEntry)
			return
		case token == "enum":
			if !scanner.Scan() || scanner.Text() != "class" || !scanner.Scan() {
				log.Fatalf("%d: bad enum", lineNumber)
			}
			if !strings.Contains(codeline, "{") || strings.Contains(codeline, "}") {
				log.Fatalf("%d: bad formatting", lineNumber)
			}
			stackEntry := EnumDefinition{scanner.Text(), context.generateQualifier(), nil}
			context.stack = append(context.stack, &stackEntry)
			return
		case len(context.stack) > 1:
			depth := len(context.stack) - 1
			switch defn := context.stack[depth].(type) {
			case *StructDefinition:
				scanner.Scan()
				defaultValueTokens := context.assignmentMatcher.FindStringSubmatch(codeline)
				defaultValue := ""
				if len(defaultValueTokens) > 0 {
					defaultValue = defaultValueTokens[1]
				}
				defaultValue = context.distillValue(defaultValue, lineNumber)
				var field StructField
				skipJson := strings.Contains(codeline, `%codegen_skip_json%`)
				skipJs := strings.Contains(codeline, `%codegen_skip_javascript%`)
				if inPlaceDefinition != "" {
					field = StructField{inPlaceDefinition, token, defaultValue, skipJson, skipJs}
				} else {
					field = StructField{token, scanner.Text(), defaultValue, skipJson, skipJs}
				}
				defn.Fields = append(defn.Fields, field)
				return
			case *EnumDefinition:
				if strings.Contains(codeline, "=") {
					log.Fatalf("%d: custom values are not allowed", lineNumber)
				}
				defn.Values = append(defn.Values, strings.Trim(token, ","))
				return
			}
		}
	}
	if err := scanner.Err(); err != nil {
		log.Fatalf("%d: %s", lineNumber, err)
	}
}
