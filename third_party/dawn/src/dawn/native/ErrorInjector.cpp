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

#include "dawn/native/ErrorInjector.h"

#include "dawn/common/Assert.h"
#include "dawn/native/DawnNative.h"

namespace dawn::native {

namespace {

bool sIsEnabled = false;
uint64_t sNextIndex = 0;
uint64_t sInjectedFailureIndex = 0;
bool sHasPendingInjectedError = false;

}  // anonymous namespace

void EnableErrorInjector() {
    sIsEnabled = true;
}

void DisableErrorInjector() {
    sIsEnabled = false;
}

void ClearErrorInjector() {
    sNextIndex = 0;
    sHasPendingInjectedError = false;
}

bool ErrorInjectorEnabled() {
    return sIsEnabled;
}

uint64_t AcquireErrorInjectorCallCount() {
    uint64_t count = sNextIndex;
    ClearErrorInjector();
    return count;
}

bool ShouldInjectError() {
    uint64_t index = sNextIndex++;
    if (sHasPendingInjectedError && index == sInjectedFailureIndex) {
        sHasPendingInjectedError = false;
        return true;
    }
    return false;
}

void InjectErrorAt(uint64_t index) {
    // Only one error can be injected at a time.
    DAWN_ASSERT(!sHasPendingInjectedError);
    sInjectedFailureIndex = index;
    sHasPendingInjectedError = true;
}

}  // namespace dawn::native
