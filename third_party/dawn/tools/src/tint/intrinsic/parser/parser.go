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

// Package parser provides a basic parser for the Tint builtin definition
// language
package parser

import (
	"fmt"
	"os"
	"strconv"

	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/ast"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/lexer"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/tok"
)

// Parse produces a list of tokens for the given source code
func Parse(source, filepath string) (*ast.AST, error) {
	out := &ast.AST{}
	if err := parse(source, filepath, out); err != nil {
		return nil, err
	}
	return out, nil
}

func parse(source, filepath string, out *ast.AST) error {
	runes := []rune(source)
	tokens, err := lexer.Lex(runes, filepath)
	if err != nil {
		return err
	}
	p := parser{tokens: tokens}
	return p.parse(out)
}

type parser struct {
	tokens []tok.Token
	err    error
}

func (p *parser) parse(out *ast.AST) error {
	var attributes ast.Attributes
	var implicitParams []ast.TemplateParam
	for p.err == nil {
		t := p.peek(0)
		if t == nil {
			break
		}
		switch t.Kind {
		case tok.Attr:
			attributes = append(attributes, p.attributes()...)
		case tok.Enum:
			if len(implicitParams) > 0 {
				p.err = fmt.Errorf("%v unexpected implicitParams", implicitParams[0].Source)
			}
			out.Enums = append(out.Enums, p.enumDecl(attributes))
			attributes = nil
		case tok.Match:
			if len(attributes) > 0 {
				p.err = fmt.Errorf("%v unexpected attribute", attributes[0].Source)
			}
			if len(implicitParams) > 0 {
				p.err = fmt.Errorf("%v unexpected implicitParams", implicitParams[0].Source)
			}
			out.Matchers = append(out.Matchers, p.matcherDecl())
		case tok.Import:
			if len(attributes) > 0 {
				p.err = fmt.Errorf("%v unexpected attribute", attributes[0].Source)
			}
			if len(implicitParams) > 0 {
				p.err = fmt.Errorf("%v unexpected implicitParams", implicitParams[0].Source)
			}
			p.importDecl(out)
		case tok.Type:
			if len(implicitParams) > 0 {
				p.err = fmt.Errorf("%v unexpected implicitParams", implicitParams[0].Source)
			}
			out.Types = append(out.Types, p.typeDecl(attributes))
			attributes = nil
		case tok.Function:
			out.Builtins = append(out.Builtins, p.builtinDecl(attributes, implicitParams))
			attributes = nil
			implicitParams = nil
		case tok.Operator:
			out.Operators = append(out.Operators, p.operatorDecl(attributes, implicitParams))
			attributes = nil
			implicitParams = nil
		case tok.Constructor:
			out.Constructors = append(out.Constructors, p.constructorDecl(attributes, implicitParams))
			attributes = nil
			implicitParams = nil
		case tok.Converter:
			out.Converters = append(out.Converters, p.converterDecl(attributes, implicitParams))
			attributes = nil
			implicitParams = nil
		case tok.Implicit:
			p.expect(tok.Implicit, "implicit")
			p.expect(tok.Lparen, "implicit template parameter list")
			for p.err == nil && p.peekIs(0, tok.Identifier) {
				implicitParams = append(implicitParams, p.templateParam())
			}
			p.expect(tok.Rparen, "implicit template parameter list")

		default:
			p.err = fmt.Errorf("%v unexpected token '%v'", t.Source, t.Kind)
		}
		if p.err != nil {
			return p.err
		}
	}
	return nil
}

func (p *parser) enumDecl(decos ast.Attributes) ast.EnumDecl {
	p.expect(tok.Enum, "enum declaration")
	name := p.expect(tok.Identifier, "enum name")
	e := ast.EnumDecl{Source: name.Source, Name: string(name.Runes), Attributes: decos}

	p.expect(tok.Lbrace, "enum declaration")
	for p.err == nil && p.match(tok.Rbrace) == nil {
		e.Entries = append(e.Entries, p.enumEntry())
	}
	return e
}

func (p *parser) enumEntry() ast.EnumEntry {
	decos := p.attributes()
	name := p.expect(tok.Identifier, "enum entry")
	return ast.EnumEntry{Source: name.Source, Attributes: decos, Name: string(name.Runes)}
}

func (p *parser) matcherDecl() ast.MatcherDecl {
	p.expect(tok.Match, "matcher declaration")
	name := p.expect(tok.Identifier, "matcher name")
	m := ast.MatcherDecl{Source: name.Source, Name: string(name.Runes)}
	p.expect(tok.Colon, "matcher declaration")
	if p.peekIs(1, tok.Dot) { // enum list
		for p.err == nil {
			m.Options.Enums = append(m.Options.Enums, p.memberName())
			if p.match(tok.Or) == nil {
				break
			}
		}
	} else { // type list
		for p.err == nil {
			m.Options.Types = append(m.Options.Types, p.templatedName())
			if p.match(tok.Or) == nil {
				break
			}
		}
	}
	return m
}

