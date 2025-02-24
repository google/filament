//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GeometryShader_test.cpp:
// tests for compiling a Geometry Shader
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "compiler/translator/BaseTypes.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class GeometryShaderTest : public ShaderCompileTreeTest
{
  public:
    GeometryShaderTest() {}

  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->EXT_geometry_shader = 1;
    }

    ::GLenum getShaderType() const override { return GL_GEOMETRY_SHADER_EXT; }

    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }

    bool compileGeometryShader(const std::string &statement1, const std::string &statement2)
    {
        std::ostringstream sstream;
        sstream << kHeader << statement1 << statement2 << kEmptyBody;
        return compile(sstream.str());
    }

    bool compileGeometryShader(const std::string &statement1,
                               const std::string &statement2,
                               const std::string &statement3,
                               const std::string &statement4)
    {
        std::ostringstream sstream;
        sstream << kHeader << statement1 << statement2 << statement3 << statement4 << kEmptyBody;
        return compile(sstream.str());
    }

    static std::string GetGeometryShaderLayout(const std::string &layoutType,
                                               const std::string &primitive,
                                               int invocations,
                                               int maxVertices)
    {
        std::ostringstream sstream;

        sstream << "layout (";
        if (!primitive.empty())
        {
            sstream << primitive;
        }
        if (invocations > 0)
        {
            sstream << ", invocations = " << invocations;
        }
        if (maxVertices >= 0)
        {
            sstream << ", max_vertices = " << maxVertices;
        }
        sstream << ") " << layoutType << ";" << std::endl;

        return sstream.str();
    }

    static std::string GetInputDeclaration(const std::string &var, int size)
    {
        std::ostringstream sstream;

        sstream << "in ";
        if (size < 0)
        {
            sstream << var << "[];\n";
        }
        else
        {
            sstream << var << "[" << size << "];\n";
        }

        return sstream.str();
    }

    const std::string kVersion = "#version 310 es\n";
    const std::string kHeader =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n";
    const std::string kInputLayout  = "layout (points) in;\n";
    const std::string kOutputLayout = "layout (points, max_vertices = 1) out;\n";

    const std::array<std::string, 4> kInterpolationQualifiers = {{"flat", "smooth", "centroid"}};
    const std::map<std::string, int> kInputPrimitivesAndInputArraySizeMap = {
        {"points", 1},
        {"lines", 2},
        {"lines_adjacency", 4},
        {"triangles", 3},
        {"triangles_adjacency", 6}};

    const std::string kEmptyBody =
        "void main()\n"
        "{\n"
        "}\n";
};

class GeometryShaderOutputCodeTest : public MatchOutputCodeTest
{
  public:
    GeometryShaderOutputCodeTest() : MatchOutputCodeTest(GL_GEOMETRY_SHADER_EXT, SH_ESSL_OUTPUT)
    {
        ShCompileOptions defaultCompileOptions = {};
        defaultCompileOptions.objectCode       = true;
        setDefaultCompileOptions(defaultCompileOptions);

        getResources()->EXT_geometry_shader = 1;
    }
};

