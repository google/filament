//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// draw_call_perf_utils.cpp:
//   Common utilities for performance tests that need to do a large amount of draw calls.
//

#include "draw_call_perf_utils.h"

#include <vector>

#include "util/shader_utils.h"

namespace
{
constexpr char kSimpleScaleAndOffsetVS[] = R"(attribute vec2 vPosition;
uniform float uScale;
uniform float uOffset;
void main()
{
    gl_Position = vec4(vPosition * vec2(uScale) + vec2(uOffset), 0, 1);
})";

constexpr char kSimpleDrawVS[] = R"(attribute vec2 vPosition;
const float scale = 0.5;
const float offset = -0.5;

void main()
{
    gl_Position = vec4(vPosition * vec2(scale) + vec2(offset), 0, 1);
})";

constexpr char kSimpleTexCoordVS[] = R"(attribute vec2 vPosition;
varying vec2 texCoord;
void main()
{
    gl_Position = vec4(vPosition, 0, 1);
    texCoord = vPosition * 0.5 + vec2(0.5);
})";

constexpr char kSimpleFS[] = R"(precision mediump float;
void main()
{
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

constexpr char kSimpleTextureFS[] = R"(precision mediump float;
varying vec2 texCoord;
uniform sampler2D tex;
void main()
{
    gl_FragColor = texture2D(tex, texCoord);
})";

constexpr char kDoubleTextureFS[] = R"(precision mediump float;
varying vec2 texCoord;
uniform sampler2D tex1;
uniform sampler2D tex2;
void main()
{
    gl_FragColor = texture2D(tex1, texCoord) + texture2D(tex2, texCoord);
})";

constexpr char kEightTextureFS[] = R"(precision mediump float;
varying vec2 texCoord;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;
uniform sampler2D tex5;
uniform sampler2D tex6;
uniform sampler2D tex7;
uniform sampler2D tex8;
void main()
{
    gl_FragColor = texture2D(tex1, texCoord) + texture2D(tex2, texCoord) +
                   texture2D(tex3, texCoord) + texture2D(tex4, texCoord) +
                   texture2D(tex5, texCoord) + texture2D(tex6, texCoord) +
                   texture2D(tex7, texCoord) + texture2D(tex8, texCoord);
})";

void Generate2DTriangleData(size_t numTris, std::vector<float> *floatData)
{
    for (size_t triIndex = 0; triIndex < numTris; ++triIndex)
    {
        floatData->push_back(1.0f);
        floatData->push_back(2.0f);

        floatData->push_back(0.0f);
        floatData->push_back(0.0f);

        floatData->push_back(2.0f);
        floatData->push_back(0.0f);
    }
}

}  // anonymous namespace

GLuint SetupSimpleScaleAndOffsetProgram()
{
    GLuint program = CompileProgram(kSimpleScaleAndOffsetVS, kSimpleFS);
    if (program == 0u)
    {
        return program;
    }

    // Use the program object
    glUseProgram(program);

    GLfloat scale  = 0.5f;
    GLfloat offset = -0.5f;

    glUniform1f(glGetUniformLocation(program, "uScale"), scale);
    glUniform1f(glGetUniformLocation(program, "uOffset"), offset);
    return program;
}

GLuint SetupSimpleDrawProgram()
{
    GLuint program = CompileProgram(kSimpleDrawVS, kSimpleFS);
    if (program == 0u)
    {
        return program;
    }

    // Use the program object
    glUseProgram(program);

    return program;
}

GLuint SetupSimpleTextureProgram()
{
    GLuint program = CompileProgram(kSimpleTexCoordVS, kSimpleTextureFS);
    if (program == 0u)
    {
        return program;
    }

    // Use the program object
    glUseProgram(program);

    return program;
}

GLuint SetupDoubleTextureProgram()
{
    GLuint program = CompileProgram(kSimpleTexCoordVS, kDoubleTextureFS);
    if (program == 0u)
    {
        return program;
    }

    // Use the program object
    glUseProgram(program);

    return program;
}

GLuint SetupEightTextureProgram()
{
    GLuint program = CompileProgram(kSimpleTexCoordVS, kEightTextureFS);
    if (program == 0u)
    {
        return program;
    }

    // Use the program object
    glUseProgram(program);

    return program;
}

GLuint Create2DTriangleBuffer(size_t numTris, GLenum usage)
{
    GLuint buffer = 0u;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    std::vector<GLfloat> floatData;
    Generate2DTriangleData(numTris, &floatData);

    // To avoid generating GL errors when testing validation-only with zero triangles.
    if (floatData.empty())
    {
        floatData.push_back(0.0f);
    }

    glBufferData(GL_ARRAY_BUFFER, floatData.size() * sizeof(GLfloat), &floatData[0], usage);

    return buffer;
}

void CreateColorFBO(GLsizei width, GLsizei height, GLuint *fbo, GLuint *texture)
{
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);
}
