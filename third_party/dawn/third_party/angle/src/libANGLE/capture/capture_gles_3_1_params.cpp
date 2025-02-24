//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// capture_gles31_params.cpp:
//   Pointer parameter capture functions for the OpenGL ES 3.1 entry points.

#include "libANGLE/capture/capture_gles_3_1_autogen.h"

using namespace angle;

namespace gl
{

void CaptureCreateShaderProgramv_strings(const State &glState,
                                         bool isCallValid,
                                         ShaderType typePacked,
                                         GLsizei count,
                                         const GLchar *const *strings,
                                         ParamCapture *paramCapture)
{
    CaptureShaderStrings(count, strings, nullptr, paramCapture);
}

void CaptureDeleteProgramPipelines_pipelinesPacked(const State &glState,
                                                   bool isCallValid,
                                                   GLsizei n,
                                                   const ProgramPipelineID *pipelines,
                                                   ParamCapture *paramCapture)
{
    CaptureArray(pipelines, n, paramCapture);
}

void CaptureDrawArraysIndirect_indirect(const State &glState,
                                        bool isCallValid,
                                        PrimitiveMode modePacked,
                                        const void *indirect,
                                        ParamCapture *paramCapture)
{
    // DrawArraysIndirect requires that all data sourced for the command,
    // including the DrawArraysIndirectCommand structure, be in buffer objects,
    // and may not be called when the default vertex array object is bound.
    // Indirect pointer is automatically captured in capture_gles_3_1_autogen.cpp
    assert(!isCallValid || glState.getTargetBuffer(gl::BufferBinding::DrawIndirect));
}

void CaptureDrawElementsIndirect_indirect(const State &glState,
                                          bool isCallValid,
                                          PrimitiveMode modePacked,
                                          DrawElementsType typePacked,
                                          const void *indirect,
                                          ParamCapture *paramCapture)
{
    // DrawElementsIndirect requires that all data sourced for the command,
    // including the DrawElementsIndirectCommand structure, be in buffer objects,
    // and may not be called when the default vertex array object is bound
    // Indirect pointer is automatically captured in capture_gles_3_1_autogen.cpp
    assert(!isCallValid || glState.getTargetBuffer(gl::BufferBinding::DrawIndirect));
}

void CaptureGenProgramPipelines_pipelinesPacked(const State &glState,
                                                bool isCallValid,
                                                GLsizei n,
                                                ProgramPipelineID *pipelines,
                                                ParamCapture *paramCapture)
{
    CaptureGenHandles(n, pipelines, paramCapture);
}

void CaptureGetBooleani_v_data(const State &glState,
                               bool isCallValid,
                               GLenum target,
                               GLuint index,
                               GLboolean *data,
                               ParamCapture *paramCapture)
{
    CaptureMemory(data, sizeof(GLboolean), paramCapture);
}

void CaptureGetFramebufferParameteriv_params(const State &glState,
                                             bool isCallValid,
                                             GLenum target,
                                             GLenum pname,
                                             GLint *params,
                                             ParamCapture *paramCapture)
{
    // All glGetFramebufferParameteriv queries write back one single value.
    paramCapture->readBufferSizeBytes = sizeof(GLint);
}

void CaptureGetMultisamplefv_val(const State &glState,
                                 bool isCallValid,
                                 GLenum pname,
                                 GLuint index,
                                 GLfloat *val,
                                 ParamCapture *paramCapture)
{
    // GL_SAMPLE_POSITION: 2 floats
    paramCapture->readBufferSizeBytes = sizeof(GLfloat) * 2;
}

void CaptureGetProgramInterfaceiv_params(const State &glState,
                                         bool isCallValid,
                                         ShaderProgramID program,
                                         GLenum programInterface,
                                         GLenum pname,
                                         GLint *params,
                                         ParamCapture *paramCapture)
{
    paramCapture->readBufferSizeBytes = sizeof(GLint);
}

void CaptureGetProgramPipelineInfoLog_length(const State &glState,
                                             bool isCallValid,
                                             ProgramPipelineID pipeline,
                                             GLsizei bufSize,
                                             GLsizei *length,
                                             GLchar *infoLog,
                                             ParamCapture *paramCapture)
{
    if (length)
    {
        CaptureMemory(length, sizeof(GLsizei), paramCapture);
    }
}

void CaptureGetProgramPipelineInfoLog_infoLog(const State &glState,
                                              bool isCallValid,
                                              ProgramPipelineID pipeline,
                                              GLsizei bufSize,
                                              GLsizei *length,
                                              GLchar *infoLog,
                                              ParamCapture *paramCapture)
{
    if (bufSize > 0)
    {
        ASSERT(infoLog);
        if (length)
        {
            CaptureArray(infoLog, *length, paramCapture);
        }
        else
        {
            CaptureString(infoLog, paramCapture);
        }
    }
}

void CaptureGetProgramPipelineiv_params(const State &glState,
                                        bool isCallValid,
                                        ProgramPipelineID pipeline,
                                        GLenum pname,
                                        GLint *params,
                                        ParamCapture *paramCapture)
{
    CaptureMemory(params, sizeof(GLint), paramCapture);
}

void CaptureGetProgramResourceIndex_name(const State &glState,
                                         bool isCallValid,
                                         ShaderProgramID program,
                                         GLenum programInterface,
                                         const GLchar *name,
                                         ParamCapture *paramCapture)
{
    CaptureString(name, paramCapture);
}

void CaptureGetProgramResourceLocation_name(const State &glState,
                                            bool isCallValid,
                                            ShaderProgramID program,
                                            GLenum programInterface,
                                            const GLchar *name,
                                            ParamCapture *paramCapture)
{
    CaptureString(name, paramCapture);
}

void CaptureGetProgramResourceName_length(const State &glState,
                                          bool isCallValid,
                                          ShaderProgramID program,
                                          GLenum programInterface,
                                          GLuint index,
                                          GLsizei bufSize,
                                          GLsizei *length,
                                          GLchar *name,
                                          ParamCapture *paramCapture)
{
    paramCapture->readBufferSizeBytes = sizeof(GLsizei);
}

void CaptureGetProgramResourceName_name(const State &glState,
                                        bool isCallValid,
                                        ShaderProgramID program,
                                        GLenum programInterface,
                                        GLuint index,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLchar *name,
                                        ParamCapture *paramCapture)
{
    CaptureString(name, paramCapture);
}

void CaptureGetProgramResourceiv_props(const State &glState,
                                       bool isCallValid,
                                       ShaderProgramID program,
                                       GLenum programInterface,
                                       GLuint index,
                                       GLsizei propCount,
                                       const GLenum *props,
                                       GLsizei bufSize,
                                       GLsizei *length,
                                       GLint *params,
                                       ParamCapture *paramCapture)
{
    CaptureMemory(props, sizeof(GLenum) * propCount, paramCapture);
}

void CaptureGetProgramResourceiv_length(const State &glState,
                                        bool isCallValid,
                                        ShaderProgramID program,
                                        GLenum programInterface,
                                        GLuint index,
                                        GLsizei propCount,
                                        const GLenum *props,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLint *params,
                                        ParamCapture *paramCapture)
{
    paramCapture->readBufferSizeBytes = sizeof(GLsizei);
}

void CaptureGetProgramResourceiv_params(const State &glState,
                                        bool isCallValid,
                                        ShaderProgramID program,
                                        GLenum programInterface,
                                        GLuint index,
                                        GLsizei propCount,
                                        const GLenum *props,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLint *params,
                                        ParamCapture *paramCapture)
{
    // Prefer to only capture as many parameters as are returned,
    // but if this is not known, then capture the whole buffer
    int paramLength = length != nullptr ? *length : bufSize;
    CaptureMemory(params, sizeof(GLint) * paramLength, paramCapture);
}

void CaptureGetTexLevelParameterfv_params(const State &glState,
                                          bool isCallValid,
                                          TextureTarget targetPacked,
                                          GLint level,
                                          GLenum pname,
                                          GLfloat *params,
                                          ParamCapture *paramCapture)
{
    CaptureMemory(params, sizeof(GLfloat), paramCapture);
}

void CaptureGetTexLevelParameteriv_params(const State &glState,
                                          bool isCallValid,
                                          TextureTarget targetPacked,
                                          GLint level,
                                          GLenum pname,
                                          GLint *params,
                                          ParamCapture *paramCapture)
{
    CaptureMemory(params, sizeof(GLint), paramCapture);
}

void CaptureProgramUniform1fv_value(const State &glState,
                                    bool isCallValid,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLfloat *value,
                                    ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * count, paramCapture);
}

