// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
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

package main

import (
	"bufio"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"maps"
	"os"
	"path/filepath"
	"regexp"
	"slices"
	"strconv"
	"strings"
	"sync"
	"time"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

// Experiment mode path stems that get used multiple times
const (
	kExperimentCorporaSubDir = "corpora"
	kExperimentCorpusSubDir  = "corpus"
	kExperimentBinarySubDir  = "bin"
	kExperimentSettingsFile  = "experiment.json"
	kExperimentTaskStateFile = "state.json"
)

// CorpusDef defines the name and relative path of a fuzzer corpus.
type CorpusDef struct {
	Name string `json:"name"`
	Path string `json:"path"`
}

// DurationDef defines the configuration for a specific execution length.
// Seconds and Runs control the stopping criteria for a single fuzzer iteration and are mutually exclusive.
// Iterations provides an optional override for the experiment's default number of iterations.
type DurationDef struct {
	Seconds    *int `json:"seconds,omitempty"`
	Runs       *int `json:"runs,omitempty"`
	Iterations *int `json:"iterations,omitempty"`
}

// ExperimentSettings represents the structure of the experiment.json settings file.
type ExperimentSettings struct {
	Name                string        `json:"name"`
	Hash                string        `json:"hash"`
	Fuzzers             []string      `json:"fuzzers"`
	Timeout             *int          `json:"timeout,omitempty"`
	BenchmarkDuration   *int          `json:"benchmark_duration,omitempty"`
	BurnInDuration      *int          `json:"burnin_duration,omitempty"`
	BurnInEnabled       *bool         `json:"burnin_enabled,omitempty"`
	WgslBenchmarkCorpus string        `json:"wgsl_benchmark_corpus"`
	IrBenchmarkCorpus   string        `json:"ir_benchmark_corpus"`
	WgslCorpora         []CorpusDef   `json:"wgsl_corpora"`
	IrCorpora           []CorpusDef   `json:"ir_corpora"`
	DefaultIterations   int           `json:"default_iterations"`
	Durations           []DurationDef `json:"durations"`
}

type IterStateStatus string

const (
	IterStatePending   IterStateStatus = "pending"
	IterStateRunning   IterStateStatus = "running"
	IterStateCompleted IterStateStatus = "completed"
)

func (s *IterStateStatus) UnmarshalJSON(b []byte) error {
	var str string
	if err := json.Unmarshal(b, &str); err != nil {
		return err
	}
	switch IterStateStatus(str) {
	case IterStatePending, IterStateRunning, IterStateCompleted:
		*s = IterStateStatus(str)
		return nil
	default:
		return fmt.Errorf("invalid status: %s", str)
	}
}

type ExperimentLimitType string

const (
	ExperimentLimitTypeSeconds ExperimentLimitType = "seconds"
	ExperimentLimitTypeRuns    ExperimentLimitType = "runs"
)

func (t *ExperimentLimitType) UnmarshalJSON(b []byte) error {
	var str string
	if err := json.Unmarshal(b, &str); err != nil {
		return err
	}
	switch ExperimentLimitType(str) {
	case ExperimentLimitTypeSeconds, ExperimentLimitTypeRuns:
		*t = ExperimentLimitType(str)
		return nil
	default:
		return fmt.Errorf("invalid limit_type: %s", str)
	}
}

// IterState tracks the execution status and results of a single fuzzer iteration.
type IterState struct {
	Status        IterStateStatus     `json:"status"`
	StartTime     string              `json:"start_time"`
	EndTime       string              `json:"end_time"`
	PerfScore     float64             `json:"perf_score"`
	LimitType     ExperimentLimitType `json:"limit_type"`
	LimitValue    int                 `json:"limit_value"`
	ActualRuns    int                 `json:"actual_runs"`
	ActualSeconds float64             `json:"actual_seconds"`
}

// ExperimentTask represents a single unit of work to be executed by a worker.
type ExperimentTask struct {
	FuzzerName string
	CorpusName string
	CorpusPath string
	LimitType  ExperimentLimitType
	LimitValue int
	Iteration  int
	TaskDir    string
}

// FuzzerConfig contains the details of how a fuzzer should be built/run by the experimental framework
type FuzzerConfig struct {
	mode            FuzzMode
	additionGnFlags map[string]string
}

var (
	baseRequiredGnFlags = map[string]string{
		"use_libfuzzer":          "true",
		"tint_build_wgsl_reader": "true",
		"use_clang_coverage":     "true",
		"optimize_for_fuzzing":   "false",
	}

	fuzzerConfigs = map[string]FuzzerConfig{
		"tint_wgsl_fuzzer": {
			mode:            FuzzModeWgsl,
			additionGnFlags: map[string]string{},
		},
		"tint_ir_fuzzer": {
			mode: FuzzModeIr,
			additionGnFlags: map[string]string{
				"tint_build_ir_binary": "true",
				"tint_has_protobuf":    "true",
			},
		},
		"tint_wgsl_mesa_fuzzer": {
			mode: FuzzModeWgsl,
			additionGnFlags: map[string]string{
				"tint_build_mesa":                  "true",
				"tint_build_fuzzer_vulkan_support": "true",
			},
		},
		"tint_ir_mesa_fuzzer": {
			mode: FuzzModeIr,
			additionGnFlags: map[string]string{
				"tint_build_ir_binary":             "true",
				"tint_has_protobuf":                "true",
				"tint_build_mesa":                  "true",
				"tint_build_fuzzer_vulkan_support": "true",
			},
		},
	}
)

// runExperiment is the entry point for the 'experiment' subcommand.
// It validates the environment, builds binaries if needed, and runs the task queue.
func runExperiment(t *taskConfig) error {
	config, err := loadExperimentSettings(t, t.experimentPath)
	if err != nil {
		return err
	}

	binDir, err := buildExperimentBinariesIfNeeded(t, config)
	if err != nil {
		return err
	}

	machineResultsDir, err := generateResultsDirIfNeeded(t)
	if err != nil {
		return err
	}

	perfScores, needsBurnIn, err := runMicrobenchmarkIfNeeded(t, machineResultsDir, config)
	if err != nil {
		return err
	}

	tasks, err := calculateTasks(t, config, machineResultsDir)
	if err != nil {
		return err
	}

	pendingTasks := queueRemainingTasks(t, tasks)

	if err = runPendingTasks(t, pendingTasks, config, perfScores, binDir, needsBurnIn); err != nil {
		return err
	}

	fmt.Println("Experiment completed successfully!")
	return nil
}

// loadExperimentSettings reads and parses the experiment.json file from the
// specified experiment root directory, performing both semantic validation
// and mechanical existence checks.
func loadExperimentSettings(t *taskConfig, experimentRoot string) (ExperimentSettings, error) {
	if experimentRoot == "" {
		return ExperimentSettings{}, fmt.Errorf("experiment root path is required")
	}
	configPath := filepath.Join(experimentRoot, kExperimentSettingsFile)
	if !fileutils.IsFile(configPath, t.osWrapper) {
		return ExperimentSettings{}, fmt.Errorf("experiment configuration file '%s' not found", configPath)
	}

	settingsBytes, err := t.osWrapper.ReadFile(configPath)
	if err != nil {
		return ExperimentSettings{}, fmt.Errorf("failed to read experiment config: %w", err)
	}

	var settings ExperimentSettings
	if err := json.Unmarshal(settingsBytes, &settings); err != nil {
		return ExperimentSettings{}, fmt.Errorf("failed to parse experiment config JSON: %w", err)
	}

	if len(settings.Fuzzers) < 1 {
		return ExperimentSettings{}, fmt.Errorf("no fuzzers specified in 'fuzzers' list inside experiment.json")
	}

	activeModes := make(map[FuzzMode]bool)
	for _, fuzzer := range settings.Fuzzers {
		cfg, ok := fuzzerConfigs[fuzzer]
		if !ok {
			return ExperimentSettings{}, fmt.Errorf("unsupported fuzzer '%s'. Supported fuzzers are: %s", fuzzer, strings.Join(slices.Collect(maps.Keys(fuzzerConfigs)), ","))
		}
		activeModes[cfg.mode] = true
	}

	corporaDir := filepath.Join(experimentRoot, kExperimentCorporaSubDir)
	for mode := range activeModes {
		benchmarkCorpus := ""
		var corpora []CorpusDef

		switch mode {
		case FuzzModeWgsl:
			benchmarkCorpus = settings.WgslBenchmarkCorpus
			corpora = settings.WgslCorpora
		case FuzzModeIr:
			benchmarkCorpus = settings.IrBenchmarkCorpus
			corpora = settings.IrCorpora
		}

		if benchmarkCorpus == "" {
			return ExperimentSettings{}, fmt.Errorf("%s_benchmark_corpus is required in experiment.json because %s fuzzers are specified", mode, strings.ToUpper(mode.String()))
		}

		if len(corpora) < 1 {
			return ExperimentSettings{}, fmt.Errorf("at least one %s_corpora definition is required in experiment.json because %s fuzzers are specified", mode, strings.ToUpper(mode.String()))
		}

		bcPath := filepath.Join(corporaDir, benchmarkCorpus)
		if !fileutils.IsDir(bcPath, t.osWrapper) {
			return ExperimentSettings{}, fmt.Errorf("%s benchmark corpus directory '%s' not found under corpora root '%s'", mode, benchmarkCorpus, corporaDir)
		}

		for _, cDef := range corpora {
			cPath := filepath.Join(corporaDir, cDef.Path)
			if !fileutils.IsDir(cPath, t.osWrapper) {
				return ExperimentSettings{}, fmt.Errorf("%s corpus directory '%s' not found under corpora root '%s'", mode, cDef.Path, corporaDir)
			}
		}

	}

	for i, dDef := range settings.Durations {
		if dDef.Seconds != nil && dDef.Runs != nil {
			return ExperimentSettings{}, fmt.Errorf("duration at index %d has both 'seconds' and 'runs' defined, which is not allowed", i)
		}
		if dDef.Seconds == nil && dDef.Runs == nil {
			return ExperimentSettings{}, fmt.Errorf("duration at index %d must define either 'seconds' or 'runs'", i)
		}
	}

	return settings, nil
}

// buildExperimentBinariesIfNeeded ensures that all required fuzzer binaries
// exist in the experiment's bin directory. If any are missing, it builds them.
// Before building, it confirms that the GN args are correct for building the
// fuzzer, and also warns the user if they have sanitizers enabled, since
// that can significantly skew results.
// Returns the binary directory as a string and nil if all binaries are present
// otherwise returns an error.
func buildExperimentBinariesIfNeeded(t *taskConfig, settings ExperimentSettings) (string, error) {
	binDir := filepath.Join(t.experimentPath, kExperimentBinarySubDir)
	missingBinaries := make([]string, 0, len(settings.Fuzzers))
	for _, fuzzer := range settings.Fuzzers {
		fPath := filepath.Join(binDir, fuzzer+fileutils.ExeExt)
		if !fileutils.IsExe(fPath, t.osWrapper) {
			missingBinaries = append(missingBinaries, fuzzer)
		}
	}

	if len(missingBinaries) == 0 {
		return binDir, nil
	}

	hasSanitizers, err := checkGnArgs(t, missingBinaries)
	if err != nil {
		return "", err
	}

	if hasSanitizers {
		timeoutOverridden := settings.Timeout != nil && *settings.Timeout > 0
		warnMsg := []string{
			"",
			"================================ WARNING ================================",
			"Your build directory has compiler sanitizers (ASan, UBSan, TSan, or MSan) enabled.",
			"Fuzzing with sanitizers will significantly increase the runtime per test case,",
			"which will tend to dominate the results and possibly lead to false-positive timeouts.",
		}
		if !timeoutOverridden {
			warnMsg = append(warnMsg,
				"We highly recommend overriding the default 5s fuzzer timeout (by adding a \"timeout\"",
				"field to your experiment.json) if you proceed with sanitizers.",
			)
		}
		warnMsg = append(warnMsg,
			"=========================================================================",
			"",
		)
		fmt.Println(strings.Join(warnMsg, "\n"))
		confirm, err := promptConfirm("Are you sure you want to proceed with sanitizers enabled?")
		if err != nil {
			return "", fmt.Errorf("failed to read sanitizer confirmation: %w", err)
		}
		if !confirm {
			return "", fmt.Errorf("experiment execution aborted by user due to active sanitizers")
		}
	}

	fmt.Println("It looks like we are running a new experiment and need to build the fuzzers.")
	fmt.Println("Building will checkout commit ", settings.Hash, " and run gclient sync.")
	confirm, err := promptConfirm("Do you want to proceed with the build?")
	if err != nil {
		return "", fmt.Errorf("failed to read build confirmation: %w", err)
	}
	if !confirm {
		return "", fmt.Errorf("experiment execution aborted by user")
	}

	if err := prepareBinaries(t, &settings, binDir, missingBinaries); err != nil {
		return "", err
	}

	return binDir, nil
}

// runMicrobenchmarkIfNeeded ensures that performance normalization scores (runs/sec)
// exist for all fuzzers in the experiment. It loads existing scores from
// perf_scores.json if they exist, and runs the microbenchmark for any missing fuzzers,
// saving the results back to the file.
// Returns the scores in a map and if whether burn-in is still needed, otherwise an error.
func runMicrobenchmarkIfNeeded(t *taskConfig, machineResultsDir string, settings ExperimentSettings) (map[string]float64, bool, error) {
	perfScoresPath := filepath.Join(machineResultsDir, "perf_scores.json")
	perfScores := make(map[string]float64)
	if fileutils.IsFile(perfScoresPath, t.osWrapper) {
		perfScoresBytes, err := t.osWrapper.ReadFile(perfScoresPath)
		if err == nil {
			_ = json.Unmarshal(perfScoresBytes, &perfScores)
		}
	}

	burnInEnabled := true
	if settings.BurnInEnabled != nil {
		burnInEnabled = *settings.BurnInEnabled
	}
	needsBurnIn := burnInEnabled

	for _, fuzzer := range settings.Fuzzers {
		if _, ok := perfScores[fuzzer]; !ok {
			if needsBurnIn {
				if err := runBurnIn(t, &settings); err != nil {
					return nil, false, err
				}
				needsBurnIn = false
			}
			fmt.Println("Running microbenchmark for", fuzzer, "...")
			score, err := runMicrobenchmark(t, t.experimentPath, fuzzer, &settings)
			if err != nil {
				return nil, false, err
			}
			perfScores[fuzzer] = score
			// Save immediately in case the top-level process gets halted
			scoresBytes, _ := json.MarshalIndent(perfScores, "", "  ")
			_ = t.osWrapper.WriteFile(perfScoresPath, scoresBytes, 0644)
		}
	}
	return perfScores, needsBurnIn, nil
}

// generateResultsDirIfNeeded ensures that the results directory for the current
// machine exists within the experiment path. It creates the directory structure
// 'results/<machine_name>' (defaulting to 'default' if no machine name is set)
// if doesn't already exist.
// Returns the path to the machine-specific result directory if successful,
// otherwise an error.
func generateResultsDirIfNeeded(t *taskConfig) (string, error) {
	machine := t.machineName
	if machine == "" {
		machine = "default"
	}
	machineResultsDir := filepath.Join(t.experimentPath, "results", machine)
	if err := t.osWrapper.MkdirAll(machineResultsDir, 0755); err != nil {
		return "", fmt.Errorf("failed to create machine results directory '%s': %w", machineResultsDir, err)
	}
	return machineResultsDir, nil
}

// calculateTasks creates the full list of ExperimentTasks based on the
// experiment configuration. It iterates through all fuzzers and their
// corresponding corpora, producing tasks for each combination and
// duration.
// Returns a slice of ExperimentTask if successful, otherwise an error.
func calculateTasks(t *taskConfig, settings ExperimentSettings, machineResultsDir string) ([]ExperimentTask, error) {
	corporaDir := filepath.Join(t.experimentPath, kExperimentCorporaSubDir)
	var tasks []ExperimentTask
	for _, fuzzer := range settings.Fuzzers {
		var corpora []CorpusDef
		cfg, _ := fuzzerConfigs[fuzzer]
		switch cfg.mode {
		case FuzzModeWgsl:
			corpora = settings.WgslCorpora
		case FuzzModeIr:
			corpora = settings.IrCorpora
		default:
			return nil, fmt.Errorf("unknown fuzz mode %d", cfg.mode)
		}

		for _, corpus := range corpora {
			fuzzerTasks, err := calculateTasksForFuzzer(fuzzer, corpus, corporaDir, machineResultsDir, &settings)
			if err != nil {
				return nil, err
			}
			tasks = append(tasks, fuzzerTasks...)
		}
	}

	fmt.Println("Produced", len(tasks), "total tasks based on the experiment plan!")
	return tasks, nil
}

// queueRemainingTasks filters the list of tasks to identify those that are
// pending or were interrupted, based on the presence and content of a
// state.json file in the task directory. It also ensures the task's corpus
// directory is prepared.
// Returns a slice of ExperimentTask containing only the remaining tasks to be run.
func queueRemainingTasks(t *taskConfig, tasks []ExperimentTask) []ExperimentTask {
	// Queue up pending/interrupted tasks
	var pendingTasks []ExperimentTask
	for _, task := range tasks {
		statePath := filepath.Join(task.TaskDir, kExperimentTaskStateFile)
		if fileutils.IsFile(statePath, t.osWrapper) {
			stateBytes, err := t.osWrapper.ReadFile(statePath)
			if err == nil {
				var state IterState
				if err := json.Unmarshal(stateBytes, &state); err == nil && state.Status == IterStateCompleted {
					continue // already completed, skip
				}
			}
		}
		// Reset/Prepare task folder
		corpusDir := filepath.Join(task.TaskDir, kExperimentCorpusSubDir)
		_ = t.osWrapper.RemoveAll(corpusDir)
		_ = t.osWrapper.MkdirAll(corpusDir, 0755)
		pendingTasks = append(pendingTasks, task)
	}

	fmt.Println("Found", len(pendingTasks), "pending/incomplete tasks!")
	return pendingTasks
}

// runPendingTasks executes the provided list of experiment tasks using a parallel worker pool.
// It manages worker synchronization and context cancellation. It returns the first error
// encountered
func runPendingTasks(t *taskConfig, pendingTasks []ExperimentTask, settings ExperimentSettings, perfScores map[string]float64, binDir string, needsBurnIn bool) error {
	if len(pendingTasks) == 0 {
		return nil
	}

	if needsBurnIn {
		if err := runBurnIn(t, &settings); err != nil {
			return err
		}
	}

	fmt.Println("Executing pending/incomplete tasks using", t.numProcesses, "parallel jobs...")

	taskChan := make(chan ExperimentTask, len(pendingTasks))
	for _, task := range pendingTasks {
		taskChan <- task
	}
	close(taskChan)

	var wg sync.WaitGroup
	errChan := make(chan error, t.numProcesses)
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	timeoutVal := 5
	if settings.Timeout != nil && *settings.Timeout > 0 {
		timeoutVal = *settings.Timeout
	}

	for i := 0; i < t.numProcesses; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for {
				select {
				case <-ctx.Done():
					return
				case task, ok := <-taskChan:
					if !ok {
						return
					}
					score := perfScores[task.FuzzerName]
					if err := executeTask(ctx, t, binDir, task, score, timeoutVal); err != nil {
						if !errors.Is(err, context.Canceled) && !errors.Is(err, context.DeadlineExceeded) {
							errChan <- err
							cancel()
						}
						return
					}
				}
			}
		}()
	}

	wg.Wait()
	close(errChan)

	if len(errChan) > 0 {
		return <-errChan // return first error encountered
	}

	fmt.Println("Completed executing tasks!")
	return nil
}

