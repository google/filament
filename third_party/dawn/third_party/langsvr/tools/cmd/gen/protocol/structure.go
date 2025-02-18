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
	Properties []*Property
	// Whether this is a proposed structure. If omitted, the structure is final
	Proposed bool
	// Since when (release number) this structure is available. Is undefined if not known
	Since string
	// The 'kind' property, used to identify the structure type
	Kind string
	// Child structures of this structure
	NestedStructures []*Structure
	// The list of structure names (outermost first) to get to this structure
	NestedNames []string
}

func (*Structure) isTypeDecl() {}
