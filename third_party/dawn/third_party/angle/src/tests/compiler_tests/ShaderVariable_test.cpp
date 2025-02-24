//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CollectVariables_test.cpp:
//   Some tests for shader inspection
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "compiler/translator/Compiler.h"
#include "gtest/gtest.h"

namespace sh
{

class ShaderVariableTest : public testing::Test
{
  public:
    ShaderVariableTest() : mVariablesCompileOptions{}, mObjectCodeCompileOptions{}
    {
        mObjectCodeCompileOptions.objectCode = true;
    }

  protected:
    ShCompileOptions mVariablesCompileOptions;
    ShCompileOptions mObjectCodeCompileOptions;
};

TEST_F(ShaderVariableTest, FindInfoByMappedName)
{
    // struct A {
    //   float x[2];
    //   vec3 y;
    // };
    // struct B {
    //   A a[3];
    // };
    // B uni[2];
    ShaderVariable uni(0, 2);
    uni.name              = "uni";
    uni.mappedName        = "m_uni";
    uni.structOrBlockName = "B";
    {
        ShaderVariable a(0, 3);
        a.name              = "a";
        a.mappedName        = "m_a";
        a.structOrBlockName = "A";
        {
            ShaderVariable x(GL_FLOAT, 2);
            x.name       = "x";
            x.mappedName = "m_x";
            a.fields.push_back(x);

            ShaderVariable y(GL_FLOAT_VEC3);
            y.name       = "y";
            y.mappedName = "m_y";
            a.fields.push_back(y);
        }
        uni.fields.push_back(a);
    }

    const ShaderVariable *leafVar = nullptr;
    std::string originalFullName;

    std::string mappedFullName = "wrongName";
    EXPECT_FALSE(uni.findInfoByMappedName(mappedFullName, &leafVar, &originalFullName));

    mappedFullName = "m_uni";
    EXPECT_TRUE(uni.findInfoByMappedName(mappedFullName, &leafVar, &originalFullName));
    EXPECT_EQ(&uni, leafVar);
    EXPECT_STREQ("uni", originalFullName.c_str());

    mappedFullName = "m_uni[0].m_a[1].wrongName";
    EXPECT_FALSE(uni.findInfoByMappedName(mappedFullName, &leafVar, &originalFullName));

    mappedFullName = "m_uni[0].m_a[1].m_x";
    EXPECT_TRUE(uni.findInfoByMappedName(mappedFullName, &leafVar, &originalFullName));
    EXPECT_EQ(&(uni.fields[0].fields[0]), leafVar);
    EXPECT_STREQ("uni[0].a[1].x", originalFullName.c_str());

    mappedFullName = "m_uni[0].m_a[1].m_x[0]";
    EXPECT_TRUE(uni.findInfoByMappedName(mappedFullName, &leafVar, &originalFullName));
    EXPECT_EQ(&(uni.fields[0].fields[0]), leafVar);
    EXPECT_STREQ("uni[0].a[1].x[0]", originalFullName.c_str());

    mappedFullName = "m_uni[0].m_a[1].m_y";
    EXPECT_TRUE(uni.findInfoByMappedName(mappedFullName, &leafVar, &originalFullName));
    EXPECT_EQ(&(uni.fields[0].fields[1]), leafVar);
    EXPECT_STREQ("uni[0].a[1].y", originalFullName.c_str());
}

TEST_F(ShaderVariableTest, IsSameUniformWithDifferentFieldOrder)
{
    // struct A {
    //   float x;
    //   float y;
    // };
    // uniform A uni;
    ShaderVariable vx_a;
    vx_a.name              = "uni";
    vx_a.mappedName        = "m_uni";
    vx_a.structOrBlockName = "A";
    {
        ShaderVariable x(GL_FLOAT);
        x.name       = "x";
        x.mappedName = "m_x";
        vx_a.fields.push_back(x);

        ShaderVariable y(GL_FLOAT);
        y.name       = "y";
        y.mappedName = "m_y";
        vx_a.fields.push_back(y);
    }

    // struct A {
    //   float y;
    //   float x;
    // };
    // uniform A uni;
    ShaderVariable fx_a;
    fx_a.name              = "uni";
    fx_a.mappedName        = "m_uni";
    fx_a.structOrBlockName = "A";
    {
        ShaderVariable y(GL_FLOAT);
        y.name       = "y";
        y.mappedName = "m_y";
        fx_a.fields.push_back(y);

        ShaderVariable x(GL_FLOAT);
        x.name       = "x";
        x.mappedName = "m_x";
        fx_a.fields.push_back(x);
    }

    EXPECT_FALSE(vx_a.isSameUniformAtLinkTime(fx_a));
}

TEST_F(ShaderVariableTest, IsSameUniformWithDifferentStructNames)
{
    // struct A {
    //   float x;
    //   float y;
    // };
    // uniform A uni;
    ShaderVariable vx_a;
    vx_a.name              = "uni";
    vx_a.mappedName        = "m_uni";
    vx_a.structOrBlockName = "A";
    {
        ShaderVariable x(GL_FLOAT);
        x.name       = "x";
        x.mappedName = "m_x";
        vx_a.fields.push_back(x);

        ShaderVariable y(GL_FLOAT);
        y.name       = "y";
        y.mappedName = "m_y";
        vx_a.fields.push_back(y);
    }

    // struct B {
    //   float x;
    //   float y;
    // };
    // uniform B uni;
    ShaderVariable fx_a;
    fx_a.name       = "uni";
    fx_a.mappedName = "m_uni";
    {
        ShaderVariable x(GL_FLOAT);
        x.name       = "x";
        x.mappedName = "m_x";
        fx_a.fields.push_back(x);

        ShaderVariable y(GL_FLOAT);
        y.name       = "y";
        y.mappedName = "m_y";
        fx_a.fields.push_back(y);
    }

    fx_a.structOrBlockName = "B";
    EXPECT_FALSE(vx_a.isSameUniformAtLinkTime(fx_a));

    fx_a.structOrBlockName = "A";
    EXPECT_TRUE(vx_a.isSameUniformAtLinkTime(fx_a));

    fx_a.structOrBlockName = "";
    EXPECT_FALSE(vx_a.isSameUniformAtLinkTime(fx_a));
}

TEST_F(ShaderVariableTest, IsSameVaryingWithDifferentInvariance)
{
    // invariant varying float vary;
    ShaderVariable vx;
    vx.type        = GL_FLOAT;
    vx.precision   = GL_MEDIUM_FLOAT;
    vx.name        = "vary";
    vx.mappedName  = "m_vary";
    vx.staticUse   = true;
    vx.isInvariant = true;

    // varying float vary;
    ShaderVariable fx;
    fx.type        = GL_FLOAT;
    fx.precision   = GL_MEDIUM_FLOAT;
    fx.name        = "vary";
    fx.mappedName  = "m_vary";
    fx.staticUse   = true;
    fx.isInvariant = false;

    // Default to ESSL1 behavior: invariance must match
    EXPECT_FALSE(vx.isSameVaryingAtLinkTime(fx));
    EXPECT_FALSE(vx.isSameVaryingAtLinkTime(fx, 100));
    // ESSL3 behavior: invariance doesn't need to match
    EXPECT_TRUE(vx.isSameVaryingAtLinkTime(fx, 300));

    // invariant varying float vary;
    fx.isInvariant = true;
    EXPECT_TRUE(vx.isSameVaryingAtLinkTime(fx));
    EXPECT_TRUE(vx.isSameVaryingAtLinkTime(fx, 100));
    EXPECT_TRUE(vx.isSameVaryingAtLinkTime(fx, 300));
}

// Test that using invariant varyings doesn't trigger a double delete.
TEST_F(ShaderVariableTest, InvariantDoubleDeleteBug)
{
    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);

