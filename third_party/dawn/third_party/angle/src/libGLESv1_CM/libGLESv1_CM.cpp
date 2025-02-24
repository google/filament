//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// libGLESv1_CM.cpp: Implements the exported OpenGL ES 1.0 functions.

#include "angle_gl.h"

#include "libGLESv2/entry_points_gles_1_0_autogen.h"
#include "libGLESv2/entry_points_gles_2_0_autogen.h"
#include "libGLESv2/entry_points_gles_3_2_autogen.h"
#include "libGLESv2/entry_points_gles_ext_autogen.h"

extern "C" {

void GL_APIENTRY glAlphaFunc(GLenum func, GLfloat ref)
{
    return GL_AlphaFunc(func, ref);
}

void GL_APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    return GL_ClearColor(red, green, blue, alpha);
}

void GL_APIENTRY glClearDepthf(GLfloat d)
{
    return GL_ClearDepthf(d);
}

void GL_APIENTRY glClipPlanef(GLenum p, const GLfloat *eqn)
{
    return GL_ClipPlanef(p, eqn);
}

void GL_APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    return GL_Color4f(red, green, blue, alpha);
}

void GL_APIENTRY glDepthRangef(GLfloat n, GLfloat f)
{
    return GL_DepthRangef(n, f);
}

void GL_APIENTRY glFogf(GLenum pname, GLfloat param)
{
    return GL_Fogf(pname, param);
}

void GL_APIENTRY glFogfv(GLenum pname, const GLfloat *params)
{
    return GL_Fogfv(pname, params);
}

void GL_APIENTRY glFrustumf(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f)
{
    return GL_Frustumf(l, r, b, t, n, f);
}

void GL_APIENTRY glGetClipPlanef(GLenum plane, GLfloat *equation)
{
    return GL_GetClipPlanef(plane, equation);
}

void GL_APIENTRY glGetFloatv(GLenum pname, GLfloat *data)
{
    return GL_GetFloatv(pname, data);
}

void GL_APIENTRY glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
    return GL_GetLightfv(light, pname, params);
}

void GL_APIENTRY glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
    return GL_GetMaterialfv(face, pname, params);
}

void GL_APIENTRY glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
    return GL_GetTexEnvfv(target, pname, params);
}

void GL_APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    return GL_GetTexParameterfv(target, pname, params);
}

void GL_APIENTRY glLightModelf(GLenum pname, GLfloat param)
{
    return GL_LightModelf(pname, param);
}

void GL_APIENTRY glLightModelfv(GLenum pname, const GLfloat *params)
{
    return GL_LightModelfv(pname, params);
}

void GL_APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param)
{
    return GL_Lightf(light, pname, param);
}

void GL_APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    return GL_Lightfv(light, pname, params);
}

void GL_APIENTRY glLineWidth(GLfloat width)
{
    return GL_LineWidth(width);
}

void GL_APIENTRY glLoadMatrixf(const GLfloat *m)
{
    return GL_LoadMatrixf(m);
}

void GL_APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
    return GL_Materialf(face, pname, param);
}

void GL_APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    return GL_Materialfv(face, pname, params);
}

void GL_APIENTRY glMultMatrixf(const GLfloat *m)
{
    return GL_MultMatrixf(m);
}

void GL_APIENTRY glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    return GL_MultiTexCoord4f(target, s, t, r, q);
}

void GL_APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    return GL_Normal3f(nx, ny, nz);
}

void GL_APIENTRY glOrthof(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f)
{
    return GL_Orthof(l, r, b, t, n, f);
}

void GL_APIENTRY glPointParameterf(GLenum pname, GLfloat param)
{
    return GL_PointParameterf(pname, param);
}

void GL_APIENTRY glPointParameterfv(GLenum pname, const GLfloat *params)
{
    return GL_PointParameterfv(pname, params);
}

void GL_APIENTRY glPointSize(GLfloat size)
{
    return GL_PointSize(size);
}

void GL_APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
    return GL_PolygonOffset(factor, units);
}

void GL_APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    return GL_Rotatef(angle, x, y, z);
}

void GL_APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    return GL_Scalef(x, y, z);
}

