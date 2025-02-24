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

package cnf_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cnf"
	"github.com/google/go-cmp/cmp"
)

func TestDecompose(t *testing.T) {

	for _, test := range []struct {
		in     string
		expect cnf.Decomposed
	}{
		{
			in:     ``,
			expect: cnf.Decomposed{},
		},
		{
			in: `X`,
			expect: cnf.Decomposed{
				Unarys: []cnf.Unary{T("X")},
			},
		},
		{
			in: `X || Y`,
			expect: cnf.Decomposed{
				Ors:    []cnf.Ors{{T("X"), T("Y")}},
				Unarys: []cnf.Unary{T("X"), T("Y")},
			},
		},
		{
			in: `!X || Y`,
			expect: cnf.Decomposed{
				Ors:    []cnf.Ors{{F("X"), T("Y")}},
				Unarys: []cnf.Unary{F("X"), T("Y")},
			},
		},
		{
			in: `X || !Y`,
			expect: cnf.Decomposed{
				Ors:    []cnf.Ors{{T("X"), F("Y")}},
				Unarys: []cnf.Unary{T("X"), F("Y")},
			},
		},
		{
			in: `X || Y || Z`,
			expect: cnf.Decomposed{
				Ors:    []cnf.Ors{{T("X"), T("Y"), T("Z")}},
				Unarys: []cnf.Unary{T("X"), T("Y"), T("Z")},
			},
		},
		{
			in: `(X || Y) && Z`,
			expect: cnf.Decomposed{
				Ands:   []cnf.Ands{{{T("X"), T("Y")}, {T("Z")}}},
				Ors:    []cnf.Ors{{T("X"), T("Y")}},
				Unarys: []cnf.Unary{T("X"), T("Y"), T("Z")},
			},
		},
		{
			in: `(X || Y) && (X || Y)`,
			expect: cnf.Decomposed{
				Ands:   []cnf.Ands{{{T("X"), T("Y")}, {T("X"), T("Y")}}},
				Ors:    []cnf.Ors{{T("X"), T("Y")}},
				Unarys: []cnf.Unary{T("X"), T("Y")},
			},
		},
	} {
		expr, err := cnf.Parse(test.in)
		if err != nil {
			t.Errorf(`unexpected error returned from Parse('%v'): %v`, test.in, err)
			continue
		}
		got := cnf.Decompose(expr)
		if diff := cmp.Diff(test.expect, got); diff != "" {
			t.Errorf("Decompose('%v') returned '%v'. Diff:\n%v", test.in, got, diff)
		}
	}
}
