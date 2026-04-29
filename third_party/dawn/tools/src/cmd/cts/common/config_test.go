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

package common

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestLoadConfig_Success(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	configContent := `{
		"Gerrit": {
			"Host": "dawn-review.googlesource.com",
			"Project": "dawn"
		}
	}`
	err := wrapper.WriteFile("config.json", []byte(configContent), 0644)
	require.NoError(t, err)

	cfg, err := LoadConfig("config.json", wrapper)
	require.NoError(t, err)
	require.NotNil(t, cfg)
	require.Equal(t, "dawn-review.googlesource.com", cfg.Gerrit.Host)
	require.Equal(t, "dawn", cfg.Gerrit.Project)
	require.Equal(t, wrapper, cfg.OsWrapper)
}

func TestLoadConfig_WithComments(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	configContent := `{
		// This is a comment
		"Gerrit": {
			"Host": "dawn-review.googlesource.com",
			"Project": "dawn", // Trailing comma
		}
	}`
	err := wrapper.WriteFile("config.json", []byte(configContent), 0644)
	require.NoError(t, err)

	cfg, err := LoadConfig("config.json", wrapper)
	require.NoError(t, err)
	require.NotNil(t, cfg)
	require.Equal(t, "dawn-review.googlesource.com", cfg.Gerrit.Host)
}

func TestLoadConfig_FileError(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	// File not created

	cfg, err := LoadConfig("nonexistent.json", wrapper)
	require.Error(t, err)
	require.Nil(t, cfg)
	require.ErrorContains(t, err, "failed to open 'nonexistent.json'")
}

func TestLoadConfig_JSONError(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	configContent := `{ invalid json }`
	err := wrapper.WriteFile("config.json", []byte(configContent), 0644)
	require.NoError(t, err)

	cfg, err := LoadConfig("config.json", wrapper)
	require.Error(t, err)
	require.Nil(t, cfg)
	require.ErrorContains(t, err, "failed to load config")
}
