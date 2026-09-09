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
	"path/filepath"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/execwrapper"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestCheckGnArgs(t *testing.T) {
	tests := []struct {
		name    string
		gnJson  string
		fuzzers []string
		wantErr bool
	}{
		{
			name: "Correct baseline flags",
			gnJson: `[
				{"name": "use_libfuzzer", "current": {"value": "true"}},
				{"name": "tint_build_wgsl_reader", "current": {"value": "true"}},
				{"name": "use_clang_coverage", "current": {"value": "true"}},
				{"name": "optimize_for_fuzzing", "current": {"value": "false"}}
			]`,
			fuzzers: []string{"tint_wgsl_fuzzer"},
			wantErr: false,
		},
		{
			name: "Missing use_clang_coverage",
			gnJson: `[
				{"name": "use_libfuzzer", "current": {"value": "true"}},
				{"name": "tint_build_wgsl_reader", "current": {"value": "true"}},
				{"name": "optimize_for_fuzzing", "current": {"value": "false"}}
			]`,
			fuzzers: []string{"tint_wgsl_fuzzer"},
			wantErr: true,
		},
		{
			name: "optimize_for_fuzzing is true",
			gnJson: `[
				{"name": "use_libfuzzer", "current": {"value": "true"}},
				{"name": "tint_build_wgsl_reader", "current": {"value": "true"}},
				{"name": "use_clang_coverage", "current": {"value": "true"}},
				{"name": "optimize_for_fuzzing", "current": {"value": "true"}}
			]`,
			fuzzers: []string{"tint_wgsl_fuzzer"},
			wantErr: true,
		},
		{
			name: "Correct IR flags",
			gnJson: `[
				{"name": "use_libfuzzer", "current": {"value": "true"}},
				{"name": "tint_build_wgsl_reader", "current": {"value": "true"}},
				{"name": "use_clang_coverage", "current": {"value": "true"}},
				{"name": "optimize_for_fuzzing", "current": {"value": "false"}},
				{"name": "tint_build_ir_binary", "current": {"value": "true"}},
				{"name": "tint_has_protobuf", "current": {"value": "true"}}
			]`,
			fuzzers: []string{"tint_ir_fuzzer"},
			wantErr: false,
		},
		{
			name: "Missing IR protobuf flag",
			gnJson: `[
				{"name": "use_libfuzzer", "current": {"value": "true"}},
				{"name": "tint_build_wgsl_reader", "current": {"value": "true"}},
				{"name": "use_clang_coverage", "current": {"value": "true"}},
				{"name": "optimize_for_fuzzing", "current": {"value": "false"}},
				{"name": "tint_build_ir_binary", "current": {"value": "true"}}
			]`,
			fuzzers: []string{"tint_ir_fuzzer"},
			wantErr: true,
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			fs := oswrapper.CreateFSTestOSWrapper()
			ew := execwrapper.NewTestExecWrapperForSuccess([]byte(tc.gnJson), nil)

			cfg := &taskConfig{
				mainConfig: mainConfig{
					build:       "/build",
					osWrapper:   fs,
					execWrapper: ew,
				},
			}

			_, err := checkGnArgs(cfg, tc.fuzzers)
			if tc.wantErr {
				require.Error(t, err)
			} else {
				require.NoError(t, err)
			}
		})
	}
}

func TestGenerateTasksForFuzzer(t *testing.T) {
	secs := 3600
	runs := 100000
	customIters := 2

	corpus := CorpusDef{Name: "tint_tests", Path: "tint_tests_dir"}

	settings := &ExperimentSettings{
		DefaultIterations: 5,
		Durations: []DurationDef{
			{Seconds: &secs},
			{Runs: &runs, Iterations: &customIters},
		},
	}

	tasks, err := calculateTasksForFuzzer("tint_wgsl_fuzzer", corpus, "/root/corpora", "/root/results/default", settings)

	require.NoError(t, err)
	require.Len(t, tasks, 7)

	require.Equal(t, "tint_wgsl_fuzzer", tasks[0].FuzzerName)
	require.Equal(t, "tint_tests", tasks[0].CorpusName)
	require.Equal(t, filepath.Join("/root/corpora", "tint_tests_dir"), tasks[0].CorpusPath)
	require.Equal(t, ExperimentLimitTypeSeconds, tasks[0].LimitType)
	require.Equal(t, 3600, tasks[0].LimitValue)
	require.Equal(t, 1, tasks[0].Iteration)
	require.Equal(t, filepath.Join("/root/results/default", "tint_wgsl_fuzzer", "tint_tests", "seconds_3600", "iter_1"), tasks[0].TaskDir)

	lastIdx := len(tasks) - 1
	require.Equal(t, ExperimentLimitTypeRuns, tasks[lastIdx].LimitType)
	require.Equal(t, 100000, tasks[lastIdx].LimitValue)
	require.Equal(t, 2, tasks[lastIdx].Iteration)
	require.Equal(t, filepath.Join("/root/results/default", "tint_wgsl_fuzzer", "tint_tests", "runs_100000", "iter_2"), tasks[lastIdx].TaskDir)
}

