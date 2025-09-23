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

#ifndef SRC_DAWN_NODE_INTEROP_NODEAPI_H_
#define SRC_DAWN_NODE_INTEROP_NODEAPI_H_

#include "src/utils/compiler.h"

// Dawn is built with exceptions disabled.
#define NAPI_DISABLE_CPP_EXCEPTIONS

// Disable warnings produced by Node API header. This is done here rather than in the build file to
// scope the disablement to just this header (since we can't use -isystem everywhere), and to avoid
// ordering issues between -Wsomething and -Wno-something.
#if DAWN_COMPILER_IS_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-semi"
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#pragma clang diagnostic ignored "-Winconsistent-missing-destructor-override"
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wshadow-field"
#pragma clang diagnostic ignored "-Wsuggest-destructor-override"
#endif  // DAWN_COMPILER_IS_CLANG

#include <napi.h>

#if DAWN_COMPILER_IS_CLANG
#pragma clang diagnostic pop
#endif  // DAWN_COMPILER_IS_CLANG

#endif  // SRC_DAWN_NODE_INTEROP_NODEAPI_H_
