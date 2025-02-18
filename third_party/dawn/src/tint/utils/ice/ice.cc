// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/utils/ice/ice.h"

#include <iostream>
#include <string>

#include "src/tint/utils/ice/debugger.h"
#include "src/tint/utils/macros/compiler.h"

namespace tint {
namespace {

InternalCompilerErrorReporter* ice_reporter = nullptr;

}  // namespace

void SetInternalCompilerErrorReporter(InternalCompilerErrorReporter* reporter) {
    ice_reporter = reporter;
}

InternalCompilerError::InternalCompilerError(const char* file, size_t line)
    : file_(file), line_(line) {}

TINT_BEGIN_DISABLE_WARNING(DESTRUCTOR_NEVER_RETURNS);
InternalCompilerError::~InternalCompilerError() {
    if (ice_reporter) {
        ice_reporter(*this);
    } else {
        std::cerr << Error() << "\n\n";
    }

    debugger::Break();

#if defined(_MSC_VER) && !defined(__clang__)
    abort();
#else
    __builtin_trap();
#endif
}
TINT_END_DISABLE_WARNING(DESTRUCTOR_NEVER_RETURNS);

std::string InternalCompilerError::Error() const {
    return std::string(File()) + ":" + std::to_string(Line()) +
           " internal compiler error: " + Message();
}

}  // namespace tint
