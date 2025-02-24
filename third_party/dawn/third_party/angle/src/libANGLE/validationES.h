//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES.h: Validation functions for generic OpenGL ES entry point parameters

#ifndef LIBANGLE_VALIDATION_ES_H_
#define LIBANGLE_VALIDATION_ES_H_

#include "common/PackedEnums.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/VertexArray.h"

#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>

namespace egl
{
class Display;
class Image;
}  // namespace egl

namespace gl
{
class Context;
struct Format;
class Framebuffer;
struct LinkedUniform;
class Program;
class Shader;

#define ANGLE_VALIDATION_ERROR(errorCode, message) \
    context->getMutableErrorSetForValidation()->validationError(entryPoint, errorCode, message)
#define ANGLE_VALIDATION_ERRORF(errorCode, ...) \
    context->getMutableErrorSetForValidation()->validationErrorF(entryPoint, errorCode, __VA_ARGS__)

void SetRobustLengthParam(const GLsizei *length, GLsizei value);
bool ValidTextureTarget(const Context *context, TextureType type);
bool ValidTexture2DTarget(const Context *context, TextureType type);
bool ValidTexture3DTarget(const Context *context, TextureType target);
bool ValidTextureExternalTarget(const Context *context, TextureType target);
bool ValidTextureExternalTarget(const Context *context, TextureTarget target);
bool ValidTexture2DDestinationTarget(const Context *context, TextureTarget target);
bool ValidTexture3DDestinationTarget(const Context *context, TextureTarget target);
bool ValidTexLevelDestinationTarget(const Context *context, TextureType type);
bool ValidFramebufferTarget(const Context *context, GLenum target);
bool ValidMipLevel(const Context *context, TextureType type, GLint level);
bool ValidImageSizeParameters(const Context *context,
                              angle::EntryPoint entryPoint,
                              TextureType target,
                              GLint level,
                              GLsizei width,
                              GLsizei height,
                              GLsizei depth,
                              bool isSubImage);
bool ValidCompressedImageSize(const Context *context,
                              GLenum internalFormat,
                              GLint level,
                              GLsizei width,
                              GLsizei height,
                              GLsizei depth);
bool ValidCompressedSubImageSize(const Context *context,
                                 GLenum internalFormat,
                                 GLint xoffset,
                                 GLint yoffset,
                                 GLint zoffset,
                                 GLsizei width,
                                 GLsizei height,
                                 GLsizei depth,
                                 size_t textureWidth,
                                 size_t textureHeight,
                                 size_t textureDepth);
bool ValidImageDataSize(const Context *context,
                        angle::EntryPoint entryPoint,
                        TextureType texType,
                        GLsizei width,
                        GLsizei height,
                        GLsizei depth,
                        GLenum format,
                        GLenum type,
                        const void *pixels,
                        GLsizei imageSize);

bool ValidQueryType(const Context *context, QueryType queryType);

bool ValidateWebGLVertexAttribPointer(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      VertexAttribType type,
                                      GLboolean normalized,
                                      GLsizei stride,
                                      const void *ptr,
                                      bool pureInteger);

// Returns valid program if id is a valid program name
// Errors INVALID_OPERATION if valid shader is given and returns NULL
// Errors INVALID_VALUE otherwise and returns NULL
Program *GetValidProgram(const Context *context, angle::EntryPoint entryPoint, ShaderProgramID id);

// Returns valid shader if id is a valid shader name
// Errors INVALID_OPERATION if valid program is given and returns NULL
// Errors INVALID_VALUE otherwise and returns NULL
Shader *GetValidShader(const Context *context, angle::EntryPoint entryPoint, ShaderProgramID id);

bool ValidateAttachmentTarget(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLenum attachment);

bool ValidateBlitFramebufferParameters(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       GLint srcX0,
                                       GLint srcY0,
                                       GLint srcX1,
                                       GLint srcY1,
                                       GLint dstX0,
                                       GLint dstY0,
                                       GLint dstX1,
                                       GLint dstY1,
                                       GLbitfield mask,
                                       GLenum filter);

bool ValidateBindFramebufferBase(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLenum target,
                                 FramebufferID framebuffer);
bool ValidateBindRenderbufferBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLenum target,
                                  RenderbufferID renderbuffer);
bool ValidateFramebufferParameteriBase(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       GLenum target,
                                       GLenum pname,
                                       GLint param);
bool ValidateFramebufferRenderbufferBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         GLenum target,
                                         GLenum attachment,
                                         GLenum renderbuffertarget,
                                         RenderbufferID renderbuffer);
bool ValidateFramebufferTextureBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLenum target,
                                    GLenum attachment,
                                    TextureID texture,
                                    GLint level);
bool ValidateGenerateMipmapBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                TextureType target);

bool ValidateRenderbufferStorageParametersBase(const Context *context,
                                               angle::EntryPoint entryPoint,
                                               GLenum target,
                                               GLsizei samples,
                                               GLenum internalformat,
                                               GLsizei width,
                                               GLsizei height);

