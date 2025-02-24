//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramInterfaceTest: Tests of program interfaces.

#include "common/string_utils.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

// Variations:
//
// - bool: whether the program must be created and recreated, so that it's reloaded from cache.
using ProgramInterfaceTestParams = std::tuple<angle::PlatformParameters, bool>;

std::string ProgramInterfaceTestPrint(
    const ::testing::TestParamInfo<ProgramInterfaceTestParams> &paramsInfo)
{
    const ProgramInterfaceTestParams &params = paramsInfo.param;
    std::ostringstream out;

    out << std::get<0>(params);

    if (std::get<1>(params))
    {
        out << "__cached";
    }

    return out.str();
}

class ProgramInterfaceTestES31 : public ANGLETest<ProgramInterfaceTestParams>
{
  protected:
    ProgramInterfaceTestES31()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void createGraphicsProgram(GLProgram &program,
                               const char *vs,
                               const char *fs,
                               bool cacheAndReload);
    void createComputeProgram(GLProgram &program, const char *cs, bool cacheAndReload);
};

void ProgramInterfaceTestES31::createGraphicsProgram(GLProgram &program,
                                                     const char *vs,
                                                     const char *fs,
                                                     bool cacheAndReload)
{
    program.makeRaster(vs, fs);
    ASSERT_TRUE(program.valid());

    if (cacheAndReload)
    {
        program.reset();
        program.makeRaster(vs, fs);
        ASSERT_TRUE(program.valid());
    }
}

void ProgramInterfaceTestES31::createComputeProgram(GLProgram &program,
                                                    const char *cs,
                                                    bool cacheAndReload)
{
    program.makeCompute(cs);
    ASSERT_TRUE(program.valid());

    if (cacheAndReload)
    {
        program.reset();
        program.makeCompute(cs);
        ASSERT_TRUE(program.valid());
    }
}

// Tests glGetProgramResourceIndex.
TEST_P(ProgramInterfaceTestES31, GetResourceIndex)
{
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform vec4 color;\n"
        "out vec4 oColor;\n"
        "void main()\n"
        "{\n"
        "    oColor = color;\n"
        "}";

    GLProgram program;
    createGraphicsProgram(program, essl31_shaders::vs::Simple(), kFS, std::get<1>(GetParam()));

    GLuint index =
        glGetProgramResourceIndex(program, GL_PROGRAM_INPUT, essl31_shaders::PositionAttrib());
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    index = glGetProgramResourceIndex(program, GL_PROGRAM_INPUT, "missing");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_INVALID_INDEX, index);

    index = glGetProgramResourceIndex(program, GL_PROGRAM_OUTPUT, "oColor");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    index = glGetProgramResourceIndex(program, GL_PROGRAM_OUTPUT, "missing");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_INVALID_INDEX, index);

    index = glGetProgramResourceIndex(program, GL_ATOMIC_COUNTER_BUFFER, "missing");
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Tests glGetProgramResourceName.
TEST_P(ProgramInterfaceTestES31, GetResourceName)
{
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform vec4 color;\n"
        "out vec4 oColor[4];\n"
        "void main()\n"
        "{\n"
        "    oColor[0] = color;\n"
        "}";

    GLProgram program;
    createGraphicsProgram(program, essl31_shaders::vs::Simple(), kFS, std::get<1>(GetParam()));

    GLuint index =
        glGetProgramResourceIndex(program, GL_PROGRAM_INPUT, essl31_shaders::PositionAttrib());
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    GLchar name[64];
    GLsizei length;
    glGetProgramResourceName(program, GL_PROGRAM_INPUT, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(static_cast<int>(strlen(essl31_shaders::PositionAttrib())), length);
    EXPECT_EQ(essl31_shaders::PositionAttrib(), std::string(name));

    glGetProgramResourceName(program, GL_PROGRAM_INPUT, index, 4, &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(3, length);
    EXPECT_TRUE(angle::BeginsWith(essl31_shaders::PositionAttrib(), name));

    glGetProgramResourceName(program, GL_PROGRAM_INPUT, index, -1, &length, name);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glGetProgramResourceName(program, GL_PROGRAM_INPUT, GL_INVALID_INDEX, sizeof(name), &length,
                             name);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    index = glGetProgramResourceIndex(program, GL_PROGRAM_OUTPUT, "oColor");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    glGetProgramResourceName(program, GL_PROGRAM_OUTPUT, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(9, length);
    EXPECT_EQ("oColor[0]", std::string(name));

    glGetProgramResourceName(program, GL_PROGRAM_OUTPUT, index, 8, &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(7, length);
    EXPECT_EQ("oColor[", std::string(name));
}

// Tests glGetProgramResourceLocation.
TEST_P(ProgramInterfaceTestES31, GetResourceLocation)
{
    constexpr char kVS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "layout(location = 3) in highp vec4 position;\n"
        "in highp vec4 noLocationSpecified;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = position;\n"
        "}";

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform vec4 color;\n"
        "layout(location = 1) out vec4 oColor[3];\n"
        "void main()\n"
        "{\n"
        "    oColor[0] = color;\n"
        "}";

    GLProgram program;
    createGraphicsProgram(program, kVS, kFS, std::get<1>(GetParam()));

    GLenum invalidInterfaces[] = {GL_UNIFORM_BLOCK, GL_TRANSFORM_FEEDBACK_VARYING,
                                  GL_BUFFER_VARIABLE, GL_SHADER_STORAGE_BLOCK,
                                  GL_ATOMIC_COUNTER_BUFFER};
    GLint location;
    for (auto &invalidInterface : invalidInterfaces)
    {
        location = glGetProgramResourceLocation(program, invalidInterface, "any");
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
        EXPECT_EQ(-1, location);
    }

    location = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "position");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(3, location);

    location = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "noLocationSpecified");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(-1, location);

    location = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, "missing");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(-1, location);

    location = glGetProgramResourceLocation(program, GL_PROGRAM_OUTPUT, "oColor");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1, location);
    location = glGetProgramResourceLocation(program, GL_PROGRAM_OUTPUT, "oColor[0]");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1, location);
    location = glGetProgramResourceLocation(program, GL_PROGRAM_OUTPUT, "oColor[2]");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(3, location);
}

