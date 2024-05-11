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
	"fmt"
	"log"
	"os"
	"path/filepath"
	"text/template"
)

func EmitSerializer(definitions []db.TypeDefinition, outputFolder string) {
	// The following template extensions make it possible to generate valid C++ code with
	// fewer if-then-else blocks in the template file.
	customExtensions := template.FuncMap{
		"trailingcomma": func(index, length int) string {
			if index == length-1 {
				return ""
			}
			return ","
		},
		"flag": func(field *db.StructField, flag string) bool {
			_, exists := field.EmitterFlags[flag]
			return exists
		},
		"elseif": func(index int) string {
			if index == 0 {
				return "if"
			}
			return "else if"
		},
		"braceif": func(index int) string {
			if index == 0 {
				return "if"
			}
			return "} else if"
		},
		"cast": func(ctype string) string {
			switch ctype {
			case "bool":
				return "to_string"
			case "uint8_t":
				return "int"
			}
			return ""
		},
	}

	codegen := template.New("beamsplitter").Funcs(customExtensions)
	codegen = template.Must(codegen.ParseFiles("emitters/serializer.template"))

	generate := func(file *os.File, section string, definition db.TypeDefinition) {
		err := codegen.ExecuteTemplate(file, section, definition)
		if err != nil {
			log.Fatal(err.Error())
		}
	}

	{
		path := filepath.Join(outputFolder, "Settings_generated.cpp")
		file, err := os.Create(path)
		if err != nil {
			log.Fatal(err)
		}
		defer file.Close()
		defer fmt.Println("Generated " + path)
		generate(file, "CppHeader", nil)
		for _, definition := range definitions {
			switch definition.(type) {
			case *db.StructDefinition:
				generate(file, "CppStructReader", definition)
				generate(file, "CppStructWriter", definition)
			case *db.EnumDefinition:
				generate(file, "CppEnumReader", definition)
				generate(file, "CppEnumWriter", definition)
			}
		}
		generate(file, "CppFooter", nil)
	}
	{
		path := filepath.Join(outputFolder, "Settings_generated.h")
		file, err := os.Create(path)
		if err != nil {
			log.Fatal(err)
		}
		defer file.Close()
		defer fmt.Println("Generated " + path)
		generate(file, "HppHeader", nil)
		for _, definition := range definitions {
			switch definition.(type) {
			case *db.StructDefinition:
				generate(file, "HppStruct", definition)
			case *db.EnumDefinition:
				generate(file, "HppEnum", definition)
			}
		}
		generate(file, "HppFooter", nil)
	}
}
