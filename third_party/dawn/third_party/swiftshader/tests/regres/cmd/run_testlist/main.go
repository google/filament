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

// run_testlist is a tool runs a dEQP test list, using multiple sand-boxed
// processes.
//
// Unlike simply running deqp with its --deqp-caselist-file flag, run_testlist
// uses multiple sand-boxed processes, which greatly reduces testing time, and
// gracefully handles crashing processes.
package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"math/rand"
	"os"
	"path/filepath"
	"regexp"
	"runtime"
	"strings"
	"time"

	"swiftshader.googlesource.com/SwiftShader/tests/regres/cov"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/deqp"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/llvm"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/shell"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/testlist"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/util"
)

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}

var (
	deqpVkBinary     = flag.String("deqp-vk", "deqp-vk", "path to the deqp-vk binary")
	testList         = flag.String("test-list", "vk-master-PASS.txt", "path to a test list file")
	numThreads       = flag.Int("num-threads", min(runtime.NumCPU(), 100), "number of parallel test runner processes")
	maxTestsPerProc  = flag.Int("max-tests-per-proc", 1, "maximum number of tests running in a single process")
	maxProcMemory    = flag.Uint64("max-proc-mem", shell.MaxProcMemory, "maximum virtual memory per child process")
	output           = flag.String("output", "results.json", "path to an output JSON results file")
	filter           = flag.String("filter", "", "filter for test names. Start with a '/' to indicate regex")
	limit            = flag.Int("limit", 0, "only run a maximum of this number of tests")
	shuffle          = flag.Bool("shuffle", false, "shuffle tests")
	noResults        = flag.Bool("no-results", false, "disable generation of results.json file")
	genCoverage      = flag.Bool("coverage", false, "generate test coverage")
	enableValidation = flag.Bool("validation", false, "run deqp-vk with Vulkan validation layers")
)

const testTimeout = time.Minute * 2

func run() error {
	group := testlist.Group{
		Name: "",
		File: *testList,
		API:  testlist.Vulkan,
	}
	if err := group.Load(); err != nil {
		return err
	}

	if *filter != "" {
		if strings.HasPrefix(*filter, "/") {
			re := regexp.MustCompile((*filter)[1:])
			group = group.Filter(re.MatchString)
		} else {
			group = group.Filter(func(name string) bool {
				ok, _ := filepath.Match(*filter, name)
				return ok
			})
		}
	}

	shell.MaxProcMemory = *maxProcMemory

	if *shuffle {
		rnd := rand.New(rand.NewSource(time.Now().UnixNano()))
		rnd.Shuffle(len(group.Tests), func(i, j int) { group.Tests[i], group.Tests[j] = group.Tests[j], group.Tests[i] })
	}

	if *limit != 0 && len(group.Tests) > *limit {
		group.Tests = group.Tests[:*limit]
	}

	log.Printf("Running %d tests...\n", len(group.Tests))

	config := deqp.Config{
		ExeEgl:           "",
		ExeGles2:         "",
		ExeGles3:         "",
		ExeVulkan:        *deqpVkBinary,
		Env:              os.Environ(),
		NumParallelTests: *numThreads,
		MaxTestsPerProc:  *maxTestsPerProc,
		TestLists:        testlist.Lists{group},
		TestTimeout:      testTimeout,
		ValidationLayer:  *enableValidation,
	}

	if *genCoverage {
		icdPath := findSwiftshaderICD()
		t := findToolchain(icdPath)
		config.CoverageEnv = &cov.Env{
			LLVM:     t.llvm,
			TurboCov: t.turbocov,
			RootDir:  projectRootDir(),
			ExePath:  findSwiftshaderSO(icdPath),
		}
	}

	res, err := config.Run()
	if err != nil {
		return err
	}

	counts := map[testlist.Status]int{}
	for _, r := range res.Tests {
		counts[r.Status] = counts[r.Status] + 1
	}
	for _, s := range testlist.Statuses {
		if count := counts[s]; count > 0 {
			log.Printf("%s: %d\n", string(s), count)
		}
	}

	if *genCoverage {
		f, err := os.Create("coverage.dat")
		if err != nil {
			return fmt.Errorf("failed to open coverage.dat file: %w", err)
		}
		if err := res.Coverage.Encode("master", f); err != nil {
			return fmt.Errorf("failed to encode coverage data: %w", err)
		}
	}

	if !*noResults {
		err = res.Save(*output)
		if err != nil {
			return err
		}
	}

	return nil
}

func findSwiftshaderICD() string {
	icdPaths := strings.Split(os.Getenv("VK_ICD_FILENAMES"), ";")
	for _, icdPath := range icdPaths {
		_, file := filepath.Split(icdPath)
		if file == "vk_swiftshader_icd.json" {
			return icdPath
		}
	}
	panic("Cannot find vk_swiftshader_icd.json in VK_ICD_FILENAMES")
}

func findSwiftshaderSO(vkSwiftshaderICD string) string {
	root := struct {
		ICD struct {
			Path string `json:"library_path"`
		}
	}{}

	icd, err := ioutil.ReadFile(vkSwiftshaderICD)
	if err != nil {
		panic(fmt.Errorf("Could not read '%v'. %v", vkSwiftshaderICD, err))
	}

	if err := json.NewDecoder(bytes.NewReader(icd)).Decode(&root); err != nil {
		panic(fmt.Errorf("Could not parse '%v'. %v", vkSwiftshaderICD, err))
	}

	if util.IsFile(root.ICD.Path) {
		return root.ICD.Path
	}
	dir := filepath.Dir(vkSwiftshaderICD)
	path, err := filepath.Abs(filepath.Join(dir, root.ICD.Path))
	if err != nil {
		panic(fmt.Errorf("Could not locate ICD so at '%v'. %v", root.ICD.Path, err))
	}

	return path
}

type toolchain struct {
	llvm     llvm.Toolchain
	turbocov string
}

func findToolchain(vkSwiftshaderICD string) toolchain {
	minVersion := llvm.Version{Major: 7}

	// Try finding the llvm toolchain via the CMake generated
	// coverage-toolchain.txt file that sits next to vk_swiftshader_icd.json.
	dir := filepath.Dir(vkSwiftshaderICD)
	toolchainInfoPath := filepath.Join(dir, "coverage-toolchain.txt")
	if util.IsFile(toolchainInfoPath) {
		if file, err := os.Open(toolchainInfoPath); err == nil {
			defer file.Close()
			content := struct {
				LLVM     string `json:"llvm"`
				TurboCov string `json:"turbo-cov"`
			}{}
			err := json.NewDecoder(file).Decode(&content)
			if err != nil {
				log.Fatalf("Couldn't read 'toolchainInfoPath': %v", err)
			}
			if t := llvm.Search(content.LLVM).FindAtLeast(minVersion); t != nil {
				return toolchain{*t, content.TurboCov}
			}
		}
	}

	// Fallback, try searching PATH.
	if t := llvm.Search().FindAtLeast(minVersion); t != nil {
		return toolchain{*t, ""}
	}

	log.Fatal("Could not find LLVM toolchain")
	return toolchain{}
}

func projectRootDir() string {
	_, thisFile, _, _ := runtime.Caller(1)
	thisDir := filepath.Dir(thisFile)
	root, err := filepath.Abs(filepath.Join(thisDir, "../../../.."))
	if err != nil {
		panic(err)
	}
	return root
}

func main() {
	flag.ErrHelp = errors.New("regres is a tool to detect regressions between versions of SwiftShader")
	flag.Parse()
	if err := run(); err != nil {
		_, _ = fmt.Fprintln(os.Stderr, err)
		os.Exit(-1)
	}
}