void GL_APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    return GL_TexEnvf(target, pname, param);
}

void GL_APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    return GL_TexEnvfv(target, pname, params);
}

void GL_APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    return GL_TexParameterf(target, pname, param);
}

void GL_APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    return GL_TexParameterfv(target, pname, params);
}

void GL_APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    return GL_Translatef(x, y, z);
}

void GL_APIENTRY glActiveTexture(GLenum texture)
{
    return GL_ActiveTexture(texture);
}

void GL_APIENTRY glAlphaFuncx(GLenum func, GLfixed ref)
{
    return GL_AlphaFuncx(func, ref);
}

void GL_APIENTRY glBindBuffer(GLenum target, GLuint buffer)
{
    return GL_BindBuffer(target, buffer);
}

void GL_APIENTRY glBindTexture(GLenum target, GLuint texture)
{
    return GL_BindTexture(target, texture);
}

void GL_APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    return GL_BlendFunc(sfactor, dfactor);
}

void GL_APIENTRY glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
    return GL_BufferData(target, size, data, usage);
}

void GL_APIENTRY glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data)
{
    return GL_BufferSubData(target, offset, size, data);
}

void GL_APIENTRY glClear(GLbitfield mask)
{
    return GL_Clear(mask);
}

void GL_APIENTRY glClearColorx(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    return GL_ClearColorx(red, green, blue, alpha);
}

void GL_APIENTRY glClearDepthx(GLfixed depth)
{
    return GL_ClearDepthx(depth);
}

void GL_APIENTRY glClearStencil(GLint s)
{
    return GL_ClearStencil(s);
}

void GL_APIENTRY glClientActiveTexture(GLenum texture)
{
    return GL_ClientActiveTexture(texture);
}

void GL_APIENTRY glClipPlanex(GLenum plane, const GLfixed *equation)
{
    return GL_ClipPlanex(plane, equation);
}

void GL_APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    return GL_Color4ub(red, green, blue, alpha);
}

void GL_APIENTRY glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    return GL_Color4x(red, green, blue, alpha);
}

void GL_APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    return GL_ColorMask(red, green, blue, alpha);
}

void GL_APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    return GL_ColorPointer(size, type, stride, pointer);
}

void GL_APIENTRY glCompressedTexImage2D(GLenum target,
                                        GLint level,
                                        GLenum internalformat,
                                        GLsizei width,
                                        GLsizei height,
                                        GLint border,
                                        GLsizei imageSize,
                                        const void *data)
{
    return GL_CompressedTexImage2D(target, level, internalformat, width, height, border, imageSize,
                                   data);
}

void GL_APIENTRY glCompressedTexSubImage2D(GLenum target,
                                           GLint level,
                                           GLint xoffset,
                                           GLint yoffset,
                                           GLsizei width,
                                           GLsizei height,
                                           GLenum format,
                                           GLsizei imageSize,
                                           const void *data)
{
    return GL_CompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format,
                                      imageSize, data);
}

void GL_APIENTRY glCopyTexImage2D(GLenum target,
                                  GLint level,
                                  GLenum internalformat,
                                  GLint x,
                                  GLint y,
                                  GLsizei width,
                                  GLsizei height,
                                  GLint border)
{
    return GL_CopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

void GL_APIENTRY glCopyTexSubImage2D(GLenum target,
                                     GLint level,
                                     GLint xoffset,
                                     GLint yoffset,
                                     GLint x,
                                     GLint y,
                                     GLsizei width,
                                     GLsizei height)
{
    return GL_CopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

void GL_APIENTRY glCullFace(GLenum mode)
{
    return GL_CullFace(mode);
}

void GL_APIENTRY glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    return GL_DeleteBuffers(n, buffers);
}

void GL_APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures)
{
    return GL_DeleteTextures(n, textures);
}

void GL_APIENTRY glDepthFunc(GLenum func)
{
    return GL_DepthFunc(func);
}

void GL_APIENTRY glDepthMask(GLboolean flag)
{
    return GL_DepthMask(flag);
}

void GL_APIENTRY glDepthRangex(GLfixed n, GLfixed f)
{
    return GL_DepthRangex(n, f);
}

