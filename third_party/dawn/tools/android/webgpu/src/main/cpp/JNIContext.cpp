// Copyright 2024 The Dawn & Tint Authors
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

#include "JNIContext.h"

namespace dawn::kotlin_api {

JNIContext::JNIContext(JNIEnv* env) : env(env) {
    env->GetJavaVM(&jvm);
}

JNIContext::~JNIContext() {
    for (auto [s, utf] : mStringsToRelease) {
        env->ReleaseStringUTFChars(s, utf);
    }

    // Free int arrays with JNI_ABORT because we never modify their contents and don't need to copy
    // modifications back in the JVM's array (if a copy was done in GetIntArrayElements).
    for (auto [array, ints] : mIntArraysToRelease) {
        env->ReleaseIntArrayElements(array, ints, JNI_ABORT);
    }

    for (auto allocation : mAllocationsToFree) {
        delete allocation;
    }
    for (auto allocation : mArrayAllocationsToFree) {
        delete[] allocation;
    }
}

const char* JNIContext::GetStringUTFChars(jstring s) {
    const char* utf = env->GetStringUTFChars(s, nullptr);
    mStringsToRelease.emplace_back(s, utf);
    return utf;
}

const jint* JNIContext::GetIntArrayElements(jintArray a) {
    jint* ints = env->GetIntArrayElements(a, nullptr);
    mIntArraysToRelease.emplace_back(a, ints);
    return ints;
}

}  // namespace dawn::kotlin_api
