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

package json

import (
	"encoding/json"
	"fmt"
)

// AndType represents an and type (e.g. TextDocumentParams & WorkDoneProgressParams).
type AndType struct{ Items []Type }

func (*AndType) isType() {}

// ArrayType represents an array type (e.g. TextDocument[]).
type ArrayType struct{ Element Type }

func (*ArrayType) isType() {}

// BaseType represents a base type like string or DocumentUri.
type BaseType struct{ Name BaseTypes }

func (*BaseType) isType() {}

type BaseTypes string

const (
	URI         BaseTypes = "URI"
	DocumentUri BaseTypes = "DocumentUri"
	Integer     BaseTypes = "integer"
	Uinteger    BaseTypes = "uinteger"
	Decimal     BaseTypes = "decimal"
	RegExp      BaseTypes = "RegExp"
	String      BaseTypes = "string"
	Boolean     BaseTypes = "boolean"
	Null        BaseTypes = "null"
)

// BooleanLiteralType represents a boolean literal type (e.g. kind: true).
// kind: booleanLiteral
type BooleanLiteralType struct{ Value bool }

func (*BooleanLiteralType) isType() {}

// Enumeration defines an enumeration.
type Enumeration struct {
	// Whether the enumeration is deprecated or not. If deprecated the property contains the deprecation message.
	Deprecated string
	// An optional documentation
	Documentation string
	// The name of the enumeration
	Name string
	// Whether this is a proposed enumeration. If omitted, the enumeration is final.
	Proposed bool
	// Since when (release number) this enumeration is available. Is empty if not known.
	Since string
	// Whether the enumeration supports custom values (e.g. values which are not part of the set defined in values). If omitted no custom values are supported.
	SupportsCustomValues bool
	// The type of the elements
	Type Type
	// The enum values
	Values []EnumerationEntry
}

// EnumerationEntry defines an enumeration entry
type EnumerationEntry struct {
	// Whether the enum entry is deprecated or not. If deprecated the property contains the deprecation message.
	Deprecated string
	// An optional documentation.
	Documentation string
	// The name of the enum item.
	Name string
	// Whether this is a proposed enumeration entry. If omitted, the enumeration entry is final.
	Proposed bool
	// Since when (release number) this enumeration entry is available. Is undefined if not known.
	Since string
	// The value (string or number)
	Value any
}

// IntegerLiteralType represents an integer literal type (e.g. kind: 1).
type IntegerLiteralType struct{ Value bool }

func (*IntegerLiteralType) isType() {}

// MapKeyType represents a type that can be used as a key in a map type.
// If a reference type is used then the type must either resolve to a string or integer type.
// (e.g. type ChangeAnnotationIdentifier === string).
type MapKeyType interface{ isMapKeyType() }

type MapKeyTypeBase string

func (MapKeyTypeBase) isMapKeyType() {}

const (
	MapKeyType_URI         MapKeyTypeBase = "URI"
	MapKeyType_DocumentUri MapKeyTypeBase = "DocumentUri"
	MapKeyType_String      MapKeyTypeBase = "string"
	MapKeyType_Integer     MapKeyTypeBase = "integer"
)

// MapType represents a JSON object map (e.g. interface Map<K extends string | integer, V> { [key: K] => V; }).
type MapType struct {
	Key   Type
	Value Type
}

func (*MapType) isType() {}

// MessageDirection indicates in which direction a message is sent in the protocol
type MessageDirection string

const (
	MessageDirectionClientToServer MessageDirection = "clientToServer"
	MessageDirectionServerToClient MessageDirection = "serverToClient"
	MessageDirectionBoth           MessageDirection = "both"
)

type MetaData struct {
	// The protocol version
	Version string
}

// MetaModel represents the actual meta model
type MetaModel struct {
	// The enumerations
	Enumerations []Enumeration
	// Additional meta data
	MetaData MetaData
	// The notifications
	Notifications []Notification
	// The requests
	Requests []Request
	// The structures
	Structures []Structure
	// The type aliases
	TypeAliases []TypeAlias
}

// Notification represents a LSP notification
type Notification struct {
	// Whether the notification is deprecated or not.
	// If deprecated the property contains the deprecation message.
	Deprecated string
	// An optional documentation
	Documentation string
	// The direction in which this notification is sent in the protocol
	MessageDirection MessageDirection
	// The request's method name
	Method string
	// The parameter type(s) if any
	Params Types
	// Whether this is a proposed notification. If omitted the notification is final
	Proposed bool
	// Optional a dynamic registration method if it different from the request's method
	RegistrationMethod string
	// Optional registration options if the notification supports dynamic registration
	RegistrationOptions Type
	// Since when (release number) this notification is available. Is undefined if not known
	Since string
}

// OrType represents an or type (e.g. Location | LocationLink)
type OrType struct{ Items []Type }

func (*OrType) isType() {}

// Property represents an object property
type Property struct {
	// Whether the property is deprecated or not. If deprecated the property contains the deprecation message
	Deprecated string
	// An optional documentation
	Documentation string
	// The property name
	Name string
	// Whether the property is optional. If omitted, the property is mandatory
	Optional bool
	// Whether this is a proposed property. If omitted, the structure is final
	Proposed bool
	// Since when (release number) this property is available. Is undefined if not known
	Since string
	// The type of the property
	Type Type
}

// ReferenceType represents a reference to another type (e.g. TextDocument).
// This is either a Structure, a Enumeration or a TypeAlias in the same meta model.
type ReferenceType struct{ Name string }

