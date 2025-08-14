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
	"strings"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestDefaultPaths_PathDoesNotExist(t *testing.T) {
	tests := []struct {
		name string
		f    func(oswrapper.FilesystemReader) string
	}{
		{
			name: "DefaultExpectationsPath",
			f:    DefaultExpectationsPath,
		},
		{
			name: "DefaultCompatExpectationsPath",
			f:    DefaultCompatExpectationsPath,
		},
		{
			name: "DefaultSlowExpectationsPath",
			f:    DefaultSlowExpectationsPath,
		},
		{
			name: "DefaultTestListPath",
			f:    DefaultTestListPath,
		},
		{
			name: "DefaultCTSPath",
			f:    DefaultCTSPath,
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper, err := fileutils.CreateMemMapOSWrapperWithFakeDawnRoot()
			require.NoErrorf(t, err, "Error creating fake Dawn root: %v", err)
			require.Equal(t, testCase.f(wrapper), "")
		})
	}
}

func TestDefaultPaths_PathExists(t *testing.T) {
	tests := []struct {
		name           string
		f              func(oswrapper.FilesystemReader) string
		expectedSuffix string
	}{
		{
			name:           "DefaultExpectationsPath",
			f:              DefaultExpectationsPath,
			expectedSuffix: RelativeExpectationsPath,
		},
		{
			name:           "DefaultCompatExpectationsPath",
			f:              DefaultCompatExpectationsPath,
			expectedSuffix: RelativeCompatExpectationsPath,
		},
		{
			name:           "DefaultSlowExpectationsPath",
			f:              DefaultSlowExpectationsPath,
			expectedSuffix: RelativeSlowExpectationsPath,
		},
		{
			name:           "DefaultTestListPath",
			f:              DefaultTestListPath,
			expectedSuffix: RelativeTestListPath,
		},
		{
			name:           "DefaultCTSPath",
			f:              DefaultCTSPath,
			expectedSuffix: RelativeCTSPath,
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper, err := CreateMemMapOSWrapperWithFakeDefaultPaths()
			require.NoErrorf(t, err, "Error creating fake Dawn directories: %v", err)
			expectationsPath := testCase.f(wrapper)
			require.NotEqual(t, expectationsPath, "")
			require.True(t, strings.HasPrefix(expectationsPath, fileutils.DawnRoot(wrapper)))
			require.True(t, strings.HasSuffix(expectationsPath, testCase.expectedSuffix))
		})
	}
}

func TestDefaultExpectationsPaths_PathDoesNotExist(t *testing.T) {
	wrapper, err := fileutils.CreateMemMapOSWrapperWithFakeDawnRoot()
	require.NoErrorf(t, err, "Error creating fake Dawn root: %v", err)
	require.Equal(t, DefaultExpectationsPaths(wrapper), []string{"", ""})
}

func TestDefaultExpectationsPaths_PathExists(t *testing.T) {
	wrapper, err := CreateMemMapOSWrapperWithFakeDefaultPaths()
	require.NoErrorf(t, err, "Error creating fake Dawn directories: %v", err)
	expectationsPaths := DefaultExpectationsPaths(wrapper)
	require.NotEqual(t, expectationsPaths, []string{"", ""})
	require.Equal(t, len(expectationsPaths), 2)
	require.True(t, strings.HasPrefix(expectationsPaths[0], fileutils.DawnRoot(wrapper)))
	require.True(t, strings.HasPrefix(expectationsPaths[1], fileutils.DawnRoot(wrapper)))
	require.True(t, strings.HasSuffix(expectationsPaths[0], RelativeExpectationsPath))
	require.True(t, strings.HasSuffix(expectationsPaths[1], RelativeCompatExpectationsPath))
}