    ShHandle compiler = sh::ConstructCompiler(GL_VERTEX_SHADER, SH_GLES2_SPEC,
                                              SH_GLSL_COMPATIBILITY_OUTPUT, &resources);
    EXPECT_NE(static_cast<ShHandle>(0), compiler);

    const char *program[] = {
        "attribute vec4 position;\n"
        "varying float v;\n"
        "invariant v;\n"
        "void main() {\n"
        "  v = 1.0;\n"
        "  gl_Position = position;\n"
        "}"};

    EXPECT_TRUE(sh::Compile(compiler, program, 1, mObjectCodeCompileOptions));
    EXPECT_TRUE(sh::Compile(compiler, program, 1, mObjectCodeCompileOptions));
    sh::Destruct(compiler);
}

TEST_F(ShaderVariableTest, IllegalInvariantVarying)
{
    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);

    ShHandle compiler = sh::ConstructCompiler(GL_VERTEX_SHADER, SH_GLES2_SPEC,
                                              SH_GLSL_COMPATIBILITY_OUTPUT, &resources);
    EXPECT_NE(static_cast<ShHandle>(0), compiler);

    const char *program1[] = {
        R"(void foo()
        {
        }
        varying vec4 v_varying;
        invariant v_varying;
        void main()
        {
           foo();
           gl_Position = v_varying;
        })"};
    const char *program2[] = {
        "varying vec4 v_varying;\n"
        "void foo() {\n"
        "  invariant v_varying;\n"
        "}\n"
        "void main() {\n"
        "  foo();\n"
        "  gl_Position = v_varying;\n"
        "}"};

    EXPECT_TRUE(sh::Compile(compiler, program1, 1, mVariablesCompileOptions));
    EXPECT_FALSE(sh::Compile(compiler, program2, 1, mVariablesCompileOptions));
    sh::Destruct(compiler);
}

