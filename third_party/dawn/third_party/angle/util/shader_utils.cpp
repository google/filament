//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "util/shader_utils.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "common/utilities.h"
#include "util/test_utils.h"

namespace
{
GLuint CompileProgramInternal(const char *vsSource,
                              const char *tcsSource,
                              const char *tesSource,
                              const char *gsSource,
                              const char *fsSource,
                              const std::function<void(GLuint)> &preLinkCallback)
{
    GLuint program = glCreateProgram();

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    if (vs == 0 || fs == 0)
    {
        glDeleteShader(fs);
        glDeleteShader(vs);
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    GLuint tcs = 0;
    GLuint tes = 0;
    GLuint gs  = 0;

    if (strlen(tcsSource) > 0)
    {
        tcs = CompileShader(GL_TESS_CONTROL_SHADER_EXT, tcsSource);
        if (tcs == 0)
        {
            glDeleteShader(vs);
            glDeleteShader(fs);
            glDeleteProgram(program);
            return 0;
        }

        glAttachShader(program, tcs);
        glDeleteShader(tcs);
    }

    if (strlen(tesSource) > 0)
    {
        tes = CompileShader(GL_TESS_EVALUATION_SHADER_EXT, tesSource);
        if (tes == 0)
        {
            glDeleteShader(vs);
            glDeleteShader(fs);
            glDeleteShader(tcs);
            glDeleteProgram(program);
            return 0;
        }

        glAttachShader(program, tes);
        glDeleteShader(tes);
    }

    if (strlen(gsSource) > 0)
    {
        gs = CompileShader(GL_GEOMETRY_SHADER_EXT, gsSource);
        if (gs == 0)
        {
            glDeleteShader(vs);
            glDeleteShader(fs);
            glDeleteShader(tcs);
            glDeleteShader(tes);
            glDeleteProgram(program);
            return 0;
        }

        glAttachShader(program, gs);
        glDeleteShader(gs);
    }

    if (preLinkCallback)
    {
        preLinkCallback(program);
    }

    glLinkProgram(program);

    return CheckLinkStatusAndReturnProgram(program, true);
}

const void *gCallbackChainUserParam;

void KHRONOS_APIENTRY DebugMessageCallback(GLenum source,
                                           GLenum type,
                                           GLuint id,
                                           GLenum severity,
                                           GLsizei length,
                                           const GLchar *message,
                                           const void *userParam)
{
    std::string sourceText   = gl::GetDebugMessageSourceString(source);
    std::string typeText     = gl::GetDebugMessageTypeString(type);
    std::string severityText = gl::GetDebugMessageSeverityString(severity);
    std::cerr << sourceText << ", " << typeText << ", " << severityText << ": " << message << "\n";

    GLDEBUGPROC callbackChain = reinterpret_cast<GLDEBUGPROC>(const_cast<void *>(userParam));
    if (callbackChain)
    {
        callbackChain(source, type, id, severity, length, message, gCallbackChainUserParam);
    }
}

void GetPerfCounterValue(const CounterNameToIndexMap &counterIndexMap,
                         std::vector<angle::PerfMonitorTriplet> &triplets,
                         const char *name,
                         GLuint64 *counterOut)
{
    auto iter = counterIndexMap.find(name);
    ASSERT(iter != counterIndexMap.end());
    GLuint counterIndex = iter->second;

    for (const angle::PerfMonitorTriplet &triplet : triplets)
    {
        ASSERT(triplet.group == 0);
        if (triplet.counter == counterIndex)
        {
            *counterOut = triplet.value;
            return;
        }
    }

    // Additional logs for b/382094011
    std::cerr << "GetPerfCounterValue missing counter: " << name << "; index: " << counterIndex
              << "; triplets: " << std::endl;
    for (const angle::PerfMonitorTriplet &triplet : triplets)
    {
        std::cerr << triplet.counter << " " << triplet.value << std::endl;
    }

    UNREACHABLE();
}
}  // namespace

GLuint CompileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);

    const char *sourceArray[1] = {source};
    glShaderSource(shader, 1, sourceArray, nullptr);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    if (compileResult == 0)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        // Info log length includes the null terminator, so 1 means that the info log is an empty
        // string.
        if (infoLogLength > 1)
        {
            std::vector<GLchar> infoLog(infoLogLength);
            glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.size()), nullptr, &infoLog[0]);
            std::cerr << "shader compilation failed: " << &infoLog[0];
        }
        else
        {
            std::cerr << "shader compilation failed. <Empty log message>";
        }

        std::cerr << std::endl;

        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

