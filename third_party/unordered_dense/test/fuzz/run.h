#pragma once

#include <fuzz/provider.h>

#include <functional>

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
extern "C" {
void HF_ITER(const uint8_t** buf_ptr, size_t* len_ptr);
}
#endif

namespace fuzz {

namespace detail {

void evaluate_corpus(std::function<void(provider)> const& op);

} // namespace detail

/**
 * There are 2 modes how this the op() will be executed:
 *
 * Driven by honggfuzz: this is enabled when compiling with -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION.
 * This is done to fuzz a particular test.
 *
 * Otherwise, this is run in "corpus" mode, where all files in a directory named by the testname are evaluated
 * This should be done in normal unit testing. The location of the corpus base directory is determined in this order:
 * 1. Use FUZZ_CORPUS_BASE_DIR environment variable
 * 2. If this is not set, look in the working directory for a ".fuzz-corpus-base-dir" file which should contain
 *    the path to the base directory (relative to that particular file)
 */
template <typename Op>
void run(Op const& op) {
#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    size_t len = 0;
    uint8_t const* buf = nullptr;
    while (true) {
        ::HF_ITER(&buf, &len);
        op(provider(buf, len));
    }
#else
    detail::evaluate_corpus(op);
#endif
}

} // namespace fuzz
