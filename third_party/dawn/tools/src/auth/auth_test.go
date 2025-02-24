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

package auth

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
	"go.chromium.org/luci/auth"
)

func TestDefaultAuthOptions_NoAdditionalScopes(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	wrapper.Environment = map[string]string{
		"HOME": "/home",
	}

	authOptions := DefaultAuthOptions(wrapper)

	require.Equal(t, authOptions.SecretsDir, "/home/.config/dawn-cts")
	require.GreaterOrEqual(t, len(authOptions.Scopes), 2)
	require.Equal(
		t,
		[]string{
			"https://www.googleapis.com/auth/gerritcodereview",
			auth.OAuthScopeEmail,
		},
		authOptions.Scopes[len(authOptions.Scopes)-2:])
}

func TestDefaultAuthOptions_AdditionalScopes(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	wrapper.Environment = map[string]string{
		"HOME": "/home",
	}

	authOptions := DefaultAuthOptions(wrapper, "scope1", "scope2")

	require.Equal(t, authOptions.SecretsDir, "/home/.config/dawn-cts")
	require.GreaterOrEqual(t, len(authOptions.Scopes), 4)
	require.Equal(
		t,
		[]string{
			"https://www.googleapis.com/auth/gerritcodereview",
			auth.OAuthScopeEmail,
			"scope1",
			"scope2",
		},
		authOptions.Scopes[len(authOptions.Scopes)-4:])
}
