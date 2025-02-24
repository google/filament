//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GeometryShaderTest.cpp : Tests of the implementation of geometry shader

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class GeometryShaderTest : public ANGLETest<>
{
  protected:
    GeometryShaderTest()
    {
        setWindowWidth(64);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    static std::string CreateEmptyGeometryShader(const std::string &inputPrimitive,
                                                 const std::string &outputPrimitive,
                                                 int invocations,
                                                 int maxVertices)
    {
        std::ostringstream ostream;
        ostream << "#version 310 es\n"
                   "#extension GL_EXT_geometry_shader : require\n";
        if (!inputPrimitive.empty())
        {
            ostream << "layout (" << inputPrimitive << ") in;\n";
        }
        if (!outputPrimitive.empty())
        {
            ostream << "layout (" << outputPrimitive << ") out;\n";
        }
        if (invocations > 0)
        {
            ostream << "layout (invocations = " << invocations << ") in;\n";
        }
        if (maxVertices >= 0)
        {
            ostream << "layout (max_vertices = " << maxVertices << ") out;\n";
        }
        ostream << "void main()\n"
                   "{\n"
                   "}";
        return ostream.str();
    }

    void setupLayeredFramebuffer(GLuint framebuffer,
                                 GLuint color0,
                                 GLuint color1,
                                 GLuint depthStencil,
                                 GLenum colorTarget,
                                 const GLColor &color0InitialColor,
                                 const GLColor &color1InitialColor,
                                 float depthInitialValue,
                                 GLint stencilInitialValue);
    void setupLayeredFramebufferProgram(GLProgram *program);
    void verifyLayeredFramebufferColor(GLuint colorTexture,
                                       GLenum colorTarget,
                                       const GLColor expected[],
                                       GLsizei layerCount);
    void verifyLayeredFramebufferDepthStencil(GLuint depthStencilTexture,
                                              const float expectedDepth[],
                                              const GLint expectedStencil[],
                                              GLsizei layerCount);

    void layeredFramebufferClearTest(GLenum colorTarget);
    void layeredFramebufferPreRenderClearTest(GLenum colorTarget, bool doubleClear);
    void layeredFramebufferMidRenderClearTest(GLenum colorTarget);
    void callFramebufferTextureAPI(APIExtensionVersion usedExtension,
                                   GLenum target,
                                   GLenum attachment,
                                   GLuint texture,
                                   GLint level);
    void testNegativeFramebufferTexture(APIExtensionVersion usedExtension);
    void testCreateAndAttachGeometryShader(APIExtensionVersion usedExtension);

    static constexpr GLsizei kWidth              = 16;
    static constexpr GLsizei kHeight             = 16;
    static constexpr GLsizei kColor0Layers       = 4;
    static constexpr GLsizei kColor1Layers       = 3;
    static constexpr GLsizei kDepthStencilLayers = 5;
    static constexpr GLsizei kFramebufferLayers =
        std::min({kColor0Layers, kColor1Layers, kDepthStencilLayers});
};

class GeometryShaderTestES3 : public ANGLETest<>
{};

class GeometryShaderTestES32 : public GeometryShaderTest
{};

// Verify that a geometry shader cannot be created in an OpenGL ES 3.0 context, since at least
// ES 3.1 is required.
TEST_P(GeometryShaderTestES3, CreateGeometryShaderInES3)
{
    EXPECT_TRUE(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER_EXT);
    EXPECT_EQ(0u, geometryShader);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

void GeometryShaderTest::testCreateAndAttachGeometryShader(APIExtensionVersion usedExtension)
{
    ASSERT(usedExtension == APIExtensionVersion::EXT || usedExtension == APIExtensionVersion::OES ||
           usedExtension == APIExtensionVersion::Core);

    std::string gs;

    constexpr char kGLSLVersion31[] = R"(#version 310 es
)";
    constexpr char kGLSLVersion32[] = R"(#version 320 es
)";
    constexpr char kGeometryEXT[]   = R"(#extension GL_EXT_geometry_shader : require
)";
    constexpr char kGeometryOES[]   = R"(#extension GL_OES_geometry_shader : require
)";

    if (usedExtension == APIExtensionVersion::EXT)
    {
        gs.append(kGLSLVersion31);
        gs.append(kGeometryEXT);
    }
    else if (usedExtension == APIExtensionVersion::OES)
    {
        gs.append(kGLSLVersion31);
        gs.append(kGeometryOES);
    }
    else
    {
        gs.append(kGLSLVersion32);
    }

    constexpr char kGSBody[] = R"(
layout (invocations = 3, triangles) in;
layout (triangle_strip, max_vertices = 3) out;
in vec4 texcoord[];
out vec4 o_texcoord;
void main()
{
    int n;
    for (n = 0; n < gl_in.length(); n++)
    {
        gl_Position = gl_in[n].gl_Position;
        gl_Layer   = gl_InvocationID;
        o_texcoord = texcoord[n];
        EmitVertex();
    }
    EndPrimitive();
})";
    gs.append(kGSBody);

    GLuint geometryShader = CompileShader(GL_GEOMETRY_SHADER_EXT, gs.c_str());
    EXPECT_NE(0u, geometryShader);

    GLuint programID = glCreateProgram();
    glAttachShader(programID, geometryShader);

    glDetachShader(programID, geometryShader);
    glDeleteShader(geometryShader);
    glDeleteProgram(programID);

    EXPECT_GL_NO_ERROR();
}

// Verify that a geometry shader can be created and attached to a program using the EXT extension.
TEST_P(GeometryShaderTest, CreateAndAttachGeometryShaderEXT)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    testCreateAndAttachGeometryShader(APIExtensionVersion::EXT);
}

// Verify that a geometry shader can be created and attached to a program using the OES extension.
TEST_P(GeometryShaderTest, CreateAndAttachGeometryShaderOES)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_geometry_shader"));
    testCreateAndAttachGeometryShader(APIExtensionVersion::OES);
}

// Verify that a geometry shader can be created and attached to a program in GLES 3.2.
TEST_P(GeometryShaderTestES32, CreateAndAttachGeometryShader)
{
    testCreateAndAttachGeometryShader(APIExtensionVersion::Core);
}

// Verify that Geometry Shader can be compiled when geometry shader array input size
// is set after shader input variables.
// http://anglebug.com/42265598 GFXBench Car Chase uses this pattern
TEST_P(GeometryShaderTest, DeferredSetOfArrayInputSize)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    // "layout (invocations = 3, triangles) in;" should be declared before "in vec4 texcoord [];",
    // but here it is declared later.
    constexpr char kGS[]  = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
in vec4 texcoord[];
out vec4 o_texcoord;
layout (invocations = 3, triangles_adjacency) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
    int n;
    for (n = 0; n < gl_in.length(); n++)
    {
        gl_Position = gl_in[n].gl_Position;
        gl_Layer   = gl_InvocationID;
        o_texcoord = texcoord[n];
        EmitVertex();
    }
    EndPrimitive();
})";
    GLuint geometryShader = CompileShader(GL_GEOMETRY_SHADER_EXT, kGS);
    EXPECT_NE(0u, geometryShader);
    GLuint programID = glCreateProgram();
    glAttachShader(programID, geometryShader);
    glDetachShader(programID, geometryShader);
    glDeleteShader(geometryShader);
    glDeleteProgram(programID);
    EXPECT_GL_NO_ERROR();
}

