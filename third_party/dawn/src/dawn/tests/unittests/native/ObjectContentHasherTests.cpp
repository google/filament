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

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/ObjectContentHasher.h"
#include "dawn/tests/DawnNativeTest.h"

namespace dawn::native {
namespace {

class ObjectContentHasherTests : public DawnNativeTest {};

#define EXPECT_IF_HASH_EQ(eq, a, b)                                \
    do {                                                           \
        ObjectContentHasher ra, rb;                                \
        ra.Record(a);                                              \
        rb.Record(b);                                              \
        EXPECT_EQ(eq, ra.GetContentHash() == rb.GetContentHash()); \
    } while (0)

TEST(ObjectContentHasherTests, Pair) {
    EXPECT_IF_HASH_EQ(true, (std::pair<std::string, uint8_t>{"a", 1}),
                      (std::pair<std::string, uint8_t>{"a", 1}));
    EXPECT_IF_HASH_EQ(false, (std::pair<uint8_t, std::string>{1, "a"}),
                      (std::pair<std::string, uint8_t>{"a", 1}));
    EXPECT_IF_HASH_EQ(false, (std::pair<std::string, uint8_t>{"a", 1}),
                      (std::pair<std::string, uint8_t>{"a", 2}));
    EXPECT_IF_HASH_EQ(false, (std::pair<std::string, uint8_t>{"a", 1}),
                      (std::pair<std::string, uint8_t>{"b", 1}));
}

TEST(ObjectContentHasherTests, Vector) {
    EXPECT_IF_HASH_EQ(true, (std::vector<uint8_t>{0, 1}), (std::vector<uint8_t>{0, 1}));
    EXPECT_IF_HASH_EQ(false, (std::vector<uint8_t>{0, 1}), (std::vector<uint8_t>{0, 1, 2}));
    EXPECT_IF_HASH_EQ(false, (std::vector<uint8_t>{0, 1}), (std::vector<uint8_t>{1, 0}));
    EXPECT_IF_HASH_EQ(false, (std::vector<uint8_t>{0, 1}), (std::vector<uint8_t>{}));
    EXPECT_IF_HASH_EQ(false, (std::vector<uint8_t>{0, 1}), (std::vector<float>{0, 1}));
}

TEST(ObjectContentHasherTests, Map) {
    EXPECT_IF_HASH_EQ(true, (std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}}),
                      (std::map<std::string, uint8_t>{{"b", 2}, {"a", 1}}));
    EXPECT_IF_HASH_EQ(false, (std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}}),
                      (std::map<std::string, uint8_t>{{"a", 2}, {"b", 1}}));
    EXPECT_IF_HASH_EQ(false, (std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}}),
                      (std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}, {"c", 1}}));
    EXPECT_IF_HASH_EQ(false, (std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}}),
                      (std::map<std::string, uint8_t>{}));
}

TEST(ObjectContentHasherTests, HashCombine) {
    ObjectContentHasher ra, rb;

    ra.Record(std::vector<uint8_t>{0, 1});
    ra.Record(std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}});

    rb.Record(std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}});
    rb.Record(std::vector<uint8_t>{0, 1});

    EXPECT_NE(ra.GetContentHash(), rb.GetContentHash());
}

}  // namespace
}  // namespace dawn::native
