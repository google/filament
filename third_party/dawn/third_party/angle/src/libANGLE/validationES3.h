//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// validationES3.h:
//  Inlined validation functions for OpenGL ES 3.0 entry points.

#ifndef LIBANGLE_VALIDATION_ES3_H_
#define LIBANGLE_VALIDATION_ES3_H_

#include "libANGLE/ErrorStrings.h"
#include "libANGLE/validationES3_autogen.h"

namespace gl
{
bool ValidateTexImageFormatCombination(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       TextureType target,
                                       GLenum internalFormat,
                                       GLenum format,
                                       GLenum type);

bool ValidateES3TexImageParametersBase(const Context *context,
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

bool ValidateES3TexStorageParametersLevel(const Context *context,
                                          angle::EntryPoint entryPoint,
                                          TextureType target,
                                          GLsizei levels,
                                          GLsizei width,
                                          GLsizei height,
                                          GLsizei depth);

bool ValidateES3TexStorageParametersExtent(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           TextureType target,
                                           GLsizei levels,
                                           GLsizei width,
                                           GLsizei height,
                                           GLsizei depth);

bool ValidateES3TexStorageParametersTexObject(const Context *context,
                                              angle::EntryPoint entryPoint,
                                              TextureType target);

bool ValidateES3TexStorageParametersFormat(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           TextureType target,
                                           GLsizei levels,
                                           GLenum internalformat,
                                           GLsizei width,
                                           GLsizei height,
                                           GLsizei depth);

bool ValidateProgramParameteriBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   GLenum pname,
                                   GLint value);
}  // namespace gl

#endif  // LIBANGLE_VALIDATION_ES3_H_
