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

type Node interface{}

type RootNode struct {
	Line  int
	Child *NamespaceNode
}

type NamespaceNode struct {
	Line     int
	Name     string
	Children []Node
}

type ClassNode struct {
	Line       int
	Name       string
	Members    []Node
	IsTemplate bool
}

type StructNode struct {
	Line       int
	Name       string
	Members    []Node
	IsTemplate bool
}

type EnumNode struct {
	Line       int
	Name       string
	Values     []string
	ValueLines []int // used to find docstring for each enum value
}

type UsingNode struct {
	Line int
	Name string
	Rhs  string
}

type AccessSpecifierNode struct {
	Line   int
	Access string
}

type MethodNode struct {
	Line       int
	Name       string
	ReturnType string
	Arguments  string
	Body       string
	IsTemplate bool
}

type FieldNode struct {
	Line        int
	Name        string
	Type        string
	Rhs         string
	ArrayLength int
}
