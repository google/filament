//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// validationES2.h:
//  Inlined validation functions for OpenGL ES 2.0 entry points.

#ifndef LIBANGLE_VALIDATION_ES2_H_
#define LIBANGLE_VALIDATION_ES2_H_

#include "libANGLE/ErrorStrings.h"
#include "libANGLE/validationES.h"
#include "libANGLE/validationES2_autogen.h"

namespace gl
{
ANGLE_INLINE bool ValidateDrawArrays(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     PrimitiveMode mode,
                                     GLint first,
                                     GLsizei count)
{
    return ValidateDrawArraysCommon(context, entryPoint, mode, first, count, 1);
}

ANGLE_INLINE bool ValidateUniform1f(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    UniformLocation location,
                                    GLfloat x)
{
    return ValidateUniform(context, entryPoint, GL_FLOAT, location, 1);
}

ANGLE_INLINE bool ValidateUniform1fv(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     UniformLocation location,
                                     GLsizei count,
                                     const GLfloat *v)
{
    return ValidateUniform(context, entryPoint, GL_FLOAT, location, count);
}

ANGLE_INLINE bool ValidateUniform1i(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    UniformLocation location,
                                    GLint x)
{
    return ValidateUniform1iv(context, entryPoint, location, 1, &x);
}

ANGLE_INLINE bool ValidateUniform2f(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    UniformLocation location,
                                    GLfloat x,
                                    GLfloat y)
{
    return ValidateUniform(context, entryPoint, GL_FLOAT_VEC2, location, 1);
}

ANGLE_INLINE bool ValidateUniform2fv(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     UniformLocation location,
                                     GLsizei count,
                                     const GLfloat *v)
{
    return ValidateUniform(context, entryPoint, GL_FLOAT_VEC2, location, count);
}

ANGLE_INLINE bool ValidateUniform2i(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    UniformLocation location,
                                    GLint x,
                                    GLint y)
{
    return ValidateUniform(context, entryPoint, GL_INT_VEC2, location, 1);
}

ANGLE_INLINE bool ValidateUniform2iv(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     UniformLocation location,
                                     GLsizei count,
                                     const GLint *v)
{
    return ValidateUniform(context, entryPoint, GL_INT_VEC2, location, count);
}

ANGLE_INLINE bool ValidateUniform3f(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    UniformLocation location,
                                    GLfloat x,
                                    GLfloat y,
                                    GLfloat z)
{
    return ValidateUniform(context, entryPoint, GL_FLOAT_VEC3, location, 1);
}

ANGLE_INLINE bool ValidateUniform3fv(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     UniformLocation location,
                                     GLsizei count,
                                     const GLfloat *v)
{
    return ValidateUniform(context, entryPoint, GL_FLOAT_VEC3, location, count);
}

ANGLE_INLINE bool ValidateUniform3i(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    UniformLocation location,
                                    GLint x,
                                    GLint y,
                                    GLint z)
{
    return ValidateUniform(context, entryPoint, GL_INT_VEC3, location, 1);
}

ANGLE_INLINE bool ValidateUniform3iv(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     UniformLocation location,
                                     GLsizei count,
                                     const GLint *v)
{
    return ValidateUniform(context, entryPoint, GL_INT_VEC3, location, count);
}

ANGLE_INLINE bool ValidateUniform4f(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    UniformLocation location,
                                    GLfloat x,
                                    GLfloat y,
                                    GLfloat z,
                                    GLfloat w)
{
    return ValidateUniform(context, entryPoint, GL_FLOAT_VEC4, location, 1);
}

ANGLE_INLINE bool ValidateUniform4fv(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     UniformLocation location,
                                     GLsizei count,
                                     const GLfloat *v)
{
    return ValidateUniform(context, entryPoint, GL_FLOAT_VEC4, location, count);
}

ANGLE_INLINE bool ValidateUniform4i(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    UniformLocation location,
                                    GLint x,
                                    GLint y,
                                    GLint z,
                                    GLint w)
{
    return ValidateUniform(context, entryPoint, GL_INT_VEC4, location, 1);
}

ANGLE_INLINE bool ValidateUniform4iv(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     UniformLocation location,
                                     GLsizei count,
                                     const GLint *v)
{
    return ValidateUniform(context, entryPoint, GL_INT_VEC4, location, count);
}

ANGLE_INLINE bool ValidateUniformMatrix2fv(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           UniformLocation location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    return ValidateUniformMatrix(context, entryPoint, GL_FLOAT_MAT2, location, count, transpose);
}

ANGLE_INLINE bool ValidateUniformMatrix3fv(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           UniformLocation location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    return ValidateUniformMatrix(context, entryPoint, GL_FLOAT_MAT3, location, count, transpose);
}

ANGLE_INLINE bool ValidateUniformMatrix4fv(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           UniformLocation location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    return ValidateUniformMatrix(context, entryPoint, GL_FLOAT_MAT4, location, count, transpose);
}

ANGLE_INLINE bool ValidateVertexAttrib1f(const PrivateState &state,
                                         ErrorSet *errors,
                                         angle::EntryPoint entryPoint,
                                         GLuint index,
                                         GLfloat x)
{
    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

ANGLE_INLINE bool ValidateVertexAttrib1fv(const PrivateState &state,
                                          ErrorSet *errors,
                                          angle::EntryPoint entryPoint,
                                          GLuint index,
                                          const GLfloat *values)
{
    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

ANGLE_INLINE bool ValidateVertexAttrib2f(const PrivateState &state,
                                         ErrorSet *errors,
                                         angle::EntryPoint entryPoint,
                                         GLuint index,
                                         GLfloat x,
                                         GLfloat y)
{
    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

ANGLE_INLINE bool ValidateVertexAttrib2fv(const PrivateState &state,
                                          ErrorSet *errors,
                                          angle::EntryPoint entryPoint,
                                          GLuint index,
                                          const GLfloat *values)
{
    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

ANGLE_INLINE bool ValidateVertexAttrib3f(const PrivateState &state,
                                         ErrorSet *errors,
                                         angle::EntryPoint entryPoint,
                                         GLuint index,
                                         GLfloat x,
                                         GLfloat y,
                                         GLfloat z)
{
    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

ANGLE_INLINE bool ValidateVertexAttrib3fv(const PrivateState &state,
                                          ErrorSet *errors,
                                          angle::EntryPoint entryPoint,
                                          GLuint index,
                                          const GLfloat *values)
{
    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

ANGLE_INLINE bool ValidateVertexAttrib4f(const PrivateState &state,
                                         ErrorSet *errors,
                                         angle::EntryPoint entryPoint,
                                         GLuint index,
                                         GLfloat x,
                                         GLfloat y,
                                         GLfloat z,
                                         GLfloat w)
{
    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

ANGLE_INLINE bool ValidateVertexAttrib4fv(const PrivateState &state,
                                          ErrorSet *errors,
                                          angle::EntryPoint entryPoint,
                                          GLuint index,
                                          const GLfloat *values)
{
    return ValidateVertexAttribIndex(state, errors, entryPoint, index);
}

ANGLE_INLINE bool ValidateGenBuffers(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLint n,
                                     const BufferID *buffers)
{
    return ValidateGenOrDelete(context, entryPoint, n);
}

ANGLE_INLINE bool ValidateGenFramebuffers(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          GLint n,
                                          const FramebufferID *framebuffers)
{
    return ValidateGenOrDelete(context, entryPoint, n);
}

ANGLE_INLINE bool ValidateGenRenderbuffers(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           GLint n,
                                           const RenderbufferID *renderbuffers)
{
    return ValidateGenOrDelete(context, entryPoint, n);
}

ANGLE_INLINE bool ValidateGenTextures(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLint n,
                                      const TextureID *textures)
{
    return ValidateGenOrDelete(context, entryPoint, n);
}

ANGLE_INLINE bool ValidateGenerateMipmap(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target)
{
    return ValidateGenerateMipmapBase(context, entryPoint, target);
}

ANGLE_INLINE bool ValidateGetBufferParameteriv(const Context *context,
                                               angle::EntryPoint entryPoint,
                                               BufferBinding target,
                                               GLenum pname,
                                               const GLint *params)
{
    return ValidateGetBufferParameterBase(context, entryPoint, target, pname, false, nullptr);
}

ANGLE_INLINE bool ValidateGetRenderbufferParameteriv(const Context *context,
                                                     angle::EntryPoint entryPoint,
                                                     GLenum target,
                                                     GLenum pname,
                                                     const GLint *params)
{
    return ValidateGetRenderbufferParameterivBase(context, entryPoint, target, pname, nullptr);
}

ANGLE_INLINE bool ValidateGetShaderiv(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ShaderProgramID shader,
                                      GLenum pname,
                                      const GLint *params)
{
    return ValidateGetShaderivBase(context, entryPoint, shader, pname, nullptr);
}

ANGLE_INLINE bool ValidateGetTexParameterfv(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            TextureType target,
                                            GLenum pname,
                                            const GLfloat *params)
{
    return ValidateGetTexParameterBase(context, entryPoint, target, pname, nullptr);
}

ANGLE_INLINE bool ValidateGetTexParameteriv(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            TextureType target,
                                            GLenum pname,
                                            const GLint *params)
{
    return ValidateGetTexParameterBase(context, entryPoint, target, pname, nullptr);
}

ANGLE_INLINE bool ValidateGetUniformfv(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID program,
                                       UniformLocation location,
                                       const GLfloat *params)
{
    return ValidateGetUniformBase(context, entryPoint, program, location);
}

ANGLE_INLINE bool ValidateGetUniformiv(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID program,
                                       UniformLocation location,
                                       const GLint *params)
{
    return ValidateGetUniformBase(context, entryPoint, program, location);
}

ANGLE_INLINE bool ValidateGetVertexAttribfv(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            GLuint index,
                                            GLenum pname,
                                            const GLfloat *params)
{
    return ValidateGetVertexAttribBase(context, entryPoint, index, pname, nullptr, false, false);
}

ANGLE_INLINE bool ValidateGetVertexAttribiv(const Context *context,
                                            angle::EntryPoint entryPoint,
                                            GLuint index,
                                            GLenum pname,
                                            const GLint *params)
{
    return ValidateGetVertexAttribBase(context, entryPoint, index, pname, nullptr, false, false);
}

ANGLE_INLINE bool ValidateGetVertexAttribPointerv(const Context *context,
                                                  angle::EntryPoint entryPoint,
                                                  GLuint index,
                                                  GLenum pname,
                                                  void *const *pointer)
{
    return ValidateGetVertexAttribBase(context, entryPoint, index, pname, nullptr, true, false);
}

ANGLE_INLINE bool ValidateReadPixels(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLint x,
                                     GLint y,
                                     GLsizei width,
                                     GLsizei height,
                                     GLenum format,
                                     GLenum type,
                                     const void *pixels)
{
    return ValidateReadPixelsBase(context, entryPoint, x, y, width, height, format, type, -1,
                                  nullptr, nullptr, nullptr, pixels);
}

ANGLE_INLINE bool ValidateTexParameterf(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        TextureType target,
                                        GLenum pname,
                                        GLfloat param)
{
    return ValidateTexParameterBase(context, entryPoint, target, pname, -1, false, &param);
}

ANGLE_INLINE bool ValidateTexParameterfv(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target,
                                         GLenum pname,
                                         const GLfloat *params)
{
    return ValidateTexParameterBase(context, entryPoint, target, pname, -1, true, params);
}

ANGLE_INLINE bool ValidateTexParameteri(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        TextureType target,
                                        GLenum pname,
                                        GLint param)
{
    return ValidateTexParameterBase(context, entryPoint, target, pname, -1, false, &param);
}

ANGLE_INLINE bool ValidateTexParameteriv(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target,
                                         GLenum pname,
                                         const GLint *params)
{
    return ValidateTexParameterBase(context, entryPoint, target, pname, -1, true, params);
}

ANGLE_INLINE bool ValidateBindBuffer(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     BufferBinding target,
                                     BufferID buffer)
{
    if (!context->isValidBufferBinding(target))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, err::kInvalidBufferTypes);
        return false;
    }

    if (!context->getState().isBindGeneratesResourceEnabled() &&
        !context->isBufferGenerated(buffer))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kObjectNotGenerated);
        return false;
    }

    return true;
}

ANGLE_INLINE bool ValidateDrawElements(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       PrimitiveMode mode,
                                       GLsizei count,
                                       DrawElementsType type,
                                       const void *indices)
{
    return ValidateDrawElementsCommon(context, entryPoint, mode, count, type, indices, 1);
}

ANGLE_INLINE bool ValidateVertexAttribPointer(const Context *context,
                                              angle::EntryPoint entryPoint,
                                              GLuint index,
                                              GLint size,
                                              VertexAttribType type,
                                              GLboolean normalized,
                                              GLsizei stride,
                                              const void *ptr)
{
    if (!ValidateFloatVertexFormat(context, entryPoint, index, size, type))
    {
        return false;
    }

    if (stride < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, err::kNegativeStride);
        return false;
    }

    if (context->getClientVersion() >= ES_3_1)
    {
        const Caps &caps = context->getCaps();
        if (stride > caps.maxVertexAttribStride)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, err::kExceedsMaxVertexAttribStride);
            return false;
        }

        if (index >= static_cast<GLuint>(caps.maxVertexAttribBindings))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, err::kExceedsMaxVertexAttribBindings);
            return false;
        }
    }

    // [OpenGL ES 3.0.2] Section 2.8 page 24:
    // An INVALID_OPERATION error is generated when a non-zero vertex array object
    // is bound, zero is bound to the ARRAY_BUFFER buffer object binding point,
    // and the pointer argument is not NULL.
    bool nullBufferAllowed = context->getState().areClientArraysEnabled() &&
                             context->getState().getVertexArray()->id().value == 0;
    if (!nullBufferAllowed && context->getState().getTargetBuffer(BufferBinding::Array) == 0 &&
        ptr != nullptr)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kClientDataInVertexArray);
        return false;
    }

    if (context->isWebGL())
    {
        // WebGL 1.0 [Section 6.14] Fixed point support
        // The WebGL API does not support the GL_FIXED data type.
        if (type == VertexAttribType::Fixed)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, err::kFixedNotInWebGL);
            return false;
        }

        if (!ValidateWebGLVertexAttribPointer(context, entryPoint, type, normalized, stride, ptr,
                                              false))
        {
            return false;
        }
    }

    return true;
}