void GL_APIENTRY glDisable(GLenum cap)
{
    return GL_Disable(cap);
}

void GL_APIENTRY glDisableClientState(GLenum array)
{
    return GL_DisableClientState(array);
}

void GL_APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    return GL_DrawArrays(mode, first, count);
}

void GL_APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    return GL_DrawElements(mode, count, type, indices);
}

void GL_APIENTRY glEnable(GLenum cap)
{
    return GL_Enable(cap);
}

void GL_APIENTRY glEnableClientState(GLenum array)
{
    return GL_EnableClientState(array);
}

void GL_APIENTRY glFinish(void)
{
    return GL_Finish();
}

void GL_APIENTRY glFlush(void)
{
    return GL_Flush();
}

void GL_APIENTRY glFogx(GLenum pname, GLfixed param)
{
    return GL_Fogx(pname, param);
}

void GL_APIENTRY glFogxv(GLenum pname, const GLfixed *param)
{
    return GL_Fogxv(pname, param);
}

void GL_APIENTRY glFrontFace(GLenum mode)
{
    return GL_FrontFace(mode);
}

void GL_APIENTRY glFrustumx(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)
{
    return GL_Frustumx(l, r, b, t, n, f);
}

void GL_APIENTRY glGetBooleanv(GLenum pname, GLboolean *data)
{
    return GL_GetBooleanv(pname, data);
}

void GL_APIENTRY glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    return GL_GetBufferParameteriv(target, pname, params);
}

void GL_APIENTRY glGetClipPlanex(GLenum plane, GLfixed *equation)
{
    return GL_GetClipPlanex(plane, equation);
}

void GL_APIENTRY glGenBuffers(GLsizei n, GLuint *buffers)
{
    return GL_GenBuffers(n, buffers);
}

void GL_APIENTRY glGenTextures(GLsizei n, GLuint *textures)
{
    return GL_GenTextures(n, textures);
}

GLenum GL_APIENTRY glGetError(void)
{
    return GL_GetError();
}

void GL_APIENTRY glGetFixedv(GLenum pname, GLfixed *params)
{
    return GL_GetFixedv(pname, params);
}

void GL_APIENTRY glGetIntegerv(GLenum pname, GLint *data)
{
    return GL_GetIntegerv(pname, data);
}

void GL_APIENTRY glGetLightxv(GLenum light, GLenum pname, GLfixed *params)
{
    return GL_GetLightxv(light, pname, params);
}

void GL_APIENTRY glGetMaterialxv(GLenum face, GLenum pname, GLfixed *params)
{
    return GL_GetMaterialxv(face, pname, params);
}

void GL_APIENTRY glGetPointerv(GLenum pname, void **params)
{
    return GL_GetPointerv(pname, params);
}

const GLubyte *GL_APIENTRY glGetString(GLenum name)
{
    return GL_GetString(name);
}

void GL_APIENTRY glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
    return GL_GetTexEnviv(target, pname, params);
}

void GL_APIENTRY glGetTexEnvxv(GLenum target, GLenum pname, GLfixed *params)
{
    return GL_GetTexEnvxv(target, pname, params);
}

void GL_APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    return GL_GetTexParameteriv(target, pname, params);
}

void GL_APIENTRY glGetTexParameterxv(GLenum target, GLenum pname, GLfixed *params)
{
    return GL_GetTexParameterxv(target, pname, params);
}

void GL_APIENTRY glHint(GLenum target, GLenum mode)
{
    return GL_Hint(target, mode);
}

GLboolean GL_APIENTRY glIsBuffer(GLuint buffer)
{
    return GL_IsBuffer(buffer);
}

GLboolean GL_APIENTRY glIsEnabled(GLenum cap)
{
    return GL_IsEnabled(cap);
}

GLboolean GL_APIENTRY glIsTexture(GLuint texture)
{
    return GL_IsTexture(texture);
}

void GL_APIENTRY glLightModelx(GLenum pname, GLfixed param)
{
    return GL_LightModelx(pname, param);
}

void GL_APIENTRY glLightModelxv(GLenum pname, const GLfixed *param)
{
    return GL_LightModelxv(pname, param);
}

