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

package parser_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/ast"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/parser"
	"github.com/google/go-cmp/cmp"
)

var ignoreSource = cmp.FilterPath(func(p cmp.Path) bool {
	return p.Last().String() == ".Source"
}, cmp.Ignore())

func TestParser(t *testing.T) {
	type test struct {
		location string
		src      string
		expect   ast.AST
	}

	for _, test := range []test{
		{
			fileutils.ThisLine(),
			"enum E {}",
			ast.AST{
				Enums: []ast.EnumDecl{{Name: "E"}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"enum E { A @attr B C }",
			ast.AST{
				Enums: []ast.EnumDecl{{
					Name: "E",
					Entries: []ast.EnumEntry{
						{Name: "A"},
						{
							Attributes: ast.Attributes{{
								Name:   "attr",
								Values: nil,
							}},
							Name: "B",
						},
						{Name: "C"},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"type T",
			ast.AST{
				Types: []ast.TypeDecl{{Name: "T"}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"type T<A, B, C>",
			ast.AST{
				Types: []ast.TypeDecl{{
					Name: "T",
					TemplateParams: []ast.TemplateParam{
						{Name: "A"},
						{Name: "B"},
						{Name: "C"},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"@attr type T",
			ast.AST{
				Types: []ast.TypeDecl{{
					Attributes: ast.Attributes{
						{Name: "attr", Values: nil},
					},
					Name: "T",
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"@attr_a @attr_b type T",
			ast.AST{
				Types: []ast.TypeDecl{{
					Attributes: ast.Attributes{
						{Name: "attr_a", Values: nil},
						{Name: "attr_b", Values: nil},
					},
					Name: "T",
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			`@attr("a", "b") type T`, ast.AST{
				Types: []ast.TypeDecl{{
					Attributes: ast.Attributes{
						{Name: "attr", Values: []any{"a", "b"}},
					},
					Name: "T",
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			`@attr(1, "x", 2.0) type T`, ast.AST{
				Types: []ast.TypeDecl{{
					Attributes: ast.Attributes{
						{Name: "attr", Values: []any{1, "x", 2.0}},
					},
					Name: "T",
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"match M : A",
			ast.AST{
				Matchers: []ast.MatcherDecl{{
					Name: "M",
					Options: ast.MatcherOptions{
						Types: ast.TemplatedNames{
							{Name: "A"},
						},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"match M : A | B",
			ast.AST{
				Matchers: []ast.MatcherDecl{{
					Name: "M",
					Options: ast.MatcherOptions{
						Types: ast.TemplatedNames{
							{Name: "A"},
							{Name: "B"},
						},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"match M : A.B",
			ast.AST{
				Matchers: []ast.MatcherDecl{{
					Name: "M",
					Options: ast.MatcherOptions{
						Enums: ast.MemberNames{
							{Owner: "A", Member: "B"},
						},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"match M : A.B | B.C",
			ast.AST{
				Matchers: []ast.MatcherDecl{{
					Name: "M",
					Options: ast.MatcherOptions{
						Enums: ast.MemberNames{
							{Owner: "A", Member: "B"},
							{Owner: "B", Member: "C"},
						},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"fn F()",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind:       ast.Builtin,
					Name:       "F",
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"@attr fn F()",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					Attributes: ast.Attributes{
						{Name: "attr", Values: nil},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"fn F(a)",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"fn F(a: T)",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "T"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"fn F(a, b)",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
						{Type: ast.TemplatedName{Name: "b"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"fn F<A : B<C> >()",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					ExplicitTemplateParams: []ast.TemplateParam{
						{
							Name: "A", Type: ast.TemplatedName{
								Name: "B",
								TemplateArgs: ast.TemplatedNames{
									{Name: "C"},
								},
							},
						},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"fn F<T>(a: X, b: Y<T>)",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					ExplicitTemplateParams: []ast.TemplateParam{
						{Name: "T"},
					},
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "X"}},
						{Name: "b", Type: ast.TemplatedName{
							Name:         "Y",
							TemplateArgs: ast.TemplatedNames{{Name: "T"}},
						}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"implicit(A : B<C>) fn F()",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					ImplicitTemplateParams: []ast.TemplateParam{
						{
							Name: "A",
							Type: ast.TemplatedName{
								Name: "B",
								TemplateArgs: ast.TemplatedNames{
									{Name: "C"},
								},
							},
						},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"implicit(T) fn F(a: X, b: Y<T>)",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					ImplicitTemplateParams: []ast.TemplateParam{
						{Name: "T"},
					},
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "X"}},
						{Name: "b", Type: ast.TemplatedName{
							Name:         "Y",
							TemplateArgs: ast.TemplatedNames{{Name: "T"}},
						}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"implicit(B: C<D>) fn F<A>()",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					ExplicitTemplateParams: []ast.TemplateParam{
						{Name: "A"},
					},
					ImplicitTemplateParams: []ast.TemplateParam{
						{
							Name: "B",
							Type: ast.TemplatedName{
								Name: "C",
								TemplateArgs: ast.TemplatedNames{
									{Name: "D"},
								},
							},
						},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"implicit(T) fn F(a: X, b: Y<T>)",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					ImplicitTemplateParams: []ast.TemplateParam{
						{Name: "T"},
					},
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "X"}},
						{Name: "b", Type: ast.TemplatedName{
							Name:         "Y",
							TemplateArgs: ast.TemplatedNames{{Name: "T"}},
						}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"fn F() -> X",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind:       ast.Builtin,
					Name:       "F",
					ReturnType: &ast.TemplatedName{Name: "X"},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"fn F() -> X<T>",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					ReturnType: &ast.TemplatedName{
						Name:         "X",
						TemplateArgs: ast.TemplatedNames{{Name: "T"}},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"op F()",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind:       ast.Operator,
					Name:       "F",
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"@attr op F()",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					Attributes: ast.Attributes{
						{Name: "attr", Values: nil},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"op F(a)",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"op F(@blah a)",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					Parameters: ast.Parameters{
						{
							Attributes: ast.Attributes{{Name: "blah", Values: nil}},
							Type:       ast.TemplatedName{Name: "a"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"op F(a: T)",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "T"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"op F(a, b)",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
						{Type: ast.TemplatedName{Name: "b"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"op F<A : B<C> >()",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					ExplicitTemplateParams: []ast.TemplateParam{
						{
							Name: "A", Type: ast.TemplatedName{
								Name: "B",
								TemplateArgs: ast.TemplatedNames{
									{Name: "C"},
								},
							},
						},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"op F<T>(a: X, b: Y<T>)",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					ExplicitTemplateParams: []ast.TemplateParam{
						{Name: "T"},
					},
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "X"}},
						{Name: "b", Type: ast.TemplatedName{
							Name:         "Y",
							TemplateArgs: ast.TemplatedNames{{Name: "T"}},
						}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"implicit(A : B<C>) op F()",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					ImplicitTemplateParams: []ast.TemplateParam{
						{
							Name: "A", Type: ast.TemplatedName{
								Name: "B",
								TemplateArgs: ast.TemplatedNames{
									{Name: "C"},
								},
							},
						},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"implicit(T) op F(a: X, b: Y<T>)",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					ImplicitTemplateParams: []ast.TemplateParam{
						{Name: "T"},
					},
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "X"}},
						{Name: "b", Type: ast.TemplatedName{
							Name:         "Y",
							TemplateArgs: ast.TemplatedNames{{Name: "T"}},
						}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"implicit(D) op F<A : B<C> >()",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					ExplicitTemplateParams: []ast.TemplateParam{
						{
							Name: "A", Type: ast.TemplatedName{
								Name: "B",
								TemplateArgs: ast.TemplatedNames{
									{Name: "C"},
								},
							},
						},
					},
					ImplicitTemplateParams: []ast.TemplateParam{
						{Name: "D"},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"op F() -> X",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind:       ast.Operator,
					Name:       "F",
					ReturnType: &ast.TemplatedName{Name: "X"},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"op F() -> X<T>",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					ReturnType: &ast.TemplatedName{
						Name:         "X",
						TemplateArgs: ast.TemplatedNames{{Name: "T"}},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"ctor F()",
			ast.AST{
				Constructors: []ast.IntrinsicDecl{{
					Kind:       ast.Constructor,
					Name:       "F",
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"@attr ctor F()",
			ast.AST{
				Constructors: []ast.IntrinsicDecl{{
					Kind: ast.Constructor,
					Name: "F",
					Attributes: ast.Attributes{
						{Name: "attr", Values: nil},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"ctor F(a)",
			ast.AST{
				Constructors: []ast.IntrinsicDecl{{
					Kind: ast.Constructor,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"ctor F(a: T)",
			ast.AST{
				Constructors: []ast.IntrinsicDecl{{
					Kind: ast.Constructor,
					Name: "F",
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "T"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"ctor F(a, b)",
			ast.AST{
				Constructors: []ast.IntrinsicDecl{{
					Kind: ast.Constructor,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
						{Type: ast.TemplatedName{Name: "b"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"ctor F<A : B<C> >()",
			ast.AST{
				Constructors: []ast.IntrinsicDecl{{
					Kind: ast.Constructor,
					Name: "F",
					ExplicitTemplateParams: []ast.TemplateParam{
						{
							Name: "A", Type: ast.TemplatedName{
								Name: "B",
								TemplateArgs: ast.TemplatedNames{
									{Name: "C"},
								},
							},
						},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"ctor F<T>(a: X, b: Y<T>)",
			ast.AST{
				Constructors: []ast.IntrinsicDecl{{
					Kind: ast.Constructor,
					Name: "F",
					ExplicitTemplateParams: []ast.TemplateParam{
						{Name: "T"},
					},
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "X"}},
						{Name: "b", Type: ast.TemplatedName{
							Name:         "Y",
							TemplateArgs: ast.TemplatedNames{{Name: "T"}},
						}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"ctor F() -> X",
			ast.AST{
				Constructors: []ast.IntrinsicDecl{{
					Kind:       ast.Constructor,
					Name:       "F",
					ReturnType: &ast.TemplatedName{Name: "X"},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"ctor F() -> X<T>",
			ast.AST{
				Constructors: []ast.IntrinsicDecl{{
					Kind: ast.Constructor,
					Name: "F",
					ReturnType: &ast.TemplatedName{
						Name:         "X",
						TemplateArgs: ast.TemplatedNames{{Name: "T"}},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"conv F()",
			ast.AST{
				Converters: []ast.IntrinsicDecl{{
					Kind:       ast.Converter,
					Name:       "F",
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"@attr conv F()",
			ast.AST{
				Converters: []ast.IntrinsicDecl{{
					Kind: ast.Converter,
					Name: "F",
					Attributes: ast.Attributes{
						{Name: "attr", Values: nil},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"conv F(a)",
			ast.AST{
				Converters: []ast.IntrinsicDecl{{
					Kind: ast.Converter,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"conv F(a: T)",
			ast.AST{
				Converters: []ast.IntrinsicDecl{{
					Kind: ast.Converter,
					Name: "F",
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "T"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"conv F(a, b)",
			ast.AST{
				Converters: []ast.IntrinsicDecl{{
					Kind: ast.Converter,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
						{Type: ast.TemplatedName{Name: "b"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"conv F<A : B<C> >()",
			ast.AST{
				Converters: []ast.IntrinsicDecl{{
					Kind: ast.Converter,
					Name: "F",
					ExplicitTemplateParams: []ast.TemplateParam{
						{
							Name: "A", Type: ast.TemplatedName{
								Name: "B",
								TemplateArgs: ast.TemplatedNames{
									{Name: "C"},
								},
							},
						},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"conv F<T>(a: X, b: Y<T>)",
			ast.AST{
				Converters: []ast.IntrinsicDecl{{
					Kind: ast.Converter,
					Name: "F",
					ExplicitTemplateParams: []ast.TemplateParam{
						{Name: "T"},
					},
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "X"}},
						{Name: "b", Type: ast.TemplatedName{
							Name:         "Y",
							TemplateArgs: ast.TemplatedNames{{Name: "T"}},
						}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"conv F() -> X",
			ast.AST{
				Converters: []ast.IntrinsicDecl{{
					Kind:       ast.Converter,
					Name:       "F",
					ReturnType: &ast.TemplatedName{Name: "X"},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			fileutils.ThisLine(),
			"conv F() -> X<T>",
			ast.AST{
				Converters: []ast.IntrinsicDecl{{
					Kind: ast.Converter,
					Name: "F",
					ReturnType: &ast.TemplatedName{
						Name:         "X",
						TemplateArgs: ast.TemplatedNames{{Name: "T"}},
					},
					Parameters: ast.Parameters{},
				}},
			}},
	} {
		got, err := parser.Parse(test.src, "file.txt")
		if err != nil {
			t.Errorf("\n%v\nWhile parsing:\n%s\nParse() returned error: %v",
				test.location, test.src, err)
			continue
		}

		if diff := cmp.Diff(got, &test.expect, ignoreSource); diff != "" {
			t.Errorf("\n%v\nWhile parsing:\n%s\n\n%s",
				test.location, test.src, diff)
		}
	}
}

func TestErrors(t *testing.T) {
	type test struct {
		src    string
		expect string
	}

	for _, test := range []test{
		{
			"£",
			"test.txt:1:1: unexpected '£'",
		},
		{
			"123",
			"test.txt:1:1 unexpected token 'integer'",
		},
		{
			"@123",
			"test.txt:1:2 expected 'ident' for attribute name, got 'integer'",
		},
	} {
		got, err := parser.Parse(test.src, "test.txt")
		gotErr := ""
		if err != nil {
			gotErr = err.Error()
		}
		if test.expect != gotErr {
			t.Errorf(`Parse() returned error "%+v", expected error "%+v"`, gotErr, test.expect)
		}
		if got != nil {
			t.Errorf("Lex() returned non-nil for error")
		}
	}
}
