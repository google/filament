//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Context_gles_1_0.cpp: Implements the GLES1-specific parts of Context.

#include "libANGLE/Context.h"

#include "common/mathutil.h"
#include "common/utilities.h"

#include "libANGLE/GLES1Renderer.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/queryutils.h"

namespace gl
{

void Context::clientActiveTexture(GLenum texture)
{
    getMutableGLES1State()->setClientTextureUnit(texture - GL_TEXTURE0);
    mStateCache.onGLES1ClientStateChange(this);
}

void Context::colorPointer(GLint size, VertexAttribType type, GLsizei stride, const void *ptr)
{
    // Note that we normalize data for UnsignedByte types. This is to match the behavior
    // of current native GLES drivers.
    vertexAttribPointer(vertexArrayIndex(ClientVertexArrayType::Color), size, type,
                        type == VertexAttribType::UnsignedByte, stride, ptr);
}

void Context::disableClientState(ClientVertexArrayType clientState)
{
    getMutableGLES1State()->setClientStateEnabled(clientState, false);
    disableVertexAttribArray(vertexArrayIndex(clientState));
    mStateCache.onGLES1ClientStateChange(this);
}

void Context::enableClientState(ClientVertexArrayType clientState)
{
    getMutableGLES1State()->setClientStateEnabled(clientState, true);
    enableVertexAttribArray(vertexArrayIndex(clientState));
    mStateCache.onGLES1ClientStateChange(this);
}

void Context::getFixedv(GLenum pname, GLfixed *params)
{
    GLenum nativeType;
    unsigned int numParams = 0;

    getQueryParameterInfo(pname, &nativeType, &numParams);

    std::vector<GLfloat> paramsf(numParams, 0);
    CastStateValues(this, nativeType, pname, numParams, paramsf.data());

    for (unsigned int i = 0; i < numParams; i++)
    {
        params[i] = ConvertFloatToFixed(paramsf[i]);
    }
}

void Context::getTexParameterxv(TextureType target, GLenum pname, GLfixed *params)
{
    const Texture *const texture = getTextureByType(target);
    QueryTexParameterxv(this, texture, pname, params);
}

void Context::normalPointer(VertexAttribType type, GLsizei stride, const void *ptr)
{
    vertexAttribPointer(vertexArrayIndex(ClientVertexArrayType::Normal), 3, type, GL_FALSE, stride,
                        ptr);
}

void Context::texCoordPointer(GLint size, VertexAttribType type, GLsizei stride, const void *ptr)
{
    vertexAttribPointer(vertexArrayIndex(ClientVertexArrayType::TextureCoord), size, type, GL_FALSE,
                        stride, ptr);
}

void Context::texParameterx(TextureType target, GLenum pname, GLfixed param)
{
    Texture *const texture = getTextureByType(target);
    SetTexParameterx(this, texture, pname, param);
}

void Context::texParameterxv(TextureType target, GLenum pname, const GLfixed *params)
{
    Texture *const texture = getTextureByType(target);
    SetTexParameterxv(this, texture, pname, params);
}

void Context::vertexPointer(GLint size, VertexAttribType type, GLsizei stride, const void *ptr)
{
    vertexAttribPointer(vertexArrayIndex(ClientVertexArrayType::Vertex), size, type, GL_FALSE,
                        stride, ptr);
}

// GL_OES_draw_texture
void Context::drawTexf(float x, float y, float z, float width, float height)
{
    mGLES1Renderer->drawTexture(this, &mState, getMutableGLES1State(), x, y, z, width, height);
}

void Context::drawTexfv(const GLfloat *coords)
{
    mGLES1Renderer->drawTexture(this, &mState, getMutableGLES1State(), coords[0], coords[1],
                                coords[2], coords[3], coords[4]);
}

void Context::drawTexi(GLint x, GLint y, GLint z, GLint width, GLint height)
{
    mGLES1Renderer->drawTexture(this, &mState, getMutableGLES1State(), static_cast<GLfloat>(x),
                                static_cast<GLfloat>(y), static_cast<GLfloat>(z),
                                static_cast<GLfloat>(width), static_cast<GLfloat>(height));
}

void Context::drawTexiv(const GLint *coords)
{
    mGLES1Renderer->drawTexture(this, &mState, getMutableGLES1State(),
                                static_cast<GLfloat>(coords[0]), static_cast<GLfloat>(coords[1]),
                                static_cast<GLfloat>(coords[2]), static_cast<GLfloat>(coords[3]),
                                static_cast<GLfloat>(coords[4]));
}

void Context::drawTexs(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
    mGLES1Renderer->drawTexture(this, &mState, getMutableGLES1State(), static_cast<GLfloat>(x),
                                static_cast<GLfloat>(y), static_cast<GLfloat>(z),
                                static_cast<GLfloat>(width), static_cast<GLfloat>(height));
}

void Context::drawTexsv(const GLshort *coords)
{
    mGLES1Renderer->drawTexture(this, &mState, getMutableGLES1State(),
                                static_cast<GLfloat>(coords[0]), static_cast<GLfloat>(coords[1]),
                                static_cast<GLfloat>(coords[2]), static_cast<GLfloat>(coords[3]),
                                static_cast<GLfloat>(coords[4]));
}

void Context::drawTexx(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
    mGLES1Renderer->drawTexture(this, &mState, getMutableGLES1State(), ConvertFixedToFloat(x),
                                ConvertFixedToFloat(y), ConvertFixedToFloat(z),
                                ConvertFixedToFloat(width), ConvertFixedToFloat(height));
}

void Context::drawTexxv(const GLfixed *coords)
{
    mGLES1Renderer->drawTexture(this, &mState, getMutableGLES1State(),
                                ConvertFixedToFloat(coords[0]), ConvertFixedToFloat(coords[1]),
                                ConvertFixedToFloat(coords[2]), ConvertFixedToFloat(coords[3]),
                                ConvertFixedToFloat(coords[4]));
}

// GL_OES_matrix_palette
void Context::currentPaletteMatrix(GLuint matrixpaletteindex)
{
    UNIMPLEMENTED();
}

void Context::loadPaletteFromModelViewMatrix()
{
    UNIMPLEMENTED();
}

void Context::matrixIndexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    UNIMPLEMENTED();
}

