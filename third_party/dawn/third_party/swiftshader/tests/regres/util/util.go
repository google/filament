// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Package util provides small utility functions.
package util

import (
	"os"
)

// IsFile returns true if path is a file.
func IsFile(path string) bool {
	s, err := os.Stat(path)
	if err != nil {
		return false
	}
	return !s.IsDir()
}

// IsDir returns true if path is a directory.
func IsDir(path string) bool {
	s, err := os.Stat(path)
	if err != nil {
		return false
	}
	return s.IsDir()
}

// Percent returns the percentage completion of i items out of n.
func Percent(i, n int) int {
	return int(Percent64(int64(i), int64(n)))
}

// Percent64 returns the percentage completion of i items out of n.
func Percent64(i, n int64) int64 {
	if n == 0 {
		return 0
	}
	return (100 * i) / n
}
