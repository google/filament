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

package common

import (
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
)

// AddToListingMeta will add new entries to listing_meta.json for new tests, using
// the timings in results
func AddToListingMeta(ctsDir string, results Results) error {
	path := filepath.Join(ctsDir, "src/webgpu/listing_meta.json")

	bytes, err := os.ReadFile(path)
	if err != nil {
		return fmt.Errorf("failed to read '%v': %w", path, err)
	}

	lines := strings.Split(string(bytes), "\n")

	re := regexp.MustCompile(`\s*"([^"]+)":\s*{\s*"subcaseMS":\s*([\d\.]+)\s*},`)

	timings := container.NewMap[string, float64]()
	for testCase, result := range results {
		q := query.Parse(string(testCase))
		q.Cases = "*" // Strip Cases
		noCases := q.String()
		timings[noCases] = timings[noCases] + float64(result.Duration.Microseconds())/1000
	}

	header := []string{}
	footer := []string{}
	timingsFound := false
	for i, line := range lines {
		match := re.FindStringSubmatch(line)
		if len(match) == 3 {
			ms, err := strconv.ParseFloat(match[2], 64)
			if err != nil {
				return fmt.Errorf("%v:%d failed to parse duration: %w", path, i+1, err)
			}
			timings.Remove(match[1])
			timings.Add(match[1], ms)
			timingsFound = true
		} else if line != "" {
			if timingsFound {
				footer = append(footer, line)
			} else {
				header = append(header, line)
			}
		}
	}

	out, err := os.Create(path)
	if err != nil {
		return fmt.Errorf("failed to open '%v' for writing: %w", path, err)
	}
	defer out.Close()

	for _, line := range header {
		fmt.Fprintln(out, line)
	}

	for _, test := range timings.Keys() {
		fmt.Fprintf(out, "  \"%v\": { \"subcaseMS\": %.3f },\n", test, timings[test])
	}

	for _, line := range footer {
		fmt.Fprintln(out, line)
	}

	return nil
}
