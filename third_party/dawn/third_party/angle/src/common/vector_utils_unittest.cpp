//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vector_utils_unittests.cpp: Unit tests for the vector utils.
//

#include "vector_utils.h"

#include <gtest/gtest.h>

using namespace angle;

namespace
{

// First test that comparing vectors work
TEST(VectorUtilsTest, Comparison)
{
    // Don't use ASSERT_EQ at first because the == is more hidden
    ASSERT_TRUE(Vector2(2.0, 3.0) == Vector2(2.0, 3.0));
    ASSERT_TRUE(Vector2(2.0, 3.0) != Vector2(2.0, 4.0));

    // Check ASSERT_EQ and ASSERT_NE work correctly
    ASSERT_EQ(Vector2(2.0, 3.0), Vector2(2.0, 3.0));
    ASSERT_NE(Vector2(2.0, 3.0), Vector2(2.0, 4.0));

    // Check comparison works on all elements
    ASSERT_EQ(Vector4(0.0), Vector4(0.0));
    ASSERT_NE(Vector4(1.0, 0.0, 0.0, 0.0), Vector4(0.0));
    ASSERT_NE(Vector4(0.0, 1.0, 0.0, 0.0), Vector4(0.0));
    ASSERT_NE(Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0));
    ASSERT_NE(Vector4(0.0, 0.0, 0.0, 1.0), Vector4(0.0));
}

// Test indexing
TEST(VectorUtilsTest, Indexing)
{
    Vector2 vec(1.0, 2.0);
    ASSERT_EQ(1.0, vec[0]);
    ASSERT_EQ(2.0, vec[1]);

    vec[0] = 3.0;
    vec[1] = 4.0;
    ASSERT_EQ(Vector2(3.0, 4.0), vec);
}

// Test for the various constructors
TEST(VectorUtilsTest, Constructors)
{
    // Constructor initializing all to a single element
    {
        Vector2 vec(3.0);
        ASSERT_EQ(3.0, vec[0]);
        ASSERT_EQ(3.0, vec[1]);
    }

    // Constructor initializing from another Vector
    {
        Vector2 vec(Vector2(1.0, 2.0));
        ASSERT_EQ(1.0, vec[0]);
        ASSERT_EQ(2.0, vec[1]);
    }

    // Mixed constructor
    {
        Vector4 vec(1.0, Vector2(2.0, 3.0), 4.0);
        ASSERT_EQ(1.0, vec[0]);
        ASSERT_EQ(2.0, vec[1]);
        ASSERT_EQ(3.0, vec[2]);
        ASSERT_EQ(4.0, vec[3]);
    }
}

// Test accessing the data directly
TEST(VectorUtilsTest, DataAccess)
{
    Vector2 vec(1.0, 2.0);
    ASSERT_EQ(2u, vec.size());

    ASSERT_EQ(1.0, vec.data()[0]);
    ASSERT_EQ(2.0, vec.data()[1]);

    vec.data()[0] = 3.0;
    vec.data()[1] = 4.0;

    ASSERT_EQ(Vector2(3.0, 4.0), vec);
}

// Test accessing the data directly
TEST(VectorUtilsTest, LoadStore)
{
    float data[] = {1.0, 2.0};

    Vector2 vec = Vector2::Load(data);

    ASSERT_EQ(1.0, vec.data()[0]);
    ASSERT_EQ(2.0, vec.data()[1]);

    vec = Vector2(3.0, 4.0);
    Vector2::Store(vec, data);

    ASSERT_EQ(3.0, data[0]);
    ASSERT_EQ(4.0, data[1]);
}

// Test basic arithmetic operations
TEST(VectorUtilsTest, BasicArithmetic)
{
    ASSERT_EQ(Vector2(2.0, 3.0), +Vector2(2.0, 3.0));
    ASSERT_EQ(Vector2(-2.0, -3.0), -Vector2(2.0, 3.0));
    ASSERT_EQ(Vector2(4.0, 6.0), Vector2(1.0, 2.0) + Vector2(3.0, 4.0));
    ASSERT_EQ(Vector2(-2.0, -2.0), Vector2(1.0, 2.0) - Vector2(3.0, 4.0));
    ASSERT_EQ(Vector2(3.0, 8.0), Vector2(1.0, 2.0) * Vector2(3.0, 4.0));
    ASSERT_EQ(Vector2(3.0, 2.0), Vector2(3.0, 4.0) / Vector2(1.0, 2.0));

    ASSERT_EQ(Vector2(2.0, 4.0), Vector2(1.0, 2.0) * 2);
    ASSERT_EQ(Vector2(2.0, 4.0), 2 * Vector2(1.0, 2.0));
    ASSERT_EQ(Vector2(0.5, 1.0), Vector2(1.0, 2.0) / 2);
}

