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
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"
	"text/template"

	"beamsplitter/parse"
)

// Returns a templating function that automatically checks for fatal errors. The returned function
// takes an output stream, a template name to invoke, and a template context object.
func createJavaCodeGenerator() func(*os.File, string, parse.TypeDefinition) {
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
		"java_type": func(cpptype string) string {
			switch cpptype {
			case "bool":
				return "boolean"
			case "uint8_t":
				return "int"
			}
			return cpptype
		},
		"java_value": func(cppval string) string {
			return strings.ReplaceAll(cppval, "::", ".")
		},
	}

	templ := template.New("beamsplitter").Funcs(customExtensions)
	templ = template.Must(templ.ParseFiles("java.template"))
	return func(file *os.File, section string, definition parse.TypeDefinition) {
		err := templ.ExecuteTemplate(file, section, definition)
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

	generate := createJavaCodeGenerator()

	enabled := false
	if enabled {
		for _, definition := range definitions {
			switch definition.(type) {
			case *parse.StructDefinition:
				generate(file, "Struct", definition)
			case *parse.EnumDefinition:
				generate(file, "Enum", definition)
			}
		}
	}

	file.WriteString("}\n")
}
