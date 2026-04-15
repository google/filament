// Copyright 2025 The Dawn & Tint Authors
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

package validate

import (
	"context"
	"os"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestValidate_ValidRegularAndSlow(t *testing.T) {
	// Valid expectations
	content := `# Tags
# tags: [ linux mac ]

crbug.com/123 [ mac ] q [ Failure ]
`
	wrapper := oswrapper.CreateFSTestOSWrapper()
	path := "expectations.txt"
	require.NoError(t, wrapper.WriteFile(path, []byte(content), 0666))

	slowContent := `# Tags
# tags: [ linux mac ]

crbug.com/123 [ linux ] q2 [ Slow ]`
	slowPath := "slow.txt"
	require.NoError(t, wrapper.WriteFile(slowPath, []byte(slowContent), 0666))

	cfg := common.Config{
		OsWrapper: wrapper,
	}
	c := &cmd{}
	c.flags.expectations = []string{path}
	c.flags.slow = slowPath

	err := c.Run(context.Background(), cfg)
	require.NoError(t, err)
}

func TestValidate_InvalidRegular(t *testing.T) {
	content := `# Tags
# tags: [ linux ]

crbug.com/123 [ mac ] q [ Slow ]
`
	wrapper := oswrapper.CreateFSTestOSWrapper()
	path := "expectations.txt"
	require.NoError(t, wrapper.WriteFile(path, []byte(content), 0666))

	slowContent := `# Tags
# tags: [ linux mac ]

crbug.com/123 [ linux ] q2 [ Slow ]`
	slowPath := "slow.txt"
	require.NoError(t, wrapper.WriteFile(slowPath, []byte(slowContent), 0666))

	cfg := common.Config{
		OsWrapper: wrapper,
	}
	c := &cmd{}
	c.flags.expectations = []string{path}
	c.flags.slow = slowPath

	err := c.Run(context.Background(), cfg)
	require.ErrorContains(t, err, "1 errors found in expectations.txt")
}

func TestValidate_InvalidSlow(t *testing.T) {
	content := ""
	wrapper := oswrapper.CreateFSTestOSWrapper()
	path := "expectations.txt"
	require.NoError(t, wrapper.WriteFile(path, []byte(content), 0666))

	// Invalid slow expectations (using Failure status)
	slowContent := `q [ Failure ]`
	slowPath := "slow.txt"
	require.NoError(t, wrapper.WriteFile(slowPath, []byte(slowContent), 0666))

	cfg := common.Config{
		OsWrapper: wrapper,
	}
	c := &cmd{}
	c.flags.expectations = []string{path}
	c.flags.slow = slowPath

	err := c.Run(context.Background(), cfg)
	require.ErrorContains(t, err, "1 errors found in slow.txt")
}

func TestValidate_LoadExpectationsFail(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()

	cfg := common.Config{
		OsWrapper: wrapper,
	}
	c := &cmd{}
	c.flags.expectations = []string{"missing_expectations.txt"}
	c.flags.slow = "slow.txt"

	err := c.Run(context.Background(), cfg)
	require.ErrorIs(t, err, os.ErrNotExist)
}

func TestValidate_LoadSlowExpectationsFail(t *testing.T) {
	content := ""
	wrapper := oswrapper.CreateFSTestOSWrapper()
	path := "expectations.txt"
	require.NoError(t, wrapper.WriteFile(path, []byte(content), 0666))

	slowPath := "missing_slow.txt"

	cfg := common.Config{
		OsWrapper: wrapper,
	}
	c := &cmd{}
	c.flags.expectations = []string{path}
	c.flags.slow = slowPath

	err := c.Run(context.Background(), cfg)
	require.ErrorIs(t, err, os.ErrNotExist)
}
