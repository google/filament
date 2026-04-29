// Copyright 2025 The Dawn & Tint Authors
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

#include <gtest/gtest.h>

#include "dawn/native/ObjectBase.h"

namespace dawn::native {
namespace {

// Default constructed ErrorMonad is initialized and not an error
TEST(ErrorMonad, DefaultConstructor) {
    ErrorMonad obj;
    ASSERT_EQ(obj.IsInitialized(), true);
    ASSERT_EQ(obj.IsError(), false);
}

// Default constructed ErrorMonad with the error tag is initialized and an error
TEST(ErrorMonad, ErrorTag) {
    ErrorMonad obj(ErrorMonad::ErrorTag{});
    ASSERT_EQ(obj.IsInitialized(), true);
    ASSERT_EQ(obj.IsError(), true);
}

// Default constructed ErrorMonad with the DelayedInitializationTag is not initialized. It can
// transition to error or no error states.
TEST(ErrorMonad, DelayedInitialization) {
    class DelayedInitErrorMonad : public ErrorMonad {
      public:
        DelayedInitErrorMonad() : ErrorMonad(DelayedInitializationTag{}) {}

        void TransitionToError() { SetInitializedError(); }

        void TransitionToNoError() { SetInitializedNoError(); }
    };

    {
        DelayedInitErrorMonad errorObj;
        ASSERT_EQ(errorObj.IsInitialized(), false);
        errorObj.TransitionToError();
        ASSERT_EQ(errorObj.IsInitialized(), true);
        ASSERT_EQ(errorObj.IsError(), true);
    }

    {
        DelayedInitErrorMonad noErrorObj;
        ASSERT_EQ(noErrorObj.IsInitialized(), false);
        noErrorObj.TransitionToError();
        ASSERT_EQ(noErrorObj.IsInitialized(), true);
        ASSERT_EQ(noErrorObj.IsError(), true);
    }
}

}  // namespace
}  // namespace dawn::native