// GnValue holds the string value of a GN argument.
type GnValue struct {
	Value string `json:"value"`
}

// GnArg represents a single GN argument as returned by 'gn args --json'.
type GnArg struct {
	Name    string   `json:"name"`
	Current *GnValue `json:"current,omitempty"`
	Default *GnValue `json:"default,omitempty"`
}

// GetValue returns the effective value of a GN argument, stripping quotes.
func (g *GnArg) GetValue() string {
	val := ""
	if g.Current != nil {
		val = g.Current.Value
	} else if g.Default != nil {
		val = g.Default.Value
	}
	if strings.HasPrefix(val, "\"") && strings.HasSuffix(val, "\"") {
		val = strings.Trim(val, "\"")
	}
	return val
}

// promptConfirm displays a yes/no prompt to the user.
func promptConfirm(msg string) (bool, error) {
	fmt.Printf("%s [y/N]: ", msg)
	reader := bufio.NewReader(os.Stdin)
	text, err := reader.ReadString('\n')
	if err != nil {
		return false, err
	}
	text = strings.TrimSpace(strings.ToLower(text))
	return text == "y" || text == "yes", nil
}

// checkGnArgs verifies that the build directory has the required GN flags set for the fuzzers being tested.
// It also checks and returns true if any standard compiler sanitizers are enabled.
func checkGnArgs(t *taskConfig, fuzzers []string) (bool, error) {
	out, err := t.runCmd("gn", "args", t.build, "--list", "--json")
	if err != nil {
		return false, fmt.Errorf("failed to query GN arguments for build directory '%s': %w", t.build, err)
	}

	var args []GnArg
	if err := json.Unmarshal(out, &args); err != nil {
		return false, fmt.Errorf("failed to parse GN arguments JSON: %w", err)
	}

	flagValues := make(map[string]string)
	for _, arg := range args {
		flagValues[arg.Name] = arg.GetValue()
	}

	requiredFlags := maps.Clone(baseRequiredGnFlags)
	for _, fuzzer := range fuzzers {
		cfg, _ := fuzzerConfigs[fuzzer]
		maps.Copy(requiredFlags, cfg.additionGnFlags)
	}

	var incorrect []string
	for flag, req := range requiredFlags {
		val := flagValues[flag]
		if val != req {
			incorrect = append(incorrect, fmt.Sprintf("%s (expected: %s, actual: %s)", flag, req, val))
		}
	}

	if len(incorrect) > 0 {
		return false, fmt.Errorf("the build directory '%s' has incorrect/missing GN configuration flags:\n  - %s\nPlease update your GN args and re-run", t.build, strings.Join(incorrect, "\n  - "))
	}

	hasSanitizers := flagValues["is_asan"] == "true" ||
		flagValues["is_ubsan"] == "true" ||
		flagValues["is_tsan"] == "true" ||
		flagValues["is_msan"] == "true"

	return hasSanitizers, nil
}

