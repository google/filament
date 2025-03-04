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
	"reflect"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/transform"
)

func check(got, expect any) error {
	if !reflect.DeepEqual(got, expect) {
		return fmt.Errorf(`
got:    %v
expect: %v
`, got, expect)
	}
	return nil
}

func TestFilter(t *testing.T) {
	in := []int{5, 8, 2, 4}
	out := transform.Filter(in, func(i int) bool {
		return i != 8
	})
	if e := check(out, []int{5, 2, 4}); e != nil {
		t.Error(e)
	}
}

func TestFlatten(t *testing.T) {
	in := [][]int{{5, 8}, {2}, {4, 6, 5}}
	out := transform.Flatten(in)
	if e := check(out, []int{5, 8, 2, 4, 6, 5}); e != nil {
		t.Error(e)
	}
}

func TestSlice(t *testing.T) {
	in := []int{5, 8, 2, 4}
	out, err := transform.Slice(in, func(i int) (int, error) {
		return i + 1, nil
	})
	if e := check(out, []int{6, 9, 3, 5}); e != nil {
		t.Error(e)
	}
	if e := check(err, nil); e != nil {
		t.Error(e)
	}
}

func TestSliceErr(t *testing.T) {
	in := []int{5, 8, 2, 4}
	out, err := transform.Slice(in, func(i int) (int, error) {
		return 0, fmt.Errorf("E(%v)", i)
	})
	if e := check(out, ([]int)(nil)); e != nil {
		t.Error(e)
	}
	if e := check(err.Error(), "E(5)"); e != nil {
		t.Error(e)
	}
}

func TestGoSlice(t *testing.T) {
	in := []int{5, 8, 2, 4}
	out, err := transform.GoSlice(in, func(i int) (int, error) {
		return i + 1, nil
	})
	if e := check(out, []int{6, 9, 3, 5}); e != nil {
		t.Error(e)
	}
	if e := check(err, nil); e != nil {
		t.Error(e)
	}
}

func TestGoSliceErr(t *testing.T) {
	in := []int{5, 8, 2, 4}
	out, err := transform.GoSlice(in, func(i int) (int, error) {
		return 0, fmt.Errorf("E(%v)", i)
	})
	if e := check(out, ([]int)(nil)); e != nil {
		t.Error(e)
	}
	if e := check(err.Error(), `E(2)
E(4)
E(5)
E(8)`); e != nil {
		t.Error(e)
	}
}

func TestSliceToChan(t *testing.T) {
	in := []int{5, 8, 2, 4}
	c := transform.SliceToChan(in)
	out := []int{}
	for i := range c {
		out = append(out, i)
	}
	if e := check(in, out); e != nil {
		t.Error(e)
	}
}
