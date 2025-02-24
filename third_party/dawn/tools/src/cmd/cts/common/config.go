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
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"

	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/resultsdb"
	"github.com/tidwall/jsonc"
)

// Config holds the configuration data for the 'cts' command.
// Config is loaded from a JSON file stored next to the
// tools/src/cmd/cts/config.json.
type Config struct {
	// Test holds configuration data for test results.
	Tests []TestConfig
	// Gerrit holds configuration for Dawn's Gerrit server.
	Gerrit struct {
		// The host URL
		Host string
		// The project name
		Project string
	}
	// Git holds configuration data for the various Git repositories.
	Git struct {
		// The CTS git repository.
		CTS GitProject
		// The Dawn git repository.
		Dawn GitProject
	}
	// Builders is a map of builder name (as displayed in the UI) to buildbucket
	// builder information.
	Builders map[string]buildbucket.Builder
	// Tags holds configuration data for cleaning result tags before processing
	Tag struct {
		// Remove holds tags that should be removed before processing.
		// See crbug.com/dawn/1401 for more information.
		Remove []string
	}
	// Sheets holds information about the Google Sheets document used for
	// tracking CTS statistics.
	Sheets struct {
		ID string
	}
	OsWrapper oswrapper.OSWrapper
	Querier   resultsdb.Querier
}

// TestConfig holds configuration data for a single test type.
type TestConfig struct {
	// Mode used to refer to tests
	ExecutionMode result.ExecutionMode
	// The ResultDB string prefix for CTS tests.
	Prefixes []string
}

// GitProject holds a git host URL and project.
type GitProject struct {
	Host    string
	Project string
}

// HttpsURL returns the https URL of the project
func (g GitProject) HttpsURL() string {
	return fmt.Sprintf("https://%v/%v", g.Host, g.Project)
}

// LoadConfig loads the JSON config file at the given path
func LoadConfig(path string) (*Config, error) {
	data, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("failed to open '%v': %w", path, err)
	}

	// Remove comments, trailing commas.
	data = jsonc.ToJSONInPlace(data)

	cfg := Config{}
	if err := json.NewDecoder(bytes.NewReader(data)).Decode(&cfg); err != nil {
		return nil, fmt.Errorf("failed to load config: %w", err)
	}
	return &cfg, nil
}
