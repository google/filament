// Copyright 2020 Google LLC
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

// Package glob provides file globbing utilities
package glob

import (
	"bytes"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/match"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

// Glob returns all the paths that match the given filepath glob
func Glob(str string, fsReader oswrapper.FilesystemReader) ([]string, error) {
	abs, err := filepath.Abs(str)
	if err != nil {
		return nil, err
	}
	root, glob := "", ""
	// Look for rightmost directory delimiter that's left of a wildcard. Use
	// that to split the 'root' from the match 'glob'.
	for i, c := range abs {
		switch c {
		case filepath.Separator:
			root, glob = abs[:i], abs[i+1:]
		case '*', '?':
			test, err := match.New(glob)
			if err != nil {
				return nil, err
			}
			files, err := Scan(root, Config{Paths: searchRules{
				func(path string, cond bool) bool { return test(path) },
			}}, fsReader)
			if err != nil {
				return nil, err
			}
			for i, f := range files {
				files[i] = filepath.Join(root, f) // rel -> abs
			}
			return files, nil
		}
	}
	// No wildcard found. Does the file exist at 'str'?
	if s, err := fsReader.Stat(str); err == nil && !s.IsDir() {
		return []string{str}, nil
	}
	return []string{}, nil
}

// Scan walks all files and subdirectories from root, returning those
// that Config.shouldExamine() returns true for.
func Scan(root string, cfg Config, fsReader oswrapper.FilesystemReader) ([]string, error) {
	files := []string{}
	err := fsReader.Walk(root, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		rel, err := filepath.Rel(root, path)
		if err != nil {
			rel = path
		}

		if rel == ".git" {
			return filepath.SkipDir
		}

		if !cfg.shouldExamine(root, path) {
			return nil
		}

		if !info.IsDir() {
			files = append(files, rel)
		}

		return nil
	})
	if err != nil {
		return nil, err
	}
	return files, nil
}

// Configs is a slice of Config.
type Configs []Config

// Config is used to parse the JSON configuration file.
type Config struct {
	// Paths holds a number of JSON objects that contain either a "includes" or
	// "excludes" key to an array of path patterns.
	// Each path pattern is considered in turn to either include or exclude the
	// file path for license scanning. Pattern use forward-slashes '/' for
	// directory separators, and may use the following wildcards:
	//  ?  - matches any single non-separator character
	//  *  - matches any sequence of non-separator characters
	//  ** - matches any sequence of characters including separators
	//
	// Rules are processed in the order in which they are declared, with later
	// rules taking precedence over earlier rules.
	//
	// All files are excluded before the first rule is evaluated.
	//
	// Example:
	//
	// {
	//   "paths": [
	// 	  { "exclude": [ "out/*", "build/*" ] },
	// 	  { "include": [ "out/foo.txt" ] }
	//   ],
	// }
	Paths searchRules
}

// LoadConfig loads a config file at path.
func LoadConfig(path string, fsReader oswrapper.FilesystemReader) (Config, error) {
	cfgBody, err := fsReader.ReadFile(path)
	if err != nil {
		return Config{}, err
	}
	return ParseConfig(string(cfgBody))
}

// ParseConfig parses the config from a JSON string.
func ParseConfig(config string) (Config, error) {
	d := json.NewDecoder(strings.NewReader(config))
	cfg := Config{}
	if err := d.Decode(&cfg); err != nil {
		return Config{}, err
	}
	return cfg, nil
}

// MustParseConfig parses the config from a JSON string, panicing if the config
// does not parse
func MustParseConfig(config string) Config {
	d := json.NewDecoder(strings.NewReader(config))
	cfg := Config{}
	if err := d.Decode(&cfg); err != nil {
		panic(fmt.Errorf("Failed to parse config: %w\nConfig:\n%v", err, config))
	}
	return cfg
}

// rule is a search path predicate.
// root is the project relative path.
// cond is the value to return if the rule doesn't either include or exclude.
type rule func(path string, cond bool) bool

// searchRules is a ordered list of search rules.
// searchRules is its own type as it has to perform custom JSON unmarshalling.
type searchRules []rule

// UnmarshalJSON unmarshals the array of rules in the form:
// { "include": [ ... ] } or { "exclude": [ ... ] }
func (l *searchRules) UnmarshalJSON(body []byte) error {
	type parsed struct {
		Include []string
		Exclude []string
	}

	p := []parsed{}
	if err := json.NewDecoder(bytes.NewReader(body)).Decode(&p); err != nil {
		return err
	}

	*l = searchRules{}
	for _, rule := range p {
		rule := rule
		switch {
		case len(rule.Include) > 0 && len(rule.Exclude) > 0:
			return fmt.Errorf("Rule cannot contain both include and exclude")
		case len(rule.Include) > 0:
			tests := make([]match.Test, len(rule.Include))
			for i, pattern := range rule.Include {
				test, err := match.New(pattern)
				if err != nil {
					return err
				}
				tests[i] = test
			}
			*l = append(*l, func(path string, cond bool) bool {
				if cond {
					return true
				}
				for _, test := range tests {
					if test(path) {
						return true
					}
				}
				return false
			})
		case len(rule.Exclude) > 0:
			tests := make([]match.Test, len(rule.Exclude))
			for i, pattern := range rule.Exclude {
				test, err := match.New(pattern)
				if err != nil {
					return err
				}
				tests[i] = test
			}
			*l = append(*l, func(path string, cond bool) bool {
				if !cond {
					return false
				}
				for _, test := range tests {
					if test(path) {
						return false
					}
				}
				return true
			})
		}
	}
	return nil
}

// shouldExamine returns true if the file at absPath should be scanned.
func (c Config) shouldExamine(root, absPath string) bool {
	root = filepath.ToSlash(root)       // Canonicalize
	absPath = filepath.ToSlash(absPath) // Canonicalize
	relPath, err := filepath.Rel(root, absPath)
	if err != nil {
		return false
	}
	relPath = filepath.ToSlash(relPath) // Canonicalize

	res := false
	for _, rule := range c.Paths {
		res = rule(relPath, res)
	}

	return res
}