// Test compound arithmetic operations
TEST(VectorUtilsTest, CompoundArithmetic)
{
    {
        Vector2 vec(1.0, 2.0);
        vec += Vector2(3.0, 4.0);
        ASSERT_EQ(Vector2(4.0, 6.0), vec);
    }
    {
        Vector2 vec(1.0, 2.0);
        vec -= Vector2(3.0, 4.0);
        ASSERT_EQ(Vector2(-2.0, -2.0), vec);
    }
    {
        Vector2 vec(1.0, 2.0);
        vec *= Vector2(3.0, 4.0);
        ASSERT_EQ(Vector2(3.0, 8.0), vec);
    }
    {
        Vector2 vec(3.0, 4.0);
        vec /= Vector2(1.0, 2.0);
        ASSERT_EQ(Vector2(3.0, 2.0), vec);
    }
    {
        Vector2 vec(1.0, 2.0);
        vec *= 2.0;
        ASSERT_EQ(Vector2(2.0, 4.0), vec);
    }
    {
        Vector2 vec(1.0, 2.0);
        vec /= 2.0;
        ASSERT_EQ(Vector2(0.5, 1.0), vec);
    }
}

// Test other arithmetic operations
TEST(VectorUtilsTest, OtherArithmeticOperations)
{
    Vector2 vec(3.0, 4.0);

    ASSERT_EQ(25.0, vec.lengthSquared());
    ASSERT_EQ(5.0, vec.length());
    ASSERT_EQ(Vector2(0.6, 0.8), vec.normalized());

    ASSERT_EQ(11.0, vec.dot(Vector2(1.0, 2.0)));
}

// Test element shortcuts
TEST(VectorUtilsTest, ElementShortcuts)
{
    Vector2 vec2(1.0, 2.0);
    Vector3 vec3(1.0, 2.0, 3.0);
    Vector4 vec4(1.0, 2.0, 3.0, 4.0);

    ASSERT_EQ(1.0, vec2.x());
    ASSERT_EQ(1.0, vec3.x());
    ASSERT_EQ(1.0, vec4.x());

    ASSERT_EQ(2.0, vec2.y());
    ASSERT_EQ(2.0, vec3.y());
    ASSERT_EQ(2.0, vec4.y());

    ASSERT_EQ(3.0, vec3.z());
    ASSERT_EQ(3.0, vec4.z());

    ASSERT_EQ(4.0, vec4.w());

    vec2.x() = 0.0;
    ASSERT_EQ(Vector2(0.0, 2.0), vec2);
}

// Test the cross product
TEST(VectorUtilsTest, CrossProduct)
{
    ASSERT_EQ(Vector3(0.0, 0.0, 1.0), Vector3(1.0, 0.0, 0.0).cross(Vector3(0.0, 1.0, 0.0)));
    ASSERT_EQ(Vector3(-3.0, 6.0, -3.0), Vector3(1.0, 2.0, 3.0).cross(Vector3(4.0, 5.0, 6.0)));
}

// Test basic functionality of int vectors
TEST(VectorUtilsTest, IntVector)
{
    Vector2I vec(0);

    int *data = vec.data();
    data[1]   = 1;

    ASSERT_EQ(0, vec[0]);
    ASSERT_EQ(1, vec[1]);
}

// Test basic functionality of int vectors
TEST(VectorUtilsTest, UIntVector)
{
    Vector2U vec(0);

    unsigned int *data = vec.data();
    data[1]            = 1;

    ASSERT_EQ(0u, vec[0]);
    ASSERT_EQ(1u, vec[1]);
}

}  // anonymous namespace
