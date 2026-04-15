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

package format

import (
	"context"
	"errors"
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

type mockOSWrapper struct {
	oswrapper.OSWrapper
	readFileErr error
	createErr   error
}

func (m *mockOSWrapper) ReadFile(name string) ([]byte, error) {
	if m.readFileErr != nil {
		return nil, m.readFileErr
	}
	return m.OSWrapper.ReadFile(name)
}

func (m *mockOSWrapper) Create(name string) (oswrapper.File, error) {
	if m.createErr != nil {
		return nil, m.createErr
	}
	return m.OSWrapper.Create(name)
}

func TestFormat(t *testing.T) {
	unformatted := `# Tags
# tags: [ linux mac ]

crbug.com/456 [ linux ] q [ Failure ]
crbug.com/123 [ mac ] q [ Failure ]
`
	formatted := `# Tags
# tags: [ linux mac ]

crbug.com/123 [ mac ] q [ Failure ]
crbug.com/456 [ linux ] q [ Failure ]
`
	path := "expectations.txt"
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile(path, []byte(unformatted), 0666))

	cfg := common.Config{
		OsWrapper: wrapper,
	}
	c := &cmd{}
	c.flags.expectations = path

	err := c.Run(context.Background(), cfg)
	require.NoError(t, err)

	data, err := wrapper.ReadFile(path)
	require.NoError(t, err)
	require.Equal(t, formatted, string(data))
}

func TestFormat_MultipleFiles(t *testing.T) {
	content1 := `# Tags
# tags: [ linux mac ]

crbug.com/456 [ linux ] q [ Failure ]
`
	content2 := `# Tags
# tags: [ win android ]

crbug.com/789 [ android ] q [ Failure ]
`
	formatted1 := `# Tags
# tags: [ linux mac ]

crbug.com/456 [ linux ] q [ Failure ]
`
	formatted2 := `# Tags
# tags: [ win android ]

crbug.com/789 [ android ] q [ Failure ]
`

	path1 := "expectations1.txt"
	path2 := "expectations2.txt"

	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile(path1, []byte(content1), 0666))
	require.NoError(t, wrapper.WriteFile(path2, []byte(content2), 0666))

	cfg := common.Config{
		OsWrapper: wrapper,
	}
	c := &cmd{}
	c.flags.expectations = strings.Join([]string{path1, path2}, ",")

	err := c.Run(context.Background(), cfg)
	require.NoError(t, err)

	data1, err := wrapper.ReadFile(path1)
	require.NoError(t, err)
	require.Equal(t, formatted1, string(data1))

	data2, err := wrapper.ReadFile(path2)
	require.NoError(t, err)
	require.Equal(t, formatted2, string(data2))
}

func TestFormat_LoadError(t *testing.T) {
	wrapper := &mockOSWrapper{
		OSWrapper:   oswrapper.CreateFSTestOSWrapper(),
		readFileErr: errors.New("read error"),
	}

	path := "expectations.txt"
	cfg := common.Config{
		OsWrapper: wrapper,
	}
	c := &cmd{}
	c.flags.expectations = path

	err := c.Run(context.Background(), cfg)
	require.Error(t, err)
	require.ErrorContains(t, err, "read error")
}

func TestFormat_SaveError(t *testing.T) {
	innerWrapper := oswrapper.CreateFSTestOSWrapper()
	path := "expectations.txt"
	content := `# Tags
# tags: [ linux ]

crbug.com/123 [ linux ] q [ Failure ]
`
	require.NoError(t, innerWrapper.WriteFile(path, []byte(content), 0666))

	wrapper := &mockOSWrapper{
		OSWrapper: innerWrapper,
		createErr: errors.New("create error"),
	}

	cfg := common.Config{
		OsWrapper: wrapper,
	}
	c := &cmd{}
	c.flags.expectations = path

	err := c.Run(context.Background(), cfg)
	require.Error(t, err)
	require.ErrorContains(t, err, "create error")
}
