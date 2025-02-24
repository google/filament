//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CircularBuffer_unittest:
//   Tests of the CircularBuffer class
//

#include <gtest/gtest.h>

#include "common/CircularBuffer.h"

namespace angle
{
// Make sure the various constructors compile and do basic checks
TEST(CircularBuffer, Constructors)
{
    CircularBuffer<int, 5> defaultContructor;
    EXPECT_EQ(5u, defaultContructor.size());

    CircularBuffer<int, 5> valueConstructor(3);
    EXPECT_EQ(5u, valueConstructor.size());
    EXPECT_EQ(3, valueConstructor.front());

    CircularBuffer<int, 5> copy(valueConstructor);
    EXPECT_EQ(5u, copy.size());

    CircularBuffer<int, 5> copyRValue(std::move(valueConstructor));
    EXPECT_EQ(5u, copyRValue.size());
}

// Make sure the destructor destroys all elements.
TEST(CircularBuffer, Destructor)
{
    struct s
    {
        s() {}
        s(int *c) : counter(c) {}
        ~s()
        {
            if (counter)
            {
                ++*counter;
            }
        }

        s(const s &) = default;
        s &operator=(const s &) = default;

        int *counter;
    };

    int destructorCount = 0;

    {
        CircularBuffer<s, 11> buf((s(&destructorCount)));

        // Destructor called once for the temporary above.
        EXPECT_EQ(destructorCount, 1);

        // Change front index to be non-zero.
        buf.next();
        buf.next();
        buf.next();
    }

    // Destructor should be called 11 more times, once for each element.
    EXPECT_EQ(destructorCount, 12);
}

// Test circulating behavior.
TEST(CircularBuffer, Circulate)
{
    CircularBuffer<int, 7> buf(128);

    for (int i = 0; i < 7; ++i)
    {
        int &value = buf.front();
        EXPECT_EQ(value, 128);

        value = i + 10;

        buf.next();
    }

    for (int i = 0; i < 93; ++i)
    {
        EXPECT_EQ(buf.front(), i % 7 + 10);
        buf.next();
    }
}

// Test iteration.
TEST(CircularBuffer, Iterate)
{
    CircularBuffer<int, 3> buf(12);

    for (int i = 0; i < 3; ++i)
    {
        int &value = buf.front();
        buf.next();

        EXPECT_EQ(value, 12);

        value = i;
    }

    // Check that iteration returns all the values (with unknown order) regardless of where the
    // front is pointing.
    for (int i = 0; i < 10; ++i)
    {
        uint32_t valuesSeen = 0;
        for (int value : buf)
        {
            valuesSeen |= 1 << value;
        }
        EXPECT_EQ(valuesSeen, 0x7u);

        // Make sure iteration hasn't affected the front index.
        EXPECT_EQ(buf.front(), i % 3);

        buf.next();
    }
}

// Tests buffer operations with a non copyable type.
TEST(CircularBuffer, NonCopyable)
{
    struct s : angle::NonCopyable
    {
        s() : x(0) {}
        s(s &&other) : x(other.x) {}
        s &operator=(s &&other)
        {
            x = other.x;
            return *this;
        }
        int x;
    };

    CircularBuffer<s, 4> buf;

    for (int i = 0; i < 4; ++i)
    {
        s &value = buf.front();
        value.x  = i;

        buf.next();
    }

    // Make the front index non-zero.
    buf.next();
    EXPECT_EQ(buf.front().x, 1);

    CircularBuffer<s, 4> copy = std::move(buf);

    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(copy.front().x, (i + 1) % 4);
        copy.next();
    }
}
}  // namespace angle
