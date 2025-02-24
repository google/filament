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

package expectations_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
	"github.com/google/go-cmp/cmp"
)

func TestRemoveLowerPriorityTags(t *testing.T) {
	tags := expectations.Tags{
		Sets: []expectations.TagSet{
			{
				Name: "OS",
				Tags: container.NewSet(
					"os_a",
					"os_b",
					"os_c",
				),
			},
			{
				Name: "GPU",
				Tags: container.NewSet(
					"gpu_a",
					"gpu_b",
					"gpu_c",
				),
			},
		},
		ByName: map[string]expectations.TagSetAndPriority{
			"os_a":  {Set: "OS", Priority: 0},
			"os_b":  {Set: "OS", Priority: 1},
			"os_c":  {Set: "OS", Priority: 2},
			"gpu_a": {Set: "GPU", Priority: 0},
			"gpu_b": {Set: "GPU", Priority: 1},
			"gpu_c": {Set: "GPU", Priority: 2},
		},
	}
	type Test struct {
		in       []string
		expected []string
	}
	for _, test := range []Test{
		{in: []string{"os_a"}, expected: []string{"os_a"}},
		{in: []string{"gpu_b"}, expected: []string{"gpu_b"}},
		{in: []string{"gpu_b", "os_a"}, expected: []string{"gpu_b", "os_a"}},
		{in: []string{"gpu_a", "gpu_b"}, expected: []string{"gpu_b"}},
		{in: []string{"gpu_b", "gpu_c"}, expected: []string{"gpu_c"}},
		{in: []string{"os_a", "os_b"}, expected: []string{"os_b"}},
		{in: []string{"os_b", "os_c"}, expected: []string{"os_c"}},
		{in: []string{"gpu_a", "gpu_c", "os_b", "os_c"}, expected: []string{"gpu_c", "os_c"}},
	} {
		got := tags.RemoveLowerPriorityTags(container.NewSet(test.in...)).List()
		if diff := cmp.Diff(got, test.expected); diff != "" {
			t.Errorf("TestRemoveLowerPriorityTags(%v) returned %v:\n%v", test.in, got, diff)
		}
	}
}

func TestRemoveHigherPriorityTags(t *testing.T) {
	tags := expectations.Tags{
		Sets: []expectations.TagSet{
			{
				Name: "OS",
				Tags: container.NewSet(
					"os_a",
					"os_b",
					"os_c",
				),
			},
			{
				Name: "GPU",
				Tags: container.NewSet(
					"gpu_a",
					"gpu_b",
					"gpu_c",
				),
			},
		},
		ByName: map[string]expectations.TagSetAndPriority{
			"os_a":  {Set: "OS", Priority: 0},
			"os_b":  {Set: "OS", Priority: 1},
			"os_c":  {Set: "OS", Priority: 2},
			"gpu_a": {Set: "GPU", Priority: 0},
			"gpu_b": {Set: "GPU", Priority: 1},
			"gpu_c": {Set: "GPU", Priority: 2},
		},
	}
	type Test struct {
		in       []string
		expected []string
	}
	for _, test := range []Test{
		{in: []string{"os_a"}, expected: []string{"os_a"}},
		{in: []string{"gpu_b"}, expected: []string{"gpu_b"}},
		{in: []string{"gpu_b", "os_a"}, expected: []string{"gpu_b", "os_a"}},
		{in: []string{"gpu_a", "gpu_b"}, expected: []string{"gpu_a"}},
		{in: []string{"gpu_b", "gpu_c"}, expected: []string{"gpu_b"}},
		{in: []string{"os_a", "os_b"}, expected: []string{"os_a"}},
		{in: []string{"os_b", "os_c"}, expected: []string{"os_b"}},
		{in: []string{"gpu_a", "gpu_c", "os_b", "os_c"}, expected: []string{"gpu_a", "os_b"}},
		{in: []string{"gpu_c", "gpu_a", "os_c", "os_b"}, expected: []string{"gpu_a", "os_b"}},
	} {
		got := tags.RemoveHigherPriorityTags(container.NewSet(test.in...)).List()
		if diff := cmp.Diff(got, test.expected); diff != "" {
			t.Errorf("TestRemoveHigherPriorityTags(%v) returned %v:\n%v", test.in, got, diff)
		}
	}
}
