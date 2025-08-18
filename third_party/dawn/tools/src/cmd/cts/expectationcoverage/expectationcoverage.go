// Copyright 2025 The Dawn & Tint Authors
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

package expectationcoverage

import (
	"context"
	"flag"
	"fmt"
	"io"
	"os"
	"runtime"
	"slices"
	"sort"
	"strings"
	"sync"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/cts/expectations"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

const maxResultsPerWorker = 200000

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		maxOutput               int
		checkCompatExpectations bool
		individualExpectations  bool
		ignoreSkipExpectations  bool
		cacheDir                string
		verbose                 bool
	}
}

type ChunkWithCounter struct {
	Chunk *expectations.Chunk
	Count int
}

func (cmd) Name() string {
	return "expectation-coverage"
}

func (cmd) Desc() string {
	return "checks how much test coverage is lost due to expectations"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	flag.IntVar(
		&c.flags.maxOutput,
		"max-output",
		25,
		"limit output to the top X expectation groups, set to 0 for unlimited")
	flag.BoolVar(
		&c.flags.checkCompatExpectations,
		"check-compat-expectations",
		false,
		"check compat expectations instead of regular expectations")
	flag.BoolVar(
		&c.flags.individualExpectations,
		"check-individual-expectations",
		false,
		"check individual expectations instead of groups")
	flag.BoolVar(
		&c.flags.ignoreSkipExpectations,
		"ignore-skip-expectations",
		false,
		"do not check the impact of Skip expectations")
	flag.StringVar(
		&c.flags.cacheDir,
		"cache",
		common.DefaultCacheDir,
		"path to the results cache")
	flag.BoolVar(
		&c.flags.verbose,
		"verbose",
		false,
		"emit additional logging")
	return nil, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	individualExpectations := c.flags.individualExpectations

	// Parse expectation file
	fmt.Println("Getting trimmed expectation file content")
	startTime := time.Now()
	var expectationPath string
	if c.flags.checkCompatExpectations {
		expectationPath = common.DefaultCompatExpectationsPath(cfg.OsWrapper)
	} else {
		expectationPath = common.DefaultExpectationsPath(cfg.OsWrapper)
	}
	content, err := getTrimmedContent(expectationPath,
		individualExpectations,
		c.flags.ignoreSkipExpectations,
		c.flags.verbose,
		cfg.OsWrapper)
	if err != nil {
		return err
	}
	if c.flags.verbose {
		fmt.Printf("Took %s\n", time.Now().Sub(startTime).String())
		fmt.Printf("Got %d chunks/individual expectations\n", len(content.Chunks))
	}

	// Get ResultDB data
	fmt.Println("Getting results")
	startTime = time.Now()
	var uniqueResults result.List
	if c.flags.checkCompatExpectations {
		uniqueResults, err = common.CacheRecentUniqueSuppressedCompatResults(
			ctx, cfg, c.flags.cacheDir, cfg.Querier, cfg.OsWrapper)
	} else {
		uniqueResults, err = common.CacheRecentUniqueSuppressedCoreResults(
			ctx, cfg, c.flags.cacheDir, cfg.Querier, cfg.OsWrapper)
	}
	if err != nil {
		return err
	}
	if c.flags.verbose {
		fmt.Printf("Took %s\n", time.Now().Sub(startTime).String())
		fmt.Printf("Got %d unique results\n", len(uniqueResults))
	}

	// Process ResultDB data
	fmt.Println("Processing results")
	startTime = time.Now()
	orderedChunks := getChunksOrderedByCoverageLoss(&content, &uniqueResults)
	if c.flags.verbose {
		fmt.Printf("Took %s\n", time.Now().Sub(startTime).String())
	}

	// Output results.
	outputResults(orderedChunks, c.flags.maxOutput, individualExpectations, os.Stdout)

	return nil
}

