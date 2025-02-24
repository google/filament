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

// Package git provides functions for interacting with Git.
package git

import (
	"encoding/hex"
	"fmt"
	"io/ioutil"
	"net/url"
	"os"
	"os/exec"
	"strings"
	"time"

	"swiftshader.googlesource.com/SwiftShader/tests/regres/shell"
)

const (
	gitTimeout = time.Minute * 15 // timeout for a git operation
)

var exe string

func init() {
	path, err := exec.LookPath("git")
	if err != nil {
		panic(fmt.Errorf("failed to find path to git executable: %w", err))
	}
	exe = path
}

// Hash is a 20 byte, git object hash.
type Hash [20]byte

func (h Hash) String() string { return hex.EncodeToString(h[:]) }

// ParseHash returns a Hash from a hexadecimal string.
func ParseHash(s string) Hash {
	b, _ := hex.DecodeString(s)
	h := Hash{}
	copy(h[:], b)
	return h
}

// Add calls 'git add <file>'.
func Add(wd, file string) error {
	if err := shell.Shell(gitTimeout, exe, wd, "add", file); err != nil {
		return fmt.Errorf("`git add %v` in working directory %v failed: %w", file, wd, err)
	}
	return nil
}

// CommitFlags advanced flags for Commit
type CommitFlags struct {
	Name  string // Used for author and committer
	Email string // Used for author and committer
}

// Commit calls 'git commit -m <msg> --author <author>'.
func Commit(wd, msg string, flags CommitFlags) error {
	args := []string{}
	if flags.Name != "" {
		args = append(args, "-c", "user.name="+flags.Name)
	}
	if flags.Email != "" {
		args = append(args, "-c", "user.email="+flags.Email)
	}
	args = append(args, "commit", "-m", msg)
	return shell.Shell(gitTimeout, exe, wd, args...)
}

func InjectUserInHostUrl(userEmail string, url string) string {
	if userEmail == "" {
		return url
	}

	user := strings.Replace(userEmail, "@", "%40", 1)
	return strings.Replace(url, "://", "://"+user+"@", 1)
}

// PushFlags advanced flags for Commit
type PushFlags struct {
	Username string // Used for authentication when uploading
	Password string // Used for authentication when uploading
}

// Push pushes the local branch to remote.
func Push(wd, remote, localBranch, remoteBranch string, flags PushFlags) error {
	args := []string{}
	if flags.Username != "" {
		f, err := ioutil.TempFile("", "regres-cookies.txt")
		if err != nil {
			return fmt.Errorf("failed to create cookie file: %w", err)
		}
		defer f.Close()
		defer os.Remove(f.Name())
		u, err := url.Parse(remote)
		if err != nil {
			return fmt.Errorf("failed to parse url '%v': %w", remote, err)
		}
		f.WriteString(fmt.Sprintf("%v	FALSE	/	TRUE	2147483647	o	%v=%v\n", u.Host, flags.Username, flags.Password))
		f.Close()
		args = append(args, "-c", "http.cookiefile="+f.Name())

		remote = InjectUserInHostUrl(flags.Username, remote)
	}
	args = append(args, "push", remote, localBranch+":"+remoteBranch)
	return shell.Shell(gitTimeout, exe, wd, args...)
}

// CheckoutRemoteBranch performs a git fetch and checkout of the given branch into path.
func CheckoutRemoteBranch(path, url string, branch string, flags CommitFlags) error {
	if err := os.MkdirAll(path, 0777); err != nil {
		return fmt.Errorf("mkdir '"+path+"' failed: %w", err)
	}

	url = InjectUserInHostUrl(flags.Email, url)

	for _, cmds := range [][]string{
		{"init"},
		{"config", "user.name", flags.Name},
		{"config", "user.email", flags.Email},
		{"remote", "add", "origin", url},
		{"fetch", "origin", "--depth=1", branch},
		{"checkout", branch},
	} {
		if err := shell.Shell(gitTimeout, exe, path, cmds...); err != nil {
			os.RemoveAll(path)
			return err
		}
	}

	return nil
}