// Geometry Shaders are not supported in GLSL ES shaders version lower than 310.
TEST_F(GeometryShaderTest, Version300)
{
    const std::string &shaderString =
        R"(#version 300 es
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders are not supported in GLSL ES shaders version 310 without extension
// EXT_geometry_shader enabled.
TEST_F(GeometryShaderTest, Version310WithoutExtension)
{
    const std::string &shaderString =
        R"(#version 310 es
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders are not supported in GLSL ES shaders version 310 with extension
// EXT_geometry_shader disabled.
TEST_F(GeometryShaderTest, Version310ExtensionDisabled)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : disable
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders are supported in GLSL ES shaders version 310 with EXT_geometry_shader enabled.
TEST_F(GeometryShaderTest, Version310WithEXTExtension)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout(points) in;
        layout(points, max_vertices = 1) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Missing the declaration of input primitive in a geometry shader should be a link error instead of
// a compile error.
TEST_F(GeometryShaderTest, NoInputPrimitives)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout(points, max_vertices = 1) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders can only support 5 kinds of input primitives, which cannot be used as output
// primitives except 'points'.
// Skip testing "points" as it can be used as both input and output primitives.
TEST_F(GeometryShaderTest, ValidInputPrimitives)
{
    const std::array<std::string, 4> kInputPrimitives = {
        {"lines", "lines_adjacency", "triangles", "triangles_adjacency"}};

    for (const std::string &inputPrimitive : kInputPrimitives)
    {
        if (!compileGeometryShader(GetGeometryShaderLayout("in", inputPrimitive, -1, -1),
                                   kOutputLayout))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }
        if (compileGeometryShader(kInputLayout,
                                  GetGeometryShaderLayout("out", inputPrimitive, -1, 6)))
        {
            FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
        }
    }
}

// Geometry Shaders allow duplicated declaration of input primitive, but all of them must be same.
TEST_F(GeometryShaderTest, RedeclareInputPrimitives)
{
    const std::array<std::string, 5> kInputPrimitives = {
        {"points", "lines", "lines_adjacency", "triangles", "triangles_adjacency"}};

    for (GLuint i = 0; i < kInputPrimitives.size(); ++i)
    {
        const std::string &inputLayoutStr1 =
            GetGeometryShaderLayout("in", kInputPrimitives[i], -1, -1);
        if (!compileGeometryShader(inputLayoutStr1, inputLayoutStr1, kOutputLayout, ""))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }

        for (GLuint j = i + 1; j < kInputPrimitives.size(); ++j)
        {
            const std::string &inputLayoutStr2 =
                GetGeometryShaderLayout("in", kInputPrimitives[j], -1, -1);
            if (compileGeometryShader(inputLayoutStr1, inputLayoutStr2, kOutputLayout, ""))
            {
                FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
            }
        }
    }
}

// Geometry Shaders don't allow declaring different input primitives in one layout.
TEST_F(GeometryShaderTest, DeclareDifferentInputPrimitivesInOneLayout)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, triangles) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow 'invocations' < 1.
TEST_F(GeometryShaderTest, InvocationsLessThanOne)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, invocations = 0) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders allow declaring 'invocations' == 1 together with input primitive declaration in
// one layout.
TEST_F(GeometryShaderTest, InvocationsEqualsOne)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, invocations = 1) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow declaring 'invocations' == 1 in an individual layout.
TEST_F(GeometryShaderTest, InvocationsEqualsOneInSeparatedLayout)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (invocations = 1) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow 'invocations' larger than the implementation-dependent maximum value
// (32 in this test).
TEST_F(GeometryShaderTest, TooLargeInvocations)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, invocations = 9989899) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders allow 'invocations' declared together with input primitives in one layout.
TEST_F(GeometryShaderTest, InvocationsDeclaredWithInputPrimitives)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, invocations = 3) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow 'invocations' declared before input primitives in one input layout.
TEST_F(GeometryShaderTest, InvocationsBeforeInputPrimitives)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (invocations = 3, points) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow 'invocations' declared in an individual input layout.
TEST_F(GeometryShaderTest, InvocationsInIndividualLayout)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (invocations = 3) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow duplicated 'invocations' declarations.
TEST_F(GeometryShaderTest, DuplicatedInvocations)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, invocations = 3) in;
        layout (invocations = 3) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow multiple different 'invocations' declarations in different
// layouts.
TEST_F(GeometryShaderTest, RedeclareDifferentInvocations)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, invocations = 3) in;
        layout (invocations = 5) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow multiple different 'invocations' declarations in different
// layouts.
TEST_F(GeometryShaderTest, RedeclareDifferentInvocationsAfterInvocationEqualsOne)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, invocations = 1) in;
        layout (invocations = 5) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow multiple different 'invocations' declarations in one layout.
