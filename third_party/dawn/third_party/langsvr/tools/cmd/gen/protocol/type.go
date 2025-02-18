// Copyright 2024 The langsvr Authors
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

package protocol

// Type is an LSP type
type Type interface {
	isType()
	SubTypes() []Type
}

func RecursiveTypesOf(t Type) []Type {
	stack := []Type{t}
	seen := map[Type]struct{}{}
	types := []Type{}
	for len(stack) > 0 {
		stack, t = stack[:len(stack)-1], stack[len(stack)-1]
		if _, existing := seen[t]; !existing {
			seen[t] = struct{}{}
			types = append(types, t)
			stack = append(stack, t.SubTypes()...)
		}
	}
	return types
}

type URIType struct{}

func (*URIType) isType()          {}
func (*URIType) SubTypes() []Type { return nil }

type DocumentUriType struct{}

func (*DocumentUriType) isType()          {}
func (*DocumentUriType) SubTypes() []Type { return nil }

type IntegerType struct{}

func (*IntegerType) isType()          {}
func (*IntegerType) SubTypes() []Type { return nil }

type UintegerType struct{}

func (*UintegerType) isType()          {}
func (*UintegerType) SubTypes() []Type { return nil }

type DecimalType struct{}

func (*DecimalType) isType()          {}
func (*DecimalType) SubTypes() []Type { return nil }

type RegExpType struct{}

func (*RegExpType) isType()          {}
func (*RegExpType) SubTypes() []Type { return nil }

type StringType struct{}

func (*StringType) isType()          {}
func (*StringType) SubTypes() []Type { return nil }

type BooleanType struct{}

func (*BooleanType) isType()          {}
func (*BooleanType) SubTypes() []Type { return nil }

type NullType struct{}

func (*NullType) isType()          {}
func (*NullType) SubTypes() []Type { return nil }

// AndType represents an and type (e.g. TextDocumentParams & WorkDoneProgressParams).
type AndType struct{ Items []Type }

func (*AndType) isType()            {}
func (t *AndType) SubTypes() []Type { return t.Items }

// ArrayType represents an array type (e.g. TextDocument[]).
type ArrayType struct{ Element Type }

func (*ArrayType) isType()            {}
func (t *ArrayType) SubTypes() []Type { return []Type{t.Element} }

// BooleanLiteralType represents a boolean literal type (e.g. kind: true).
// kind: booleanLiteral
type BooleanLiteralType struct{ Value bool }

func (*BooleanLiteralType) isType()          {}
func (*BooleanLiteralType) SubTypes() []Type { return nil }

// IntegerLiteralType represents an integer literal type (e.g. kind: 1).
type IntegerLiteralType struct{ Value bool }

func (*IntegerLiteralType) isType()          {}
func (*IntegerLiteralType) SubTypes() []Type { return nil }

// MapType represents a JSON object map (e.g. interface Map<K extends string | integer, V> { [key: K] => V; }).
type MapType struct {
	Key   Type
	Value Type
}

func (*MapType) isType()            {}
func (t *MapType) SubTypes() []Type { return []Type{t.Key, t.Value} }

// OrType represents an or type (e.g. Location | LocationLink)
type OrType struct{ Items []Type }

func (*OrType) isType()            {}
func (t *OrType) SubTypes() []Type { return t.Items }

// StringLiteralType represents a string literal type (e.g. kind: 'rename')
type StringLiteralType struct{ Value string }

func (*StringLiteralType) isType()          {}
func (*StringLiteralType) SubTypes() []Type { return nil }

// StructureLiteralType represents a literal structure (e.g. property: { start: uinteger; end: uinteger; })
type StructureLiteralType struct{ Value *StructureLiteral }

func (*StructureLiteralType) isType()          {}
func (*StructureLiteralType) SubTypes() []Type { return nil }

// ReferenceType represents a reference to another type (e.g. TextDocument).
// This is either a Structure, a Enumeration or a TypeAlias in the same meta model.
type ReferenceType struct {
	Name     string
	TypeDecl TypeDecl
}

func (r *ReferenceType) isType()          {}
func (r *ReferenceType) SubTypes() []Type { return nil }

// TupleType represents a tuple type (e.g. [integer, integer]).
type TupleType struct{ Items []Type }

func (*TupleType) isType()            {}
func (t *TupleType) SubTypes() []Type { return t.Items }
