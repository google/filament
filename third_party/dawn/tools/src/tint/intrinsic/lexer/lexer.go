// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Package lexer provides a basic lexer for the Tint intrinsic definition
// language
package lexer

import (
	"fmt"
	"unicode"

	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/tok"
)

// Lex produces a list of tokens for the given source code
func Lex(src []rune, filepath string) ([]tok.Token, error) {
	l := lexer{
		tok.Location{Line: 1, Column: 1, Rune: 0, Filepath: filepath},
		src,
		[]tok.Token{},
	}
	if err := l.lex(); err != nil {
		return nil, err
	}
	return l.tokens, nil
}

type lexer struct {
	loc    tok.Location
	runes  []rune
	tokens []tok.Token
}

// lex() lexes the source, populating l.tokens
func (l *lexer) lex() error {
	for {
		switch l.peek(0) {
		case 0:
			return nil
		case ' ', '\t':
			l.next()
		case '\r', '\n':
			l.next()
		case '@':
			l.tok(1, tok.Attr)
		case '(':
			l.tok(1, tok.Lparen)
		case ')':
			l.tok(1, tok.Rparen)
		case '{':
			l.tok(1, tok.Lbrace)
		case '}':
			l.tok(1, tok.Rbrace)
		case ':':
			l.tok(1, tok.Colon)
		case ',':
			l.tok(1, tok.Comma)
		case '*':
			l.tok(1, tok.Star)
		case '+':
			l.tok(1, tok.Plus)
		case '%':
			l.tok(1, tok.Modulo)
		case '^':
			l.tok(1, tok.Xor)
		case '~':
			l.tok(1, tok.Complement)
		case '"':
			start := l.loc
			l.next() // Skip opening quote
			n := l.count(toFirst('\n', '"'))
			if l.peek(n) != '"' {
				return fmt.Errorf("%v unterminated string", start)
			}
			l.tok(n, tok.String)
			l.next() // Skip closing quote
		default:
			switch {
			case l.peek(0) == '/' && l.peek(1) == '/':
				l.skip(l.count(toFirst('\n')))
				l.next() // Consume newline
			case l.match("/", tok.Divide):
			case l.match(".", tok.Dot):
			case l.match("->", tok.Arrow):
			case unicode.IsLetter(l.peek(0)) || l.peek(0) == '_':
				n := l.count(alphaNumericOrUnderscore)
				switch string(l.runes[:n]) {
				case "fn":
					l.tok(n, tok.Function)
				case "op":
					l.tok(n, tok.Operator)
				case "enum":
					l.tok(n, tok.Enum)
				case "type":
					l.tok(n, tok.Type)
				case "ctor":
					l.tok(n, tok.Constructor)
				case "conv":
					l.tok(n, tok.Converter)
				case "match":
					l.tok(n, tok.Match)
				case "import":
					l.tok(n, tok.Import)
				case "implicit":
					l.tok(n, tok.Implicit)
				default:
					l.tok(n, tok.Identifier)
				}
			case unicode.IsNumber(l.peek(0)) || l.peek(0) == '-':
				isFloat := false
				isNegative := false
				isFirst := true
				pred := func(r rune) bool {
					if isFirst && r == '-' {
						isNegative = true
						isFirst = false
						return true
					}
					isFirst = false

					if unicode.IsNumber(r) {
						return true
					}

					if !isFloat && r == '.' {
						isFloat = true
						return true
					}
					return false
				}
				n := l.count(pred)
				if isNegative && n == 1 {
					l.tok(1, tok.Minus)
				} else if isFloat {
					l.tok(n, tok.Float)
				} else {
					l.tok(n, tok.Integer)
				}
			case l.match("&&", tok.AndAnd):
			case l.match("&", tok.And):
			case l.match("||", tok.OrOr):
			case l.match("|", tok.Or):
			case l.match("!=", tok.NotEqual):
			case l.match("!", tok.Not):
			case l.match("==", tok.Equal):
			case l.match("=", tok.Assign):
			case l.match("<<", tok.Shl):
			case l.match("<=", tok.Le):
			case l.match("<", tok.Lt):
			case l.match(">=", tok.Ge):
			case l.match(">>", tok.Shr):
			case l.match(">", tok.Gt):
			case l.match("[", tok.Lbracket):
			case l.match("]", tok.Rbracket):
			default:
				return fmt.Errorf("%v: unexpected '%v'", l.loc, string(l.runes[0]))
			}
		}
	}
}

// next() consumes and returns the next rune in the source, or 0 if reached EOF
func (l *lexer) next() rune {
	if len(l.runes) > 0 {
		r := l.runes[0]
		l.runes = l.runes[1:]
		l.loc.Rune++
		if r == '\n' {
			l.loc.Line++
			l.loc.Column = 1
		} else {
			l.loc.Column++
		}
		return r
	}
	return 0
}

// skip() consumes the next `n` runes in the source
func (l *lexer) skip(n int) {
	for i := 0; i < n; i++ {
		l.next()
	}
}

// peek() returns the rune `i` runes ahead of the current position
func (l *lexer) peek(i int) rune {
	if i >= len(l.runes) {
		return 0
	}
	return l.runes[i]
}

// predicate is a function that can be passed to count()
type predicate func(r rune) bool

// count() returns the number of sequential runes from the current position that
// match the predicate `p`
func (l *lexer) count(p predicate) int {
	for i := 0; i < len(l.runes); i++ {
		if !p(l.peek(i)) {
			return i
		}
	}
	return len(l.runes)
}

// tok() appends a new token of kind `k` using the next `n` runes.
// The next `n` runes are consumed by tok().
func (l *lexer) tok(n int, k tok.Kind) {
	start := l.loc
	runes := l.runes[:n]
	l.skip(n)
	end := l.loc

	src := tok.Source{S: start, E: end}
	l.tokens = append(l.tokens, tok.Token{Kind: k, Source: src, Runes: runes})
}

// match() checks whether the next runes are equal to `s`. If they are, then
// these runes are used to append a new token of kind `k`, and match() returns
// true. If the next runes are not equal to `s` then false is returned, and no
// runes are consumed.
func (l *lexer) match(s string, kind tok.Kind) bool {
	runes := []rune(s)
	if len(l.runes) < len(runes) {
		return false
	}
	for i, r := range runes {
		if l.runes[i] != r {
			return false
		}
	}
	l.tok(len(runes), kind)
	return true
}

// toFirst() returns a predicate that returns true if the rune is not in `runes`
// toFirst() is intended to be used with count(), so `count(toFirst('x'))` will
// count up to, but not including the number of consecutive runes that are not
// 'x'.
func toFirst(runes ...rune) predicate {
	return func(r rune) bool {
		for _, t := range runes {
			if t == r {
				return false
			}
		}
		return true
	}
}

// alphaNumericOrUnderscore() returns true if the rune `r` is a number, letter
// or underscore.
func alphaNumericOrUnderscore(r rune) bool {
	return r == '_' || unicode.IsLetter(r) || unicode.IsNumber(r)
}
