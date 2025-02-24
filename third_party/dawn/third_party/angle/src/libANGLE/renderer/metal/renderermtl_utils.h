//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// renderermtl_utils:
//   Helper methods pertaining to the Metal backend.
//

#ifndef LIBANGLE_RENDERER_METAL_RENDERERMTL_UTILS_H_
#define LIBANGLE_RENDERER_METAL_RENDERERMTL_UTILS_H_

#include <cstdint>

#include <limits>
#include <map>

#include "GLSLANG/ShaderLang.h"
#include "common/angleutils.h"
#include "common/utilities.h"
#include "libANGLE/angletypes.h"

namespace rx
{
namespace mtl
{

template <int cols, int rows>
struct SetFloatUniformMatrixMetal
{
    static void Run(unsigned int arrayElementOffset,
                    unsigned int elementCount,
                    GLsizei countIn,
                    GLboolean transpose,
                    const GLfloat *value,
                    uint8_t *targetData);
};

// Helper method to de-tranpose a matrix uniform for an API query, Metal Version
void GetMatrixUniformMetal(GLenum type, GLfloat *dataOut, const GLfloat *source, bool transpose);

template <typename NonFloatT>
void GetMatrixUniformMetal(GLenum type,
                           NonFloatT *dataOut,
                           const NonFloatT *source,
                           bool transpose);
}  // namespace mtl
}  // namespace rx
#endif
