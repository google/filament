/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <utils/sstream.h>

using namespace utils::io;

TEST(sstream, EmptyString) {
    sstream ss;
    EXPECT_STREQ("", ss.c_str());
}

TEST(sstream, Formatting) {
    {
        sstream ss;
        ss << (short) 32767;
        EXPECT_STREQ("32767", ss.c_str());
    }
    {
        sstream ss;
        ss << (unsigned short) 65535;
        EXPECT_STREQ("65535", ss.c_str());
    }
    {
        sstream ss;
        ss << (char) 'A';
        EXPECT_STREQ("A", ss.c_str());
    }
    {
        sstream ss;
        ss << (unsigned char) 'B';
        EXPECT_STREQ("B", ss.c_str());
    }
    {
        sstream ss;
        ss << (int) 2147483647;
        EXPECT_STREQ("2147483647", ss.c_str());
    }
    {
        sstream ss;
        ss << (unsigned int) 4294967295;
        EXPECT_STREQ("4294967295", ss.c_str());
    }
    {
        sstream ss;
        ss << (long) -2147483647;
        EXPECT_STREQ("-2147483647", ss.c_str());
    }
    {
        sstream ss;
        ss << (unsigned long) 4294967295;
        EXPECT_STREQ("4294967295", ss.c_str());
    }
    {
        sstream ss;
        ss << (long long) 9223372036854775807;
        EXPECT_STREQ("9223372036854775807", ss.c_str());
    }
    {
        sstream ss;
        ss << (unsigned long long) 18446744073709551615u;
        EXPECT_STREQ("18446744073709551615", ss.c_str());
    }
    {
        sstream ss;
        ss << (float) 3.14;
        EXPECT_STREQ("3.140000", ss.c_str());
    }
    {
        sstream ss;
        ss << (double) -1;
        EXPECT_STREQ("-1.000000", ss.c_str());
    }
    {
        sstream ss;
        ss << (long double) 1;
        EXPECT_STREQ("1.000000", ss.c_str());
    }
    {
        sstream ss;
        ss << (bool) true;
        EXPECT_STREQ("1", ss.c_str());
    }
    {
        sstream ss;
        ss << (const char *) "hello";
        EXPECT_STREQ("hello", ss.c_str());
    }
    {
        sstream ss;
        ss << (const unsigned char *) "world";
        EXPECT_STREQ("world", ss.c_str());
    }
}

TEST(sstream, LargeBuffer) {
    sstream ss;

    const char* filler = "1234567890ABCDEF";

    // Fill the buffer with 1024 * 1024 * 16 bytes (~16MB).
    for (size_t i = 0; i < 1024 * 1024; i++) {
        ss << filler;
    }

    EXPECT_EQ(1024 * 1024 * 16, strlen(ss.c_str()));
}

TEST(sstream, LargeString) {
    sstream ss;

    // Create a 1GB C-string.
    constexpr size_t size = 1024 * 1024 * 1024;
    char* filler = (char*) malloc(size + 1);
    std::fill_n(filler, size, 'A');
    filler[size] = 0;   // null-terminator

    ss << filler;

    EXPECT_EQ(size, strlen(ss.c_str()));
    EXPECT_STREQ(filler, ss.c_str());

    free(filler);
}

TEST(sstream, SeveralStrings) {
    sstream ss;

    // Create a 1KB C-string.
    constexpr size_t sizeA = 1024;
    char* fillerA = (char*) malloc(sizeA + 1);
    std::fill_n(fillerA, sizeA, 'A');
    fillerA[sizeA] = 0;   // null-terminator

    // Create a 7KB C-string.
    constexpr size_t sizeB = 7 * 1024;
    char* fillerB = (char*) malloc(sizeB + 1);
    std::fill_n(fillerB, sizeB, 'B');
    fillerB[sizeB] = 0;

    ss << fillerA;
    ss << fillerB;

    EXPECT_EQ(sizeA + sizeB, strlen(ss.c_str()));

    free(fillerA);
    free(fillerB);
}
