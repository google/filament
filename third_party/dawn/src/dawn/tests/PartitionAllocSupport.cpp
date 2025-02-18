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
#include "dawn/tests/PartitionAllocSupport.h"

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"

#if defined(DAWN_ENABLE_PARTITION_ALLOC)
#include "partition_alloc/dangling_raw_ptr_checks.h"
// TODO(https://crbug.com/1505382): Enforce those warning inside PartitionAlloc.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-statement-expression-from-macro-expansion"
#pragma GCC diagnostic ignored "-Wzero-length-array"
#include "partition_alloc/shim/allocator_shim.h"
#include "partition_alloc/shim/allocator_shim_default_dispatch_to_partition_alloc.h"
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
    // TODO(arthursonzogni): It would have been nice to record StackTraces from the two handlers:
    // - partition_alloc::SetDanglingRawPtrDetectedFn(ptr)
    // - partition_alloc::SetDanglingRawPtrReleasedFn(ptr)
    // Displaying them together would help developers finding where memory was deleted, and where
    // the dangling raw_ptr was. Unfortunately, Dawn doesn't have a way to display StackTraces at
    // the moment.
    //
    // We decided to crash when a dangling raw_ptr<T> get released. Finding the raw_ptr<T> is
    // usually more difficult than finding where the associated memory was released.
    partition_alloc::SetDanglingRawPtrReleasedFn([](uintptr_t ptr) {
        ErrorLog() << "DanglingPointerDetector: A pointer was dangling!";
        ErrorLog() << "                         Documentation: "
                      "https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/"
                      "docs/dangling-pointer-detector.md";
        DAWN_CHECK(false);
    });
#endif
}

}  // namespace dawn