func (*ReferenceType) isType()      {}
func (ReferenceType) isMapKeyType() {}

// Request represents a LSP request
type Request struct {
	// Whether the request is deprecated or not. If deprecated the property contains the deprecation message
	Deprecated string
	// An optional documentation
	Documentation string
	// An optional error data type
	ErrorData Type
	// The direction in which this request is sent in the protocol
	MessageDirection MessageDirection
	// The request's method name
	Method string
	// The parameter type(s) if any
	Params Types
	// Optional partial result type if the request supports partial result reporting
	PartialResult Type
	// Whether this is a proposed feature. If omitted the feature is final
	Proposed bool
	// Optional a dynamic registration method if it different from the request's method
	RegistrationMethod string
	// Optional registration options if the request supports dynamic registration
	RegistrationOptions Type
	// The result type
	Result Type
	// Since when (release number) this request is available. Is undefined if not known
	Since string
}

// StringLiteralType represents a string literal type (e.g. kind: 'rename')
type StringLiteralType struct{ Value string }

func (*StringLiteralType) isType() {}

// Structure defines the structure of an object literal
type Structure struct {
	// Whether the structure is deprecated or not. If deprecated the property contains the deprecation message
	Deprecated string
	// An optional documentation
	Documentation string
	// Structures extended from. This structures form a polymorphic type hierarchy
	Extends []Type
	// Structures to mix in. The properties of these structures are `copied` into this structure. Mixins don't form a polymorphic type hierarchy in LSP
	Mixins []Type
	// The name of the structure
	Name string
	// The properties
	Properties []Property
	// Whether this is a proposed structure. If omitted, the structure is final
	Proposed bool
	// Since when (release number) this structure is available. Is undefined if not known
	Since string
}

// StructureLiteral defines an unnamed structure of an object literal
type StructureLiteral struct {
	// Whether the literal is deprecated or not. If deprecated the property contains the deprecation message
	Deprecated string
	// An optional documentation
	Documentation string
	// The properties
	Properties []Property
	// Whether this is a proposed structure. If omitted, the structure is final
	Proposed bool
	// Since when (release number) this structure is available. Is undefined if not known
	Since string
}

// StructureLiteralType represents a literal structure (e.g. property: { start: uinteger; end: uinteger; })
type StructureLiteralType struct{ Value StructureLiteral }

func (*StructureLiteralType) isType() {}

// TupleType represents a tuple type (e.g. [integer, integer]).
type TupleType struct{ Items []Type }

func (*TupleType) isType() {}

type TypeImpl interface{ isType() }

type Type struct {
	Impl TypeImpl
}

func (t *Type) UnmarshalJSON(data []byte) error {
	s := struct{ Kind TypeKind }{}
	if err := json.Unmarshal(data, &s); err != nil {
		return err
	}
	switch s.Kind {
	case TypeKindBase:
		t.Impl = &BaseType{}
	case TypeKindReference:
		t.Impl = &ReferenceType{}
	case TypeKindArray:
		t.Impl = &ArrayType{}
	case TypeKindMap:
		t.Impl = &MapType{}
	case TypeKindAnd:
		t.Impl = &AndType{}
	case TypeKindOr:
		t.Impl = &OrType{}
	case TypeKindTuple:
		t.Impl = &TupleType{}
	case TypeKindLiteral:
		t.Impl = &StructureLiteralType{}
	case TypeKindStringLiteral:
		t.Impl = &StringLiteralType{}
	case TypeKindIntegerLiteral:
		t.Impl = &IntegerLiteralType{}
	case TypeKindBooleanLiteral:
		t.Impl = &BooleanLiteralType{}
	default:
		return fmt.Errorf("unhandled Type kind '%v'", s.Kind)
	}
	return json.Unmarshal(data, t.Impl)
}

type Types struct {
	Impl []TypeImpl
}

func (t *Types) UnmarshalJSON(data []byte) error {
	single := Type{}
	if err := json.Unmarshal(data, &single); err == nil {
		t.Impl = []TypeImpl{single.Impl}
		return nil
	}
	multi := []Type{}
	if err := json.Unmarshal(data, &multi); err != nil {
		return err
	}
	for _, e := range multi {
		t.Impl = append(t.Impl, e.Impl)
	}
	return nil
}

// TypeAlias defines a type alias. (e.g. type Definition = Location | LocationLink)
type TypeAlias struct {
	// Whether the type alias is deprecated or not.
	// If deprecated the property contains the deprecation message
	Deprecated string
	// An optional documentation
	Documentation string
	// The name of the type alias
	Name string
	// Whether this is a proposed type alias. If omitted, the type alias is final
	Proposed bool
	// Since when (release number) this structure is available. Is undefined if not known
	Since string
	// The aliased type
	Type Type
}

type TypeKind string

const (
	TypeKindBase           TypeKind = "base"
	TypeKindReference      TypeKind = "reference"
	TypeKindArray          TypeKind = "array"
	TypeKindMap            TypeKind = "map"
	TypeKindAnd            TypeKind = "and"
	TypeKindOr             TypeKind = "or"
	TypeKindTuple          TypeKind = "tuple"
	TypeKindLiteral        TypeKind = "literal"
	TypeKindStringLiteral  TypeKind = "stringLiteral"
	TypeKindIntegerLiteral TypeKind = "integerLiteral"
	TypeKindBooleanLiteral TypeKind = "booleanLiteral"
)
