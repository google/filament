//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// capture_gles32_params.cpp:
//   Pointer parameter capture functions for the OpenGL ES 3.2 entry points.

#include "libANGLE/capture/capture_gles_2_0_autogen.h"
#include "libANGLE/capture/capture_gles_3_2_autogen.h"

using namespace angle;

namespace gl
{

void CaptureDebugMessageCallback_userParam(const State &glState,
                                           bool isCallValid,
                                           GLDEBUGPROC callback,
                                           const void *userParam,
                                           ParamCapture *userParamParam)
{
    // Skipped
}

void CaptureDebugMessageControl_ids(const State &glState,
                                    bool isCallValid,
                                    GLenum source,
                                    GLenum type,
                                    GLenum severity,
                                    GLsizei count,
                                    const GLuint *ids,
                                    GLboolean enabled,
                                    ParamCapture *idsParam)
{
    // Skipped
}

void CaptureDebugMessageInsert_buf(const State &glState,
                                   bool isCallValid,
                                   GLenum source,
                                   GLenum type,
                                   GLuint id,
                                   GLenum severity,
                                   GLsizei length,
                                   const GLchar *buf,
                                   ParamCapture *bufParam)
{
    // Skipped
}

void CaptureDrawElementsBaseVertex_indices(const State &glState,
                                           bool isCallValid,
                                           PrimitiveMode modePacked,
                                           GLsizei count,
                                           DrawElementsType typePacked,
                                           const void *indices,
                                           GLint basevertex,
                                           ParamCapture *indicesParam)
{
    CaptureDrawElements_indices(glState, isCallValid, modePacked, count, typePacked, indices,
                                indicesParam);
}

void CaptureDrawElementsInstancedBaseVertex_indices(const State &glState,
                                                    bool isCallValid,
                                                    PrimitiveMode modePacked,
                                                    GLsizei count,
                                                    DrawElementsType typePacked,
                                                    const void *indices,
                                                    GLsizei instancecount,
                                                    GLint basevertex,
                                                    ParamCapture *indicesParam)
{
    UNIMPLEMENTED();
}

void CaptureDrawRangeElementsBaseVertex_indices(const State &glState,
                                                bool isCallValid,
                                                PrimitiveMode modePacked,
                                                GLuint start,
                                                GLuint end,
                                                GLsizei count,
                                                DrawElementsType typePacked,
                                                const void *indices,
                                                GLint basevertex,
                                                ParamCapture *indicesParam)
{
    UNIMPLEMENTED();
}

void CaptureGetDebugMessageLog_sources(const State &glState,
                                       bool isCallValid,
                                       GLuint count,
                                       GLsizei bufSize,
                                       GLenum *sources,
                                       GLenum *types,
                                       GLuint *ids,
                                       GLenum *severities,
                                       GLsizei *lengths,
                                       GLchar *messageLog,
                                       ParamCapture *sourcesParam)
{
    // Skipped
}

void CaptureGetDebugMessageLog_types(const State &glState,
                                     bool isCallValid,
                                     GLuint count,
                                     GLsizei bufSize,
                                     GLenum *sources,
                                     GLenum *types,
                                     GLuint *ids,
                                     GLenum *severities,
                                     GLsizei *lengths,
                                     GLchar *messageLog,
                                     ParamCapture *typesParam)
{
    // Skipped
}

void CaptureGetDebugMessageLog_ids(const State &glState,
                                   bool isCallValid,
                                   GLuint count,
                                   GLsizei bufSize,
                                   GLenum *sources,
                                   GLenum *types,
                                   GLuint *ids,
                                   GLenum *severities,
                                   GLsizei *lengths,
                                   GLchar *messageLog,
                                   ParamCapture *idsParam)
{
    // Skipped
}

void CaptureGetDebugMessageLog_severities(const State &glState,
                                          bool isCallValid,
                                          GLuint count,
                                          GLsizei bufSize,
                                          GLenum *sources,
                                          GLenum *types,
                                          GLuint *ids,
                                          GLenum *severities,
                                          GLsizei *lengths,
                                          GLchar *messageLog,
                                          ParamCapture *severitiesParam)
{
    // Skipped
}

void CaptureGetDebugMessageLog_lengths(const State &glState,
                                       bool isCallValid,
                                       GLuint count,
                                       GLsizei bufSize,
                                       GLenum *sources,
                                       GLenum *types,
                                       GLuint *ids,
                                       GLenum *severities,
                                       GLsizei *lengths,
                                       GLchar *messageLog,
                                       ParamCapture *lengthsParam)
{
    // Skipped
}

void CaptureGetDebugMessageLog_messageLog(const State &glState,
                                          bool isCallValid,
                                          GLuint count,
                                          GLsizei bufSize,
                                          GLenum *sources,
                                          GLenum *types,
                                          GLuint *ids,
                                          GLenum *severities,
                                          GLsizei *lengths,
                                          GLchar *messageLog,
                                          ParamCapture *messageLogParam)
{
    // Skipped
}

void CaptureGetObjectLabel_length(const State &glState,
                                  bool isCallValid,
                                  GLenum identifier,
                                  GLuint name,
                                  GLsizei bufSize,
                                  GLsizei *length,
                                  GLchar *label,
                                  ParamCapture *lengthParam)
{
    // Skipped
}

void CaptureGetObjectLabel_label(const State &glState,
                                 bool isCallValid,
                                 GLenum identifier,
                                 GLuint name,
                                 GLsizei bufSize,
                                 GLsizei *length,
                                 GLchar *label,
                                 ParamCapture *labelParam)
{
    // Skipped
}

void CaptureGetObjectPtrLabel_ptr(const State &glState,
                                  bool isCallValid,
                                  const void *ptr,
                                  GLsizei bufSize,
                                  GLsizei *length,
                                  GLchar *label,
                                  ParamCapture *ptrParam)
{
    // Skipped
}

void CaptureGetObjectPtrLabel_length(const State &glState,
                                     bool isCallValid,
                                     const void *ptr,
                                     GLsizei bufSize,
                                     GLsizei *length,
                                     GLchar *label,
                                     ParamCapture *lengthParam)
{
    // Skipped
}

void CaptureGetObjectPtrLabel_label(const State &glState,
                                    bool isCallValid,
                                    const void *ptr,
                                    GLsizei bufSize,
                                    GLsizei *length,
                                    GLchar *label,
                                    ParamCapture *labelParam)
{
    // Skipped
}

void CaptureGetPointerv_params(const State &glState,
                               bool isCallValid,
                               GLenum pname,
                               void **params,
                               ParamCapture *paramCapture)
{
    paramCapture->readBufferSizeBytes = sizeof(void *);
}

void CaptureGetSamplerParameterIiv_params(const State &glState,
                                          bool isCallValid,
                                          SamplerID sampler,
                                          GLenum pname,
                                          GLint *params,
                                          ParamCapture *paramsParam)
{
    // page 458 https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf
    paramsParam->readBufferSizeBytes = sizeof(GLint) * 4;
}

void CaptureGetSamplerParameterIuiv_params(const State &glState,
                                           bool isCallValid,
                                           SamplerID sampler,
                                           GLenum pname,
                                           GLuint *params,
                                           ParamCapture *paramsParam)
{
    // page 458 https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf
    paramsParam->readBufferSizeBytes = sizeof(GLuint) * 4;
}

void CaptureGetTexParameterIiv_params(const State &glState,
                                      bool isCallValid,
                                      TextureType targetPacked,
                                      GLenum pname,
                                      GLint *params,
                                      ParamCapture *paramsParam)
{
    // page 192 https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf
    // TEXTURE_BORDER_COLOR: 4 floats, ints, uints
    paramsParam->readBufferSizeBytes = sizeof(GLint) * 4;
}

void CaptureGetTexParameterIuiv_params(const State &glState,
                                       bool isCallValid,
                                       TextureType targetPacked,
                                       GLenum pname,
                                       GLuint *params,
                                       ParamCapture *paramsParam)
{
    // page 192 https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf
    // TEXTURE_BORDER_COLOR: 4 floats, ints, uints
    paramsParam->readBufferSizeBytes = sizeof(GLuint) * 4;
}

void CaptureGetnUniformfv_params(const State &glState,
                                 bool isCallValid,
                                 ShaderProgramID program,
                                 UniformLocation location,
                                 GLsizei bufSize,
                                 GLfloat *params,
                                 ParamCapture *paramsParam)
{
    UNIMPLEMENTED();
}

void CaptureGetnUniformiv_params(const State &glState,
                                 bool isCallValid,
                                 ShaderProgramID program,
                                 UniformLocation location,
                                 GLsizei bufSize,
                                 GLint *params,
                                 ParamCapture *paramsParam)
{
    UNIMPLEMENTED();
}

void CaptureGetnUniformuiv_params(const State &glState,
                                  bool isCallValid,
                                  ShaderProgramID program,
                                  UniformLocation location,
                                  GLsizei bufSize,
                                  GLuint *params,
                                  ParamCapture *paramsParam)
{
    UNIMPLEMENTED();
}

void CaptureObjectLabel_label(const State &glState,
                              bool isCallValid,
                              GLenum identifier,
                              GLuint name,
                              GLsizei length,
                              const GLchar *label,
                              ParamCapture *labelParam)
{
    // Skipped
}

void CaptureObjectPtrLabel_ptr(const State &glState,
                               bool isCallValid,
                               const void *ptr,
                               GLsizei length,
                               const GLchar *label,
                               ParamCapture *ptrParam)
{
    // Skipped
}

void CaptureObjectPtrLabel_label(const State &glState,
                                 bool isCallValid,
                                 const void *ptr,
                                 GLsizei length,
                                 const GLchar *label,
                                 ParamCapture *labelParam)
{
    // Skipped
}

void CapturePushDebugGroup_message(const State &glState,
                                   bool isCallValid,
                                   GLenum source,
                                   GLuint id,
                                   GLsizei length,
                                   const GLchar *message,
                                   ParamCapture *messageParam)
{
    // Skipped
}

void CaptureReadnPixels_data(const State &glState,
                             bool isCallValid,
                             GLint x,
                             GLint y,
                             GLsizei width,
                             GLsizei height,
                             GLenum format,
                             GLenum type,
                             GLsizei bufSize,
                             void *data,
                             ParamCapture *dataParam)
{
    UNIMPLEMENTED();
}

void CaptureSamplerParameterIiv_param(const State &glState,
                                      bool isCallValid,
                                      SamplerID sampler,
                                      GLenum pname,
                                      const GLint *param,
                                      ParamCapture *paramParam)
{
    CaptureTextureAndSamplerParameter_params<GLint>(pname, param, paramParam);
}

void CaptureSamplerParameterIuiv_param(const State &glState,
                                       bool isCallValid,
                                       SamplerID sampler,
                                       GLenum pname,
                                       const GLuint *param,
                                       ParamCapture *paramParam)
{
    CaptureTextureAndSamplerParameter_params<GLuint>(pname, param, paramParam);
}

void CaptureTexParameterIiv_params(const State &glState,
                                   bool isCallValid,
                                   TextureType targetPacked,
                                   GLenum pname,
                                   const GLint *params,
                                   ParamCapture *paramParam)
{
    CaptureTextureAndSamplerParameter_params<GLint>(pname, params, paramParam);
}

void CaptureTexParameterIuiv_params(const State &glState,
                                    bool isCallValid,
                                    TextureType targetPacked,
                                    GLenum pname,
                                    const GLuint *params,
                                    ParamCapture *paramParam)
{
    CaptureTextureAndSamplerParameter_params<GLuint>(pname, params, paramParam);
}

}  // namespace gl
