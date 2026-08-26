// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_TINT_CMD_FUZZ_COMMON_RUNNER_H_
#define SRC_TINT_CMD_FUZZ_COMMON_RUNNER_H_

#include <iostream>
#include <string>
#include <thread>

#include "src/tint/cmd/fuzz/common/options.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::fuzz::common {

/// RunFuzzers is a helper function that runs a list of fuzzers.
/// @param fuzzers the list of fuzzers to run
/// @param options the options for running the fuzzers
/// @param exec the function to execute a single fuzzer. Signature: `void(const FUZZER&, size_t
/// index)`
template <typename FUZZER_LIST, typename EXEC_FN>
void RunFuzzers(const FUZZER_LIST& fuzzers, const Options& options, EXEC_FN&& exec) {
    bool ran_atleast_once = false;

    if (options.run_concurrently) {
        size_t n = fuzzers.Length();
        tint::Vector<std::thread, 32> threads;
        threads.Reserve(n);
        for (size_t i = 0; i < n; i++) {
            if (!options.filter.empty() &&
                fuzzers[i].name.find(options.filter) == std::string::npos) {
                continue;
            }
            ran_atleast_once = true;

            threads.Push(std::thread([&fuzzer = fuzzers[i], i, &exec] { exec(fuzzer, i); }));
        }
        for (auto& thread : threads) {
            thread.join();
        }
    } else {
        for (size_t i = 0; i < fuzzers.Length(); i++) {
            auto& fuzzer = fuzzers[i];
            if (!options.filter.empty() && fuzzer.name.find(options.filter) == std::string::npos) {
                continue;
            }
            ran_atleast_once = true;

            exec(fuzzer, i);
        }
    }

    if (!options.filter.empty() && !ran_atleast_once) {
        std::cerr << "ERROR: --filter=" << options.filter << " did not match any fuzzers\n";
        exit(EXIT_FAILURE);
    }
}

}  // namespace tint::fuzz::common

#endif  // SRC_TINT_CMD_FUZZ_COMMON_RUNNER_H_