void Context::weightPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    UNIMPLEMENTED();
}

// GL_OES_point_size_array
void Context::pointSizePointer(VertexAttribType type, GLsizei stride, const void *ptr)
{
    vertexAttribPointer(vertexArrayIndex(ClientVertexArrayType::PointSize), 1, type, GL_FALSE,
                        stride, ptr);
}

// GL_OES_query_matrix
GLbitfield Context::queryMatrixx(GLfixed *mantissa, GLint *exponent)
{
    UNIMPLEMENTED();
    return 0;
}

// GL_OES_texture_cube_map
void Context::getTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
    UNIMPLEMENTED();
}

void Context::getTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
    UNIMPLEMENTED();
}

void Context::getTexGenxv(GLenum coord, GLenum pname, GLfixed *params)
{
    UNIMPLEMENTED();
}

void Context::texGenf(GLenum coord, GLenum pname, GLfloat param)
{
    UNIMPLEMENTED();
}

void Context::texGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
    UNIMPLEMENTED();
}

void Context::texGeni(GLenum coord, GLenum pname, GLint param)
{
    UNIMPLEMENTED();
}

void Context::texGeniv(GLenum coord, GLenum pname, const GLint *params)
{
    UNIMPLEMENTED();
}

void Context::texGenx(GLenum coord, GLenum pname, GLfixed param)
{
    UNIMPLEMENTED();
}

void Context::texGenxv(GLenum coord, GLenum pname, const GLint *params)
{
    UNIMPLEMENTED();
}

int Context::vertexArrayIndex(ClientVertexArrayType type) const
{
    return GLES1Renderer::VertexArrayIndex(type, mState.gles1());
}

// static
int Context::TexCoordArrayIndex(unsigned int unit)
{
    return GLES1Renderer::TexCoordArrayIndex(unit);
}
}  // namespace gl