void GL_APIENTRY glLightx(GLenum light, GLenum pname, GLfixed param)
{
    return GL_Lightx(light, pname, param);
}

void GL_APIENTRY glLightxv(GLenum light, GLenum pname, const GLfixed *params)
{
    return GL_Lightxv(light, pname, params);
}

void GL_APIENTRY glLineWidthx(GLfixed width)
{
    return GL_LineWidthx(width);
}

void GL_APIENTRY glLoadIdentity(void)
{
    return GL_LoadIdentity();
}

void GL_APIENTRY glLoadMatrixx(const GLfixed *m)
{
    return GL_LoadMatrixx(m);
}

void GL_APIENTRY glLogicOp(GLenum opcode)
{
    return GL_LogicOp(opcode);
}

void GL_APIENTRY glMaterialx(GLenum face, GLenum pname, GLfixed param)
{
    return GL_Materialx(face, pname, param);
}

void GL_APIENTRY glMaterialxv(GLenum face, GLenum pname, const GLfixed *param)
{
    return GL_Materialxv(face, pname, param);
}

void GL_APIENTRY glMatrixMode(GLenum mode)
{
    return GL_MatrixMode(mode);
}

void GL_APIENTRY glMultMatrixx(const GLfixed *m)
{
    return GL_MultMatrixx(m);
}

void GL_APIENTRY glMultiTexCoord4x(GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
    return GL_MultiTexCoord4x(texture, s, t, r, q);
}

void GL_APIENTRY glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
    return GL_Normal3x(nx, ny, nz);
}

void GL_APIENTRY glNormalPointer(GLenum type, GLsizei stride, const void *pointer)
{
    return GL_NormalPointer(type, stride, pointer);
}

void GL_APIENTRY glOrthox(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)
{
    return GL_Orthox(l, r, b, t, n, f);
}

void GL_APIENTRY glPixelStorei(GLenum pname, GLint param)
{
    return GL_PixelStorei(pname, param);
}

void GL_APIENTRY glPointParameterx(GLenum pname, GLfixed param)
{
    return GL_PointParameterx(pname, param);
}

void GL_APIENTRY glPointParameterxv(GLenum pname, const GLfixed *params)
{
    return GL_PointParameterxv(pname, params);
}

void GL_APIENTRY glPointSizex(GLfixed size)
{
    return GL_PointSizex(size);
}

void GL_APIENTRY glPolygonOffsetx(GLfixed factor, GLfixed units)
{
    return GL_PolygonOffsetx(factor, units);
}

void GL_APIENTRY glPopMatrix(void)
{
    return GL_PopMatrix();
}

void GL_APIENTRY glPushMatrix(void)
{
    return GL_PushMatrix();
}

void GL_APIENTRY glReadPixels(GLint x,
                              GLint y,
                              GLsizei width,
                              GLsizei height,
                              GLenum format,
                              GLenum type,
                              void *pixels)
{
    return GL_ReadPixels(x, y, width, height, format, type, pixels);
}

void GL_APIENTRY glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
    return GL_Rotatex(angle, x, y, z);
}

void GL_APIENTRY glSampleCoverage(GLfloat value, GLboolean invert)
{
    return GL_SampleCoverage(value, invert);
}

void GL_APIENTRY glSampleCoveragex(GLclampx value, GLboolean invert)
{
    return GL_SampleCoveragex(value, invert);
}

void GL_APIENTRY glScalex(GLfixed x, GLfixed y, GLfixed z)
{
    return GL_Scalex(x, y, z);
}

void GL_APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    return GL_Scissor(x, y, width, height);
}

void GL_APIENTRY glShadeModel(GLenum mode)
{
    return GL_ShadeModel(mode);
}

void GL_APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    return GL_StencilFunc(func, ref, mask);
}

void GL_APIENTRY glStencilMask(GLuint mask)
{
    return GL_StencilMask(mask);
}

void GL_APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    return GL_StencilOp(fail, zfail, zpass);
}

void GL_APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    return GL_TexCoordPointer(size, type, stride, pointer);
}