TEST_F(GeometryShaderTest, RedeclareDifferentInvocationsInOneLayout)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, invocations = 3, invocations = 5) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow 'invocations' in out layouts.
TEST_F(GeometryShaderTest, DeclareInvocationsInOutLayout)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, invocations = 3, max_vertices = 1) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow 'invocations' in layouts without 'in' qualifier.
TEST_F(GeometryShaderTest, DeclareInvocationsInLayoutNoQualifier)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (invocations = 3);
        layout (points, max_vertices = 1) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders allow declaring output primitive before input primitive declaration.
TEST_F(GeometryShaderTest, DeclareOutputPrimitiveBeforeInputPrimitiveDeclare)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, max_vertices = 1) out;
        layout (points) in;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow declaring 'max_vertices' before output primitive in one output layout.
TEST_F(GeometryShaderTest, DeclareMaxVerticesBeforeOutputPrimitive)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (max_vertices = 1, points) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Missing the declaration of output primitive should be a link error instead of a compile error in
// a geometry shader.
TEST_F(GeometryShaderTest, NoOutputPrimitives)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (max_vertices = 1) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders can only support 3 kinds of output primitives, which cannot be used as input
// primitives except 'points'.
// Skip testing "points" as it can be used as both input and output primitives.
TEST_F(GeometryShaderTest, ValidateOutputPrimitives)
{
    const std::string outputPrimitives[] = {"line_strip", "triangle_strip"};

    for (const std::string &outPrimitive : outputPrimitives)
    {
        if (!compileGeometryShader(kInputLayout,
                                   GetGeometryShaderLayout("out", outPrimitive, -1, 6)))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }

        if (compileGeometryShader(GetGeometryShaderLayout("in", outPrimitive, -1, -1),
                                  kOutputLayout))
        {
            FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
        }
    }
}

// Geometry Shaders allow duplicated output primitive declarations, but all of them must be same.
TEST_F(GeometryShaderTest, RedeclareOutputPrimitives)
{
    const std::array<std::string, 3> outPrimitives = {{"points", "line_strip", "triangle_strip"}};

    for (GLuint i = 0; i < outPrimitives.size(); i++)
    {
        constexpr int maxVertices = 1;
        const std::string &outputLayoutStr1 =
            GetGeometryShaderLayout("out", outPrimitives[i], -1, maxVertices);
        const std::string &outputLayoutStr2 =
            GetGeometryShaderLayout("out", outPrimitives[i], -1, -1);
        if (!compileGeometryShader(kInputLayout, outputLayoutStr1, outputLayoutStr2, ""))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }
        for (GLuint j = i + 1; j < outPrimitives.size(); j++)
        {
            const std::string &outputLayoutStr3 =
                GetGeometryShaderLayout("out", outPrimitives[j], -1, -1);
            if (compileGeometryShader(kInputLayout, outputLayoutStr1, outputLayoutStr3, ""))
            {
                FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
            }
        }
    }
}

// Geometry Shaders don't allow declaring different output primitives in one layout.
TEST_F(GeometryShaderTest, RedeclareDifferentOutputPrimitivesInOneLayout)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 3, line_strip) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Missing the declarations of output primitives and 'max_vertices' in a geometry shader should
// be a link error instead of a compile error.
TEST_F(GeometryShaderTest, NoOutLayouts)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Missing the declarations of 'max_vertices' in a geometry shader should be a link error
// instead of a compile error.
TEST_F(GeometryShaderTest, NoMaxVertices)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders cannot declare a negative 'max_vertices'.
TEST_F(GeometryShaderTest, NegativeMaxVertices)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = -1) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders allow max_vertices == 0.
TEST_F(GeometryShaderTest, ZeroMaxVertices)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 0) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders cannot declare a 'max_vertices' that is greater than
// MAX_GEOMETRY_OUTPUT_VERTICES_EXT (256 in this test).
TEST_F(GeometryShaderTest, TooLargeMaxVertices)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 257) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders can declare 'max_vertices' in an individual out layout.
TEST_F(GeometryShaderTest, MaxVerticesInIndividualLayout)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 1) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow duplicated 'max_vertices' declarations.
TEST_F(GeometryShaderTest, DuplicatedMaxVertices)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 1) out;
        layout (max_vertices = 1) out;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow declaring different 'max_vertices'.
