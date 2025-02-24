//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShCompile_test.cpp
//   Test the sh::Compile interface with different parameters.
//

#include <clocale>
#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "common/angleutils.h"
#include "common/platform.h"
#include "gtest/gtest.h"

class ShCompileTest : public testing::Test
{
  public:
    ShCompileTest() {}

  protected:
    void SetUp() override
    {
        sh::InitBuiltInResources(&mResources);
        mCompiler = sh::ConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL_SPEC,
                                          SH_GLSL_COMPATIBILITY_OUTPUT, &mResources);
        ASSERT_TRUE(mCompiler != nullptr) << "Compiler could not be constructed.";
    }

    void TearDown() override
    {
        if (mCompiler)
        {
            sh::Destruct(mCompiler);
            mCompiler = nullptr;
        }
    }

    void testCompile(const char **shaderStrings, int stringCount, bool expectation)
    {
        ShCompileOptions options    = {};
        options.objectCode          = true;
        options.initOutputVariables = true;

        bool success                  = sh::Compile(mCompiler, shaderStrings, stringCount, options);
        const std::string &compileLog = sh::GetInfoLog(mCompiler);
        EXPECT_EQ(expectation, success) << compileLog;
    }

    ShBuiltInResources mResources;

    class [[nodiscard]] ScopedRestoreDefaultLocale : angle::NonCopyable
    {
      public:
        ScopedRestoreDefaultLocale();
        ~ScopedRestoreDefaultLocale();

      private:
        std::locale defaultLocale;
    };

  public:
    ShHandle mCompiler;
};

ShCompileTest::ScopedRestoreDefaultLocale::ScopedRestoreDefaultLocale()
{
    defaultLocale = std::locale();
}

ShCompileTest::ScopedRestoreDefaultLocale::~ScopedRestoreDefaultLocale()
{
    std::locale::global(defaultLocale);
}

class ShCompileComputeTest : public ShCompileTest
{
  public:
    ShCompileComputeTest() {}

  protected:
    void SetUp() override
    {
        sh::InitBuiltInResources(&mResources);
        mCompiler = sh::ConstructCompiler(GL_COMPUTE_SHADER, SH_WEBGL3_SPEC,
                                          SH_GLSL_COMPATIBILITY_OUTPUT, &mResources);
        ASSERT_TRUE(mCompiler != nullptr) << "Compiler could not be constructed.";
    }
};

// Test calling sh::Compile with compute shader source string.
TEST_F(ShCompileComputeTest, ComputeShaderString)
{
    constexpr char kComputeShaderString[] =
        R"(#version 310 es
        layout(local_size_x=1) in;
        void main()
        {
        })";

    const char *shaderStrings[] = {kComputeShaderString};

    testCompile(shaderStrings, 1, true);
}

// Test calling sh::Compile with more than one shader source string.
TEST_F(ShCompileTest, MultipleShaderStrings)
{
    const std::string &shaderString1 =
        "precision mediump float;\n"
        "void main() {\n";
    const std::string &shaderString2 =
        "    gl_FragColor = vec4(0.0);\n"
        "}";

    const char *shaderStrings[] = {shaderString1.c_str(), shaderString2.c_str()};

    testCompile(shaderStrings, 2, true);
}

// Test calling sh::Compile with a tokens split into different shader source strings.
TEST_F(ShCompileTest, TokensSplitInShaderStrings)
{
    const std::string &shaderString1 =
        "precision mediump float;\n"
        "void ma";
    const std::string &shaderString2 =
        "in() {\n"
        "#i";
    const std::string &shaderString3 =
        "f 1\n"
        "    gl_FragColor = vec4(0.0);\n"
        "#endif\n"
        "}";

    const char *shaderStrings[] = {shaderString1.c_str(), shaderString2.c_str(),
                                   shaderString3.c_str()};

    testCompile(shaderStrings, 3, true);
}

// Parsing floats in shaders can run afoul of locale settings.
// Eg. in de_DE, `strtof("1.5")` will yield `1.0f`. (It's expecting "1.5")
TEST_F(ShCompileTest, DecimalSepLocale)
{
    // Locale names are platform dependent, add platform-specific names of locales to be tested here
    const std::string availableLocales[] = {
        "de_DE",
        "de-DE",
        "de_DE.UTF-8",
        "de_DE.ISO8859-1",
        "de_DE.ISO8859-15",
        "de_DE@euro",
        "de_DE.88591",
        "de_DE.88591.en",
        "de_DE.iso88591",
        "de_DE.ISO-8859-1",
        "de_DE.ISO_8859-1",
        "de_DE.iso885915",
        "de_DE.ISO-8859-15",
        "de_DE.ISO_8859-15",
        "de_DE.8859-15",
        "de_DE.8859-15@euro",
#if !defined(_WIN32)
        // TODO(https://crbug.com/972372): Add this test back on Windows once the
        // CRT no longer throws on code page sections ('ISO-8859-15@euro') that
        // are >= 16 characters long.
        "de_DE.ISO-8859-15@euro",
#endif
        "de_DE.UTF-8@euro",
        "de_DE.utf8",
        "German_germany",
        "German_Germany",
        "German_Germany.1252",
        "German_Germany.UTF-8",
        "German",
        // One ubuntu tester doesn't have a german locale, but da_DK.utf8 has similar float
        // representation
        "da_DK.utf8"
    };

    const auto localeExists = [](const std::string name) {
        return bool(setlocale(LC_ALL, name.c_str()));
    };

    const char kSource[] = R"(
    void main()
    {
        gl_FragColor = vec4(1.5);
    })";
    const char *parts[]  = {kSource};

    // Ensure the locale is reset after the test runs.
    ScopedRestoreDefaultLocale restoreLocale;

    for (const std::string &locale : availableLocales)
    {
        // If the locale doesn't exist on the testing platform, the locale constructor will fail,
        // throwing an exception
        // We use setlocale() (through localeExists) to test whether a locale
        // exists before calling the locale constructor
        if (localeExists(locale))
        {
            std::locale localizedLoc(locale);

            ShCompileOptions compileOptions = {};
            compileOptions.objectCode       = true;

            // std::locale::global() must be used instead of setlocale() to affect new streams'
            // default locale
            std::locale::global(std::locale::classic());
            sh::Compile(mCompiler, parts, 1, compileOptions);
            std::string referenceOut = sh::GetObjectCode(mCompiler);
            EXPECT_NE(referenceOut.find("1.5"), std::string::npos)
                << "float formatted incorrectly with classic locale";

            sh::ClearResults(mCompiler);

            std::locale::global(localizedLoc);
            sh::Compile(mCompiler, parts, 1, compileOptions);
            std::string localizedOut = sh::GetObjectCode(mCompiler);
            EXPECT_NE(localizedOut.find("1.5"), std::string::npos)
                << "float formatted incorrectly with locale (" << localizedLoc.name() << ") set";

            ASSERT_EQ(referenceOut, localizedOut)
                << "different output with locale (" << localizedLoc.name() << ") set";
        }
    }
}
