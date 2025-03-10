// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_ERRORINJECTOR_H_
#define SRC_DAWN_NATIVE_ERRORINJECTOR_H_

#include <stdint.h>
#include <type_traits>

namespace dawn::native {

template <typename ErrorType>
struct InjectedErrorResult {
    ErrorType error;
    bool injected;
};

bool ErrorInjectorEnabled();

bool ShouldInjectError();

template <typename ErrorType>
InjectedErrorResult<ErrorType> MaybeInjectError(ErrorType errorType) {
    return InjectedErrorResult<ErrorType>{errorType, ShouldInjectError()};
}

template <typename ErrorType, typename... ErrorTypes>
InjectedErrorResult<ErrorType> MaybeInjectError(ErrorType errorType, ErrorTypes... errorTypes) {
    if (ShouldInjectError()) {
        return InjectedErrorResult<ErrorType>{errorType, true};
    }
    return MaybeInjectError(errorTypes...);
}

}  // namespace dawn::native

#if defined(DAWN_ENABLE_ERROR_INJECTION)

#define INJECT_ERROR_OR_RUN(stmt, ...)                                                   \
    [&] {                                                                                \
        if (DAWN_UNLIKELY(::dawn::native::ErrorInjectorEnabled())) {                     \
            /* Only used for testing and fuzzing, so it's okay if this is deoptimized */ \
            auto injectedError = ::dawn::native::MaybeInjectError(__VA_ARGS__);          \
            if (injectedError.injected) {                                                \
                return injectedError.error;                                              \
            }                                                                            \
        }                                                                                \
        return (stmt);                                                                   \
    }()

#else

#define INJECT_ERROR_OR_RUN(stmt, ...) stmt

#endif

#endif  // SRC_DAWN_NATIVE_ERRORINJECTOR_H_