TEST_F(GeometryShaderTest, RedeclareDifferentMaxVertices)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 1) out;
        layout (max_vertices = 2) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow declaring different 'max_vertices'.
TEST_F(GeometryShaderTest, RedeclareDifferentMaxVerticesInOneLayout)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2, max_vertices = 1) out;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow 'location' declared with input/output primitives in one layout.
TEST_F(GeometryShaderTest, InvalidLocation)
{
    const std::string &shaderString1 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, location = 1) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
        })";

    const std::string &shaderString2 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (invocations = 2, location = 1) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
        })";

    const std::string &shaderString3 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, location = 3, max_vertices = 2) out;
        void main()
        {
        })";

    const std::string &shaderString4 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 2, location = 3) out;
        void main()
        {
        })";

    if (compile(shaderString1) || compile(shaderString2) || compile(shaderString3) ||
        compile(shaderString4))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow invalid layout qualifier declarations.
TEST_F(GeometryShaderTest, InvalidLayoutQualifiers)
{
    const std::string &shaderString1 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, abc) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
        })";

    const std::string &shaderString2 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, abc, max_vertices = 2) out;
        void main()
        {
        })";

    const std::string &shaderString3 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, xyz = 2) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
        })";

    const std::string &shaderString4 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 2, xyz = 3) out;
        void main()
        {
        })";

    if (compile(shaderString1) || compile(shaderString2) || compile(shaderString3) ||
        compile(shaderString4))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that indexing an array with a constant integer on gl_in is legal.
TEST_F(GeometryShaderTest, IndexGLInByConstantInteger)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            vec4 position;
            position = gl_in[0].gl_Position;
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that indexing an array with an integer variable on gl_in is legal.
TEST_F(GeometryShaderTest, IndexGLInByVariable)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (lines) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            vec4 position;
            for (int i = 0; i < 2; i++)
            {
                position = gl_in[i].gl_Position;
            }
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that indexing an array on gl_in without input primitive declaration causes a compile
// error.
TEST_F(GeometryShaderTest, IndexGLInWithoutInputPrimitive)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, max_vertices = 2) out;
        void main()
        {
            vec4 position = gl_in[0].gl_Position;
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that using gl_in.length() without input primitive declaration causes a compile error.
TEST_F(GeometryShaderTest, UseGLInLengthWithoutInputPrimitive)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, max_vertices = 2) out;
        void main()
        {
            int length = gl_in.length();
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that using gl_in.length() with input primitive declaration can compile.
TEST_F(GeometryShaderTest, UseGLInLengthWithInputPrimitive)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            gl_Position = vec4(gl_in.length());
            EmitVertex();
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that gl_in[].gl_Position cannot be l-value.
TEST_F(GeometryShaderTest, AssignValueToGLIn)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            gl_in[0].gl_Position = vec4(0, 0, 0, 1);
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify Geometry Shader supports all required built-in variables.
TEST_F(GeometryShaderTest, BuiltInVariables)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, invocations = 2) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            gl_Position = gl_in[gl_InvocationID].gl_Position;
            int invocation = gl_InvocationID;
            gl_Layer = invocation;
            int primitiveIn = gl_PrimitiveIDIn;
            gl_PrimitiveID = primitiveIn;
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that gl_PrimitiveIDIn cannot be l-value.
TEST_F(GeometryShaderTest, AssignValueToGLPrimitiveIn)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, invocations = 2) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            gl_PrimitiveIDIn = 1;
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that gl_InvocationID cannot be l-value.
TEST_F(GeometryShaderTest, AssignValueToGLInvocations)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, invocations = 2) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            gl_InvocationID = 1;
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that both EmitVertex() and EndPrimitive() are supported in Geometry Shader.
TEST_F(GeometryShaderTest, GeometryShaderBuiltInFunctions)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            gl_Position = gl_in[0].gl_Position;
            EmitVertex();
            EndPrimitive();
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that using EmitVertex() or EndPrimitive() without GL_EXT_geometry_shader declared causes a
// compile error.
TEST_F(GeometryShaderTest, GeometryShaderBuiltInFunctionsWithoutExtension)
{
    const std::string &shaderString1 =
        R"(#version 310 es
        void main()
        {
            EmitVertex();
        })";

    const std::string &shaderString2 =
        R"(#version 310 es
        void main()
        {
            EndPrimitive();
        })";

    if (compile(shaderString1) || compile(shaderString2))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that all required built-in constant values are supported in Geometry Shaders