// Verify that all the implementation dependent geometry shader related resource limits meet the
// requirement of GL_EXT_geometry_shader SPEC.
TEST_P(GeometryShaderTest, GeometryShaderImplementationDependentLimits)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    // http://anglebug.com/42264048
    ANGLE_SKIP_TEST_IF(IsIntel() && IsVulkan() && IsLinux());

    const std::map<GLenum, int> limits = {{GL_MAX_FRAMEBUFFER_LAYERS_EXT, 256},
                                          {GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT, 1024},
                                          {GL_MAX_GEOMETRY_UNIFORM_BLOCKS_EXT, 12},
                                          {GL_MAX_GEOMETRY_INPUT_COMPONENTS_EXT, 64},
                                          {GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_EXT, 64},
                                          {GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, 256},
                                          {GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT, 1024},
                                          {GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT, 16},
                                          {GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_EXT, 0},
                                          {GL_MAX_GEOMETRY_ATOMIC_COUNTERS_EXT, 0},
                                          {GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT, 0},
                                          {GL_MAX_GEOMETRY_IMAGE_UNIFORMS_EXT, 0},
                                          {GL_MAX_GEOMETRY_SHADER_INVOCATIONS_EXT, 32}};

    GLint value;
    for (const auto &limit : limits)
    {
        value = 0;
        glGetIntegerv(limit.first, &value);
        EXPECT_GL_NO_ERROR();
        EXPECT_GE(value, limit.second);
    }

    value = 0;
    glGetIntegerv(GL_LAYER_PROVOKING_VERTEX_EXT, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_TRUE(value == GL_FIRST_VERTEX_CONVENTION_EXT || value == GL_LAST_VERTEX_CONVENTION_EXT ||
                value == GL_UNDEFINED_VERTEX_EXT);
}

// Verify that all the combined resource limits meet the requirement of GL_EXT_geometry_shader SPEC.
TEST_P(GeometryShaderTest, CombinedResourceLimits)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    // See http://anglebug.com/42260977.
    ANGLE_SKIP_TEST_IF(IsAndroid());

    const std::map<GLenum, int> limits = {{GL_MAX_UNIFORM_BUFFER_BINDINGS, 48},
                                          {GL_MAX_COMBINED_UNIFORM_BLOCKS, 36},
                                          {GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 64}};

    GLint value;
    for (const auto &limit : limits)
    {
        value = 0;
        glGetIntegerv(limit.first, &value);
        EXPECT_GL_NO_ERROR();
        EXPECT_GE(value, limit.second);
    }
}

// Verify that linking a program with an uncompiled geometry shader causes a link failure.
TEST_P(GeometryShaderTest, LinkWithUncompiledGeometryShader)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    GLuint vertexShader   = CompileShader(GL_VERTEX_SHADER, essl31_shaders::vs::Simple());
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, essl31_shaders::fs::Red());
    ASSERT_NE(0u, vertexShader);
    ASSERT_NE(0u, fragmentShader);

    GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER_EXT);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glAttachShader(program, geometryShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(geometryShader);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(0, linkStatus);

    glDeleteProgram(program);
    ASSERT_GL_NO_ERROR();
}

// Verify that linking a program with geometry shader whose version is different from other shaders
// in this program causes a link error.
TEST_P(GeometryShaderTest, LinkWhenShaderVersionMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    const std::string &emptyGeometryShader = CreateEmptyGeometryShader("points", "points", 2, 1);

    GLuint program = CompileProgramWithGS(essl3_shaders::vs::Simple(), emptyGeometryShader.c_str(),
                                          essl3_shaders::fs::Red());
    EXPECT_EQ(0u, program);
}

// Verify that linking a program with geometry shader that lacks input primitive,
// output primitive, or declaration on 'max_vertices' causes a link failure.
TEST_P(GeometryShaderTest, LinkValidationOnGeometryShaderLayouts)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    const std::string gsWithoutInputPrimitive  = CreateEmptyGeometryShader("", "points", 2, 1);
    const std::string gsWithoutOutputPrimitive = CreateEmptyGeometryShader("points", "", 2, 1);
    const std::string gsWithoutInvocations = CreateEmptyGeometryShader("points", "points", -1, 1);
    const std::string gsWithoutMaxVertices = CreateEmptyGeometryShader("points", "points", 2, -1);

    // Linking a program with a geometry shader that only lacks 'invocations' should not cause a
    // link failure.
    GLuint program = CompileProgramWithGS(essl31_shaders::vs::Simple(),
                                          gsWithoutInvocations.c_str(), essl31_shaders::fs::Red());
    EXPECT_NE(0u, program);

    glDeleteProgram(program);

    // Linking a program with a geometry shader that lacks input primitive, output primitive or
    // 'max_vertices' causes a link failure.
    program = CompileProgramWithGS(essl31_shaders::vs::Simple(), gsWithoutInputPrimitive.c_str(),
                                   essl31_shaders::fs::Red());
    EXPECT_EQ(0u, program);

    program = CompileProgramWithGS(essl31_shaders::vs::Simple(), gsWithoutOutputPrimitive.c_str(),
                                   essl31_shaders::fs::Red());
    EXPECT_EQ(0u, program);

    program = CompileProgramWithGS(essl31_shaders::vs::Simple(), gsWithoutMaxVertices.c_str(),
                                   essl31_shaders::fs::Red());
    EXPECT_EQ(0u, program);

    ASSERT_GL_NO_ERROR();
}

// Verify that an link error occurs when the vertex shader has an array output and there is a
// geometry shader in the program.
TEST_P(GeometryShaderTest, VertexShaderArrayOutput)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr char kVS[] = R"(#version 310 es
in vec4 vertex_in;
out vec4 vertex_out[3];
void main()
{
    gl_Position = vertex_in;
    vertex_out[0] = vec4(1.0, 0.0, 0.0, 1.0);
    vertex_out[1] = vec4(0.0, 1.0, 0.0, 1.0);
    vertex_out[2] = vec4(0.0, 0.0, 1.0, 1.0);
})";

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout (invocations = 3, triangles) in;
layout (points, max_vertices = 3) out;
in vec4 vertex_out[];
out vec4 geometry_color;
void main()
{
    gl_Position = gl_in[0].gl_Position;
    geometry_color = vertex_out[0];
    EmitVertex();
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
in vec4 geometry_color;
layout (location = 0) out vec4 output_color;
void main()
{
    output_color = geometry_color;
})";

    GLuint program = CompileProgramWithGS(kVS, kGS, kFS);
    EXPECT_EQ(0u, program);

    EXPECT_GL_NO_ERROR();
}

// Verify that an link error occurs when the definition of a unform in fragment shader is different
// from those in a geometry shader.
TEST_P(GeometryShaderTest, UniformMismatchBetweenGeometryAndFragmentShader)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr char kVS[] = R"(#version 310 es
uniform highp vec4 uniform_value_vert;
in vec4 vertex_in;
out vec4 vertex_out;
void main()
{
    gl_Position = vertex_in;
    vertex_out = uniform_value_vert;
})";

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
uniform vec4 uniform_value;
layout (invocations = 3, triangles) in;
layout (points, max_vertices = 3) out;
in vec4 vertex_out[];
out vec4 geometry_color;
void main()
{
    gl_Position = gl_in[0].gl_Position;
    geometry_color = vertex_out[0] + uniform_value;
    EmitVertex();
})";

    constexpr char kFS[] = R"(#version 310 es
precision highp float;
uniform float uniform_value;
in vec4 geometry_color;
layout (location = 0) out vec4 output_color;
void main()
{
    output_color = vec4(geometry_color.rgb, uniform_value);
})";

    GLuint program = CompileProgramWithGS(kVS, kGS, kFS);
    EXPECT_EQ(0u, program);

    EXPECT_GL_NO_ERROR();
}

// Verify that an link error occurs when the number of uniform blocks in a geometry shader exceeds
// MAX_GEOMETRY_UNIFORM_BLOCKS_EXT.
TEST_P(GeometryShaderTest, TooManyUniformBlocks)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    GLint maxGeometryUniformBlocks = 0;
    glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS_EXT, &maxGeometryUniformBlocks);

    GLint numUniformBlocks = maxGeometryUniformBlocks + 1;
    std::ostringstream stream;
    stream << "#version 310 es\n"
              "#extension GL_EXT_geometry_shader : require\n"
              "uniform ubo\n"
              "{\n"
              "    vec4 value1;\n"
              "} block0["
           << numUniformBlocks
           << "];\n"
              "layout (triangles) in;\n"
              "layout (points, max_vertices = 1) out;\n"
              "void main()\n"
              "{\n"
              "    gl_Position = gl_in[0].gl_Position;\n";

    for (GLint i = 0; i < numUniformBlocks; ++i)
    {
        stream << "    gl_Position += block0[" << i << "].value1;\n";
    }
    stream << "    EmitVertex();\n"
              "}\n";

    GLuint program = CompileProgramWithGS(essl31_shaders::vs::Simple(), stream.str().c_str(),
                                          essl31_shaders::fs::Red());
    EXPECT_EQ(0u, program);

    EXPECT_GL_NO_ERROR();
}