void GL_APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    return GL_TexEnvi(target, pname, param);
}

void GL_APIENTRY glTexEnvx(GLenum target, GLenum pname, GLfixed param)
{
    return GL_TexEnvx(target, pname, param);
}

void GL_APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
    return GL_TexEnviv(target, pname, params);
}

void GL_APIENTRY glTexEnvxv(GLenum target, GLenum pname, const GLfixed *params)
{
    return GL_TexEnvxv(target, pname, params);
}

void GL_APIENTRY glTexImage2D(GLenum target,
                              GLint level,
                              GLint internalformat,
                              GLsizei width,
                              GLsizei height,
                              GLint border,
                              GLenum format,
                              GLenum type,
                              const void *pixels)
{
    return GL_TexImage2D(target, level, internalformat, width, height, border, format, type,
                         pixels);
}

void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    return GL_TexParameteri(target, pname, param);
}

void GL_APIENTRY glTexParameterx(GLenum target, GLenum pname, GLfixed param)
{
    return GL_TexParameterx(target, pname, param);
}

void GL_APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    return GL_TexParameteriv(target, pname, params);
}

void GL_APIENTRY glTexParameterxv(GLenum target, GLenum pname, const GLfixed *params)
{
    return GL_TexParameterxv(target, pname, params);
}

void GL_APIENTRY glTexSubImage2D(GLenum target,
                                 GLint level,
                                 GLint xoffset,
                                 GLint yoffset,
                                 GLsizei width,
                                 GLsizei height,
                                 GLenum format,
                                 GLenum type,
                                 const void *pixels)
{
    return GL_TexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void GL_APIENTRY glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
    return GL_Translatex(x, y, z);
}

void GL_APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    return GL_VertexPointer(size, type, stride, pointer);
}

void GL_APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    return GL_Viewport(x, y, width, height);
}

// GL_OES_draw_texture
void GL_APIENTRY glDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height)
{
    return GL_DrawTexfOES(x, y, z, width, height);
}

void GL_APIENTRY glDrawTexfvOES(const GLfloat *coords)
{
    return GL_DrawTexfvOES(coords);
}

void GL_APIENTRY glDrawTexiOES(GLint x, GLint y, GLint z, GLint width, GLint height)
{
    return GL_DrawTexiOES(x, y, z, width, height);
}

void GL_APIENTRY glDrawTexivOES(const GLint *coords)
{
    return GL_DrawTexivOES(coords);
}

void GL_APIENTRY glDrawTexsOES(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
    return GL_DrawTexsOES(x, y, z, width, height);
}

void GL_APIENTRY glDrawTexsvOES(const GLshort *coords)
{
    return GL_DrawTexsvOES(coords);
}

void GL_APIENTRY glDrawTexxOES(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
    return GL_DrawTexxOES(x, y, z, width, height);
}

void GL_APIENTRY glDrawTexxvOES(const GLfixed *coords)
{
    return GL_DrawTexxvOES(coords);
}

// GL_OES_matrix_palette
void GL_APIENTRY glCurrentPaletteMatrixOES(GLuint matrixpaletteindex)
{
    return GL_CurrentPaletteMatrixOES(matrixpaletteindex);
}

void GL_APIENTRY glLoadPaletteFromModelViewMatrixOES()
{
    return GL_LoadPaletteFromModelViewMatrixOES();
}

void GL_APIENTRY glMatrixIndexPointerOES(GLint size,
                                         GLenum type,
                                         GLsizei stride,
                                         const void *pointer)
{
    return GL_MatrixIndexPointerOES(size, type, stride, pointer);
}

void GL_APIENTRY glWeightPointerOES(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    return GL_WeightPointerOES(size, type, stride, pointer);
}

// GL_OES_point_size_array
void GL_APIENTRY glPointSizePointerOES(GLenum type, GLsizei stride, const void *pointer)
{
    return GL_PointSizePointerOES(type, stride, pointer);
}

// GL_OES_query_matrix
GLbitfield GL_APIENTRY glQueryMatrixxOES(GLfixed *mantissa, GLint *exponent)
{
    return GL_QueryMatrixxOES(mantissa, exponent);
}

}  // extern "C"