void CaptureProgramUniform1iv_value(const State &glState,
                                    bool isCallValid,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLint *value,
                                    ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLint) * count, paramCapture);
}

void CaptureProgramUniform1uiv_value(const State &glState,
                                     bool isCallValid,
                                     ShaderProgramID program,
                                     UniformLocation location,
                                     GLsizei count,
                                     const GLuint *value,
                                     ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLuint) * count, paramCapture);
}

void CaptureProgramUniform2fv_value(const State &glState,
                                    bool isCallValid,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLfloat *value,
                                    ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 2 * count, paramCapture);
}

void CaptureProgramUniform2iv_value(const State &glState,
                                    bool isCallValid,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLint *value,
                                    ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLint) * 2 * count, paramCapture);
}

void CaptureProgramUniform2uiv_value(const State &glState,
                                     bool isCallValid,
                                     ShaderProgramID program,
                                     UniformLocation location,
                                     GLsizei count,
                                     const GLuint *value,
                                     ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLuint) * 2 * count, paramCapture);
}

void CaptureProgramUniform3fv_value(const State &glState,
                                    bool isCallValid,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLfloat *value,
                                    ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 3 * count, paramCapture);
}

void CaptureProgramUniform3iv_value(const State &glState,
                                    bool isCallValid,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLint *value,
                                    ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLint) * 3 * count, paramCapture);
}