// Verify that an link error occurs when the number of shader storage blocks in a geometry shader
// exceeds MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT.
TEST_P(GeometryShaderTest, TooManyShaderStorageBlocks)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    GLint maxGeometryShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT, &maxGeometryShaderStorageBlocks);

    GLint numSSBOs = maxGeometryShaderStorageBlocks + 1;
    std::ostringstream stream;
    stream << "#version 310 es\n"
              "#extension GL_EXT_geometry_shader : require\n"
              "buffer ssbo\n"
              "{\n"
              "    vec4 value1;\n"
              "} block0["
           << numSSBOs
           << "];\n"
              "layout (triangles) in;\n"
              "layout (points, max_vertices = 1) out;\n"
              "void main()\n"
              "{\n"
              "    gl_Position = gl_in[0].gl_Position;\n";

    for (GLint i = 0; i < numSSBOs; ++i)
    {
        stream << "    gl_Position += block0[" << i << "].value1;\n";
    }
    stream << "    EmitVertex();\n"
              "}\n";

    GLuint program = CompileProgramWithGS(essl31_shaders::vs::Simple(), stream.str().c_str(),
                                          essl31_shaders::fs::Red());
    EXPECT_EQ(0u, program);

    EXPECT_GL_NO_ERROR();
}

// Verify that an link error occurs when the definition of a unform block in the vertex shader is
// different from that in a geometry shader.
TEST_P(GeometryShaderTest, UniformBlockMismatchBetweenVertexAndGeometryShader)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr char kVS[] = R"(#version 310 es
uniform ubo
{
    vec4 uniform_value_vert;
} block0;
in vec4 vertex_in;
out vec4 vertex_out;
void main()
{
    gl_Position = vertex_in;
    vertex_out = block0.uniform_value_vert;
})";

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
uniform ubo
{
    vec4 uniform_value_geom;
} block0;
layout (triangles) in;
layout (points, max_vertices = 1) out;
in vec4 vertex_out[];
void main()
{
    gl_Position = gl_in[0].gl_Position + vertex_out[0];
    gl_Position += block0.uniform_value_geom;
    EmitVertex();
})";

    GLuint program = CompileProgramWithGS(kVS, kGS, essl31_shaders::fs::Red());
    EXPECT_EQ(0u, program);

    EXPECT_GL_NO_ERROR();
}

// Verify that an link error occurs when the definition of a shader storage block in the geometry
// shader is different from that in a fragment shader.
TEST_P(GeometryShaderTest, ShaderStorageBlockMismatchBetweenGeometryAndFragmentShader)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    GLint maxGeometryShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT, &maxGeometryShaderStorageBlocks);

    // The minimun value of MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT can be 0.
    // [EXT_geometry_shader] Table 20.43gs
    ANGLE_SKIP_TEST_IF(maxGeometryShaderStorageBlocks == 0);

    GLint maxFragmentShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);

    // The minimun value of MAX_FRAGMENT_SHADER_STORAGE_BLOCKS can be 0.
    // [OpenGL ES 3.1] Table 20.44
    ANGLE_SKIP_TEST_IF(maxFragmentShaderStorageBlocks == 0);

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
buffer ssbo
{
    vec4 ssbo_value;
} block0;
layout (triangles) in;
layout (points, max_vertices = 1) out;
void main()
{
    gl_Position = gl_in[0].gl_Position + block0.ssbo_value;
    EmitVertex();
})";

    constexpr char kFS[] = R"(#version 310 es
precision highp float;
buffer ssbo
{
    vec3 ssbo_value;
} block0;
layout (location = 0) out vec4 output_color;
void main()
{
    output_color = vec4(block0.ssbo_value, 1);
})";

    GLuint program = CompileProgramWithGS(essl31_shaders::vs::Simple(), kGS, kFS);
    EXPECT_EQ(0u, program);

    EXPECT_GL_NO_ERROR();
}

// Verify GL_REFERENCED_BY_GEOMETRY_SHADER_EXT cannot be used on platforms that don't support
// EXT_geometry_shader, or we will get an INVALID_ENUM error.
TEST_P(GeometryShaderTest, ReferencedByGeometryShaderWithoutExtensionEnabled)
{
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr char kFS[] = R"(#version 310 es
precision highp float;
uniform vec4 color;
layout(location = 0) out vec4 oColor;
void main()
{
    oColor = color;
})";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);

    const GLuint index = glGetProgramResourceIndex(program, GL_UNIFORM, "color");
    ASSERT_GL_NO_ERROR();
    ASSERT_NE(GL_INVALID_INDEX, index);

    constexpr GLenum kProps[]    = {GL_REFERENCED_BY_GEOMETRY_SHADER_EXT};
    constexpr GLsizei kPropCount = static_cast<GLsizei>(ArraySize(kProps));
    GLint params[ArraySize(kProps)];
    GLsizei length;

    glGetProgramResourceiv(program, GL_UNIFORM, index, kPropCount, kProps, kPropCount, &length,
                           params);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Verify GL_REFERENCED_BY_GEOMETRY_SHADER_EXT can work correctly on platforms that support
// EXT_geometry_shader.
TEST_P(GeometryShaderTest, ReferencedByGeometryShader)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr char kVS[] = R"(#version 310 es
precision highp float;
layout(location = 0) in highp vec4 position;
void main()
{
    gl_Position = position;
})";

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout (binding = 3) uniform ubo0
{
    vec4 ubo0_location;
} block0;
layout (binding = 4) uniform ubo1
{
    vec4 ubo1_location;
} block1;
uniform vec4 u_color;
layout (triangles) in;
layout (points, max_vertices = 1) out;
out vec4 gs_out;
void main()
{
    gl_Position = gl_in[0].gl_Position;
    gl_Position += block0.ubo0_location + block1.ubo1_location;
    gs_out = u_color;
    EmitVertex();
})";

    constexpr char kFS[] = R"(#version 310 es