TEST_F(ShaderVariableTest, InvariantLeakAcrossShaders)
{
    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);

    ShHandle compiler = sh::ConstructCompiler(GL_VERTEX_SHADER, SH_GLES2_SPEC,
                                              SH_GLSL_COMPATIBILITY_OUTPUT, &resources);
    EXPECT_NE(static_cast<ShHandle>(0), compiler);

    const char *program1[] = {
        "varying vec4 v_varying;\n"
        "invariant v_varying;\n"
        "void main() {\n"
        "  gl_Position = v_varying;\n"
        "}"};
    const char *program2[] = {
        "varying vec4 v_varying;\n"
        "void main() {\n"
        "  gl_Position = v_varying;\n"
        "}"};

    EXPECT_TRUE(sh::Compile(compiler, program1, 1, mVariablesCompileOptions));
    const std::vector<sh::ShaderVariable> *varyings = sh::GetOutputVaryings(compiler);
    for (const sh::ShaderVariable &varying : *varyings)
    {
        if (varying.name == "v_varying")
        {
            EXPECT_TRUE(varying.isInvariant);
        }
    }
    EXPECT_TRUE(sh::Compile(compiler, program2, 1, mVariablesCompileOptions));
    varyings = sh::GetOutputVaryings(compiler);
    for (const sh::ShaderVariable &varying : *varyings)
    {
        if (varying.name == "v_varying")
        {
            EXPECT_FALSE(varying.isInvariant);
        }
    }
    sh::Destruct(compiler);
}

TEST_F(ShaderVariableTest, GlobalInvariantLeakAcrossShaders)
{
    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);

    ShHandle compiler = sh::ConstructCompiler(GL_VERTEX_SHADER, SH_GLES2_SPEC,
                                              SH_GLSL_COMPATIBILITY_OUTPUT, &resources);
    EXPECT_NE(static_cast<ShHandle>(0), compiler);

    const char *program1[] = {
        "#pragma STDGL invariant(all)\n"
        "varying vec4 v_varying;\n"
        "void main() {\n"
        "  gl_Position = v_varying;\n"
        "}"};
    const char *program2[] = {
        "varying vec4 v_varying;\n"
        "void main() {\n"
        "  gl_Position = v_varying;\n"
        "}"};

    EXPECT_TRUE(sh::Compile(compiler, program1, 1, mVariablesCompileOptions));
    const std::vector<sh::ShaderVariable> *varyings = sh::GetOutputVaryings(compiler);
    for (const sh::ShaderVariable &varying : *varyings)
    {
        if (varying.name == "v_varying")
        {
            EXPECT_TRUE(varying.isInvariant);
        }
    }
    EXPECT_TRUE(sh::Compile(compiler, program2, 1, mVariablesCompileOptions));
    varyings = sh::GetOutputVaryings(compiler);
    for (const sh::ShaderVariable &varying : *varyings)
    {
        if (varying.name == "v_varying")
        {
            EXPECT_FALSE(varying.isInvariant);
        }
    }
    sh::Destruct(compiler);
}

