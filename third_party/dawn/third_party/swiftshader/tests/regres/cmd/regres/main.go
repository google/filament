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

// regres is a tool that detects test regressions with SwiftShader changes.
//
// Regres monitors changes that have been put up for review with Gerrit.
// Once a new patchset has been found, regres will checkout, build and test the
// change against the parent changelist. Any differences in results are reported
// as a review comment on the change.
//
// Once a day regres will also test another, larger set of tests, and post the
// full test results as a Gerrit changelist. The CI test lists can be based from
// this daily test list, so testing can be limited to tests that were known to
// pass.
package main

import (
	"bytes"
	"crypto/sha1"
	"encoding/hex"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io/fs"
	"io/ioutil"
	"log"
	"math"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strings"
	"time"

	"swiftshader.googlesource.com/SwiftShader/tests/regres/consts"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/cov"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/deqp"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/git"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/llvm"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/shell"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/testlist"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/util"

	gerrit "github.com/andygrunwald/go-gerrit"
)

const (
	gitURL                = "https://swiftshader.googlesource.com/SwiftShader"
	gitDailyBranch        = "HEAD"
	gerritURL             = "https://swiftshader-review.googlesource.com/"
	gerritUserName        = "SwiftShader Regression Bot"
	coverageURL           = "https://$USERNAME:$PASSWORD@github.com/swiftshader-regres/swiftshader-coverage.git"
	coverageBranch        = "gh-pages"
	coveragePath          = "coverage/coverage.zip"
	reportHeader          = "Regres report:"
	changeUpdateFrequency = time.Minute * 5
	changeQueryFrequency  = time.Minute * 5
	testTimeout           = time.Minute * 2  // timeout for a single test
	buildTimeout          = time.Minute * 10 // timeout for a build
	fullTestListRelPath   = "tests/regres/full-tests.json"
	ciTestListRelPath     = "tests/regres/ci-tests.json"
	deqpConfigRelPath     = "tests/regres/deqp.json"
	swsTestLists          = "tests/regres/testlists"
	deqpTestLists         = "external/vulkancts/mustpass/main"
)

var (
	numParallelTests = runtime.NumCPU()
	llvmVersion      = llvm.Version{Major: 17, Point: 6}

	cacheDir        = flag.String("cache", "cache", "path to the output cache directory")
	gerritEmail     = flag.String("email", "$SS_REGRES_EMAIL", "gerrit email address for posting regres results")
	gerritUser      = flag.String("user", "$SS_REGRES_USER", "gerrit username for posting regres results")
	gerritPass      = flag.String("pass", "$SS_REGRES_PASS", "gerrit password for posting regres results")
	githubUser      = flag.String("gh-user", "$SS_GITHUB_USER", "github user for posting coverage results")
	githubPass      = flag.String("gh-pass", "$SS_GITHUB_PASS", "github password for posting coverage results")
	keepCheckouts   = flag.Bool("keep", false, "don't delete checkout directories after use")
	dryRun          = flag.Bool("dry", false, "don't post regres reports to gerrit")
	maxTestsPerProc = flag.Int("max-tests-per-proc", 100, "maximum number of tests running in a single process")
	maxProcMemory   = flag.Uint64("max-proc-mem", shell.MaxProcMemory, "maximum virtual memory per child process")
	dailyNow        = flag.Bool("dailynow", false, "Start by running the daily pass")
	dailyOnly       = flag.Bool("dailyonly", false, "Run only the daily pass")
	dailyChange     = flag.String("dailychange", "", "Change hash to use for daily pass, HEAD if not provided")
	priority        = flag.Int("priority", 0, "Prioritize a single change with the given number")
	limit           = flag.Int("limit", 0, "only run a maximum of this number of tests")
)

func main() {
	flag.ErrHelp = errors.New("regres is a tool to detect regressions between versions of SwiftShader")
	flag.Parse()

	shell.MaxProcMemory = *maxProcMemory

	r := regres{
		cacheRoot:     *cacheDir,
		gerritEmail:   os.ExpandEnv(*gerritEmail),
		gerritUser:    os.ExpandEnv(*gerritUser),
		gerritPass:    os.ExpandEnv(*gerritPass),
		githubUser:    os.ExpandEnv(*githubUser),
		githubPass:    os.ExpandEnv(*githubPass),
		keepCheckouts: *keepCheckouts,
		dryRun:        *dryRun,
		dailyNow:      *dailyNow,
		dailyOnly:     *dailyOnly,
		dailyChange:   *dailyChange,
		priority:      *priority,
	}

	if err := r.run(); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(-1)
	}
}

type regres struct {
	cmake         string          // path to cmake executable
	make          string          // path to make executable
	python        string          // path to python executable
	tar           string          // path to tar executable
	cacheRoot     string          // path to the regres cache directory
	toolchain     *llvm.Toolchain // the LLVM toolchain used to build SwiftShader
	gerritEmail   string          // gerrit email address used for posting results
	gerritUser    string          // gerrit username used for posting results
	gerritPass    string          // gerrit password used for posting results
	githubUser    string          // github username used for posting results
	githubPass    string          // github password used for posting results
	keepCheckouts bool            // don't delete source & build checkouts after testing
	dryRun        bool            // don't post any reviews
	maxProcMemory uint64          // max virtual memory for child processes
	dailyNow      bool            // start with a daily run
	dailyOnly     bool            // run only the daily run
	dailyChange   string          // Change hash to use for daily pass, HEAD if not provided
	priority      int             // Prioritize a single change with the given number
}

// getToolchain returns the LLVM toolchain, possibly downloading and
// decompressing it if it wasn't found in the cache directory.
func getToolchain(tarExe, cacheRoot string) (*llvm.Toolchain, error) {
	path := filepath.Join(cacheRoot, "llvm")

	if toolchain := llvm.Search(path).Find(llvmVersion); toolchain != nil {
		return toolchain, nil
	}

	// LLVM toolchain may have been updated, remove the directory if it exists.
	os.RemoveAll(path)

	log.Printf("Downloading LLVM %v toolchain...\n", llvmVersion)
	tar, err := llvmVersion.Download()
	if err != nil {
		return nil, fmt.Errorf("failed to download LLVM %v: %w", llvmVersion, err)
	}

	tarFile := filepath.Join(cacheRoot, "llvm.tar.xz")
	if err := ioutil.WriteFile(tarFile, tar, 0666); err != nil {
		return nil, fmt.Errorf("failed to write '%v': %w", tarFile, err)
	}
	defer os.Remove(tarFile)

	log.Printf("Decompressing LLVM %v toolchain...\n", llvmVersion)
	target := filepath.Join(cacheRoot, "llvm-tmp")
	os.MkdirAll(target, 0755)
	defer os.RemoveAll(target)
	if err := exec.Command(tarExe, "-xf", tarFile, "-C", target).Run(); err != nil {
		return nil, fmt.Errorf("failed to decompress LLVM tar download: %w", err)
	}

	// The tar, once decompressed, holds a single root directory with a name
	// starting with 'clang+llvm'. Move this to path.
	files, err := filepath.Glob(filepath.Join(target, "*"))
	if err != nil {
		return nil, fmt.Errorf("failed to glob decompressed files: %w", err)
	}
	if len(files) != 1 || !util.IsDir(files[0]) {
		return nil, fmt.Errorf("Unexpected decompressed files: %+v", files)
	}
	if err := os.Rename(files[0], path); err != nil {
		return nil, fmt.Errorf("failed to move %v to %v: %w", files[0], path, err)
	}

	// We should now have everything in the right place.
	toolchain := llvm.Search(path).Find(llvmVersion)
	if toolchain == nil {
		return nil, fmt.Errorf("failed to find LLVM toolchain after downloading")
	}

	return toolchain, nil
}

