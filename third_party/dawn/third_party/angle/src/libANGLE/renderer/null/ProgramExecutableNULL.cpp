//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramExecutableNULL.cpp: Implementation of ProgramExecutableNULL.

#include "libANGLE/renderer/null/ProgramExecutableNULL.h"

namespace rx
{
ProgramExecutableNULL::ProgramExecutableNULL(const gl::ProgramExecutable *executable)
    : ProgramExecutableImpl(executable)
{}

ProgramExecutableNULL::~ProgramExecutableNULL() = default;

void ProgramExecutableNULL::destroy(const gl::Context *context) {}

void ProgramExecutableNULL::setUniform1fv(GLint location, GLsizei count, const GLfloat *v) {}

void ProgramExecutableNULL::setUniform2fv(GLint location, GLsizei count, const GLfloat *v) {}

void ProgramExecutableNULL::setUniform3fv(GLint location, GLsizei count, const GLfloat *v) {}

void ProgramExecutableNULL::setUniform4fv(GLint location, GLsizei count, const GLfloat *v) {}

void ProgramExecutableNULL::setUniform1iv(GLint location, GLsizei count, const GLint *v) {}

void ProgramExecutableNULL::setUniform2iv(GLint location, GLsizei count, const GLint *v) {}

void ProgramExecutableNULL::setUniform3iv(GLint location, GLsizei count, const GLint *v) {}

void ProgramExecutableNULL::setUniform4iv(GLint location, GLsizei count, const GLint *v) {}

void ProgramExecutableNULL::setUniform1uiv(GLint location, GLsizei count, const GLuint *v) {}

void ProgramExecutableNULL::setUniform2uiv(GLint location, GLsizei count, const GLuint *v) {}

void ProgramExecutableNULL::setUniform3uiv(GLint location, GLsizei count, const GLuint *v) {}

void ProgramExecutableNULL::setUniform4uiv(GLint location, GLsizei count, const GLuint *v) {}

void ProgramExecutableNULL::setUniformMatrix2fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{}

void ProgramExecutableNULL::setUniformMatrix3fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{}

void ProgramExecutableNULL::setUniformMatrix4fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{}

void ProgramExecutableNULL::setUniformMatrix2x3fv(GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{}

void ProgramExecutableNULL::setUniformMatrix3x2fv(GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{}

void ProgramExecutableNULL::setUniformMatrix2x4fv(GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{}

void ProgramExecutableNULL::setUniformMatrix4x2fv(GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{}

void ProgramExecutableNULL::setUniformMatrix3x4fv(GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{}

void ProgramExecutableNULL::setUniformMatrix4x3fv(GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{}

void ProgramExecutableNULL::getUniformfv(const gl::Context *context,
                                         GLint location,
                                         GLfloat *params) const
{
    // TODO: Write some values.
}

void ProgramExecutableNULL::getUniformiv(const gl::Context *context,
                                         GLint location,
                                         GLint *params) const
{
    // TODO: Write some values.
}

void ProgramExecutableNULL::getUniformuiv(const gl::Context *context,
                                          GLint location,
                                          GLuint *params) const
{
    // TODO: Write some values.
}
}  // namespace rx
