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
	"errors"
	"fmt"
	"log"
	"strings"
	"unicode"
	"unicode/utf8"
)

var VERBOSE_LEXER = false
var KNOWN_MACROS = []string{"UTILS_PUBLIC"}

// item represents a token or text string returned from the scanner.
type item struct {
	typ  itemType // The type of this item.
	pos  int      // The starting position, in bytes, of this item in the input string.
	val  string   // The value of this item.
	line int      // The line number at the start of this item.
}

func (i item) String() string {
	dbg := strings.ReplaceAll(i.val, "\n", "\\n")
	switch {
	case i.typ == itemEOF:
		return "EOF"
	case i.typ == itemError:
		return i.val
	case i.typ > itemKeywords_:
		return dbg
	case len(i.val) > 30:
		return dbg[:10] + "..." + dbg[len(dbg)-10:]
	}
	return dbg
}

// itemType identifies the type of lex items.
type itemType int

const (
	itemError        itemType = iota // error occurred; value is text of error
	itemSimpleType                   // examples: `Texture* const`, `uint8_t`, `BlendMode`
	itemMethodBody                   // unparsed blob, includes outermost { and }
	itemMethodArgs                   // unparsed blob, includes outermost ( and )
	itemTemplateArgs                 // unparsed blob, includes outermost < and >
	itemDefaultValue                 // the unparsed RHS of an expression
	itemIdentifier                   // legal C++ identifier
	itemEOF

	itemCloseBrace
	itemColon
	itemComma
	itemEquals
	itemOpenBrace
	itemSemicolon
	itemOpenBracket
	itemCloseBracket
	itemArrayLength

	itemKeywords_ // unused enum separator
	itemClass
	itemConst
	itemConstexpr
	itemEnum
	itemFriend
	itemNamespace
	itemNoexcept
	itemPrivate
	itemProtected
	itemPublic
	itemStruct
	itemTemplate
	itemUsing
)

const eof = -1

// lexer holds the state of the scanner.
type lexer struct {
	input     string    // the entire contents of the file being scanned
	items     chan item // channel of scanned items
	line      int       // 1+number of newlines seen
	startLine int       // start line of this item
	pos       int       // current position in the input
	start     int       // start position of this item
	atEOF     bool      // we have hit the end of input
	lookahead []item
}

// next returns the next rune in the input.
func (l *lexer) next() rune {
	if int(l.pos) >= len(l.input) {
		l.atEOF = true
		return eof
	}
	r, w := utf8.DecodeRuneInString(l.input[l.pos:])
	l.pos += w
	if r == '\n' {
		l.line++
	}
	return r
}

// backup steps back one rune.
func (l *lexer) backup() {
	if !l.atEOF && l.pos > 0 {
		r, w := utf8.DecodeLastRuneInString(l.input[:l.pos])
		l.pos -= w
		// Correct newline count.
		if r == '\n' {
			l.line--
		}
	}
}

func (lex *lexer) backupMultiple(count int) {
	for i := 0; i < count; i++ {
		lex.backup()
	}
}

func (lex *lexer) backupKeyword() {
	lex.backup()
	for unicode.IsLetter(rune(lex.input[lex.pos])) {
		lex.backup()
	}
	lex.next()
}

// Passes an item back to the parser and moves the start pointer to catch up to the cursor.
// The start pointer marks the beginning of the next item.
// It helps to imagine a crawling worm whose tail catches up with its head in one movement.
func (l *lexer) emit(t itemType) {
	text := strings.TrimSpace(l.input[l.start:l.pos])
	if text == "" {
		column := l.pos - l.findLineStartPos()
		// Can occur when calling emit() twice in a row without "accepting" anything in between.
		log.Fatalf("%d: internal error at column %d\n", l.line, column)
	}
	token := item{t, l.start, text, l.startLine}
	if l.lookahead != nil {
		if VERBOSE_LEXER {
			fmt.Printf("%03d Stashing %s\n", token.line, token.String())
		}
		l.lookahead = append(l.lookahead, token)
	} else {
		if VERBOSE_LEXER {
			fmt.Printf("%03d Emitting %s\n", token.line, token.String())
		}
		l.items <- token
	}
	l.start = l.pos
	l.startLine = l.line
	l.discardOptionalSpace()
}

