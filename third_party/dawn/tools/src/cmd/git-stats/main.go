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

// git-stats gathers statistics about changes made to a git repo.
package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
	"regexp"
	"runtime"
	"sort"
	"strings"
	"sync"
	"text/tabwriter"
	"time"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/git"
)

// Flags
var (
	repo       = flag.String("repo", ".", "path to git directory")
	afterFlag  = flag.String("after", "", "start date")
	beforeFlag = flag.String("before", "", "end date")
	daysFlag   = flag.Int("days", 182, "interval in days (used if --after is not specified)")
)

// main entry point
func main() {
	flag.Parse()
	if err := run(); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

// Date format strings
const (
	yyyymmdd = "2006-01-02"
	yyyymm   = "2006-01"
)

// Returns true if the file with the given path should be included for addition / deletion stats.
func shouldConsiderLinesOfFile(path string) bool {
	for _, ignore := range []string{
		"Doxyfile",
		"package-lock.json",
		"src/tint/builtin_table.inl",
		"src/tint/lang/core/intrinsic/table.inl",
		"src/tint/lang/core/*.cc",
		"src/tint/lang/core/*.h",
		"test/tint/",
		"third_party/gn/webgpu-cts/test_list.txt",
		"third_party/khronos/",
		"webgpu-cts/",
		"src/external/petamoriken",
	} {
		if strings.HasPrefix(path, ignore) {
			return false
		}
	}
	return true
}

// Returns true if the commit with the given hash should be included for addition / deletion stats.
func shouldConsiderLinesOfCommit(hash string) bool {
	for _, ignore := range []string{
		"41e4d9a34c1d9dcb2eef3ff39ff9c1f987bfa02a", // Consistent formatting for Dawn/Tint.
		"e87ac76f7ddf9237f3022cda90224bd0691fb318", // Merge tint -> dawn
		"b0acbd436dbd499505a3fa8bf89e69231ec4d1e0", // Fix build/namespaces issues
	} {
		if hash == ignore {
			return false
		}
	}
	return true
}

// Regular expression used to parse the email from an author string. Example:
// Bob Bobson <bob@bobmail.com>
// ____________^^^^^^^^^^^^^^^_
var reEmail = regexp.MustCompile(`<([^>]+)>`)

func run() error {
	// Parse the --after and --before flags
	var after, before time.Time
	var err error
	if *beforeFlag != "" {
		before, err = time.Parse(yyyymmdd, *beforeFlag)
		if err != nil {
			return fmt.Errorf("Couldn't parse before date: %w", err)
		}
	} else {
		before = time.Now()
	}
	if *afterFlag != "" {
		after, err = time.Parse(yyyymmdd, *afterFlag)
		if err != nil {
			return fmt.Errorf("Couldn't parse after date: %w", err)
		}
	} else {
		after = before.Add(-time.Hour * time.Duration(24**daysFlag))
	}

	// Find 'git'
	gitExe, err := exec.LookPath("git")
	if err != nil {
		return err
	}

	// Create the git.Git wrapper
	g, err := git.New(gitExe)
	if err != nil {
		return err
	}

	// Open the repo
	r, err := g.Open(*repo)
	if err != nil {
		return err
	}

	// Information obtained about a single commit
	type CommitStat struct {
		author     string
		commit     *git.CommitInfo
		insertions int
		deletions  int
		fileDeltas container.Map[string, int]
	}

	// Kick a goroutine to gather all the commits in the git log between
	// 'after' and 'before', streaming the commits to the 'commits' chan.
	// This chan will be closed by the goroutine when all commits have been
	// gathered.
	commits := make(chan git.CommitInfo, 256)
	go func() {
		log, err := r.LogBetween(after, before, &git.LogBetweenOptions{})
		if err != nil {
			panic(fmt.Errorf("failed to gather commits: %w", err))
		}
		for _, commit := range log {
			commits <- commit
		}
		close(commits)
	}()

	// Kick 'numWorkers' goroutines to gather the commit statistics of the
	// commits in the 'commits' chan, streaming the commit statistics to the
	// 'commitStats' chan.
	commitStats := make(chan CommitStat, 256)
	numWorkers := runtime.NumCPU()
	wg := sync.WaitGroup{}
	wg.Add(numWorkers)
	for worker := 0; worker < numWorkers; worker++ {
		go func() {
			defer wg.Done()
			for commit := range commits {
				commit := commit
				email := reEmail.FindStringSubmatch(commit.Author)[1]
				stats, err := r.Stats(commit, nil)
				if err != nil {
					panic(fmt.Errorf("failed to get stats for commit '%v': %w", commit.Hash, err))
				}

				s := CommitStat{
					author:     email,
					commit:     &commit,
					fileDeltas: container.NewMap[string, int](),
				}
				if shouldConsiderLinesOfCommit(commit.Hash.String()) {
					for file, stats := range stats {
						if shouldConsiderLinesOfFile(file) {
							s.insertions += stats.Insertions
							s.deletions += stats.Deletions
							s.fileDeltas[file] = stats.Insertions + stats.Deletions
						}
					}
				}
				commitStats <- s
			}
		}()
	}

	// Kick a helper goroutine that waits for all the goroutines that feed the
	// 'commitStats' chan to complete, and then closes the 'commitStats' chan.
	go func() {
		wg.Wait()
		close(commitStats)
	}()

	// CommitDelta holds the sum of line additions and deletions for a given
	// commit.
	type CommitDelta struct {
		commit *git.CommitInfo
		delta  int
	}

	// Stream in the commit statistics from the 'commitStats' chan, and collect
	// statistics by author and by file.
	statsByAuthor := container.NewMap[string, AuthorStats]()
	fileDeltas := container.NewMap[string, int]()
	commitDeltas := []CommitDelta{}
	for cs := range commitStats {
		as := statsByAuthor[cs.author]
		as.insertions += cs.insertions
		as.deletions += cs.deletions
		as.commits++
		if as.commitsByMonth == nil {
			as.commitsByMonth = container.NewMap[string, int]()
		}
		month := cs.commit.Date.Format(yyyymm)
		as.commitsByMonth[month] = as.commitsByMonth[month] + 1
		statsByAuthor[cs.author] = as

		commitDelta := 0
		for path, delta := range cs.fileDeltas {
			fileDeltas[path] = fileDeltas[path] + delta
			commitDelta += delta
		}
		commitDeltas = append(commitDeltas, CommitDelta{cs.commit, commitDelta})
	}

	// Transform the 'statsByAuthor' map, so that authors that have statistics
	// for both a @google.com and @chromium.org account have all their
	// statistics merged into the @google.com account.
	for google, g := range statsByAuthor {
		if strings.HasSuffix(google, "@google.com") {
			combined := strings.TrimSuffix(google, "@google.com")
			chromium := combined + "@chromium.org"
			if c, hasChromium := statsByAuthor[chromium]; hasChromium {
				statsByAuthor[google] = combine(g, c)
				delete(statsByAuthor, chromium)
			}
		}
	}

	// Print those stats!

	fmt.Printf("Between %v and %v:\n", after, before)

	// Print the top 10 most modified files.
	// This is helpful to identify files that are automatically generated, which
	// we should exclude from the statistics.
	{
		type FileDelta struct {
			file  string
			delta int
		}
		l := make([]FileDelta, 0, len(fileDeltas))
		for file, delta := range fileDeltas {
			l = append(l, FileDelta{file, delta})
		}
		sort.Slice(l, func(i, j int) bool { return l[i].delta > l[j].delta })
		n := len(l)
		if n > 10 {
			n = 10
		}
		fmt.Println()
		fmt.Printf("Top %v most modified files:\n", n)
		fmt.Println()
		tw := tabwriter.NewWriter(os.Stdout, 0, 0, 0, ' ', 0)
		fmt.Fprintln(tw, "  delta\t | file")
		for _, fd := range l[:n] {
			fmt.Fprintln(tw,
				" ", fd.delta,
				"\t |", fd.file)
		}
		tw.Flush()
	}

	// Print the top 10 largest commits.
	// This is helpful to identify commits that may contain a large bulk
	// refactor, which we should exclude from the statistics.
	{
		sort.Slice(commitDeltas, func(i, j int) bool {
			return commitDeltas[i].delta > commitDeltas[j].delta
		})
		n := len(commitDeltas)
		if n > 10 {
			n = 10
		}
		fmt.Println()
		fmt.Printf("Top %v largest commits:\n", n)
		fmt.Println()
		tw := tabwriter.NewWriter(os.Stdout, 0, 0, 0, ' ', 0)
		fmt.Fprintln(tw,
			"  delta\t | author\t | hash\t | description")
		for _, fd := range commitDeltas[:n] {
			fmt.Fprintln(tw,
				" ", fd.delta,
				"\t |", fd.commit.Author,
				"\t |", fd.commit.Hash.String()[:6],
				"\t |", fd.commit.Subject)
		}
		tw.Flush()
	}

	// Print the contributions by author.
	{
		fmt.Println()
		fmt.Println("Total contributions by author:")
		tw := tabwriter.NewWriter(os.Stdout, 0, 0, 0, ' ', 0)
		fmt.Println()
		fmt.Fprintln(tw, "  author\t | commits\t | added\t | removed")
		for _, author := range statsByAuthor.Keys() {
			s := statsByAuthor[author]
			fmt.Fprintln(tw,
				"  "+author,
				"\t |", s.commits,
				"\t |", s.insertions,
				"\t |", s.deletions)
		}
		tw.Flush()
	}

	// Print the per-author contributions by month.
	{
		allMonths := container.NewSet[string]()
		for _, author := range statsByAuthor {
			for month := range author.commitsByMonth {
				allMonths.Add(month)
			}
		}

		months := allMonths.List()

		fmt.Println()
		fmt.Println("Commits by author by month:")
		tw := tabwriter.NewWriter(os.Stdout, 0, 0, 0, ' ', 0)
		fmt.Println()
		fmt.Fprintf(tw, "  author")
		for _, month := range months {
			fmt.Fprint(tw, "\t | ", month)
		}
		fmt.Fprintln(tw)

		for _, author := range statsByAuthor.Keys() {
			fmt.Fprint(tw, "  ", author)
			cbm := statsByAuthor[author].commitsByMonth
			for _, month := range months {
				fmt.Fprint(tw, "\t | ", cbm[month])
			}
			fmt.Fprintln(tw)
		}
		tw.Flush()
	}

	return nil
}

type AuthorStats struct {
	commits        int
	commitsByMonth container.Map[string, int]
	insertions     int
	deletions      int
}

// combine returns a new AuthorStats, with the summed statistics of 'a' and 'b'.
func combine(a, b AuthorStats) AuthorStats {
	out := AuthorStats{
		commits:    a.commits + b.commits,
		insertions: a.insertions + b.insertions,
		deletions:  a.deletions + b.deletions,
	}
	out.commitsByMonth = container.NewMap[string, int]()
	for month, commits := range a.commitsByMonth {
		out.commitsByMonth[month] = commits
	}
	for month, commits := range b.commitsByMonth {
		out.commitsByMonth[month] = out.commitsByMonth[month] + commits
	}
	return out
}

func today() time.Time {
	return time.Now()
}

func date(t time.Time) string {
	return t.Format(yyyymmdd)
}