TEST_F(GeometryShaderTest, GeometryShaderBuiltInConstants)
{
    const std::string &kShaderHeader =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            gl_Position.x = float()";

    const std::array<std::string, 9> kGeometryShaderBuiltinConstants = {{
        "gl_MaxGeometryInputComponents",
        "gl_MaxGeometryOutputComponents",
        "gl_MaxGeometryImageUniforms",
        "gl_MaxGeometryTextureImageUnits",
        "gl_MaxGeometryOutputVertices",
        "gl_MaxGeometryTotalOutputComponents",
        "gl_MaxGeometryUniformComponents",
        "gl_MaxGeometryAtomicCounters",
        "gl_MaxGeometryAtomicCounterBuffers",
    }};

    const std::string &kShaderTail =
        R"();
            EmitVertex();
        })";

    for (const std::string &kGSBuiltinConstant : kGeometryShaderBuiltinConstants)
    {
        std::ostringstream ostream;
        ostream << kShaderHeader << kGSBuiltinConstant << kShaderTail;
        if (!compile(ostream.str()))
        {
            FAIL() << "Shader compilation failed, expecting success: \n" << mInfoLog;
        }
    }
}

// Verify that using any Geometry Shader built-in constant values without GL_EXT_geometry_shader
// declared causes a compile error.
TEST_F(GeometryShaderTest, GeometryShaderBuiltInConstantsWithoutExtension)
{
    const std::string &kShaderHeader =
        "#version 310 es\n"
        "void main()\n"
        "{\n"
        "    int val = ";

    const std::array<std::string, 9> kGeometryShaderBuiltinConstants = {{
        "gl_MaxGeometryInputComponents",
        "gl_MaxGeometryOutputComponents",
        "gl_MaxGeometryImageUniforms",
        "gl_MaxGeometryTextureImageUnits",
        "gl_MaxGeometryOutputVertices",
        "gl_MaxGeometryTotalOutputComponents",
        "gl_MaxGeometryUniformComponents",
        "gl_MaxGeometryAtomicCounters",
        "gl_MaxGeometryAtomicCounterBuffers",
    }};

    const std::string &kShaderTail =
        ";\n"
        "}\n";

    for (const std::string &kGSBuiltinConstant : kGeometryShaderBuiltinConstants)
    {
        std::ostringstream ostream;
        ostream << kShaderHeader << kGSBuiltinConstant << kShaderTail;
        if (compile(ostream.str()))
        {
            FAIL() << "Shader compilation succeeded, expecting failure: \n" << mInfoLog;
        }
    }
}

// Verify that Geometry Shaders cannot accept non-array inputs.
TEST_F(GeometryShaderTest, NonArrayInput)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 1) out;
        in vec4 texcoord;
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that compilation errors do not occur even if declaring an unsized Geometry Shader input
// before a valid input primitive declaration.
TEST_F(GeometryShaderTest, DeclareUnsizedInputBeforeInputPrimitive)
{
    const std::string &shaderString1 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        in vec4 texcoord[];
        layout (points) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
            vec4 coord = texcoord[0];
            int length = texcoord.length();
        })";

    const std::string &shaderString2 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        in vec4 texcoord1[1];
        in vec4 texcoord2[];
        layout (points) in;
        layout (points, max_vertices = 1) out;
        void main()
        {
            vec4 coord = texcoord2[0];
            int length = texcoord2.length();
        })";

    if (!compile(shaderString1) || !compile(shaderString2))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that it is a compile error to declare an unsized Geometry Shader input without a valid