// getTrimmedContent returns a Content with certain Chunks removed or modified
// based on the provided arguments.
func getTrimmedContent(
	expectationPath string,
	individualExpectations bool,
	ignoreSkipExpectations bool,
	verbose bool,
	fsReader oswrapper.FilesystemReader) (expectations.Content, error) {
	rawFileContentBytes, err := fsReader.ReadFile(expectationPath)
	if err != nil {
		return expectations.Content{}, err
	}
	rawFileContent := string(rawFileContentBytes[:])

	content, err := expectations.Parse(expectationPath, rawFileContent)
	if err != nil {
		return expectations.Content{}, err
	}

	// Remove any permanent Skip expectations since they are never expected to
	// be removed from the file. Also remove any pure comment chunks.
	permanentSkipContent, err := getPermanentSkipContent(expectationPath, rawFileContent)
	if err != nil {
		return expectations.Content{}, err
	}

	// Get a copy of all relevant chunks.
	var trimmedChunks []expectations.Chunk
	for _, chunk := range content.Chunks {
		if chunk.IsCommentOnly() {
			continue
		}
		if chunk.ContainedWithinList(&permanentSkipContent.Chunks) {
			continue
		}

		var maybeSkiplessChunk expectations.Chunk
		if ignoreSkipExpectations {
			maybeSkiplessChunk = expectations.Chunk{
				Comments:     chunk.Comments,
				Expectations: expectations.Expectations{},
			}
			for _, e := range chunk.Expectations {
				if slices.Contains(e.Status, string(result.Skip)) {
					continue
				}
				maybeSkiplessChunk.Expectations = append(maybeSkiplessChunk.Expectations, e)
			}
		} else {
			maybeSkiplessChunk = chunk
		}

		if maybeSkiplessChunk.IsCommentOnly() {
			continue
		}
		trimmedChunks = append(trimmedChunks, maybeSkiplessChunk)
	}

	// Split chunks into individual expectations if requested.
	var maybeSplitChunks []expectations.Chunk
	if individualExpectations {
		for _, chunk := range trimmedChunks {
			for _, e := range chunk.Expectations {
				individualChunk := expectations.Chunk{
					Comments:     chunk.Comments,
					Expectations: expectations.Expectations{e},
				}
				maybeSplitChunks = append(maybeSplitChunks, individualChunk)
			}
		}
	} else {
		maybeSplitChunks = trimmedChunks
	}
	content.Chunks = maybeSplitChunks

	return content, nil
}

// getPermanentSkipContent returns a Content only containing Chunks for
// permanent Skip expectations.
func getPermanentSkipContent(
	expectationPath string,
	rawFileContent string) (expectations.Content, error) {
	// Since the standard format for expectation files is:
	//  - Permanent Skip expectations
	//  - Temporary Skip expectations
	//  - Triaged flakes/failure expectations
	//  - Untriaged auto-generated expectations
	// Assume we care about everything up to the temporary Skip expectation
	// section.
	targetLine := "# Temporary Skip Expectations"
	var keptLines []string
	brokeEarly := false
	for _, line := range strings.Split(rawFileContent, "\n") {
		if strings.HasPrefix(line, targetLine) {
			brokeEarly = true
			break
		}
		keptLines = append(keptLines, line)
	}

	if !brokeEarly {
		fmt.Println("Unable to find permanent Skip expectations, assuming none exist")
		return expectations.Content{}, nil
	}

	permanentSkipRawContent := strings.Join(keptLines, "\n")
	permanentSkipContent, err := expectations.Parse(expectationPath, permanentSkipRawContent)
	if err != nil {
		return expectations.Content{}, err
	}

	// Omit any pure comment chunks.
	var trimmedChunks []expectations.Chunk
	for _, chunk := range permanentSkipContent.Chunks {
		if !chunk.IsCommentOnly() {
			trimmedChunks = append(trimmedChunks, chunk)
		}
	}
	permanentSkipContent.Chunks = trimmedChunks

	return permanentSkipContent, nil
}

// math.Min only works on floats, and the built-in min is not available until
// go 1.21.
func minInt(a, b int) int {
	if a < b {
		return a
	}
	return b
}

// getChunksOrderedByCoverageLost returns the Chunks contained within 'content'
// ordered by how many results from 'uniqueResults' are affected by expectations
// within the Chunk.
//
// Under the hood, actual processing is farmed out to goroutines to better
// handle large amounts of results.
func getChunksOrderedByCoverageLoss(
	content *expectations.Content,
	uniqueResults *result.List) []ChunkWithCounter {

	affectedChunks := make([]ChunkWithCounter, len(content.Chunks))
	for i, _ := range content.Chunks {
		affectedChunks[i].Chunk = &(content.Chunks[i])
	}

	// Create a goroutine pool. Each worker pulls a single ChunkWithCounter from
	// the queue at a time and handles all of the processing for it.
	numWorkers := minInt(len(affectedChunks), runtime.NumCPU())
	workQueue := make(chan *ChunkWithCounter)
	waitGroup := new(sync.WaitGroup)
	waitGroup.Add(numWorkers)
	for i := 0; i < numWorkers; i++ {
		go processChunk(workQueue, uniqueResults, waitGroup)
	}

	// Each of the ChunkWithCounter will have its Count filled in place by a
	// worker when picked up.
	for i, _ := range affectedChunks {
		workQueue <- &(affectedChunks[i])
	}
	close(workQueue)
	waitGroup.Wait()

	// Sort based on the final tally.
	sortFunc := func(i, j int) bool {
		return affectedChunks[i].Count > affectedChunks[j].Count
	}
	sort.SliceStable(affectedChunks, sortFunc)

	return affectedChunks
}

