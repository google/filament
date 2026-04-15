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

package expectations

import (
	"context"
	"path/filepath"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestUpdateExpectations(t *testing.T) {
	tests := []struct {
		name                 string
		resultsFile          string
		resultsContent       string
		testListPath         string
		testListContent      string
		expectationsPath     string
		expectationsContent  string
		expectationsFlag     []string
		wantErr              string
		wantLog              string
		verifyContent        func(t *testing.T, fs oswrapper.FilesystemReader, path string)
		generateExplicitTags bool
	}{
		{
			name:             "Success",
			resultsFile:      "results.txt",
			resultsContent:   "webgpu:test:one linux,nvidia Failure 0s false\ncore",
			testListPath:     "third_party/gn/webgpu-cts/test_list.txt",
			testListContent:  "webgpu:test:one\nwebgpu:test:two",
			expectationsPath: "expectations.txt",
			expectationsContent: `# BEGIN TAG HEADER
# OS
# tags: [ mac win linux android ]
# GPU
# tags: [ amd intel nvidia qualcomm ]
# Device
# tags: [ android-pixel-4 android-pixel-6 chromeos-board-amd64-generic fuchsia-board-qemu-x64 ]
# END TAG HEADER

# existing
[ mac ] webgpu:test:two: [ Failure ]
`,
			expectationsFlag: []string{"expectations.txt"},
			verifyContent: func(t *testing.T, fs oswrapper.FilesystemReader, path string) {
				content, err := fs.ReadFile(path)
				require.NoError(t, err)
				text := string(content)
				require.Contains(t, text, "[ linux nvidia ] webgpu:test:one: [ Failure ]")
				require.Contains(t, text, "[ mac ] webgpu:test:two: [ Failure ]")
			},
		},
		{
			name:             "Results Fetch Error",
			resultsFile:      "nonexistent.txt",
			testListPath:     "third_party/gn/webgpu-cts/test_list.txt",
			testListContent:  "webgpu:test:one",
			expectationsPath: "expectations.txt",
			expectationsFlag: []string{"expectations.txt"},
			wantErr:          "does not exist",
		},
		{
			name:             "Test List Load Error",
			resultsFile:      "results.txt",
			resultsContent:   "webgpu:test:one linux Failure 0s false\ncore",
			testListPath:     "nonexistent/test_list.txt",
			expectationsPath: "expectations.txt",
			expectationsFlag: []string{"expectations.txt"},
			wantErr:          "failed to load test list",
		},
		{
			name:             "Expectations Load Error",
			resultsFile:      "results.txt",
			resultsContent:   "webgpu:test:one linux Failure 0s false\ncore",
			testListPath:     "third_party/gn/webgpu-cts/test_list.txt",
			testListContent:  "webgpu:test:one\nwebgpu:test:two",
			expectationsPath: "expectations.txt",
			expectationsFlag: []string{"nonexistent.txt"},
			wantErr:          "does not exist",
		},
		{
			name:                "Expectations Validation Error",
			resultsFile:         "results.txt",
			resultsContent:      "webgpu:test:one linux Failure 0s false\ncore",
			testListPath:        "third_party/gn/webgpu-cts/test_list.txt",
			testListContent:     "webgpu:test:one",
			expectationsPath:    "expectations.txt",
			expectationsContent: "INVALID SYNTAX",
			expectationsFlag:    []string{"expectations.txt"},
			wantErr:             "expected status",
		},
		{
			name:             "Compat Mode",
			resultsFile:      "results.txt",
			resultsContent:   "webgpu:test:compat linux,nvidia Failure 0s false\ncompat",
			testListPath:     "third_party/gn/webgpu-cts/test_list.txt",
			testListContent:  "webgpu:test:compat",
			expectationsPath: "compat_expectations.txt",
			expectationsContent: `# BEGIN TAG HEADER
# OS
# tags: [ mac win linux android ]
# GPU
# tags: [ amd intel nvidia qualcomm ]
# Device
# tags: [ android-pixel-4 android-pixel-6 chromeos-board-amd64-generic fuchsia-board-qemu-x64 ]
# END TAG HEADER
`,
			expectationsFlag: []string{"compat_expectations.txt"},
			verifyContent: func(t *testing.T, fs oswrapper.FilesystemReader, path string) {
				content, err := fs.ReadFile(path)
				require.NoError(t, err)
				require.Contains(t, string(content), "[ linux nvidia ] webgpu:test:compat: [ Failure ]")
			},
		},
		{
			name:             "Unknown Tests Removal",
			resultsFile:      "results.txt",
			resultsContent:   "webgpu:test:known linux,nvidia Failure 0s false\ncore",
			testListPath:     "third_party/gn/webgpu-cts/test_list.txt",
			testListContent:  "webgpu:test:known",
			expectationsPath: "expectations.txt",
			expectationsContent: `# BEGIN TAG HEADER
# OS
# tags: [ mac win linux android ]
# GPU
# tags: [ amd intel nvidia qualcomm ]
# Device
# tags: [ android-pixel-4 android-pixel-6 chromeos-board-amd64-generic fuchsia-board-qemu-x64 ]
# END TAG HEADER

webgpu:test:unknown: [ Failure ]`,
			expectationsFlag: []string{"expectations.txt"},
			verifyContent: func(t *testing.T, fs oswrapper.FilesystemReader, path string) {
				content, err := fs.ReadFile(path)
				require.NoError(t, err)
				require.NotContains(t, string(content), "webgpu:test:unknown")
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			wrapper := oswrapper.CreateFSTestOSWrapper()
			realWrapper := oswrapper.GetRealOSWrapper()
			rootDir := fileutils.DawnRoot(realWrapper)

			// Helper to write file relative to root
			write := func(path, content string) {
				fullPath := filepath.Join(rootDir, path)
				require.NoError(t, wrapper.MkdirAll(filepath.Dir(fullPath), 0755))
				require.NoError(t, wrapper.WriteFile(fullPath, []byte(content), 0666))
			}

			if tt.resultsContent != "" {
				write(tt.resultsFile, tt.resultsContent)
			}
			if tt.testListContent != "" {
				write(tt.testListPath, tt.testListContent)
			}
			if tt.expectationsContent != "" {
				write(tt.expectationsPath, tt.expectationsContent)
			}

			c := &cmd{}

			// Resolve paths to absolute for flags
			absExpectations := make([]string, len(tt.expectationsFlag))
			for i, p := range tt.expectationsFlag {
				absExpectations[i] = filepath.Join(rootDir, p)
			}
			c.flags.expectations = absExpectations

			if tt.resultsFile != "" {
				c.flags.results.File = filepath.Join(rootDir, tt.resultsFile)
			}

			c.flags.verbose = true
			c.flags.generateExplicitTags = tt.generateExplicitTags

			ctx := context.Background()
			cfg := common.Config{OsWrapper: wrapper}

			write("DEPS", "") // Ensure DawnRoot detection works

			err := c.Run(ctx, cfg)
			if tt.wantErr != "" {
				require.Error(t, err)
				require.ErrorContains(t, err, tt.wantErr)
			} else {
				require.NoError(t, err)
			}

			if tt.verifyContent != nil {
				// Check content of the first expectations file
				path := filepath.Join(rootDir, tt.expectationsPath)
				tt.verifyContent(t, wrapper, path)
			}
		})
	}
}
