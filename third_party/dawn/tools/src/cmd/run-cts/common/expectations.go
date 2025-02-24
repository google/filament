// Copyright 2023 The Dawn & Tint Authors
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
	"encoding/json"
	"fmt"
	"os"
	"sort"
)

// Expectations is a map of test to status
type Expectations map[TestCase]Status

// Load loads the results from path
func (e *Expectations) Load(path string) error {
	f, err := os.Open(path)
	if err != nil {
		return fmt.Errorf("failed to open expectations file: %w", err)
	}
	defer f.Close()

	statuses := []testcaseStatus{}
	if err := json.NewDecoder(f).Decode(&statuses); err != nil {
		return fmt.Errorf("failed to read expectations file: %w", err)
	}

	*e = make(Expectations, len(statuses))
	for _, s := range statuses {
		(*e)[s.TestCase] = s.Status
	}
	return nil
}

// Save saves the results to path
func (e Expectations) Save(path string) error {
	f, err := os.Create(path)
	if err != nil {
		return fmt.Errorf("failed to create expectations file: %w", err)
	}
	defer f.Close()

	statuses := make([]testcaseStatus, 0, len(e))
	for testcase, status := range e {
		statuses = append(statuses, testcaseStatus{testcase, status})
	}
	sort.Slice(statuses, func(i, j int) bool { return statuses[i].TestCase < statuses[j].TestCase })

	enc := json.NewEncoder(f)
	enc.SetIndent("", "  ")
	if err := enc.Encode(&statuses); err != nil {
		return fmt.Errorf("failed to save expectations file: %w", err)
	}

	return nil
}

// testcaseStatus is a pair of testcase name and result status
// Intended to be serialized for expectations files.
type testcaseStatus struct {
	TestCase TestCase
	Status   Status
}