void RecordBindTextureTypeError(const Context *context,
                                angle::EntryPoint entryPoint,
                                TextureType target);

ANGLE_INLINE bool ValidateBindTexture(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      TextureType target,
                                      TextureID texture)
{
    if (!context->getStateCache().isValidBindTextureType(target))
    {
        RecordBindTextureTypeError(context, entryPoint, target);
        return false;
    }

    if (texture.value == 0)
    {
        return true;
    }

    Texture *textureObject = context->getTexture(texture);
    if (textureObject && textureObject->getType() != target)
    {
        ANGLE_VALIDATION_ERRORF(GL_INVALID_OPERATION, err::kTextureTargetMismatchWithLabel,
                                static_cast<uint8_t>(target),
                                static_cast<uint8_t>(textureObject->getType()),
                                textureObject->getLabel().c_str());
        return false;
    }

    if (!context->getState().isBindGeneratesResourceEnabled() &&
        !context->isTextureGenerated(texture))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, err::kObjectNotGenerated);
        return false;
    }

    return true;
}

// Validation of all Tex[Sub]Image2D parameters except TextureTarget.
bool ValidateES2TexImageParametersBase(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureTarget target,
                                       GLint level,
                                       GLenum internalformat,
                                       bool isCompressed,
                                       bool isSubImage,
                                       GLint xoffset,
                                       GLint yoffset,
                                       GLsizei width,
                                       GLsizei height,
                                       GLint border,
                                       GLenum format,
                                       GLenum type,
                                       GLsizei imageSize,
                                       const void *pixels);

