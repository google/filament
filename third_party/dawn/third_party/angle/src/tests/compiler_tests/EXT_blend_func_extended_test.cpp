//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EXT_blend_func_extended.cpp:
//   Test for EXT_blend_func_extended_test
//

#include "tests/test_utils/ShaderExtensionTest.h"

namespace
{
const char EXTBFEPragma[] = "#extension GL_EXT_blend_func_extended : require\n";

const char ESSL100_SimpleShader1[] =
    "precision mediump float;\n"
    "void main() { \n"
    "    gl_FragColor = vec4(1.0);\n"
    "    gl_SecondaryFragColorEXT = vec4(gl_MaxDualSourceDrawBuffersEXT / 10);\n"
    "}\n";

// Shader that tests only the access to gl_MaxDualSourceDrawBuffersEXT.
const char ESSL100_MaxDualSourceAccessShader[] =
    "precision mediump float;\n"
    "void main() { gl_FragColor = vec4(gl_MaxDualSourceDrawBuffersEXT / 10); }\n";

// Shader that writes to SecondaryFragData.
const char ESSL100_FragDataShader[] =
    "#extension GL_EXT_draw_buffers : require\n"
    "precision mediump float;\n"
    "void main() {\n"
    "    gl_FragData[gl_MaxDrawBuffers - 1] = vec4(1.0);\n"
    "    gl_SecondaryFragDataEXT[gl_MaxDualSourceDrawBuffersEXT - 1] = vec4(0.1);\n"
    "}\n";

// Shader that writes to SecondaryFragColor and SecondaryFragData does not compile.
const char ESSL100_ColorAndDataWriteFailureShader1[] =
    "precision mediump float;\n"
    "void main() {\n"
    "    gl_SecondaryFragColorEXT = vec4(1.0);\n"
    "    gl_SecondaryFragDataEXT[gl_MaxDualSourceDrawBuffersEXT] = vec4(0.1);\n"
    "}\n";

// Shader that writes to FragColor and SecondaryFragData does not compile.
const char ESSL100_ColorAndDataWriteFailureShader2[] =
    "precision mediump float;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(1.0);\n"
    "    gl_SecondaryFragDataEXT[gl_MaxDualSourceDrawBuffersEXT] = vec4(0.1);\n"
    "}\n";

// Shader that writes to FragData and SecondaryFragColor.
const char ESSL100_ColorAndDataWriteFailureShader3[] =
    "#extension GL_EXT_draw_buffers : require\n"
    "precision mediump float;\n"
    "void main() {\n"
    "    gl_SecondaryFragColorEXT = vec4(1.0);\n"
    "    gl_FragData[gl_MaxDrawBuffers] = vec4(0.1);\n"
    "}\n";

// Dynamic indexing of SecondaryFragData is not allowed in WebGL 2.0.
const char ESSL100_IndexSecondaryFragDataWithNonConstantShader[] =
    "precision mediump float;\n"
    "void main() {\n"
    "    for (int i = 0; i < 2; ++i) {\n"
    "        gl_SecondaryFragDataEXT[true ? 0 : i] = vec4(0.0);\n"
    "    }\n"
    "}\n";

// In GLSL version 300 es, the gl_MaxDualSourceDrawBuffersEXT is available.
const char ESSL300_MaxDualSourceAccessShader[] =
    "precision mediump float;\n"
    "layout(location = 0) out mediump vec4 fragColor;"
    "void main() {\n"
    "    fragColor = vec4(gl_MaxDualSourceDrawBuffersEXT / 10);\n"
    "}\n";

// In ES 3.0, the locations can be assigned through the API with glBindFragDataLocationIndexedEXT.
// It's fine to have a mix of specified and unspecified locations.
const char ESSL300_LocationAndUnspecifiedOutputShader[] =
    "precision mediump float;\n"
    "layout(location = 0) out mediump vec4 fragColor;"
    "out mediump vec4 secondaryFragColor;"
    "void main() {\n"
    "    fragColor = vec4(1.0);\n"
    "    secondaryFragColor = vec4(1.0);\n"
    "}\n";

// It's also fine to leave locations completely unspecified.
const char ESSL300_TwoUnspecifiedLocationOutputsShader[] =
    "precision mediump float;\n"
    "out mediump vec4 fragColor;"
    "out mediump vec4 secondaryFragColor;"
    "void main() {\n"
    "    fragColor = vec4(1.0);\n"
    "    secondaryFragColor = vec4(1.0);\n"
    "}\n";

// Shader that is specifies two outputs with the same location but different indexes is valid.
const char ESSL300_LocationIndexShader[] =
    R"(precision mediump float;
layout(location = 0) out mediump vec4 fragColor;
layout(location = 0, index = 1) out mediump vec4 secondaryFragColor;
void main() {
    fragColor = vec4(1);
    secondaryFragColor = vec4(1);
})";