// processChunk counts how many Results in 'uniqueResults' apply to Expectations
// in a provided ChunkWithCounter that is provided via 'workQueue'. The function
// will continue to pull work from 'workQueue' until it is closed and empty, at
// which point the function will exit and signal to 'waitGroup' that it is
// finished.
//
// Under the hood, actual processing is farmed out to additional goroutines to
// better handle large amounts of results.
func processChunk(
	workQueue chan *ChunkWithCounter,
	uniqueResults *result.List,
	waitGroup *sync.WaitGroup) {

	defer waitGroup.Done()

	// Create a pool of workers to handle processing of subsets of results. Each
	// worker handles every ith result and returns the number of those results
	// that applied to an expectation within the given ChunkWithCounter.
	numWorkers := int(len(*uniqueResults)/maxResultsPerWorker) + 1
	subWorkQueues := []chan *ChunkWithCounter{}
	subResultQueues := []chan int{}
	for i := 0; i < numWorkers; i++ {
		subWorkQueues = append(subWorkQueues, make(chan *ChunkWithCounter))
		subResultQueues = append(subResultQueues, make(chan int))
		go processChunkForResultSubset(
			subWorkQueues[i],
			subResultQueues[i],
			uniqueResults,
			i,
			numWorkers)
	}

	for {
		chunkWithCounter, queueOpen := <-workQueue
		if !queueOpen {
			for _, swq := range subWorkQueues {
				close(swq)
			}
			return
		}

		for i := 0; i < numWorkers; i++ {
			subWorkQueues[i] <- chunkWithCounter
		}
		for i := 0; i < numWorkers; i++ {
			chunkWithCounter.Count += <-subResultQueues[i]
		}
	}
}

// processChunkForResultSubset counts how many Results in 'uniqueResults' apply
// to Expectations in a provided ChunkWithCounter that is provided via
// 'workQueue'. Only every 'numWorkers' element of 'uniqueResults' is processed,
// starting at the 'workNumber' element. The count for each ChunkWithCounter is
// returned via 'resultQueue' in the same order that the work was provided.
//
// The function will continue to pull work from 'workQueue' until it is closed
// and empty.
func processChunkForResultSubset(
	workQueue chan *ChunkWithCounter,
	resultQueue chan int,
	uniqueResults *result.List,
	workerNumber, numWorkers int) {

	for {
		chunkWithCounter, queueOpen := <-workQueue
		if !queueOpen {
			return
		}

		numApplicableResults := 0
		for i := workerNumber; i < len(*uniqueResults); i += numWorkers {
			result := (*uniqueResults)[i]
			for _, expectation := range chunkWithCounter.Chunk.Expectations {
				if expectation.AppliesToResult(result) {
					numApplicableResults += 1
					break
				}
			}
		}

		resultQueue <- numApplicableResults
	}
}

func outputResults(
	orderedChunks []ChunkWithCounter,
	maxChunksToOutput int,
	individualExpectations bool,
	writer io.Writer) {

	var expectationPrefix, chunkType string
	if individualExpectations {
		chunkType = "individual expectation"
		expectationPrefix = "Expectation: "
	} else {
		chunkType = "chunk"
		expectationPrefix = "First expectation: "
	}

	if maxChunksToOutput == 0 {
		fmt.Fprintln(writer, "\nComplete output:")
	} else {
		fmt.Fprintf(
			writer,
			"\nTop %d %ss contributing to test coverage loss:\n",
			maxChunksToOutput,
			chunkType)
	}

	for i, chunkWithCounter := range orderedChunks {
		if maxChunksToOutput != 0 && i == maxChunksToOutput {
			break
		}

		chunk := chunkWithCounter.Chunk
		firstExpectation := chunk.Expectations[0]
		fmt.Fprintln(writer, "")
		fmt.Fprintf(writer, "Comment: %s\n", strings.Join(chunk.Comments, "\n"))
		fmt.Fprintf(writer, "%s%s\n", expectationPrefix, firstExpectation.AsExpectationFileString())
		fmt.Fprintf(writer, "Line number: %d\n", firstExpectation.Line)
		fmt.Fprintf(writer, "Affected %d test results\n", chunkWithCounter.Count)
	}
}
