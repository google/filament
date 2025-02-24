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

#include "src/tint/utils/text/string_stream.h"

#include <math.h>
#include <cstring>
#include <limits>

#include "gtest/gtest.h"

#include "src/tint/utils/rtti/traits.h"

namespace tint::utils {
namespace {

static_assert(traits::IsOStream<StringStream>);

using StringStreamTest = testing::Test;

TEST_F(StringStreamTest, Zero) {
    StringStream s;
    s << 0.0f;
    EXPECT_EQ(s.str(), "0.0");
}

TEST_F(StringStreamTest, One) {
    StringStream s;
    s << 1.0f;
    EXPECT_EQ(s.str(), "1.0");
}

TEST_F(StringStreamTest, MinusOne) {
    StringStream s;
    s << -1.0f;
    EXPECT_EQ(s.str(), "-1.0");
}

TEST_F(StringStreamTest, Billion) {
    StringStream s;
    s << 1e9f;
    EXPECT_EQ(s.str(), "1000000000.0");
}

TEST_F(StringStreamTest, Small) {
    StringStream s;
    s << std::numeric_limits<float>::epsilon();
    EXPECT_NE(s.str(), "0.0");
}

TEST_F(StringStreamTest, Highest) {
    const auto highest = std::numeric_limits<float>::max();
    const auto expected_highest = 340282346638528859811704183484516925440.0f;

    if (highest < expected_highest || highest > expected_highest) {
        GTEST_SKIP() << "std::numeric_limits<float>::max() is not as expected for "
                        "this target";
    }

    StringStream s;
    s << std::numeric_limits<float>::max();
    EXPECT_EQ(s.str(), "340282346638528859811704183484516925440.0");
}

TEST_F(StringStreamTest, Lowest) {
    // Some compilers complain if you test floating point numbers for equality.
    // So say it via two inequalities.
    const auto lowest = std::numeric_limits<float>::lowest();
    const auto expected_lowest = -340282346638528859811704183484516925440.0f;
    if (lowest < expected_lowest || lowest > expected_lowest) {
        GTEST_SKIP() << "std::numeric_limits<float>::lowest() is not as expected for "
                        "this target";
    }

    StringStream s;
    s << std::numeric_limits<float>::lowest();
    EXPECT_EQ(s.str(), "-340282346638528859811704183484516925440.0");
}

TEST_F(StringStreamTest, Precision) {
    {
        StringStream s;
        s << 1e-8f;
        EXPECT_EQ(s.str(), "0.00000000999999993923");
    }
    {
        StringStream s;
        s << 1e-9f;
        EXPECT_EQ(s.str(), "0.00000000099999997172");
    }
    {
        StringStream s;
        s << 1e-10f;
        EXPECT_EQ(s.str(), "0.00000000010000000134");
    }
    {
        StringStream s;
        s << 1e-20f;
        EXPECT_EQ(s.str(), "0.00000000000000000001");
    }
}

}  // namespace
}  // namespace tint::utils