// input primitive declaration.
TEST_F(GeometryShaderTest, DeclareUnsizedInputWithoutInputPrimitive)
{
    const std::string &shaderString1 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, max_vertices = 1) out;
        in vec4 texcoord[];
        void main()
        {
        })";

    const std::string &shaderString2 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, max_vertices = 1) out;
        in vec4 texcoord1[1];
        in vec4 texcoord2[];
        void main()
        {
        })";

    if (compile(shaderString1) || compile(shaderString2))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that indexing an unsized Geometry Shader input which is declared after a
// valid input primitive declaration can compile.
TEST_F(GeometryShaderTest, IndexingUnsizedInputDeclaredAfterInputPrimitive)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 1) out;
        in vec4 texcoord[], texcoord2[];
        in vec4[] texcoord3, texcoord4;
        void main()
        {
            gl_Position = texcoord[0] + texcoord2[0] + texcoord3[0] + texcoord4[0];
            EmitVertex();
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that calling length() function on an unsized Geometry Shader input which
// is declared before a valid input primitive declaration can compile.
TEST_F(GeometryShaderTest, CallingLengthOnUnsizedInputDeclaredAfterInputPrimitive)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 1) out;
        in vec4 texcoord[];
        void main()
        {
            gl_Position = vec4(texcoord.length());
            EmitVertex();
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that assigning a value to the input of a geometry shader causes a compile error.
TEST_F(GeometryShaderTest, AssignValueToInput)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 1) out;
        in vec4 texcoord[];
        void main()
        {
            texcoord[0] = vec4(1.0, 0.0, 0.0, 1.0);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders allow inputs with location qualifier.
TEST_F(GeometryShaderTest, InputWithLocations)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (triangles) in;
        layout (points, max_vertices = 1) out;
        layout (location = 0) in vec4 texcoord1[];
        layout (location = 1) in vec4 texcoord2[];
        void main()
        {
            int index = 0;
            vec4 coord1 = texcoord1[0];
            vec4 coord2 = texcoord2[index];
            gl_Position = coord1 + coord2;
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow inputs with explicit size declared before the declaration of the
// input primitive, but they should have same size and match the declaration of the
// following input primitive declarations.
TEST_F(GeometryShaderTest, InputWithSizeBeforeInputPrimitive)
{
    for (auto &primitiveAndArraySize : kInputPrimitivesAndInputArraySizeMap)
    {
        const std::string &inputLayoutStr =
            GetGeometryShaderLayout("in", primitiveAndArraySize.first, -1, -1);
        const int inputSize = primitiveAndArraySize.second;

        const std::string &inputDeclaration1 = GetInputDeclaration("vec4 input1", inputSize);
        if (!compileGeometryShader(inputDeclaration1, "", inputLayoutStr, kOutputLayout))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }

        const std::string &inputDeclaration2 = GetInputDeclaration("vec4 input2", inputSize + 1);
        if (compileGeometryShader(inputDeclaration2, "", inputLayoutStr, kOutputLayout) ||
            compileGeometryShader(inputDeclaration1, inputDeclaration2, inputLayoutStr,
                                  kOutputLayout))
        {
            FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
        }
    }
}

// Geometry shaders allow inputs with explicit size declared after the declaration of the
// input primitive, but their sizes should match the previous input primitive declaration.
TEST_F(GeometryShaderTest, InputWithSizeAfterInputPrimitive)
{
    for (auto &primitiveAndArraySize : kInputPrimitivesAndInputArraySizeMap)
    {
        const std::string &inputLayoutStr =
            GetGeometryShaderLayout("in", primitiveAndArraySize.first, -1, -1);
        const int inputSize = primitiveAndArraySize.second;

        const std::string &inputDeclaration1 = GetInputDeclaration("vec4 input1", inputSize);
        if (!compileGeometryShader(inputLayoutStr, kOutputLayout, inputDeclaration1, ""))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }

        const std::string &inputDeclaration2 = GetInputDeclaration("vec4 input2", inputSize + 1);
        if (compileGeometryShader(inputLayoutStr, kOutputLayout, inputDeclaration2, ""))
        {
            FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
        }
    }
}

