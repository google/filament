// Copyright 2020 Google LLC
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

package match_test

import (
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/match"
)

func TestMatch(t *testing.T) {
	for _, test := range []struct {
		pattern string
		path    string
		expect  bool
	}{
		{"a", "a", true},
		{"b", "a", false},

		{"?", "a", true},
		{"a/?/c", "a/x/c", true},
		{"a/??/c", "a/x/c", false},
		{"a/??/c", "a/xx/c", true},
		{"a/???/c", "a/x z/c", true},
		{"a/?/c", "a/xx/c", false},
		{"a/?/?/c", "a/x/y/c", true},
		{"a/?/?/?/c", "a/x/y/z/c", true},
		{"a/???/c", "a/x/y/c", false},
		{"a/?????", "a/x/y/c", false},

		{"*", "a", true},
		{"*", "abc", true},
		{"*", "abc 123", true},
		{"*", "xxx/yyy", false},
		{"*/*", "xxx/yyy", true},
		{"*/*", "xxx/yyy/zzz", false},
		{"*/*/c", "xxx/yyy/c", true},
		{"a/*/*", "a/xxx/yyy", true},
		{"a/*/c", "a/xxx/c", true},
		{"a/*/c", "a/xxx/c", true},
		{"a/*/*/c", "a/b/c", false},

		{"**", "a", true},
		{"**", "abc", true},
		{"**", "abc 123", true},
		{"**", "xxx/yyy", true},
		{"**", "xxx/yyy/zzz", true},
		{"**/**", "xxx", false},
		{"**/**", "xxx/yyy", true},
		{"**/**", "xxx/yyy/zzz", true},
		{"**/**/**", "xxx/yyy/zzz", true},
		{"**/**/c", "xxx/yyy/c", true},
		{"**/**/c", "xxx/yyy/c/d", false},
		{"a/**/**", "a/xxx/yyy", true},
		{"a/**/c", "a/xxx/c", true},
		{"a/**/c", "a/xxx/yyy/c", true},
		{"a/**/c", "a/xxx/y y/zzz/c", true},

		{"a/**/c", "a/c", false},
		{"a/**c", "a/c", true},

		{"xxx/**.foo", "xxx/aaa.foo", true},
		{"xxx/**.foo", "xxx/yyy/zzz/.foo", true},
		{"xxx/**.foo", "xxx/yyy/zzz/bar.foo", true},
	} {
		f, err := match.New(test.pattern)
		if err != nil {
			t.Errorf(`match.New("%v")`, test.pattern)
			continue
		}
		matched := f(test.path)
		switch {
		case matched && !test.expect:
			t.Errorf(`Path "%v" matched against pattern "%v"`, test.path, test.pattern)
		case !matched && test.expect:
			t.Errorf(`Path "%v" did not match against pattern "%v"`, test.path, test.pattern)
		}
	}
}

func TestErrOnPlaceholder(t *testing.T) {
	for _, pattern := range []string{"a/b••c", "a/b•c", "a/b/¿c"} {
		_, err := match.New(pattern)
		if err == nil {
			t.Errorf(`match.New("%v") did not return an expected error`, pattern)
			continue
		}
		if !strings.Contains(err.Error(), "Pattern must not contain") {
			t.Errorf(`match.New("%v") returned unrecognised error: %v`, pattern, err)
			continue
		}
	}
}
