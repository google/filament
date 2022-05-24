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

package parse

import (
	"bufio"
	"log"
	"os"
	"regexp"
	"strings"
)

// Consumes a C++ header file and produces a type database.
func Parse(sourcePath string) []TypeDefinition {
	sourceFile, err := os.Open(sourcePath)
	if err != nil {
		log.Fatal(err)
	}
	defer sourceFile.Close()

	// In the first pass, gather all block-style comments.
	context := parserContext{}
	context.commentBlocks = gatherCommentBlocks(sourceFile)
	sourceFile.Seek(0, 0)

	// In the second pass, pry apart each C++ codeline.
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

type TypeDefinition interface {
	BaseName() string
	QualifiedName() string
	Parent() TypeDefinition
}

type StructField struct {
	TypeString   string
	Name         string
	DefaultValue string
	Description  string
	LineNumber   int
	EmitterFlags map[string]struct{}
	CustomType   TypeDefinition
}

type StructDefinition struct {
	name        string
	qualifier   string
	Fields      []StructField
	Description string
	parent      TypeDefinition
}

func (defn StructDefinition) BaseName() string       { return defn.name }
func (defn StructDefinition) QualifiedName() string  { return defn.qualifier + defn.name }
func (defn StructDefinition) Parent() TypeDefinition { return defn.parent }

type EnumValue struct {
	Description string
	Name        string
}

type EnumDefinition struct {
	name        string
	qualifier   string
	Values      []EnumValue
	Description string
	parent      TypeDefinition
}

func (defn EnumDefinition) BaseName() string       { return defn.name }
func (defn EnumDefinition) QualifiedName() string  { return defn.qualifier + defn.name }
func (defn EnumDefinition) Parent() TypeDefinition { return defn.parent }

type Documented interface{ GetDoc() string }

func (defn EnumDefinition) GetDoc() string   { return defn.Description }
func (defn StructDefinition) GetDoc() string { return defn.Description }
func (field StructField) GetDoc() string     { return field.Description }
func (value EnumValue) GetDoc() string       { return value.Description }

type generalScope struct{}

type scope interface {
	BaseName() string
	QualifiedName() string
	Parent() TypeDefinition
}

func (defn generalScope) BaseName() string       { return "" }
func (defn generalScope) QualifiedName() string  { return "" }
func (defn generalScope) Parent() TypeDefinition { return nil }

type parserContext struct {
	definitions      []TypeDefinition
	stack            []scope
	insideComment    bool
	commentBlocks    map[int]string
	cppTokenizer     *regexp.Regexp
	floatMatcher     *regexp.Regexp
	vectorMatcher    *regexp.Regexp
	fieldParser      *regexp.Regexp
	fieldDescParser  *regexp.Regexp
	customFlagFinder *regexp.Regexp
}

// https://github.com/google/re2/wiki/Syntax
func (context *parserContext) compileRegexps() {
	context.cppTokenizer = regexp.MustCompile(`((?:/\*)|(?:\*/)|(?:;)|(?://)|(?:\})|(?:\{))`)
	context.floatMatcher = regexp.MustCompile(`(\-?[0-9]+\.[0-9]*)f?`)
	context.vectorMatcher = regexp.MustCompile(`\{(\s*\-?[0-9\.]+\s*(,\s*\-?[0-9\.]+\s*){1,})\}`)
	context.customFlagFinder = regexp.MustCompile(`\s*\%codegen_([a-zA-Z0-9_]+)\%\s*`)

	const kFieldType = `(?P<type>.*)`
	const kFieldName = `(?P<name>[A-Za-z0-9_]+)`
	const kFieldValue = `(?P<value>(.*?))`
	const kFieldDesc = `(?://\s*\!\<\s*(?P<description>.*))?`
	context.fieldParser = regexp.MustCompile(
		`^\s*` + kFieldType + `\s+` + kFieldName + `\s*=\s*` + kFieldValue + `\s*;\s*` + kFieldDesc)

	context.fieldDescParser = regexp.MustCompile(`(?://\s*\!\<\s*(.*))`)
}

// Creates a mapping from line numbers to strings, where the strings are entire block comments
// and the line numbers correspond to the last line of each block comment.
func gatherCommentBlocks(sourceFile *os.File) map[int]string {
	comments := make(map[int]string)
	scanner := bufio.NewScanner(sourceFile)
	var comment = ""
	var indention = 0
	for lineNumber := 1; scanner.Scan(); lineNumber++ {
		codeline := scanner.Text()
		if strings.Contains(codeline, `/**`) {
			indention = strings.Index(codeline, `/**`)
			comment = codeline[indention:] + "\n"
			continue
		}
		if comment != "" {
			if len(codeline) > indention {
				codeline = codeline[indention:]
			}
			comment += codeline + "\n"
			if strings.Contains(codeline, `*/`) {
				comments[lineNumber] = comment
				comment = ""
			}
		}
	}
	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}
	return comments
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

func (context parserContext) findParent() TypeDefinition {
	for i := len(context.stack) - 1; i >= 0; i-- {
		switch defn := context.stack[i].(type) {
		case *StructDefinition, *EnumDefinition:
			var result TypeDefinition = defn
			return result
		}
	}
	return nil
}

// Annotates struct fields that have custom types (i.e. enums or structs).
func (context parserContext) addTypeQualifiers() {
	typeMap := make(map[string]TypeDefinition)
	for _, defn := range context.definitions {
		typeMap[defn.QualifiedName()] = defn
	}
	for _, defn := range context.definitions {
		structDefn, isStruct := defn.(*StructDefinition)
		if !isStruct {
			continue
		}
		for fieldIndex, field := range structDefn.Fields {
			// Extract the namespace prefix (if any) explicitly specified for this field type.
			var namespace string
			localTypeName := field.TypeString
			if index := strings.LastIndex(field.TypeString, "::"); index > -1 {
				namespace = field.TypeString[:index]
				localTypeName = field.TypeString[index+2:]
			}

			// Prepend additional qualifiers to the type string by searching upward through
			// the current namespace hierarchy, and looking for a match.
			mutable := &structDefn.Fields[fieldIndex]
			for ancestor := defn; ; ancestor = ancestor.Parent() {
				var qualified string
				if namespace != "" && strings.HasSuffix(ancestor.QualifiedName(), namespace) {
					qualified = ancestor.QualifiedName() + "::" + localTypeName
				} else {
					qualified = ancestor.QualifiedName() + "::" + field.TypeString
				}
				if fieldType, found := typeMap[qualified]; found {
					mutable.TypeString = qualified
					mutable.CustomType = fieldType
					break
				}
				if ancestor.Parent() == nil {
					if fieldType, found := typeMap[field.TypeString]; found {
						mutable.CustomType = fieldType
					}
					break
				}
			}

			if mutable.CustomType == nil {
				continue
			}

			// Prepend additional qualifiers to the value string if it is a known enum.
			var fieldType TypeDefinition = structDefn.Fields[fieldIndex].CustomType
			enumDefn, isEnum := fieldType.(*EnumDefinition)
			if isEnum {
				selectedEnum := field.DefaultValue
				if index := strings.LastIndex(field.DefaultValue, "::"); index > -1 {
					selectedEnum = field.DefaultValue[index+2:]
				}
				mutable.DefaultValue = enumDefn.QualifiedName() + "::" + selectedEnum
			}
		}
	}
}

// Validates and transforms the RHS of an assignment.
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
			LineNumber: lineNumber,
		}

		// Extract all custom flags into a string set. These are special backend-specific directives
		// delimited by percent signs, e.g. %codegen_skip_javascript%
		if matches := context.customFlagFinder.FindAllStringSubmatch(codeline, -1); matches != nil {
			field.EmitterFlags = make(map[string]struct{}, len(matches))
			for _, flag := range matches {
				field.EmitterFlags[flag[1]] = struct{}{}
			}
		}
		codeline = context.customFlagFinder.ReplaceAllString(codeline, "")

		// Normally when we're inside a struct, the first word on each codeline is the field type,
		// and the second word is the field name. However if a nested struct is defined, then the
		// type is potentially anonymous and the first word is the field name.
		if !scanner.Scan() {
			log.Fatalf("%d: bad struct field", lineNumber)
		}

		// Check if this field type has an in place definition. For example:
		//     struct OuterType {
		//         int foo;
		//         struct Baz { int bar } baz;
		//     };
		// In the above example, inPlaceDefinition == Baz.
		if inPlaceDefinition != "" {
			field.TypeString = inPlaceDefinition
			field.Name = firstWord
			defn.Fields = append(defn.Fields, field)
			return
		}

		if !context.fieldParser.MatchString(codeline) {
			log.Fatalf("%d: unexpected form in struct field declaration", lineNumber)
		}

		// To make the regex usage somewhat readable, extract the named subgroups into a map rather
		// than referring to each result by index.
		subexpList := context.fieldParser.FindStringSubmatch(codeline)
		subexpMap := make(map[string]string)
		for i, name := range context.fieldParser.SubexpNames() {
			if i != 0 && name != "" {
				subexpMap[name] = subexpList[i]
			}
		}

		field.TypeString = subexpMap["type"]
		field.Name = subexpMap["name"]
		field.Description = subexpMap["description"]
		field.DefaultValue = context.distillValue(subexpMap["value"], lineNumber)

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
			if !context.insideComment {
				log.Fatalf("%d: strange comment", lineNumber)
			}
			context.insideComment = false
		case context.insideComment:
			// Do nothing.
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
			stackEntry := StructDefinition{
				name:        scanner.Text(),
				qualifier:   context.generateQualifier(),
				Description: context.commentBlocks[lineNumber-1],
				parent:      context.findParent(),
			}
			context.stack = append(context.stack, &stackEntry)
			return
		case token == "enum":
			if !scanner.Scan() || scanner.Text() != "class" || !scanner.Scan() {
				log.Fatalf("%d: bad enum", lineNumber)
			}
			if !strings.Contains(codeline, "{") || strings.Contains(codeline, "}") {
				log.Fatalf("%d: bad formatting", lineNumber)
			}
			stackEntry := EnumDefinition{
				name:        scanner.Text(),
				qualifier:   context.generateQualifier(),
				Description: context.commentBlocks[lineNumber-1],
				parent:      context.findParent(),
			}
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
				value := EnumValue{
					Name: strings.Trim(token, ","),
				}
				if matches := context.fieldDescParser.FindStringSubmatch(codeline); matches != nil {
					value.Description = matches[1]
				}
				defn.Values = append(defn.Values, value)
				return
			}
		}
	}
	if err := scanner.Err(); err != nil {
		log.Fatalf("%d: %s", lineNumber, err)
	}
}
