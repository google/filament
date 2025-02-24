//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// validationES31.h:
//  Inlined validation functions for OpenGL ES 3.1 entry points.

#ifndef LIBANGLE_VALIDATION_ES31_H_
#define LIBANGLE_VALIDATION_ES31_H_

#include "libANGLE/ErrorStrings.h"
#include "libANGLE/validationES31_autogen.h"

namespace gl
{

bool ValidateTexBufferBase(const Context *context,
                           angle::EntryPoint entryPoint,
                           TextureType target,
                           GLenum internalformat,
                           BufferID bufferPacked);
bool ValidateTexBufferRangeBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                TextureType target,
                                GLenum internalformat,
                                BufferID bufferPacked,
                                GLintptr offset,
                                GLsizeiptr size);

// GL_EXT_geometry_shader
bool ValidateFramebufferTextureCommon(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLenum target,
                                      GLenum attachment,
                                      TextureID texture,
                                      GLint level);

// GL_EXT_multi_draw_indirect
bool ValidateMultiDrawIndirectBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLsizei drawcount,
                                   GLsizei stride);

// GL_EXT_separate_shader_objects
bool ValidateActiveShaderProgramBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ProgramPipelineID pipelinePacked,
                                     ShaderProgramID programPacked);
bool ValidateBindProgramPipelineBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ProgramPipelineID pipelinePacked);
bool ValidateCreateShaderProgramvBase(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ShaderType typePacked,
                                      GLsizei count,
                                      const GLchar **strings);
bool ValidateDeleteProgramPipelinesBase(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        GLsizei n,
                                        const ProgramPipelineID *pipelinesPacked);
bool ValidateGenProgramPipelinesBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLsizei n,
                                     const ProgramPipelineID *pipelinesPacked);
bool ValidateGetProgramPipelineInfoLogBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ProgramPipelineID pipelinePacked,
                                           GLsizei bufSize,
                                           const GLsizei *length,
                                           const GLchar *infoLog);
bool ValidateGetProgramPipelineivBase(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ProgramPipelineID pipelinePacked,
                                      GLenum pname,
                                      const GLint *params);
bool ValidateIsProgramPipelineBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ProgramPipelineID pipelinePacked);
bool ValidateProgramParameteriBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   GLenum pname,
                                   GLint value);
bool ValidateProgramUniform1fBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID programPacked,
                                  UniformLocation locationPacked,
                                  GLfloat v0);
bool ValidateProgramUniform1fvBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLsizei count,
                                   const GLfloat *value);
bool ValidateProgramUniform1iBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID programPacked,
                                  UniformLocation locationPacked,
                                  GLint v0);
bool ValidateProgramUniform1ivBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLsizei count,
                                   const GLint *value);
bool ValidateProgramUniform1uiBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLuint v0);
bool ValidateProgramUniform1uivBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    ShaderProgramID programPacked,
                                    UniformLocation locationPacked,
                                    GLsizei count,
                                    const GLuint *value);
bool ValidateProgramUniform2fBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID programPacked,
                                  UniformLocation locationPacked,
                                  GLfloat v0,
                                  GLfloat v1);
bool ValidateProgramUniform2fvBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLsizei count,
                                   const GLfloat *value);
bool ValidateProgramUniform2iBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID programPacked,
                                  UniformLocation locationPacked,
                                  GLint v0,
                                  GLint v1);
bool ValidateProgramUniform2ivBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLsizei count,
                                   const GLint *value);
bool ValidateProgramUniform2uiBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLuint v0,
                                   GLuint v1);
bool ValidateProgramUniform2uivBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    ShaderProgramID programPacked,
                                    UniformLocation locationPacked,
                                    GLsizei count,
                                    const GLuint *value);
bool ValidateProgramUniform3fBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID programPacked,
                                  UniformLocation locationPacked,
                                  GLfloat v0,
                                  GLfloat v1,
                                  GLfloat v2);
bool ValidateProgramUniform3fvBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLsizei count,
                                   const GLfloat *value);
bool ValidateProgramUniform3iBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID programPacked,
                                  UniformLocation locationPacked,
                                  GLint v0,
                                  GLint v1,
                                  GLint v2);
bool ValidateProgramUniform3ivBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLsizei count,
                                   const GLint *value);
bool ValidateProgramUniform3uiBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLuint v0,
                                   GLuint v1,
                                   GLuint v2);
bool ValidateProgramUniform3uivBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    ShaderProgramID programPacked,
                                    UniformLocation locationPacked,
                                    GLsizei count,
                                    const GLuint *value);
bool ValidateProgramUniform4fBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID programPacked,
                                  UniformLocation locationPacked,
                                  GLfloat v0,
                                  GLfloat v1,
                                  GLfloat v2,
                                  GLfloat v3);
bool ValidateProgramUniform4fvBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLsizei count,
                                   const GLfloat *value);
bool ValidateProgramUniform4iBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID programPacked,
                                  UniformLocation locationPacked,
                                  GLint v0,
                                  GLint v1,
                                  GLint v2,
                                  GLint v3);
bool ValidateProgramUniform4ivBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLsizei count,
                                   const GLint *value);
bool ValidateProgramUniform4uiBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID programPacked,
                                   UniformLocation locationPacked,
                                   GLuint v0,
                                   GLuint v1,
                                   GLuint v2,
                                   GLuint v3);
bool ValidateProgramUniform4uivBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    ShaderProgramID programPacked,
                                    UniformLocation locationPacked,
                                    GLsizei count,
                                    const GLuint *value);
bool ValidateProgramUniformMatrix2fvBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ShaderProgramID programPacked,
                                         UniformLocation locationPacked,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value);
bool ValidateProgramUniformMatrix2x3fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID programPacked,
                                           UniformLocation locationPacked,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);
bool ValidateProgramUniformMatrix2x4fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID programPacked,
                                           UniformLocation locationPacked,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);
bool ValidateProgramUniformMatrix3fvBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ShaderProgramID programPacked,
                                         UniformLocation locationPacked,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value);
bool ValidateProgramUniformMatrix3x2fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID programPacked,
                                           UniformLocation locationPacked,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);
bool ValidateProgramUniformMatrix3x4fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID programPacked,
                                           UniformLocation locationPacked,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);
bool ValidateProgramUniformMatrix4fvBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ShaderProgramID programPacked,
                                         UniformLocation locationPacked,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value);
bool ValidateProgramUniformMatrix4x2fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID programPacked,
                                           UniformLocation locationPacked,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);
bool ValidateProgramUniformMatrix4x3fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID programPacked,
                                           UniformLocation locationPacked,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);
bool ValidateUseProgramStagesBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ProgramPipelineID pipelinePacked,
                                  GLbitfield stages,
                                  ShaderProgramID programPacked);
bool ValidateValidateProgramPipelineBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ProgramPipelineID pipelinePacked);

// GL_EXT_tessellation_shader
bool ValidatePatchParameteriBase(const PrivateState &state,
                                 ErrorSet *errors,
                                 angle::EntryPoint entryPoint,
                                 GLenum pname,
                                 GLint value);
}  // namespace gl

#endif  // LIBANGLE_VALIDATION_ES31_H_