// prepareBinaries checks out the specified git hash, syncs dependencies,
// builds the fuzzers, and copies them to the experiment's bin directory.
func prepareBinaries(t *taskConfig, settings *ExperimentSettings, binDir string, fuzzers []string) error {
	return t.atGitHash(settings.Hash,
		func() error {
			if err := t.syncAndBuildTargets(fuzzers); err != nil {
				return err
			}

			if err := t.osWrapper.MkdirAll(binDir, 0755); err != nil {
				return fmt.Errorf("failed to create binary folder '%s': %w", binDir, err)
			}

			for _, fuzzer := range fuzzers {
				srcPath := filepath.Join(t.build, fuzzer+fileutils.ExeExt)
				dstPath := filepath.Join(binDir, fuzzer+fileutils.ExeExt)
				if err := fileutils.CopyFile(dstPath, srcPath, t.osWrapper); err != nil {
					return fmt.Errorf("failed to copy built binary %s to bin folder: %w", fuzzer, err)
				}
			}

			files, err := t.osWrapper.ReadDir(t.build)
			if err == nil {
				for _, f := range files {
					if !f.IsDir() {
						ext := filepath.Ext(f.Name())
						if ext == ".so" || ext == ".dylib" || ext == ".dll" {
							srcLib := filepath.Join(t.build, f.Name())
							dstLib := filepath.Join(binDir, f.Name())
							_ = fileutils.CopyFile(dstLib, srcLib, t.osWrapper)
						}
					}
				}
			}
			return nil
		})
}

