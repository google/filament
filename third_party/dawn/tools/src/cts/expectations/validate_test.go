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

package expectations_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
	"github.com/stretchr/testify/require"
)

const header = `# BEGIN TAG HEADER
# OS
# tags: [ os-a os-b os-c ]
# GPU
# tags: [ gpu-a gpu-b gpu-c ]
# END TAG HEADER
`

func TestValidate(t *testing.T) {
	type Test struct {
		name         string
		expectations string
		diagnostics  expectations.Diagnostics
	}
	for _, test := range []Test{
		{
			name:         "empty",
			expectations: ``,
		},
		{
			name: "simple",
			expectations: `
crbug.com/a/123 a:b,c:d,* [ Failure ]
`,
		},
		{
			name: "slow invalid",
			expectations: `
crbug.com/a/123 a:b,c:d,* [ Slow ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Line:     8,
					Severity: expectations.Error,
					Message:  "\"Slow\" expectation is not valid here. Use slow_tests.txt instead.",
				},
			},
		},
	} {
		t.Run(test.name, func(t *testing.T) {
			ex, err := expectations.Parse("expectations.txt", header+test.expectations)
			require.NoError(t, err)

			diagnostics := ex.Validate()
			require.Equal(t, test.diagnostics, diagnostics)
		})
	}
}

func TestValidateSlowTests(t *testing.T) {
	type Test struct {
		name         string
		expectations string
		diagnostics  expectations.Diagnostics
	}
	for _, test := range []Test{
		{
			name:         "empty",
			expectations: ``,
		},
		{
			name: "simple",
			expectations: `
crbug.com/a/123 a:b,c:d,* [ Slow ]
`,
		},
		{
			name: "failure invalid",
			expectations: `
crbug.com/a/123 a:b,c:d,* [ Failure ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Line:     8,
					Severity: expectations.Error,
					Message:  "slow test expectation for a:b,c:d,* must be [Slow] but was [Failure]",
				},
			},
		},
		{
			name: "mixed invalid",
			expectations: `
crbug.com/a/123 a:b,c:d,* [ Slow Failure ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Line:     8,
					Severity: expectations.Error,
					Message:  "slow test expectation for a:b,c:d,* must be [Slow] but was [Failure Slow]",
				},
			},
		},
		{
			name: "empty tags invalid",
			expectations: `
crbug.com/a/123 a:b,c:d,* [ ]
`,
			diagnostics: expectations.Diagnostics{
				{
					Line:     8,
					Severity: expectations.Error,
					Message:  "slow test expectation for a:b,c:d,* must be [Slow] but was []",
				},
			},
		},
	} {
		t.Run(test.name, func(t *testing.T) {
			ex, err := expectations.Parse("expectations.txt", header+test.expectations)
			require.NoError(t, err)

			diagnostics := ex.ValidateSlowTests()
			require.Equal(t, test.diagnostics, diagnostics)
		})
	}
}
