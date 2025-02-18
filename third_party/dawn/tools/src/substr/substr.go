// Copyright 2021 The Dawn & Tint Authors
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

package substr

import (
	diff "github.com/sergi/go-diff/diffmatchpatch"
)

// Fix attempts to reconstruct substr by comparing it to body.
// substr is a fuzzy substring of body.
// Fix returns a new exact substring of body, by calculating a diff of the text.
// If no match could be made, Fix() returns an empty string.
func Fix(body, substr string) string {
	dmp := diff.New()

	diffs := dmp.DiffMain(body, substr, false)
	if len(diffs) == 0 {
		return ""
	}

	front := func() diff.Diff { return diffs[0] }
	back := func() diff.Diff { return diffs[len(diffs)-1] }

	start, end := 0, len(body)

	// Trim edits that remove text from body start
	for len(diffs) > 0 && front().Type == diff.DiffDelete {
		start += len(front().Text)
		diffs = diffs[1:]
	}

	// Trim edits that remove text from body end
	for len(diffs) > 0 && back().Type == diff.DiffDelete {
		end -= len(back().Text)
		diffs = diffs[:len(diffs)-1]
	}

	// New substring is the span for the remainder of the edits
	return body[start:end]
}