// runMicrobenchmark executes a short fuzzer run to determine the execution
// speed (runs/sec) on the current hardware for normalization of results.
func runMicrobenchmark(t *taskConfig, root string, fuzzer string, settings *ExperimentSettings) (float64, error) {
	var corpusPath string
	cfg, _ := fuzzerConfigs[fuzzer]
	switch cfg.mode {
	case FuzzModeWgsl:
		corpusPath = filepath.Join(root, kExperimentCorporaSubDir, settings.WgslBenchmarkCorpus)
	case FuzzModeIr:
		corpusPath = filepath.Join(root, kExperimentCorporaSubDir, settings.IrBenchmarkCorpus)
	default:
		return 1.0, fmt.Errorf("unknown fuzz mode %d", cfg.mode)
	}

	binPath := filepath.Join(root, kExperimentBinarySubDir, fuzzer+fileutils.ExeExt)

	timeoutVal := 5
	if settings.Timeout != nil && *settings.Timeout > 0 {
		timeoutVal = *settings.Timeout
	}

	benchmarkDuration := 60
	if settings.BenchmarkDuration != nil && *settings.BenchmarkDuration > 0 {
		benchmarkDuration = *settings.BenchmarkDuration
	}

	actualRuns, elapsed, err := runBenchmarkCmd(t, binPath, corpusPath, benchmarkDuration, timeoutVal)
	if err != nil {
		return 1.0, err
	}

	score := float64(actualRuns) / elapsed
	if score <= 0 {
		score = 1.0
	}
	fmt.Println(fmt.Sprintf("Microbenchmark result for %s: %d runs in %.2fs (%.2f runs/sec)", fuzzer, actualRuns, elapsed, score))
	return score, nil
}

