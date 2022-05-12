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
	context.addTypeQualifiers()
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
	Description    string
	SkipJson       bool
	SkipJavaScript bool
	LineNumber     int
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
	stack            []Scope
	definitions      []Scope
	insideComment    bool
	cppTokenizer     *regexp.Regexp
	rhsMatcher       *regexp.Regexp
	floatMatcher     *regexp.Regexp
	vectorMatcher    *regexp.Regexp
	fieldDescMatcher *regexp.Regexp
}

// https://github.com/google/re2/wiki/Syntax
func (context *parserContext) compileRegexps() {
	context.cppTokenizer = regexp.MustCompile(`((?:/\*)|(?:\*/)|(?:;)|(?://)|(?:\})|(?:\{))`)
	context.rhsMatcher = regexp.MustCompile(`=(.*);`)
	context.floatMatcher = regexp.MustCompile(`(\-?[0-9]+\.[0-9]*)f?`)
	context.vectorMatcher = regexp.MustCompile(`\{(\s*\-?[0-9\.]+\s*(,\s*\-?[0-9\.]+\s*){1,})\}`)
	context.fieldDescMatcher = regexp.MustCompile(`//.*\!\<\s*(.*)`)
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

func (context parserContext) addTypeQualifiers() {
	typeMap := make(map[string]Scope)
	for _, defn := range context.definitions {
		typeMap[defn.BaseName()] = defn
	}
	for _, defn := range context.definitions {
		switch structDefn := defn.(type) {
		case *StructDefinition:
			for fieldIndex, field := range structDefn.Fields {
				if typeDefn, found := typeMap[field.Type]; found {
					mutable := &structDefn.Fields[fieldIndex]

					// If this is an enum, add qualifier to the RHS of the assignment.
					// Remember that all enums must be class enums.
					if end := strings.LastIndex(field.DefaultValue, "::"); end != -1 {
						enumValue := field.DefaultValue[end+2:]
						mutable.DefaultValue = typeDefn.QualifiedName() + "::" + enumValue
					}

					mutable.Type = typeDefn.QualifiedName()
					continue
				}
				if isSimpleType(field.Type) {
					continue
				}
				// If this field is neither a simple type nor a type that was defined in the source
				// file, then emit a warning.  Unknown custom types are error prone because it is
				// difficult to know if additional scoping qualifiers are required, or how they are
				// bound in the target language.
				log.Printf("%d: WARNING: %v is an unrecognized type", field.LineNumber, field.Type)
			}
		}
	}
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
		if context.vectorMatcher.MatchString(cppvalue) {
			cppvalue = context.vectorMatcher.ReplaceAllString(cppvalue, "[$1]")
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
		context.compileRegexps()
	}
	codeline = context.cppTokenizer.ReplaceAllString(codeline, " $1 ")
	scanner := bufio.NewScanner(strings.NewReader(codeline))
	scanner.Split(bufio.ScanWords)
	inPlaceDefinition := ""

	scanStructField := func(defn *StructDefinition, firstWord string) {
		var field = StructField{
			SkipJson:       strings.Contains(codeline, `%codegen_skip_json%`),
			SkipJavaScript: strings.Contains(codeline, `%codegen_skip_javascript%`),
			LineNumber:     lineNumber,
		}

		// Normally when we're inside a struct, the first word on each codeline is the field type,
		// and the second word is the field name. However if a nested struct is defined, then the
		// type is potentially anonymous and the first word is the field name.
		if !scanner.Scan() {
			log.Fatalf("%d: bad struct field", lineNumber)
		}
		if inPlaceDefinition == "" {
			// Field has a "normal" type.
			field.Type = firstWord
			field.Name = scanner.Text()
		} else {
			// Field has a nested type.
			field.Type = inPlaceDefinition
			field.Name = firstWord
		}

		// Use a regex to extract the doxygen comment.
		if matches := context.fieldDescMatcher.FindStringSubmatch(codeline); len(matches) > 0 {
			field.Description = matches[1]
		}

		// Use a regex to extract the right hand side in the default value assignment, then
		// do some custom processing via distillValue().
		if matches := context.rhsMatcher.FindStringSubmatch(codeline); len(matches) > 0 {
			field.DefaultValue = context.distillValue(matches[1], lineNumber)
		}

		defn.Fields = append(defn.Fields, field)
	}

	for scanner.Scan() {
		depth := len(context.stack) - 1
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
			if depth < 0 {
				log.Fatalf("%d: bizarre nesting", lineNumber)
			}
			switch defn := context.stack[depth].(type) {
			case *StructDefinition, *EnumDefinition:
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
		case depth > 0:
			switch defn := context.stack[depth].(type) {
			case *StructDefinition:
				scanStructField(defn, token)
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

func isSimpleType(cpptype string) bool {
	if strings.HasPrefix(cpptype, "math::") {
		return true
	}
	switch cpptype {
	case "bool", "int", "uint8_t", "uint16_t", "uint32_t", "uint64_t":
		return true
	case "float", "double":
		return true
	case "LinearColor", "LinearColorA":
		return true
	}
	return false
}
