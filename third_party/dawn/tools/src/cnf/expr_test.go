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
)

func TestAssumeTrue(t *testing.T) {
	for _, test := range []struct {
		expr   string
		isTrue string
		expect string
	}{
		{expr: ``, isTrue: ``, expect: ``},
		{expr: `a`, isTrue: ``, expect: `a`},
		{expr: `a`, isTrue: `!a`, expect: `a`},
		{expr: `a`, isTrue: `!b`, expect: `a`},
		{expr: `a`, isTrue: `c`, expect: `a`},
		{expr: `a`, isTrue: `c`, expect: `a`},
		{expr: `a || b`, isTrue: ``, expect: `a || b`},
		{expr: `a || b`, isTrue: `!a`, expect: `a || b`},
		{expr: `a || b`, isTrue: `!b`, expect: `a || b`},
		{expr: `a || b`, isTrue: `c`, expect: `a || b`},
		{expr: `a || b`, isTrue: `c`, expect: `a || b`},
		{expr: `a && b`, isTrue: ``, expect: `a && b`},
		{expr: `a && b`, isTrue: `!a`, expect: `a && b`},
		{expr: `a && b`, isTrue: `!b`, expect: `a && b`},
		{expr: `a && b`, isTrue: `c`, expect: `a && b`},
		{expr: `a && b`, isTrue: `c`, expect: `a && b`},

		{expr: `a`, isTrue: `a`, expect: ``},
		{expr: `a || b`, isTrue: `a`, expect: ``},
		{expr: `a || b`, isTrue: `b`, expect: ``},
		{expr: `a && b`, isTrue: `a`, expect: `b`},
		{expr: `a && b`, isTrue: `b`, expect: `a`},

		{expr: `a`, isTrue: `a && b`, expect: ``},
		{expr: `a || b`, isTrue: `a && b`, expect: ``},
		{expr: `a || b`, isTrue: `b && b`, expect: ``},
		{expr: `a && b`, isTrue: `a && b`, expect: ``},

		{expr: `a && c`, isTrue: `a && b`, expect: `c`},
		{expr: `(a || b) && c`, isTrue: `a && b`, expect: `c`},
		{expr: `(a || b) && c`, isTrue: `b && c`, expect: ``},
		{expr: `a && b && c`, isTrue: `a && b`, expect: `c`},
	} {
		expr, err := cnf.Parse(test.expr)
		if err != nil {
			t.Errorf(`unexpected error returned from Parse('%v'): %v`, test.expr, err)
			continue
		}
		isTrue, err := cnf.Parse(test.isTrue)
		if err != nil {
			t.Errorf(`unexpected error returned from Parse('%v'): %v`, test.isTrue, err)
			continue
		}
		got := expr.AssumeTrue(isTrue).String()
		if test.expect != got {
			t.Errorf("('%v').AssumeTrue('%v') returned '%v', expected '%v'", expr, isTrue, got, test.expect)
		}
	}
}
