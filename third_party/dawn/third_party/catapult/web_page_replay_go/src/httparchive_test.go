// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"testing"

	"github.com/urfave/cli/v2"
)

func TestFlags(t *testing.T) {
	cfg := &Config{}
	baseFlags := []string{"command", "host", "full_path", "status_code", "decode_response_body"}
	addFlags := []string{"skip-existing", "overwrite-existing"}
	trimFlags := append([]string{"invert-match"}, baseFlags...)

	cases := map[string]struct {
		command   string
		flags     []cli.Flag
		wantFlags []string
	}{
		"ls": {
			command:   "ls",
			flags:     cfg.DefaultFlags(),
			wantFlags: baseFlags,
		},
		"cat": {
			command:   "cat",
			flags:     cfg.DefaultFlags(),
			wantFlags: baseFlags,
		},
		"edit": {
			command:   "edit",
			flags:     cfg.DefaultFlags(),
			wantFlags: baseFlags,
		},
		"merge": {
			command:   "merge",
			flags:     []cli.Flag{},
			wantFlags: []string{},
		},
		"add": {
			command:   "add",
			flags:     cfg.AddFlags(),
			wantFlags: addFlags,
		},
		"addAll": {
			command:   "addAll",
			flags:     cfg.AddFlags(),
			wantFlags: addFlags,
		},
		"trim": {
			command:   "trim",
			flags:     cfg.TrimFlags(),
			wantFlags: trimFlags,
		},
	}

	for name, tt := range cases {
		t.Run(name, func(t *testing.T) {
			if len(tt.wantFlags) != len(tt.flags) {
				t.Fatalf("Incorrect '%s' flags returned, wanted:%d, actual:%d", name, len(tt.wantFlags), len(tt.flags))
			}
			for i, f := range tt.flags {
				actualFlagName := f.Names()[0]
				t.Logf("%s[%d] = %s", name, i, actualFlagName)
				if actualFlagName != tt.wantFlags[i] {
					t.Fatalf("Incorrect flag for '%s' in position %d. wanted:%s, actual:%s", name, i, tt.wantFlags[i], actualFlagName)
				}
			}
		})
	}
}
