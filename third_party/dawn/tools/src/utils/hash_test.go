// Copyright 2022 The Dawn & Tint Authors
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

package utils_test

import (
	"dawn.googlesource.com/dawn/tools/src/utils"
	"testing"
)

func TestHash(t *testing.T) {
	type Test struct {
		a, b        []any
		expectEqual bool
	}
	for _, test := range []Test{
		{a: []any{1}, b: []any{1}, expectEqual: true},
		{a: []any{1}, b: []any{2}, expectEqual: false},
		{a: []any{1}, b: []any{1.0}, expectEqual: false},
		{a: []any{1.0}, b: []any{1.0}, expectEqual: true},
		{a: []any{'x'}, b: []any{'x'}, expectEqual: true},
		{a: []any{'x'}, b: []any{'y'}, expectEqual: false},
		{a: []any{1, 2}, b: []any{1, 2}, expectEqual: true},
		{a: []any{1, 2}, b: []any{1}, expectEqual: false},
		{a: []any{1, 2}, b: []any{1, 3}, expectEqual: false},
	} {
		hashA := utils.Hash(test.a...)
		hashB := utils.Hash(test.b...)
		equal := hashA == hashB
		if equal != test.expectEqual {
			t.Errorf("Hash(%v): %v\nHash(%v): %v", test.a, hashA, test.b, hashB)
		}
	}
}