// runBenchmarkCmd runs a single fuzzer benchmark execution for the given duration (in seconds).
// It returns the number of runs executed, the actual elapsed time, and any error.
func runBenchmarkCmd(t *taskConfig, binPath string, corpusPath string, duration int, timeoutVal int) (int, float64, error) {
	tmpOut, err := t.osWrapper.MkdirTemp("", "perf_out")
	if err != nil {
		return 0, 0, err
	}
	defer t.osWrapper.RemoveAll(tmpOut)

	args := []string{tmpOut, corpusPath, fmt.Sprintf("-timeout=%d", timeoutVal), fmt.Sprintf("-max_total_time=%d", duration)}

	start := time.Now()
	out, err := t.runCmd(binPath, args...)
	elapsed := time.Since(start).Seconds()

	if err != nil {
		// The fuzzer may exit with non-zero if interrupted by max_total_time or if it hit a crash report, which is
		// fine, unless it exited really fast, since that means it is finding issues quickly, which means that the whole
		// experimental apparatus is not going to perform as expected
		if elapsed < 1.0 {
			return 0, elapsed, fmt.Errorf("microbenchmark terminated too quickly: %w", err)
		}
	}

	actualRuns, err := parseActualRuns(out)
	if err != nil {
		return 0, elapsed, err
	}

	return actualRuns, elapsed, nil
}