bool ValidatePixelPack(const Context *context,
                       angle::EntryPoint entryPoint,
                       GLenum format,
                       GLenum type,
                       GLint x,
                       GLint y,
                       GLsizei width,
                       GLsizei height,
                       GLsizei bufSize,
                       GLsizei *length,
                       const void *pixels);

bool ValidateReadPixelsBase(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLint x,
                            GLint y,
                            GLsizei width,
                            GLsizei height,
                            GLenum format,
                            GLenum type,
                            GLsizei bufSize,
                            GLsizei *length,
                            GLsizei *columns,
                            GLsizei *rows,
                            const void *pixels);
bool ValidateReadPixelsRobustANGLE(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLint x,
                                   GLint y,
                                   GLsizei width,
                                   GLsizei height,
                                   GLenum format,
                                   GLenum type,
                                   GLsizei bufSize,
                                   const GLsizei *length,
                                   const GLsizei *columns,
                                   const GLsizei *rows,
                                   const void *pixels);
bool ValidateReadnPixelsEXT(const Context *context,
                            angle::EntryPoint entryPoint,
                            GLint x,
                            GLint y,
                            GLsizei width,
                            GLsizei height,
                            GLenum format,
                            GLenum type,
                            GLsizei bufSize,
                            const void *pixels);
bool ValidateReadnPixelsRobustANGLE(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLint x,
                                    GLint y,
                                    GLsizei width,
                                    GLsizei height,
                                    GLenum format,
                                    GLenum type,
                                    GLsizei bufSize,
                                    const GLsizei *length,
                                    const GLsizei *columns,
                                    const GLsizei *rows,
                                    const void *data);

bool ValidateGenQueriesEXT(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLsizei n,
                           const QueryID *ids);
bool ValidateDeleteQueriesEXT(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLsizei n,
                              const QueryID *ids);
bool ValidateIsQueryEXT(const Context *context, angle::EntryPoint entryPoint, QueryID id);
bool ValidateBeginQueryBase(const Context *context,
                            angle::EntryPoint entryPoint,
                            QueryType target,
                            QueryID id);
bool ValidateBeginQueryEXT(const Context *context,
                           angle::EntryPoint entryPoint,
                           QueryType target,
                           QueryID id);
bool ValidateEndQueryBase(const Context *context, angle::EntryPoint entryPoint, QueryType target);
bool ValidateEndQueryEXT(const Context *context, angle::EntryPoint entryPoint, QueryType target);
bool ValidateQueryCounterEXT(const Context *context,
                             angle::EntryPoint entryPoint,
                             QueryID id,
                             QueryType target);
bool ValidateGetQueryivBase(const Context *context,
                            angle::EntryPoint entryPoint,
                            QueryType target,
                            GLenum pname,
                            GLsizei *numParams);
bool ValidateGetQueryivEXT(const Context *context,
                           angle::EntryPoint entryPoint,
                           QueryType target,
                           GLenum pname,
                           const GLint *params);
bool ValidateGetQueryivRobustANGLE(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   QueryType target,
                                   GLenum pname,
                                   GLsizei bufSize,
                                   const GLsizei *length,
                                   const GLint *params);
bool ValidateGetQueryObjectValueBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     QueryID id,
                                     GLenum pname,
                                     GLsizei *numParams);
bool ValidateGetQueryObjectivEXT(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 QueryID id,
                                 GLenum pname,
                                 const GLint *params);
bool ValidateGetQueryObjectivRobustANGLE(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         QueryID id,
                                         GLenum pname,
                                         GLsizei bufSize,
                                         const GLsizei *length,
                                         const GLint *params);
bool ValidateGetQueryObjectuivEXT(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  QueryID id,
                                  GLenum pname,
                                  const GLuint *params);
bool ValidateGetQueryObjectuivRobustANGLE(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          QueryID id,
                                          GLenum pname,
                                          GLsizei bufSize,
                                          const GLsizei *length,
                                          const GLuint *params);
bool ValidateGetQueryObjecti64vEXT(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   QueryID id,
                                   GLenum pname,
                                   GLint64 *params);
bool ValidateGetQueryObjecti64vRobustANGLE(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           QueryID id,
                                           GLenum pname,
                                           GLsizei bufSize,
                                           const GLsizei *length,
                                           GLint64 *params);
bool ValidateGetQueryObjectui64vEXT(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    QueryID id,
                                    GLenum pname,
                                    GLuint64 *params);
bool ValidateGetQueryObjectui64vRobustANGLE(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            QueryID id,
                                            GLenum pname,
                                            GLsizei bufSize,
                                            const GLsizei *length,
                                            GLuint64 *params);

