//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ConstructCompiler_test.cpp
//   Test the sh::ConstructCompiler interface with different parameters.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"

// Test default parameters.
TEST(ConstructCompilerTest, DefaultParameters)
{
    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);
    ShHandle compiler = sh::ConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL_SPEC,
                                              SH_GLSL_COMPATIBILITY_OUTPUT, &resources);
    ASSERT_NE(nullptr, compiler);
    sh::Destruct(compiler);
}

// Test invalid MaxDrawBuffers zero.
TEST(ConstructCompilerTest, InvalidMaxDrawBuffers)
{
    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);
    resources.MaxDrawBuffers = 0;
    ShHandle compiler        = sh::ConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL_SPEC,
                                              SH_GLSL_COMPATIBILITY_OUTPUT, &resources);
    ASSERT_EQ(nullptr, compiler);
}

// Test invalid MaxDualSourceDrawBuffers zero.
TEST(ConstructCompilerTest, InvalidMaxDualSourceDrawBuffers)
{
    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);
    resources.EXT_blend_func_extended  = 1;
    resources.MaxDualSourceDrawBuffers = 0;
    ShHandle compiler                  = sh::ConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL_SPEC,
                                              SH_GLSL_COMPATIBILITY_OUTPUT, &resources);
    ASSERT_EQ(nullptr, compiler);
}
