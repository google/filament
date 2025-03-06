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

// / Decomposed holds a decomposed expression
// / See Decompose()
type Decomposed struct {
	Ands   []Ands  // The decomposed Ands
	Ors    []Ors   // The decomposed Ors
	Unarys []Unary // The decomposed Unarys
}

// Decompose returns the unique Ands, Ors and Unarys that make up the expression.
// If e has two or more OR expressions AND'd together, then Decomposed.Ands will
// hold the deduplicated AND expressions, otherwise Decomposed.Ands will be
// empty.
// If e has two or more Unary expressions OR'd together, then Decomposed.ORs
// will hold the deduplicated OR expressions, otherwise Decomposed.ORs will be
// empty.
// Decomposed.Unarys will hold all the deduplicated Unary expressions.
func Decompose(e Expr) Decomposed {
	ors := container.NewMap[Key, Ors]()
	unarys := container.NewMap[Key, Unary]()
	for _, o := range e {
		for _, u := range o {
			unarys.Add(u.Key(), u)
		}
		if len(o) > 1 {
			ors.Add(o.Key(), o)
		}
	}
	d := Decomposed{}
	if len(e) > 1 {
		d.Ands = []Ands{e}
	}
	if len(ors) > 0 {
		d.Ors = ors.Values()
	}
	if len(unarys) > 0 {
		d.Unarys = unarys.Values()
	}
	return d
}
