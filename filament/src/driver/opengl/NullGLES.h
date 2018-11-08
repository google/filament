/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_DRIVER_NULLGLES_H
#define TNT_FILAMENT_DRIVER_NULLGLES_H

/*
 * This is used for debugging (stubbing out GLES calls)
 *
 * Simply uncomment the line
 *   using namespace nullgles;
 *
 * below.
 *
 */

namespace filament {
namespace nullgles {

inline void glScissor(GLint, GLint, GLsizei, GLsizei) { }
inline void glViewport(GLint, GLint, GLsizei, GLsizei) { }
inline void glDepthRangef(GLfloat, GLfloat) { }
inline void glClearColor(GLfloat, GLfloat, GLfloat, GLfloat) { }
inline void glClearStencil(GLint) { }
inline void glClearDepthf(GLfloat) { }

inline void glBindBufferBase(GLenum, GLuint, GLuint) { }
inline void glBindVertexArray (GLuint) { }
inline void glBindTexture (GLenum, GLuint)   { }
inline void glBindBuffer (GLenum, GLuint) { }
inline void glBindFramebuffer (GLenum, GLuint)   { }
inline void glBindRenderbuffer (GLenum, GLuint) { }
inline void glUseProgram (GLuint)   { }

inline void glBufferData(GLenum, GLsizeiptr, const void *, GLenum) { }
inline void glBufferSubData(GLenum, GLintptr, GLsizeiptr, const void *) { }
inline void glCompressedTexSubImage2D(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void *) { }
inline void glTexSubImage2D(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *) { }
inline void glGenerateMipmap(GLenum) { }

inline void glVertexAttribPointer(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *) { }
inline void glDisableVertexAttribArray (GLuint) { }
inline void glEnableVertexAttribArray (GLuint) { }
inline void glDisable (GLenum cap) { }
inline void glEnable (GLenum cap) { }
inline void glCullFace (GLenum mode) { }
inline void glBlendFunc (GLenum, GLenum) { }

inline void glInvalidateFramebuffer (GLenum, GLsizei, const GLenum *) { }
inline void glInvalidateSubFramebuffer (GLenum, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei);

inline void glSamplerParameteri (GLuint, GLenum, GLint) { }
inline void glSamplerParameterf (GLuint, GLenum, GLfloat) { }

inline void glFramebufferRenderbuffer (GLenum, GLenum, GLenum, GLuint) { }
inline void glRenderbufferStorageMultisample (GLenum, GLsizei, GLenum, GLsizei, GLsizei) { }
inline void glRenderbufferStorage (GLenum, GLenum, GLsizei, GLsizei) { }

inline void glClear(GLbitfield) { }
inline void glDrawRangeElements(GLenum, GLuint, GLuint, GLsizei, GLenum, const void *)  { }
inline void glBlitFramebuffer (GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum) { }
inline void glReadPixels (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *) { }

inline GLenum glGetError() { return GL_NO_ERROR; }

} // namespace nullgles

// turn GLES calls defined above into no-ops
//using namespace nullgles;

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_NULLGLES_H
