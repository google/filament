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

func TestParse(t *testing.T) {
	for _, test := range []struct {
		in     string
		expect cnf.Expr
	}{
		{in: ``, expect: nil},
		{in: `X`, expect: cnf.Expr{{T("X")}}},
		{in: `(X)`, expect: cnf.Expr{{T("X")}}},
		{in: `((X))`, expect: cnf.Expr{{T("X")}}},
		{in: `!X`, expect: cnf.Expr{{F("X")}}},
		{in: `!(X)`, expect: cnf.Expr{{F("X")}}},
		{in: `!!(X)`, expect: cnf.Expr{{T("X")}}},
		{in: `(!(X))`, expect: cnf.Expr{{F("X")}}},
		{in: `!(!(X))`, expect: cnf.Expr{{T("X")}}},
		{in: `X && Y`, expect: cnf.Expr{{T("X")}, {T("Y")}}},
		{in: `X && Y && Z`, expect: cnf.Expr{{T("X")}, {T("Y")}, {T("Z")}}},
		{in: `X && !Y && Z`, expect: cnf.Expr{{T("X")}, {F("Y")}, {T("Z")}}},
		{in: `!X && Y && !Z`, expect: cnf.Expr{{F("X")}, {T("Y")}, {F("Z")}}},
		{in: `X || Y`, expect: cnf.Expr{{T("X"), T("Y")}}},
		{in: `X || Y || Z`, expect: cnf.Expr{{T("X"), T("Y"), T("Z")}}},
		{in: `X || !Y || Z`, expect: cnf.Expr{{T("X"), F("Y"), T("Z")}}},
		{in: `!X || Y || !Z`, expect: cnf.Expr{{F("X"), T("Y"), F("Z")}}},
		{in: `(X || Y) && Z`, expect: cnf.Expr{{T("X"), T("Y")}, {T("Z")}}},
		{in: `(  X || Y  ) && Z`, expect: cnf.Expr{{T("X"), T("Y")}, {T("Z")}}},
		{in: `X || (Y && Z)`, expect: cnf.Expr{{T("X"), T("Y")}, {T("X"), T("Z")}}},
		{in: `(X && Y) || Z`, expect: cnf.Expr{{T("X"), T("Z")}, {T("Y"), T("Z")}}},
		{in: `X && (Y || Z)`, expect: cnf.Expr{{T("X")}, {T("Y"), T("Z")}}},
		{in: `(!X && Y) || Z`, expect: cnf.Expr{{F("X"), T("Z")}, {T("Y"), T("Z")}}},
		{in: `(X && !Y) || Z`, expect: cnf.Expr{{T("X"), T("Z")}, {F("Y"), T("Z")}}},
		{in: `(X && Y) || !Z`, expect: cnf.Expr{{T("X"), F("Z")}, {T("Y"), F("Z")}}},
		{in: `!X && (Y || Z)`, expect: cnf.Expr{{F("X")}, {T("Y"), T("Z")}}},
		{in: `!(!X && (Y || Z))`, expect: cnf.Expr{{T("X"), F("Y")}, {T("X"), F("Z")}}},
		{in: `X && (!Y || Z)`, expect: cnf.Expr{{T("X")}, {F("Y"), T("Z")}}},
		{in: `!(X && (!Y || Z))`, expect: cnf.Expr{{F("X"), T("Y")}, {F("X"), F("Z")}}},
		{in: `X && (Y || !Z)`, expect: cnf.Expr{{T("X")}, {T("Y"), F("Z")}}},
		{in: `X && !(!Y || Z)`, expect: cnf.Expr{{T("X")}, {T("Y")}, {F("Z")}}},
		{in: `!(X && (Y || !Z))`, expect: cnf.Expr{{F("X"), F("Y")}, {F("X"), T("Z")}}},
		{in: `!(X && !(Y || !Z))`, expect: cnf.Expr{{F("X"), T("Y"), F("Z")}}},
	} {
		expr, err := cnf.Parse(test.in)
		if err != nil {
			t.Errorf(`unexpected error returned from Parse('%v'): %v`, test.in, err)
			continue
		}
		if diff := cmp.Diff(test.expect, expr); diff != "" {
			t.Errorf("Parse('%v') returned '%v'. Diff:\n%v", test.in, expr, diff)
		}
	}
}

func TestParseErr(t *testing.T) {
	for _, test := range []struct {
		in     string
		expect string
	}{
		{
			in: `)`,
			expect: `Parse error:

)
^

expected 'ident', got ')'`,
		},
		{
			in: ` )`,
			expect: `Parse error:

 )
 ^

expected 'ident', got ')'`,
		},
		{
			in: `(`,
			expect: `Parse error:

(
 ^

expected 'ident'`,
		},
		{
			in: ` (`,
			expect: `Parse error:

 (
  ^

expected 'ident'`,
		},
		{
			in: `(x`,
			expect: `Parse error:

(x
  ^

expected ')'`,
		},
		{
			in: `((x)`,
			expect: `Parse error:

((x)
    ^

expected ')'`,
		},
		{
			in: `X || Y && Z`,
			expect: `Parse error:

X || Y && Z
       ^^

cannot mix '&&' and '||' without parentheses`,
		},
	} {
		_, err := cnf.Parse(test.in)
		errStr := ""
		if err != nil {
			errStr = err.Error()
		}
		if test.expect != errStr {
			t.Errorf(`unexpected error returned from Parse('%v'): %v`, test.in, err)
			continue
		}
	}
}