// Tests glGetProgramResource.
TEST_P(ProgramInterfaceTestES31, GetResource)
{
    constexpr char kVS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "layout(location = 3) in highp vec4 position;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = position;\n"
        "}";

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform vec4 color;\n"
        "layout(location = 1) out vec4 oColor[3];\n"
        "void main()\n"
        "{\n"
        "    oColor[0] = color;\n"
        "}";

    GLProgram program;
    createGraphicsProgram(program, kVS, kFS, std::get<1>(GetParam()));

    GLuint index = glGetProgramResourceIndex(program, GL_PROGRAM_INPUT, "position");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    GLenum props[]    = {GL_TYPE,
                         GL_ARRAY_SIZE,
                         GL_LOCATION,
                         GL_NAME_LENGTH,
                         GL_REFERENCED_BY_VERTEX_SHADER,
                         GL_REFERENCED_BY_FRAGMENT_SHADER,
                         GL_REFERENCED_BY_COMPUTE_SHADER};
    GLsizei propCount = static_cast<GLsizei>(ArraySize(props));
    GLint params[ArraySize(props)];
    GLsizei length;

    glGetProgramResourceiv(program, GL_PROGRAM_INPUT, index, propCount, props, propCount, &length,
                           params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(GL_FLOAT_VEC4, params[0]);  // type
    EXPECT_EQ(1, params[1]);              // array_size
    EXPECT_EQ(3, params[2]);              // location
    EXPECT_EQ(9, params[3]);              // name_length
    EXPECT_EQ(1, params[4]);              // referenced_by_vertex_shader
    EXPECT_EQ(0, params[5]);              // referenced_by_fragment_shader
    EXPECT_EQ(0, params[6]);              // referenced_by_compute_shader

    index = glGetProgramResourceIndex(program, GL_PROGRAM_OUTPUT, "oColor[0]");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(index, GL_INVALID_INDEX);
    // bufSize is smaller than propCount.
    glGetProgramResourceiv(program, GL_PROGRAM_OUTPUT, index, propCount, props, propCount - 1,
                           &length, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount - 1, length);
    EXPECT_EQ(GL_FLOAT_VEC4, params[0]);  // type
    EXPECT_EQ(3, params[1]);              // array_size
    EXPECT_EQ(1, params[2]);              // location
    EXPECT_EQ(10, params[3]);             // name_length
    EXPECT_EQ(0, params[4]);              // referenced_by_vertex_shader
    EXPECT_EQ(1, params[5]);              // referenced_by_fragment_shader

    GLenum invalidOutputProp = GL_OFFSET;
    glGetProgramResourceiv(program, GL_PROGRAM_OUTPUT, index, 1, &invalidOutputProp, 1, &length,
                           params);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests glGetProgramInterfaceiv.
TEST_P(ProgramInterfaceTestES31, GetProgramInterface)
{
    // TODO(jiajia.qin@intel.com): Don't skip this test once SSBO are supported on render pipeline.
    // http://anglebug.com/40644618
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform vec4 color;\n"
        "out vec4 oColor;\n"
        "uniform ub {\n"
        "    vec4 mem0;\n"
        "    vec4 mem1;\n"
        "} instance;\n"
        "layout(std430) buffer shaderStorageBlock1 {\n"
        "    vec3 target;\n"
        "};\n"
        "layout(std430) buffer shaderStorageBlock2 {\n"
        "    vec3 target;\n"
        "} blockInstance2[1];\n"
        "void main()\n"
        "{\n"
        "    oColor = color;\n"
        "    target = vec3(0, 0, 0);\n"
        "    blockInstance2[0].target = vec3(1, 1, 1);\n"
        "}";

    GLProgram program;
    createGraphicsProgram(program, essl31_shaders::vs::Simple(), kFS, std::get<1>(GetParam()));

    GLint num;
    glGetProgramInterfaceiv(program, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1, num);

    glGetProgramInterfaceiv(program, GL_PROGRAM_INPUT, GL_MAX_NAME_LENGTH, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(static_cast<GLint>(strlen(essl3_shaders::PositionAttrib())) + 1, num);

    glGetProgramInterfaceiv(program, GL_PROGRAM_INPUT, GL_MAX_NUM_ACTIVE_VARIABLES, &num);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1, num);

    glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_MAX_NAME_LENGTH, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(7, num);

    glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_MAX_NUM_ACTIVE_VARIABLES, &num);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1, num);

    glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_MAX_NAME_LENGTH, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(3, num);

    glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_MAX_NUM_ACTIVE_VARIABLES, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(2, num);  // mem0, mem1

    glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(3, num);

    glGetProgramInterfaceiv(program, GL_UNIFORM, GL_MAX_NAME_LENGTH, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(8, num);  // "ub.mem0"

    glGetProgramInterfaceiv(program, GL_UNIFORM, GL_MAX_NUM_ACTIVE_VARIABLES, &num);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(2, num);

    glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(23, num);  // "shaderStorageBlock2[0]"

    glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_MAX_NUM_ACTIVE_VARIABLES, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1, num);
}