ANGLE_INLINE bool ValidateUniformCommonBase(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            const Program *program,
                                            UniformLocation location,
                                            GLsizei count,
                                            const LinkedUniform **uniformOut)
{
    // TODO(Jiajia): Add image uniform check in future.
    if (count < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, err::kNegativeCount);
        return false;
    }

    if (!program)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kInvalidProgramName);
        return false;
    }

    if (!program->isLinked())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kProgramNotLinked);
        return false;
    }

    if (location.value == -1)
    {
        // Silently ignore the uniform command
        return false;
    }

    const ProgramExecutable &executable = program->getExecutable();
    const auto &uniformLocations        = executable.getUniformLocations();
    size_t castedLocation               = static_cast<size_t>(location.value);
    if (castedLocation >= uniformLocations.size())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kInvalidUniformLocation);
        return false;
    }

    const auto &uniformLocation = uniformLocations[castedLocation];
    if (uniformLocation.ignored)
    {
        // Silently ignore the uniform command
        return false;
    }

    if (!uniformLocation.used())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kInvalidUniformLocation);
        return false;
    }

    const LinkedUniform &uniform = executable.getUniformByIndex(uniformLocation.index);

    // attempting to write an array to a non-array uniform is an INVALID_OPERATION
    if (count > 1 && !uniform.isArray())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kInvalidUniformCount);
        return false;
    }

    *uniformOut = &uniform;
    return true;
}

bool ValidateUniform1ivValue(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLenum uniformType,
                             GLsizei count,
                             const GLint *value);

ANGLE_INLINE bool ValidateUniformValue(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       GLenum valueType,
                                       GLenum uniformType)
{
    // Check that the value type is compatible with uniform type.
    // Do the cheaper test first, for a little extra speed.
    if (valueType != uniformType && VariableBoolVectorType(valueType) != uniformType)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kUniformSizeMismatch);
        return false;
    }
    return true;
}

bool ValidateUniformMatrixValue(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLenum valueType,
                                GLenum uniformType);
bool ValidateUniform(const Context *context,
                     angle::EntryPoint entryPoint,
                     GLenum uniformType,
                     UniformLocation location,
                     GLsizei count);
bool ValidateUniformMatrix(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLenum matrixType,
                           UniformLocation location,
                           GLsizei count,
                           GLboolean transpose);
bool ValidateGetBooleanvRobustANGLE(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLenum pname,
                                    GLsizei bufSize,
                                    const GLsizei *length,
                                    const GLboolean *params);
bool ValidateGetFloatvRobustANGLE(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLenum pname,
                                  GLsizei bufSize,
                                  const GLsizei *length,
                                  const GLfloat *params);
bool ValidateStateQuery(const Context *context,
                        angle::EntryPoint entryPoint,
                        GLenum pname,
                        GLenum *nativeType,
                        unsigned int *numParams);
bool ValidateGetIntegervRobustANGLE(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLenum pname,
                                    GLsizei bufSize,
                                    const GLsizei *length,
                                    const GLint *data);
bool ValidateGetInteger64vRobustANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLenum pname,
                                      GLsizei bufSize,
                                      const GLsizei *length,
                                      GLint64 *data);
bool ValidateRobustStateQuery(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLenum pname,
                              GLsizei bufSize,
                              GLenum *nativeType,
                              unsigned int *numParams);

bool ValidateCopyImageSubDataBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLuint srcName,
                                  GLenum srcTarget,
                                  GLint srcLevel,
                                  GLint srcX,
                                  GLint srcY,
                                  GLint srcZ,
                                  GLuint dstName,
                                  GLenum dstTarget,
                                  GLint dstLevel,
                                  GLint dstX,
                                  GLint dstY,
                                  GLint dstZ,
                                  GLsizei srcWidth,
                                  GLsizei srcHeight,
                                  GLsizei srcDepth);

bool ValidateCopyTexImageParametersBase(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        TextureTarget target,
                                        GLint level,
                                        GLenum internalformat,
                                        bool isSubImage,
                                        GLint xoffset,
                                        GLint yoffset,
                                        GLint zoffset,
                                        GLint x,
                                        GLint y,
                                        GLsizei width,
                                        GLsizei height,
                                        GLint border,
                                        Format *textureFormatOut);

void RecordDrawModeError(const Context *context, angle::EntryPoint entryPoint, PrimitiveMode mode);
const char *ValidateDrawElementsStates(const Context *context);

ANGLE_INLINE bool ValidateDrawBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   PrimitiveMode mode)
{
    intptr_t drawStatesError = context->getStateCache().getBasicDrawStatesErrorString(
        context, &context->getPrivateStateCache());
    if (drawStatesError)
    {
        const char *errorMessage = reinterpret_cast<const char *>(drawStatesError);
        GLenum errorCode         = context->getStateCache().getBasicDrawElementsErrorCode();
        ANGLE_VALIDATION_ERROR(errorCode, errorMessage);
        return false;
    }

    if (!context->getStateCache().isValidDrawMode(mode))
    {
        RecordDrawModeError(context, entryPoint, mode);
        return false;
    }

    return true;
}

bool ValidateDrawArraysInstancedBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     PrimitiveMode mode,
                                     GLint first,
                                     GLsizei count,
                                     GLsizei primcount,
                                     GLuint baseinstance);
bool ValidateDrawArraysInstancedANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      PrimitiveMode mode,
                                      GLint first,
                                      GLsizei count,
                                      GLsizei primcount);
bool ValidateDrawArraysInstancedEXT(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    PrimitiveMode mode,
                                    GLint first,
                                    GLsizei count,
                                    GLsizei primcount);