GLuint CompileShaderFromFile(GLenum type, const std::string &sourcePath)
{
    std::string source;
    if (!angle::ReadEntireFileToString(sourcePath.c_str(), &source))
    {
        std::cerr << "Error reading shader file: " << sourcePath << "\n";
        return 0;
    }

    return CompileShader(type, source.c_str());
}

GLuint CheckLinkStatusAndReturnProgram(GLuint program, bool outputErrorMessages)
{
    if (glGetError() != GL_NO_ERROR)
        return 0;

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == 0)
    {
        if (outputErrorMessages)
        {
            GLint infoLogLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

            // Info log length includes the null terminator, so 1 means that the info log is an
            // empty string.
            if (infoLogLength > 1)
            {
                std::vector<GLchar> infoLog(infoLogLength);
                glGetProgramInfoLog(program, static_cast<GLsizei>(infoLog.size()), nullptr,
                                    &infoLog[0]);

                std::cerr << "program link failed: " << &infoLog[0];
            }
            else
            {
                std::cerr << "program link failed. <Empty log message>";
            }
        }

        glDeleteProgram(program);
        return 0;
    }

    return program;
}

GLuint GetProgramShader(GLuint program, GLint requestedType)
{
    static constexpr GLsizei kMaxShaderCount = 16;
    GLuint attachedShaders[kMaxShaderCount]  = {0u};
    GLsizei count                            = 0;
    glGetAttachedShaders(program, kMaxShaderCount, &count, attachedShaders);
    for (int i = 0; i < count; ++i)
    {
        GLint type = 0;
        glGetShaderiv(attachedShaders[i], GL_SHADER_TYPE, &type);
        if (type == requestedType)
        {
            return attachedShaders[i];
        }
    }

    return 0;
}

GLuint CompileProgramWithTransformFeedback(
    const char *vsSource,
    const char *fsSource,
    const std::vector<std::string> &transformFeedbackVaryings,
    GLenum bufferMode)
{
    auto preLink = [&](GLuint program) {
        if (transformFeedbackVaryings.size() > 0)
        {
            std::vector<const char *> constCharTFVaryings;

            for (const std::string &transformFeedbackVarying : transformFeedbackVaryings)
            {
                constCharTFVaryings.push_back(transformFeedbackVarying.c_str());
            }

            glTransformFeedbackVaryings(program,
                                        static_cast<GLsizei>(transformFeedbackVaryings.size()),
                                        &constCharTFVaryings[0], bufferMode);
        }
    };

    return CompileProgramInternal(vsSource, "", "", "", fsSource, preLink);
}

GLuint CompileProgram(const char *vsSource, const char *fsSource)
{
    return CompileProgramInternal(vsSource, "", "", "", fsSource, nullptr);
}

GLuint CompileProgram(const char *vsSource,
                      const char *fsSource,
                      const std::function<void(GLuint)> &preLinkCallback)
{
    return CompileProgramInternal(vsSource, "", "", "", fsSource, preLinkCallback);
}

GLuint CompileProgramWithGS(const char *vsSource, const char *gsSource, const char *fsSource)
{
    return CompileProgramInternal(vsSource, "", "", gsSource, fsSource, nullptr);
}

GLuint CompileProgramWithTESS(const char *vsSource,
                              const char *tcsSource,
                              const char *tesSource,
                              const char *fsSource)
{
    return CompileProgramInternal(vsSource, tcsSource, tesSource, "", fsSource, nullptr);
}

GLuint CompileProgramFromFiles(const std::string &vsPath, const std::string &fsPath)
{
    std::string vsSource;
    if (!angle::ReadEntireFileToString(vsPath.c_str(), &vsSource))
    {
        std::cerr << "Error reading shader: " << vsPath << "\n";
        return 0;
    }

    std::string fsSource;
    if (!angle::ReadEntireFileToString(fsPath.c_str(), &fsSource))
    {
        std::cerr << "Error reading shader: " << fsPath << "\n";
        return 0;
    }

    return CompileProgram(vsSource.c_str(), fsSource.c_str());
}

