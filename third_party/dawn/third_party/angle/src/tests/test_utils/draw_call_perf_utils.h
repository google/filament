//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// draw_call_perf_utils.h:
//   Common utilities for performance tests that need to do a large amount of draw calls.
//

#ifndef TESTS_TEST_UTILS_DRAW_CALL_PERF_UTILS_H_
#define TESTS_TEST_UTILS_DRAW_CALL_PERF_UTILS_H_

#include <stddef.h>

#include "util/gles_loader_autogen.h"

// Returns program ID. The program is left in use, no uniforms.
GLuint SetupSimpleDrawProgram();

// Returns program ID. Uses a 2D texture.
GLuint SetupSimpleTextureProgram();

// Returns program ID. Uses two 2D textures.
GLuint SetupDoubleTextureProgram();

// Returns program ID. Uses eight 2D textures.
GLuint SetupEightTextureProgram();

// Returns program ID. The program is left in use and the uniforms are set to default values:
// uScale = 0.5, uOffset = -0.5
GLuint SetupSimpleScaleAndOffsetProgram();

// Returns buffer ID filled with 2-component triangle coordinates. The buffer is left as bound.
// Generates triangles like this with 2-component coordinates:
//    A
//   / \.
//  /   \.
// B-----C
GLuint Create2DTriangleBuffer(size_t numTris, GLenum usage);

// Creates an FBO with a texture color attachment. The texture is GL_RGBA and has dimensions
// width/height. The FBO and texture ids are written to the out parameters.
void CreateColorFBO(GLsizei width, GLsizei height, GLuint *fbo, GLuint *texture);

#endif  // TESTS_TEST_UTILS_DRAW_CALL_PERF_UTILS_H_