precision highp float;
in vec4 gs_out;
layout(location = 0) out vec4 oColor;
void main()
{
    oColor = gs_out;
})";

    ANGLE_GL_PROGRAM_WITH_GS(program, kVS, kGS, kFS);

    constexpr GLenum kProps[]    = {GL_REFERENCED_BY_GEOMETRY_SHADER_EXT};
    constexpr GLsizei kPropCount = static_cast<GLsizei>(ArraySize(kProps));
    std::array<GLint, ArraySize(kProps)> params;
    GLsizei length;

    params.fill(1);
    GLuint index = glGetProgramResourceIndex(program, GL_PROGRAM_INPUT, "position");
    glGetProgramResourceiv(program, GL_PROGRAM_INPUT, index, kPropCount, kProps, kPropCount,
                           &length, params.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0, params[0]);

    params.fill(1);
    index = glGetProgramResourceIndex(program, GL_PROGRAM_OUTPUT, "oColor");
    glGetProgramResourceiv(program, GL_PROGRAM_OUTPUT, index, kPropCount, kProps, kPropCount,
                           &length, params.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0, params[0]);

    index = glGetProgramResourceIndex(program, GL_UNIFORM, "u_color");
    glGetProgramResourceiv(program, GL_UNIFORM, index, kPropCount, kProps, kPropCount, &length,
                           params.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1, params[0]);

    params.fill(0);
    index = glGetProgramResourceIndex(program, GL_UNIFORM_BLOCK, "ubo1");
    glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, index, kPropCount, kProps, kPropCount,
                           &length, params.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1, params[0]);

    GLint maxGeometryShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT, &maxGeometryShaderStorageBlocks);
    // The maximum number of shader storage blocks in a geometry shader can be 0.
    // [EXT_geometry_shader] Table 20.43gs
    if (maxGeometryShaderStorageBlocks > 0)
    {
        constexpr char kGSWithSSBO[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout (binding = 2) buffer ssbo
{
    vec4 ssbo_value;
} block0;
layout (triangles) in;
layout (points, max_vertices = 1) out;
out vec4 gs_out;
void main()
{
    gl_Position = gl_in[0].gl_Position + block0.ssbo_value;
    gs_out = block0.ssbo_value;
    EmitVertex();
})";

        ANGLE_GL_PROGRAM_WITH_GS(programWithSSBO, kVS, kGSWithSSBO, kFS);

        params.fill(0);
        index = glGetProgramResourceIndex(programWithSSBO, GL_SHADER_STORAGE_BLOCK, "ssbo");
        glGetProgramResourceiv(programWithSSBO, GL_SHADER_STORAGE_BLOCK, index, kPropCount, kProps,
                               kPropCount, &length, params.data());
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(1, params[0]);

        params.fill(0);
        index = glGetProgramResourceIndex(programWithSSBO, GL_BUFFER_VARIABLE, "ssbo.ssbo_value");
        glGetProgramResourceiv(programWithSSBO, GL_BUFFER_VARIABLE, index, kPropCount, kProps,
                               kPropCount, &length, params.data());
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(1, params[0]);
    }

    GLint maxGeometryAtomicCounterBuffers = 0;
    glGetIntegerv(GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_EXT, &maxGeometryAtomicCounterBuffers);
    // The maximum number of atomic counter buffers in a geometry shader can be 0.
    // [EXT_geometry_shader] Table 20.43gs
    if (maxGeometryAtomicCounterBuffers > 0)
    {
        constexpr char kGSWithAtomicCounters[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout(binding = 1, offset = 0) uniform atomic_uint gs_counter;
layout (triangles) in;
layout (points, max_vertices = 1) out;
out vec4 gs_out;
void main()
{
    atomicCounterIncrement(gs_counter);
    gl_Position = gl_in[0].gl_Position;
    gs_out = vec4(1.0, 0.0, 0.0, 1.0);
    EmitVertex();
})";

        ANGLE_GL_PROGRAM_WITH_GS(programWithAtomicCounter, kVS, kGSWithAtomicCounters, kFS);

        params.fill(0);
        index = glGetProgramResourceIndex(programWithAtomicCounter, GL_UNIFORM, "gs_counter");
        EXPECT_GL_NO_ERROR();
        glGetProgramResourceiv(programWithAtomicCounter, GL_ATOMIC_COUNTER_BUFFER, index,
                               kPropCount, kProps, kPropCount, &length, params.data());
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(1, params[0]);
    }
}

void GeometryShaderTest::callFramebufferTextureAPI(APIExtensionVersion usedExtension,
                                                   GLenum target,
                                                   GLenum attachment,
                                                   GLuint texture,
                                                   GLint level)
{
    ASSERT(usedExtension == APIExtensionVersion::EXT || usedExtension == APIExtensionVersion::OES ||
           usedExtension == APIExtensionVersion::Core);
    if (usedExtension == APIExtensionVersion::EXT)
    {
        glFramebufferTextureEXT(target, attachment, texture, level);
    }
    else if (usedExtension == APIExtensionVersion::OES)
    {
        glFramebufferTextureOES(target, attachment, texture, level);
    }
    else
    {
        glFramebufferTexture(target, attachment, texture, level);
    }
}

void GeometryShaderTest::testNegativeFramebufferTexture(APIExtensionVersion usedExtension)
{
    ASSERT(usedExtension == APIExtensionVersion::EXT || usedExtension == APIExtensionVersion::OES ||
           usedExtension == APIExtensionVersion::Core);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 32, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // [EXT_geometry_shader] Section 9.2.8, "Attaching Texture Images to a Framebuffer"
    // An INVALID_ENUM error is generated if <target> is not DRAW_FRAMEBUFFER, READ_FRAMEBUFFER, or
    // FRAMEBUFFER.
    callFramebufferTextureAPI(usedExtension, GL_TEXTURE_2D, GL_COLOR_ATTACHMENT0, tex, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // An INVALID_ENUM error is generated if <attachment> is not one of the attachments in Table
    // 9.1.
    callFramebufferTextureAPI(usedExtension, GL_FRAMEBUFFER, GL_TEXTURE_2D, tex, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // An INVALID_OPERATION error is generated if zero is bound to <target>.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    callFramebufferTextureAPI(usedExtension, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // An INVALID_VALUE error is generated if <texture> is not the name of a texture object, or if
    // <level> is not a supported texture level for <texture>.
    GLuint tex2;
    glGenTextures(1, &tex2);
    glDeleteTextures(1, &tex2);
    ASSERT_FALSE(glIsTexture(tex2));
    callFramebufferTextureAPI(usedExtension, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    GLint max3DSize;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max3DSize);
    GLint max3DLevel = static_cast<GLint>(std::log2(max3DSize));
    callFramebufferTextureAPI(usedExtension, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex,
                              max3DLevel + 1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Verify that correct errors are reported when we use illegal parameters in FramebufferTextureEXT.
TEST_P(GeometryShaderTest, NegativeFramebufferTextureEXT)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    testNegativeFramebufferTexture(APIExtensionVersion::EXT);
}

// Verify that correct errors are reported when we use illegal parameters in FramebufferTextureOES.
TEST_P(GeometryShaderTest, NegativeFramebufferTextureOES)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_geometry_shader"));
    testNegativeFramebufferTexture(APIExtensionVersion::OES);
}

// Verify CheckFramebufferStatus can work correctly on layered depth and stencil attachments.
TEST_P(GeometryShaderTest, LayeredFramebufferCompletenessWithDepthAttachment)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    GLint maxFramebufferLayers;
    glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS_EXT, &maxFramebufferLayers);

    constexpr GLint kTexLayers = 2;
    ASSERT_LT(kTexLayers, maxFramebufferLayers);

    GLTexture layeredColorTex;
    glBindTexture(GL_TEXTURE_3D, layeredColorTex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 32, 32, kTexLayers, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    // [EXT_geometry_shader] section 9.4.1, "Framebuffer Completeness"
    // If any framebuffer attachment is layered, all populated attachments must be layered.
    // {FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT }
    GLTexture layeredDepthStencilTex;
    glBindTexture(GL_TEXTURE_2D_ARRAY, layeredDepthStencilTex);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH24_STENCIL8, 32, 32, kTexLayers, 0,
                 GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

    // 1. Color attachment is layered, while depth attachment is not layered.
    GLFramebuffer fbo1;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, layeredColorTex, 0);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, layeredDepthStencilTex, 0, 0);
    GLenum status1 = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT, status1);

    // 2. Color attachment is not layered, while depth attachment is layered.
    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, layeredColorTex, 0, 0);
    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, layeredDepthStencilTex, 0);
    GLenum status2 = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT, status2);

    // 3. Color attachment is not layered, while stencil attachment is layered.
    GLFramebuffer fbo3;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo3);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, layeredColorTex, 0, 0);
    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, layeredDepthStencilTex, 0);
    GLenum status3 = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT, status3);

    // [EXT_geometry_shader] section 9.4.1, "Framebuffer Completeness"
    // If <image> is a three-dimensional texture or a two-dimensional array texture and the
    // attachment is layered, the depth or layer count, respectively, of the texture is less than or
    // equal to the value of MAX_FRAMEBUFFER_LAYERS_EXT.
    GLint maxArrayTextureLayers;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxArrayTextureLayers);
    GLint depthTexLayer4 = maxFramebufferLayers + 1;
    ANGLE_SKIP_TEST_IF(maxArrayTextureLayers < depthTexLayer4);

    // Use a depth attachment whose layer count exceeds MAX_FRAMEBUFFER_LAYERS
    GLTexture depthTex4;
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthTex4);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH24_STENCIL8, 32, 32, depthTexLayer4, 0,
                 GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    GLFramebuffer fbo4;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo4);
    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex4, 0);
    GLenum status4 = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, status4);
}

// Verify correct errors can be reported when we use layered cube map attachments on a framebuffer.
TEST_P(GeometryShaderTest, NegativeLayeredFramebufferCompletenessWithCubeMapTextures)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    GLTexture tex;
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, status);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0);
    ASSERT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, status);
}

// Verify that we can use default interpolation in the GS.
TEST_P(GeometryShaderTest, FlatQualifierNotRequired)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout(points) in;
layout(points, max_vertices=1) out;

