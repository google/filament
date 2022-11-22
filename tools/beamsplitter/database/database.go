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

package database

import (
	"beamsplitter/parse"
	"bufio"
	"io"
	"log"
	"regexp"
	"strings"
)

func Create(root *parse.RootNode, contents string) []TypeDefinition {
	context := gatherContext{}
	context.compileRegexps()

	// Line numbers are 1-based in beamsplitter, so we add an extra newline at the top.
	context.codelines = strings.Split("\n"+contents, "\n")
	context.commentBlocks = gatherCommentBlocks(strings.NewReader(contents))

	// Recurse into the AST using pre-order traversal, gathering type information along the way.
	context.gatherTypeDefinitions(root.Child, "", nil)

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

type gatherContext struct {
	definitions       []TypeDefinition
	stack             []scope
	commentBlocks     map[int]string
	codelines         []string
	floatMatcher      *regexp.Regexp
	vectorMatcher     *regexp.Regexp
	fieldDocParser    *regexp.Regexp
	emitterFlagFinder *regexp.Regexp
}

// https://github.com/google/re2/wiki/Syntax
func (context *gatherContext) compileRegexps() {
	context.floatMatcher = regexp.MustCompile(`(\-?[0-9]+\.[0-9]*)f?`)
	context.vectorMatcher = regexp.MustCompile(`\{(\s*\-?[0-9\.]+\s*(,\s*\-?[0-9\.]+\s*){1,})\}`)
	context.emitterFlagFinder = regexp.MustCompile(`\s*\%codegen_([a-zA-Z0-9_]+)\%\s*`)
	context.fieldDocParser = regexp.MustCompile(`(?://\s*\!\<\s*(.*))`)
}

// Creates a mapping from line numbers to strings, where the strings are entire block comments
// and the line numbers correspond to the last line of each block comment.
func gatherCommentBlocks(sourceFile io.Reader) map[int]string {
	comments := make(map[int]string)
	scanner := bufio.NewScanner(sourceFile)
	var comment = ""
	var indention = 0
	for lineNumber := 1; scanner.Scan(); lineNumber++ {
		codeline := scanner.Text()
		if strings.Contains(codeline, `/**`) {
			indention = strings.Index(codeline, `/**`)
			if strings.Contains(codeline, `*/`) {
				comments[lineNumber] = codeline[indention:] + "\n"
				continue
			}
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

// Annotates struct fields that have custom types (i.e. enums or structs).
func (context gatherContext) addTypeQualifiers() {
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
func (context gatherContext) distillValue(cppvalue string, lineNumber int) string {
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
	return cppvalue
}

func (context *gatherContext) getDescription(line int) string {
	desc := context.commentBlocks[line-1]
	if desc == "" {
		codeline := context.codelines[line]
		if matches := context.fieldDocParser.FindStringSubmatch(codeline); matches != nil {
			desc = matches[1]
		}
	}
	return context.emitterFlagFinder.ReplaceAllString(desc, "")
}

func (context *gatherContext) getEmitterFlags(line int) map[string]struct{} {
	codeline := context.codelines[line]
	if matches := context.emitterFlagFinder.FindAllStringSubmatch(codeline, -1); matches != nil {
		result := make(map[string]struct{}, len(matches))
		for _, flag := range matches {
			result[flag[1]] = struct{}{}
		}
		return result
	}
	return nil
}

// Search for all enums and structs and gather them into a flat list of type definitions.
func (context *gatherContext) gatherTypeDefinitions(node parse.Node, prefix string, parent TypeDefinition) {
	switch concrete := node.(type) {
	case *parse.NamespaceNode:
		// HACK: filament namespace is a special case, remove it from the type database.
		if concrete.Name != "filament" {
			prefix = prefix + concrete.Name + "::"
		}
		for _, child := range concrete.Children {
			context.gatherTypeDefinitions(child, prefix, parent)
		}
	case *parse.EnumNode:
		defn := &EnumDefinition{
			name:        concrete.Name,
			qualifier:   prefix,
			Values:      make([]EnumValue, len(concrete.Values)),
			Description: context.getDescription(int(concrete.Line)),
			parent:      parent,
		}
		for i, val := range concrete.Values {
			defn.Values[i] = EnumValue{
				Name:        val,
				Description: context.getDescription(int(concrete.ValueLines[i])),
			}
		}
		context.definitions = append(context.definitions, defn)
	case *parse.StructNode:
		defn := &StructDefinition{
			name:        concrete.Name,
			qualifier:   prefix,
			Fields:      make([]StructField, 0),
			Description: context.getDescription(int(concrete.Line)),
			parent:      parent,
		}
		prefix = prefix + concrete.Name + "::"
		for _, child := range concrete.Members {
			switch member := child.(type) {
			case *parse.StructNode, *parse.ClassNode, *parse.EnumNode, *parse.NamespaceNode:
				context.gatherTypeDefinitions(child, prefix, defn)
			case *parse.FieldNode:
				defn.Fields = append(defn.Fields, StructField{
					TypeString:   member.Type,
					Name:         member.Name,
					DefaultValue: context.distillValue(member.Rhs, int(member.Line)),
					Description:  context.getDescription(int(member.Line)),
					EmitterFlags: context.getEmitterFlags(int(member.Line)),
				})
			}
		}
		context.definitions = append(context.definitions, defn)
	case *parse.ClassNode:
		prefix = prefix + concrete.Name + "::"
		for _, child := range concrete.Members {
			switch child.(type) {
			case *parse.StructNode, *parse.ClassNode, *parse.EnumNode, *parse.NamespaceNode:
				context.gatherTypeDefinitions(child, prefix, parent)
			}
		}
	}
}