// Tests the resource property query for uniform can be done correctly.
TEST_P(ProgramInterfaceTestES31, GetUniformProperties)
{
    // Check atomic support.
    GLint numSupported;
    glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTERS, &numSupported);
    EXPECT_GL_NO_ERROR();
    ANGLE_SKIP_TEST_IF(numSupported < 1);

    constexpr char kVS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform layout(location=12) vec4 color;\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint foo;\n"
        "void main()\n"
        "{\n"
        "    atomicCounterIncrement(foo);\n"
        "}";

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform vec4 color;\n"
        "out vec4 oColor;\n"
        "void main()\n"
        "{\n"
        "    oColor = color;\n"
        "}";

    GLProgram program;
    createGraphicsProgram(program, kVS, kFS, std::get<1>(GetParam()));

    GLuint index = glGetProgramResourceIndex(program, GL_UNIFORM, "color");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    GLchar name[64];
    GLsizei length;
    glGetProgramResourceName(program, GL_UNIFORM, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(5, length);
    EXPECT_EQ("color", std::string(name));

    GLint location = glGetProgramResourceLocation(program, GL_UNIFORM, "color");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(12, location);

    GLenum props[]    = {GL_TYPE,
                         GL_ARRAY_SIZE,
                         GL_LOCATION,
                         GL_NAME_LENGTH,
                         GL_REFERENCED_BY_VERTEX_SHADER,
                         GL_REFERENCED_BY_FRAGMENT_SHADER,
                         GL_REFERENCED_BY_COMPUTE_SHADER,
                         GL_ARRAY_STRIDE,
                         GL_BLOCK_INDEX,
                         GL_IS_ROW_MAJOR,
                         GL_MATRIX_STRIDE,
                         GL_OFFSET,
                         GL_ATOMIC_COUNTER_BUFFER_INDEX};
    GLsizei propCount = static_cast<GLsizei>(ArraySize(props));
    GLint params[ArraySize(props)];
    glGetProgramResourceiv(program, GL_UNIFORM, index, propCount, props, propCount, &length,
                           params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(GL_FLOAT_VEC4, params[0]);  // type
    EXPECT_EQ(1, params[1]);              // array_size
    EXPECT_EQ(12, params[2]);             // location
    EXPECT_EQ(6, params[3]);              // name_length
    EXPECT_EQ(0, params[4]);              // referenced_by_vertex_shader
    EXPECT_EQ(1, params[5]);              // referenced_by_fragment_shader
    EXPECT_EQ(0, params[6]);              // referenced_by_compute_shader
    EXPECT_EQ(-1, params[7]);             // array_stride
    EXPECT_EQ(-1, params[8]);             // block_index
    EXPECT_EQ(0, params[9]);              // is_row_major
    EXPECT_EQ(-1, params[10]);            // matrix_stride
    EXPECT_EQ(-1, params[11]);            // offset
    EXPECT_EQ(-1, params[12]);            // atomic_counter_buffer_index

    index = glGetProgramResourceIndex(program, GL_UNIFORM, "foo");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    glGetProgramResourceName(program, GL_UNIFORM, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(3, length);
    EXPECT_EQ("foo", std::string(name));

    location = glGetProgramResourceLocation(program, GL_UNIFORM, "foo");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(-1, location);

    glGetProgramResourceiv(program, GL_UNIFORM, index, propCount, props, propCount, &length,
                           params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(GL_UNSIGNED_INT_ATOMIC_COUNTER, params[0]);  // type
    EXPECT_EQ(1, params[1]);                               // array_size
    EXPECT_EQ(-1, params[2]);                              // location
    EXPECT_EQ(4, params[3]);                               // name_length
    EXPECT_EQ(1, params[4]);                               // referenced_by_vertex_shader
    EXPECT_EQ(0, params[5]);                               // referenced_by_fragment_shader
    EXPECT_EQ(0, params[6]);                               // referenced_by_compute_shader
    EXPECT_EQ(0, params[7]);                               // array_stride
    EXPECT_EQ(-1, params[8]);                              // block_index
    EXPECT_EQ(0, params[9]);                               // is_row_major
    EXPECT_EQ(0, params[10]);                              // matrix_stride
    EXPECT_EQ(4, params[11]);                              // offset
    EXPECT_NE(-1, params[12]);                             // atomic_counter_buffer_index
}

// Tests the resource property query for uniform block can be done correctly.
TEST_P(ProgramInterfaceTestES31, GetUniformBlockProperties)
{
    constexpr char kVS[] =
        "#version 310 es\n"
        "in vec2 position;\n"
        "out vec2 v;\n"
        "layout(binding = 2) uniform blockName {\n"
        "  float f1;\n"
        "  float f2;\n"
        "} instanceName;\n"
        "void main() {\n"
        "  v = vec2(instanceName.f1, instanceName.f2);\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "}";

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "in vec2 v;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "  color = vec4(v, 0, 1);\n"
        "}";

    GLProgram program;
    createGraphicsProgram(program, kVS, kFS, std::get<1>(GetParam()));

    GLuint index = glGetProgramResourceIndex(program, GL_UNIFORM_BLOCK, "blockName");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    GLchar name[64];
    GLsizei length;
    glGetProgramResourceName(program, GL_UNIFORM_BLOCK, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(9, length);
    EXPECT_EQ("blockName", std::string(name));

    GLenum props[]         = {GL_BUFFER_BINDING,
                              GL_BUFFER_DATA_SIZE,
                              GL_NAME_LENGTH,
                              GL_NUM_ACTIVE_VARIABLES,
                              GL_ACTIVE_VARIABLES,
                              GL_REFERENCED_BY_VERTEX_SHADER,
                              GL_REFERENCED_BY_FRAGMENT_SHADER,
                              GL_REFERENCED_BY_COMPUTE_SHADER};
    GLsizei propCount      = static_cast<GLsizei>(ArraySize(props));
    constexpr int kBufSize = 256;
    GLint params[kBufSize];
    GLint magic = 0xBEEF;

    // Tests bufSize is respected even some prop returns more than one value.
    params[propCount] = magic;
    glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, index, propCount, props, propCount, &length,
                           params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(2, params[0]);   // buffer_binding
    EXPECT_NE(0, params[1]);   // buffer_data_size
    EXPECT_EQ(10, params[2]);  // name_length
    EXPECT_EQ(2, params[3]);   // num_active_variables
    EXPECT_LE(0, params[4]);   // index of 'f1' or 'f2'
    EXPECT_LE(0, params[5]);   // index of 'f1' or 'f2'
    EXPECT_EQ(1, params[6]);   // referenced_by_vertex_shader
    EXPECT_EQ(0, params[7]);   // referenced_by_fragment_shader
    EXPECT_EQ(magic, params[8]);

    glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, index, propCount, props, kBufSize, &length,
                           params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount + 1, length);
    EXPECT_EQ(0, params[8]);  // referenced_by_compute_shader

    // bufSize is reached in middle of outputting values for GL_ACTIVE_VARIABLES.
    GLenum actvieVariablesProperty = GL_ACTIVE_VARIABLES;
    params[1]                      = magic;
    glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, index, 1, &actvieVariablesProperty, 1,
                           &length, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1, length);
    EXPECT_LE(0, params[0]);  // index of 'f1' or 'f2'
    EXPECT_EQ(magic, params[1]);
}

// Tests atomic counter buffer qeury works correctly.
TEST_P(ProgramInterfaceTestES31, QueryAtomicCounteBuffer)
{
    // Check atomic support.
    GLint numSupportedInVertex;
    GLint numSupportedInFragment;
    glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTERS, &numSupportedInVertex);
    glGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTERS, &numSupportedInFragment);
    EXPECT_GL_NO_ERROR();
    ANGLE_SKIP_TEST_IF(numSupportedInVertex < 1 || numSupportedInFragment < 1);

    constexpr char kVS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "layout(binding = 2, offset = 0) uniform atomic_uint vcounter;\n"
        "in highp vec4 a_position;\n"
        "void main()\n"
        "{\n"
        "    atomicCounterIncrement(vcounter);\n"
        "    gl_Position = a_position;\n"
        "}\n";

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint fcounter;\n"
        "out highp vec4 my_color;\n"
        "void main()\n"
        "{\n"
        "    atomicCounterDecrement(fcounter);\n"
        "    my_color = vec4(0.0);\n"
        "}\n";

    GLProgram program;
    createGraphicsProgram(program, kVS, kFS, std::get<1>(GetParam()));

    GLint num;
    glGetProgramInterfaceiv(program, GL_ATOMIC_COUNTER_BUFFER, GL_ACTIVE_RESOURCES, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1, num);

    glGetProgramInterfaceiv(program, GL_ATOMIC_COUNTER_BUFFER, GL_MAX_NUM_ACTIVE_VARIABLES, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(2, num);

    GLenum props[]    = {GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES, GL_REFERENCED_BY_VERTEX_SHADER,
                         GL_REFERENCED_BY_FRAGMENT_SHADER, GL_REFERENCED_BY_COMPUTE_SHADER};
    GLsizei propCount = static_cast<GLsizei>(ArraySize(props));
    GLint params[ArraySize(props)];
    GLsizei length = 0;
    glGetProgramResourceiv(program, GL_ATOMIC_COUNTER_BUFFER, 0, propCount, props, propCount,
                           &length, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(2, params[0]);  // buffer_binding
    EXPECT_EQ(2, params[1]);  // num_active_variables
    EXPECT_EQ(1, params[2]);  // referenced_by_vertex_shader
    EXPECT_EQ(1, params[3]);  // referenced_by_fragment_shader
    EXPECT_EQ(0, params[4]);  // referenced_by_compute_shader
}

// Tests the resource property query for buffer variable can be done correctly.
TEST_P(ProgramInterfaceTestES31, GetBufferVariableProperties)
{
    // TODO(jiajia.qin@intel.com): Don't skip this test once non-simple SSBO sentences are supported
    // on d3d backend. http://anglebug.com/40644618
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // Check SSBO support
    GLint numSupported;
    glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &numSupported);
    EXPECT_GL_NO_ERROR();
    ANGLE_SKIP_TEST_IF(numSupported < 2);

    constexpr char kVS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "struct S {\n"
        "    vec3 a;\n"
        "    ivec2 b[4];\n"
        "};\n"
        "layout(std140) buffer blockName0 {\n"
        "    S s0;\n"
        "    vec2 v0;\n"
        "    S s1[2];\n"
        "    uint u0;\n"
        "};\n"
        "layout(binding = 1) buffer blockName1 {\n"
        "    uint u1[2];\n"
        "    float f1;\n"
        "} instanceName1[2];\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(instanceName1[0].f1, s1[0].a);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "layout(binding = 1) buffer blockName1 {\n"
        "    uint u1[2];\n"
        "    float f1;\n"
        "} instanceName1[2];\n"
        "out vec4 oColor;\n"
        "void main()\n"
        "{\n"
        "    oColor = vec4(instanceName1[0].f1, 0, 0, 1);\n"
        "}";

    GLProgram program;
    createGraphicsProgram(program, kVS, kFS, std::get<1>(GetParam()));

    GLuint index = glGetProgramResourceIndex(program, GL_BUFFER_VARIABLE, "blockName1.f1");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    GLchar name[64];
    GLsizei length;
    glGetProgramResourceName(program, GL_BUFFER_VARIABLE, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(13, length);
    EXPECT_EQ("blockName1.f1", std::string(name));

    GLenum props[]         = {GL_ARRAY_SIZE,
                              GL_ARRAY_STRIDE,
                              GL_BLOCK_INDEX,
                              GL_IS_ROW_MAJOR,
                              GL_MATRIX_STRIDE,
                              GL_NAME_LENGTH,
                              GL_OFFSET,
                              GL_REFERENCED_BY_VERTEX_SHADER,
                              GL_REFERENCED_BY_FRAGMENT_SHADER,
                              GL_REFERENCED_BY_COMPUTE_SHADER,
                              GL_TOP_LEVEL_ARRAY_SIZE,
                              GL_TOP_LEVEL_ARRAY_STRIDE,
                              GL_TYPE};
    GLsizei propCount      = static_cast<GLsizei>(ArraySize(props));
    constexpr int kBufSize = 256;
    GLint params[kBufSize];

    glGetProgramResourceiv(program, GL_BUFFER_VARIABLE, index, propCount, props, kBufSize, &length,
                           params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(1, params[0]);   // array_size
    EXPECT_LE(0, params[1]);   // array_stride
    EXPECT_LE(0, params[2]);   // block_index
    EXPECT_EQ(0, params[3]);   // is_row_major
    EXPECT_EQ(0, params[4]);   // matrix_stride
    EXPECT_EQ(14, params[5]);  // name_length
    EXPECT_LE(0, params[6]);   // offset

    EXPECT_EQ(1, params[7]);  // referenced_by_vertex_shader
    EXPECT_EQ(1, params[8]);  // referenced_by_fragment_shader
    EXPECT_EQ(0, params[9]);  // referenced_by_compute_shader

    EXPECT_EQ(1, params[10]);  // top_level_array_size
    EXPECT_LE(0, params[11]);  // top_level_array_stride

    EXPECT_EQ(GL_FLOAT, params[12]);  // type

    index = glGetProgramResourceIndex(program, GL_BUFFER_VARIABLE, "s1[0].a");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    glGetProgramResourceName(program, GL_BUFFER_VARIABLE, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(7, length);
    EXPECT_EQ("s1[0].a", std::string(name));

    glGetProgramResourceiv(program, GL_BUFFER_VARIABLE, index, propCount, props, kBufSize, &length,
                           params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(1, params[0]);  // array_size
    EXPECT_LE(0, params[1]);  // array_stride
    EXPECT_LE(0, params[2]);  // block_index
    EXPECT_EQ(0, params[3]);  // is_row_major
    EXPECT_EQ(0, params[4]);  // matrix_stride
    EXPECT_EQ(8, params[5]);  // name_length
    EXPECT_LE(0, params[6]);  // offset

    EXPECT_EQ(1, params[7]);  // referenced_by_vertex_shader
    EXPECT_EQ(0, params[8]);  // referenced_by_fragment_shader
    EXPECT_EQ(0, params[9]);  // referenced_by_compute_shader

    EXPECT_EQ(2, params[10]);   // top_level_array_size
    EXPECT_EQ(80, params[11]);  // top_level_array_stride

    EXPECT_EQ(GL_FLOAT_VEC3, params[12]);  // type
}

// Tests the resource property querying for buffer variable in std430 SSBO works correctly.
TEST_P(ProgramInterfaceTestES31, GetStd430BufferVariableProperties)
{
    ANGLE_SKIP_TEST_IF(IsAMD() && IsWindows() && IsOpenGL());

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
struct S
{
    uvec2 v;
    mat2 m;
};
layout(std430, binding = 0) buffer blockIn {
    uint u;
    uint a[2];
    S s;
} instanceIn;
layout(std430, binding = 1) buffer blockOut {
    uint u;
    uint a[2];
    S s;
} instanceOut;
void main()
{
    instanceOut.u = instanceIn.u;
    instanceOut.a[0] = instanceIn.a[0];
    instanceOut.a[1] = instanceIn.a[1];
    instanceOut.s.v = instanceIn.s.v;
    instanceOut.s.m = instanceIn.s.m;
}
)";

    GLProgram program;
    createComputeProgram(program, kComputeShaderSource, std::get<1>(GetParam()));

    GLuint index = glGetProgramResourceIndex(program, GL_BUFFER_VARIABLE, "blockIn.a");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    GLchar name[64];
    GLsizei length;
    glGetProgramResourceName(program, GL_BUFFER_VARIABLE, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(12, length);
    EXPECT_EQ("blockIn.a[0]", std::string(name));

    GLenum props[]         = {GL_ARRAY_SIZE,
                              GL_ARRAY_STRIDE,
                              GL_BLOCK_INDEX,
                              GL_IS_ROW_MAJOR,
                              GL_MATRIX_STRIDE,
                              GL_NAME_LENGTH,
                              GL_OFFSET,
                              GL_REFERENCED_BY_VERTEX_SHADER,
                              GL_REFERENCED_BY_FRAGMENT_SHADER,
                              GL_REFERENCED_BY_COMPUTE_SHADER,
                              GL_TOP_LEVEL_ARRAY_SIZE,
                              GL_TOP_LEVEL_ARRAY_STRIDE,
                              GL_TYPE};
    GLsizei propCount      = static_cast<GLsizei>(ArraySize(props));
    constexpr int kBufSize = 256;
    GLint params[kBufSize];

    glGetProgramResourceiv(program, GL_BUFFER_VARIABLE, index, propCount, props, kBufSize, &length,
                           params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(2, params[0]);   // array_size
    EXPECT_LE(4, params[1]);   // array_stride
    EXPECT_LE(0, params[2]);   // block_index
    EXPECT_EQ(0, params[3]);   // is_row_major
    EXPECT_EQ(0, params[4]);   // matrix_stride
    EXPECT_EQ(13, params[5]);  // name_length
    EXPECT_EQ(4, params[6]);   // offset

    EXPECT_EQ(0, params[7]);  // referenced_by_vertex_shader
    EXPECT_EQ(0, params[8]);  // referenced_by_fragment_shader
    EXPECT_EQ(1, params[9]);  // referenced_by_compute_shader

    EXPECT_EQ(1, params[10]);                // top_level_array_size
    EXPECT_EQ(0, params[11]);                // top_level_array_stride
    EXPECT_EQ(GL_UNSIGNED_INT, params[12]);  // type

    index = glGetProgramResourceIndex(program, GL_BUFFER_VARIABLE, "blockIn.s.m");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    glGetProgramResourceName(program, GL_BUFFER_VARIABLE, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(11, length);
    EXPECT_EQ("blockIn.s.m", std::string(name));

    glGetProgramResourceiv(program, GL_BUFFER_VARIABLE, index, propCount, props, kBufSize, &length,
                           params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(1, params[0]);   // array_size
    EXPECT_LE(0, params[1]);   // array_stride
    EXPECT_LE(0, params[2]);   // block_index
    EXPECT_EQ(0, params[3]);   // is_row_major
    EXPECT_EQ(8, params[4]);   // matrix_stride
    EXPECT_EQ(12, params[5]);  // name_length
    EXPECT_EQ(24, params[6]);  // offset

    EXPECT_EQ(0, params[7]);  // referenced_by_vertex_shader
    EXPECT_EQ(0, params[8]);  // referenced_by_fragment_shader
    // TODO(jiajia.qin@intel.com): referenced_by_compute_shader is not
    // correctly handled. http://anglebug.com/42260711.
    // EXPECT_EQ(1, params[9]);   // referenced_by_compute_shader

    EXPECT_EQ(1, params[10]);              // top_level_array_size
    EXPECT_EQ(0, params[11]);              // top_level_array_stride
    EXPECT_EQ(GL_FLOAT_MAT2, params[12]);  // type
}

// Test that TOP_LEVEL_ARRAY_STRIDE for buffer variable with aggregate type works correctly.
TEST_P(ProgramInterfaceTestES31, TopLevelArrayStrideWithAggregateType)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
struct S
{
    uvec2 v;
    mat2 m;
};
layout(std430, binding = 0) buffer blockIn {
    uint u;
    uint a[2];
    S s;
} instanceIn;
layout(std430, binding = 1) buffer blockOut {
    uint u;
    uint a[4][3];
    S s[3][2];
} instanceOut;
void main()
{
    instanceOut.u = instanceIn.u;
    instanceOut.a[0][0] = instanceIn.a[0];
    instanceOut.a[0][1] = instanceIn.a[1];
    instanceOut.s[0][0].v = instanceIn.s.v;
    instanceOut.s[0][0].m = instanceIn.s.m;
}
)";

    GLProgram program;
    createComputeProgram(program, kComputeShaderSource, std::get<1>(GetParam()));

    GLuint index = glGetProgramResourceIndex(program, GL_BUFFER_VARIABLE, "blockOut.s[0][0].m");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    GLchar name[64];
    GLsizei length;
    glGetProgramResourceName(program, GL_BUFFER_VARIABLE, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(18, length);
    EXPECT_EQ("blockOut.s[0][0].m", std::string(name));

    GLenum props[]         = {GL_ARRAY_SIZE,
                              GL_ARRAY_STRIDE,
                              GL_BLOCK_INDEX,
                              GL_IS_ROW_MAJOR,
                              GL_MATRIX_STRIDE,
                              GL_NAME_LENGTH,
                              GL_OFFSET,
                              GL_REFERENCED_BY_VERTEX_SHADER,
                              GL_REFERENCED_BY_FRAGMENT_SHADER,
                              GL_REFERENCED_BY_COMPUTE_SHADER,
                              GL_TOP_LEVEL_ARRAY_SIZE,
                              GL_TOP_LEVEL_ARRAY_STRIDE,
                              GL_TYPE};
    GLsizei propCount      = static_cast<GLsizei>(ArraySize(props));
    constexpr int kBufSize = 256;
    GLint params[kBufSize];
    glGetProgramResourceiv(program, GL_BUFFER_VARIABLE, index, propCount, props, kBufSize, &length,
                           params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(1, params[0]);   // array_size
    EXPECT_LE(0, params[1]);   // array_stride
    EXPECT_LE(0, params[2]);   // block_index
    EXPECT_EQ(0, params[3]);   // is_row_major
    EXPECT_EQ(8, params[4]);   // matrix_stride
    EXPECT_EQ(19, params[5]);  // name_length
    EXPECT_EQ(64, params[6]);  // offset

    EXPECT_EQ(0, params[7]);  // referenced_by_vertex_shader
    EXPECT_EQ(0, params[8]);  // referenced_by_fragment_shader
    // TODO(jiajia.qin@intel.com): referenced_by_compute_shader is not
    // correctly handled. http://anglebug.com/42260711.
    // EXPECT_EQ(1, params[9]);   // referenced_by_compute_shader
    EXPECT_EQ(3, params[10]);              // top_level_array_size
    EXPECT_EQ(48, params[11]);             // top_level_array_stride
    EXPECT_EQ(GL_FLOAT_MAT2, params[12]);  // type

    index = glGetProgramResourceIndex(program, GL_BUFFER_VARIABLE, "blockOut.a[0][0]");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    glGetProgramResourceName(program, GL_BUFFER_VARIABLE, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(16, length);
    EXPECT_EQ("blockOut.a[0][0]", std::string(name));

    glGetProgramResourceiv(program, GL_BUFFER_VARIABLE, index, propCount, props, kBufSize, &length,
                           params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(3, params[0]);   // array_size
    EXPECT_LE(0, params[1]);   // array_stride
    EXPECT_LE(0, params[2]);   // block_index
    EXPECT_EQ(0, params[3]);   // is_row_major
    EXPECT_EQ(0, params[4]);   // matrix_stride
    EXPECT_EQ(17, params[5]);  // name_length
    EXPECT_EQ(4, params[6]);   // offset

    EXPECT_EQ(0, params[7]);                 // referenced_by_vertex_shader
    EXPECT_EQ(0, params[8]);                 // referenced_by_fragment_shader
    EXPECT_EQ(1, params[9]);                 // referenced_by_compute_shader
    EXPECT_EQ(4, params[10]);                // top_level_array_size
    EXPECT_EQ(12, params[11]);               // top_level_array_stride
    EXPECT_EQ(GL_UNSIGNED_INT, params[12]);  // type
}

// Tests the resource property query for shader storage block can be done correctly.
TEST_P(ProgramInterfaceTestES31, GetShaderStorageBlockProperties)
{
    // TODO(jiajia.qin@intel.com): Don't skip this test once non-simple SSBO sentences are supported
    // on d3d backend. http://anglebug.com/40644618
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // Check SSBO support
    GLint numSupported;
    glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &numSupported);
    EXPECT_GL_NO_ERROR();
    ANGLE_SKIP_TEST_IF(numSupported < 3);

    constexpr char kVS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "struct S {\n"
        "    vec3 a;\n"
        "    ivec2 b[4];\n"
        "};\n"
        "layout(std140) buffer blockName0 {\n"
        "    S s0;\n"
        "    vec2 v0;\n"
        "    S s1[2];\n"
        "    uint u0;\n"
        "};\n"
        "layout(binding = 1) buffer blockName1 {\n"
        "    uint u1[2];\n"
        "    float f1;\n"
        "} instanceName1[2];\n"
        "layout(binding = 2) buffer blockName2 {\n"
        "    uint u2;\n"
        "    float f2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(instanceName1[0].f1, s1[0].a);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "uniform vec4 color;\n"
        "out vec4 oColor;\n"
        "void main()\n"
        "{\n"
        "    oColor = color;\n"
        "}";

    GLProgram program;
    createGraphicsProgram(program, kVS, kFS, std::get<1>(GetParam()));

    GLuint index = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, "blockName0");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    GLchar name[64];
    GLsizei length;
    glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(10, length);
    EXPECT_EQ("blockName0", std::string(name));

    GLenum props[]         = {GL_ACTIVE_VARIABLES,
                              GL_BUFFER_BINDING,
                              GL_NUM_ACTIVE_VARIABLES,
                              GL_BUFFER_DATA_SIZE,
                              GL_NAME_LENGTH,
                              GL_REFERENCED_BY_VERTEX_SHADER,
                              GL_REFERENCED_BY_FRAGMENT_SHADER,
                              GL_REFERENCED_BY_COMPUTE_SHADER};
    GLsizei propCount      = static_cast<GLsizei>(ArraySize(props));
    constexpr int kBufSize = 256;
    GLint params[kBufSize];

    glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, index, propCount, props, kBufSize,
                           &length, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(13, length);
    EXPECT_LE(0, params[0]);   // active_variables s0.a
    EXPECT_LE(0, params[1]);   // active_variables s0.b
    EXPECT_LE(0, params[2]);   // active_variables v0
    EXPECT_LE(0, params[3]);   // active_variables s1[0].a
    EXPECT_LE(0, params[4]);   // active_variables s1[0].b
    EXPECT_LE(0, params[5]);   // active_variables u0
    EXPECT_EQ(0, params[6]);   // buffer_binding
    EXPECT_EQ(6, params[7]);   // num_active_variables
    EXPECT_LE(0, params[8]);   // buffer_data_size
    EXPECT_EQ(11, params[9]);  // name_length

    EXPECT_EQ(1, params[10]);  // referenced_by_vertex_shader
    EXPECT_EQ(0, params[11]);  // referenced_by_fragment_shader
    EXPECT_EQ(0, params[12]);  // referenced_by_compute_shader

    index = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, "blockName1");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, index, sizeof(name), &length, name);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(13, length);
    EXPECT_EQ("blockName1[0]", std::string(name));
}

// Tests querying the program resources of atomic counter buffers.
TEST_P(ProgramInterfaceTestES31, GetAtomicCounterProperties)
{
    constexpr char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(binding = 0) uniform atomic_uint acbase;
layout(binding = 0, offset = 8) uniform atomic_uint ac[1];
layout(binding = 0) uniform atomic_uint ac2;

void main()
{
    atomicCounterIncrement(acbase);
    atomicCounterIncrement(ac[0]);
    atomicCounterIncrement(ac2);
})";

    GLProgram program;
    createComputeProgram(program, kCSSource, std::get<1>(GetParam()));

    GLuint index = glGetProgramResourceIndex(program, GL_UNIFORM, "ac");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(GL_INVALID_INDEX, index);

    GLenum props[]    = {GL_ATOMIC_COUNTER_BUFFER_INDEX};
    GLsizei propCount = static_cast<GLsizei>(ArraySize(props));
    GLint atomicIndex;
    GLsizei length;

    glGetProgramResourceiv(program, GL_UNIFORM, index, propCount, props, 1, &length, &atomicIndex);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1, length);
    EXPECT_LE(0, atomicIndex);

    GLenum atomicProps[] = {GL_ACTIVE_VARIABLES,
                            GL_BUFFER_BINDING,
                            GL_NUM_ACTIVE_VARIABLES,
                            GL_BUFFER_DATA_SIZE,
                            GL_REFERENCED_BY_VERTEX_SHADER,
                            GL_REFERENCED_BY_FRAGMENT_SHADER,
                            GL_REFERENCED_BY_COMPUTE_SHADER};

    GLsizei atomicPropsCount = static_cast<GLsizei>(ArraySize(atomicProps));
    constexpr int kBufSize   = 256;
    GLint params[kBufSize];
    GLsizei length2;

    glGetProgramResourceiv(program, GL_ATOMIC_COUNTER_BUFFER, atomicIndex, atomicPropsCount,
                           atomicProps, kBufSize, &length2, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(9, length2);

    EXPECT_LE(0, params[0]);   // active_variables acbase
    EXPECT_LE(0, params[1]);   // active_variables ac[1]
    EXPECT_LE(0, params[2]);   // active_variables ac2
    EXPECT_EQ(0, params[3]);   // buffer_binding
    EXPECT_EQ(3, params[4]);   // num_active_variables
    EXPECT_EQ(16, params[5]);  // buffer_data_size

    EXPECT_EQ(0, params[6]);  // referenced_by_vertex_shader
    EXPECT_EQ(0, params[7]);  // referenced_by_fragment_shader
    EXPECT_EQ(1, params[8]);  // referenced_by_compute_shader
}

// Tests transform feedback varying qeury works correctly.
TEST_P(ProgramInterfaceTestES31, QueryTransformFeedbackVarying)
{
    constexpr char kVS[] = R"(#version 310 es
in vec3 position;
out float outSingleType;
out vec2 outWholeArray[2];
out vec3 outArrayElements[16];
void main() {
    outSingleType = 0.0;
    outWholeArray[0] = vec2(position);
    outArrayElements[7] = vec3(0, 0, 0);
    outArrayElements[15] = position;
    gl_Position = vec4(position, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
out vec4 color;
in float outSingleType;
in vec2 outWholeArray[2];
in vec3 outArrayElements[16];
void main() {
    color = vec4(0);
})";

    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("outArrayElements[7]");
    tfVaryings.push_back("outArrayElements[15]");
    tfVaryings.push_back("outSingleType");
    tfVaryings.push_back("outWholeArray");

    GLuint program =
        CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_NE(0u, program);

    GLint num;
    glGetProgramInterfaceiv(program, GL_TRANSFORM_FEEDBACK_VARYING, GL_ACTIVE_RESOURCES, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(4, num);

    glGetProgramInterfaceiv(program, GL_TRANSFORM_FEEDBACK_VARYING, GL_MAX_NAME_LENGTH, &num);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(21, num);  // outArrayElements[15]

    // GLES 3.10, Page 77:
    // For TRANSFORM_FEEDBACK_VARYING, the active resource list will use the variable order
    // specified in the most recent call to TransformFeedbackVaryings before the last call to
    // LinkProgram.
    GLuint index =
        glGetProgramResourceIndex(program, GL_TRANSFORM_FEEDBACK_VARYING, "outArrayElements[7]");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0u, index);
    index =
        glGetProgramResourceIndex(program, GL_TRANSFORM_FEEDBACK_VARYING, "outArrayElements[15]");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1u, index);
    index = glGetProgramResourceIndex(program, GL_TRANSFORM_FEEDBACK_VARYING, "outSingleType");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(2u, index);
    index = glGetProgramResourceIndex(program, GL_TRANSFORM_FEEDBACK_VARYING, "outWholeArray");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(3u, index);

    // GLES 3.10, Page 80:
    // For TRANSFORM_FEEDBACK_VARYING resources, name must match one of the variables to be captured
    // as specified by a previous call to TransformFeedbackVaryings. Otherwise, INVALID_INDEX is
    // returned.
    // If name does not match a resource as described above, the value INVALID_INDEX is returned,
    // but no GL error is generated.
    index = glGetProgramResourceIndex(program, GL_TRANSFORM_FEEDBACK_VARYING, "outWholeArray[0]");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_INVALID_INDEX, index);

    GLenum props[]    = {GL_TYPE, GL_ARRAY_SIZE, GL_NAME_LENGTH};
    GLsizei propCount = static_cast<GLsizei>(ArraySize(props));
    GLint params[ArraySize(props)];
    GLsizei length = 0;
    // Query properties of 'outArrayElements[15]'.
    glGetProgramResourceiv(program, GL_TRANSFORM_FEEDBACK_VARYING, 1, propCount, props, propCount,
                           &length, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(GL_FLOAT_VEC3, params[0]);  // type
    EXPECT_EQ(1, params[1]);              // array_size
    EXPECT_EQ(21, params[2]);             // name_length

    // Query properties of 'outWholeArray'.
    glGetProgramResourceiv(program, GL_TRANSFORM_FEEDBACK_VARYING, 3, propCount, props, propCount,
                           &length, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(propCount, length);
    EXPECT_EQ(GL_FLOAT_VEC2, params[0]);  // type
    EXPECT_EQ(2, params[1]);              // array_size
    EXPECT_EQ(14, params[2]);             // name_length

    glDeleteProgram(program);
}

// Regression test for crash report in http://anglebug.com/42264603.
TEST_P(ProgramInterfaceTestES31, ReloadFromCacheShouldNotCrash)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_multi_draw"));

    // TODO(jiajia.qin@intel.com): Don't skip this test once non-simple SSBO sentences are supported
    // on d3d backend. http://anglebug.com/40644618
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr char kVS[] = R"(#version 310 es
#extension GL_ANGLE_multi_draw : require
precision highp int;
precision highp float;
layout(std140) buffer;
struct TransformInfo
{
    mat4 mvp;
};
layout(binding = 0) buffer pe_transforms
{
    TransformInfo transforms[];
};
out vec2 texCoord;
uniform int pe_base_draw_id;
layout(location = 0) in vec3 pe_vertex;
layout(location = 1) in vec3 pe_normal;
layout(location = 2) in vec2 pe_tex_coord;
void main()
{
    vec4 v = vec4(pe_vertex, 1.0);
    texCoord = pe_tex_coord;
    gl_Position = transforms[(gl_DrawID + pe_base_draw_id)].mvp * v;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_ANGLE_multi_draw : require
precision highp int;
precision highp float;
layout(std140) buffer;
in vec2 texCoord;
layout(binding = 0) uniform sampler2D pe_tex_main;
out vec4 pe_frag_color;
void main()
{
    vec4 u = texture(pe_tex_main, texCoord);
    if(u.a < 0.05)
        discard;
    pe_frag_color = u;
}
)";

    GLProgram program;
    createGraphicsProgram(program, kVS, kFS, std::get<1>(GetParam()));
    EXPECT_GL_NO_ERROR();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProgramInterfaceTestES31);
ANGLE_INSTANTIATE_TEST_COMBINE_1(ProgramInterfaceTestES31,
                                 ProgramInterfaceTestPrint,
                                 testing::Bool(),
                                 ANGLE_ALL_TEST_PLATFORMS_ES31);

}  // anonymous namespace