// runBurnIn runs the burn-in phase.
// It executes the micro benchmarks for each of the configured fuzzers in parallel matching t.numProcesses to attempt to
// get the physical hardware to a thermal steady state to avoid unexpected throttling mid-experiment.
func runBurnIn(t *taskConfig, settings *ExperimentSettings) error {
	burnInDuration := 300
	if settings.BurnInDuration != nil {
		burnInDuration = *settings.BurnInDuration
	}

	fmt.Println("Starting burn-in phase for", burnInDuration, "seconds using", t.numProcesses, "parallel jobs...")

	timeoutVal := 5
	if settings.Timeout != nil && *settings.Timeout > 0 {
		timeoutVal = *settings.Timeout
	}

	var wg sync.WaitGroup
	errChan := make(chan error, t.numProcesses)

	for i := 0; i < t.numProcesses; i++ {
		fuzzer := settings.Fuzzers[i%len(settings.Fuzzers)]
		var corpusPath string
		cfg, _ := fuzzerConfigs[fuzzer]
		switch cfg.mode {
		case FuzzModeWgsl:
			corpusPath = filepath.Join(t.experimentPath, kExperimentCorporaSubDir, settings.WgslBenchmarkCorpus)
		case FuzzModeIr:
			corpusPath = filepath.Join(t.experimentPath, kExperimentCorporaSubDir, settings.IrBenchmarkCorpus)
		default:
			return fmt.Errorf("unknown fuzz mode %d for fuzzer %s", cfg.mode, fuzzer)
		}

		binPath := filepath.Join(t.experimentPath, kExperimentBinarySubDir, fuzzer+fileutils.ExeExt)

		wg.Add(1)
		go func(workerID int, f string, cp string, bp string) {
			defer wg.Done()
			_, _, err := runBenchmarkCmd(t, bp, cp, burnInDuration, timeoutVal)
			if err != nil {
				errChan <- fmt.Errorf("burn-in worker %d failed: %w", workerID, err)
			}
		}(i, fuzzer, corpusPath, binPath)
	}

	wg.Wait()
	close(errChan)

	if len(errChan) > 0 {
		return <-errChan
	}

	fmt.Println("Burn-in phase completed successfully!")
	return nil
}