// Shader that specifies index layout qualifier but not location fails to compile.
const char ESSL300_LocationIndexFailureShader[] =
    R"(precision mediump float;
layout(index = 0) out vec4 fragColor;
void main() {
    fragColor = vec4(1.0);
})";

// Shader that specifies index layout qualifier multiple times fails to compile.
const char ESSL300_DoubleIndexFailureShader[] =
    R"(precision mediump float;
layout(index = 0, location = 0, index = 1) out vec4 fragColor;
void main() {
    fragColor = vec4(1.0);
})";

// Shader that specifies an output with out-of-bounds location
// for index 0 when another output uses index 1 is invalid.
const char ESSL300_Index0OutOfBoundsFailureShader[] =
    R"(precision mediump float;
layout(location = 1, index = 0) out mediump vec4 fragColor;
layout(location = 0, index = 1) out mediump vec4 secondaryFragColor;
void main() {
    fragColor = vec4(1);
    secondaryFragColor = vec4(1);
})";

// Shader that specifies an output with out-of-bounds location for index 1 is invalid.
const char ESSL300_Index1OutOfBoundsFailureShader[] =
    R"(precision mediump float;
layout(location = 1, index = 1) out mediump vec4 secondaryFragColor;
void main() {
    secondaryFragColor = vec4(1);
})";

// Shader that specifies two outputs with the same location
// but different indices and different base types is invalid.
const char ESSL300_IndexTypeMismatchFailureShader[] =
    R"(precision mediump float;
layout(location = 0, index = 0) out mediump vec4 fragColor;
layout(location = 0, index = 1) out mediump ivec4 secondaryFragColor;
void main() {
    fragColor = vec4(1);
    secondaryFragColor = ivec4(1);
})";

// Global index layout qualifier fails.
const char ESSL300_GlobalIndexFailureShader[] =
    R"(precision mediump float;
layout(index = 0);
out vec4 fragColor;
void main() {
    fragColor = vec4(1.0);
})";

// Index layout qualifier on a non-output variable fails.
const char ESSL300_IndexOnUniformVariableFailureShader[] =
    R"(precision mediump float;
layout(index = 0) uniform vec4 u;
out vec4 fragColor;
void main() {
    fragColor = u;
})";

// Index layout qualifier on a struct fails.
const char ESSL300_IndexOnStructFailureShader[] =
    R"(precision mediump float;
layout(index = 0) struct S {
    vec4 field;
};
out vec4 fragColor;
void main() {
    fragColor = vec4(1.0);
})";

// Index layout qualifier on a struct member fails.
const char ESSL300_IndexOnStructFieldFailureShader[] =
    R"(precision mediump float;
struct S {
    layout(index = 0) vec4 field;
};
out mediump vec4 fragColor;
void main() {
    fragColor = vec4(1.0);
})";

class EXTBlendFuncExtendedTest : public sh::ShaderExtensionTest
{
  protected:
    void SetUp() override
    {
        sh::ShaderExtensionTest::SetUp();
        // EXT_draw_buffers is used in some of the shaders for test purposes.
        mResources.EXT_draw_buffers = 1;
        mResources.NV_draw_buffers  = 2;
    }
};

// Extension flag is required to compile properly. Expect failure when it is
// not present.
TEST_P(EXTBlendFuncExtendedTest, CompileFailsWithoutExtension)
{
    mResources.EXT_blend_func_extended = 0;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTBFEPragma));
}

// Extension directive is required to compile properly. Expect failure when
// it is not present.
TEST_P(EXTBlendFuncExtendedTest, CompileFailsWithExtensionWithoutPragma)
{
    mResources.EXT_blend_func_extended  = 1;
    mResources.MaxDualSourceDrawBuffers = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(""));
}

// With extension flag and extension directive, compiling succeeds.
// Also test that the extension directive state is reset correctly.
TEST_P(EXTBlendFuncExtendedTest, CompileSucceedsWithExtensionAndPragma)
{
    mResources.EXT_blend_func_extended  = 1;
    mResources.MaxDualSourceDrawBuffers = 1;
    InitializeCompiler();
    EXPECT_TRUE(TestShaderCompile(EXTBFEPragma));
    // Test reset functionality.
    EXPECT_FALSE(TestShaderCompile(""));
    EXPECT_TRUE(TestShaderCompile(EXTBFEPragma));
}