// Verify that Geometry Shaders accept non-array outputs.
TEST_F(GeometryShaderTest, NonArrayOutputs)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 1) out;
        out vec4 color;
        void main()
        {
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that Geometry Shaders allow declaring outputs with 'location' layout qualifier.
TEST_F(GeometryShaderTest, OutputsWithLocation)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (triangles) in;
        layout (points, max_vertices = 1) out;
        layout (location = 0) out vec4 color1;
        layout (location = 1) out vec4 color2;
        void main()
        {
            color1 = vec4(0.0, 1.0, 0.0, 1.0);
            color2 = vec4(1.0, 0.0, 0.0, 1.0);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow declaring sized array outputs.
TEST_F(GeometryShaderTest, SizedArrayOutputs)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (triangles) in;
        layout (points, max_vertices = 1) out;
        out vec4 color[2];
        void main()
        {
            color[0] = vec4(0.0, 1.0, 0.0, 1.0);
            color[1] = vec4(1.0, 0.0, 0.0, 1.0);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that Geometry Shader outputs cannot be declared as an unsized array.
TEST_F(GeometryShaderTest, UnsizedArrayOutputs)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (triangles) in;
        layout (points, max_vertices = 1) out;
        out vec4 color[];
        void main()
        {
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that Geometry Shader inputs can use interpolation qualifiers.
TEST_F(GeometryShaderTest, InputWithInterpolationQualifiers)
{
    for (const std::string &qualifier : kInterpolationQualifiers)
    {
        std::ostringstream stream;
        stream << kHeader << kInputLayout << kOutputLayout << qualifier << " in vec4 texcoord[];\n"
               << kEmptyBody;

        if (!compile(stream.str()))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }
    }
}

// Verify that Geometry Shader outputs can use interpolation qualifiers.
TEST_F(GeometryShaderTest, OutputWithInterpolationQualifiers)
{
    for (const std::string &qualifier : kInterpolationQualifiers)
    {
        std::ostringstream stream;
        stream << kHeader << kInputLayout << kOutputLayout << qualifier << " out vec4 color;\n"
               << kEmptyBody;

        if (!compile(stream.str()))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }
    }
}

// Verify that Geometry Shader outputs can use 'invariant' qualifier.
TEST_F(GeometryShaderTest, InvariantOutput)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2) out;
        invariant out vec4 gs_output;
        void main()
        {
            gl_Position = gl_in[0].gl_Position;
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that the member of gl_in won't be incorrectly changed in the output shader string.
TEST_F(GeometryShaderOutputCodeTest, ValidateGLInMembersInOutputShaderString)
{
    const std::string &shaderString1 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (lines) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            vec4 position;
            for (int i = 0; i < 2; i++)
            {
                position = gl_in[i].gl_Position;
            }
        })";

    compile(shaderString1);
    EXPECT_TRUE(foundInESSLCode("].gl_Position"));

    const std::string &shaderString2 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            vec4 position;
            position = gl_in[0].gl_Position;
        })";

    compile(shaderString2);
    EXPECT_TRUE(foundInESSLCode("].gl_Position"));
}

// Verify that geometry shader inputs can be declared as struct arrays.
TEST_F(GeometryShaderTest, StructArrayInput)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2) out;
        struct S
        {
            float value1;
            vec4 value2;
        };
        in S gs_input[];
        out S gs_output;
        void main()
        {
            gs_output = gs_input[0];
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that geometry shader outputs cannot be declared as struct arrays.
TEST_F(GeometryShaderTest, StructArrayOutput)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2) out;
        struct S
        {
            float value1;
            vec4 value2;
        };
        out S gs_output[1];
        void main()
        {
            gs_output[0].value1 = 1.0;
            gs_output[0].value2 = vec4(1.0, 0.0, 0.0, 1.0);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}