in highp int target[];
highp uniform vec4 dummyZero; // Default value is vec4(0.0).

void main()
{
    highp vec4 retValue = dummyZero;
    retValue += vec4(float(target[0]));
    retValue += gl_in[0].gl_Position;
    gl_Position = retValue;
    EmitVertex();
})";

    GLuint geometryShader = CompileShader(GL_GEOMETRY_SHADER_EXT, kGS);

    EXPECT_NE(0u, geometryShader);

    GLuint programID = glCreateProgram();
    glAttachShader(programID, geometryShader);

    glDetachShader(programID, geometryShader);
    glDeleteShader(geometryShader);
    glDeleteProgram(programID);

    EXPECT_GL_NO_ERROR();
}

void GeometryShaderTest::setupLayeredFramebuffer(GLuint framebuffer,
                                                 GLuint color0,
                                                 GLuint color1,
                                                 GLuint depthStencil,
                                                 GLenum colorTarget,
                                                 const GLColor &color0InitialColor,
                                                 const GLColor &color1InitialColor,
                                                 float depthInitialValue,
                                                 GLint stencilInitialValue)
{
    const uint32_t depthInitialValueUnorm   = static_cast<uint32_t>(depthInitialValue * 0xFFFFFF);
    const uint32_t depthStencilInitialValue = depthInitialValueUnorm << 8 | stencilInitialValue;

    const std::vector<GLColor> kColor0InitData(kWidth * kHeight * kColor0Layers,
                                               color0InitialColor);
    const std::vector<GLColor> kColor1InitData(kWidth * kHeight * kColor1Layers,
                                               color1InitialColor);
    const std::vector<uint32_t> kDepthStencilInitData(kWidth * kHeight * kDepthStencilLayers,
                                                      depthStencilInitialValue);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    ASSERT_GL_NO_ERROR();

    glBindTexture(colorTarget, color0);
    glTexStorage3D(colorTarget, 1, GL_RGBA8, kWidth, kHeight, kColor0Layers);
    glTexSubImage3D(colorTarget, 0, 0, 0, 0, kWidth, kHeight, kColor0Layers, GL_RGBA,
                    GL_UNSIGNED_BYTE, kColor0InitData.data());
    ASSERT_GL_NO_ERROR();

    glBindTexture(colorTarget, color1);
    glTexStorage3D(colorTarget, 1, GL_RGBA8, kWidth, kHeight, kColor1Layers);
    glTexSubImage3D(colorTarget, 0, 0, 0, 0, kWidth, kHeight, kColor1Layers, GL_RGBA,
                    GL_UNSIGNED_BYTE, kColor1InitData.data());
    ASSERT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D_ARRAY, depthStencil);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH24_STENCIL8, kWidth, kHeight,
                   kDepthStencilLayers);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, kWidth, kHeight, kDepthStencilLayers,
                    GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, kDepthStencilInitData.data());
    ASSERT_GL_NO_ERROR();

    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color0, 0);
    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, color1, 0);
    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthStencil, 0);

    constexpr GLenum kDrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, kDrawBuffers);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
}

void GeometryShaderTest::setupLayeredFramebufferProgram(GLProgram *program)
{
    constexpr char kVS[] = R"(#version 310 es

in highp vec4 position;

void main()
{
    gl_Position = position;
})";

    static_assert(kFramebufferLayers == 3,
                  "Adjust the invocations parameter in the geometry shader, and color arrays in "
                  "fragment shader");

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout (invocations = 3, triangles) in;
layout (triangle_strip, max_vertices = 3) out;

void main()
{
    for (int n = 0; n < gl_in.length(); n++)
    {
        gl_Position = gl_in[n].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
precision mediump float;

layout(location = 0) out mediump vec4 color0;
layout(location = 1) out mediump vec4 color1;

const vec4 color0Layers[3] = vec4[](
    vec4(1, 0, 0, 1),
    vec4(0, 1, 0, 1),
    vec4(0, 0, 1, 1)
);

const vec4 color1Layers[3] = vec4[](
    vec4(1, 1, 0, 1),
    vec4(0, 1, 1, 1),
    vec4(1, 0, 1, 1)
);

void main()
{
    color0 = color0Layers[gl_Layer];
    color1 = color1Layers[gl_Layer];
})";

    program->makeRaster(kVS, kGS, kFS);
    ASSERT_TRUE(program->valid());
}

void GeometryShaderTest::verifyLayeredFramebufferColor(GLuint colorTexture,
                                                       GLenum colorTarget,
                                                       const GLColor expected[],
                                                       GLsizei layerCount)
{
    // Note: the OpenGL and Vulkan specs are unclear regarding whether clear should affect all
    // layers of the framebuffer or the attachment.  The GL spec says:
    //
    // > When the Clear or ClearBuffer* commands described in section 15.2.3 are
    // > used to clear a layered framebuffer attachment, all layers of the attachment are
    // > cleared.
    //
    // Which implies that all layers are cleared.  However, it's common among vendors to consider
    // only the attachments accessible to a draw call to be affected by clear (otherwise
    // clear-through-draw cannot be done).
    //
    // There is inconsistency between implementations in both the OpenGL and Vulkan implementations
    // in this regard.  In OpenGL, Qualcomm and Intel drivers clear all layers while Nvidia drivers
    // clear only the framebuffer layers.  In Vulkan, Intel and AMD windows drivers clear all layers
    // with loadOp=CLEAR, while the other implementations (including Intel mesa) only clear the
    // framebuffer layers.
    //
    // Due to this inconsistency, only the framebuffer layers are verified.  The other layers, if
    // the texture has them will either contain the initial or the cleared color, but is not
    // verified by these tests.
    layerCount = kFramebufferLayers;

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindTexture(colorTarget, colorTexture);

    for (GLsizei layer = 0; layer < layerCount; ++layer)
    {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorTexture, 0, layer);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        EXPECT_PIXEL_COLOR_NEAR(0, 0, expected[layer], 1);
        EXPECT_PIXEL_COLOR_NEAR(kWidth - 1, 0, expected[layer], 1);
        EXPECT_PIXEL_COLOR_NEAR(0, kHeight - 1, expected[layer], 1);
        EXPECT_PIXEL_COLOR_NEAR(kWidth - 1, kHeight - 1, expected[layer], 1);
    }
}

void GeometryShaderTest::verifyLayeredFramebufferDepthStencil(GLuint depthStencilTexture,
                                                              const float expectedDepth[],
                                                              const GLint expectedStencil[],
                                                              GLsizei layerCount)
{
    // See comment in verifyLayeredFramebufferColor
    layerCount = kFramebufferLayers;

    // Setup program
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Set up state to validate depth and stencil
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);
    glClearColor(0, 0, 0, 0);

    // Set up framebuffer
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

    glBindTexture(GL_TEXTURE_2D_ARRAY, depthStencilTexture);

    for (GLsizei layer = 0; layer < layerCount; ++layer)
    {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthStencilTexture,
                                  0, layer);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glClear(GL_COLOR_BUFFER_BIT);

        glStencilFunc(GL_EQUAL, expectedStencil[layer], 0xFF);

        // Pass depth slightly less than expected
        glDepthFunc(GL_LESS);
        glUniform4f(colorUniformLocation, 0.1f, 0.2f, 0.3f, 0.4f);
        drawQuad(drawColor, essl1_shaders::PositionAttrib(), expectedDepth[layer] * 2 - 1 - 0.01f);

        // Fail depth slightly greater than expected
        glUniform4f(colorUniformLocation, 0.5f, 0.6f, 0.7f, 0.8f);
        drawQuad(drawColor, essl1_shaders::PositionAttrib(), expectedDepth[layer] * 2 - 1 + 0.01f);

        ASSERT_GL_NO_ERROR();

        // Verify results
        const GLColor kExpected(25, 51, 76, 102);

        EXPECT_PIXEL_COLOR_NEAR(0, 0, kExpected, 1);
        EXPECT_PIXEL_COLOR_NEAR(kWidth - 1, 0, kExpected, 1);
        EXPECT_PIXEL_COLOR_NEAR(0, kHeight - 1, kExpected, 1);
        EXPECT_PIXEL_COLOR_NEAR(kWidth - 1, kHeight - 1, kExpected, 1);
    }
}

