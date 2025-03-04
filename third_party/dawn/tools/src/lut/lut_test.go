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

package lut_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/lut"
)

func TestCompactWithFragments(t *testing.T) {
	lut := lut.New[rune]()
	indices := []*int{
		lut.Add([]rune("the life in your")),
		lut.Add([]rune("in your life that count")),
		lut.Add([]rune("In the end,")),
		lut.Add([]rune("the life in")),
		lut.Add([]rune("count. It's the")),
		lut.Add([]rune("years")),
		lut.Add([]rune("in your years.")),
		lut.Add([]rune("it's not the years in")),
		lut.Add([]rune("not the years")),
		lut.Add([]rune("not the years in your")),
		lut.Add([]rune("end, it's")),
	}

	runes := lut.Compact()

	expect := "In the end, it's not the years in your life that count. It's the life in your years."
	got := string(runes)
	if got != expect {
		t.Errorf("Compact result was not as expected\nExpected: '%v'\nGot:      '%v'", expect, got)
	}
	expectedIndices := []int{
		61, //                                                              the life in your
		31, //                                in your life that count
		0,  // In the end,
		61, //                                                              the life in
		49, //                                                  count. It's the
		25, //                          years
		70, //                                                                       in your years.
		12, //             it's not the years in
		17, //                  not the years
		17, //                  not the years in your
		7,  //        end, it's
	}
	for i, p := range indices {
		if expected, got := expectedIndices[i], *p; expected != got {
			t.Errorf("Index %v was not expected. Expected %v, got %v", i, expected, got)
		}
	}
}
