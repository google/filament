// Copyright 2021 The Dawn & Tint Authors
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

package resolver_test

import (
	"fmt"
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/parser"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/resolver"
)

func TestResolver(t *testing.T) {
	type test struct {
		src string
		err string
	}

	success := ""
	for _, test := range []test{
		{
			`type X`,
			success,
		}, {
			`enum E {}`,
			success,
		}, {
			`enum E {A B C}`,
			success,
		}, {
			`type X`,
			success,
		}, {
			`@display("Y") type X`,
			success,
		}, {
			`
type x
match y: x`,
			success,
		}, {
			`
enum e {a b c}
match y: e.c | e.a | e.b`,
			success,
		}, {
			`fn f()`,
			success,
		}, {
			`implicit(T) fn f()`,
			success,
		}, {
			`
type f32
fn f<T>(T) -> f32`,
			success,
		}, {
			`
type f32
implicit(N: num) fn f()`,
			success,
		}, {
			`
enum e { a b c }
implicit(N: e) fn f()`,
			success,
		}, {
			`
type f32
implicit(T) fn f(T) -> f32`,
			success,
		}, {
			`
type f32
implicit(N: num) fn f<T: f32>()`,
			success,
		}, {
			`
type f32
implicit(T: f32) fn f(T: f32) -> f32`,
			success,
		}, {
			`
type f32
type P<T>
match m: f32
implicit(T: m) fn f(P<T>) -> T`,
			success,
		}, {
			`
enum e { a }
match m: e.a
fn f(m)`,
			success,
		}, {
			`
enum e { a b }
type T<E: e>
match m: e.a
fn f(T<m>)`,
			success,
		}, {
			`
enum e { a }
type T<E: e>
match m : e.a
fn f(T<m>)`,
			success,
		}, {
			`
enum e { a }
type T<E: e>
match a : e.a
fn f(T<a>)`,
			success,
		}, {
			`
type T<E: num>
implicit(E: num) fn f(T<E>)`,
			success,
		}, {
			`implicit(T) fn f(T)`,
			success,
		}, {
			`
enum e { a b }
implicit(E: e) fn f()`,
			success,
		}, {
			`
enum e { a b }
match m: e.a | e.b
implicit(E: m) fn f()`,
			success,
		}, {
			`
type f32
type T<x>
fn f(T< T<f32> >)`,
			success,
		}, {
			`
type a
type b
type c
match S: a | b | c
type V<N: num, T>
implicit(N: num, T: S, U: S) fn f<I: V<N, T> >(V<N, U>) -> I`,
			success,
		}, {
			`
type f32
op -(f32)`,
			success,
		}, {
			`
type f32
type T<x>
op +(T<f32>, T<f32>)`,
			success,
		}, {
			`
type f32
ctor f32(f32)`,
			success,
		}, {
			`
type f32
type T<x>
ctor f32(T<f32>)`,
			success,
		}, {
			`
type f32
type i32
conv f32(i32)`,
			success,
		}, {
			`
type f32
type T<x>
conv f32(T<f32>)`,
			success,
		}, {
			`
type f32
@must_use fn f() -> f32`,
			success,
		}, {
			`
type f32
@member_function fn f(f32)`,
			success,
		}, {
			`
type f32
type P<T>
match m: f32
fn f(m)`,
			success,
		}, {
			`
type f32
type P<T>
match m: f32
fn f(P<m>)`,
			success,
		}, {
			`enum E {A A}`,
			`
file.txt:1:11 duplicate enum entry 'A'
`,
		},
		{
			`type X type X`,
			`
file.txt:1:13 'X' already declared
First declared here: file.txt:1:6`,
		}, {
			`@meow type X`,
			`
file.txt:1:2 unknown attribute
`,
		}, {
			`@display("Y", "Z") type X`,
			`
file.txt:1:2 expected a single value for 'display' attribute`,
		}, {
			`
enum e { a }
enum e { b }`,
			`
file.txt:2:6 'e' already declared
First declared here: file.txt:1:6`,
		}, {
			`
type X
match X : X`,
			`
file.txt:2:7 'X' already declared
First declared here: file.txt:1:6`,
		}, {
			`type T<X>
match M : T`,
			`file.txt:2:11 'T' requires 1 template arguments, but 0 were provided`,
		}, {
			`
match x: y`,
			`
file.txt:1:10 cannot resolve 'y'
`,
		}, {
			`
type a
match x: a | b`,
			`
file.txt:2:14 cannot resolve 'b'
`,
		}, {
			`
type a
type b
match x: a | b | a`,
			`
file.txt:3:18 duplicate option 'a' in matcher
First declared here: file.txt:3:10
`,
		}, {
			`
enum e { a c }
match x: e.a | e.b | e.c`,
			`
file.txt:2:18 enum 'e' does not contain 'b'
`,
		}, {
			`
enum e { a }
match x: e.a
match x: e.a`,
			`
file.txt:3:7 'x' already declared
First declared here: file.txt:2:7
`,
		}, {
			`
type t
match x: t
match y: x`,
			`
file.txt:3:10 'x' resolves to type matcher 'x' but type is expected
`,
		}, {
			`fn f(u)`,
			`file.txt:1:6 cannot resolve 'u'`,
		}, {
			`fn f() -> u`,
			`file.txt:1:11 cannot resolve 'u'`,
		}, {
			`implicit(T: u) fn f()`,
			`file.txt:1:13 cannot resolve 'u'`,
		}, {
			`
enum e { a }
fn f() -> e`,
			`file.txt:2:11 cannot use 'e' as return type. Must be a type or template type`,
		}, {
			`
type T<x>
fn f(T<u>)`,
			`file.txt:2:8 cannot resolve 'u'`,
		}, {
			`
type x
implicit(T) fn f(T<x>)`,
			`file.txt:2:18 'T' template parameters do not accept template arguments`,
		}, {
			`
type A<N: num>
type B
fn f(A<B>)`,
			`file.txt:3:8 cannot use type 'B' as template number`,
		}, {
			`
type T
type P<N: num>
match m: T
fn f(P<m>)`,
			`file.txt:4:8 cannot use type matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { b }
fn f(P<E>)`,
			`file.txt:3:8 cannot use enum 'E' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
fn f(P<m>)`,
			`file.txt:4:8 cannot use enum matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
implicit(M: m) fn f(P<M>)`,
			`file.txt:4:23 cannot use template enum 'E' as template number`,
		}, {
			`
type i
enum e { a }
op << (i) -> e`,
			`file.txt:3:14 cannot use 'e' as return type. Must be a type or template type`,
		}, {
			`
type T<x>
op << (T<u>)`,
			`file.txt:2:10 cannot resolve 'u'`,
		}, {
			`
op << ()`,
			`file.txt:1:4 operators must have either 1 or 2 parameters`,
		}, {
			`
type i
op << (i, i, i)`,
			`file.txt:2:4 operators must have either 1 or 2 parameters`,
		}, {
			`
type x
implicit(T) op << (T<x>)`,
			`file.txt:2:20 'T' template parameters do not accept template arguments`,
		}, {
			`
type A<N: num>
type B
op << (A<B>)`,
			`file.txt:3:10 cannot use type 'B' as template number`,
		}, {
			`
type A<N>
enum E { b }
match M: E.b
op << (A<M>)`,
			`file.txt:4:10 cannot use enum matcher 'M' as template type`,
		}, {
			`
type T
type P<N: num>
match m: T
op << (P<m>)`,
			`file.txt:4:10 cannot use type matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { b }
op << (P<E>)`,
			`file.txt:3:10 cannot use enum 'E' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
op << (P<m>)`,
			`file.txt:4:10 cannot use enum matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
implicit(M: m) op << (P<M>)`,
			`file.txt:4:25 cannot use template enum 'E' as template number`,
		}, {
			`
type i
enum e { a }
ctor F(i) -> e`,
			`file.txt:3:14 cannot use 'e' as return type. Must be a type or template type`,
		}, {
			`
type T<x>
ctor F(T<u>)`,
			`file.txt:2:10 cannot resolve 'u'`,
		}, {
			`
type x
implicit(T) ctor F(T<x>)`,
			`file.txt:2:20 'T' template parameters do not accept template arguments`,
		}, {
			`
type A<N: num>
type B
ctor F(A<B>)`,
			`file.txt:3:10 cannot use type 'B' as template number`,
		}, {
			`
type A<N>
enum E { b }
match M: E.b
ctor F(A<M>)`,
			`file.txt:4:10 cannot use enum matcher 'M' as template type`,
		}, {
			`
type T
type P<N: num>
match m: T
ctor F(P<m>)`,
			`file.txt:4:10 cannot use type matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { b }
ctor F(P<E>)`,
			`file.txt:3:10 cannot use enum 'E' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
ctor F(P<m>)`,
			`file.txt:4:10 cannot use enum matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
implicit(M: m) ctor F(P<M>)`,
			`file.txt:4:25 cannot use template enum 'E' as template number`,
		}, {
			`
conv F()`,
			`file.txt:1:6 conversions must have a single parameter`,
		}, {
			`
type i
conv F(i, i, i)`,
			`file.txt:2:6 conversions must have a single parameter`,
		}, {
			`
type i
enum e { a }
conv F(i) -> e`,
			`file.txt:3:14 cannot use 'e' as return type. Must be a type or template type`,
		}, {
			`
type T<x>
conv F(T<u>)`,
			`file.txt:2:10 cannot resolve 'u'`,
		}, {
			`
type x
implicit(T) conv F(T<x>)`,
			`file.txt:2:20 'T' template parameters do not accept template arguments`,
		}, {
			`
type A<N: num>
type B
conv F(A<B>)`,
			`file.txt:3:10 cannot use type 'B' as template number`,
		}, {
			`
type A<N>
enum E { b }
match M: E.b
conv F(A<M>)`,
			`file.txt:4:10 cannot use enum matcher 'M' as template type`,
		}, {
			`
type T
type P<N: num>
match m: T
conv F(P<m>)`,
			`file.txt:4:10 cannot use type matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { b }
conv F(P<E>)`,
			`file.txt:3:10 cannot use enum 'E' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
conv F(P<m>)`,
			`file.txt:4:10 cannot use enum matcher 'm' as template number`,
		}, {
			`
type P<N: num>
enum E { a b }
match m: E.a | E.b
implicit(M: m) conv F(P<M>)`,
			`file.txt:4:25 cannot use template enum 'E' as template number`,
		}, {
			`
@must_use fn f()`,
			`file.txt:1:2 @must_use can only be used on a function with a return type`,
		}, {
			`
type f32
@member_function(0) fn f(f32)`,
			`file.txt:2:2 unexpected value for member_function attribute`,
		}, {
			`
@member_function fn f()`,
			`file.txt:1:2 @member_function can only be used on a function with at least one parameter`,
		}, {
			`
implicit(T) fn f<T>()`,
			`file.txt:1:18 'T' already declared
First declared here: file.txt:1:10`,
		},
	} {

		ast, err := parser.Parse(strings.TrimSpace(string(test.src)), "file.txt")
		if err != nil {
			t.Errorf("While parsing:\n%s\nUnexpected parser error: %v", test.src, err)
			continue
		}

		expectErr := strings.TrimSpace(test.err)
		_, err = resolver.Resolve(ast)
		if err != nil {
			gotErr := strings.TrimSpace(fmt.Sprint(err))
			if gotErr != expectErr {
				t.Errorf("While parsing:\n%s\nGot error:\n%s\nExpected:\n%s", test.src, gotErr, expectErr)
			}
		} else if expectErr != success {
			t.Errorf("While parsing:\n%s\nGot no error, expected error:\n%s", test.src, expectErr)
		}
	}
}
