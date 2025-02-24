//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramExecutableImpl.h: Defines the abstract rx::ProgramExecutableImpl class.

#ifndef LIBANGLE_RENDERER_PROGRAMEXECUTABLEIMPL_H_
#define LIBANGLE_RENDERER_PROGRAMEXECUTABLEIMPL_H_

#include "common/angleutils.h"

namespace gl
{
class Context;
class ProgramExecutable;
}  // namespace gl

namespace rx
{
// ProgramExecutable holds the result of link.  The backend ProgramExecutable* classes similarly
// hold additonaly backend-specific link results.  A program's executable is changed on successful
// link.  This allows the program to continue to work with its existing executable despite a failed
// relink.
class ProgramExecutableImpl : angle::NonCopyable
{
  public:
    ProgramExecutableImpl(const gl::ProgramExecutable *executable) : mExecutable(executable) {}
    virtual ~ProgramExecutableImpl() {}

    virtual void destroy(const gl::Context *context) {}

    virtual void setUniform1fv(GLint location, GLsizei count, const GLfloat *v) = 0;
    virtual void setUniform2fv(GLint location, GLsizei count, const GLfloat *v) = 0;
    virtual void setUniform3fv(GLint location, GLsizei count, const GLfloat *v) = 0;
    virtual void setUniform4fv(GLint location, GLsizei count, const GLfloat *v) = 0;
    virtual void setUniform1iv(GLint location, GLsizei count, const GLint *v)   = 0;
    virtual void setUniform2iv(GLint location, GLsizei count, const GLint *v)   = 0;
    virtual void setUniform3iv(GLint location, GLsizei count, const GLint *v)   = 0;
    virtual void setUniform4iv(GLint location, GLsizei count, const GLint *v)   = 0;
    virtual void setUniform1uiv(GLint location, GLsizei count, const GLuint *v) = 0;
    virtual void setUniform2uiv(GLint location, GLsizei count, const GLuint *v) = 0;
    virtual void setUniform3uiv(GLint location, GLsizei count, const GLuint *v) = 0;
    virtual void setUniform4uiv(GLint location, GLsizei count, const GLuint *v) = 0;
    virtual void setUniformMatrix2fv(GLint location,
                                     GLsizei count,
                                     GLboolean transpose,
                                     const GLfloat *value)                      = 0;
    virtual void setUniformMatrix3fv(GLint location,
                                     GLsizei count,
                                     GLboolean transpose,
                                     const GLfloat *value)                      = 0;
    virtual void setUniformMatrix4fv(GLint location,
                                     GLsizei count,
                                     GLboolean transpose,
                                     const GLfloat *value)                      = 0;
    virtual void setUniformMatrix2x3fv(GLint location,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)                    = 0;
    virtual void setUniformMatrix3x2fv(GLint location,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)                    = 0;
    virtual void setUniformMatrix2x4fv(GLint location,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)                    = 0;
    virtual void setUniformMatrix4x2fv(GLint location,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)                    = 0;
    virtual void setUniformMatrix3x4fv(GLint location,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)                    = 0;
    virtual void setUniformMatrix4x3fv(GLint location,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)                    = 0;

    // Done in the back-end to avoid having to keep a system copy of uniform data.
    virtual void getUniformfv(const gl::Context *context,
                              GLint location,
                              GLfloat *params) const                                           = 0;
    virtual void getUniformiv(const gl::Context *context, GLint location, GLint *params) const = 0;
    virtual void getUniformuiv(const gl::Context *context,
                               GLint location,
                               GLuint *params) const                                           = 0;
    // Optional. Implement in backends that fill |postLinkSubTasksOut| in |LinkTask|.
    virtual void waitForPostLinkTasks(const gl::Context *context) { UNIMPLEMENTED(); }
    const gl::ProgramExecutable *getExecutable() const { return mExecutable; }

  protected:
    const gl::ProgramExecutable *mExecutable;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_PROGRAMEXECUTABLEIMPL_H_