GLuint CompileComputeProgram(const char *csSource, bool outputErrorMessages)
{
    GLuint program = glCreateProgram();

    GLuint cs = CompileShader(GL_COMPUTE_SHADER, csSource);
    if (cs == 0)
    {
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, cs);

    glLinkProgram(program);

    return CheckLinkStatusAndReturnProgram(program, outputErrorMessages);
}

GLuint LoadBinaryProgramOES(const std::vector<uint8_t> &binary, GLenum binaryFormat)
{
    GLuint program = glCreateProgram();
    glProgramBinaryOES(program, binaryFormat, binary.data(), static_cast<GLint>(binary.size()));
    return CheckLinkStatusAndReturnProgram(program, true);
}

GLuint LoadBinaryProgramES3(const std::vector<uint8_t> &binary, GLenum binaryFormat)
{
    GLuint program = glCreateProgram();
    glProgramBinary(program, binaryFormat, binary.data(), static_cast<GLint>(binary.size()));
    return CheckLinkStatusAndReturnProgram(program, true);
}

bool LinkAttachedProgram(GLuint program)
{
    glLinkProgram(program);
    return (CheckLinkStatusAndReturnProgram(program, true) != 0);
}

void EnableDebugCallback(GLDEBUGPROC callbackChain, const void *userParam)
{
    gCallbackChainUserParam = userParam;

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    // Enable medium and high priority messages.
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr,
                             GL_TRUE);
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr,
                             GL_TRUE);
    // Disable low and notification priority messages.
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr,
                             GL_FALSE);
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
                             GL_FALSE);
    // Disable performance messages to reduce spam.
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, 0, nullptr,
                             GL_FALSE);
    glDebugMessageCallbackKHR(DebugMessageCallback, reinterpret_cast<const void *>(callbackChain));
}

CounterNameToIndexMap BuildCounterNameToIndexMap()
{
    GLint numCounters = 0;
    glGetPerfMonitorCountersAMD(0, &numCounters, nullptr, 0, nullptr);
    if (glGetError() != GL_NO_ERROR)
    {
        std::cerr << "glGetPerfMonitorCountersAMD failed (count)" << std::endl;
        return {};
    }

    std::vector<GLuint> counterIndexes(numCounters, 0);
    glGetPerfMonitorCountersAMD(0, nullptr, nullptr, numCounters, counterIndexes.data());
    if (glGetError() != GL_NO_ERROR)
    {
        std::cerr << "glGetPerfMonitorCountersAMD failed (data)" << std::endl;
        return {};
    }

    CounterNameToIndexMap indexMap;

    for (GLuint counterIndex : counterIndexes)
    {
        static constexpr size_t kBufSize = 1000;
        char buffer[kBufSize]            = {};
        glGetPerfMonitorCounterStringAMD(0, counterIndex, kBufSize, nullptr, buffer);
        if (glGetError() != GL_NO_ERROR)
        {
            std::cerr << "glGetPerfMonitorCounterStringAMD failed" << std::endl;
            return {};
        }

        indexMap[buffer] = counterIndex;
    }

    return indexMap;
}

std::vector<angle::PerfMonitorTriplet> GetPerfMonitorTriplets()
{
    GLuint resultSize = 0;
    glGetPerfMonitorCounterDataAMD(0, GL_PERFMON_RESULT_SIZE_AMD, sizeof(GLuint), &resultSize,
                                   nullptr);
    if (glGetError() != GL_NO_ERROR || resultSize == 0)
    {
        std::cerr << "glGetPerfMonitorCounterDataAMD failed (count)" << std::endl;
        return {};
    }

    std::vector<angle::PerfMonitorTriplet> perfResults(resultSize /
                                                       sizeof(angle::PerfMonitorTriplet));
    GLint bytesWritten = 0;
    glGetPerfMonitorCounterDataAMD(
        0, GL_PERFMON_RESULT_AMD, static_cast<GLsizei>(perfResults.size() * sizeof(perfResults[0])),
        &perfResults.data()->group, &bytesWritten);

    if (glGetError() != GL_NO_ERROR)
    {
        std::cerr << "glGetPerfMonitorCounterDataAMD failed (data)" << std::endl;
        return {};
    }
    ASSERT(static_cast<GLuint>(bytesWritten) == resultSize);

    return perfResults;
}