// toolchainEnv() returns the environment variables for executing CMake commands.
func (r *regres) toolchainEnv() []string {
	return append([]string{
		"CC=" + r.toolchain.Clang(),
		"CXX=" + r.toolchain.ClangXX(),
	}, os.Environ()...)
}

// resolveDirs ensures that the necessary directories used can be found, and
// expands them to absolute paths.
func (r *regres) resolveDirs() error {
	allDirs := []*string{
		&r.cacheRoot,
	}

	for _, path := range allDirs {
		abs, err := filepath.Abs(*path)
		if err != nil {
			return fmt.Errorf("failed to find path '%v': %w", *path, err)
		}
		*path = abs
	}

	if err := os.MkdirAll(r.cacheRoot, 0777); err != nil {
		return fmt.Errorf("failed to create cache root directory: %w", err)
	}

	for _, path := range allDirs {
		if !util.IsDir(*path) {
			return fmt.Errorf("failed to find path '%v'", *path)
		}
	}

	return nil
}

// resolveExes resolves all external executables used by regres.
func (r *regres) resolveExes() error {
	type exe struct {
		name string
		path *string
	}
	for _, e := range []exe{
		{"cmake", &r.cmake},
		{"make", &r.make},
		{"python", &r.python},
		{"tar", &r.tar},
	} {
		path, err := exec.LookPath(e.name)
		if err != nil {
			return fmt.Errorf("failed to find path to %s: %w", e.name, err)
		}
		*e.path = path
	}
	return nil
}

// run performs the main processing loop for the regress tool. It:
//   - Scans for open and recently updated changes in gerrit using queryChanges()
//     and changeInfo.update().
//   - Builds the most recent patchset and the commit's parent CL using
//     r.newTest(<hash>).lazyRun().
//   - Compares the results of the tests using compare().
//   - Posts the results of the compare to gerrit as a review.
//   - Repeats the above steps until the process is interrupted.
func (r *regres) run() error {
	if err := r.resolveExes(); err != nil {
		return fmt.Errorf("failed to resolve all exes: %w", err)
	}

	if err := r.resolveDirs(); err != nil {
		return fmt.Errorf("failed to resolve all directories: %w", err)
	}

	toolchain, err := getToolchain(r.tar, r.cacheRoot)
	if err != nil {
		return fmt.Errorf("failed to download LLVM toolchain: %w", err)
	}
	r.toolchain = toolchain

	client, err := gerrit.NewClient(gerritURL, nil)
	if err != nil {
		return fmt.Errorf("failed to create gerrit client: %w", err)
	}
	if r.gerritUser != "" {
		client.Authentication.SetBasicAuth(r.gerritUser, r.gerritPass)
	}

	changes := map[int]*changeInfo{} // Change number -> changeInfo
	lastUpdatedTestLists := toDate(time.Now())
	lastQueriedChanges := time.Time{}

	if r.dailyNow || r.dailyOnly {
		lastUpdatedTestLists = date{}
	}

	for {
		if now := time.Now(); toDate(now) != lastUpdatedTestLists {
			lastUpdatedTestLists = toDate(now)
			if err := r.runDaily(client, backendLLVM); err != nil {
				log.Println(err.Error())
			}
			if err := r.runDaily(client, backendSubzero); err != nil {
				log.Println(err.Error())
			}
		}

		if r.dailyOnly {
			log.Println("Daily finished with --dailyonly. Stopping")
			return nil
		}

		// Update list of tracked changes.
		if time.Since(lastQueriedChanges) > changeQueryFrequency {
			lastQueriedChanges = time.Now()
			if err := queryChanges(client, changes); err != nil {
				log.Println(err.Error())
			}
		}

		// Update change info.
		for _, change := range changes {
			if time.Since(change.lastUpdated) > changeUpdateFrequency {
				change.lastUpdated = time.Now()
				err := change.update(client)
				if err != nil {
					log.Println(fmt.Errorf("failed to update info for change '%v': %w", change.number, err))
				}
			}
		}

		for _, c := range changes {
			if c.pending && r.priority == c.number {
				log.Printf("Prioritizing change '%v'\n", c.number)
				c.priority = 1e6
			}
		}

		// Find the change with the highest priority.
		var change *changeInfo
		numPending := 0
		for _, c := range changes {
			if c.pending {
				numPending++
				if change == nil || c.priority > change.priority {
					change = c
				}
			}
		}

		if change == nil {
			// Everything up to date. Take a break.
			log.Println("Nothing to do. Sleeping")
			time.Sleep(time.Minute)
			continue
		}

		log.Printf("%d changes queued for testing\n", numPending)

		log.Printf("Testing change '%v'\n", change.number)

		// Test the latest patchset in the change, diff against parent change.
		msg, alert, err := r.test(change)
		if err != nil {
			log.Println(fmt.Errorf("failed to test changelist '%s': %w", change.latest, err))
			time.Sleep(time.Minute)
			change.pending = false
			continue
		}

		// Always include the reportHeader in the message.
		// changeInfo.update() uses this header to detect whether a patchset has
		// already got a test result.
		msg = reportHeader + "\n\n" + msg

		// Limit the message length to prevent '400 Bad Request' response.
		maxMsgLength := 16000
		if len(msg) > maxMsgLength {
			trunc := " [truncated]\n"
			msg = msg[0:maxMsgLength-len(trunc)] + trunc
		}

		if r.dryRun {
			log.Printf("DRY RUN: add review to change '%v':\n%v\n", change.number, msg)
		} else {
			log.Printf("Posting review to '%v'\n", change.number)
			notify := "OWNER"
			if alert {
				notify = "OWNER_REVIEWERS"
			}
			_, _, err = client.Changes.SetReview(fmt.Sprintf("%v", change.number), change.latest.String(), &gerrit.ReviewInput{
				Message: msg,
				Tag:     "autogenerated:regress",
				Notify:  notify,
			})
			if err != nil {
				return fmt.Errorf("failed to post comments on change '%v': %w", change.number, err)
			}
		}
		change.pending = false
	}
}

func (r *regres) test(change *changeInfo) (string, bool, error) {
	latest := r.newTest(change.latest)
	defer latest.cleanup()

	if err := latest.checkout(); err != nil {
		return "", true, fmt.Errorf("failed to checkout '%s': %w", change.latest, err)
	}

	deqpBuild, err := r.getOrBuildDEQP(latest)
	if err != nil {
		return "", true, fmt.Errorf("failed to build dEQP '%v' for change: %w", change.number, err)
	}

	log.Printf("Testing latest patchset for change '%v'\n", change.number)
	latestResults, testlists, err := r.testLatest(change, latest, deqpBuild)
	if err != nil {
		return "", true, fmt.Errorf("failed to test latest change of '%v': %w", change.number, err)
	}

	log.Printf("Testing parent of change '%v'\n", change.number)
	parentResults, err := r.testParent(change, testlists, deqpBuild)
	if err != nil {
		return "", true, fmt.Errorf("failed to test parent change of '%v': %w", change.number, err)
	}

	log.Println("Comparing latest patchset's results with parent")
	msg, alert := compare(parentResults, latestResults)

	return msg, alert, nil
}