// calculateTasksForFuzzer creates the list of ExperimentTasks for a specific
// fuzzer and corpus combination based on the experiment durations.
func calculateTasksForFuzzer(fuzzer string, corpus CorpusDef, corporaDir string, machineDir string, settings *ExperimentSettings) ([]ExperimentTask, error) {
	var tasks []ExperimentTask
	cPath := filepath.Join(corporaDir, corpus.Path)

	for _, dDef := range settings.Durations {
		var limitType ExperimentLimitType
		limitValue := 0
		if dDef.Seconds != nil {
			limitType = ExperimentLimitTypeSeconds
			limitValue = *dDef.Seconds
		} else {
			limitType = ExperimentLimitTypeRuns
			limitValue = *dDef.Runs
		}

		iterations := settings.DefaultIterations
		if dDef.Iterations != nil {
			iterations = *dDef.Iterations
		}

		limitStr := fmt.Sprintf("%s_%d", limitType, limitValue)

		for i := 1; i <= iterations; i++ {
			taskDir := filepath.Join(machineDir, fuzzer, corpus.Name, limitStr, fmt.Sprintf("iter_%d", i))
			tasks = append(tasks, ExperimentTask{
				FuzzerName: fuzzer,
				CorpusName: corpus.Name,
				CorpusPath: cPath,
				LimitType:  limitType,
				LimitValue: limitValue,
				Iteration:  i,
				TaskDir:    taskDir,
			})
		}
	}
	return tasks, nil
}

