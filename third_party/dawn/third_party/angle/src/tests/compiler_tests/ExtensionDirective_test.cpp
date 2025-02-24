//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ExtensionDirective_test.cpp:
//   Miscellaneous tests for extension directives toggling functionality correctly.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "compiler/translator/ExtensionBehavior.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class FragmentShaderExtensionDirectiveTest : public ShaderCompileTreeTest
{
  public:
    FragmentShaderExtensionDirectiveTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }

    void testCompileNeedsExtensionDirective(const std::string &shader, const std::string &extension)
    {
        testCompileNeedsExtensionDirective(shader, extension, "");
    }

    void testCompileNeedsExtensionDirective(const std::string &shader,
                                            const std::string &extension,
                                            const std::string &versionDirective)
    {
        if (compile(versionDirective + shader))
        {
            FAIL()
                << "Shader compilation without extension directive succeeded, expecting failure:\n"
                << mInfoLog;
        }
        if (compile(versionDirective + getExtensionDirective(extension, sh::EBhDisable) + shader))
        {
            FAIL() << "Shader compilation with extension disable directive succeeded, expecting "
                      "failure:\n"
                   << mInfoLog;
        }
        if (!compile(versionDirective + getExtensionDirective(extension, sh::EBhEnable) + shader))
        {
            FAIL()
                << "Shader compilation with extension enable directive failed, expecting success:\n"
                << mInfoLog;
        }

        if (!compile(versionDirective + getExtensionDirective(extension, sh::EBhWarn) + shader))
        {
            FAIL()
                << "Shader compilation with extension warn directive failed, expecting success:\n"
                << mInfoLog;
        }
        else if (!hasWarning())
        {
            FAIL() << "Expected compilation to succeed with warning, but warning not present:\n"
                   << mInfoLog;
        }
    }

  private:
    std::string getExtensionDirective(const std::string &extension, sh::TBehavior behavior)
    {
        std::string extensionDirective("#extension ");
        extensionDirective += extension + " : ";
        switch (behavior)
        {
            case EBhRequire:
                extensionDirective += "require";
                break;
            case EBhEnable:
                extensionDirective += "enable";
                break;
            case EBhWarn:
                extensionDirective += "warn";
                break;
            case EBhDisable:
                extensionDirective += "disable";
                break;
            default:
                break;
        }
        extensionDirective += "\n";
        return extensionDirective;
    }
};

class OESEGLImageExternalExtensionTest : public FragmentShaderExtensionDirectiveTest
{
  public:
    OESEGLImageExternalExtensionTest() {}

  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->OES_EGL_image_external = 1;
    }
};

// OES_EGL_image_external needs to be enabled in GLSL to be able to use samplerExternalOES.
TEST_F(OESEGLImageExternalExtensionTest, SamplerExternalOESUsageNeedsExtensionDirective)
{
    const std::string &shaderString =
        R"(
        precision mediump float;

        uniform samplerExternalOES s;
        void main()
        {})";
    testCompileNeedsExtensionDirective(shaderString, "GL_OES_EGL_image_external");
}

class NVEGLStreamConsumerExternalExtensionTest : public FragmentShaderExtensionDirectiveTest
{
  public:
    NVEGLStreamConsumerExternalExtensionTest() {}

  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->NV_EGL_stream_consumer_external = 1;
    }
};

// NV_EGL_stream_consumer_external needs to be enabled in GLSL to be able to use samplerExternalOES.
TEST_F(NVEGLStreamConsumerExternalExtensionTest, SamplerExternalOESUsageNeedsExtensionDirective)
{
    const std::string &shaderString =
        R"(
        precision mediump float;

        uniform samplerExternalOES s;
        void main()
        {})";
    testCompileNeedsExtensionDirective(shaderString, "GL_NV_EGL_stream_consumer_external");
}

class EXTYUVTargetExtensionTest : public FragmentShaderExtensionDirectiveTest
{
  public:
    EXTYUVTargetExtensionTest() {}

  protected:
    void initResources(ShBuiltInResources *resources) override { resources->EXT_YUV_target = 1; }
};

// GL_EXT_YUV_target needs to be enabled in GLSL to be able to use samplerExternal2DY2YEXT.
TEST_F(EXTYUVTargetExtensionTest, SamplerExternal2DY2YEXTUsageNeedsExtensionDirective)
{
    const std::string &shaderString =
        R"(
        precision mediump float;

        uniform __samplerExternal2DY2YEXT s;
        void main()
        {})";
    testCompileNeedsExtensionDirective(shaderString, "GL_EXT_YUV_target", "#version 300 es\n");
}

// GL_EXT_YUV_target needs to be enabled in GLSL to be able to use samplerExternal2DY2YEXT.
TEST_F(EXTYUVTargetExtensionTest, YUVLayoutNeedsExtensionDirective)
{
    const std::string &shaderString =
        R"(
        precision mediump float;

        layout(yuv) out vec4 color;
        void main()
        {})";
    testCompileNeedsExtensionDirective(shaderString, "GL_EXT_YUV_target", "#version 300 es\n");
}