func (p *parser) importDecl(out *ast.AST) {
	p.expect(tok.Import, "import declaration")
	path := p.string()

	content, err := os.ReadFile(path)
	if err != nil {
		p.err = fmt.Errorf("%v failed to load '%v': %w",
			p.tokens[0].Source, path, err)
		return
	}

	p.err = parse(string(content), path, out)
}

func (p *parser) typeDecl(decos ast.Attributes) ast.TypeDecl {
	p.expect(tok.Type, "type declaration")
	name := p.expect(tok.Identifier, "type name")
	m := ast.TypeDecl{
		Source:     name.Source,
		Attributes: decos,
		Name:       string(name.Runes),
	}
	m.TemplateParams = p.typeTemplateParams()

	return m
}

func (p *parser) attributes() ast.Attributes {
	var out ast.Attributes
	for p.match(tok.Attr) != nil && p.err == nil {
		name := p.expect(tok.Identifier, "attribute name")
		var values []any
		if p.match(tok.Lparen) != nil {
		loop:
			for p.err == nil {
				t := p.next()
				switch t.Kind {
				case tok.Rparen:
					break loop
				case tok.String:
					values = append(values, string(t.Runes))
				case tok.Integer:
					i, _ := strconv.ParseInt(string(t.Runes), 10, 64)
					values = append(values, int(i))
				case tok.Float:
					f, _ := strconv.ParseFloat(string(t.Runes), 64)
					values = append(values, f)
				default:
					p.err = fmt.Errorf("%v invalid attribute value kind: %v", t.Source, t.Kind)
					return nil
				}
				if p.match(tok.Comma) == nil {
					break
				}
			}
			p.expect(tok.Rparen, "attribute values")
		}
		out = append(out, ast.Attribute{
			Source: name.Source,
			Name:   string(name.Runes),
			Values: values,
		})
	}
	return out
}

func (p *parser) builtinDecl(decos ast.Attributes, implicitParams []ast.TemplateParam) ast.IntrinsicDecl {
	p.expect(tok.Function, "function declaration")
	name := p.expect(tok.Identifier, "function name")
	f := ast.IntrinsicDecl{
		Source:     name.Source,
		Kind:       ast.Builtin,
		Attributes: decos,
		Name:       string(name.Runes),
	}
	f.ExplicitTemplateParams = p.intrinsicTemplateParams()
	f.ImplicitTemplateParams = implicitParams
	f.Parameters = p.parameters()
	if p.match(tok.Arrow) != nil {
		ret := p.templatedName()
		f.ReturnType = &ret
	}
	return f
}

func (p *parser) operatorDecl(decos ast.Attributes, implicitParams []ast.TemplateParam) ast.IntrinsicDecl {
	p.expect(tok.Operator, "operator declaration")
	name := p.next()
	f := ast.IntrinsicDecl{
		Source:     name.Source,
		Kind:       ast.Operator,
		Attributes: decos,
		Name:       string(name.Runes),
	}
	f.ExplicitTemplateParams = p.intrinsicTemplateParams()
	f.ImplicitTemplateParams = implicitParams
	f.Parameters = p.parameters()
	if p.match(tok.Arrow) != nil {
		ret := p.templatedName()
		f.ReturnType = &ret
	}
	return f
}

func (p *parser) constructorDecl(decos ast.Attributes, implicitParams []ast.TemplateParam) ast.IntrinsicDecl {
	p.expect(tok.Constructor, "constructor declaration")
	name := p.next()
	f := ast.IntrinsicDecl{
		Source:     name.Source,
		Kind:       ast.Constructor,
		Attributes: decos,
		Name:       string(name.Runes),
	}
	f.ExplicitTemplateParams = p.intrinsicTemplateParams()
	f.ImplicitTemplateParams = implicitParams
	f.Parameters = p.parameters()
	if p.match(tok.Arrow) != nil {
		ret := p.templatedName()
		f.ReturnType = &ret
	}
	return f
}

func (p *parser) converterDecl(decos ast.Attributes, implicitParams []ast.TemplateParam) ast.IntrinsicDecl {
	p.expect(tok.Converter, "converter declaration")
	name := p.next()
	f := ast.IntrinsicDecl{
		Source:     name.Source,
		Kind:       ast.Converter,
		Attributes: decos,
		Name:       string(name.Runes),
	}
	f.ExplicitTemplateParams = p.intrinsicTemplateParams()
	f.ImplicitTemplateParams = implicitParams
	f.Parameters = p.parameters()
	if p.match(tok.Arrow) != nil {
		ret := p.templatedName()
		f.ReturnType = &ret
	}
	return f
}