// executeTask runs a single fuzzer iteration, manages its state file,
// and captures the output logs and performance data.
func executeTask(ctx context.Context, t *taskConfig, binDir string, task ExperimentTask, score float64, timeoutVal int) error {
	statePath := filepath.Join(task.TaskDir, kExperimentTaskStateFile)

	// Update state to running
	state := IterState{
		Status:     IterStateRunning,
		StartTime:  time.Now().Format(time.RFC3339),
		PerfScore:  score,
		LimitType:  task.LimitType,
		LimitValue: task.LimitValue,
	}
	stateBytes, _ := json.MarshalIndent(state, "", "  ")
	_ = t.osWrapper.WriteFile(statePath, stateBytes, 0644)

	corpusOut := filepath.Join(task.TaskDir, kExperimentCorpusSubDir)
	_ = t.osWrapper.MkdirAll(corpusOut, 0755)

	absCorpusOut, err := filepath.Abs(corpusOut)
	if err != nil {
		return fmt.Errorf("failed to get absolute path for corpus out: %w", err)
	}

	absCorpusPath, err := filepath.Abs(task.CorpusPath)
	if err != nil {
		return fmt.Errorf("failed to get absolute path for corpus input: %w", err)
	}

	args := []string{absCorpusOut, absCorpusPath, fmt.Sprintf("-timeout=%d", timeoutVal)}
	switch task.LimitType {
	case ExperimentLimitTypeSeconds:
		args = append(args, fmt.Sprintf("-max_total_time=%d", task.LimitValue))
	case ExperimentLimitTypeRuns:
		args = append(args, fmt.Sprintf("-runs=%d", task.LimitValue))
	default:
		return fmt.Errorf("unknown limit type %s", task.LimitType)
	}

	cfg, _ := fuzzerConfigs[task.FuzzerName]
	// Add dictionary if running WGSL fuzzer
	if cfg.mode == FuzzModeWgsl {
		dictPath := filepath.Join(filepath.Join(fileutils.DawnRoot(t.osWrapper), "src", "tint", "cmd", "fuzz", "wgsl"), "dictionary.txt")
		if fileutils.IsFile(dictPath, t.osWrapper) {
			absDictPath, err := filepath.Abs(dictPath)
			if err != nil {
				return fmt.Errorf("failed to get absolute path for dictionary: %w", err)
			}
			args = append(args, "-dict="+absDictPath)
		}
	}

	fmt.Println(fmt.Sprintf("[%s] Starting: %s on %s (%s=%d, iter %d)",
		time.Now().Format("15:04:05"), task.FuzzerName, task.CorpusName, task.LimitType, task.LimitValue, task.Iteration))

	start := time.Now()
	// Run the binary directly from binDir, setting working directory to TaskDir so profraw files are not combined
	// between tasks
	binPath := filepath.Join(binDir, task.FuzzerName+fileutils.ExeExt)
	absTaskBin, err := filepath.Abs(binPath)
	if err != nil {
		return fmt.Errorf("failed to get absolute path of fuzzer binary: %w", err)
	}

	var out []byte
	for retry := 0; retry < 5; retry++ {
		cmd := t.execWrapper.CommandContext(ctx, absTaskBin, args...).WithDir(task.TaskDir)
		out, err = cmd.RunWithCombinedOutput()
		if err == nil {
			break
		}
		if strings.Contains(err.Error(), "text file busy") {
			time.Sleep(100 * time.Millisecond)
			continue
		}
		break
	}
	duration := time.Since(start)

	if err != nil {
		if ctx.Err() != nil {
			return ctx.Err()
		}
		if t.verbose {
			fmt.Println("Fuzzer process returned error/exit status:", err)
		}
	}

	logPath := filepath.Join(task.TaskDir, "fuzzer.log")
	_ = t.osWrapper.WriteFile(logPath, out, 0644)

	actualRuns, err := parseActualRuns(out)
	if err != nil {
		return err
	}

	state.Status = IterStateCompleted
	state.EndTime = time.Now().Format(time.RFC3339)
	state.ActualRuns = actualRuns
	state.ActualSeconds = duration.Seconds()

	fmt.Println(fmt.Sprintf("[%s] Completed: %s on %s (iter %d) in %.2fs, %d runs",
		time.Now().Format("15:04:05"), task.FuzzerName, task.CorpusName, task.Iteration, duration.Seconds(), actualRuns))

	stateBytes, _ = json.MarshalIndent(state, "", "  ")
	_ = t.osWrapper.WriteFile(statePath, stateBytes, 0644)

	return nil
}

// parseActualRuns extracts the number of fuzzer executions from the fuzzer output.
func parseActualRuns(output []byte) (int, error) {
	// First, try to parse the 'stat::number_of_executed_units' line.
	re := regexp.MustCompile(`stat::number_of_executed_units:\s*(\d+)`)
	if match := re.FindStringSubmatch(string(output)); len(match) > 1 {
		if val, err := strconv.Atoi(match[1]); err == nil {
			return val, nil
		}
	}

	// Fallback to parsing pulse lines like: #1234	PULSE
	rePulse := regexp.MustCompile(`#(\d+)\s+`)
	matches := rePulse.FindAllStringSubmatch(string(output), -1)
	if len(matches) > 0 {
		lastMatch := matches[len(matches)-1]
		if val, err := strconv.Atoi(lastMatch[1]); err == nil {
			return val, nil
		}
	}

	return 0, fmt.Errorf("failed to parse actual runs from fuzzer output")
}
