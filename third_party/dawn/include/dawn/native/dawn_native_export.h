// Copyright 2018 The Dawn & Tint Authors
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

#ifndef INCLUDE_DAWN_NATIVE_DAWN_NATIVE_EXPORT_H_
#define INCLUDE_DAWN_NATIVE_DAWN_NATIVE_EXPORT_H_

#if defined(DAWN_NATIVE_SHARED_LIBRARY)
#if defined(_WIN32)
#if defined(DAWN_NATIVE_IMPLEMENTATION)
#define DAWN_NATIVE_EXPORT __declspec(dllexport)
#else
#define DAWN_NATIVE_EXPORT __declspec(dllimport)
#endif
#else  // defined(_WIN32)
#if defined(DAWN_NATIVE_IMPLEMENTATION)
#define DAWN_NATIVE_EXPORT __attribute__((visibility("default")))
#else
#define DAWN_NATIVE_EXPORT
#endif
#endif  // defined(_WIN32)
#else   // defined(DAWN_NATIVE_SHARED_LIBRARY)
#define DAWN_NATIVE_EXPORT
#endif  // defined(DAWN_NATIVE_SHARED_LIBRARY)

#endif  // INCLUDE_DAWN_NATIVE_DAWN_NATIVE_EXPORT_H_
