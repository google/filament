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

func TestOptimize(t *testing.T) {
	for _, test := range []struct {
		in  string
		out string
	}{
		// no-ops
		{in: `X`, out: `X`},
		{in: `X && Y`, out: `X && Y`},
		{in: `X && Y && Z`, out: `X && Y && Z`},
		{in: `X || Y || Z`, out: `X || Y || Z`},
		{in: `(X || Y) && (X || Z)`, out: `(X || Y) && (X || Z)`},

		// Sorting
		{in: `Z || X || Y`, out: `X || Y || Z`},
		{in: `!Z || X || Y`, out: `X || Y || (!Z)`},
		{in: `X || !X || Y`, out: `X || (!X) || Y`},
		{in: `Z && X && Y`, out: `X && Y && Z`},
		{in: `Z && !X && Y`, out: `(!X) && Y && Z`},

		// Combine common
		{in: `X || Y || X`, out: `X || Y`},
		{in: `X && Y && X`, out: `X && Y`},
		{in: `X && Y && X`, out: `X && Y`},
		{in: `(X || Y) && (X || Y)`, out: `X || Y`},

		// Complex cases
		{in: `(X || Y) || (Y || Z)`, out: `X || Y || Z`},
		{in: `(X || Y) || (Y && Z)`, out: `(X || Y) && (X || Y || Z)`},
		{in: `(X && Y) && (Y && Z)`, out: `X && Y && Z`},
		{in: `!(X && !(Y || Z)) && Z`, out: `((!X) || Y || Z) && Z`},
		{in: `Z || !(X && !(Y || Z))`, out: `(!X) || Y || Z`},
	} {
		expr, err := cnf.Parse(test.in)
		if err != nil {
			t.Errorf(`unexpected error returned from Parse('%v'): %v`, test.in, err)
			continue
		}
		opt := cnf.Optimize(expr)
		if diff := cmp.Diff(test.out, opt.String()); diff != "" {
			t.Errorf("Optimize('%v') returned '%v'. Diff:\n%v", test.in, opt, diff)
		}
	}
}
