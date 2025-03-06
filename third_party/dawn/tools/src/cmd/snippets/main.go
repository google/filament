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

// snippets gathers information about changes merged for weekly reports (snippets).
package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/auth"
	"dawn.googlesource.com/dawn/tools/src/dawn"
	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"dawn.googlesource.com/dawn/tools/src/git"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"go.chromium.org/luci/auth/client/authcli"
)

const yyyymmdd = "2006-01-02"

var (
	userFlag   = flag.String("user", defaultUser(), "user name / email")
	afterFlag  = flag.String("after", "", "start date")
	beforeFlag = flag.String("before", "", "end date")
	daysFlag   = flag.Int("days", 7, "interval in days (used if --after is not specified)")
	authFlags  = authcli.Flags{}
)

func defaultUser() string {
	if gitExe, err := exec.LookPath("git"); err == nil {
		if g, err := git.New(gitExe); err == nil {
			if cwd, err := os.Getwd(); err == nil {
				if r, err := g.Open(cwd); err == nil {
					if cfg, err := r.Config(nil); err == nil {
						return cfg["user.email"]
					}
				}
			}
		}
	}
	return ""
}

func main() {
	authFlags.Register(flag.CommandLine, auth.DefaultAuthOptions(oswrapper.GetRealOSWrapper()))

	flag.Parse()
	if err := run(); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

func run() error {
	var after, before time.Time
	var err error
	user := *userFlag
	if user == "" {
		return fmt.Errorf("Missing required 'user' flag")
	}
	if *beforeFlag != "" {
		before, err = time.Parse(yyyymmdd, *beforeFlag)
		if err != nil {
			return fmt.Errorf("Couldn't parse before date: %w", err)
		}
	} else {
		before = time.Now().Add(time.Hour * 24)
	}
	if *afterFlag != "" {
		after, err = time.Parse(yyyymmdd, *afterFlag)
		if err != nil {
			return fmt.Errorf("Couldn't parse after date: %w", err)
		}
	} else {
		after = before.Add(-time.Hour * time.Duration(24**daysFlag))
	}

	ctx := context.Background()
	auth, err := authFlags.Options()
	if err != nil {
		return err
	}

	g, err := gerrit.New(ctx, auth, dawn.GerritURL)
	if err != nil {
		return err
	}

	submitted, _, err := g.QueryChanges(
		"status:merged",
		"owner:"+user,
		"after:"+date(after),
		"before:"+date(before))
	if err != nil {
		return fmt.Errorf("Query failed: %w", err)
	}

	changesByProject := map[string][]string{}
	for _, change := range submitted {
		str := fmt.Sprintf(`* [%s](%sc/%s/+/%d)`, change.Subject, dawn.GerritURL, change.Project, change.Number)
		changesByProject[change.Project] = append(changesByProject[change.Project], str)
	}

	for _, project := range []string{"tint", "dawn"} {
		if changes := changesByProject[project]; len(changes) > 0 {
			fmt.Println("##", strings.Title(project))
			for i := len(changes) - 1; i >= 0; i-- {
				fmt.Println(changes[i])
			}
			fmt.Println()
		}
	}

	return nil
}

func today() time.Time {
	return time.Now()
}

func date(t time.Time) string {
	return t.Format(yyyymmdd)
}