type deqpBuild struct {
	path string // path to deqp directory
	hash string // hash of the deqp config
}

// DeqpConfig holds the JSON payload of the deqp.json file
type DeqpConfig struct {
	Remote  string   `json:"remote"`
	Branch  string   `json:"branch"`
	SHA     string   `json:"sha"`
	Patches []string `json:"patches"`
}

func loadConfigFromFile(deqpConfigFile string) (DeqpConfig, error) {
	file, err := os.Open(deqpConfigFile)
	if err != nil {
		return DeqpConfig{}, fmt.Errorf("failed to open dEQP config file: %w", err)
	}
	defer file.Close()

	cfg := DeqpConfig{}
	if err := json.NewDecoder(file).Decode(&cfg); err != nil {
		return DeqpConfig{}, fmt.Errorf("failed to parse %s: %w", deqpConfigRelPath, err)
	}

	return cfg, nil
}

func (r *regres) getOrBuildDEQP(test *test) (deqpBuild, error) {
	checkoutDir := test.checkoutDir
	if p := path.Join(checkoutDir, deqpConfigRelPath); !util.IsFile(p) {
		checkoutDir, _ = os.Getwd()
		log.Printf("failed to open dEQP config file from change (%v), falling back to internal version\n", p)
	} else {
		log.Println("Using dEQP config file from change")
	}

	cfg, err := loadConfigFromFile(path.Join(checkoutDir, deqpConfigRelPath))
	if err != nil {
		return deqpBuild{}, fmt.Errorf("failed to load config file: %w", err)
	}

	return r.getOrBuildDEQPFromConfig(test, cfg, checkoutDir)
}

func (r *regres) getOrBuildDEQPFromConfig(test *test, cfg DeqpConfig, checkoutDir string) (deqpBuild, error) {
	hasher := sha1.New()
	if err := json.NewEncoder(hasher).Encode(&cfg); err != nil {
		return deqpBuild{}, fmt.Errorf("failed to re-encode %s: %w", deqpConfigRelPath, err)
	}
	hash := hex.EncodeToString(hasher.Sum(nil))
	cacheDir := path.Join(r.cacheRoot, "deqp", hash)
	buildDir := path.Join(cacheDir, "build")
	if !util.IsDir(cacheDir) {
		if err := os.MkdirAll(cacheDir, 0777); err != nil {
			return deqpBuild{}, fmt.Errorf("failed to make deqp cache directory '%s': %w", cacheDir, err)
		}

		success := false
		defer func() {
			if !success {
				os.RemoveAll(cacheDir)
			}
		}()

		if cfg.Branch != "" {
			// If a branch is specified, then fetch the branch then checkout the
			// commit by SHA. This is a workaround for git repos that error when
			// attempting to directly checkout a remote commit.
			log.Printf("Checking out deqp %v branch %v into %v\n", cfg.Remote, cfg.Branch, cacheDir)
			if err := git.CheckoutRemoteBranch(cacheDir, cfg.Remote, cfg.Branch, git.CommitFlags{}); err != nil {
				return deqpBuild{}, fmt.Errorf("failed to checkout deqp branch %v @ %v: %w", cfg.Remote, cfg.Branch, err)
			}
			log.Printf("Checking out deqp %v commit %v \n", cfg.Remote, cfg.SHA)
			if err := git.CheckoutCommit(cacheDir, git.ParseHash(cfg.SHA)); err != nil {
				return deqpBuild{}, fmt.Errorf("failed to checkout deqp commit %v @ %v: %w", cfg.Remote, cfg.SHA, err)
			}
		} else {
			log.Printf("Checking out deqp %v @ %v into %v\n", cfg.Remote, cfg.SHA, cacheDir)
			if err := git.CheckoutRemoteCommit(cacheDir, cfg.Remote, git.ParseHash(cfg.SHA), git.CommitFlags{}); err != nil {
				return deqpBuild{}, fmt.Errorf("failed to checkout deqp commit %v @ %v: %w", cfg.Remote, cfg.SHA, err)
			}
		}

		log.Println("Fetching deqp dependencies")
		if err := shell.Shell(buildTimeout, r.python, cacheDir, "external/fetch_sources.py"); err != nil {
			return deqpBuild{}, fmt.Errorf("failed to fetch deqp sources %v @ %v: %w", cfg.Remote, cfg.SHA, err)
		}

		log.Println("Applying deqp patches")
		for _, patch := range cfg.Patches {
			fullPath := path.Join(checkoutDir, patch)
			if err := git.Apply(cacheDir, fullPath); err != nil {
				return deqpBuild{}, fmt.Errorf("failed to apply deqp patch %v for %v @ %v: %w", patch, cfg.Remote, cfg.SHA, err)
			}
		}

		log.Printf("Building deqp into %v\n", buildDir)
		if err := os.MkdirAll(buildDir, 0777); err != nil {
			return deqpBuild{}, fmt.Errorf("failed to make deqp build directory '%v': %w", buildDir, err)
		}

		if err := shell.Shell(buildTimeout, r.cmake, buildDir,
			"-DDEQP_TARGET=default",
			"-DCMAKE_BUILD_TYPE=Release",
			".."); err != nil {
			return deqpBuild{}, fmt.Errorf("failed to generate build rules for deqp %v @ %v: %w", cfg.Remote, cfg.SHA, err)
		}

		if err := shell.Shell(buildTimeout, r.make, buildDir,
			fmt.Sprintf("-j%d", runtime.NumCPU()),
			"deqp-vk"); err != nil {
			return deqpBuild{}, fmt.Errorf("failed to build deqp %v @ %v: %w", cfg.Remote, cfg.SHA, err)
		}

		success = true
	}

	return deqpBuild{
		path: cacheDir,
		hash: hash,
	}, nil
}

var additionalTestsRE = regexp.MustCompile(`\n\s*Test[s]?:\s*([^\s]+)[^\n]*`)

func (r *regres) testLatest(change *changeInfo, test *test, d deqpBuild) (*deqp.Results, testlist.Lists, error) {
	// Get the test results for the latest patchset in the change.
	testlists, err := test.loadTestLists(ciTestListRelPath)
	if err != nil {
		return nil, nil, fmt.Errorf("failed to load '%s': %w", change.latest, err)
	}

	if matches := additionalTestsRE.FindAllStringSubmatch(change.commitMessage, -1); len(matches) > 0 {
		log.Println("Change description contains additional test patterns")

		// Change specifies additional tests to try. Load the full test list.
		fullTestLists, err := test.loadTestLists(fullTestListRelPath)
		if err != nil {
			return nil, nil, fmt.Errorf("failed to load '%s': %w", change.latest, err)
		}

		// Add any tests in the full list that match the pattern to the list to test.
		for _, match := range matches {
			if len(match) > 1 {
				pattern := match[1]
				log.Printf("Adding custom tests with pattern '%s'\n", pattern)
				filtered := fullTestLists.Filter(func(name string) bool {
					ok, _ := filepath.Match(pattern, name)
					return ok
				})
				testlists = append(testlists, filtered...)
			}
		}
	}

	cachePath := test.resultsCachePath(testlists, d)

	if results, err := deqp.LoadResults(cachePath); err == nil {
		return results, testlists, nil // Use cached results
	}

	// Build the change and test it.
	results := test.buildAndRun(testlists, d)

	// Cache the results for future tests
	if err := results.Save(cachePath); err != nil {
		log.Printf("Warning: Couldn't save results of test to '%v'\n", cachePath)
	}

	return results, testlists, nil
}

