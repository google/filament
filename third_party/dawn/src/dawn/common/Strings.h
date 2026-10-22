// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_STRINGS_H_
#define SRC_DAWN_COMMON_STRINGS_H_

// Macro for multiline strings with "C-like" contents such as WGSL/MSL code.
//
// - Omits comments because they are removed before macro expansion.
//   (WGSL conveniently has similar comment syntax to C.)
// - Collapses sequences of whitespace into a single space.
// - Removes leading and trailing whitespace.
//
// **Important notes:**
// - Contents MUST have balanced parentheses so that the macro invocation can be
//   parsed in the first place.
// - Does NOT work on strings containing things that look like C preprocessor
//   directives (like MSL) because those get parsed before macro expansion.
// - Contents SHOULD have balanced braces, otherwise this causes issues with
//   clang-format indentation of following lines.
// - There are other bugs in clang-format relating to WhitespaceSensitiveMacros
//   and sometimes it will try to reformat the contents. Sometimes, blank
//   comments can be used to avoid bad formatting. Hopefully reformatting
//   shouldn't affect the semantics of any code.
// - Due to some bugs in clang-format, possibly relating to partially-formatting
//   files, these strings sometimes get inappropriately formatted, but hopefully
//   their actual contents shouldn't change. Sometimes blank comments can be
//   used to avoid bad formatting.
#define DAWN_MULTILINE(...) #__VA_ARGS__

#endif  // SRC_DAWN_COMMON_STRINGS_H_
