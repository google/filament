// Copyright 2023 The Dawn & Tint Authors
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
#include "src/dawn/tests/PartitionAllocSupport.h"

#include "src/utils/assert.h"
#include "src/utils/log.h"

#if defined(DAWN_ENABLE_PARTITION_ALLOC)
#include <array>
#include <span>
#include <sstream>
#include <utility>

#include "absl/debugging/stacktrace.h"
#include "absl/debugging/symbolize.h"
#include "partition_alloc/dangling_raw_ptr_checks.h"  // nogncheck
#include "src/dawn/common/MutexProtected.h"
// TODO(https://crbug.com/1505382): Enforce those warning inside PartitionAlloc.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-statement-expression-from-macro-expansion"
#pragma GCC diagnostic ignored "-Wzero-length-array"
#include "partition_alloc/shim/allocator_shim.h"                                      // nogncheck
#include "partition_alloc/shim/allocator_shim_default_dispatch_to_partition_alloc.h"  // nogncheck
#pragma GCC diagnostic pop
#endif

namespace dawn {

void InitializePartitionAllocForTesting() {
#if defined(DAWN_ENABLE_PARTITION_ALLOC)
    allocator_shim::ConfigurePartitionsForTesting();
#endif
}

void InitializeDanglingPointerDetectorForTesting() {
#if defined(DAWN_ENABLE_PARTITION_ALLOC)

    using StackTraceStorage = std::array<void*, 64>;

    struct StackTraceEntry {
        uintptr_t ptr = 0;
        StackTraceStorage pcs = {};
        size_t depth = 0;
    };
    static MutexProtected<std::array<StackTraceEntry, 20>> g_dangling_stack_traces;

    // Called from the allocator during "free". This callback isn't allowed to allocate memory to
    // avoid reentrancy issues.
    partition_alloc::SetDanglingRawPtrDetectedFn([](uintptr_t ptr) {
        StackTraceEntry candidate;
        candidate.ptr = ptr;
        candidate.depth = absl::GetStackTrace(candidate.pcs.data(), candidate.pcs.size(), 1);

        g_dangling_stack_traces.Use([&](auto stack_traces) {
            for (auto& entry : *stack_traces) {
                if (entry.ptr == 0) {
                    std::swap(entry, candidate);
                    break;
                }
            }
        });
    });

    // This function is allowed to allocate memory.
    partition_alloc::SetDanglingRawPtrReleasedFn([](uintptr_t ptr) {
        StackTraceEntry free_entry;

        // Find the StackTrace when "free" was called and the pointer becomes dangling.
        g_dangling_stack_traces.Use([&](auto stack_traces) {
            for (auto& entry : *stack_traces) {
                if (entry.ptr == ptr) {
                    std::swap(entry, free_entry);
                    break;
                }
            }
        });

        std::stringstream error;

        error << "\n-----------\n";
        auto PrintStack = [&](std::span<void* const> pcs) {
            for (size_t i = 0; i < pcs.size(); ++i) {
                std::array<char, 1024> symbol;
                if (absl::Symbolize(pcs[i], symbol.data(), symbol.size())) {
                    error << "\n    #" << i << " " << pcs[i] << " " << symbol.data();
                } else {
                    error << "\n    #" << i << " " << pcs[i] << " (unknown)";
                }
            }
        };

        error << "DanglingPointerDetector: A pointer was dangling!\n"
                 "Documentation: "
                 "https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/"
                 "docs/dangling-pointer-detector.md";

        if (free_entry.ptr != 0 && free_entry.depth > 0) {
            error << "\n\nMemory was freed at:";
            PrintStack(std::span(free_entry.pcs).first(free_entry.depth));
        } else {
            error << "\n\nThe free stack trace wasn't recorded";
        }

        error << "\n\nDangling raw_ptr was released at:";
        StackTraceStorage release_pcs;
        size_t depth = absl::GetStackTrace(release_pcs.data(), release_pcs.size(), 1);
        PrintStack(std::span(release_pcs).first(depth));

        error << "\n-----------\n";

        ErrorLog() << error.str();

        DAWN_UNREACHABLE();
    });
#endif
}

}  // namespace dawn