angle::VulkanPerfCounters GetPerfCounters(const CounterNameToIndexMap &indexMap)
{
    std::vector<angle::PerfMonitorTriplet> perfResults = GetPerfMonitorTriplets();

    angle::VulkanPerfCounters counters;

#define ANGLE_UNPACK_PERF_COUNTER(COUNTER) \
    GetPerfCounterValue(indexMap, perfResults, #COUNTER, &counters.COUNTER);

    ANGLE_VK_PERF_COUNTERS_X(ANGLE_UNPACK_PERF_COUNTER)

#undef ANGLE_UNPACK_PERF_COUNTER

    return counters;
}

CounterNameToValueMap BuildCounterNameToValueMap()
{
    CounterNameToIndexMap indexMap                     = BuildCounterNameToIndexMap();
    std::vector<angle::PerfMonitorTriplet> perfResults = GetPerfMonitorTriplets();

    CounterNameToValueMap valueMap;

    for (const auto &iter : indexMap)
    {
        const std::string &name = iter.first;
        GLuint index            = iter.second;

        valueMap[name] = perfResults[index].value;
    }

    return valueMap;
}

namespace angle
{

namespace essl1_shaders
{

const char *PositionAttrib()
{
    return "a_position";
}
const char *ColorUniform()
{
    return "u_color";
}

const char *Texture2DUniform()
{
    return "u_tex2D";
}

namespace vs
{

// A shader that sets gl_Position to zero.
const char *Zero()
{
    return R"(void main()
{
    gl_Position = vec4(0);
})";
}

// A shader that sets gl_Position to attribute a_position.
const char *Simple()
{
    return R"(precision highp float;
attribute vec4 a_position;

void main()
{
    gl_Position = a_position;
})";
}

// A shader that sets gl_Position to attribute a_position, and sets gl_PointSize to 1.
const char *SimpleForPoints()
{
    return R"(precision highp float;
attribute vec4 a_position;

void main()
{
    gl_Position = a_position;
    gl_PointSize = 1.0;
})";
}

// A shader that simply passes through attribute a_position, setting it to gl_Position and varying
// v_position.
const char *Passthrough()
{
    return R"(precision highp float;
attribute vec4 a_position;
varying vec4 v_position;

void main()
{
    gl_Position = a_position;
    v_position = a_position;
})";
}

// A shader that simply passes through attribute a_position, setting it to gl_Position and varying
// texcoord.
const char *Texture2D()
{
    return R"(precision highp float;
attribute vec4 a_position;
varying vec2 v_texCoord;

void main()
{
    gl_Position = a_position;
    v_texCoord = a_position.xy * 0.5 + vec2(0.5);
})";
}

const char *Texture2DArray()
{
    return R"(#version 300 es
out vec2 v_texCoord;
in vec4 a_position;
void main()
{
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
    v_texCoord = (a_position.xy * 0.5) + 0.5;
})";
}

}  // namespace vs

namespace fs
{

// A shader that renders a simple checker pattern of red and green. X axis and y axis separate the
// different colors. Needs varying v_position.
const char *Checkered()
{
    return R"(precision highp float;
varying vec4 v_position;

void main()
{
    bool isLeft = v_position.x < 0.0;
    bool isTop = v_position.y < 0.0;
    if (isLeft)
    {
        if (isTop)
        {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
        else
        {
            gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        }
    }
    else
    {
        if (isTop)
        {
            gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
        }
        else
        {
            gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
        }
    }
})";
}

// A shader that fills with color taken from uniform named "color".
const char *UniformColor()
{
    return R"(uniform mediump vec4 u_color;
void main(void)
{
    gl_FragColor = u_color;
})";
}

// A shader that fills with 100% opaque red.
const char *Red()
{
    return R"(precision mediump float;

void main()
{
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";
}

// A shader that fills with 100% opaque green.
const char *Green()
{
    return R"(precision mediump float;

void main()
{
    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
})";
}

// A shader that fills with 100% opaque blue.
const char *Blue()
{
    return R"(precision mediump float;

void main()
{
    gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
})";
}