// CheckoutRemoteCommit performs a git fetch and checkout of the given commit into path.
func CheckoutRemoteCommit(path, url string, commit Hash, flags CommitFlags) error {
	if err := os.MkdirAll(path, 0777); err != nil {
		return fmt.Errorf("mkdir '"+path+"' failed: %w", err)
	}

	url = InjectUserInHostUrl(flags.Email, url)

	for _, cmds := range [][]string{
		{"init"},
		{"config", "user.name", flags.Name},
		{"config", "user.email", flags.Email},
		{"remote", "add", "origin", url},
		{"fetch", "origin", "--depth=1", commit.String()},
		{"checkout", commit.String()},
	} {
		if err := shell.Shell(gitTimeout, exe, path, cmds...); err != nil {
			os.RemoveAll(path)
			return err
		}
	}

	return nil
}

// CheckoutCommit performs a git checkout of the given commit.
func CheckoutCommit(path string, commit Hash) error {
	return shell.Shell(gitTimeout, exe, path, "checkout", commit.String())
}

// Apply applys the patch file to the git repo at dir.
func Apply(dir, patch string) error {
	return shell.Shell(gitTimeout, exe, dir, "apply", patch)
}

// FetchRefHash returns the git hash of the given ref.
func FetchRefHash(ref, url string, userEmail string) (Hash, error) {
	url = InjectUserInHostUrl(userEmail, url)
	out, err := shell.Exec(gitTimeout, exe, "", nil, "", "ls-remote", url, ref)
	if err != nil {
		return Hash{}, err
	}
	return ParseHash(string(out)), nil
}

type ChangeList struct {
	Hash        Hash
	Date        time.Time
	Author      string
	Subject     string
	Description string
}

// Log returns the top count ChangeLists at HEAD.
func Log(path string, count int) ([]ChangeList, error) {
	return LogFrom(path, "HEAD", count)
}

// LogFrom returns the top count ChangeList starting from at.
func LogFrom(path, at string, count int) ([]ChangeList, error) {
	if at == "" {
		at = "HEAD"
	}
	out, err := shell.Exec(gitTimeout, exe, "", nil, "", "log", at, "--pretty=format:"+prettyFormat, fmt.Sprintf("-%d", count), path)
	if err != nil {
		return nil, err
	}
	return parseLog(string(out)), nil
}

// Parent returns the parent ChangeList for cl.
func Parent(cl ChangeList) (ChangeList, error) {
	out, err := shell.Exec(gitTimeout, exe, "", nil, "", "log", "--pretty=format:"+prettyFormat, fmt.Sprintf("%v^", cl.Hash))
	if err != nil {
		return ChangeList{}, err
	}
	cls := parseLog(string(out))
	if len(cls) == 0 {
		return ChangeList{}, fmt.Errorf("Unexpected output")
	}
	return cls[0], nil
}

// HeadCL returns the HEAD ChangeList at the given commit/tag/branch.
func HeadCL(path string) (ChangeList, error) {
	cls, err := LogFrom(path, "HEAD", 1)
	if err != nil {
		return ChangeList{}, err
	}
	if len(cls) == 0 {
		return ChangeList{}, fmt.Errorf("No commits found")
	}
	return cls[0], nil
}

// Show content of the file at path for the given commit/tag/branch.
func Show(path, at string) ([]byte, error) {
	return shell.Exec(gitTimeout, exe, "", nil, "", "show", at+":"+path)
}

const prettyFormat = "ǁ%Hǀ%cIǀ%an <%ae>ǀ%sǀ%b"

func parseLog(str string) []ChangeList {
	msgs := strings.Split(str, "ǁ")
	cls := make([]ChangeList, 0, len(msgs))
	for _, s := range msgs {
		if parts := strings.Split(s, "ǀ"); len(parts) == 5 {
			cl := ChangeList{
				Hash:        ParseHash(parts[0]),
				Author:      strings.TrimSpace(parts[2]),
				Subject:     strings.TrimSpace(parts[3]),
				Description: strings.TrimSpace(parts[4]),
			}
			date, err := time.Parse(time.RFC3339, parts[1])
			if err != nil {
				panic(err)
			}
			cl.Date = date

			cls = append(cls, cl)
		}
	}
	return cls
}
