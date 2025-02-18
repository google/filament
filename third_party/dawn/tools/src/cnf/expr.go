// Copyright 2023 The Dawn & Tint Authors
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

package cnf

import (
	"dawn.googlesource.com/dawn/tools/src/container"
)

// Expr is a boolean expression, expressed in a Conjunctive Normal Form.
// Expr is an alias to Ands, which represent all the OR expressions that are
// AND'd together.
type Expr = Ands

// Ands is a slice of Ors
// Ands holds a sequence of OR'd expressions that are AND'd together:
//
//	Ors[0] && Ors[1] && ... && Ors[n-1]
type Ands []Ors

// Ors is a slice of Unary
// Ors holds a sequence of unary expressions that are OR'd together:
//
//	Unary[0] || Unary[1] || ... || Unary[n-1]
type Ors []Unary

// Unary is a negatable variable
type Unary struct {
	// If true, Var is negated
	Negate bool
	// The name of the variable
	Var string
}

// AssumeTrue returns a new simplified expression assuming o is true
func (e Expr) AssumeTrue(o Expr) Expr {
	isTrue := container.NewSet[Key]()
	for _, ors := range o {
		isTrue.Add(ors.Key())
	}

	out := Expr{}
nextAnd:
	for _, ors := range e {
		for _, unary := range ors {
			if isTrue.Contains(unary.Key()) {
				continue nextAnd // At least one of the OR expressions is true
			}
		}
		if isTrue.Contains(ors.Key()) {
			continue // Whole OR expression is true
		}
		out = append(out, ors)
	}
	return out
}
