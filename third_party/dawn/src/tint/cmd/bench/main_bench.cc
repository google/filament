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

#include <iostream>

#include "src/tint/cmd/bench/bench.h"
#include "src/tint/utils/text/string.h"

namespace {

// Toggle to use the Google Benchmark output format instead of the Chromium Perf format.
static bool use_chrome_perf_format = false;

/// ChromePerfReporter is a custom benchmark reporter used to output benchmark results in the format
/// required for the Chrome Perf waterfall, as described here:
/// [chromium]//src/tools/perf/generate_legacy_perf_dashboard_json.py
class ChromePerfReporter final : public benchmark::BenchmarkReporter {
  public:
    bool ReportContext([[maybe_unused]] const Context& context) override { return true; }

    void ReportRunsConfig([[maybe_unused]] double min_time,
                          [[maybe_unused]] bool has_explicit_iters,
                          [[maybe_unused]] benchmark::IterationCount iters) override {}

    void ReportRuns(const std::vector<Run>& report) override {
        for (auto& run : report) {
            auto fullname = run.benchmark_name();

            if (run.skipped) {
                std::cout << "FAILED " << fullname << ": " << run.skip_message << "\n";
                continue;
            }

            auto graph_and_trace = tint::Split(fullname, "/");
            TINT_ASSERT(graph_and_trace.Length() <= 2u);
            if (graph_and_trace.Length() == 1u) {
                graph_and_trace.Push(graph_and_trace[0]);
                graph_and_trace[0] = "TintInternals";
            }

            std::cout << "*RESULT " << graph_and_trace[0] << ": " << graph_and_trace[1] << "= "
                      << std::fixed << run.GetAdjustedRealTime() << " ";
            switch (run.time_unit) {
                case benchmark::kNanosecond:
                    std::cout << "ns";
                    break;
                case benchmark::kMicrosecond:
                    std::cout << "us";
                    break;
                case benchmark::kMillisecond:
                    std::cout << "ms";
                    break;
                case benchmark::kSecond:
                    std::cout << "s";
                    break;
            }
            std::cout << "\n";
        }
    }

    void Finalize() override {}
};

bool ParseExtraCommandLineArgs(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-chrome-perf-format") == 0) {
            use_chrome_perf_format = true;
        } else {
            // Accept the flags that are passed by the Chromium perf waterfall, which treats this
            // executable as a GoogleTest binary.
            if (strcmp(argv[i], "--verbose") != 0 &&
                strcmp(argv[i], "--test-launcher-print-test-stdio=always") != 0 &&
                strcmp(argv[i], "--test-launcher-total-shards=1") != 0 &&
                strcmp(argv[i], "--test-launcher-shard-index=0") != 0) {
                std::cerr << "Unrecognized command-line argument: " << argv[i] << "\n";
                return false;
            }
        }
    }
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    if (!ParseExtraCommandLineArgs(argc, argv)) {
        return 1;
    }
    if (use_chrome_perf_format) {
        benchmark::RunSpecifiedBenchmarks(new ChromePerfReporter);
    } else {
        benchmark::RunSpecifiedBenchmarks();
    }
}
