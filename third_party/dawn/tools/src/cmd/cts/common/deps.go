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

package common

import (
	"fmt"
	"regexp"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/container"
)

// The regular expression used to search for the CTS hash
var reCTSHash = regexp.MustCompile(reEscape(ctsHashPrefix) + `[0-9a-fA-F]+`)

const (
	// The string prefix for the CTS hash in the DEPs file, used for identifying
	// and updating the DEPS file.
	ctsHashPrefix = `{chromium_git}/external/github.com/gpuweb/cts@`
)

func reEscape(s string) string {
	return strings.ReplaceAll(strings.ReplaceAll(s, `/`, `\/`), `.`, `\.`)
}

// UpdateCTSHashInDeps replaces the CTS hashes in 'deps' with 'newGitURL@newCTSHash'.
// Returns:
//
//	newDEPS    - the new DEPS content
//	oldCTSHash - the old CTS hash in the 'deps'
func UpdateCTSHashInDeps(deps, newGitURL, newCTSHash string) (newDEPS, oldCTSHash string, err error) {
	// Collect old CTS hashes, and replace these with newCTSHash
	b := strings.Builder{}
	oldCTSHashes := []string{}
	matches := reCTSHash.FindAllStringIndex(deps, -1)
	if len(matches) == 0 {
		return "", "", fmt.Errorf("failed to find a CTS hash in DEPS file")
	}
	end := 0
	for _, match := range matches {
		replacement := fmt.Sprintf("%v@%v", newGitURL, newCTSHash)
		replacement = strings.ReplaceAll(replacement, "https://chromium.googlesource.com", "{chromium_git}")

		oldCTSHashes = append(oldCTSHashes, deps[match[0]+len(ctsHashPrefix):match[1]])
		b.WriteString(deps[end:match[0]])
		b.WriteString(replacement)
		end = match[1]
	}
	b.WriteString(deps[end:])

	newDEPS = b.String()

	if s := container.NewSet(oldCTSHashes...); len(s) > 1 {
		fmt.Println("DEPS contained multiple hashes for CTS, using first for logs")
	}
	oldCTSHash = oldCTSHashes[0]

	return newDEPS, oldCTSHash, nil
}