void GeometryShaderTest::layeredFramebufferClearTest(GLenum colorTarget)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    const GLColor kColor0InitColor(10, 20, 30, 40);
    const GLColor kColor1InitColor(200, 210, 220, 230);
    constexpr float kDepthInitValue   = 0.35f;
    constexpr GLint kStencilInitValue = 0x33;

    GLFramebuffer framebuffer;
    GLTexture color0;
    GLTexture color1;
    GLTexture depthStencil;
    GLProgram program;

    setupLayeredFramebuffer(framebuffer, color0, color1, depthStencil, colorTarget,
                            kColor0InitColor, kColor1InitColor, kDepthInitValue, kStencilInitValue);
    setupLayeredFramebufferProgram(&program);

    glClearColor(0.5, 0.5, 0.5, 0.5);
    glClearDepthf(1.0f);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    const GLColor kClearColor(127, 127, 127, 127);
    const GLColor kExpectedColor0[kColor0Layers] = {
        kClearColor,
        kClearColor,
        kClearColor,
        kColor0InitColor,
    };
    const GLColor kExpectedColor1[kColor1Layers] = {
        kClearColor,
        kClearColor,
        kClearColor,
    };
    const float kExpectedDepth[kDepthStencilLayers] = {
        1.0f, 1.0f, 1.0f, kDepthInitValue, kDepthInitValue,
    };
    const GLint kExpectedStencil[kDepthStencilLayers] = {
        0x55, 0x55, 0x55, kStencilInitValue, kStencilInitValue,
    };

    verifyLayeredFramebufferColor(color0, colorTarget, kExpectedColor0, kColor0Layers);
    verifyLayeredFramebufferColor(color1, colorTarget, kExpectedColor1, kColor1Layers);
    verifyLayeredFramebufferDepthStencil(depthStencil, kExpectedDepth, kExpectedStencil,
                                         kDepthStencilLayers);
}

// Verify clear of layered attachments.  Uses 3D color textures.
TEST_P(GeometryShaderTest, LayeredFramebufferClear3DColor)
{
    // Mesa considers the framebuffer with mixed 3D and 2D array attachments to be incomplete.
    // http://anglebug.com/42264003
    ANGLE_SKIP_TEST_IF((IsAMD() || IsIntel()) && IsOpenGL() && IsLinux());

    layeredFramebufferClearTest(GL_TEXTURE_3D);
}

// Verify clear of layered attachments.  Uses 2D array color textures.
TEST_P(GeometryShaderTest, LayeredFramebufferClear2DArrayColor)
{
    layeredFramebufferClearTest(GL_TEXTURE_2D_ARRAY);
}

void GeometryShaderTest::layeredFramebufferPreRenderClearTest(GLenum colorTarget, bool doubleClear)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    const GLColor kColor0InitColor(10, 20, 30, 40);
    const GLColor kColor1InitColor(200, 210, 220, 230);
    constexpr float kDepthInitValue   = 0.35f;
    constexpr GLint kStencilInitValue = 0x33;

    GLFramebuffer framebuffer;
    GLTexture color0;
    GLTexture color1;
    GLTexture depthStencil;
    GLProgram program;

    setupLayeredFramebuffer(framebuffer, color0, color1, depthStencil, colorTarget,
                            kColor0InitColor, kColor1InitColor, kDepthInitValue, kStencilInitValue);
    setupLayeredFramebufferProgram(&program);

    if (doubleClear)
    {
        glClearColor(0.1, 0.2, 0.8, 0.9);
        glClearDepthf(0.2f);
        glClearStencil(0x99);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    glClearColor(0.5, 0.5, 0.5, 0.5);
    glClearDepthf(1.0f);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x5A, 0xF0);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    drawQuad(program, "position", 0.9f);

    const GLColor kExpectedColor0[kColor0Layers] = {
        GLColor::red,
        GLColor::green,
        GLColor::blue,
        kColor0InitColor,
    };
    const GLColor kExpectedColor1[kColor1Layers] = {
        GLColor::yellow,
        GLColor::cyan,
        GLColor::magenta,
    };
    const float kExpectedDepth[kDepthStencilLayers] = {
        0.95f, 0.95f, 0.95f, kDepthInitValue, kDepthInitValue,
    };
    const GLint kExpectedStencil[kDepthStencilLayers] = {
        0x5A, 0x5A, 0x5A, kStencilInitValue, kStencilInitValue,
    };

    verifyLayeredFramebufferColor(color0, colorTarget, kExpectedColor0, kColor0Layers);
    verifyLayeredFramebufferColor(color1, colorTarget, kExpectedColor1, kColor1Layers);
    verifyLayeredFramebufferDepthStencil(depthStencil, kExpectedDepth, kExpectedStencil,
                                         kDepthStencilLayers);
}

// Verify pre-render clear of layered attachments.  Uses 3D color textures.
TEST_P(GeometryShaderTest, LayeredFramebufferPreRenderClear3DColor)
{
    // Mesa considers the framebuffer with mixed 3D and 2D array attachments to be incomplete.
    // http://anglebug.com/42264003
    ANGLE_SKIP_TEST_IF((IsAMD() || IsIntel()) && IsOpenGL() && IsLinux());

    layeredFramebufferPreRenderClearTest(GL_TEXTURE_3D, false);
}

// Same as LayeredFramebufferPreRenderClear3DColor, but clears twice.
TEST_P(GeometryShaderTest, LayeredFramebufferPreRenderDoubleClear3DColor)
{
    // Mesa considers the framebuffer with mixed 3D and 2D array attachments to be incomplete.
    // http://anglebug.com/42264003
    ANGLE_SKIP_TEST_IF((IsAMD() || IsIntel()) && IsOpenGL() && IsLinux());

    layeredFramebufferPreRenderClearTest(GL_TEXTURE_3D, true);
}

// Verify pre-render clear of layered attachments.  Uses 2D array color textures.
TEST_P(GeometryShaderTest, LayeredFramebufferPreRenderClear2DArrayColor)
{
    layeredFramebufferPreRenderClearTest(GL_TEXTURE_2D_ARRAY, false);
}

// Same as LayeredFramebufferPreRenderClear2DArrayColor, but clears twice.
TEST_P(GeometryShaderTest, LayeredFramebufferPreRenderDoubleClear2DArrayColor)
{
    layeredFramebufferPreRenderClearTest(GL_TEXTURE_2D_ARRAY, true);
}

void GeometryShaderTest::layeredFramebufferMidRenderClearTest(GLenum colorTarget)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    // Vulkan's draw path for clear doesn't support layered framebuffers.
    // http://anglebug.com/42263992
    ANGLE_SKIP_TEST_IF(IsVulkan());

    const GLColor kColor0InitColor(10, 20, 30, 40);
    const GLColor kColor1InitColor(200, 210, 220, 230);
    constexpr float kDepthInitValue   = 0.35f;
    constexpr GLint kStencilInitValue = 0x33;

    GLFramebuffer framebuffer;
    GLTexture color0;
    GLTexture color1;
    GLTexture depthStencil;
    GLProgram program;

    setupLayeredFramebuffer(framebuffer, color0, color1, depthStencil, colorTarget,
                            kColor0InitColor, kColor1InitColor, kDepthInitValue, kStencilInitValue);
    setupLayeredFramebufferProgram(&program);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x55, 0xF0);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    drawQuad(program, "position", 0.3f);

    glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_TRUE);
    glClearColor(0.5, 0.5, 0.5, 0.5);
    glClearDepthf(0.8f);
    glStencilMask(0xF0);
    glClearStencil(0xAA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0xFF);

    const GLColor kExpectedColor0[kColor0Layers] = {
        GLColor(255, 0, 127, 127),
        GLColor(0, 255, 127, 127),
        GLColor(0, 0, 127, 127),
        kColor0InitColor,
    };
    const GLColor kExpectedColor1[kColor1Layers] = {
        GLColor(255, 255, 127, 127),
        GLColor(0, 255, 127, 127),
        GLColor(255, 0, 127, 127),
    };
    const float kExpectedDepth[kDepthStencilLayers] = {
        0.6f, 0.6f, 0.6f, kDepthInitValue, kDepthInitValue,
    };
    const GLint kExpectedStencil[kDepthStencilLayers] = {
        0xA5, 0xA5, 0xA5, kStencilInitValue, kStencilInitValue,
    };

    verifyLayeredFramebufferColor(color0, colorTarget, kExpectedColor0, kColor0Layers);
    verifyLayeredFramebufferColor(color1, colorTarget, kExpectedColor1, kColor1Layers);
    verifyLayeredFramebufferDepthStencil(depthStencil, kExpectedDepth, kExpectedStencil,
                                         kDepthStencilLayers);
}