bool ValidateDrawElementsInstancedBase(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       PrimitiveMode mode,
                                       GLsizei count,
                                       DrawElementsType type,
                                       const void *indices,
                                       GLsizei primcount,
                                       GLuint baseinstance);
bool ValidateDrawElementsInstancedANGLE(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        PrimitiveMode mode,
                                        GLsizei count,
                                        DrawElementsType type,
                                        const void *indices,
                                        GLsizei primcount);
bool ValidateDrawElementsInstancedEXT(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      PrimitiveMode mode,
                                      GLsizei count,
                                      DrawElementsType type,
                                      const void *indices,
                                      GLsizei primcount);

bool ValidateDrawInstancedANGLE(const Context *context, angle::EntryPoint entryPoint);

bool ValidateGetUniformBase(const Context *context,
                            angle::EntryPoint entryPoint,
                            ShaderProgramID program,
                            UniformLocation location);
bool ValidateSizedGetUniform(const Context *context,
                             angle::EntryPoint entryPoint,
                             ShaderProgramID program,
                             UniformLocation location,
                             GLsizei bufSize,
                             GLsizei *length);
bool ValidateGetnUniformfvEXT(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID program,
                              UniformLocation location,
                              GLsizei bufSize,
                              const GLfloat *params);
bool ValidateGetnUniformfvRobustANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ShaderProgramID program,
                                      UniformLocation location,
                                      GLsizei bufSize,
                                      const GLsizei *length,
                                      const GLfloat *params);
bool ValidateGetnUniformivEXT(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID program,
                              UniformLocation location,
                              GLsizei bufSize,
                              const GLint *params);
bool ValidateGetnUniformivRobustANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ShaderProgramID program,
                                      UniformLocation location,
                                      GLsizei bufSize,
                                      const GLsizei *length,
                                      const GLint *params);
bool ValidateGetnUniformuivRobustANGLE(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID program,
                                       UniformLocation location,
                                       GLsizei bufSize,
                                       const GLsizei *length,
                                       const GLuint *params);
bool ValidateGetUniformfvRobustANGLE(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ShaderProgramID program,
                                     UniformLocation location,
                                     GLsizei bufSize,
                                     const GLsizei *length,
                                     const GLfloat *params);
bool ValidateGetUniformivRobustANGLE(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ShaderProgramID program,
                                     UniformLocation location,
                                     GLsizei bufSize,
                                     const GLsizei *length,
                                     const GLint *params);
bool ValidateGetUniformuivRobustANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ShaderProgramID program,
                                      UniformLocation location,
                                      GLsizei bufSize,
                                      const GLsizei *length,
                                      const GLuint *params);

bool ValidateDiscardFramebufferBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLenum target,
                                    GLsizei numAttachments,
                                    const GLenum *attachments,
                                    bool defaultFramebuffer);

bool ValidateInsertEventMarkerEXT(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLsizei length,
                                  const char *marker);
bool ValidatePushGroupMarkerEXT(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLsizei length,
                                const char *marker);
bool ValidateEGLImageObject(const Context *context,
                            angle::EntryPoint entryPoint,
                            TextureType type,
                            egl::ImageID image);
bool ValidateEGLImageTargetTexture2DOES(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        TextureType type,
                                        egl::ImageID image);
bool ValidateEGLImageTargetRenderbufferStorageOES(const Context *context,
                                                  angle::EntryPoint entryPoint,
                                                  GLenum target,
                                                  egl::ImageID image);

bool ValidateProgramBinaryBase(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID program,
                               GLenum binaryFormat,
                               const void *binary,
                               GLint length);
bool ValidateGetProgramBinaryBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  GLsizei bufSize,
                                  const GLsizei *length,
                                  const GLenum *binaryFormat,
                                  const void *binary);

bool ValidateDrawBuffersBase(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLsizei n,
                             const GLenum *bufs);

bool ValidateGetBufferPointervBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   BufferBinding target,
                                   GLenum pname,
                                   GLsizei *length,
                                   void *const *params);
bool ValidateUnmapBufferBase(const Context *context,
                             angle::EntryPoint entryPoint,
                             BufferBinding target);
bool ValidateMapBufferRangeBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                BufferBinding target,
                                GLintptr offset,
                                GLsizeiptr length,
                                GLbitfield access);
bool ValidateFlushMappedBufferRangeBase(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        BufferBinding target,
                                        GLintptr offset,
                                        GLsizeiptr length);

bool ValidateGenOrDelete(const Context *context, angle::EntryPoint entryPoint, GLint n);

bool ValidateRobustEntryPoint(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLsizei bufSize);
bool ValidateRobustBufferSize(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLsizei bufSize,
                              GLsizei numParams);

bool ValidateGetFramebufferAttachmentParameterivBase(const Context *context,
                                                     angle::EntryPoint entryPoint,
                                                     GLenum target,
                                                     GLenum attachment,
                                                     GLenum pname,
                                                     GLsizei *numParams);

bool ValidateGetFramebufferParameterivBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           GLenum target,
                                           GLenum pname,
                                           const GLint *params);

bool ValidateGetBufferParameterBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    BufferBinding target,
                                    GLenum pname,
                                    bool pointerVersion,
                                    GLsizei *numParams);

bool ValidateGetProgramivBase(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID program,
                              GLenum pname,
                              GLsizei *numParams);

bool ValidateGetRenderbufferParameterivBase(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            GLenum target,
                                            GLenum pname,
                                            GLsizei *length);

bool ValidateGetShaderivBase(const Context *context,
                             angle::EntryPoint entryPoint,
                             ShaderProgramID shader,
                             GLenum pname,
                             GLsizei *length);

bool ValidateGetTexParameterBase(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 TextureType target,
                                 GLenum pname,
                                 GLsizei *length);

template <typename ParamType>
bool ValidateTexParameterBase(const Context *context,
                              angle::EntryPoint entryPoint,
                              TextureType target,
                              GLenum pname,
                              GLsizei bufSize,
                              bool vectorParams,
                              const ParamType *params);

bool ValidateGetVertexAttribBase(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLuint index,
                                 GLenum pname,
                                 GLsizei *length,
                                 bool pointer,
                                 bool pureIntegerEntryPoint);

ANGLE_INLINE bool ValidateVertexFormat(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       GLuint index,
                                       GLint size,
                                       VertexAttribTypeCase validation)
{
    const Caps &caps = context->getCaps();
    if (index >= static_cast<GLuint>(caps.maxVertexAttributes))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, err::kIndexExceedsMaxVertexAttribute);
        return false;
    }

    switch (validation)
    {
        case VertexAttribTypeCase::Invalid:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, err::kInvalidType);
            return false;
        case VertexAttribTypeCase::Valid:
            if (size < 1 || size > 4)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, err::kInvalidVertexAttrSize);
                return false;
            }
            break;
        case VertexAttribTypeCase::ValidSize4Only:
            if (size != 4)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kInvalidVertexAttribSize2101010);
                return false;
            }
            break;
        case VertexAttribTypeCase::ValidSize3or4:
            if (size != 3 && size != 4)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kInvalidVertexAttribSize1010102);
                return false;
            }
            break;
    }

    return true;
}

// Note: These byte, short, and int types are all converted to float for the shader.
ANGLE_INLINE bool ValidateFloatVertexFormat(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            GLuint index,
                                            GLint size,
                                            VertexAttribType type)
{
    return ValidateVertexFormat(context, entryPoint, index, size,
                                context->getStateCache().getVertexAttribTypeValidation(type));
}

ANGLE_INLINE bool ValidateIntegerVertexFormat(const Context *context,
                                              angle::EntryPoint entryPoint,
                                              GLuint index,
                                              GLint size,
                                              VertexAttribType type)
{
    return ValidateVertexFormat(
        context, entryPoint, index, size,
        context->getStateCache().getIntegerVertexAttribTypeValidation(type));
}

ANGLE_INLINE bool ValidateColorMasksForSharedExponentColorBuffers(const BlendStateExt &blendState,
                                                                  const Framebuffer *framebuffer)
{
    // Get a mask of draw buffers that have color writemasks
    // incompatible with shared exponent color buffers.
    // The compatible writemasks are RGBA, RGB0, 000A, 0000.
    const BlendStateExt::ColorMaskStorage::Type rgbEnabledBits =
        blendState.expandColorMaskValue(true, true, true, false);
    const BlendStateExt::ColorMaskStorage::Type colorMaskNoAlphaBits =
        blendState.getColorMaskBits() & rgbEnabledBits;
    const DrawBufferMask incompatibleDiffMask =
        BlendStateExt::ColorMaskStorage::GetDiffMask(colorMaskNoAlphaBits, 0) &
        BlendStateExt::ColorMaskStorage::GetDiffMask(colorMaskNoAlphaBits, rgbEnabledBits);

    const DrawBufferMask sharedExponentBufferMask =
        framebuffer->getActiveSharedExponentColorAttachmentDrawBufferMask();
    return (sharedExponentBufferMask & incompatibleDiffMask).none();
}

bool ValidateRobustCompressedTexImageBase(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          GLsizei imageSize,
                                          GLsizei dataSize);

bool ValidateVertexAttribIndex(const PrivateState &state,
                               ErrorSet *errors,
                               angle::EntryPoint entryPoint,
                               GLuint index);

bool ValidateGetActiveUniformBlockivBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ShaderProgramID program,
                                         UniformBlockIndex uniformBlockIndex,
                                         GLenum pname,
                                         GLsizei *length);

bool ValidateGetSamplerParameterBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     SamplerID sampler,
                                     GLenum pname,
                                     GLsizei *length);

template <typename ParamType>
bool ValidateSamplerParameterBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  SamplerID sampler,
                                  GLenum pname,
                                  GLsizei bufSize,
                                  bool vectorParams,
                                  const ParamType *params);

bool ValidateGetInternalFormativBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLenum target,
                                     GLenum internalformat,
                                     GLenum pname,
                                     GLsizei bufSize,
                                     GLsizei *numParams);

