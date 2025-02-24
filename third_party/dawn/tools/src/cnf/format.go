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
	"strings"

	"dawn.googlesource.com/dawn/tools/src/transform"
)

// String returns the expression in a C-like syntax
func (e Expr) String() string {
	return e.Format(" && ", " || ", "!")
}

// String returns the expression in a C-like syntax
func (o Ors) String() string {
	return o.Format(" || ", "!")
}

// String returns the expression in a C-like syntax
func (u Unary) String() string {
	return u.Format("!")
}

// Format returns the expression using the provided operators
func (e Expr) Format(and, or, not string) string {
	parts, _ := transform.Slice(e, func(ors Ors) (string, error) {
		if len(e) > 1 && len(ors) > 1 {
			return "(" + ors.Format(or, not) + ")", nil
		}
		return ors.Format(or, not), nil
	})
	return strings.Join(parts, and)
}

// Format returns the expression using the provided operators
func (o Ors) Format(or, not string) string {
	parts, _ := transform.Slice(o, func(u Unary) (string, error) {
		return u.Format(not), nil
	})
	return strings.Join(parts, or)
}

// Format returns the expression using the provided operator
func (u Unary) Format(not string) string {
	if u.Negate {
		return "(" + not + u.Var + ")"
	}
	return u.Var
}