func (r *regres) testParent(change *changeInfo, testlists testlist.Lists, d deqpBuild) (*deqp.Results, error) {
	// Get the test results for the changes's parent changelist.
	test := r.newTest(change.parent)
	defer test.cleanup()

	cachePath := test.resultsCachePath(testlists, d)

	if results, err := deqp.LoadResults(cachePath); err == nil {
		return results, nil // Use cached results
	}

	// Couldn't load cached results. Have to build them.
	if err := test.checkout(); err != nil {
		return nil, fmt.Errorf("failed to checkout '%s': %w", change.parent, err)
	}

	// Build the parent change and test it.
	results := test.buildAndRun(testlists, d)

	// Store the results of the parent change to the cache.
	if err := results.Save(cachePath); err != nil {
		log.Printf("Warning: Couldn't save results of test to '%v'\n", cachePath)
	}

	return results, nil
}

// runDaily runs a full deqp run on the HEAD change, posting the results to a
// new or existing gerrit change.
func (r *regres) runDaily(client *gerrit.Client, reactorBackend reactorBackend) error {
	log.Printf("Updating test lists (Backend: %v)\n", reactorBackend)

	var dailyHash git.Hash
	if r.dailyChange == "" {
		headHash, err := git.FetchRefHash(gitDailyBranch, gitURL, r.gerritEmail)
		if err != nil {
			return fmt.Errorf("failed to get hash of master HEAD: %w", err)
		}
		dailyHash = headHash
	} else {
		dailyHash = git.ParseHash(r.dailyChange)
	}

	// Get the full test results.
	test := r.newTest(dailyHash).setReactorBackend(reactorBackend)
	defer test.cleanup()

	// Always need to checkout the change.
	if err := test.checkout(); err != nil {
		return fmt.Errorf("failed to checkout '%s': %w", dailyHash, err)
	}

	// Update dEQP to latest
	newPaths, err := r.updateLocalDeqpFiles(test)
	if err != nil {
		return fmt.Errorf("failed to update test lists from dEQP: %w", err)
	}

	d, err := r.getOrBuildDEQP(test)
	if err != nil {
		return fmt.Errorf("failed to build deqp for '%s': %w", dailyHash, err)
	}

	// Load the test lists.
	testLists, err := test.loadTestLists(fullTestListRelPath)
	if err != nil {
		return fmt.Errorf("failed to load full test lists for '%s': %w", dailyHash, err)
	}

	// Build the change.
	if err := test.build(); err != nil {
		return fmt.Errorf("failed to build '%s': %w", dailyHash, err)
	}

	// Run the tests on the change.
	results, err := test.run(testLists, d)
	if err != nil {
		return fmt.Errorf("failed to test '%s': %w", dailyHash, err)
	}

	if err := r.postDailyResults(client, test, testLists, results, reactorBackend, dailyHash, newPaths); err != nil {
		msg := strings.Builder{}
		msg.WriteString(err.Error() + "\n")
		return fmt.Errorf("%s", msg.String())
	}

	return nil
}

// copyFileIfDifferent copies src to dst if src doesn't exist or if there are differences
// between the files.
func copyFileIfDifferent(dst, src string) error {
	srcFileInfo, err := os.Stat(src)
	if err != nil {
		return err
	}
	srcContents, err := os.ReadFile(src)
	if err != nil {
		return err
	}

	dstContents, err := os.ReadFile(dst)
	if err != nil && !errors.Is(err, os.ErrNotExist) {
		return err
	}

	if !bytes.Equal(srcContents, dstContents) {
		if err := os.MkdirAll(path.Dir(dst), 0777); err != nil {
			return err
		}
		if err := os.WriteFile(dst, srcContents, srcFileInfo.Mode()); err != nil {
			return err
		}
	}
	return nil
}

// deleteFileIfNotPresent deletes a file if the corresponding file doesn't exist
func deleteFileIfNotPresent(toDeleteFile, checkFile string) error {
	if _, err := os.Stat(checkFile); errors.Is(err, os.ErrNotExist) {
		return os.Remove(toDeleteFile)
	}

	return nil
}

// updateLocalDeqpFiles sets the SHA in deqp.json to the latest dEQP revision,
// then it uses getOrBuildDEQP to checkout that revision and copy over its testlists
func (r *regres) updateLocalDeqpFiles(test *test) ([]string, error) {
	out := []string{}
	// Update deqp.json
	deqpJsonPath := path.Join(test.checkoutDir, deqpConfigRelPath)
	if !util.IsFile(deqpJsonPath) {
		return nil, fmt.Errorf("Failed to locate %s while trying to update the dEQP SHA", deqpConfigRelPath)
	}
	cfg, err := loadConfigFromFile(deqpJsonPath)
	if err != nil {
		return nil, fmt.Errorf("failed to open dEQP config file: %w", err)
	}

	hash, err := git.FetchRefHash("HEAD", cfg.Remote, "")
	if err != nil {
		return nil, fmt.Errorf("failed to fetch dEQP ref: %w", err)
	}
	cfg.SHA = hash.String()
	log.Println("New dEQP revision: ", cfg.SHA)

	newFile, err := os.Create(deqpJsonPath)
	if err != nil {
		return nil, fmt.Errorf("failed to open %s for encoding: %w", deqpConfigRelPath, err)
	}
	defer newFile.Close()

	encoder := json.NewEncoder(newFile)
	// Make the encoder create a new-line and space-based indents for each field
	encoder.SetIndent("", "    ")
	if err := encoder.Encode(&cfg); err != nil {
		return nil, fmt.Errorf("failed to re-encode %s: %w", deqpConfigRelPath, err)
	}
	out = append(out, deqpJsonPath)

	// Use getOrBuildDEQPFromConfig as it'll prevent us from copying data from a revision of dEQP that has build errors.
	deqpBuild, err := r.getOrBuildDEQPFromConfig(test, cfg, test.checkoutDir)
	if err != nil {
		return nil, fmt.Errorf("failed to retrieve dEQP build information: %w", err)
	}

	log.Println("Copying deqp's vulkan testlist to checkout ", test.commit)
	deqpTestlistDir := path.Join(deqpBuild.path, deqpTestLists)
	swsTestlistDir := path.Join(test.checkoutDir, swsTestLists)

	deqpDefault := path.Join(deqpTestlistDir, "vk-default.txt")
	swsDefault := path.Join(swsTestlistDir, "vk-master.txt")

	if err := copyFileIfDifferent(swsDefault, deqpDefault); err != nil {
		return nil, fmt.Errorf("failed to copy '%s' to '%s': %w", deqpDefault, swsDefault, err)
	}

	out = append(out, swsDefault)

	deqpTestlistVkDefaultDir := path.Join(deqpTestlistDir, "vk-default")
	swsTestlistVkDefaultDir := path.Join(swsTestlistDir, "vk-default")

	// First, copy over any existing dEQP file and add new dEQP files
	err = filepath.WalkDir(deqpTestlistVkDefaultDir,
		func(deqpFile string, d fs.DirEntry, err error) error {
			if d.IsDir() || err != nil {
				return err
			}

			relPath, err := filepath.Rel(deqpTestlistVkDefaultDir, deqpFile)
			if err != nil {
				return err
			}

			swsFile := path.Join(swsTestlistVkDefaultDir, relPath)

			if err := copyFileIfDifferent(swsFile, deqpFile); err != nil {
				return fmt.Errorf("failed to copy '%s' to '%s': %w", deqpFile, swsFile, err)
			}
			out = append(out, swsFile)

			return nil
		})
	if err != nil {
		return nil, fmt.Errorf("failed to read files from %s: %w", deqpTestlistVkDefaultDir, err)
	}

	// Second, delete files which no longer exist in dEQP
	err = filepath.WalkDir(swsTestlistVkDefaultDir,
		func(swsFile string, d fs.DirEntry, err error) error {
			if d.IsDir() || err != nil {
				return err
			}

			relPath, err := filepath.Rel(swsTestlistVkDefaultDir, swsFile)
			if err != nil {
				return err
			}

			deqpFile := path.Join(deqpTestlistVkDefaultDir, relPath)

			if err := deleteFileIfNotPresent(swsFile, deqpFile); err != nil {
				return fmt.Errorf("failed to delete '%s': %w", swsFile, err)
			}
			out = append(out, swsFile)

			return nil
		})
	if err != nil {
		return nil, fmt.Errorf("failed to read files from %s: %w", swsTestlistVkDefaultDir, err)
	}

	return out, nil
}