bool ValidateFramebufferNotMultisampled(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        const Framebuffer *framebuffer,
                                        bool checkReadBufferResourceSamples);

bool ValidateMultitextureUnit(const PrivateState &state,
                              ErrorSet *errors,
                              angle::EntryPoint entryPoint,
                              GLenum texture);

bool ValidateTransformFeedbackPrimitiveMode(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            PrimitiveMode transformFeedbackPrimitiveMode,
                                            PrimitiveMode renderPrimitiveMode);

// Common validation for 2D and 3D variants of TexStorage*Multisample.
bool ValidateTexStorageMultisample(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   TextureType target,
                                   GLsizei samples,
                                   GLint internalFormat,
                                   GLsizei width,
                                   GLsizei height);

bool ValidateTexStorage2DMultisampleBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target,
                                         GLsizei samples,
                                         GLint internalFormat,
                                         GLsizei width,
                                         GLsizei height);

bool ValidateTexStorage3DMultisampleBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target,
                                         GLsizei samples,
                                         GLenum internalformat,
                                         GLsizei width,
                                         GLsizei height,
                                         GLsizei depth);

bool ValidateGetTexLevelParameterBase(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      TextureTarget target,
                                      GLint level,
                                      GLenum pname,
                                      GLsizei *length);

bool ValidateMapBufferBase(const Context *context,
                           angle::EntryPoint entryPoint,
                           BufferBinding target);
bool ValidateIndexedStateQuery(const Context *context,
                               angle::EntryPoint entryPoint,
                               GLenum pname,
                               GLuint index,
                               GLsizei *length);
bool ValidateES3TexImage2DParameters(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     TextureTarget target,
                                     GLint level,
                                     GLenum internalformat,
                                     bool isCompressed,
                                     bool isSubImage,
                                     GLint xoffset,
                                     GLint yoffset,
                                     GLint zoffset,
                                     GLsizei width,
                                     GLsizei height,
                                     GLsizei depth,
                                     GLint border,
                                     GLenum format,
                                     GLenum type,
                                     GLsizei imageSize,
                                     const void *pixels);
bool ValidateES3CopyTexImage2DParameters(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureTarget target,
                                         GLint level,
                                         GLenum internalformat,
                                         bool isSubImage,
                                         GLint xoffset,
                                         GLint yoffset,
                                         GLint zoffset,
                                         GLint x,
                                         GLint y,
                                         GLsizei width,
                                         GLsizei height,
                                         GLint border);
bool ValidateES3TexStorageParametersBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target,
                                         GLsizei levels,
                                         GLenum internalformat,
                                         GLsizei width,
                                         GLsizei height,
                                         GLsizei depth);
bool ValidateES3TexStorage2DParameters(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureType target,
                                       GLsizei levels,
                                       GLenum internalformat,
                                       GLsizei width,
                                       GLsizei height,
                                       GLsizei depth);
bool ValidateES3TexStorage3DParameters(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureType target,
                                       GLsizei levels,
                                       GLenum internalformat,
                                       GLsizei width,
                                       GLsizei height,
                                       GLsizei depth);

bool ValidateGetMultisamplefvBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLenum pname,
                                  GLuint index,
                                  const GLfloat *val);
bool ValidateSampleMaskiBase(const PrivateState &state,
                             ErrorSet *errors,
                             angle::EntryPoint entryPoint,
                             GLuint maskNumber,
                             GLbitfield mask);

bool ValidateProgramExecutableXFBBuffersPresent(const Context *context,
                                                const ProgramExecutable *programExecutable);

// We should check with Khronos if returning INVALID_FRAMEBUFFER_OPERATION is OK when querying
// implementation format info for incomplete framebuffers. It seems like these queries are
// incongruent with the other errors.
// Inlined for speed.
template <GLenum ErrorCode = GL_INVALID_FRAMEBUFFER_OPERATION>
ANGLE_INLINE bool ValidateFramebufferComplete(const Context *context,
                                              angle::EntryPoint entryPoint,
                                              const Framebuffer *framebuffer)
{
    const FramebufferStatus &framebufferStatus = framebuffer->checkStatus(context);
    if (!framebufferStatus.isComplete())
    {
        ASSERT(framebufferStatus.reason != nullptr);
        ANGLE_VALIDATION_ERROR(ErrorCode, framebufferStatus.reason);
        return false;
    }

    return true;
}

const char *ValidateProgramPipelineDrawStates(const State &state,
                                              const Extensions &extensions,
                                              ProgramPipeline *programPipeline);
const char *ValidateProgramPipelineAttachedPrograms(ProgramPipeline *programPipeline);
const char *ValidateDrawStates(const Context *context, GLenum *outErrorCode);
const char *ValidateProgramPipeline(const Context *context);

void RecordDrawAttribsError(const Context *context, angle::EntryPoint entryPoint);

