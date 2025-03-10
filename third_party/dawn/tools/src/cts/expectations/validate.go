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

// Package expectations provides types and helpers for parsing, updating and
// writing WebGPU expectations files.
//
// See <dawn>/webgpu-cts/expectations.txt for more information.
package expectations

import (
	"fmt"

	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"github.com/google/go-cmp/cmp"
)

// Validate checks that the expectations do not contain errors
func (c Content) Validate() Diagnostics {
	tree, _ := query.NewTree[Expectations]()
	for _, chunk := range c.Chunks {
		for _, ex := range chunk.Expectations {
			node := tree.GetOrCreate(query.Parse(ex.Query), func() Expectations {
				return Expectations{}
			})
			*node = append(*node, ex)
		}
	}
	var out Diagnostics
	for _, chunk := range c.Chunks {
		for _, ex := range chunk.Expectations {
			for _, status := range ex.Status {
				if status == "Slow" {
					out = append(out, Diagnostic{
						Severity: Error,
						Line:     ex.Line,
						Message:  fmt.Sprintf("\"Slow\" expectation is not valid here. Use slow_tests.txt instead."),
					})
				}
			}
			_, err := tree.Glob(query.Parse(ex.Query))
			if err != nil {
				out = append(out, Diagnostic{
					Severity: Error,
					Line:     ex.Line,
					Message:  err.Error(),
				})
				continue
			}
		}
	}
	return out
}

// ValidateSlowTests checks that the expectations are only [ Slow ]
func (c Content) ValidateSlowTests() Diagnostics {
	var out Diagnostics
	for _, chunk := range c.Chunks {
		for _, ex := range chunk.Expectations {
			if !cmp.Equal(ex.Status, []string{"Slow"}) {
				out = append(out, Diagnostic{
					Severity: Error,
					Line:     ex.Line,
					Message:  fmt.Sprintf("slow test expectation for %v must be %v but was %v", ex.Query, []string{"Slow"}, ex.Status),
				})
			}
		}
	}
	return out
}
