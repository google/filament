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
	"runtime"

	db "beamsplitter/database"
	"beamsplitter/emitters"
	"beamsplitter/parse"
)

var CHECK_API_SYNTAX = false

func findFilamentRoot() string {
	var (
		_, b, _, _  = runtime.Caller(0)
		thisFolder  = filepath.Dir(b)
		toolsFolder = filepath.Dir(thisFolder)
	)
	return filepath.Dir(toolsFolder)
}

func main() {
	const sourceFilename = "Options.h"
	log.SetFlags(0)
	log.SetPrefix(sourceFilename + ":")
	root := findFilamentRoot()
	sourcePath := filepath.Join(root, "filament", "include", "filament", sourceFilename)

	data, err := os.ReadFile(sourcePath)
	if err != nil {
		log.Fatal(err)
	}

	contents := string(data)
	ast := parse.Parse(contents)
	definitions := db.Create(ast, contents)

	emitters.EmitSerializer(definitions, filepath.Join(root, "libs", "viewer", "src"))

	jsfolder := filepath.Join(root, "web", "filament-js")
	emitters.EmitJavaScript(definitions, "View", jsfolder)
	emitters.EditTypeScript(definitions, "View", jsfolder)

	javafolder := filepath.FromSlash("com/google/android/filament")
	javafolder = filepath.Join(root, "android/filament-android/src/main/java", javafolder)
	emitters.EditJava(definitions, "View", javafolder)

	fmt.Print(`
Note that this tool does not generate bindings for setter methods on
filament::View. If you added or renamed one of the setter methods, you
will likely need to manually modify the following files:

 - web/filament-js/extensions.js
 - web/filament-js/jsbindings.cpp
 - web/filament-js/filament.d.ts
 - filament/include/filament/View.h
 - filament/src/View.cpp
 - filament/src/details/View.cpp
 - android/filament-android/src/main/java/.../View.java
 - android/filament-android/src/main/cpp/View.cpp
`)

	if CHECK_API_SYNTAX {
		sources := []string{}
		apiPath := filepath.Join(root, "filament", "include", "filament")
		entries, err := os.ReadDir(apiPath)
		for _, entry := range entries {
			sources = append(sources, filepath.Join(apiPath, entry.Name()))
		}
		for _, source := range sources {
			log.SetPrefix(filepath.Base(source) + ":")
			data, err = os.ReadFile(source)
			if err != nil {
				log.Fatal(err)
			}
			contents = string(data)
			parse.Parse(contents)
		}
	}
}