func (p *parser) parameters() ast.Parameters {
	l := ast.Parameters{}
	p.expect(tok.Lparen, "function parameter list")
	if p.match(tok.Rparen) == nil {
		for p.err == nil {
			l = append(l, p.parameter())
			if p.match(tok.Comma) == nil {
				break
			}
		}
		p.expect(tok.Rparen, "function parameter list")
	}
	return l
}

func (p *parser) parameter() ast.Parameter {
	attributes := p.attributes()
	if p.peekIs(1, tok.Colon) {
		// name type
		name := p.expect(tok.Identifier, "parameter name")
		p.expect(tok.Colon, "parameter type")
		return ast.Parameter{
			Source:     name.Source,
			Name:       string(name.Runes),
			Attributes: attributes,
			Type:       p.templatedName(),
		}
	}
	// type
	ty := p.templatedName()
	return ast.Parameter{
		Source:     ty.Source,
		Attributes: attributes,
		Type:       ty,
	}
}

func (p *parser) string() string {
	s := p.expect(tok.String, "string")
	return string(s.Runes)
}

func (p *parser) memberName() ast.MemberName {
	owner := p.expect(tok.Identifier, "member name")
	p.expect(tok.Dot, "member name")
	member := p.expect(tok.Identifier, "member name")
	return ast.MemberName{
		Source: member.Source,
		Owner:  string(owner.Runes),
		Member: string(member.Runes),
	}
}

func (p *parser) templatedName() ast.TemplatedName {
	name := p.expect(tok.Identifier, "type name")
	m := ast.TemplatedName{Source: name.Source, Name: string(name.Runes)}
	if p.match(tok.Lt) != nil {
		for p.err == nil {
			m.TemplateArgs = append(m.TemplateArgs, p.templatedName())
			if p.match(tok.Comma) == nil {
				break
			}
		}
		p.expect(tok.Gt, "template argument type list")
	}
	return m
}

func (p *parser) typeTemplateParams() []ast.TemplateParam {
	if p.match(tok.Lt) == nil {
		return nil
	}
	t := []ast.TemplateParam{}
	for p.err == nil && p.peekIs(0, tok.Identifier) {
		t = append(t, p.templateParam())
	}
	p.expect(tok.Gt, "type template parameter list")
	return t
}

func (p *parser) intrinsicTemplateParams() (explicit []ast.TemplateParam) {
	if p.match(tok.Lt) != nil { // <...>
		for p.err == nil && p.peekIs(0, tok.Identifier) {
			explicit = append(explicit, p.templateParam())
		}
		p.expect(tok.Gt, "explicit template parameter list")
	}
	return explicit
}

func (p *parser) templateParam() ast.TemplateParam {
	name := p.match(tok.Identifier)
	t := ast.TemplateParam{
		Source: name.Source,
		Name:   string(name.Runes),
	}
	if p.match(tok.Colon) != nil {
		t.Type = p.templatedName()
	}
	p.match(tok.Comma)
	return t
}

func (p *parser) expect(kind tok.Kind, use string) tok.Token {
	if p.err != nil {
		return tok.Invalid
	}
	t := p.match(kind)
	if t == nil {
		if len(p.tokens) > 0 {
			p.err = fmt.Errorf("%v expected '%v' for %v, got '%v'",
				p.tokens[0].Source, kind, use, p.tokens[0].Kind)
		} else {
			p.err = fmt.Errorf("expected '%v' for %v, but reached end of file", kind, use)
		}
		return tok.Invalid
	}
	return *t
}

func (p *parser) ident(use string) string {
	return string(p.expect(tok.Identifier, use).Runes)
}

func (p *parser) match(kind tok.Kind) *tok.Token {
	if p.err != nil || len(p.tokens) == 0 {
		return nil
	}
	t := p.tokens[0]
	if t.Kind != kind {
		return nil
	}
	p.tokens = p.tokens[1:]
	return &t
}

func (p *parser) next() *tok.Token {
	if p.err != nil {
		return nil
	}
	if len(p.tokens) == 0 {
		p.err = fmt.Errorf("reached end of file")
	}
	t := p.tokens[0]
	p.tokens = p.tokens[1:]
	return &t
}

func (p *parser) peekIs(i int, kind tok.Kind) bool {
	t := p.peek(i)
	if t == nil {
		return false
	}
	return t.Kind == kind
}

func (p *parser) peek(i int) *tok.Token {
	if len(p.tokens) <= i {
		return nil
	}
	return &p.tokens[i]
}
