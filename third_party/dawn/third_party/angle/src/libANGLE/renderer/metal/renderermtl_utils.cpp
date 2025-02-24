//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// renderermtl_utils:
//   Helper methods pertaining to the Metal back-end.
//

#include "libANGLE/renderer/metal/renderermtl_utils.h"
#include "libANGLE/renderer/renderer_utils.h"

namespace rx
{

namespace
{

template <int cols, int rows, bool IsColumnMajor>
inline int GetFlattenedIndex(int col, int row)
{
    if (IsColumnMajor)
    {
        return col * rows + row;
    }
    else
    {
        return row * cols + col;
    }
}

template <typename T,
          bool IsSrcColumnMajor,
          int colsSrc,
          int rowsSrc,
          bool IsDstColumnMajor,
          int colsDst,
          int rowsDst>
void ExpandMatrix(T *target, const GLfloat *value)
{
    static_assert(colsSrc <= colsDst && rowsSrc <= rowsDst, "Can only expand!");

    constexpr int kDstFlatSize = colsDst * rowsDst;
    T staging[kDstFlatSize]    = {0};

    for (int r = 0; r < rowsSrc; r++)
    {
        for (int c = 0; c < colsSrc; c++)
        {
            int srcIndex = GetFlattenedIndex<colsSrc, rowsSrc, IsSrcColumnMajor>(c, r);
            int dstIndex = GetFlattenedIndex<colsDst, rowsDst, IsDstColumnMajor>(c, r);

            staging[dstIndex] = static_cast<T>(value[srcIndex]);
        }
    }

    memcpy(target, staging, kDstFlatSize * sizeof(T));
}

template <bool IsSrcColumMajor,
          int colsSrc,
          int rowsSrc,
          bool IsDstColumnMajor,
          int colsDst,
          int rowsDst>
void SetFloatUniformMatrix(unsigned int arrayElementOffset,
                           unsigned int elementCount,
                           GLsizei countIn,
                           const GLfloat *value,
                           uint8_t *targetData)
{
    unsigned int count =
        std::min(elementCount - arrayElementOffset, static_cast<unsigned int>(countIn));

    const unsigned int targetMatrixStride = colsDst * rowsDst;
    GLfloat *target                       = reinterpret_cast<GLfloat *>(
        targetData + arrayElementOffset * sizeof(GLfloat) * targetMatrixStride);

    for (unsigned int i = 0; i < count; i++)
    {
        ExpandMatrix<GLfloat, IsSrcColumMajor, colsSrc, rowsSrc, IsDstColumnMajor, colsDst,
                     rowsDst>(target, value);

        target += targetMatrixStride;
        value += colsSrc * rowsSrc;
    }
}

void SetFloatUniformMatrixFast(unsigned int arrayElementOffset,
                               unsigned int elementCount,
                               GLsizei countIn,
                               size_t matrixSize,
                               const GLfloat *value,
                               uint8_t *targetData)
{
    const unsigned int count =
        std::min(elementCount - arrayElementOffset, static_cast<unsigned int>(countIn));

    const uint8_t *valueData = reinterpret_cast<const uint8_t *>(value);
    targetData               = targetData + arrayElementOffset * matrixSize;

    memcpy(targetData, valueData, matrixSize * count);
}

}  // anonymous namespace

namespace mtl
{

#define ANGLE_INSTANTIATE_SET_UNIFORM_MATRIX_FUNC(api, cols, rows) \
    template void SetFloatUniformMatrix##api<cols, rows>::Run(     \
        unsigned int, unsigned int, GLsizei, GLboolean, const GLfloat *, uint8_t *)

ANGLE_INSTANTIATE_SET_UNIFORM_MATRIX_FUNC(Metal, 2, 2);
ANGLE_INSTANTIATE_SET_UNIFORM_MATRIX_FUNC(Metal, 3, 3);
ANGLE_INSTANTIATE_SET_UNIFORM_MATRIX_FUNC(Metal, 2, 3);
ANGLE_INSTANTIATE_SET_UNIFORM_MATRIX_FUNC(Metal, 3, 2);
ANGLE_INSTANTIATE_SET_UNIFORM_MATRIX_FUNC(Metal, 2, 4);
ANGLE_INSTANTIATE_SET_UNIFORM_MATRIX_FUNC(Metal, 3, 4);
ANGLE_INSTANTIATE_SET_UNIFORM_MATRIX_FUNC(Metal, 4, 3);

#undef ANGLE_INSTANTIATE_SET_UNIFORM_MATRIX_FUNC

#define ANGLE_SPECIALIZATION_COLS_SET_UNIFORM_MATRIX_FUNC(api, cols, rows) \
    template void SetFloatUniformMatrix##api<cols, rows>::Run(             \
        unsigned int, unsigned int, GLsizei, GLboolean, const GLfloat *, uint8_t *)

template <int cols>
struct SetFloatUniformMatrixMetal<cols, 4>
{
    static void Run(unsigned int arrayElementOffset,
                    unsigned int elementCount,
                    GLsizei countIn,
                    GLboolean transpose,
                    const GLfloat *value,
                    uint8_t *targetData);
};

