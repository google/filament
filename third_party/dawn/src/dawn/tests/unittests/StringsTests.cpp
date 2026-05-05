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

#include <gtest/gtest.h>

#include "dawn/common/Strings.h"

#define STATIC_EXPECT_STREQ(x, y) static_assert(std::string_view(x) == std::string_view(y))

// DAWN_MULTILINE tests
STATIC_EXPECT_STREQ("", DAWN_MULTILINE());
STATIC_EXPECT_STREQ("", DAWN_MULTILINE(  ));
STATIC_EXPECT_STREQ("", DAWN_MULTILINE(
));
STATIC_EXPECT_STREQ("foo", DAWN_MULTILINE(foo));
STATIC_EXPECT_STREQ("foo bar", DAWN_MULTILINE(  foo   bar  ));
STATIC_EXPECT_STREQ("foo ( bar ) { baz ; { bu , zz } }", DAWN_MULTILINE(  foo  (  bar  ) { baz ; { bu , zz } } ));
STATIC_EXPECT_STREQ("foo (bar) { baz; { bu, zz } }", DAWN_MULTILINE(  foo  (bar) { baz; { bu, zz } } ));
STATIC_EXPECT_STREQ("foo { bar", DAWN_MULTILINE( foo { bar));
STATIC_EXPECT_STREQ("foo bar }", DAWN_MULTILINE( foo bar }));
STATIC_EXPECT_STREQ("@fragment fn fs_empty_main ( ) { 2 / 3 }",
                    DAWN_MULTILINE(
    // comment
    // comment
    @fragment  // comment
    // comment
    // comment
    fn fs_empty_main  ( )  {  2 / 3  }
    // comment
    // comment
));
