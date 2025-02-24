//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// JsonSerializer_unittests.cpp: Unit tests for the JSON based serializer
//

#if !defined(ANGLE_HAS_RAPIDJSON)
#    error RapidJSON must be available to build this file.
#endif  // !defined(ANGLE_HAS_RAPIDJSON)

#include "JsonSerializer.h"

#include <gtest/gtest.h>

class JsonSerializerTest : public ::testing::Test
{
  protected:
    void SetUp() override;
    void check(const std::string &expect);

    angle::JsonSerializer js;
};

// Test writing one integer value
TEST_F(JsonSerializerTest, NamedIntValue1)
{
    js.addScalar("test1", 1);

    const std::string expect =
        R"({
    "context": {
        "test1": 1
    }
})";
    check(expect);
}

// Test writing one long value
TEST_F(JsonSerializerTest, NamedLongValue)
{
    long v = -12;
    js.addScalar("test1", v);

    const std::string expect =
        R"({
    "context": {
        "test1": -12
    }
})";
    check(expect);
}

// Test writing one unsigned long value
TEST_F(JsonSerializerTest, NamedULongValue)
{
    unsigned long v = 12;
    js.addScalar("test1", v);

    const std::string expect =
        R"({
    "context": {
        "test1": 12
    }
})";
    check(expect);
}

// Test writing another integer value
TEST_F(JsonSerializerTest, NamedIntValue2)
{
    js.addScalar("test2", 2);

    const std::string expect =
        R"({
    "context": {
        "test2": 2
    }
})";

    check(expect);
}

// Test writing one string value
TEST_F(JsonSerializerTest, NamedStringValue)
{
    js.addCString("test2", "value");

    const std::string expect =
        R"({
    "context": {
        "test2": "value"
    }
})";

    check(expect);
}

// Test writing one byte array
// Since he serialiter is only used for testing we don't store
// the whole byte array, but only it's SHA1 checksum
TEST_F(JsonSerializerTest, ByteArrayValue)
{
    const uint8_t value[5] = {10, 0, 0xcc, 0xff, 0xaa};
    js.addBlob("test2", value, 5);

    const std::string expect =
        R"({
    "context": {
        "test2-hash": "SHA1:4315724B1AB1EB2C0128E8E9DAD6D76254BA711D",
        "test2-raw[0-4]": [
            10,
            0,
            204,
            255,
            170
        ]
    }
})";

    check(expect);
}

// Test writing one vector of integer values
TEST_F(JsonSerializerTest, IntVectorValue)
{
    std::vector<int> v = {0, 1, -1};

    js.addVector("test2", v);

    const std::string expect =
        R"({
    "context": {
        "test2": [
            0,
            1,
            -1
        ]
    }
})";

    check(expect);
}

// Test writing one vector of integer values
TEST_F(JsonSerializerTest, IntVectorAsBlobValue)
{
    std::vector<int> v = {0, 1, -1};

    js.addVectorAsHash("test2", v);
    const std::string expect =
        R"({
    "context": {
        "test2-hash": "SHA1:6216A439C16A113E2F1E53AB63FB88877D3597F5",
        "test2-raw[0-11]": [
            0,
            0,
            0,
            0,
            1,
            0,
            0,
            0,
            255,
            255,
            255,
            255
        ]
    }
})";
    check(expect);
}

// Test unsorted input gets sorted
TEST_F(JsonSerializerTest, SortValues1)
{
    js.addScalar("b", 1.0);
    js.addScalar("a", 2.0);
    const std::string expect =
        R"({
    "context": {
        "a": 2.0,
        "b": 1.0
    }
})";
    check(expect);
}

// Test writing one vector of short integer values
TEST_F(JsonSerializerTest, ShortVectorAsBlobValue)
{
    std::vector<short> v = {0, 1, -1};

    js.addVectorAsHash("test2", v);
    const std::string expect =
        R"({
    "context": {
        "test2-hash": "SHA1:0BA7C0DE700CE0F8018D084B8CF447B150A9465D",
        "test2-raw[0-5]": [
            0,
            0,
            1,
            0,
            255,
            255
        ]
    }
})";
    check(expect);
}

// Test adding the same key twice
TEST_F(JsonSerializerTest, KeyUsedTwice)
{
    js.addScalar("a", 1.0);
    js.addScalar("a", 1.0);

    const std::string expect =
        R"({
    "context": {
        "a": 1.0,
        "a": 1.0
    }
})";

    check(expect);
}

// Test writing boolean values
TEST_F(JsonSerializerTest, NamedBoolValues)
{
    js.addScalar("test_false", false);
    js.addScalar("test_true", true);

    const std::string expect =
        R"({
    "context": {
        "test_false": false,
        "test_true": true
    }
})";

    check(expect);
}

// test writing two values in a sub-group
TEST_F(JsonSerializerTest, GroupedIntValue)
{
    js.startGroup("group");
    js.addScalar("test1", 1);
    js.addScalar("test2", 2);
    js.endGroup();

    const std::string expect =
        R"({
    "context": {
        "group": {
            "test1": 1,
            "test2": 2
        }
    }
})";

    check(expect);
}

void JsonSerializerTest::SetUp()
{
    js.startGroup("context");
}

void JsonSerializerTest::check(const std::string &expect)
{
    js.endGroup();
    EXPECT_EQ(js.data(), expect);
    EXPECT_EQ(js.length(), expect.length());
    std::vector<uint8_t> expectAsUbyte(expect.begin(), expect.end());
    EXPECT_EQ(js.getData(), expectAsUbyte);
}