// postDailyResults posts the results of the daily full deqp run to gerrit as
// a new change, or reusing an old, unsubmitted change.
// This change contains the updated test lists, an updated deqp.json that
// points to the latest dEQP commit, and updated dEQP test files, along with a
// summary of the test results.
func (r *regres) postDailyResults(
	client *gerrit.Client,
	test *test,
	testLists testlist.Lists,
	results *deqp.Results,
	reactorBackend reactorBackend,
	dailyHash git.Hash,
	newPaths []string) error {

	// Write out the test list status files.
	filePaths, err := test.writeTestListsByStatus(testLists, results)
	if err != nil {
		return fmt.Errorf("failed to write test lists by status: %w", err)
	}

	filePaths = append(filePaths, newPaths...)

	// Stage all the updated test files.
	for _, path := range filePaths {
		log.Println("Staging", path)
		if err := git.Add(test.checkoutDir, path); err != nil {
			return err
		}
	}

	log.Println("Checking for existing test list")
	existingChange, err := r.findTestListChange(client)
	if err != nil {
		return err
	}

	commitMsg := strings.Builder{}
	commitMsg.WriteString(consts.TestListUpdateCommitSubjectPrefix + dailyHash.String()[:8])
	commitMsg.WriteString("\n\nReactor backend: " + string(reactorBackend))
	if existingChange != nil {
		// Reuse gerrit change ID if there's already a change up for review.
		commitMsg.WriteString("\n\n")
		commitMsg.WriteString("Change-Id: " + existingChange.ChangeID + "\n")
	}

	if err := git.Commit(test.checkoutDir, commitMsg.String(), git.CommitFlags{
		Name:  gerritUserName,
		Email: r.gerritEmail,
	}); err != nil {
		return fmt.Errorf("failed to commit test results: %w", err)
	}

	if r.dryRun {
		log.Printf("DRY RUN: post results for review")
	} else {
		log.Println("Pushing test results for review")
		if err := git.Push(test.checkoutDir, gitURL, "HEAD", "refs/for/master", git.PushFlags{
			Username: r.gerritUser,
			Password: r.gerritPass,
		}); err != nil {
			return fmt.Errorf("failed to push test results for review: %w", err)
		}
		log.Println("Test results posted for review")
	}

	// We've just pushed a new commit. Let's reset back to the parent commit
	// (dailyHash), so that we can run runDaily again for another backend,
	// and have it update the commit with the same change-id.
	if err := git.CheckoutCommit(test.checkoutDir, dailyHash); err != nil {
		return fmt.Errorf("failed to checkout parent commit: %w", err)
	}
	log.Println("Checked out parent commit")

	change, err := r.findTestListChange(client)
	if err != nil {
		return err
	}

	if err := r.postMostCommonFailures(client, change, results); err != nil {
		return err
	}

	return nil
}

func (r *regres) postCoverageResults(cov *cov.Tree, revision git.Hash) error {
	log.Printf("Committing coverage for %v\n", revision.String())

	url := coverageURL
	url = strings.ReplaceAll(url, "$USERNAME", r.githubUser)
	url = strings.ReplaceAll(url, "$PASSWORD", r.githubPass)

	dir := filepath.Join(r.cacheRoot, "coverage")
	defer os.RemoveAll(dir)
	if err := git.CheckoutRemoteBranch(dir, url, coverageBranch, git.CommitFlags{}); err != nil {
		return fmt.Errorf("failed to checkout gh-pages branch: %w", err)
	}

	filePath := filepath.Join(dir, "coverage.dat")
	file, err := os.Create(filePath)
	if err != nil {
		return fmt.Errorf("failed to create file '%s': %w", filePath, err)
	}
	defer file.Close()

	if err := cov.Encode(revision.String(), file); err != nil {
		return fmt.Errorf("failed to encode coverage: %w", err)
	}
	file.Close()

	if err := git.Add(dir, filePath); err != nil {
		return fmt.Errorf("failed to git add '%s': %w", filePath, err)
	}

	shortHash := revision.String()[:8]

	err = git.Commit(dir, "Update coverage data @ "+shortHash, git.CommitFlags{
		Name:  gerritUserName,
		Email: r.gerritEmail,
	})
	if err != nil {
		return fmt.Errorf("failed to git commit: %w", err)
	}

	if !r.dryRun {
		err = git.Push(dir, url, coverageBranch, coverageBranch, git.PushFlags{})
		if err != nil {
			return fmt.Errorf("failed to 'git push': %w", err)
		}
		log.Printf("Coverage for %v pushed to Github\n", shortHash)
	}

	return nil
}

// postMostCommonFailures posts the most common failure cases as a review
// comment on the given change.
func (r *regres) postMostCommonFailures(client *gerrit.Client, change *gerrit.ChangeInfo, results *deqp.Results) error {
	const limit = 25

	failures := commonFailures(results)
	if len(failures) > limit {
		failures = failures[:limit]
	}
	sb := strings.Builder{}
	sb.WriteString(fmt.Sprintf("Top %v most common failures:\n", len(failures)))
	for _, f := range failures {
		lines := strings.Split(f.error, "\n")
		if len(lines) == 1 {
			line := lines[0]
			if line != "" {
				sb.WriteString(fmt.Sprintf(" • %d occurrences: %v: %v\n", f.count, f.status, line))
			} else {
				sb.WriteString(fmt.Sprintf(" • %d occurrences: %v\n", f.count, f.status))
			}
		} else {
			sb.WriteString(fmt.Sprintf(" • %d occurrences: %v:\n", f.count, f.status))
			for _, l := range lines {
				sb.WriteString("    > ")
				sb.WriteString(l)
				sb.WriteString("\n")
			}
		}
		sb.WriteString(fmt.Sprintf("    Example test: %v\n", f.exampleTest))

	}
	msg := sb.String()

	if r.dryRun {
		log.Printf("DRY RUN: add most common failures: %v\n", msg)
	} else {
		log.Printf("Posting most common failures to '%v'\n", change.Number)
		_, _, err := client.Changes.SetReview(fmt.Sprintf("%v", change.Number), change.CurrentRevision, &gerrit.ReviewInput{
			Message: msg,
			Tag:     "autogenerated:regress",
		})
		if err != nil {
			return fmt.Errorf("failed to post comments on change '%v': %w", change.Number, err)
		}
	}
	return nil
}