ANGLE_INLINE bool ValidateDrawAttribs(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      int64_t maxVertex)
{
    // For non-instanced attributes, the maximum vertex must be accessible in the attribute buffers.
    // For instanced attributes, in non-instanced draw calls only attribute 0 is accessed.  In
    // instanced draw calls, the instance limit is checked in ValidateDrawInstancedAttribs.
    if (maxVertex >= context->getStateCache().getNonInstancedVertexElementLimit() ||
        context->getStateCache().getInstancedVertexElementLimit() < 1)
    {
        RecordDrawAttribsError(context, entryPoint);
        return false;
    }

    return true;
}

ANGLE_INLINE bool ValidateDrawArraysAttribs(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            GLint first,
                                            GLsizei count)
{
    if (!context->isBufferAccessValidationEnabled())
    {
        return true;
    }

    // Check the computation of maxVertex doesn't overflow.
    // - first < 0 has been checked as an error condition.
    // - if count <= 0, skip validating no-op draw calls.
    // From this we know maxVertex will be positive, and only need to check if it overflows GLint.
    ASSERT(first >= 0);
    ASSERT(count > 0);
    int64_t maxVertex = static_cast<int64_t>(first) + static_cast<int64_t>(count) - 1;
    if (maxVertex > static_cast<int64_t>(std::numeric_limits<GLint>::max()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kIntegerOverflow);
        return false;
    }

    return ValidateDrawAttribs(context, entryPoint, maxVertex);
}

ANGLE_INLINE bool ValidateDrawInstancedAttribs(const Context *context,
                                               angle::EntryPoint entryPoint,
                                               GLint primcount,
                                               GLuint baseinstance)
{
    if (!context->isBufferAccessValidationEnabled())
    {
        return true;
    }

    // Validate that the buffers bound for the attributes can hold enough vertices for this
    // instanced draw.  For attributes with a divisor of 0, ValidateDrawAttribs already checks this.
    // Thus, the following only checks attributes with a non-zero divisor (i.e. "instanced").
    const GLint64 limit = context->getStateCache().getInstancedVertexElementLimit();
    if (baseinstance >= limit || primcount > limit - baseinstance)
    {
        RecordDrawAttribsError(context, entryPoint);
        return false;
    }

    return true;
}

ANGLE_INLINE bool ValidateDrawArraysCommon(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           PrimitiveMode mode,
                                           GLint first,
                                           GLsizei count,
                                           GLsizei primcount)
{
    if (first < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, err::kNegativeStart);
        return false;
    }

    if (count <= 0)
    {
        if (count < 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, err::kNegativeCount);
            return false;
        }

        // Early exit.
        return ValidateDrawBase(context, entryPoint, mode);
    }

    if (primcount <= 0)
    {
        if (primcount < 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, err::kNegativeCount);
            return false;
        }
        // Early exit.
        return ValidateDrawBase(context, entryPoint, mode);
    }

    if (!ValidateDrawBase(context, entryPoint, mode))
    {
        return false;
    }

    if (context->getStateCache().isTransformFeedbackActiveUnpaused() &&
        !context->supportsGeometryOrTesselation())
    {
        const State &state                      = context->getState();
        TransformFeedback *curTransformFeedback = state.getCurrentTransformFeedback();
        if (!curTransformFeedback->checkBufferSpaceForDraw(count, primcount))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kTransformFeedbackBufferTooSmall);
            return false;
        }
    }

    return ValidateDrawArraysAttribs(context, entryPoint, first, count);
}

ANGLE_INLINE bool ValidateDrawElementsBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           PrimitiveMode mode,
                                           DrawElementsType type)
{
    if (!context->getStateCache().isValidDrawElementsType(type))
    {
        if (type == DrawElementsType::UnsignedInt)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, err::kTypeNotUnsignedShortByte);
            return false;
        }

        ASSERT(type == DrawElementsType::InvalidEnum);
        ANGLE_VALIDATION_ERRORF(GL_INVALID_ENUM, err::kEnumInvalid);
        return false;
    }

    intptr_t drawElementsError = context->getStateCache().getBasicDrawElementsError(context);
    if (drawElementsError)
    {
        // All errors from ValidateDrawElementsStates return INVALID_OPERATION.
        const char *errorMessage = reinterpret_cast<const char *>(drawElementsError);
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, errorMessage);
        return false;
    }

    // Note that we are missing overflow checks for active transform feedback buffers.
    return true;
}