// A shader that samples the texture.
const char *Texture2D()
{
    return R"(precision mediump float;
uniform sampler2D u_tex2D;
varying vec2 v_texCoord;

void main()
{
    gl_FragColor = texture2D(u_tex2D, v_texCoord);
})";
}

const char *Texture2DArray()
{
    return R"(#version 300 es
precision highp float;
uniform highp sampler2DArray tex2DArray;
uniform int slice;
in vec2 v_texCoord;
out vec4 fragColor;
void main()
{
    fragColor = texture(tex2DArray, vec3(v_texCoord, float(slice)));
})";
}

}  // namespace fs
}  // namespace essl1_shaders

namespace essl3_shaders
{

const char *PositionAttrib()
{
    return "a_position";
}
const char *Texture2DUniform()
{
    return "u_tex2D";
}
const char *LodUniform()
{
    return "u_lod";
}

namespace vs
{

// A shader that sets gl_Position to zero.
const char *Zero()
{
    return R"(#version 300 es
void main()
{
    gl_Position = vec4(0);
})";
}

// A shader that sets gl_Position to attribute a_position.
const char *Simple()
{
    return R"(#version 300 es
in vec4 a_position;
void main()
{
    gl_Position = a_position;
})";
}

// A shader that sets gl_Position to attribute a_position, and sets gl_PointSize to 1.
const char *SimpleForPoints()
{
    return R"(#version 300 es
in vec4 a_position;
void main()
{
    gl_Position = a_position;
    gl_PointSize = 1.0;
})";
}

// A shader that simply passes through attribute a_position, setting it to gl_Position and varying
// v_position.
const char *Passthrough()
{
    return R"(#version 300 es
in vec4 a_position;
out vec4 v_position;
void main()
{
    gl_Position = a_position;
    v_position = a_position;
})";
}

// A shader that simply passes through attribute a_position, setting it to gl_Position and varying
// texcoord.
const char *Texture2DLod()
{
    return R"(#version 300 es
in vec4 a_position;
out vec2 v_texCoord;

void main()
{
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
    v_texCoord = a_position.xy * 0.5 + vec2(0.5);
})";
}

}  // namespace vs

namespace fs
{

// A shader that fills with 100% opaque red.
const char *Red()
{
    return R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";
}

// A shader that fills with 100% opaque green.
const char *Green()
{
    return R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
})";
}

// A shader that fills with 100% opaque blue.
const char *Blue()
{
    return R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
})";
}

// A shader that samples the texture at a given lod.
const char *Texture2DLod()
{
    return R"(#version 300 es
precision mediump float;
uniform sampler2D u_tex2D;
uniform float u_lod;
in vec2 v_texCoord;
out vec4 my_FragColor;

void main()
{
    my_FragColor = textureLod(u_tex2D, v_texCoord, u_lod);
})";
}

}  // namespace fs
}  // namespace essl3_shaders

namespace essl31_shaders
{

const char *PositionAttrib()
{
    return "a_position";
}

namespace vs
{

// A shader that sets gl_Position to zero.
const char *Zero()
{
    return R"(#version 310 es
void main()
{
    gl_Position = vec4(0);
})";
}

// A shader that sets gl_Position to attribute a_position.
const char *Simple()
{
    return R"(#version 310 es
in vec4 a_position;
void main()
{
    gl_Position = a_position;
})";
}

// A shader that simply passes through attribute a_position, setting it to gl_Position and varying
// v_position.
const char *Passthrough()
{
    return R"(#version 310 es
in vec4 a_position;
out vec4 v_position;
void main()
{
    gl_Position = a_position;
    v_position = a_position;
})";
}

}  // namespace vs

namespace fs
{

// A shader that fills with 100% opaque red.
const char *Red()
{
    return R"(#version 310 es
precision highp float;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";
}

// A shader that fills with 100% opaque green.
const char *Green()
{
    return R"(#version 310 es
precision highp float;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
})";
}

// A shader that renders a simple gradient of red to green. Needs varying v_position.
const char *RedGreenGradient()
{
    return R"(#version 310 es
precision highp float;
in vec4 v_position;
out vec4 my_FragColor;

void main()
{
    my_FragColor = vec4(v_position.xy * 0.5 + vec2(0.5), 0.0, 1.0);
})";
}

}  // namespace fs
}  // namespace essl31_shaders
}  // namespace angle
