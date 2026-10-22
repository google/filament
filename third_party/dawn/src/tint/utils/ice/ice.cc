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
#include <sstream>
#include <string>

#include "src/tint/utils/ice/debugger.h"
#include "src/tint/utils/macros/compiler.h"

namespace tint {

InternalCompilerError::InternalCompilerError(const char* file,
                                             size_t line,
                                             InternalCompilerErrorCallback callback)
    : file_(file), line_(line), callback_info_(callback) {}

TINT_BEGIN_DISABLE_WARNING(DESTRUCTOR_NEVER_RETURNS);
InternalCompilerError::~InternalCompilerError() {
    std::ostringstream err;
    err << Error();
    err << R"(

********************************************************************
*  The tint shader compiler has encountered an unexpected error.   *
*                                                                  *
*  Please help us fix this issue by submitting a bug report at     *
*  crbug.com/tint with the source program that triggered the bug.  *
********************************************************************
)";

    // When consuming the ICE, log the error message and never return.
    // Default to stderr unless an ICE callback is provided.
    if (callback_info_ && callback_info_->callback) {
        callback_info_->callback(err.str(), callback_info_->userdata);
    } else {
        std::cerr << err.str();
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
