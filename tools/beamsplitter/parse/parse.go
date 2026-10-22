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
	"log"
	"strconv"
)

// Consumes the entire content of a C++ header file and produces an abstract syntax tree.
func Parse(contents string) *RootNode {
	lexer := createLexer(contents)
	return parseRoot(lexer)
}

// Assumes that we have just consumed the open brace from the lexer.
// This consumes all lexemes in the entire struct, including the outer end brace.
// It does not consume anything after the end brace.
func parseStructBody(lex *lexer) []Node {
	var members []Node
	append := func(node Node) {
		members = append(members, node)
	}

	// Assumes that we have just consumed the end paren in the argument list.
	parseMethod := func(name, returns, args item, isTemplate bool) *MethodNode {
		item := lex.nextItem()
		for item.typ == itemConst || item.typ == itemNoexcept {
			item = lex.nextItem()
		}
		for item.typ == itemConst || item.typ == itemNoexcept {
			item = lex.nextItem()
		}
		method := &MethodNode{
			Line:       item.line,
			Name:       name.val,
			ReturnType: returns.val,
			Arguments:  args.val,
			Body:       "",
			IsTemplate: isTemplate,
		}
		switch item.typ {
		case itemMethodBody:
			method.Body = item.val
		case itemSemicolon:
		default:
			panic(lex, item)
			return nil
		}
		return method
	}
	for item := lex.nextItem(); item.typ != itemCloseBrace; item = lex.nextItem() {
		switch {
		case item.val == "constexpr", item.val == "friend":
			// do nothing for these annotations
		case item.val == "enum":
			append(parseEnum(lex))
		case item.val == "struct":
			append(parseStruct(lex))
		case item.val == "class":
			append(parseClass(lex))
		case item.val == "namespace":
			append(parseNamespace(lex))
		case item.val == "using":
			append(parseUsing(lex))
		case item.val == "public", item.val == "private", item.val == "protected":
			expect(lex, itemColon)
			append(&AccessSpecifierNode{
				Line:   item.line,
				Access: item.val,
			})
		case item.typ == itemCloseBrace:
			break
		case item.typ == itemSimpleType:
			name := expect(lex, itemIdentifier)
			nextItem := lex.nextItem()
			arrayLength := 0
			if nextItem.typ == itemOpenBracket {
				arrayLength, _ = strconv.Atoi(expect(lex, itemArrayLength).val)
				expect(lex, itemCloseBracket)
				nextItem = lex.nextItem()
			}
			switch nextItem.typ {
			case itemMethodArgs:
				parseMethod(name, item, nextItem, false)
			case itemSemicolon:
				append(&FieldNode{
					Line:        item.line,
					Name:        name.val,
					Type:        item.val,
					Rhs:         "",
					ArrayLength: arrayLength,
				})
			case itemEquals:
				rhs := expect(lex, itemDefaultValue)
				expect(lex, itemSemicolon)
				append(&FieldNode{
					Line:        item.line,
					Name:        name.val,
					Type:        item.val,
					Rhs:         rhs.val,
					ArrayLength: arrayLength,
				})
			}
		case item.typ == itemTemplate:
			expect(lex, itemTemplateArgs)
			returns := expect(lex, itemSimpleType)
			name := expect(lex, itemIdentifier)
			args := expect(lex, itemMethodArgs)
			append(parseMethod(name, returns, args, true))
		default:
			panic(lex, item)
		}
	}
	return members
}

// Assumes that we have just consumed the "class" keyword from the lexer.
// Consumes everything up to (and including) the trailing semicolon.
func parseClass(lex *lexer) *ClassNode {
	name := expect(lex, itemIdentifier)
	item := lex.nextItem()
	if item.typ == itemSemicolon {
		// We don't have an AST node for forward declarations, just skip it.
		return nil
	}
	if item.typ == itemColon {
		// Only one base class is allowed.
		item = lex.nextItem()
		if item.typ == itemPublic {
			item = lex.nextItem()
		}
		expect(lex, itemSimpleType)
		item = lex.nextItem()
	}
	if item.typ != itemOpenBrace {
		panic(lex, item)
	}
	members := parseStructBody(lex)
	expect(lex, itemSemicolon)
	return &ClassNode{
		Line:    name.line,
		Name:    name.val,
		Members: members,
	}
}

