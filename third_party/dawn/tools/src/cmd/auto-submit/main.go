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

// auto-submit applies the 'Commit-Queue+2' label to Gerrit changes authored by the user
// that are ready to be submitted
package main

import (
	"context"
	"flag"
	"fmt"
	"log"
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

const (
	toolName       = "auto-submit"
	cqEmailAccount = "dawn-scoped@luci-project-accounts.iam.gserviceaccount.com"
)

var (
	repoFlag    = flag.String("repo", "dawn", "the repo")
	userFlag    = flag.String("user", defaultUser(), "user name / email")
	verboseFlag = flag.Bool("v", false, "verbose mode")
	dryrunFlag  = flag.Bool("dry", false, "dry mode. Don't apply any labels")
	authFlags   = authcli.Flags{}
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

	flag.Usage = func() {
		out := flag.CommandLine.Output()
		fmt.Fprintf(out,
			`%v applies the 'Commit-Queue+2' label to Gerrit changes authored by the user that are ready to be submitted.

The tool monitors Gerrit changes authored by the user, looking for changes that have the labels
'Kokoro+1', 'Auto-Submit +1' and 'Code-Review +2' and applies the 'Commit-Queue +2' label.
`, toolName)
		fmt.Fprintf(out, "\n")
		flag.PrintDefaults()
	}
	flag.Parse()
	if err := run(); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

func run() error {
	user := *userFlag
	if user == "" {
		return fmt.Errorf("Missing required 'user' flag")
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

	app := app{Gerrit: g, user: user}

	log.Println("Monitoring for changes ready to be submitted...")

	for {
		err := app.submitReadyChanges()
		if err != nil {
			fmt.Println("error: ", err)
			time.Sleep(time.Minute * 10)
		}
		time.Sleep(time.Minute * 5)
	}
}

type app struct {
	*gerrit.Gerrit
	user string // User account of changes to submit
}

func (a *app) submitReadyChanges() error {
	if *verboseFlag {
		log.Println("Scanning for changes to submit...")
	}

	changes, _, err := a.QueryChangesWith(
		gerrit.QueryExtraData{
			Labels:           true,
			Messages:         true,
			CurrentRevision:  true,
			DetailedAccounts: true,
			Submittable:      true,
		},
		"status:open",
		"author:"+a.user,
		"-is:wip",
		"label:auto-submit",
		"label:kokoro",
		"repo:"+*repoFlag)
	if err != nil {
		return fmt.Errorf("failed to query changes: %w", err)
	}

	for _, change := range changes {
		// Returns true if the change has the label with the given value
		hasLabel := func(name string, value int) bool {
			if label, ok := change.Labels[name]; ok {
				for _, vote := range label.All {
					if vote.Value == value {
						return true
					}
				}
			}
			return false
		}

		isReadyToSubmit := true &&
			change.Submittable &&
			hasLabel("Kokoro", 1) &&
			hasLabel("Auto-Submit", 1) &&
			hasLabel("Code-Review", 2) &&
			!hasLabel("Code-Review", -1) &&
			!hasLabel("Code-Review", -2)
		if !isReadyToSubmit {
			// Change is not ready to be submitted
			continue
		}

		if hasLabel("Commit-Queue", 2) {
			// Change already in the process of submitting
			continue
		}

		submittedTogether, err := a.ChangesSubmittedTogether(change.ChangeID)
		if err != nil {
			return fmt.Errorf("failed to query changes submitted together: %w", err)
		}
		if len(submittedTogether) > 1 { // Include the change itself
			// Change has unsubmitted parents
			if *verboseFlag {
				log.Printf("%v has %v unsubmitted parents", change.ChangeID, len(submittedTogether)-1)
			}
			continue
		}

		switch parseCQStatus(change) {
		case cqUnknown, cqPassed:
			if *dryrunFlag {
				log.Printf("Would submit %v: %v... (--dry)\n", change.ChangeID, change.Subject)
				continue
			}

			log.Printf("Submitting %v: %v...\n", change.ChangeID, change.Subject)
			err := a.AddLabel(change.ChangeID, change.CurrentRevision, "Auto submitting change", "Commit-Queue", 2)
			if err != nil {
				return fmt.Errorf("failed to set Commit-Queue label: %w", err)
			}

		case cqFailed:
			if *verboseFlag {
				log.Printf("Change failed CQ: %v: %v...\n", change.ChangeID, change.Subject)
			}
		}
	}

	return nil
}

// CQ result status enumerator
type cqStatus int

// CQ result status enumerator values
const (
	cqUnknown cqStatus = iota
	cqPassed
	cqFailed
)

// Attempt to parse the CQ result from the latest patchset's messages from CQ
func parseCQStatus(change gerrit.ChangeInfo) cqStatus {
	currentPatchset := change.Revisions[change.CurrentRevision].Number
	for _, msg := range change.Messages {
		if msg.RevisionNumber != currentPatchset {
			continue
		}
		if msg.Author.Email == cqEmailAccount {
			if strings.Contains(msg.Message, "This CL has failed the run") {
				return cqFailed
			}
			if strings.Contains(msg.Message, "This CL has passed the run") {
				return cqPassed
			}
		}
	}
	return cqUnknown
}
