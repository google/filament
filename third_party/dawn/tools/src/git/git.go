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

// Package git provides helpers for interfacing with the git tool
package git

import (
	"context"
	"encoding/hex"
	"errors"
	"fmt"
	"net/url"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

// Hash is a 20 byte, git object hash.
type Hash [20]byte

func (h Hash) String() string { return hex.EncodeToString(h[:]) }

// IsZero returns true if the hash h is all zeros
func (h Hash) IsZero() bool {
	zero := Hash{}
	return h == zero
}

// ParseHash returns a Hash from a hexadecimal string.
func ParseHash(s string) (Hash, error) {
	b, err := hex.DecodeString(s)
	if err != nil {
		return Hash{}, fmt.Errorf("failed to parse hash '%v':\n  %w", s, err)
	}
	h := Hash{}
	copy(h[:], b)
	return h, nil
}

// The timeout for git operations if no other timeout is specified
var DefaultTimeout = 5 * time.Minute

// Git wraps the 'git' executable
type Git struct {
	// Path to the git executable
	exe string
	// Debug flag to print all command to the `git` executable
	LogAllActions bool
}

// New returns a new Git instance
func New(exe string) (*Git, error) {
	if exe == "" {
		g, err := exec.LookPath("git")
		if err != nil {
			return nil, fmt.Errorf("failed to find git: %v", err)
		}
		exe = g
	}
	if _, err := os.Stat(exe); err != nil {
		return nil, err
	}
	return &Git{exe: exe}, nil
}

// Credentials holds the user name and password used to perform git operations.
type Credentials struct {
	Username string
	Password string
}

// Empty return true if there's no username or password for authentication
func (a Credentials) Empty() bool {
	return a.Username == "" && a.Password == ""
}

// addToURL returns the url with the credentials appended
func (c Credentials) addToURL(u string) (string, error) {
	if !c.Empty() {
		modified, err := url.Parse(u)
		if err != nil {
			return "", fmt.Errorf("failed to parse url '%v': %v", u, err)
		}
		modified.User = url.UserPassword(c.Username, c.Password)
		u = modified.String()
	}
	return u, nil
}

// ErrRepositoryDoesNotExist indicates that a repository does not exist
var ErrRepositoryDoesNotExist = errors.New("repository does not exist")

// Open opens an existing git repo at path. If the repository does not exist at
// path then ErrRepositoryDoesNotExist is returned.
func (g Git) Open(path string) (*Repository, error) {
	info, err := os.Stat(filepath.Join(path, ".git"))
	if err != nil || !info.IsDir() {
		return nil, ErrRepositoryDoesNotExist
	}
	return &Repository{g, path}, nil
}

// Optional settings for Git.Clone
type CloneOptions struct {
	// If specified then the given branch will be cloned instead of the default
	Branch string
	// Timeout for the operation
	Timeout time.Duration
	// Authentication for the clone
	Credentials Credentials
}

// Clone performs a clone of the repository at url to path.
func (g Git) Clone(path, url string, opt *CloneOptions) (*Repository, error) {
	if err := os.MkdirAll(path, 0777); err != nil {
		return nil, err
	}
	if opt == nil {
		opt = &CloneOptions{}
	}
	url, err := opt.Credentials.addToURL(url)
	if err != nil {
		return nil, err
	}
	r := &Repository{g, path}
	args := []string{"clone", url, "."}
	if opt.Branch != "" {
		args = append(args, "--branch", opt.Branch)
	}
	if _, err := r.run(nil, opt.Timeout, args...); err != nil {
		return nil, err
	}
	return r, nil
}

// Repository points to a git repository
type Repository struct {
	// Path to the 'git' executable
	Git Git
	// Repo directory
	Path string
}

// Optional settings for Repository.Fetch
type FetchOptions struct {
	// The remote name. Defaults to 'origin'
	Remote string
	// Timeout for the operation
	Timeout time.Duration
	// Git authentication for the remote
	Credentials Credentials
}

// Fetch performs a fetch of a reference from the remote, returning the Hash of
// the fetched reference.
func (r Repository) Fetch(ref string, opt *FetchOptions) (Hash, error) {
	if opt == nil {
		opt = &FetchOptions{}
	}
	if opt.Remote == "" {
		opt.Remote = "origin"
	}
	if _, err := r.run(nil, opt.Timeout, "fetch", opt.Remote, ref); err != nil {
		return Hash{}, err
	}
	out, err := r.run(nil, 0, "rev-parse", "FETCH_HEAD")
	if err != nil {
		return Hash{}, err
	}
	return ParseHash(out)
}

// Optional settings for Repository.Push
type PushOptions struct {
	// The remote name. Defaults to 'origin'
	Remote string
	// Timeout for the operation
	Timeout time.Duration
	// Git authentication for the remote
	Credentials Credentials
}

// Push performs a push of the local reference to the remote reference.
func (r Repository) Push(localRef, remoteRef string, opt *PushOptions) error {
	if opt == nil {
		opt = &PushOptions{}
	}
	if opt.Remote == "" {
		opt.Remote = "origin"
	}
	url, err := r.run(nil, opt.Timeout, "remote", "get-url", opt.Remote)
	if err != nil {
		return err
	}
	url, err = opt.Credentials.addToURL(url)
	if err != nil {
		return err
	}
	if _, err := r.run(nil, opt.Timeout, "push", url, localRef+":"+remoteRef); err != nil {
		return err
	}
	return nil
}

// Optional settings for Repository.Add
type AddOptions struct {
	// Timeout for the operation
	Timeout time.Duration
	// Git authentication for the remote
	Credentials Credentials
}

// Add stages the listed files
func (r Repository) Add(path string, opt *AddOptions) error {
	if opt == nil {
		opt = &AddOptions{}
	}
	if _, err := r.run(nil, opt.Timeout, "add", path); err != nil {
		return err
	}
	return nil
}

// Optional settings for Repository.Commit
type CommitOptions struct {
	// Timeout for the operation
	Timeout time.Duration
	// Author name
	AuthorName string
	// Author email address
	AuthorEmail string
	// Amend last commit?
	Amend bool
}

// Commit commits the staged files with the given message, returning the hash of
// commit
func (r Repository) Commit(msg string, opt *CommitOptions) (Hash, error) {
	if opt == nil {
		opt = &CommitOptions{}
	}

	args := []string{"commit"}
	if opt.Amend {
		args = append(args, "--amend")
	} else {
		args = append(args, "-m", msg)
	}

	var env []string
	if opt.AuthorName != "" || opt.AuthorEmail != "" {
		env = []string{
			fmt.Sprintf("GIT_AUTHOR_NAME=%v", opt.AuthorName),
			fmt.Sprintf("GIT_AUTHOR_EMAIL=%v", opt.AuthorEmail),
			fmt.Sprintf("GIT_COMMITTER_NAME=%v", opt.AuthorName),
			fmt.Sprintf("GIT_COMMITTER_EMAIL=%v", opt.AuthorEmail),
		}
	}
	if _, err := r.run(env, opt.Timeout, "commit", "-m", msg); err != nil {
		return Hash{}, err
	}
	out, err := r.run(nil, 0, "rev-parse", "HEAD")
	if err != nil {
		return Hash{}, err
	}
	return ParseHash(out)
}

// Optional settings for Repository.Clean
type CleanOptions struct {
	// Timeout for the operation
	Timeout time.Duration
}

// Clean deletes all the untracked files in the repo
func (r Repository) Clean(opt *CleanOptions) error {
	if opt == nil {
		opt = &CleanOptions{}
	}
	if _, err := r.run(nil, opt.Timeout, "clean", "-fd"); err != nil {
		return err
	}
	return nil
}

// Optional settings for Repository.Checkout
type CheckoutOptions struct {
	// Timeout for the operation
	Timeout time.Duration
}

// Checkout performs a checkout of a reference.
func (r Repository) Checkout(ref string, opt *CheckoutOptions) error {
	if opt == nil {
		opt = &CheckoutOptions{}
	}
	if _, err := r.run(nil, opt.Timeout, "checkout", ref); err != nil {
		return err
	}
	return nil
}

// Optional settings for Repository.Log
type LogOptions struct {
	// The git reference to the oldest commit in the range to query.
	From string
	// The git reference to the newest commit in the range to query.
	To string
	// Timeout for the operation
	Timeout time.Duration
}

const logPrettyFormatArg = "--pretty=format:ǁ%Hǀ%cIǀ%an <%ae>ǀ%sǀ%b"

// Log returns the list of commits between two references (inclusive).
// The first returned commit is the most recent.
func (r Repository) Log(opt *LogOptions) ([]CommitInfo, error) {
	if opt == nil {
		opt = &LogOptions{}
	}
	args := []string{"log"}
	rng := "HEAD"
	if opt.To != "" {
		rng = opt.To
	}
	if opt.From != "" {
		rng = opt.From + "^.." + rng
	}
	args = append(args, rng, logPrettyFormatArg)
	out, err := r.run(nil, opt.Timeout, args...)
	if err != nil {
		return nil, err
	}
	return parseLog(out)
}

// Optional settings for Repository.LogBetween
type LogBetweenOptions struct {
	// Timeout for the operation
	Timeout time.Duration
}

// LogBetween returns the list of commits between two timestamps
// The first returned commit is the most recent.
func (r Repository) LogBetween(since, until time.Time, opt *LogBetweenOptions) ([]CommitInfo, error) {
	if opt == nil {
		opt = &LogBetweenOptions{}
	}
	args := []string{"log",
		"--since", since.Format(time.RFC3339),
		"--until", until.Format(time.RFC3339),
		logPrettyFormatArg,
	}
	out, err := r.run(nil, opt.Timeout, args...)
	if err != nil {
		return nil, err
	}
	return parseLog(out)
}

// FileStats describes the changes to a given file in a commit
type FileStats struct {
	Insertions int
	Deletions  int
}

// CommitStats is a map of file to FileStats
type CommitStats map[string]FileStats

// Optional settings for Repository.Stats
type StatsOptions struct {
	// Timeout for the operation
	Timeout time.Duration
}

// StatsOptions returns the statistics for a given change
func (r Repository) Stats(commit CommitInfo, opt *StatsOptions) (CommitStats, error) {
	if opt == nil {
		opt = &StatsOptions{}
	}

	hash := commit.Hash.String()
	args := []string{"diff", "--numstat", hash, hash + "^"}
	out, err := r.run(nil, opt.Timeout, args...)
	if err != nil {
		return nil, err
	}
	stats := CommitStats{}
	for _, line := range strings.Split(out, "\n") {
		if out == "" {
			continue
		}
		parts := strings.Split(line, "\t")
		if len(parts) != 3 {
			return nil, fmt.Errorf("failed to parse stat line: '%v'", line)
		}
		insertions, deletions := 0, 0
		if parts[0] != "-" {
			insertions, err = strconv.Atoi(parts[0])
			if err != nil {
				return nil, fmt.Errorf("failed to stat insertions '%v': %w", parts[0], err)
			}
		}
		if parts[1] != "-" {
			deletions, err = strconv.Atoi(parts[1])
			if err != nil {
				return nil, fmt.Errorf("failed to stat deletions '%v': %w", parts[1], err)
			}
		}
		file := parts[2]
		stats[file] = FileStats{Insertions: insertions, Deletions: deletions}
	}
	return stats, nil
}

// CommitInfo describes a single git commit
type CommitInfo struct {
	Hash        Hash
	Date        time.Time
	Author      string
	Subject     string
	Description string
}

// Optional settings for Repository.ConfigOptions
type ConfigOptions struct {
	// Timeout for the operation
	Timeout time.Duration
}

// Config returns the git configuration values for the repo
func (r Repository) Config(opt *ConfigOptions) (map[string]string, error) {
	if opt == nil {
		opt = &ConfigOptions{}
	}
	text, err := r.run(nil, opt.Timeout, "config", "-l")
	if err != nil {
		return nil, err
	}
	lines := strings.Split(text, "\n")
	out := make(map[string]string, len(lines))
	for _, line := range lines {
		idx := strings.Index(line, "=")
		if idx > 0 {
			key, value := line[:idx], line[idx+1:]
			out[key] = value
		}
	}
	return out, nil
}

func (r Repository) run(env []string, timeout time.Duration, args ...string) (string, error) {
	return r.Git.run(r.Path, env, timeout, args...)
}

func (g Git) run(dir string, env []string, timeout time.Duration, args ...string) (string, error) {
	if timeout == 0 {
		timeout = DefaultTimeout
	}
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	cmd := exec.CommandContext(ctx, g.exe, args...)
	cmd.Dir = dir
	if env != nil {
		// Godocs for exec.Cmd.Env:
		// "If Env contains duplicate environment keys, only the last value in
		// the slice for each duplicate key is used.
		cmd.Env = append(os.Environ(), env...)
	}
	if g.LogAllActions {
		fmt.Printf("%v> %v %v\n", dir, g.exe, strings.Join(args, " "))
	}
	out, err := cmd.CombinedOutput()
	if g.LogAllActions {
		fmt.Println(string(out))
	}
	if err != nil {
		msg := fmt.Sprintf("%v> %v %v failed:", dir, g.exe, strings.Join(args, " "))
		if err := ctx.Err(); err != nil {
			msg += "\n" + err.Error()
		}
		return string(out), fmt.Errorf("%s\n  %w\n%v", msg, err, string(out))
	}
	return strings.TrimSpace(string(out)), nil
}

func parseLog(str string) ([]CommitInfo, error) {
	msgs := strings.Split(str, "ǁ")
	cls := make([]CommitInfo, 0, len(msgs))
	for _, s := range msgs {
		if parts := strings.Split(s, "ǀ"); len(parts) == 5 {
			hash, err := ParseHash(parts[0])
			if err != nil {
				return nil, err
			}
			date, err := time.Parse(time.RFC3339, parts[1])
			if err != nil {
				return nil, err
			}
			cl := CommitInfo{
				Hash:        hash,
				Date:        date,
				Author:      strings.TrimSpace(parts[2]),
				Subject:     strings.TrimSpace(parts[3]),
				Description: strings.TrimSpace(parts[4]),
			}

			cls = append(cls, cl)
		}
	}
	return cls, nil
}