// Moves the start pointer to catch up to the cursor but does not pass the item to the parser.
// This function is like emit, except that it throws the item in the trash.
func (l *lexer) discard() {
	if VERBOSE_LEXER {
		dbg := l.input[l.start:l.pos]
		dbg = strings.ReplaceAll(dbg, "\n", "\\n")
		if len(dbg) > 30 {
			dbg = dbg[:10] + "..." + dbg[len(dbg)-10:]
		}
		fmt.Printf("%03d Trashing [[%s]]\n", l.line, dbg)
	}
	l.start = l.pos
	l.startLine = l.line
}

func (l *lexer) acceptAny(valid string) bool {
	if strings.ContainsRune(valid, l.next()) {
		return true
	}
	l.backup()
	return false
}

func (lex *lexer) acceptAlphaNumeric() bool {
	next := lex.next()
	if isAlphaNumeric(next) {
		return true
	}
	lex.backup()
	return false
}

func (lex *lexer) acceptPositiveInteger() bool {
	next := lex.next()
	if !unicode.IsDigit(next) || next == '0' {
		lex.backup()
		return false
	}
	lex.acceptRun("123456789")
	return true
}

// Consumes a run of runes from the valid set.
func (l *lexer) acceptRun(valid string) {
	for strings.ContainsRune(valid, l.next()) {
	}
	l.backup()
}

func (lex *lexer) acceptSpace() bool {
	return lex.acceptAny(" \t\n")
}

func (l *lexer) acceptRune(expected rune) bool {
	if l.next() == expected {
		return true
	}
	l.backup()
	return false
}

func (lex *lexer) acceptString(expectedString string) bool {
	for i, c := range expectedString {
		if lex.next() != c {
			lex.backupMultiple(i + 1)
			return false
		}
	}
	return true
}

func (lex *lexer) acceptIdentifier() bool {
	next := lex.next()
	if next != '_' && !unicode.IsLetter(next) {
		lex.backup()
		return false
	}
	for isAlphaNumeric(lex.next()) {
	}
	lex.backup()
	return true
}

func (lex *lexer) acceptKeyword(keyword string) bool {
	start := lex.pos
	if !lex.acceptString(keyword) {
		return false
	}
	if lex.eof() {
		return true
	}
	if lex.acceptAlphaNumeric() {
		lex.backupMultiple(lex.pos - start)
		return false
	}
	return true
}

func (lex *lexer) acceptKeywords(keywords []string) bool {
	for _, keyword := range keywords {
		if lex.acceptKeyword(keyword) {
			return true
		}
	}
	return false
}

// Accepts a blob of unparsed text up until the given terminator, which is not included.
func (lex *lexer) acceptTerminatedBlob(term rune) bool {
	previous := lex.pos
	for {
		if lex.next() == term {
			lex.backup()
			return true
		}
		if lex.atEOF {
			lex.backupMultiple(lex.pos - previous)
			return false
		}
	}
}

// Accepts a blob of unparsed text with the given delimiters, which can be nested.
func (lex *lexer) acceptDelimitedBlob(start rune, stop rune) bool {
	previous := lex.pos
	next := lex.next()
	if next != start {
		lex.backup()
		return false
	}
	for depth := 1; next != eof; {
		switch lex.next() {
		case start:
			depth++
		case stop:
			depth--
			if depth == 0 {
				return true
			}
		}
	}
	lex.backupMultiple(lex.pos - previous)
	return false
}

// Discards whitespace, linefeeds, comments, and C preprocessor directives.
func (lex *lexer) discardOptionalSpace() {
	for {
		for _, macro := range KNOWN_MACROS {
			if lex.acceptKeyword(macro) {
				lex.discard()
			}
		}
		switch {
		case lex.acceptSpace():
			lex.acceptRun(" \n\t")
			lex.discard()
		case lex.acceptString("/*"):
			for !lex.acceptString("*/") {
				lex.next()
			}
			lex.discard()
		case lex.acceptString("//") || lex.acceptRune('#'):
			for !lex.acceptRune('\n') {
				lex.next()
			}
			lex.discard()
		default:
			return
		}
	}
}

// Returns the next item from the input.
// Called by the parser, not in the lexing goroutine.
func (l *lexer) nextItem() item {
	return <-l.items
}

// Drains the output so the lexing goroutine will exit.
// Called by the parser, not in the lexing goroutine.
func (l *lexer) drain() {
	for range l.items {
	}
}

