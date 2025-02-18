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

package transform_test

import (
	"fmt"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/transform"
)

func TestMap(t *testing.T) {
	in := map[string]int{"five": 5, "eight": 8, "two": 2, "four": 4}
	out, err := transform.Map(in, func(k string, v int) (string, int, error) {
		return k + "+1", v + 1, nil
	})
	if e := check(out, map[string]int{"five+1": 6, "eight+1": 9, "two+1": 3, "four+1": 5}); e != nil {
		t.Error(e)
	}
	if e := check(err, nil); e != nil {
		t.Error(e)
	}
}

func TestMapErr(t *testing.T) {
	in := map[string]int{"five": 5, "eight": 8, "two": 2, "four": 4}
	out, err := transform.Map(in, func(k string, v int) (string, int, error) {
		return "", 0, fmt.Errorf("%v:%v", k, v)
	})
	if e := check(out, map[string]int(nil)); e != nil {
		t.Error(e)
	}
	if e := check(err.Error(), `eight:8
five:5
four:4
two:2`); e != nil {
		t.Error(e)
	}
}

func TestGoMap(t *testing.T) {
	in := map[string]int{"five": 5, "eight": 8, "two": 2, "four": 4}
	out, err := transform.GoMap(in, func(k string, v int) (string, int, error) {
		return k + "+1", v + 1, nil
	})
	if e := check(out, map[string]int{"five+1": 6, "eight+1": 9, "two+1": 3, "four+1": 5}); e != nil {
		t.Error(e)
	}
	if e := check(err, nil); e != nil {
		t.Error(e)
	}
}

func TestGoMapErr(t *testing.T) {
	in := map[string]int{"five": 5, "eight": 8, "two": 2, "four": 4}
	out, err := transform.GoMap(in, func(k string, v int) (string, int, error) {
		return "", 0, fmt.Errorf("%v:%v", k, v)
	})
	if e := check(out, map[string]int(nil)); e != nil {
		t.Error(e)
	}
	if e := check(err.Error(), `eight:8
five:5
four:4
two:2`); e != nil {
		t.Error(e)
	}
}