ANGLE_INLINE bool ValidateDrawElementsCommon(const Context *context,
                                             angle::EntryPoint entryPoint,
                                             PrimitiveMode mode,
                                             GLsizei count,
                                             DrawElementsType type,
                                             const void *indices,
                                             GLsizei primcount)
{
    if (!ValidateDrawElementsBase(context, entryPoint, mode, type))
    {
        return false;
    }

    ASSERT(isPow2(GetDrawElementsTypeSize(type)) && GetDrawElementsTypeSize(type) > 0);

    const State &state         = context->getState();
    const VertexArray *vao     = state.getVertexArray();
    Buffer *elementArrayBuffer = vao->getElementArrayBuffer();
    GLuint typeBytes           = GetDrawElementsTypeSize(type);

    if (elementArrayBuffer != nullptr)
    {
        if ((reinterpret_cast<uintptr_t>(indices) & static_cast<uintptr_t>(typeBytes - 1)) != 0)
        {
            // [WebGL 1.0] Section 6.4 Buffer Offset and Stride Requirements:
            // The offset arguments to drawElements and [...], must be a multiple of the size of the
            // data type passed to the call, or an INVALID_OPERATION error is generated.
            // [GLES 3.2] Section 6.3:
            // Clients must align data elements consistently with the requirements of the
            // client platform, with an additional base-level requirement that an offset within a
            // buffer to a datum comprising N basic machine units be a multiple of N.
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kOffsetMustBeMultipleOfType);
            return false;
        }

        // [WebGL 1.0] Section 6.4 Buffer Offset and Stride Requirements
        // In addition the offset argument to drawElements must be non-negative or an INVALID_VALUE
        // error is generated.
        if (reinterpret_cast<intptr_t>(indices) < 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, err::kNegativeOffset);
            return false;
        }
    }

    if (count <= 0)
    {
        if (count < 0)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, err::kNegativeCount);
            return false;
        }

        // Early exit.
        return ValidateDrawBase(context, entryPoint, mode);
    }

    if (!ValidateDrawBase(context, entryPoint, mode))
    {
        return false;
    }

    if (!elementArrayBuffer)
    {
        if (!indices)
        {
            // This is an application error that would normally result in a crash, but we catch
            // it and return an error
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kElementArrayNoBufferOrPointer);
            return false;
        }
    }
    else
    {
        // The max possible type size is 8 and count is on 32 bits so doing the multiplication
        // in a 64 bit integer is safe. Also we are guaranteed that here count > 0.
        static_assert(std::is_same<int, GLsizei>::value, "GLsizei isn't the expected type");
        constexpr uint64_t kMaxTypeSize = 8;
        constexpr uint64_t kIntMax      = std::numeric_limits<int>::max();
        constexpr uint64_t kUint64Max   = std::numeric_limits<uint64_t>::max();
        static_assert(kIntMax < kUint64Max / kMaxTypeSize, "");

        uint64_t elementCount = static_cast<uint64_t>(count);
        ASSERT(elementCount > 0 && GetDrawElementsTypeSize(type) <= kMaxTypeSize);

        // Doing the multiplication here is overflow-safe
        uint64_t elementDataSizeNoOffset = elementCount << GetDrawElementsTypeShift(type);

        // The offset can be any value, check for overflows
        uint64_t offset = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(indices));
        uint64_t elementDataSizeWithOffset = elementDataSizeNoOffset + offset;
        if (elementDataSizeWithOffset < elementDataSizeNoOffset)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kIntegerOverflow);
            return false;
        }

        // Related to possible test bug: https://github.com/KhronosGroup/WebGL/issues/3064
        if ((elementDataSizeWithOffset > static_cast<uint64_t>(elementArrayBuffer->getSize())) &&
            (primcount > 0))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kInsufficientBufferSize);
            return false;
        }
    }

    if (context->isBufferAccessValidationEnabled() && primcount > 0)
    {
        // Use the parameter buffer to retrieve and cache the index range.
        // TODO: this calculation should take basevertex into account for
        // glDrawElementsInstancedBaseVertexBaseInstanceEXT.  http://anglebug.com/41481166
        IndexRange indexRange{IndexRange::Undefined()};
        ANGLE_VALIDATION_TRY(vao->getIndexRange(context, type, count, indices, &indexRange));

        // If we use an index greater than our maximum supported index range, return an error.
        // The ES3 spec does not specify behaviour here, it is undefined, but ANGLE should
        // always return an error if possible here.
        if (static_cast<GLint64>(indexRange.end) >= context->getCaps().maxElementIndex)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kExceedsMaxElement);
            return false;
        }

        if (!ValidateDrawAttribs(context, entryPoint, static_cast<GLint>(indexRange.end)))
        {
            return false;
        }

        // No op if there are no real indices in the index data (all are primitive restart).
        return (indexRange.vertexIndexCount > 0);
    }

    return true;
}

ANGLE_INLINE bool ValidateBindVertexArrayBase(const Context *context,
                                              angle::EntryPoint entryPoint,
                                              VertexArrayID array)
{
    if (!context->isVertexArrayGenerated(array))
    {
        // The default VAO should always exist
        ASSERT(array.value != 0);
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kInvalidVertexArray);
        return false;
    }

    return true;
}

ANGLE_INLINE bool ValidateVertexAttribIndex(const PrivateState &state,
                                            ErrorSet *errors,
                                            angle::EntryPoint entryPoint,
                                            GLuint index)
{
    if (index >= static_cast<GLuint>(state.getCaps().maxVertexAttributes))
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, err::kIndexExceedsMaxVertexAttribute);
        return false;
    }

    return true;
}

bool ValidateLogicOpCommon(const PrivateState &state,
                           ErrorSet *errors,
                           angle::EntryPoint entryPoint,
                           LogicalOperation opcodePacked);
}  // namespace gl

#endif  // LIBANGLE_VALIDATION_ES_H_