TEST_F(ShaderVariableTest, BuiltinInvariantVarying)
{
    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);

    ShHandle compiler = sh::ConstructCompiler(GL_VERTEX_SHADER, SH_GLES2_SPEC,
                                              SH_GLSL_COMPATIBILITY_OUTPUT, &resources);
    EXPECT_NE(static_cast<ShHandle>(0), compiler);

    const char *program1[] = {
        "invariant gl_Position;\n"
        "void main() {\n"
        "  gl_Position = vec4(0, 0, 0, 0);\n"
        "}"};
    const char *program2[] = {
        "void main() {\n"
        "  gl_Position = vec4(0, 0, 0, 0);\n"
        "}"};
    const char *program3[] = {
        "void main() {\n"
        "  invariant gl_Position;\n"
        "  gl_Position = vec4(0, 0, 0, 0);\n"
        "}"};

    EXPECT_TRUE(sh::Compile(compiler, program1, 1, mVariablesCompileOptions));
    const std::vector<sh::ShaderVariable> *varyings = sh::GetOutputVaryings(compiler);
    for (const sh::ShaderVariable &varying : *varyings)
    {
        if (varying.name == "gl_Position")
        {
            EXPECT_TRUE(varying.isInvariant);
        }
    }
    EXPECT_TRUE(sh::Compile(compiler, program2, 1, mVariablesCompileOptions));
    varyings = sh::GetOutputVaryings(compiler);
    for (const sh::ShaderVariable &varying : *varyings)
    {
        if (varying.name == "gl_Position")
        {
            EXPECT_FALSE(varying.isInvariant);
        }
    }
    EXPECT_FALSE(sh::Compile(compiler, program3, 1, mVariablesCompileOptions));
    sh::Destruct(compiler);
}

// Verify in ES3.1 two varyings with either same name or same declared location can match.
TEST_F(ShaderVariableTest, IsSameVaryingWithDifferentName)
{
    // Varying float vary1;
    ShaderVariable vx;
    vx.type        = GL_FLOAT;
    vx.precision   = GL_MEDIUM_FLOAT;
    vx.name        = "vary1";
    vx.mappedName  = "m_vary1";
    vx.staticUse   = true;
    vx.isInvariant = false;

    // Varying float vary2;
    ShaderVariable fx;
    fx.type        = GL_FLOAT;
    fx.precision   = GL_MEDIUM_FLOAT;
    fx.name        = "vary2";
    fx.mappedName  = "m_vary2";
    fx.staticUse   = true;
    fx.isInvariant = false;

    // ESSL3 behavior: name must match
    EXPECT_FALSE(vx.isSameVaryingAtLinkTime(fx, 300));

    // ESSL3.1 behavior:
    // [OpenGL ES 3.1 SPEC Chapter 7.4.1]
    // An output variable is considered to match an input variable in the subsequent shader if:
    // - the two variables match in name, type, and qualification; or
    // - the two variables are declared with the same location qualifier and match in type and
    //   qualification.
    vx.location = 0;
    fx.location = 0;
    EXPECT_TRUE(vx.isSameVaryingAtLinkTime(fx, 310));

    fx.name       = vx.name;
    fx.mappedName = vx.mappedName;

    fx.location = -1;
    EXPECT_FALSE(vx.isSameVaryingAtLinkTime(fx, 310));

    fx.location = 1;
    EXPECT_FALSE(vx.isSameVaryingAtLinkTime(fx, 310));

    fx.location = 0;
    EXPECT_TRUE(vx.isSameVaryingAtLinkTime(fx, 310));
}

// Test that using two consecutive underscores (__) can be used for declaring an identifier
TEST_F(ShaderVariableTest, DoubleUnderscoresForIdentifier)
{
    ShBuiltInResources resources;
    sh::InitBuiltInResources(&resources);

    ShHandle compiler = sh::ConstructCompiler(GL_VERTEX_SHADER, SH_GLES3_SPEC,
                                              SH_GLSL_COMPATIBILITY_OUTPUT, &resources);
    EXPECT_NE(static_cast<ShHandle>(0), compiler);

    const char *front_underscores[] = {
        "#version 300 es\n"
        "in vec4 __position;\n"
        "out float v;\n"
        "void main() {\n"
        "  v = 1.0;\n"
        "  gl_Position = __position;\n"
        "}"};
    EXPECT_TRUE(sh::Compile(compiler, front_underscores, 1, mObjectCodeCompileOptions));

    const char *middle_underscores[] = {
        "#version 300 es\n"
        "in vec4 position__in;\n"
        "out float v;\n"
        "void main() {\n"
        "  v = 1.0;\n"
        "  gl_Position = position__in;\n"
        "}"};
    EXPECT_TRUE(sh::Compile(compiler, middle_underscores, 1, mObjectCodeCompileOptions));

    const char *end_underscores[] = {
        "#version 300 es\n"
        "in vec4 position__;\n"
        "out float v;\n"
        "void main() {\n"
        "  v = 1.0;\n"
        "  gl_Position = position__;\n"
        "}"};
    EXPECT_TRUE(sh::Compile(compiler, end_underscores, 1, mObjectCodeCompileOptions));

    sh::Destruct(compiler);
}

}  // namespace sh