func (lex *lexer) eof() bool {
	return lex.pos >= len(lex.input)
}

// Creates a new scanner for the input string.
func createLexer(input string) *lexer {
	l := &lexer{
		input:     input,
		items:     make(chan item),
		line:      1,
		startLine: 1,
	}
	go l.run()
	return l
}

func (lex *lexer) run() {
	if err := lexRoot(lex); err != nil {
		column := lex.pos - lex.findLineStartPos()
		log.Fatalf("%d:%d: lexer expected %s", lex.line, column, err.Error())
	}
	close(lex.items)
}

func (lex *lexer) findLineStartPos() int {
	lineNo := 1
	for pos, c := range lex.input {
		if c != '\n' {
			continue
		}
		lineNo++
		if lineNo == lex.line {
			return pos
		}
	}
	return 0
}

func isAlphaNumeric(r rune) bool {
	return r == '_' || unicode.IsLetter(r) || unicode.IsDigit(r)
}

// Causes the lexer to enter a special "lookahead state" in which it stashes
// the lexer state and buffers all emitted items.
func lookahead(lex *lexer, cb func(*lexer) error) bool {
	if lex.lookahead != nil {
		log.Fatal("Nested lookahead.")
	}
	lex.lookahead = make([]item, 0)
	previousStart := lex.start
	previousStartLine := lex.startLine
	previousPos := lex.pos
	previousLine := lex.line
	if err := cb(lex); err == nil {
		for _, item := range lex.lookahead {
			if VERBOSE_LEXER {
				fmt.Printf("%03d Emitting %s\n", item.line, item.String())
			}
			lex.items <- item
		}
		lex.lookahead = nil
		return true
	}
	lex.lookahead = nil
	lex.start = previousStart
	lex.startLine = previousStartLine
	lex.line = previousLine
	lex.pos = previousPos
	return false
}

// -------------------------------------------------------------------------------------------------
// The remainder of this file has one function for each nonterminal in the BNF.
// The ordering should be consistent with the grammar in the README.
// These functions are all prefixed with "lex" and return an error or nil.
// Error strings are automatically prefixed with the line number and "lexer expected ".
// In lookahead situations, the returned error might be intentionally discarded.
// -------------------------------------------------------------------------------------------------

func lexRoot(lex *lexer) error {
	if lex.eof() {
		return nil
	}
	lex.discardOptionalSpace()
	if lex.acceptKeyword("namespace") {
		return lexNamespace(lex)
	}
	return errors.New("namespace")
}

// Assumptions: cursor is just past the namespace keyword
// Directly emits: keyword, the optional identifier, opening and closing braces
// Indirectly emits: content of the namespace
func lexNamespace(lex *lexer) error {
	lex.emit(itemNamespace)
	if lex.acceptIdentifier() {
		lex.emit(itemIdentifier)
	}
	if !lex.acceptRune('{') {
		return errors.New("{")
	}
	lex.emit(itemOpenBrace)
	for !lex.acceptRune('}') {
		if err := lexBlock(lex); err != nil {
			return err
		}
	}
	lex.emit(itemCloseBrace)
	return nil
}

// Assumptions: cursor is just before one of the block keywords
// Directly emits: struct or class keywords
// Indirectly emits: the block keyword, its braces, contents, semicolons
func lexBlock(lex *lexer) error {
	switch {
	case lex.acceptKeyword("namespace"):
		return lexNamespace(lex)
	case lex.acceptKeyword("template"):
		lex.emit(itemTemplate)
		if !lex.acceptDelimitedBlob('<', '>') {
			return errors.New("template arguments")
		}
		lex.emit(itemTemplateArgs)
		if !lex.acceptKeywords([]string{"class", "struct"}) {
			return errors.New("class or struct")
		}
		lex.backupKeyword()
		return lexBlock(lex)
	case lex.acceptKeyword("struct"):
		lex.emit(itemStruct)
		if lookahead(lex, lexForwardDeclaration) {
			return nil
		}
		return lexStruct(lex)
	case lex.acceptKeyword("class"):
		lex.emit(itemClass)
		if lookahead(lex, lexForwardDeclaration) {
			return nil
		}
		return lexClass(lex)
	case lex.acceptKeyword("using"):
		return lexUsing(lex)
	case lex.acceptKeyword("enum"):
		return lexEnum(lex)
	}
	return errors.New("namespace, struct, class, enum, or using")
}

