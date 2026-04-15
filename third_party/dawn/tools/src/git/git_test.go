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

package git

import (
	"io/fs"
	"path/filepath"
	"testing"
	"testing/fstest"
	"time"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestHashString(t *testing.T) {
	hash := Hash{0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67}
	got := hash.String()
	want := "0123456789abcdef0123456789abcdef01234567"
	require.Equal(t, want, got)
}

func TestHashIsZero(t *testing.T) {
	require.True(t, (Hash{}).IsZero())
	require.False(t, (Hash{1}).IsZero())
}

func TestParseHash(t *testing.T) {
	t.Run("valid hash", func(t *testing.T) {
		got, err := ParseHash("0123456789abcdef0123456789abcdef01234567")
		require.NoError(t, err)
		want := Hash{0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67}
		require.Equal(t, want, got)
	})

	t.Run("invalid hash string", func(t *testing.T) {
		_, err := ParseHash("invalidhash")
		require.Error(t, err)
		require.Contains(t, err.Error(), "failed to parse hash 'invalidhash'")
	})
}

func TestCredentialsEmpty(t *testing.T) {
	require.True(t, (Credentials{}).Empty())
	require.False(t, (Credentials{Username: "a"}).Empty())
	require.False(t, (Credentials{Password: "b"}).Empty())
}

func TestCredentialsAddToURL(t *testing.T) {
	t.Run("valid url", func(t *testing.T) {
		c := Credentials{"user", "pass"}
		got, err := c.addToURL("https://example.com")
		require.NoError(t, err)
		want := "https://user:pass@example.com"
		require.Equal(t, want, got)
	})

	t.Run("invalid url", func(t *testing.T) {
		c := Credentials{"user", "pass"}
		_, err := c.addToURL("://invalid")
		require.Error(t, err)
	})
}

func TestGitOpen(t *testing.T) {
	// Test opening a valid repository
	t.Run("valid repository", func(t *testing.T) {
		os := oswrapper.CreateFSTestOSWrapper()
		gitDir := filepath.Join("my-repo", ".git")
		os.FS[".git"] = &fstest.MapFile{Mode: fs.ModeDir}
		os.FS["my-repo"] = &fstest.MapFile{Mode: fs.ModeDir}
		os.FS[filepath.ToSlash(gitDir)] = &fstest.MapFile{Mode: fs.ModeDir}

		g := Git{os: os}
		repo, err := g.Open("my-repo")
		require.NoError(t, err)
		require.Equal(t, "my-repo", repo.Path)
	})

	// Test opening a non-existent repository
	t.Run("non-existent repository", func(t *testing.T) {
		os := oswrapper.CreateFSTestOSWrapper()
		g := Git{os: os}
		_, err := g.Open("non-existent-repo")
		require.Equal(t, ErrRepositoryDoesNotExist, err)
	})

	// Test opening a path that is a file
	t.Run("path is a file", func(t *testing.T) {
		os := oswrapper.CreateFSTestOSWrapper()
		os.FS[".git"] = &fstest.MapFile{Data: []byte("not a directory")}
		g := Git{os: os}
		_, err := g.Open(".")
		require.Equal(t, ErrRepositoryDoesNotExist, err)
	})
}

func TestParseLog(t *testing.T) {
	t.Run("valid log", func(t *testing.T) {
		const log = `ǁde0122585373851323c2115b2231165334f26e41ǀ2022-07-22T15:42:05-07:00ǀTest User <test.user@example.com>ǀSubject 1ǀDescription 1
ǁaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaǀ2022-07-21T15:42:05-07:00ǀTest User <test.user@example.com>ǀSubject 2ǀDescription 2
`
		got, err := parseLog(log)
		require.NoError(t, err)
		want := []CommitInfo{
			{
				Hash:        mustParseHash("de0122585373851323c2115b2231165334f26e41"),
				Date:        time.Date(2022, 07, 22, 15, 42, 05, 0, time.FixedZone("", -7*60*60)),
				Author:      "Test User <test.user@example.com>",
				Subject:     "Subject 1",
				Description: "Description 1",
			},
			{
				Hash:        mustParseHash("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"),
				Date:        time.Date(2022, 07, 21, 15, 42, 05, 0, time.FixedZone("", -7*60*60)),
				Author:      "Test User <test.user@example.com>",
				Subject:     "Subject 2",
				Description: "Description 2",
			},
		}

		// Compare fields individually, especially time.Time using UnixNano for robustness
		require.Len(t, got, len(want))
		for i := range got {
			require.Equal(t, want[i].Hash, got[i].Hash)
			require.Equal(t, want[i].Date.UnixNano(), got[i].Date.UnixNano())
			require.Equal(t, want[i].Author, got[i].Author)
			require.Equal(t, want[i].Subject, got[i].Subject)
			require.Equal(t, want[i].Description, got[i].Description)
		}
	})

	t.Run("invalid hash in log", func(t *testing.T) {
		const log = `ǁinvalidhashstringǀ2022-07-22T15:42:05-07:00ǀTest User <test.user@example.com>ǀSubject 1ǀDescription 1`
		_, err := parseLog(log)
		require.Error(t, err)
		require.Contains(t, err.Error(), "failed to parse hash 'invalidhashstring'")
	})

	t.Run("invalid date in log", func(t *testing.T) {
		const log = `ǁde0122585373851323c2115b2231165334f26e41ǀinvalid-dateǀTest User <test.user@example.com>ǀSubject 1ǀDescription 1`
		_, err := parseLog(log)
		require.Error(t, err)
		require.Contains(t, err.Error(), "cannot parse \"invalid-date\"")
	})
}

func mustParseHash(s string) Hash {
	h, err := ParseHash(s)
	if err != nil {
		panic(err)
	}
	return h
}
