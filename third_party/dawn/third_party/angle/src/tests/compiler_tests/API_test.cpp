//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// API_test.cpp:
//   Some tests for the compiler API.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"

TEST(APITest, CompareShBuiltInResources)
{
    ShBuiltInResources a_resources;
    memset(&a_resources, 88, sizeof(a_resources));
    sh::InitBuiltInResources(&a_resources);

    ShBuiltInResources b_resources;
    memset(&b_resources, 77, sizeof(b_resources));
    sh::InitBuiltInResources(&b_resources);

    EXPECT_TRUE(memcmp(&a_resources, &b_resources, sizeof(a_resources)) == 0);
}