func lexForwardDeclaration(lex *lexer) error {
	if !lex.acceptIdentifier() {
		return errors.New("identifier")
	}
	lex.emit(itemIdentifier)
	if !lex.acceptRune(';') {
		return errors.New("; after forward declaration")
	}
	lex.emit(itemSemicolon)
	return nil
}

// Assumptions: cursor is just past the class keyword
// Directly emits: identifier, opening and closing braces, semicolon
// Indirectly emits: content of the struct
func lexClass(lex *lexer) error {
	if !lex.acceptIdentifier() {
		return errors.New("does not allow anonymous classes")
	}
	lex.emit(itemIdentifier)
	if lex.acceptRune(':') {
		lex.emit(itemColon)
		if lex.acceptKeyword("public") {
			lex.emit(itemPublic)
		}
		if err := lexSimpleType(lex); err != nil {
			return err
		}
	}
	if !lex.acceptRune('{') {
		return errors.New("{ before class body")
	}
	lex.emit(itemOpenBrace)
	if err := lexStructBody(lex); err != nil {
		return err
	}
	lex.emit(itemCloseBrace)
	if !lex.acceptRune(';') {
		return errors.New(": after class")
	}
	lex.emit(itemSemicolon)
	return nil
}

// Assumptions: cursor is just past the struct keyword
// Directly emits: the optional identifier, opening and closing braces, semicolon
// Indirect emits: content of the struct
func lexStruct(lex *lexer) error {
	if lex.acceptIdentifier() {
		lex.emit(itemIdentifier)
	}
	if !lex.acceptRune('{') {
		return errors.New("{ before struct body")
	}
	lex.emit(itemOpenBrace)
	if err := lexStructBody(lex); err != nil {
		return err
	}
	lex.emit(itemCloseBrace)
	if !lex.acceptRune(';') {
		return errors.New("; after struct body")
	}
	lex.emit(itemSemicolon)
	return nil
}

// Assumptions: cursor is just past the enum keyword
// Directly emits: all parts of the enum, including the trailing semicolon
func lexEnum(lex *lexer) error {
	lex.emit(itemEnum)
	if !lex.acceptKeyword("class") {
		return errors.New("class-style enum")
	}
	lex.emit(itemClass)
	if !lex.acceptIdentifier() {
		return errors.New("valid identifier (anonymous enums are not supported)")
	}
	lex.emit(itemIdentifier)
	if lex.acceptRune(':') {
		lex.emit(itemColon)
		if err := lexSimpleType(lex); err != nil {
			return err
		}
	}
	if !lex.acceptRune('{') {
		return errors.New("{ before enum definition")
	}
	lex.emit(itemOpenBrace)

	if !lex.acceptIdentifier() {
		return errors.New("at least one value in the enum")
	}
	lex.emit(itemIdentifier)

	for !lex.acceptRune('}') {
		if !lex.acceptRune(',') {
			return errors.New(", between enum values")
		}
		lex.emit(itemComma)
		if lex.acceptRune('}') {
			break
		}
		if !lex.acceptIdentifier() {
			return errors.New("valid identifier in enum")
		}
		lex.emit(itemIdentifier)
	}
	lex.emit(itemCloseBrace)
	if !lex.acceptRune(';') {
		return errors.New("; after enum definition")
	}
	lex.emit(itemSemicolon)
	return nil
}

func lexUsing(lex *lexer) error {
	lex.emit(itemUsing)
	if !lex.acceptIdentifier() {
		return errors.New("valid identifier in type alias")
	}
	lex.emit(itemIdentifier)
	if !lex.acceptRune('=') {
		return errors.New("= in type alias")
	}
	lex.emit(itemEquals)
	if err := lexSimpleType(lex); err != nil {
		return err
	}

	if !lex.acceptRune(';') {
		return errors.New("; after type alias")
	}
	lex.emit(itemSemicolon)
	return nil
}

// Assumptions: cursor is just past the opening brace of a struct or class
// Directly emits: nothing
// Indirect emits: entire content of the struct or class, but not the outer braces
func lexStructBody(lex *lexer) error {
	accessKeywords := []string{"public", "private", "protected"}
	blockKeywords := []string{"class", "struct", "enum", "using", "template", "namespace"}
	for {
		switch {
		case lex.acceptRune('}'):
			return nil
		case lex.acceptKeywords(accessKeywords):
			lex.backupKeyword()
			if err := lexAccessSpecifier(lex); err != nil {
				return err
			}
		case lex.acceptKeywords(blockKeywords):
			lex.backupKeyword()
			if err := lexBlock(lex); err != nil {
				return err
			}
		default:
			if lookahead(lex, lexMethod) {
				continue
			}
			if err := lexField(lex); err != nil {
				return err
			}
		}
	}
}

