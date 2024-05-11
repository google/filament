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

package emitters

import (
	db "beamsplitter/database"
	"bufio"
	"bytes"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"strings"
	"text/template"
)

func EditJava(definitions []db.TypeDefinition, classname string, folder string) {
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

	// Forward declarations for usage in a closure.
	var flattener func(*db.StructDefinition) string
	var sharedExtensions template.FuncMap

	javifyType := func(field db.StructField) string {
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
		result := strings.ReplaceAll(field.TypeString, "*", "")
		result = strings.ReplaceAll(result, "::", ".")
		return " " + result
	}

	javifyValue := func(field db.StructField) string {

		// When forcing an array to be bound to a float, extract the first component and use
		// that as the default value.
		if _, exists := field.EmitterFlags["java_float"]; exists {
			arrayContents := strings.Trim(field.DefaultValue, " []")
			if comma := strings.Index(arrayContents, ","); comma > -1 {
				arrayContents = arrayContents[:comma]
			}
			if !strings.HasSuffix(arrayContents, "f") {
				arrayContents += "f"
			}
			return " " + arrayContents
		}

		if field.DefaultValue == "nullptr" {
			return " null"
		}

		// Handle enums
		value := strings.ReplaceAll(field.DefaultValue, "::", ".")

		// Add "f" suffix for scalars
		if field.TypeString == "float" {
			if field.DefaultValue == "INFINITY" {
				return " Float.POSITIVE_INFINITY"
			} else {
				value += "f"
			}
		} else if c := len(value); c > 1 && value[0] == '[' && value[c-1] == ']' {
			// For vector types, replace [] with {} and add "f" suffix to components.
			switch field.TypeString {
			case "math::float2", "math::float3", "math::float4", "LinearColor", "LinearColorA":
				slices := strings.Split(value[1:c-1], ",")
				for i := range slices {
					slices[i] = strings.TrimSpace(slices[i])
					if !strings.HasSuffix(slices[i], "f") {
						slices[i] += "f"
					}
				}
				value = "{" + strings.Join(slices, ", ") + "}"
			default:
				value = "{" + value[1:c-1] + "}"
			}
		}
		return " " + value
	}

	getDocBlock := func(defn db.Documented, depth int) string {
		doc := defn.GetDoc()
		if doc == "" {
			return ""
		}
		indent := strings.Repeat("    ", depth)
		if strings.Count(doc, "\n") > 0 {
			return strings.ReplaceAll(doc, "\n", "\n"+indent)
		}
		return "/**\n" + indent + " * " + doc + "\n" + indent + " */\n" + indent
	}

	getFieldAnnotation := func(field db.StructField, depth int) string {
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
	}

	flattenStruct := func(defn *db.StructDefinition) string {
		prefix := strings.ToLower(defn.BaseName())
		buf := &bytes.Buffer{}
		for _, field := range defn.Fields {
			doc := getDocBlock(defn, 0)
			buf.WriteString(doc)
			buf.WriteString(getFieldAnnotation(field, 0))
			buf.WriteString("public")
			buf.WriteString(javifyType(field))
			buf.WriteByte(' ')
			buf.WriteString(prefix + strings.ToUpper(field.Name[0:1]) + field.Name[1:])
			buf.WriteString(" =")
			buf.WriteString(javifyValue(field))
			buf.WriteString(";\n")
		}
		return indent(buf.String(), 2)
	}
	flattener = flattenStruct

	// These template extensions are used to transmogrify C++ symbols and value literals to Java.
	customExtensions := template.FuncMap{
		"docblock": getDocBlock,
		"nested_type_declarations": func(parent db.TypeDefinition) string {
			// Look for all fields that request flattening since we should skip their emission.
			flattenedTypes := make(map[db.TypeDefinition]struct{})
			if structDefn, isStruct := parent.(*db.StructDefinition); isStruct {
				for _, field := range structDefn.Fields {
					_, flatten := field.EmitterFlags["java_flatten"]
					if flatten && field.CustomType != nil {
						flattenedTypes[field.CustomType] = struct{}{}
					}
				}
			}

			// Find all child types and emit them.
			generate := createJavaCodeGenerator(sharedExtensions)
			buf := &bytes.Buffer{}
			for _, definition := range definitions {
				if definition.Parent() != parent {
					continue
				}
				_, skip := flattenedTypes[definition]
				if skip {
					continue
				}
				switch definition.(type) {
				case *db.StructDefinition:
					generate(buf, "Struct", definition)
				case *db.EnumDefinition:
					generate(buf, "Enum", definition)
				}
			}
			return indent(buf.String(), 1)
		},
		"annotation": getFieldAnnotation,
		"java_type":  javifyType,
		"java_value": javifyValue,
		"flatten": func(field *db.StructField) string {
			if structDefn, isStruct := field.CustomType.(*db.StructDefinition); isStruct {
				return strings.TrimLeft(flattener(structDefn), " ")
			}
			log.Fatal("Unexpected flatten flag.")
			return ""
		},
		"flag": func(field *db.StructField, flag string) bool {
			_, exists := field.EmitterFlags[flag]
			return exists
		},
	}

	sharedExtensions = customExtensions

	generate := createJavaCodeGenerator(customExtensions)
	for _, definition := range definitions {
		if definition.Parent() != nil {
			continue
		}
		switch definition.(type) {
		case *db.StructDefinition:
			generate(file, "Struct", definition)
		case *db.EnumDefinition:
			generate(file, "Enum", definition)
		}
	}

	file.WriteString("}\n")
}

// Adds one level of indention to the given multi-line string.
// Isolated newlines are intentially not indented.
func indent(src string, depth int) string {
	dst := &bytes.Buffer{}
	buf := bytes.NewBufferString(src)
	scanner := bufio.NewScanner(buf)
	for scanner.Scan() {
		codeline := scanner.Text()
		if codeline != "" {
			dst.WriteString(strings.Repeat("    ", depth))
			dst.WriteString(scanner.Text())
		}
		dst.WriteByte('\n')
	}
	return dst.String()
}

// Wrapper for ExecuteTemplate that performs error checking. Takes an output stream, a template name
// to invoke, and a template context object.
type templateFn = func(io.Writer, string, db.TypeDefinition)

func createJavaCodeGenerator(customExtensions template.FuncMap) templateFn {
	templ := template.New("beamsplitter").Funcs(customExtensions)
	templ = template.Must(templ.ParseFiles("emitters/java.template"))
	return func(writer io.Writer, section string, definition db.TypeDefinition) {
		err := templ.ExecuteTemplate(writer, section, definition)
		if err != nil {
			log.Fatal(err.Error())
		}
	}
}