func TestLoadExperimentSettings(t *testing.T) {
	tests := []struct {
		name        string
		setupFS     func(fs oswrapper.FSTestOSWrapper)
		wantErr     bool
		errContains string
		validate    func(t *testing.T, settings ExperimentSettings)
	}{
		{
			name: "Valid experiment config",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.MkdirAll("/root/corpora/test_corp", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10},
						{"runs": 1000}
					]
				}`), 0644)
			},
			wantErr: false,
			validate: func(t *testing.T, settings ExperimentSettings) {
				require.Nil(t, settings.BenchmarkDuration)
			},
		},
		{
			name: "Valid experiment config with benchmark duration",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.MkdirAll("/root/corpora/test_corp", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10},
						{"runs": 1000}
					],
					"benchmark_duration": 45
				}`), 0644)
			},
			wantErr: false,
			validate: func(t *testing.T, settings ExperimentSettings) {
				require.NotNil(t, settings.BenchmarkDuration)
				require.Equal(t, 45, *settings.BenchmarkDuration)
			},
		},
		{
			name: "Valid experiment config with burn-in duration and enabled",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.MkdirAll("/root/corpora/test_corp", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					],
					"burnin_duration": 120,
					"burnin_enabled": false
				}`), 0644)
			},
			wantErr: false,
			validate: func(t *testing.T, config ExperimentSettings) {
				require.NotNil(t, config.BurnInDuration)
				require.Equal(t, 120, *config.BurnInDuration)
				require.NotNil(t, config.BurnInEnabled)
				require.False(t, *config.BurnInEnabled)
			},
		},
		{
			name: "Invalid experiment config (both limit types set)",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.MkdirAll("/root/corpora/test_corp", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10, "runs": 1000}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "has both 'seconds' and 'runs' defined",
		},
		{
			name: "Invalid experiment config (neither limit type set)",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.MkdirAll("/root/corpora/test_corp", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"iterations": 5}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "must define either 'seconds' or 'runs'",
		},
		{
			name: "Missing config file",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
			},
			wantErr:     true,
			errContains: "experiment configuration file '/root/experiment.json' not found",
		},
		{
			name: "No fuzzers specified",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": [],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "no fuzzers specified in 'fuzzers' list",
		},
		{
			name: "Unsupported fuzzer",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["invalid_fuzzer_name"],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "unsupported fuzzer 'invalid_fuzzer_name'",
		},
		{
			name: "Missing WGSL benchmark corpus",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "wgsl_benchmark_corpus is required in experiment.json because WGSL fuzzers are specified",
		},
		{
			name: "Missing WGSL corpora",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "at least one wgsl_corpora definition is required in experiment.json because WGSL fuzzers are specified",
		},
		{
			name: "WGSL benchmark corpus directory missing",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "wgsl benchmark corpus directory 'bench' not found under corpora root",
		},
		{
			name: "WGSL corpus directory missing",
			setupFS: func(fs oswrapper.FSTestOSWrapper) {
				_ = fs.MkdirAll("/root/corpora/bench", 0755)
				_ = fs.WriteFile("/root/experiment.json", []byte(`{
					"name": "test_exp",
					"hash": "abcdef",
					"fuzzers": ["tint_wgsl_fuzzer"],
					"wgsl_benchmark_corpus": "bench",
					"wgsl_corpora": [
						{"name": "corp1", "path": "test_corp"}
					],
					"default_iterations": 2,
					"durations": [
						{"seconds": 10}
					]
				}`), 0644)
			},
			wantErr:     true,
			errContains: "wgsl corpus directory 'test_corp' not found under corpora root",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			fs := oswrapper.CreateFSTestOSWrapper()
			tc.setupFS(fs)

			cfg := &taskConfig{
				mainConfig: mainConfig{
					osWrapper: fs,
				},
			}

			settings, err := loadExperimentSettings(cfg, "/root")
			if tc.wantErr {
				require.Error(t, err)
				require.Contains(t, err.Error(), tc.errContains)
			} else {
				require.NoError(t, err)
				require.Equal(t, "test_exp", settings.Name)
				if tc.validate != nil {
					tc.validate(t, settings)
				}
			}
		})
	}
}
