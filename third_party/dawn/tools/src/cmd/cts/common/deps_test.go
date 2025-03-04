// Copyright 2024 The Dawn & Tint Authors
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

package common_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
)

func TestUpdateCTSHashInDeps(t *testing.T) {
	for _, test := range []struct {
		oldDeps          string
		newGitURL        string
		newCTSHash       string
		expectedDeps     string
		expectedRevision string
	}{
		{
			oldDeps: `
BEFORE BEFORE BEFORE
'third_party/webgpu-cts': {
	'url': '{chromium_git}/external/github.com/gpuweb/cts@3e45aee0b16dc724a79a0feb0490e2ddb06c9f0d',
	'condition': 'build_with_chromium',
},
AFTER AFTER AFTER
`,
			newGitURL:  `https://chromium.googlesource.com/external/github.com/gpuweb/cts`,
			newCTSHash: `ef28f69a837732739a1e0411cdf7d44a36bfeaa1`,
			expectedDeps: `
BEFORE BEFORE BEFORE
'third_party/webgpu-cts': {
	'url': '{chromium_git}/external/github.com/gpuweb/cts@ef28f69a837732739a1e0411cdf7d44a36bfeaa1',
	'condition': 'build_with_chromium',
},
AFTER AFTER AFTER
`,
			expectedRevision: `3e45aee0b16dc724a79a0feb0490e2ddb06c9f0d`,
		},
		{
			oldDeps: `
BEFORE BEFORE BEFORE
'third_party/webgpu-cts': {
	'url': '{chromium_git}/external/github.com/gpuweb/cts@3e45aee0b16dc724a79a0feb0490e2ddb06c9f0d',
	'condition': 'build_with_chromium',
},
AFTER AFTER AFTER
`,
			newGitURL:  `https://github.com/a_cts_contributor/cts.git`,
			newCTSHash: `ef28f69a837732739a1e0411cdf7d44a36bfeaa1`,
			expectedDeps: `
BEFORE BEFORE BEFORE
'third_party/webgpu-cts': {
	'url': 'https://github.com/a_cts_contributor/cts.git@ef28f69a837732739a1e0411cdf7d44a36bfeaa1',
	'condition': 'build_with_chromium',
},
AFTER AFTER AFTER
`,
			expectedRevision: `3e45aee0b16dc724a79a0feb0490e2ddb06c9f0d`,
		},
	} {
		newDeps, newRevision, err := common.UpdateCTSHashInDeps(test.oldDeps, test.newGitURL, test.newCTSHash)
		if err != nil {
			t.Error(err)
			continue
		}
		if newDeps != test.expectedDeps {
			t.Errorf("returned DEPS was not as expected.\nexpected:\n%v\ngot:\n%v\n", test.expectedDeps, newDeps)
		}
		if newRevision != test.expectedRevision {
			t.Errorf("returned revision was not as expected. expected: '%v', got: '%v'", test.expectedDeps, newDeps)
		}
	}
}
