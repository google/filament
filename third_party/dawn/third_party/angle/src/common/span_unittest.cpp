//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// span_unittests.cpp: Unit tests for the angle::Span class.
//

#include "common/span.h"

#include <gtest/gtest.h>

using namespace angle;

namespace
{

using Span                                      = angle::Span<const unsigned int>;
constexpr size_t kSpanDataSize                  = 16;
constexpr unsigned int kSpanData[kSpanDataSize] = {0, 1, 2,  3,  4,  5,  6,  7,
                                                   8, 9, 10, 11, 12, 13, 14, 15};

// Test that comparing spans work
TEST(SpanTest, Comparison)
{
    // Duplicate data to make sure comparison is being done on values (and not addresses).
    constexpr unsigned int kSpanDataDup[kSpanDataSize] = {0, 1, 2,  3,  4,  5,  6,  7,
                                                          8, 9, 10, 11, 12, 13, 14, 15};

    // Don't use ASSERT_EQ at first because the == is more hidden
    ASSERT_TRUE(Span() == Span(kSpanData, 0));
    ASSERT_TRUE(Span(kSpanData + 3, 4) != Span(kSpanDataDup + 5, 4));

    // Check ASSERT_EQ and ASSERT_NE work correctly
    ASSERT_EQ(Span(kSpanData, kSpanDataSize), Span(kSpanDataDup, kSpanDataSize));
    ASSERT_NE(Span(kSpanData, kSpanDataSize - 1), Span(kSpanDataDup + 1, kSpanDataSize - 1));
    ASSERT_NE(Span(kSpanData, kSpanDataSize), Span(kSpanDataDup, kSpanDataSize - 1));
    ASSERT_NE(Span(kSpanData, kSpanDataSize - 1), Span(kSpanDataDup, kSpanDataSize));
    ASSERT_NE(Span(kSpanData, 0), Span(kSpanDataDup, 1));
    ASSERT_NE(Span(kSpanData, 1), Span(kSpanDataDup, 0));
}

// Test indexing
TEST(SpanTest, Indexing)
{
    constexpr Span sp(kSpanData, kSpanDataSize);

    for (size_t i = 0; i < kSpanDataSize; ++i)
    {
        ASSERT_EQ(sp[i], i);
    }

    unsigned int storage[kSpanDataSize] = {};
    angle::Span<unsigned int> writableSpan(storage, kSpanDataSize);

    for (size_t i = 0; i < kSpanDataSize; ++i)
    {
        writableSpan[i] = i;
    }
    for (size_t i = 0; i < kSpanDataSize; ++i)
    {
        ASSERT_EQ(writableSpan[i], i);
    }
    for (size_t i = 0; i < kSpanDataSize; ++i)
    {
        ASSERT_EQ(storage[i], i);
    }
}

// Test for the various constructors
TEST(SpanTest, Constructors)
{
    // Default constructor
    {
        Span sp;
        ASSERT_TRUE(sp.size() == 0);
        ASSERT_TRUE(sp.empty());
    }

    // Constexpr construct from pointer
    {
        constexpr Span sp(kSpanData, kSpanDataSize);
        ASSERT_EQ(sp.data(), kSpanData);
        ASSERT_EQ(sp.size(), kSpanDataSize);
        ASSERT_FALSE(sp.empty());
    }

    // Copy constructor and copy assignment
    {
        Span sp(kSpanData, kSpanDataSize);
        Span sp2(sp);
        Span sp3;

        ASSERT_EQ(sp, sp2);
        ASSERT_EQ(sp2.data(), kSpanData);
        ASSERT_EQ(sp2.size(), kSpanDataSize);
        ASSERT_FALSE(sp2.empty());

        sp3 = sp;

        ASSERT_EQ(sp, sp3);
        ASSERT_EQ(sp3.data(), kSpanData);
        ASSERT_EQ(sp3.size(), kSpanDataSize);
        ASSERT_FALSE(sp3.empty());
    }
}

// Test accessing the data directly
TEST(SpanTest, DataAccess)
{
    constexpr Span sp(kSpanData, kSpanDataSize);
    const unsigned int *data = sp.data();

    for (size_t i = 0; i < kSpanDataSize; ++i)
    {
        ASSERT_EQ(data[i], i);
    }
}

// Test front and back
TEST(SpanTest, FrontAndBack)
{
    constexpr Span sp(kSpanData, kSpanDataSize);
    ASSERT_TRUE(sp.front() == 0);
    ASSERT_EQ(sp.back(), kSpanDataSize - 1);
}

// Test begin and end
TEST(SpanTest, BeginAndEnd)
{
    constexpr Span sp(kSpanData, kSpanDataSize);

    size_t currentIndex = 0;
    for (unsigned int value : sp)
    {
        ASSERT_EQ(value, currentIndex);
        ++currentIndex;
    }
}

// Test reverse begin and end
TEST(SpanTest, RbeginAndRend)
{
    constexpr Span sp(kSpanData, kSpanDataSize);

    size_t currentIndex = 0;
    for (auto iter = sp.rbegin(); iter != sp.rend(); ++iter)
    {
        ASSERT_EQ(*iter, kSpanDataSize - 1 - currentIndex);
        ++currentIndex;
    }
}

// Test first and last
TEST(SpanTest, FirstAndLast)
{
    constexpr Span sp(kSpanData, kSpanDataSize);
    constexpr size_t kSplitSize = kSpanDataSize / 4;
    constexpr Span first        = sp.first(kSplitSize);
    constexpr Span last         = sp.last(kSplitSize);

    ASSERT_EQ(first, Span(kSpanData, kSplitSize));
    ASSERT_EQ(first.data(), kSpanData);
    ASSERT_EQ(first.size(), kSplitSize);

    ASSERT_EQ(last, Span(kSpanData + kSpanDataSize - kSplitSize, kSplitSize));
    ASSERT_EQ(last.data(), kSpanData + kSpanDataSize - kSplitSize);
    ASSERT_EQ(last.size(), kSplitSize);
}

// Test subspan
TEST(SpanTest, Subspan)
{
    constexpr Span sp(kSpanData, kSpanDataSize);
    constexpr size_t kSplitOffset = kSpanDataSize / 4;
    constexpr size_t kSplitSize   = kSpanDataSize / 2;
    constexpr Span subspan        = sp.subspan(kSplitOffset, kSplitSize);

    ASSERT_EQ(subspan, Span(kSpanData + kSplitOffset, kSplitSize));
    ASSERT_EQ(subspan.data(), kSpanData + kSplitOffset);
    ASSERT_EQ(subspan.size(), kSplitSize);
}

}  // anonymous namespace
