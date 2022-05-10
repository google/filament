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
	"strings"
	"text/template"
)

func EmitJavaScript(definitions []Scope, outputFolder string) {
	// These template extensions are used to transmogrify C++ symbols and value literals into
	// JavaScript. We mostly don't need to do anything since the parser has already done some
	// massaging and verification (e.g. it removed the trailing "f" from floating point literals).
	// However enums need some special care here. Emscripten bindings are flat, so our own
	// convention is to use $ for the scoping delimiter, which is a legal symbol character in JS.
	// However we still use . to separate the enum value from the enum type, because emscripten has
	// first-class support for class enums.
	customExtensions := template.FuncMap{
		"qualifiedtype": func(typename string) string {
			typename = strings.ReplaceAll(typename, "::", "$")
			return typename
		},
		"qualifiedvalue": func(name string) string {
			count := strings.Count(name, "::")
			if count > 0 {
				name = "Filament.View$" + name
			}
			name = strings.Replace(name, "::", "$", count-1)
			name = strings.Replace(name, "::", ".", 1)
			return name
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