// Verify that Geometry Shader's gl_Layer is ineffective when the framebuffer is not layered.
TEST_P(GeometryShaderTest, GLLayerIneffectiveWithoutLayeredFramebuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr char kVS[] = R"(#version 310 es
in highp vec4 position;
void main()
{
    gl_Position = position;
})";

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout (invocations = 3, triangles) in;
layout (triangle_strip, max_vertices = 3) out;

void main()
{
    for (int n = 0; n < gl_in.length(); n++)
    {
        gl_Position = gl_in[n].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
precision mediump float;

layout(location = 0) out mediump vec4 color;

void main()
{
    if (gl_Layer == 0)
        color = vec4(0, 1, 0, 1);
    else
        color = vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM_WITH_GS(program, kVS, kGS, kFS);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    GLTexture color;

    glBindTexture(GL_TEXTURE_2D_ARRAY, color);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, kWidth, kHeight, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color, 0, 1);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, "position", 0.3f);

    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Verify mid-render clear of layered attachments.  Uses 3D color textures.
TEST_P(GeometryShaderTest, LayeredFramebufferMidRenderClear3DColor)
{
    // Mesa considers the framebuffer with mixed 3D and 2D array attachments to be incomplete.
    // http://anglebug.com/42264003
    ANGLE_SKIP_TEST_IF((IsAMD() || IsIntel()) && IsOpenGL() && IsLinux());

    layeredFramebufferMidRenderClearTest(GL_TEXTURE_3D);
}

// Verify mid-render clear of layered attachments.  Uses 2D array color textures.
TEST_P(GeometryShaderTest, LayeredFramebufferMidRenderClear2DArrayColor)
{
    layeredFramebufferMidRenderClearTest(GL_TEXTURE_2D_ARRAY);
}

// Verify that prerotation applies to the geometry shader stage if present.
TEST_P(GeometryShaderTest, Prerotation)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // Geometry shader will output fixed positions, so this is ignored.
    gl_Position = vec4(0);
})";

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

void main()
{
    // Generate two triangles to cover the lower-left quarter of the screen.
    gl_Position = vec4(0, -1, 0, 1);
    EmitVertex();

    gl_Position = vec4(0, 0, 0, 1);
    EmitVertex();

    gl_Position = vec4(-1, -1, 0, 1);
    EmitVertex();

    gl_Position = vec4(-1, 0, 0, 1);
    EmitVertex();

    EndPrimitive();
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;

layout(location = 0) out mediump vec4 color;

void main()
{
    // Output solid green
    color = vec4(0, 1.0, 0, 1.0);
})";

    ANGLE_GL_PROGRAM_WITH_GS(program, kVS, kGS, kFS);
    EXPECT_GL_NO_ERROR();

    glClearColor(1.0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    EXPECT_PIXEL_RECT_EQ(0, 0, w / 2, h / 2, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(0, h / 2, w, h / 2, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(w / 2, 0, w / 2, h / 2, GLColor::red);
}

// Verify that correct errors are reported when we use illegal parameters in FramebufferTexture.
TEST_P(GeometryShaderTestES32, NegativeFramebufferTexture)
{
    testNegativeFramebufferTexture(APIExtensionVersion::Core);
}

// Verify that we can have the max amount of uniforms with a geometry shader.
TEST_P(GeometryShaderTestES32, MaxGeometryImageUniforms)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    GLint maxGeometryImageUnits;
    glGetIntegerv(GL_MAX_GEOMETRY_IMAGE_UNIFORMS_EXT, &maxGeometryImageUnits);

    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(1.0);
})";

    std::stringstream geomStringStream;

    geomStringStream << R"(#version 310 es
#extension GL_OES_geometry_shader : require
layout (points)                   in;
layout (points, max_vertices = 1) out;

precision highp iimage2D;

ivec4 counter = ivec4(0);
)";

    for (GLint index = 0; index < maxGeometryImageUnits; ++index)
    {
        geomStringStream << "layout(binding = " << index << ", r32i) uniform iimage2D img" << index
                         << ";" << std::endl;
    }

    geomStringStream << R"(
void main()
{
)";

    for (GLint index = 0; index < maxGeometryImageUnits; ++index)
    {
        geomStringStream << "counter += imageLoad(img" << index << ", ivec2(0, 0));" << std::endl;
    }

    geomStringStream << R"(
    gl_Position = vec4(float(counter.x), 0.0, 0.0, 1.0);
    EmitVertex();
}
)";

    ANGLE_GL_PROGRAM_WITH_GS(program, vertString, geomStringStream.str().c_str(), fragString);
    EXPECT_GL_NO_ERROR();

    glClearColor(1.0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);

    std::vector<GLTexture> textures(maxGeometryImageUnits);
    for (GLint index = 0; index < maxGeometryImageUnits; ++index)
    {
        GLint value = index + 1;

        glBindTexture(GL_TEXTURE_2D, textures[index]);

        glTexStorage2D(GL_TEXTURE_2D, 1 /*levels*/, GL_R32I, 1 /*width*/, 1 /*height*/);

        glTexSubImage2D(GL_TEXTURE_2D, 0 /*level*/, 0 /*xoffset*/, 0 /*yoffset*/, 1 /*width*/,
                        1 /*height*/, GL_RED_INTEGER, GL_INT, &value);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindImageTexture(index, textures[index], 0 /*level*/, GL_FALSE /*is layered?*/,
                           0 /*layer*/, GL_READ_ONLY, GL_R32I);
    }

    glDrawArrays(GL_POINTS, 0, 3);
    EXPECT_GL_NO_ERROR();
}

// Verify that depth viewport transform applies to the geometry shader stage if present.
TEST_P(GeometryShaderTest, DepthViewportTransform)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // Geometry shader will output fixed positions, so this is ignored.
    gl_Position = vec4(0);
})";

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

void main()
{
    // Generate two triangles to cover the whole screen, with depth at -0.5.  After viewport
    // transformation, the depth buffer should contain 0.25.
    gl_Position = vec4(1, -1, -0.5, 1);
    EmitVertex();

    gl_Position = vec4(1, 1, -0.5, 1);
    EmitVertex();

    gl_Position = vec4(-1, -1, -0.5, 1);
    EmitVertex();

    gl_Position = vec4(-1, 1, -0.5, 1);
    EmitVertex();

    EndPrimitive();
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;

layout(location = 0) out mediump vec4 color;

void main()
{
    // Output solid green
    color = vec4(0, 1.0, 0, 1.0);
})";

    ANGLE_GL_PROGRAM_WITH_GS(program, kVS, kGS, kFS);
    EXPECT_GL_NO_ERROR();

    glClearColor(1.0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    EXPECT_PIXEL_RECT_EQ(0, 0, w, h, GLColor::green);

    // Verify depth
    glDepthFunc(GL_LESS);

    // An epsilon below the depth value should pass the depth test
    ANGLE_GL_PROGRAM(drawBlue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(drawBlue, essl1_shaders::PositionAttrib(), -0.5f - 0.01f);

    // An epsilon above the depth value should fail the depth test
    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), -0.5f + 0.01f);

    EXPECT_PIXEL_RECT_EQ(0, 0, w, h, GLColor::blue);
}

// Tests separating the VS from the GS/FS and then modifying the shader.
TEST_P(GeometryShaderTest, RecompileSeparableVSWithVaryings)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    // Errors in D3D11/GL. No plans to fix this.
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    const char *kVS = R"(#version 310 es
precision mediump float;
in vec4 position;
out vec4 vgVarying;
uniform vec4 uniVec;
void main()
{
   vgVarying = uniVec;
   gl_Position = position;
})";

    const char *kGS = R"(#version 310 es