func (r *regres) findTestListChange(client *gerrit.Client) (*gerrit.ChangeInfo, error) {
	log.Println("Checking for existing test list change")
	changes, _, err := client.Changes.QueryChanges(&gerrit.QueryChangeOptions{
		QueryOptions: gerrit.QueryOptions{
			Query: []string{fmt.Sprintf(`status:open+owner:"%v"`, r.gerritEmail)},
			Limit: 1,
		},
		ChangeOptions: gerrit.ChangeOptions{
			AdditionalFields: []string{"CURRENT_REVISION"},
		},
	})
	if err != nil {
		return nil, fmt.Errorf("failed to checking for existing test list: %w", err)
	}
	if len(*changes) > 0 {
		// TODO: This currently assumes that only change changes from
		// gerritEmail are test lists updates. This may not always be true.
		return &(*changes)[0], nil
	}
	return nil, nil
}

// changeInfo holds the important information about a single, open change in
// gerrit.
type changeInfo struct {
	pending       bool      // Is this change waiting a test for the latest patchset?
	priority      int       // Calculated priority based on Gerrit labels.
	latest        git.Hash  // Git hash of the latest patchset in the change.
	parent        git.Hash  // Git hash of the changelist this change is based on.
	lastUpdated   time.Time // Time the change was last fetched.
	number        int       // The number gerrit assigned to the change
	commitMessage string
}

// queryChanges updates the changes map by querying gerrit for the latest open
// changes.
func queryChanges(client *gerrit.Client, changes map[int]*changeInfo) error {
	log.Println("Checking for latest changes")
	results, _, err := client.Changes.QueryChanges(&gerrit.QueryChangeOptions{
		QueryOptions: gerrit.QueryOptions{
			Query: []string{"status:open+-age:3d"},
			Limit: 100,
		},
	})
	if err != nil {
		return fmt.Errorf("failed to get list of changes: %w", err)
	}

	ids := map[int]bool{}
	for _, r := range *results {
		ids[r.Number] = true
	}

	// Add new changes
	for number := range ids {
		if _, found := changes[number]; !found {
			log.Printf("Tracking new change '%v'\n", number)
			changes[number] = &changeInfo{number: number}
		}
	}

	// Remove old changes
	for number := range changes {
		if _, found := ids[number]; !found {
			log.Printf("Untracking change '%v'\n", number)
			delete(changes, number)
		}
	}

	return nil
}

// update queries gerrit for information about the given change.
func (c *changeInfo) update(client *gerrit.Client) error {
	change, _, err := client.Changes.GetChange(fmt.Sprintf("%v", c.number), &gerrit.ChangeOptions{
		AdditionalFields: []string{"CURRENT_REVISION", "CURRENT_COMMIT", "MESSAGES", "LABELS", "DETAILED_ACCOUNTS"},
	})
	if err != nil {
		return fmt.Errorf("failed to get info for change %v: %w", c.number, err)
	}

	current, ok := change.Revisions[change.CurrentRevision]
	if !ok {
		return fmt.Errorf("failed to find current revision for change %v", c.number)
	}

	if len(current.Commit.Parents) == 0 {
		return fmt.Errorf("failed to find current commit for change %v has no parents(?)", c.number)
	}

	kokoroPresubmit := change.Labels["Kokoro-Presubmit"].Approved.AccountID != 0
	codeReviewScore := change.Labels["Code-Review"].Value
	codeReviewApproved := change.Labels["Code-Review"].Approved.AccountID != 0
	presubmitReady := change.Labels["Presubmit-Ready"].Approved.AccountID != 0
	verifiedScore := change.Labels["Verified"].Value

	c.priority = 0
	if presubmitReady {
		c.priority += 10
	}
	c.priority += codeReviewScore
	if codeReviewApproved {
		c.priority += 2
	}
	if kokoroPresubmit {
		c.priority++
	}

	// Is the change from a Googler or reviewed by a Googler?
	canTest := strings.HasSuffix(current.Commit.Committer.Email, "@google.com") ||
		strings.HasSuffix(change.Labels["Code-Review"].Approved.Email, "@google.com") ||
		strings.HasSuffix(change.Labels["Code-Review"].Recommended.Email, "@google.com") ||
		strings.HasSuffix(change.Labels["Presubmit-Ready"].Approved.Email, "@google.com")

	// Don't test if the change has negative scores.
	if canTest {
		if codeReviewScore < 0 || verifiedScore < 0 {
			canTest = false
		}
	}

	// Has the latest patchset already been tested?
	if canTest {
		for _, msg := range change.Messages {
			if msg.RevisionNumber == current.Number &&
				strings.Contains(msg.Message, reportHeader) {
				canTest = false
				break
			}
		}
	}

	c.pending = canTest
	c.latest = git.ParseHash(change.CurrentRevision)
	c.parent = git.ParseHash(current.Commit.Parents[0].Commit)
	c.commitMessage = current.Commit.Message

	return nil
}

func (r *regres) newTest(commit git.Hash) *test {
	checkoutDir := filepath.Join(r.cacheRoot, "checkout", commit.String())
	resDir := filepath.Join(r.cacheRoot, "res", commit.String())
	return &test{
		r:              r,
		commit:         commit,
		checkoutDir:    checkoutDir,
		resDir:         resDir,
		buildDir:       filepath.Join(checkoutDir, "build"),
		reactorBackend: backendSubzero,
	}
}

func (t *test) setReactorBackend(reactorBackend reactorBackend) *test {
	t.reactorBackend = reactorBackend
	return t
}

type reactorBackend string

const (
	backendLLVM    reactorBackend = "LLVM"
	backendSubzero reactorBackend = "Subzero"
)

type test struct {
	r              *regres
	commit         git.Hash       // hash of the commit to test
	checkoutDir    string         // directory for the SwiftShader checkout
	resDir         string         // directory for the test results
	buildDir       string         // directory for SwiftShader build
	toolchain      llvm.Toolchain // the toolchain used for building
	reactorBackend reactorBackend // backend for SwiftShader build
	coverageEnv    *cov.Env       // coverage generation environment (optional).
}

// cleanup removes any temporary files used by the test.
func (t *test) cleanup() {
	if t.checkoutDir != "" && !t.r.keepCheckouts {
		os.RemoveAll(t.checkoutDir)
	}
}

// checkout clones the test's source commit into t.src.
func (t *test) checkout() error {
	if util.IsDir(t.checkoutDir) && t.r.keepCheckouts {
		log.Printf("Reusing source cache for commit '%s'\n", t.commit)
		return nil
	}
	log.Printf("Checking out '%s'\n", t.commit)
	os.RemoveAll(t.checkoutDir)
	if err := git.CheckoutRemoteCommit(t.checkoutDir, gitURL, t.commit, git.CommitFlags{
		Name:  gerritUserName,
		Email: t.r.gerritEmail,
	}); err != nil {
		return fmt.Errorf("failed to check out commit '%s': %w", t.commit, err)
	}
	log.Printf("Checked out commit '%s'\n", t.commit)
	return nil
}

