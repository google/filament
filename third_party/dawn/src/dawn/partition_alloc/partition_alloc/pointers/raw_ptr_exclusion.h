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

#ifndef SRC_DAWN_PARTITION_ALLOC_PARTITION_ALLOC_POINTERS_RAW_PTR_EXCLUSION_H_
#define SRC_DAWN_PARTITION_ALLOC_PARTITION_ALLOC_POINTERS_RAW_PTR_EXCLUSION_H_

// Marks a field as excluded from the `raw_ptr<T>` usage enforcement via
// Chromium Clang plugin.
//
// Example:
//     RAW_PTR_EXCLUSION Foo* foo_;
//
// `RAW_PTR_EXCLUSION` should be avoided, as exclusions makes it significantly easier for any bug
// involving the pointer to become a security vulnerability. For additional guidance please see the
// "When to use raw_ptr<T>" section of:
// https://source.chromium.org/chromium/chromium/src/+/main:base/memory/raw_ptr.md
#define RAW_PTR_EXCLUSION

#endif  // SRC_DAWN_PARTITION_ALLOC_PARTITION_ALLOC_POINTERS_RAW_PTR_EXCLUSION_H_