#extension GL_EXT_geometry_shader : require

precision mediump float;

layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 vgVarying[];
layout(location = 5) out vec4 gfVarying;

void main()
{
    for (int n = 0; n < gl_in.length(); n++)
    {
        gl_Position = gl_in[n].gl_Position;
        gfVarying = vgVarying[n];
        EmitVertex();
    }
    EndPrimitive();
})";

    const char *kFS = R"(#version 310 es
precision mediump float;

layout(location = 5) in vec4 gfVarying;
out vec4 fOut;

void main()
{
    fOut = gfVarying;
})";

    GLShader vertShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &kVS, nullptr);
    glCompileShader(vertShader);

    GLProgram vertProg;
    glProgramParameteri(vertProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(vertProg, vertShader);
    glLinkProgram(vertProg);
    ASSERT_GL_NO_ERROR();

    GLShader geomShader(GL_GEOMETRY_SHADER);
    glShaderSource(geomShader, 1, &kGS, nullptr);
    glCompileShader(geomShader);

    GLShader fragShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &kFS, nullptr);
    glCompileShader(fragShader);

    GLProgram geomFragProg;
    glProgramParameteri(geomFragProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(geomFragProg, geomShader);
    glAttachShader(geomFragProg, fragShader);
    glLinkProgram(geomFragProg);
    ASSERT_GL_NO_ERROR();

    GLProgramPipeline pipeline;
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertProg);
    glUseProgramStages(pipeline, GL_GEOMETRY_SHADER_BIT, geomFragProg);
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, geomFragProg);
    glBindProgramPipeline(pipeline);

    glActiveShaderProgram(pipeline, vertProg);
    GLint uniLoc = glGetUniformLocation(vertProg, "uniVec");
    ASSERT_NE(-1, uniLoc);
    glUniform4f(uniLoc, 0, 1, 0, 1);
    ASSERT_GL_NO_ERROR();

    drawQuadPPO(vertProg, "position", 0.5f, 1.0f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Do it again with deleted shaders.
    vertProg.reset();
    geomFragProg.reset();
    pipeline.reset();

    glProgramParameteri(vertProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(vertProg, vertShader);
    glLinkProgram(vertProg);

    // Mess up the VS.
    const char *otherVS = essl1_shaders::vs::Texture2D();
    glShaderSource(vertShader, 1, &otherVS, nullptr);
    glCompileShader(vertShader);

    ASSERT_GL_NO_ERROR();

    glProgramParameteri(geomFragProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(geomFragProg, geomShader);
    glAttachShader(geomFragProg, fragShader);
    glLinkProgram(geomFragProg);

    // Mess up the FS.
    const char *otherFS = essl1_shaders::fs::Texture2D();
    glShaderSource(fragShader, 1, &otherFS, nullptr);
    glCompileShader(fragShader);

    ASSERT_GL_NO_ERROR();

    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertProg);
    glUseProgramStages(pipeline, GL_GEOMETRY_SHADER_BIT, geomFragProg);
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, geomFragProg);
    glBindProgramPipeline(pipeline);

    glActiveShaderProgram(pipeline, vertProg);
    uniLoc = glGetUniformLocation(vertProg, "uniVec");
    ASSERT_NE(-1, uniLoc);
    glUniform4f(uniLoc, 0, 1, 0, 1);
    ASSERT_GL_NO_ERROR();

    drawQuadPPO(vertProg, "position", 0.5f, 1.0f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests that varying limits work as expected with geometry shaders.
TEST_P(GeometryShaderTest, MaxVaryings)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    // Get appropriate limitations.
    GLint maxVertexOutputComponents = 0;
    glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &maxVertexOutputComponents);
    ASSERT_GT(maxVertexOutputComponents, 0);

    GLint maxGeometryInputComponents = 0;
    glGetIntegerv(GL_MAX_GEOMETRY_INPUT_COMPONENTS, &maxGeometryInputComponents);
    ASSERT_GT(maxGeometryInputComponents, 0);

    GLint maxGeometryOutputComponents = 0;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, &maxGeometryOutputComponents);
    ASSERT_GT(maxGeometryOutputComponents, 0);

    GLint maxFragmentInputComponents = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &maxFragmentInputComponents);
    ASSERT_GT(maxFragmentInputComponents, 0);

    GLint vertexToGeometryVaryings =
        std::min(maxVertexOutputComponents, maxGeometryInputComponents) / 4;
    GLint geometryToFragmentVaryings =
        std::min(maxGeometryOutputComponents, maxFragmentInputComponents) / 4;

    GLint varyingCount = std::min(vertexToGeometryVaryings, geometryToFragmentVaryings);

    // Reserve gl_Position;
    varyingCount--;

    // Create a vertex shader with "varyingCount" outputs.
    std::stringstream vsStream;
    vsStream << R"(#version 310 es
uniform vec4 uniOne;
in vec4 position;
)";

    for (GLint varyingIndex = 0; varyingIndex < varyingCount; ++varyingIndex)
    {
        vsStream << "out vec4 v" << varyingIndex << ";\n";
    }

    vsStream << R"(
void main()
{
    gl_Position = position;
)";

    for (GLint varyingIndex = 0; varyingIndex < varyingCount; ++varyingIndex)
    {
        vsStream << "    v" << varyingIndex << " = uniOne * " << varyingIndex << ".0;\n";
    }

    vsStream << "}";

    // Create a GS with "varyingCount" inputs and "varyingCount" outputs.
    std::stringstream gsStream;
    gsStream << R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;
)";

    for (GLint varyingIndex = 0; varyingIndex < varyingCount; ++varyingIndex)
    {
        gsStream << "in vec4 v" << varyingIndex << "[];\n";
    }

    for (GLint varyingIndex = 0; varyingIndex < varyingCount; ++varyingIndex)
    {
        gsStream << "out vec4 o" << varyingIndex << ";\n";
    }

    gsStream << R"(
void main()
{
    for (int n = 0; n < gl_in.length(); n++)
    {
        gl_Position = gl_in[n].gl_Position;
)";

    for (GLint varyingIndex = 0; varyingIndex < varyingCount; ++varyingIndex)
    {
        gsStream << "        o" << varyingIndex << " = v" << varyingIndex << "[n];\n";
    }

    gsStream << R"(
        EmitVertex();
    }
    EndPrimitive();
}
)";

    // Create a FS with "varyingCount" inputs.
    std::stringstream fsStream;
    fsStream << R"(#version 310 es
precision mediump float;
out vec4 color;
)";

    for (GLint varyingIndex = 0; varyingIndex < varyingCount; ++varyingIndex)
    {
        fsStream << "in vec4 o" << varyingIndex << ";\n";
    }

    fsStream << R"(
void main()
{
    color = vec4(0, 1, 0, 1);
)";

    for (GLint varyingIndex = 0; varyingIndex < varyingCount; ++varyingIndex)
    {
        fsStream << "    if (o" << varyingIndex << " != vec4(" << varyingIndex << ".0))\n"
                 << "        color = vec4(1, 0, 0, 1);\n";
    }

    fsStream << "}";

    const std::string vs = vsStream.str();
    const std::string gs = gsStream.str();
    const std::string fs = fsStream.str();

    ANGLE_GL_PROGRAM_WITH_GS(program, vs.c_str(), gs.c_str(), fs.c_str());
    glUseProgram(program);

    GLint uniLoc = glGetUniformLocation(program, "uniOne");
    ASSERT_NE(-1, uniLoc);
    glUniform4f(uniLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    drawQuad(program, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GeometryShaderTestES3);
ANGLE_INSTANTIATE_TEST_ES3(GeometryShaderTestES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GeometryShaderTest);
ANGLE_INSTANTIATE_TEST_ES31_AND(GeometryShaderTest,
                                ES31_VULKAN().enable(Feature::EmulatedPrerotation90),
                                ES31_VULKAN().enable(Feature::EmulatedPrerotation180),
                                ES31_VULKAN().enable(Feature::EmulatedPrerotation270));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GeometryShaderTestES32);
ANGLE_INSTANTIATE_TEST_ES32(GeometryShaderTestES32);
}  // namespace