// buildAndRun calls t.build() followed by t.run(). Errors are logged and
// reported in the returned deqprun.Results.Error field.
func (t *test) buildAndRun(testLists testlist.Lists, d deqpBuild) *deqp.Results {
	// Build the parent change.
	if err := t.build(); err != nil {
		msg := fmt.Sprintf("Failed to build '%s'", t.commit)
		log.Println(fmt.Errorf("%s: %w", msg, err))
		return &deqp.Results{Error: msg}
	}

	// Run the tests on the parent change.
	results, err := t.run(testLists, d)
	if err != nil {
		msg := fmt.Sprintf("Failed to test change '%s'", t.commit)
		log.Println(fmt.Errorf("%s: %w", msg, err))
		return &deqp.Results{Error: msg}
	}

	return results
}

// build builds the SwiftShader source into t.buildDir.
func (t *test) build() error {
	log.Printf("Building '%s'\n", t.commit)

	if err := os.MkdirAll(t.buildDir, 0777); err != nil {
		return fmt.Errorf("failed to create build directory: %w", err)
	}

	args := []string{
		`..`,
		`-DCMAKE_BUILD_TYPE=Release`,
		`-DSWIFTSHADER_DCHECK_ALWAYS_ON=1`,
		`-DREACTOR_VERIFY_LLVM_IR=1`,
		`-DREACTOR_BACKEND=` + string(t.reactorBackend),
		`-DSWIFTSHADER_LLVM_VERSION=10.0`,
		`-DSWIFTSHADER_WARNINGS_AS_ERRORS=0`,
	}

	if t.coverageEnv != nil {
		args = append(args, "-DSWIFTSHADER_EMIT_COVERAGE=1")
	}

	if err := shell.Env(buildTimeout, t.r.cmake, t.buildDir, t.r.toolchainEnv(), args...); err != nil {
		return err
	}

	if err := shell.Shell(buildTimeout, t.r.make, t.buildDir, fmt.Sprintf("-j%d", runtime.NumCPU())); err != nil {
		return err
	}

	return nil
}

func (t *test) run(testLists testlist.Lists, d deqpBuild) (*deqp.Results, error) {
	log.Printf("Running tests for '%s'\n", t.commit)

	swiftshaderICDSo := filepath.Join(t.buildDir, "libvk_swiftshader.so")
	if !util.IsFile(swiftshaderICDSo) {
		return nil, fmt.Errorf("failed to find '%s'", swiftshaderICDSo)
	}

	swiftshaderICDJSON := filepath.Join(t.buildDir, "Linux", "vk_swiftshader_icd.json")
	if !util.IsFile(swiftshaderICDJSON) {
		return nil, fmt.Errorf("failed to find '%s'", swiftshaderICDJSON)
	}

	if *limit != 0 {
		log.Printf("Limiting tests to %d\n", *limit)
		testLists = append(testlist.Lists{}, testLists...)
		for i := range testLists {
			testLists[i] = testLists[i].Limit(*limit)
		}
	}

	// Directory for per-test small transient files, such as log files,
	// coverage output, etc.
	// TODO(bclayton): consider using tmpfs here.
	tempDir := filepath.Join(t.buildDir, "temp")
	os.MkdirAll(tempDir, 0777)

	// Path to SwiftShader's libvulkan.so.1, which can be loaded directly by
	// dEQP without use of the Vulkan Loader.
	swiftshaderLibvulkanPath := filepath.Join(t.buildDir, "Linux")

	config := deqp.Config{
		ExeEgl:    filepath.Join(d.path, "build", "modules", "egl", "deqp-egl"),
		ExeGles2:  filepath.Join(d.path, "build", "modules", "gles2", "deqp-gles2"),
		ExeGles3:  filepath.Join(d.path, "build", "modules", "gles3", "deqp-gles3"),
		ExeVulkan: filepath.Join(d.path, "build", "external", "vulkancts", "modules", "vulkan", "deqp-vk"),
		TempDir:   tempDir,
		TestLists: testLists,
		Env: []string{
			"LD_LIBRARY_PATH=" + os.Getenv("LD_LIBRARY_PATH") + ":" + swiftshaderLibvulkanPath,
			"VK_ICD_FILENAMES=" + swiftshaderICDJSON,
			"DISPLAY=" + os.Getenv("DISPLAY"),
			"LIBC_FATAL_STDERR_=1", // Put libc explosions into logs.
		},
		LogReplacements: map[string]string{
			t.checkoutDir: "<SwiftShader>",
		},
		NumParallelTests: numParallelTests,
		MaxTestsPerProc:  *maxTestsPerProc,
		TestTimeout:      testTimeout,
		CoverageEnv:      t.coverageEnv,
	}

	return config.Run()
}

func (t *test) writeTestListsByStatus(testLists testlist.Lists, results *deqp.Results) ([]string, error) {
	out := []string{}

	for _, list := range testLists {
		files := map[testlist.Status]*os.File{}
		for _, status := range testlist.Statuses {
			path := testlist.FilePathWithStatus(filepath.Join(t.checkoutDir, list.File), status)
			dir := filepath.Dir(path)
			os.MkdirAll(dir, 0777)
			f, err := os.Create(path)
			if err != nil {
				return nil, fmt.Errorf("failed to create file '%v': %w", path, err)
			}
			defer f.Close()
			files[status] = f

			out = append(out, path)
		}

		for _, testName := range list.Tests {
			if r, found := results.Tests[testName]; found {
				fmt.Fprintln(files[r.Status], testName)
			}
		}
	}

	return out, nil
}

// resultsCachePath returns the path to the cache results file for the given
// test, testlists and deqpBuild.
func (t *test) resultsCachePath(testLists testlist.Lists, d deqpBuild) string {
	return filepath.Join(t.resDir, testLists.Hash(), d.hash)
}

type testStatusAndError struct {
	status testlist.Status
	error  string
}

type commonFailure struct {
	count int
	testStatusAndError
	exampleTest string
}

func commonFailures(results *deqp.Results) []commonFailure {
	failures := map[testStatusAndError]int{}
	examples := map[testStatusAndError]string{}
	for name, test := range results.Tests {
		if !test.Status.Failing() {
			continue
		}
		key := testStatusAndError{test.Status, test.Err}
		if count, ok := failures[key]; ok {
			failures[key] = count + 1
		} else {
			failures[key] = 1
			examples[key] = name
		}
	}
	out := make([]commonFailure, 0, len(failures))
	for failure, count := range failures {
		out = append(out, commonFailure{count, failure, examples[failure]})
	}
	sort.Slice(out, func(i, j int) bool { return out[i].count > out[j].count })
	return out
}