void CaptureProgramUniform3uiv_value(const State &glState,
                                     bool isCallValid,
                                     ShaderProgramID program,
                                     UniformLocation location,
                                     GLsizei count,
                                     const GLuint *value,
                                     ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLuint) * 3 * count, paramCapture);
}

void CaptureProgramUniform4fv_value(const State &glState,
                                    bool isCallValid,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLfloat *value,
                                    ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 4 * count, paramCapture);
}

void CaptureProgramUniform4iv_value(const State &glState,
                                    bool isCallValid,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLint *value,
                                    ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLint) * 4 * count, paramCapture);
}

void CaptureProgramUniform4uiv_value(const State &glState,
                                     bool isCallValid,
                                     ShaderProgramID program,
                                     UniformLocation location,
                                     GLsizei count,
                                     const GLuint *value,
                                     ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLuint) * 4 * count, paramCapture);
}

void CaptureProgramUniformMatrix2fv_value(const State &glState,
                                          bool isCallValid,
                                          ShaderProgramID program,
                                          UniformLocation location,
                                          GLsizei count,
                                          GLboolean transpose,
                                          const GLfloat *value,
                                          ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 2 * 2 * count, paramCapture);
}

void CaptureProgramUniformMatrix2x3fv_value(const State &glState,
                                            bool isCallValid,
                                            ShaderProgramID program,
                                            UniformLocation location,
                                            GLsizei count,
                                            GLboolean transpose,
                                            const GLfloat *value,
                                            ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 2 * 3 * count, paramCapture);
}

void CaptureProgramUniformMatrix2x4fv_value(const State &glState,
                                            bool isCallValid,
                                            ShaderProgramID program,
                                            UniformLocation location,
                                            GLsizei count,
                                            GLboolean transpose,
                                            const GLfloat *value,
                                            ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 2 * 4 * count, paramCapture);
}

void CaptureProgramUniformMatrix3fv_value(const State &glState,
                                          bool isCallValid,
                                          ShaderProgramID program,
                                          UniformLocation location,
                                          GLsizei count,
                                          GLboolean transpose,
                                          const GLfloat *value,
                                          ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 3 * 3 * count, paramCapture);
}

void CaptureProgramUniformMatrix3x2fv_value(const State &glState,
                                            bool isCallValid,
                                            ShaderProgramID program,
                                            UniformLocation location,
                                            GLsizei count,
                                            GLboolean transpose,
                                            const GLfloat *value,
                                            ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 3 * 2 * count, paramCapture);
}

void CaptureProgramUniformMatrix3x4fv_value(const State &glState,
                                            bool isCallValid,
                                            ShaderProgramID program,
                                            UniformLocation location,
                                            GLsizei count,
                                            GLboolean transpose,
                                            const GLfloat *value,
                                            ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 3 * 4 * count, paramCapture);
}

void CaptureProgramUniformMatrix4fv_value(const State &glState,
                                          bool isCallValid,
                                          ShaderProgramID program,
                                          UniformLocation location,
                                          GLsizei count,
                                          GLboolean transpose,
                                          const GLfloat *value,
                                          ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 4 * 4 * count, paramCapture);
}

void CaptureProgramUniformMatrix4x2fv_value(const State &glState,
                                            bool isCallValid,
                                            ShaderProgramID program,
                                            UniformLocation location,
                                            GLsizei count,
                                            GLboolean transpose,
                                            const GLfloat *value,
                                            ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 4 * 2 * count, paramCapture);
}

void CaptureProgramUniformMatrix4x3fv_value(const State &glState,
                                            bool isCallValid,
                                            ShaderProgramID program,
                                            UniformLocation location,
                                            GLsizei count,
                                            GLboolean transpose,
                                            const GLfloat *value,
                                            ParamCapture *paramCapture)
{
    CaptureMemory(value, sizeof(GLfloat) * 4 * 3 * count, paramCapture);
}

}  // namespace gl
