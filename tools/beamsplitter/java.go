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
	"bytes"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"strings"
	"text/template"

	"beamsplitter/parse"
)

// Adds one level of indention to the given multi-line string.
// Isolated newlines are intentially not indented.
func indent(src string) string {
	dst := &bytes.Buffer{}
	buf := bytes.NewBufferString(src)
	scanner := bufio.NewScanner(buf)
	for scanner.Scan() {
		codeline := scanner.Text()
		if codeline != "" {
			dst.WriteString("    ")
			dst.WriteString(scanner.Text())
		}
		dst.WriteByte('\n')
	}
	return dst.String()
}

// Wrapper for ExecuteTemplate that performs error checking. Takes an output stream, a template name
// to invoke, and a template context object.
type templateFn = func(io.Writer, string, parse.TypeDefinition)

func createJavaCodeGenerator(customExtensions template.FuncMap) templateFn {
	templ := template.New("beamsplitter").Funcs(customExtensions)
	templ = template.Must(templ.ParseFiles("java.template"))
	return func(writer io.Writer, section string, definition parse.TypeDefinition) {
		err := templ.ExecuteTemplate(writer, section, definition)
		if err != nil {
			log.Fatal(err.Error())
		}
	}
}

func editJava(definitions []parse.TypeDefinition, classname string, folder string) {
	path := filepath.Join(folder, classname+".java")
	var codelines []string
	{
		sourceFile, err := os.Open(path)
		if err != nil {
			log.Fatal(err)
		}
		defer sourceFile.Close()
		lineScanner := bufio.NewScanner(sourceFile)
		foundMarker := false
		for lineNumber := 1; lineScanner.Scan(); lineNumber++ {
			codeline := lineScanner.Text()
			if strings.Contains(codeline, kCodelineMarker) {
				foundMarker = true
				break
			}
			codelines = append(codelines, codeline)
		}
		if !foundMarker {
			log.Fatal("Unable to find marker line in Java file.")
		}
	}
	file, err := os.Create(path)
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()
	defer fmt.Println("Edited " + path)
	for _, codeline := range codelines {
		file.WriteString(codeline)
		file.WriteString("\n")
	}
	file.WriteString("    // " + kCodelineMarker + "\n")

	// Declare a closure variable to handle nested types.
	var sharedExtensions template.FuncMap

	// These template extensions are used to transmogrify C++ symbols and value literals to Java.
	customExtensions := template.FuncMap{
		"docblock": func(defn parse.Documented, depth int) string {
			doc := defn.GetDoc()
			if doc == "" {
				return ""
			}
			indent := strings.Repeat("    ", depth)
			if strings.Count(doc, "\n") > 0 {
				return strings.ReplaceAll(doc, "\n", "\n"+indent)
			}
			return "/**\n" + indent + " * " + doc + "\n" + indent + " */\n" + indent
		},
		"nested_type_declarations": func(parent parse.TypeDefinition) string {
			generate := createJavaCodeGenerator(sharedExtensions)
			buf := &bytes.Buffer{}
			for _, definition := range definitions {
				if definition.Parent() != parent {
					continue
				}
				switch definition.(type) {
				case *parse.StructDefinition:
					generate(buf, "Struct", definition)
				case *parse.EnumDefinition:
					generate(buf, "Enum", definition)
				}
			}
			return indent(buf.String())
		},
		"annotation": func(field parse.StructField, depth int) string {
			if _, exists := field.EmitterFlags["java_float"]; exists {
				return ""
			}
			annotation := ""
			switch {
			case field.DefaultValue == "nullptr":
				annotation = "@Nullable"
			case field.TypeString == "math::float2":
				annotation = "@NonNull @Size(min = 2)"
			case field.TypeString == "math::float3" || field.TypeString == "LinearColor":
				annotation = "@NonNull @Size(min = 3)"
			case field.TypeString == "math::float4" || field.TypeString == "LinearColorA":
				annotation = "@NonNull @Size(min = 4)"
			case strings.Contains(field.DefaultValue, "::"):
				annotation = "@NonNull"
			default:
				return ""
			}
			return annotation + "\n" + strings.Repeat("    ", depth)
		},
		"java_type": func(field parse.StructField) string {
			if _, exists := field.EmitterFlags["java_float"]; exists {
				return " float"
			}
			switch field.TypeString {
			case "math::float2", "math::float3", "math::float4", "LinearColor", "LinearColorA":
				return " float[]"
			case "bool":
				return " boolean"
			case "uint8_t", "uint16_t", "uint32_t":
				return " int"
			}
			return " " + strings.ReplaceAll(field.TypeString, "*", "")
		},
		"java_value": func(field parse.StructField) string {
			if _, exists := field.EmitterFlags["java_float"]; exists {
				arrayContents := strings.Trim(field.DefaultValue, " []")

				// If we're forcing an array to be bound to a flat, then extract the first component
				// and use that as the default value.
				if comma := strings.Index(arrayContents, ","); comma > -1 {
					return " " + arrayContents[:comma]
				}

				return " " + arrayContents
			}
			if field.DefaultValue == "nullptr" {
				return " null"
			}
			value := strings.ReplaceAll(field.DefaultValue, "::", ".")
			if field.TypeString == "float" {
				value += "f"
			} else if c := len(value); c > 1 && value[0] == '[' && value[c-1] == ']' {
				value = "{" + value[1:c-1] + "}"
			}
			return " " + value
		},
	}

	sharedExtensions = customExtensions

	generate := createJavaCodeGenerator(customExtensions)
	for _, definition := range definitions {
		if definition.Parent() != nil {
			continue
		}
		switch definition.(type) {
		case *parse.StructDefinition:
			generate(file, "Struct", definition)
		case *parse.EnumDefinition:
			generate(file, "Enum", definition)
		}
	}

	file.WriteString("}\n")
}
