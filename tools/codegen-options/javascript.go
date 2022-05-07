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
	"fmt"
	"log"
	"os"
	"path/filepath"
	"text/template"
)

func EmitJavaScript(definitions []Scope, outputFolder string) {
	customExtensions := template.FuncMap{
		"jsify": func(cppvalue string) string {
			// Values are already massaged in the parsing phase, which is a better place than
			// here since the parser can report the line number for unexpected syntax.
			return cppvalue
		},
	}

	codegen := template.New("Settings").Funcs(customExtensions)
	codegen = template.Must(codegen.ParseFiles("javascript.template"))

	{
		path := filepath.Join(outputFolder, "jsbindings_generated.cpp")
		file, err := os.Create(path)
		if err != nil {
			log.Fatal(err)
		}
		defer file.Close()
		defer fmt.Println("Generated " + path)
		codegen.ExecuteTemplate(file, "JsBindingsHeader", nil)
		for _, definition := range definitions {
			switch definition.(type) {
			case *StructDefinition:
				codegen.ExecuteTemplate(file, "JsBindingsStruct", definition)
			}
		}
		codegen.ExecuteTemplate(file, "JsBindingsFooter", nil)
	}
	{
		path := filepath.Join(outputFolder, "jsenums_generated.cpp")
		file, err := os.Create(path)
		if err != nil {
			log.Fatal(err)
		}
		defer file.Close()
		defer fmt.Println("Generated " + path)
		codegen.ExecuteTemplate(file, "JsEnumsHeader", nil)
		for _, definition := range definitions {
			switch definition.(type) {
			case *EnumDefinition:
				codegen.ExecuteTemplate(file, "JsEnum", definition)
			}
		}
		codegen.ExecuteTemplate(file, "JsEnumsFooter", nil)
	}
	{
		path := filepath.Join(outputFolder, "extensions_generated.js")
		file, err := os.Create(path)
		if err != nil {
			log.Fatal(err)
		}
		defer file.Close()
		defer fmt.Println("Generated " + path)
		codegen.ExecuteTemplate(file, "JsExtensionsHeader", nil)
		for _, definition := range definitions {
			switch definition.(type) {
			case *StructDefinition:
				codegen.ExecuteTemplate(file, "JsExtension", definition)
			}
		}
		codegen.ExecuteTemplate(file, "JsExtensionsFooter", nil)
	}

	// TODO: filament.d.ts
}
