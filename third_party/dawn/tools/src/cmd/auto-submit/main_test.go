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

package main

import (
	"testing"
	"time"

	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"github.com/stretchr/testify/require"
)

func TestParseCQStatus(t *testing.T) {
	cqAuthor := gerrit.AccountInfo{Email: cqEmailAccount}
	otherAuthor := gerrit.AccountInfo{Email: "other@google.com"}
	now := time.Now()

	// CQ passed on the current patchset
	t.Run("cq passed", func(t *testing.T) {
		change := gerrit.ChangeInfo{
			CurrentRevision: "rev-2",
			Revisions: map[string]gerrit.RevisionInfo{
				"rev-1": {Number: 1},
				"rev-2": {Number: 2},
			},
			Messages: []gerrit.ChangeMessageInfo{
				{RevisionNumber: 1, Author: cqAuthor, Message: "This CL has passed the run", Date: gerrit.Timestamp{Time: now.Add(-time.Hour)}},
				{RevisionNumber: 2, Author: cqAuthor, Message: "This CL has passed the run", Date: gerrit.Timestamp{Time: now}},
			},
		}
		require.Equal(t, cqPassed, parseCQStatus(change))
	})

	// CQ failed on the current patchset
	t.Run("cq failed", func(t *testing.T) {
		change := gerrit.ChangeInfo{
			CurrentRevision: "rev-2",
			Revisions: map[string]gerrit.RevisionInfo{
				"rev-1": {Number: 1},
				"rev-2": {Number: 2},
			},
			Messages: []gerrit.ChangeMessageInfo{
				{RevisionNumber: 1, Author: cqAuthor, Message: "This CL has passed the run", Date: gerrit.Timestamp{Time: now.Add(-time.Hour)}},
				{RevisionNumber: 2, Author: cqAuthor, Message: "This CL has failed the run", Date: gerrit.Timestamp{Time: now}},
			},
		}
		require.Equal(t, cqFailed, parseCQStatus(change))
	})

	// CQ passed on a previous patchset, but not the current one
	t.Run("cq passed on previous patchset", func(t *testing.T) {
		change := gerrit.ChangeInfo{
			CurrentRevision: "rev-2",
			Revisions: map[string]gerrit.RevisionInfo{
				"rev-1": {Number: 1},
				"rev-2": {Number: 2},
			},
			Messages: []gerrit.ChangeMessageInfo{
				{RevisionNumber: 1, Author: cqAuthor, Message: "This CL has passed the run", Date: gerrit.Timestamp{Time: now.Add(-time.Hour)}},
			},
		}
		require.Equal(t, cqUnknown, parseCQStatus(change))
	})

	// No CQ message
	t.Run("no cq message", func(t *testing.T) {
		change := gerrit.ChangeInfo{
			CurrentRevision: "rev-1",
			Revisions: map[string]gerrit.RevisionInfo{
				"rev-1": {Number: 1},
			},
			Messages: []gerrit.ChangeMessageInfo{},
		}
		require.Equal(t, cqUnknown, parseCQStatus(change))
	})

	// Message from a different author
	t.Run("message from other author", func(t *testing.T) {
		change := gerrit.ChangeInfo{
			CurrentRevision: "rev-1",
			Revisions: map[string]gerrit.RevisionInfo{
				"rev-1": {Number: 1},
			},
			Messages: []gerrit.ChangeMessageInfo{
				{RevisionNumber: 1, Author: otherAuthor, Message: "This CL has passed the run", Date: gerrit.Timestamp{Time: now}},
			},
		}
		require.Equal(t, cqUnknown, parseCQStatus(change))
	})

	// Message on the current patchset, but not a CQ status message
	t.Run("other message on current patchset", func(t *testing.T) {
		change := gerrit.ChangeInfo{
			CurrentRevision: "rev-1",
			Revisions: map[string]gerrit.RevisionInfo{
				"rev-1": {Number: 1},
			},
			Messages: []gerrit.ChangeMessageInfo{
				{RevisionNumber: 1, Author: cqAuthor, Message: "Some other message", Date: gerrit.Timestamp{Time: now}},
			},
		}
		require.Equal(t, cqUnknown, parseCQStatus(change))
	})
}
