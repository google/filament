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
	"os"
	"path/filepath"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

const (
	// RelativeExpectationsPath is the dawn-root relative path to the
	// expectations.txt file.
	RelativeExpectationsPath = "webgpu-cts/expectations.txt"

	// RelativeCompatExpectationsPath is the dawn-root relative path to the
	// compat-expectations.txt file.
	RelativeCompatExpectationsPath = "webgpu-cts/compat-expectations.txt"

	// RelativeSlowExpectationsPath is the dawn-root relative path to the
	// slow_tests.txt file.
	RelativeSlowExpectationsPath = "webgpu-cts/slow_tests.txt"

	// RelativeTestListPath is the dawn-root relative path to the test_list.txt file.
	RelativeTestListPath = "third_party/gn/webgpu-cts/test_list.txt"

	// RelativeCTSPath is the dawn-root relative path to the WebGPU CTS directory.
	RelativeCTSPath = "third_party/webgpu-cts"
)

// DefaultExpectationsPath returns the default path to the expectations.txt
// file. Returns an empty string if the file cannot be found.
func DefaultExpectationsPath() string {
	path := filepath.Join(fileutils.DawnRoot(), RelativeExpectationsPath)
	if _, err := os.Stat(path); err != nil {
		return ""
	}
	return path
}

// DefaultCompatExpectationsPath returns the default path to the compat-expectations.txt
// file. Returns an empty string if the file cannot be found.
func DefaultCompatExpectationsPath() string {
	path := filepath.Join(fileutils.DawnRoot(), RelativeCompatExpectationsPath)
	if _, err := os.Stat(path); err != nil {
		return ""
	}
	return path
}

// DefaultExpectationsPaths returns the default set of expectations files commands
// will use if no alternative list of files is supplied.
func DefaultExpectationsPaths() []string {
	return []string{
		DefaultExpectationsPath(),
		DefaultCompatExpectationsPath(),
	}
}

// DefaultSlowExpectationsPath returns the default path to the slow_tests.txt
// file. Returns an empty string if the file cannot be found.
func DefaultSlowExpectationsPath() string {
	path := filepath.Join(fileutils.DawnRoot(), RelativeSlowExpectationsPath)
	if _, err := os.Stat(path); err != nil {
		return ""
	}
	return path
}

// DefaultTestListPath returns the default path to the test_list.txt file.
// Returns an empty string if the file cannot be found.
func DefaultTestListPath() string {
	path := filepath.Join(fileutils.DawnRoot(), RelativeTestListPath)
	if _, err := os.Stat(path); err != nil {
		return ""
	}
	return path
}

// DefaultCTSPath returns the default path to the WenGPU CTS directory.
// Returns an empty string if the file cannot be found.
func DefaultCTSPath() string {
	path := filepath.Join(fileutils.DawnRoot(), RelativeCTSPath)
	if _, err := os.Stat(path); err != nil {
		return ""
	}
	return path
}