// compare returns a string describing all differences between two
// deqprun.Results, and a boolean indicating that this there are differences
// that are considered important.
// This string is used as the report message posted to the gerrit code review.
func compare(old, new *deqp.Results) (msg string, alert bool) {
	if old.Error != "" {
		return old.Error, false
	}
	if new.Error != "" {
		return new.Error, true
	}

	oldStatusCounts, newStatusCounts := map[testlist.Status]int{}, map[testlist.Status]int{}
	totalTests := 0

	broken, fixed, failing, removed, changed := []string{}, []string{}, []string{}, []string{}, []string{}

	for test, new := range new.Tests {
		old, found := old.Tests[test]
		if !found {
			log.Printf("Test result for '%s' not found on old change\n", test)
			continue
		}
		switch {
		case !old.Status.Failing() && new.Status.Failing():
			broken = append(broken, test)
			alert = true
		case !old.Status.Passing() && new.Status.Passing():
			fixed = append(fixed, test)
		case old.Status != new.Status:
			changed = append(changed, test)
			alert = true
		case old.Status.Failing() && new.Status.Failing():
			failing = append(failing, test) // Still broken
			alert = true
		}
		totalTests++
		if found {
			oldStatusCounts[old.Status] = oldStatusCounts[old.Status] + 1
		}
		newStatusCounts[new.Status] = newStatusCounts[new.Status] + 1
	}

	for test := range old.Tests {
		if _, found := new.Tests[test]; !found {
			removed = append(removed, test)
		}
	}

	sb := strings.Builder{}
	sb.WriteString("```\n")

	// list prints the list l to sb, truncating after a limit.
	list := func(l []string) {
		const max = 10
		for i, s := range l {
			sb.WriteString("  ")
			if i == max {
				sb.WriteString(fmt.Sprintf("> %d more\n", len(l)-i))
				break
			}
			sb.WriteString(fmt.Sprintf("> %s", s))
			if n, ok := new.Tests[s]; ok {
				if o, ok := old.Tests[s]; ok && n != o {
					sb.WriteString(fmt.Sprintf(" - [%s -> %s]", o.Status, n.Status))
				} else {
					sb.WriteString(fmt.Sprintf(" - [%s]", n.Status))
				}
				sb.WriteString("\n")
				for _, line := range strings.Split(n.Err, "\n") {
					if line != "" {
						sb.WriteString(fmt.Sprintf("     %v\n", line))
					}
				}
			} else {
				sb.WriteString("\n")
			}
		}
	}

	if n := len(broken); n > 0 {
		sort.Strings(broken)
		sb.WriteString(fmt.Sprintf("\n--- This change breaks %d tests: ---\n", n))
		list(broken)
	}
	if n := len(fixed); n > 0 {
		sort.Strings(fixed)
		sb.WriteString(fmt.Sprintf("\n--- This change fixes %d tests: ---\n", n))
		list(fixed)
	}
	if n := len(removed); n > 0 {
		sort.Strings(removed)
		sb.WriteString(fmt.Sprintf("\n--- This change removes %d tests: ---\n", n))
		list(removed)
	}
	if n := len(changed); n > 0 {
		sort.Strings(changed)
		sb.WriteString(fmt.Sprintf("\n--- This change alters %d tests: ---\n", n))
		list(changed)
	}

	if len(broken) == 0 && len(fixed) == 0 && len(removed) == 0 && len(changed) == 0 {
		sb.WriteString("\n--- No change in test results ---\n")
	}

	sb.WriteString(fmt.Sprintf("          Total tests: %d\n", totalTests))
	for _, s := range []struct {
		label  string
		status testlist.Status
	}{
		{"                 Pass", testlist.Pass},
		{"                 Fail", testlist.Fail},
		{"              Timeout", testlist.Timeout},
		{"      UNIMPLEMENTED()", testlist.Unimplemented},
		{"        UNSUPPORTED()", testlist.Unsupported},
		{"        UNREACHABLE()", testlist.Unreachable},
		{"             ASSERT()", testlist.Assert},
		{"              ABORT()", testlist.Abort},
		{"                Crash", testlist.Crash},
		{"        Not Supported", testlist.NotSupported},
		{"Compatibility Warning", testlist.CompatibilityWarning},
		{"      Quality Warning", testlist.QualityWarning},
		{"              Unknown", testlist.Unknown},
	} {
		old, new := oldStatusCounts[s.status], newStatusCounts[s.status]
		if old == 0 && new == 0 {
			continue
		}
		change := util.Percent64(int64(new-old), int64(old))
		switch {
		case old == new:
			sb.WriteString(fmt.Sprintf("%s: %v\n", s.label, new))
		case change == 0:
			sb.WriteString(fmt.Sprintf("%s: %v -> %v (%+d)\n", s.label, old, new, new-old))
		default:
			sb.WriteString(fmt.Sprintf("%s: %v -> %v (%+d %+d%%)\n", s.label, old, new, new-old, change))
		}
	}

	if old, new := old.Duration, new.Duration; old != 0 && new != 0 {
		label := "           Time taken"
		change := util.Percent64(int64(new-old), int64(old))
		switch {
		case old == new:
			sb.WriteString(fmt.Sprintf("%s: %v\n", label, new))
		case change == 0:
			sb.WriteString(fmt.Sprintf("%s: %v -> %v\n", label, old, new))
		default:
			sb.WriteString(fmt.Sprintf("%s: %v -> %v (%+d%%)\n", label, old, new, change))
		}
	}

	type timingDiff struct {
		old      time.Duration
		new      time.Duration
		relDelta float64
		name     string
	}

	timingDiffs := []timingDiff{}
	for name, new := range new.Tests {
		if old, ok := old.Tests[name]; ok {
			old, new := old.TimeTaken, new.TimeTaken
			delta := new.Seconds() - old.Seconds()
			absDelta := math.Abs(delta)
			relDelta := delta / old.Seconds()
			if absDelta > 2.0 && math.Abs(relDelta) > 0.05 { // If change > ±2s and > than ±5% old time...
				timingDiffs = append(timingDiffs, timingDiff{
					old:      old,
					new:      new,
					name:     name,
					relDelta: relDelta,
				})
			}
		}
	}
	if len(timingDiffs) > 0 {
		sb.WriteString(fmt.Sprintf("\n--- Test duration changes ---\n"))
		const limit = 10
		if len(timingDiffs) > limit {
			sort.Slice(timingDiffs, func(i, j int) bool { return math.Abs(timingDiffs[i].relDelta) > math.Abs(timingDiffs[j].relDelta) })
			timingDiffs = timingDiffs[:limit]
		}
		sort.Slice(timingDiffs, func(i, j int) bool { return timingDiffs[i].relDelta < timingDiffs[j].relDelta })
		for _, d := range timingDiffs {
			percent := util.Percent64(int64(d.new-d.old), int64(d.old))
			sb.WriteString(fmt.Sprintf("  > %v: %v -> %v (%+d%%)\n", d.name, d.old, d.new, percent))
		}
	}

	sb.WriteString("```\n")

	return sb.String(), alert
}

// loadTestLists loads the full test lists from the json file.
// The file is first searched at {t.srcDir}/{relPath}
// If this cannot be found, then the file is searched at the fallback path
// {CWD}/{relPath}
// This allows CLs to alter the list of tests to be run, as well as providing
// a default set.
func (t *test) loadTestLists(relPath string) (testlist.Lists, error) {
	// Seach for the test.json file in the checked out source directory.
	if path := filepath.Join(t.checkoutDir, relPath); util.IsFile(path) {
		log.Printf("Loading test list '%v' from commit\n", relPath)
		return testlist.Load(t.checkoutDir, path)
	}

	// Not found there. Search locally.
	wd, err := os.Getwd()
	if err != nil {
		return testlist.Lists{}, fmt.Errorf("failed to get current working directory: %w", err)
	}
	if path := filepath.Join(wd, relPath); util.IsFile(path) {
		log.Printf("Loading test list '%v' from regres\n", relPath)
		return testlist.Load(wd, relPath)
	}

	return nil, errors.New("Couldn't find a test list file")
}

type date struct {
	year  int
	month time.Month
	day   int
}

func toDate(t time.Time) date {
	d := date{}
	d.year, d.month, d.day = t.Date()
	return d
}