// Validation of TexStorage*2DEXT
bool ValidateES2TexStorageParametersBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         TextureType target,
                                         GLsizei levels,
                                         GLenum internalformat,
                                         GLsizei width,
                                         GLsizei height);

// Validation of [Push,Pop]DebugGroup
bool ValidatePushDebugGroupBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLenum source,
                                GLuint id,
                                GLsizei length,
                                const GLchar *message);
bool ValidatePopDebugGroupBase(const Context *context, angle::EntryPoint entryPoint);

// Validation of ObjectLabel
bool ValidateObjectLabelBase(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLenum identifier,
                             GLuint name,
                             GLsizei length,
                             const GLchar *label);

// Validation of GetObjectLabel
bool ValidateGetObjectLabelBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLenum identifier,
                                GLuint name,
                                GLsizei bufSize,
                                const GLsizei *length,
                                const GLchar *label);

// Validation of ObjectPtrLabel
bool ValidateObjectPtrLabelBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                const void *ptr,
                                GLsizei length,
                                const GLchar *label);

// Validation of GetObjectPtrLabel
bool ValidateGetObjectPtrLabelBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   const void *ptr,
                                   GLsizei bufSize,
                                   const GLsizei *length,
                                   const GLchar *label);

}  // namespace gl

#endif  // LIBANGLE_VALIDATION_ES2_H_