// Assumes that we have just consumed the "struct" keyword from the lexer.
// Consumes everything up to (and including) the trailing semicolon.
func parseStruct(lex *lexer) *StructNode {
	name := expect(lex, itemIdentifier)
	item := lex.nextItem()
	if item.typ == itemSemicolon {
		// We don't have an AST node for forward declarations, just skip it.
		return nil
	}
	if item.typ != itemOpenBrace {
		panic(lex, item)
	}
	members := parseStructBody(lex)
	expect(lex, itemSemicolon)
	return &StructNode{
		Line:    name.line,
		Name:    name.val,
		Members: members,
	}
}

// Assumes that we have just consumed the "enum" keyword from the lexer.
// Consumes everything up to (and including) the trailing semicolon.
func parseEnum(lex *lexer) *EnumNode {
	expect(lex, itemClass)
	name := expect(lex, itemIdentifier)
	item := lex.nextItem()
	if item.typ == itemColon {
		expect(lex, itemSimpleType)
		item = lex.nextItem()
	}
	if item.typ != itemOpenBrace {
		panic(lex, item)
	}
	firstVal := expect(lex, itemIdentifier)
	node := &EnumNode{
		Name:       name.val,
		Line:       name.line,
		Values:     []string{firstVal.val},
		ValueLines: []int{firstVal.line},
	}
	for item = lex.nextItem(); item.typ != itemCloseBrace; {
		if item.typ != itemComma {
			panic(lex, item)
		}
		item = lex.nextItem()
		if item.typ == itemCloseBrace {
			break
		}
		if item.typ != itemIdentifier {
			panic(lex, item)
		}
		node.Values = append(node.Values, item.val)
		node.ValueLines = append(node.ValueLines, item.line)
		item = lex.nextItem()
	}
	expect(lex, itemSemicolon)
	return node
}

// Assumes that we have just consumed the "using" keyword from the lexer.
// Consumes everything up to (and including) the trailing semicolon.
func parseUsing(lex *lexer) *UsingNode {
	name := expect(lex, itemIdentifier)
	expect(lex, itemEquals)
	rhs := expect(lex, itemSimpleType)
	expect(lex, itemSemicolon)
	return &UsingNode{name.line, name.val, rhs.val}
}

// Assumes that we have just consumed the "namespace" keyword from the lexer.
// Consumes everything up to (and including) the closing brace.
func parseNamespace(lex *lexer) *NamespaceNode {
	name := expect(lex, itemIdentifier)
	expect(lex, itemOpenBrace)
	ns := &NamespaceNode{name.line, name.val, nil}
	item := lex.nextItem()

	// Filter out nil nodes (e.g. forward declarations)
	// Note that checking for nil is tricky due to a classic Go gotcha.
	append := func(child Node) {
		switch concrete := child.(type) {
		case *StructNode:
			if concrete == nil {
				return
			}
		case *ClassNode:
			if concrete == nil {
				return
			}
		}
		ns.Children = append(ns.Children, child)
	}

	for ; item.typ != itemCloseBrace; item = lex.nextItem() {
		switch item.typ {
		case itemTemplate:
			expect(lex, itemTemplateArgs)
			switch lex.nextItem().typ {
			case itemClass:
				node := parseClass(lex)
				node.IsTemplate = true
				append(node)
			case itemStruct:
				node := parseStruct(lex)
				node.IsTemplate = true
				append(node)
			default:
				panic(lex, item)
			}
		case itemClass:
			append(parseClass(lex))
		case itemStruct:
			append(parseStruct(lex))
		case itemEnum:
			append(parseEnum(lex))
		case itemUsing:
			append(parseUsing(lex))
		case itemNamespace:
			append(parseNamespace(lex))
		default:
			panic(lex, item)
		}
	}
	return ns
}

func parseRoot(lex *lexer) *RootNode {
	expect(lex, itemNamespace)
	ns := parseNamespace(lex)
	return &RootNode{0, ns}
}

func expect(lex *lexer, expectedType itemType) item {
	item := lex.nextItem()
	if item.typ != expectedType {
		panic(lex, item)
	}
	return item
}

func panic(lex *lexer, unexpected item) {
	lex.drain()
	// Very useful local hack: change this to Panicf to see a call stack.
	log.Fatalf("%d: parser sees unexpected lexeme %s", unexpected.line, unexpected.String())
}