// The SL #version 100 shaders that are correct work similarly
// in both GL2 and GL3, with and without the version string.
INSTANTIATE_TEST_SUITE_P(CorrectESSL100Shaders,
                         EXTBlendFuncExtendedTest,
                         Combine(Values(SH_GLES2_SPEC, SH_GLES3_SPEC),
                                 Values("", sh::ESSLVersion100),
                                 Values(ESSL100_SimpleShader1,
                                        ESSL100_MaxDualSourceAccessShader,
                                        ESSL100_FragDataShader)));

INSTANTIATE_TEST_SUITE_P(CorrectESSL300Shaders,
                         EXTBlendFuncExtendedTest,
                         Combine(Values(SH_GLES3_SPEC),
                                 Values(sh::ESSLVersion300),
                                 Values(ESSL300_MaxDualSourceAccessShader,
                                        ESSL300_LocationAndUnspecifiedOutputShader,
                                        ESSL300_TwoUnspecifiedLocationOutputsShader,
                                        ESSL300_LocationIndexShader)));

class EXTBlendFuncExtendedCompileFailureTest : public EXTBlendFuncExtendedTest
{};

TEST_P(EXTBlendFuncExtendedCompileFailureTest, CompileFails)
{
    // Expect compile failure due to shader error, with shader having correct pragma.
    mResources.EXT_blend_func_extended  = 1;
    mResources.MaxDualSourceDrawBuffers = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTBFEPragma));
}

// Incorrect #version 100 shaders fail.
INSTANTIATE_TEST_SUITE_P(IncorrectESSL100Shaders,
                         EXTBlendFuncExtendedCompileFailureTest,
                         Combine(Values(SH_GLES2_SPEC),
                                 Values(sh::ESSLVersion100),
                                 Values(ESSL100_ColorAndDataWriteFailureShader1,
                                        ESSL100_ColorAndDataWriteFailureShader2,
                                        ESSL100_ColorAndDataWriteFailureShader3)));

// Correct #version 100 shaders that are incorrect in WebGL 2.0.
INSTANTIATE_TEST_SUITE_P(IncorrectESSL100ShadersWebGL2,
                         EXTBlendFuncExtendedCompileFailureTest,
                         Combine(Values(SH_WEBGL2_SPEC),
                                 Values(sh::ESSLVersion100),
                                 Values(ESSL100_IndexSecondaryFragDataWithNonConstantShader)));

// Correct #version 300 es shaders fail in GLES2 context, regardless of version string.
INSTANTIATE_TEST_SUITE_P(CorrectESSL300Shaders,
                         EXTBlendFuncExtendedCompileFailureTest,
                         Combine(Values(SH_GLES2_SPEC),
                                 Values("", sh::ESSLVersion100, sh::ESSLVersion300),
                                 Values(ESSL300_LocationAndUnspecifiedOutputShader,
                                        ESSL300_TwoUnspecifiedLocationOutputsShader)));

// Correct #version 100 shaders fail when used with #version 300 es.
INSTANTIATE_TEST_SUITE_P(CorrectESSL100Shaders,
                         EXTBlendFuncExtendedCompileFailureTest,
                         Combine(Values(SH_GLES3_SPEC),
                                 Values(sh::ESSLVersion300),
                                 Values(ESSL100_SimpleShader1, ESSL100_FragDataShader)));

// Incorrect #version 300 es shaders always fail.
INSTANTIATE_TEST_SUITE_P(IncorrectESSL300Shaders,
                         EXTBlendFuncExtendedCompileFailureTest,
                         Combine(Values(SH_GLES3_1_SPEC),
                                 Values(sh::ESSLVersion300, sh::ESSLVersion310),
                                 Values(ESSL300_LocationIndexFailureShader,
                                        ESSL300_DoubleIndexFailureShader,
                                        ESSL300_Index0OutOfBoundsFailureShader,
                                        ESSL300_Index1OutOfBoundsFailureShader,
                                        ESSL300_IndexTypeMismatchFailureShader,
                                        ESSL300_GlobalIndexFailureShader,
                                        ESSL300_IndexOnUniformVariableFailureShader,
                                        ESSL300_IndexOnStructFailureShader,
                                        ESSL300_IndexOnStructFieldFailureShader)));

}  // namespace