template <int cols>
struct SetFloatUniformMatrixMetal<cols, 2>
{
    static void Run(unsigned int arrayElementOffset,
                    unsigned int elementCount,
                    GLsizei countIn,
                    GLboolean transpose,
                    const GLfloat *value,
                    uint8_t *targetData);
};

ANGLE_SPECIALIZATION_COLS_SET_UNIFORM_MATRIX_FUNC(Metal, 2, 4);
ANGLE_SPECIALIZATION_COLS_SET_UNIFORM_MATRIX_FUNC(Metal, 3, 4);
ANGLE_SPECIALIZATION_COLS_SET_UNIFORM_MATRIX_FUNC(Metal, 4, 4);

ANGLE_SPECIALIZATION_COLS_SET_UNIFORM_MATRIX_FUNC(Metal, 2, 2);
ANGLE_SPECIALIZATION_COLS_SET_UNIFORM_MATRIX_FUNC(Metal, 3, 2);
ANGLE_SPECIALIZATION_COLS_SET_UNIFORM_MATRIX_FUNC(Metal, 4, 2);

#undef ANGLE_SPECIALIZATION_COLS_SET_UNIFORM_MATRIX_FUNC

template <int cols>
void SetFloatUniformMatrixMetal<cols, 4>::Run(unsigned int arrayElementOffset,
                                              unsigned int elementCount,
                                              GLsizei countIn,
                                              GLboolean transpose,
                                              const GLfloat *value,
                                              uint8_t *targetData)
{
    const bool isSrcColumnMajor = !transpose;
    if (isSrcColumnMajor)
    {
        // Both src and dst matrixs have the same layout,
        // a single memcpy updates all the matrices
        constexpr size_t srcMatrixSize = sizeof(GLfloat) * cols * 4;
        SetFloatUniformMatrixFast(arrayElementOffset, elementCount, countIn, srcMatrixSize, value,
                                  targetData);
    }
    else
    {
        // fallback to general cases
        SetFloatUniformMatrix<false, cols, 4, true, cols, 4>(arrayElementOffset, elementCount,
                                                             countIn, value, targetData);
    }
}

template <int cols>
void SetFloatUniformMatrixMetal<cols, 2>::Run(unsigned int arrayElementOffset,
                                              unsigned int elementCount,
                                              GLsizei countIn,
                                              GLboolean transpose,
                                              const GLfloat *value,
                                              uint8_t *targetData)
{
    const bool isSrcColumnMajor = !transpose;
    if (isSrcColumnMajor)
    {
        // Both src and dst matrixs have the same layout,
        // a single memcpy updates all the matrices
        constexpr size_t srcMatrixSize = sizeof(GLfloat) * cols * 2;
        SetFloatUniformMatrixFast(arrayElementOffset, elementCount, countIn, srcMatrixSize, value,
                                  targetData);
    }
    else
    {
        // fallback to general cases
        SetFloatUniformMatrix<false, cols, 2, true, cols, 2>(arrayElementOffset, elementCount,
                                                             countIn, value, targetData);
    }
}

template <int cols, int rows>
void SetFloatUniformMatrixMetal<cols, rows>::Run(unsigned int arrayElementOffset,
                                                 unsigned int elementCount,
                                                 GLsizei countIn,
                                                 GLboolean transpose,
                                                 const GLfloat *value,
                                                 uint8_t *targetData)
{
    const bool isSrcColumnMajor = !transpose;
    constexpr size_t numRows    = rows == 3 ? 4 : rows;
    // Metal expects matrix uniforms to be column-major, and each column is padded to 4 rows.
    if (isSrcColumnMajor)
    {
        SetFloatUniformMatrix<true, cols, rows, true, cols, numRows>(
            arrayElementOffset, elementCount, countIn, value, targetData);
    }
    else
    {
        SetFloatUniformMatrix<false, cols, rows, true, cols, numRows>(
            arrayElementOffset, elementCount, countIn, value, targetData);
    }
}

template void GetMatrixUniformMetal<GLint>(GLenum, GLint *, const GLint *, bool);
template void GetMatrixUniformMetal<GLuint>(GLenum, GLuint *, const GLuint *, bool);

void GetMatrixUniformMetal(GLenum type, GLfloat *dataOut, const GLfloat *source, bool transpose)
{
    int columns    = gl::VariableColumnCount(type);
    int rows       = gl::VariableRowCount(type);
    int rowsPerCol = rows == 3 ? 4 : rows;
    for (GLint col = 0; col < columns; ++col)
    {
        for (GLint row = 0; row < rows; ++row)
        {
            GLfloat *outptr = dataOut + ((col * rows) + row);
            const GLfloat *inptr =
                transpose ? source + ((row * columns) + col) : source + ((col * rowsPerCol) + row);
            *outptr = *inptr;
        }
    }
}

template <typename NonFloatT>
void GetMatrixUniformMetal(GLenum type, NonFloatT *dataOut, const NonFloatT *source, bool transpose)
{
    UNREACHABLE();
}

}  // namespace mtl
}  // namespace rx