// Assumptions: cursor is just before one of the access keywords
// Directly emits: the access keyword and the colon
func lexAccessSpecifier(lex *lexer) error {
	switch {
	case lex.acceptKeyword("public"):
		lex.emit(itemPublic)
	case lex.acceptKeyword("protected"):
		lex.emit(itemProtected)
	case lex.acceptKeyword("private"):
		lex.emit(itemPrivate)
	default:
		return errors.New("legal access specifier")
	}
	if !lex.acceptRune(':') {
		return errors.New(": after access specifier")
	}
	lex.emit(itemColon)
	return nil
}

// Assumptions: cursor is just before a method declaration or implementation
// Directly emits: entire content of the method declaration or implementation
func lexMethod(lex *lexer) error {
	if lex.acceptKeyword("template") {
		lex.emit(itemTemplate)
		if !lex.acceptDelimitedBlob('<', '>') {
			return errors.New("template arguments")
		}
		lex.emit(itemTemplateArgs)
	}
	if lex.acceptKeyword("friend") {
		lex.emit(itemFriend)
	}
	if lex.acceptKeyword("constexpr") {
		lex.emit(itemConstexpr)
	}
	if err := lexSimpleType(lex); err != nil {
		return err
	}
	if !lex.acceptIdentifier() {
		return errors.New("valid identifier")
	}
	lex.emit(itemIdentifier)
	if !lex.acceptDelimitedBlob('(', ')') {
		return errors.New("function arguments")
	}
	lex.emit(itemMethodArgs)
	if lex.acceptKeyword("const") {
		lex.emit(itemConst)
	}
	if lex.acceptKeyword("noexcept") {
		lex.emit(itemNoexcept)
	}
	if lex.acceptRune(';') {
		lex.emit(itemSemicolon)
		return nil
	}
	if !lex.acceptDelimitedBlob('{', '}') {
		return errors.New("function body or ;")
	}
	lex.emit(itemMethodBody)
	return nil
}

// Assumptions: cursor is just before a data field declaration in a class or struct
// Directly emits: entire content of the method declaration or implementation
func lexField(lex *lexer) error {
	if err := lexSimpleType(lex); err != nil {
		return err
	}
	if !lex.acceptIdentifier() {
		return errors.New("valid identifier")
	}
	lex.emit(itemIdentifier)
	if lex.acceptRune('[') {
		lex.emit(itemOpenBracket)
		if !lex.acceptPositiveInteger() {
			return errors.New("positive integer")
		}
		lex.emit(itemArrayLength)
		if !lex.acceptRune(']') {
			return errors.New("valid array length")
		}
		lex.emit(itemCloseBracket)
	}
	if lex.acceptRune('=') {
		lex.emit(itemEquals)
		if !lex.acceptTerminatedBlob(';') {
			return errors.New("right-hand side of assignment terminated by ;")
		}
		lex.emit(itemDefaultValue)
	}
	if !lex.acceptRune(';') {
		return errors.New("; after field")
	}
	lex.emit(itemSemicolon)
	return nil
}

// For now, SimpleType is a very restrictive subset of the C++ type expression language. It should
// not contain parentheses or commas, so C callbacks are not allowed unless you alias them first.
// Basically, a type is a bag of tokens that must include exactly one valid C identifier, along with
// a mix of spaces, "*", "&", "::", "const", "<", ">".
//
// Assumptions: cursor is just before a type identifier
// Directly emits: itemSimpleType
func lexSimpleType(lex *lexer) error {
	encounteredIdentifier := false
	for {
		switch {
		case lex.acceptString("::"), lex.acceptRune('<'):
			encounteredIdentifier = false
			continue
		case lex.acceptAny("*& >\t\n"):
			continue
		case lex.acceptKeyword("const"):
			continue
		case encounteredIdentifier:
			lex.emit(itemSimpleType)
			return nil
		case lex.acceptIdentifier():
			encounteredIdentifier = true
		default:
			return errors.New("valid type identifier")
		}
	}
}
