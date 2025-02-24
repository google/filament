//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompressedTextureFormatsTest:
//   Tests that only the appropriate entry points are affected after
//   enabling compressed texture extensions.
//

#include "common/gl_enum_utils.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

struct FormatDesc
{
    GLenum format;
    GLsizei blockX;
    GLsizei blockY;
    GLsizei size;

    bool isPVRTC1() const
    {
        return ((format & ~3) == GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG) ||
               ((format & ~3) == GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT);
    }
    bool mayBeEmulated() const
    {
        return format == GL_COMPRESSED_R11_EAC || format == GL_COMPRESSED_RG11_EAC ||
               format == GL_COMPRESSED_SIGNED_R11_EAC || format == GL_COMPRESSED_SIGNED_RG11_EAC ||
               format == GL_ETC1_RGB8_OES || format == GL_COMPRESSED_RGB8_ETC2 ||
               format == GL_COMPRESSED_SRGB8_ETC2 ||
               format == GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 ||
               format == GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 ||
               format == GL_COMPRESSED_RGBA8_ETC2_EAC ||
               format == GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
    }
};
using CompressedTextureTestParams = std::tuple<angle::PlatformParameters, FormatDesc>;

class CompressedTextureFormatsTest : public ANGLETest<CompressedTextureTestParams>
{
  public:
    CompressedTextureFormatsTest(const std::string ext1,
                                 const std::string ext2,
                                 const bool supportsUpdates,
                                 const bool supportsPartialUpdates,
                                 const bool supports2DArray,
                                 const bool supports3D,
                                 const bool alwaysOnES3)
        : mExtNames({ext1, ext2}),
          mSupportsUpdates(supportsUpdates),
          mSupportsPartialUpdates(supportsPartialUpdates),
          mSupports2DArray(supports2DArray),
          mSupports3D(supports3D),
          mAlwaysOnES3(alwaysOnES3)
    {
        setExtensionsEnabled(false);
    }

    void testSetUp() override
    {
        // Apple platforms require PVRTC1 textures to be squares.
        mSquarePvrtc1 = IsAppleGPU();
    }

    void checkSubImage2D(FormatDesc desc, int numX)
    {
        GLubyte data[64] = {};

        // The semantic of this call is to take uncompressed data, compress it on-the-fly,
        // and perform a partial update of an existing GPU-compressed texture. This
        // operation is not supported in OpenGL ES.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.blockX, desc.blockY, GL_RGBA, GL_UNSIGNED_BYTE,
                        nullptr);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        // Compressed texture extensions never extend TexSubImage2D.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.blockX, desc.blockY, desc.format,
                        GL_UNSIGNED_BYTE, nullptr);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        // The semantic of this call is to take pixel data from the current framebuffer, compress it
        // on-the-fly, and perform a partial update of an existing GPU-compressed texture. This
        // operation is not supported in OpenGL ES.
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, desc.blockX, desc.blockY);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        // Try whole image update. It is always valid when API supports updates.
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.blockX * numX, desc.blockY * 2,
                                  desc.format, desc.size * 4, data);
        if (!mSupportsUpdates)
        {
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            return;
        }
        EXPECT_GL_NO_ERROR();

        if (!mSupportsPartialUpdates)
        {
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.blockX, desc.blockY, desc.format,
                                      desc.size, data);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            return;
        }

        // All compressed formats that support partial updates require the offsets to be
        // multiples of block dimensions.
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 1, 0, desc.blockX, desc.blockY, desc.format,
                                  desc.size, data);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 1, desc.blockX, desc.blockY, desc.format,
                                  desc.size, data);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        // All compressed formats that support partial updates require the dimensions to be
        // multiples of block dimensions or reach the image boundaries.
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.blockX - 1, desc.blockY, desc.format,
                                  desc.size, data);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.blockX, desc.blockY - 1, desc.format,
                                  desc.size, data);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        // Test should pass when replaced region dimensions are multiples the of block
        // dimensions
        // clang-format off
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0,           0,           desc.blockX, desc.blockY, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, desc.blockX, 0,           desc.blockX, desc.blockY, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0,           desc.blockY, desc.blockX, desc.blockY, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, desc.blockX, desc.blockY, desc.blockX, desc.blockY, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0,           0,           desc.blockX * 2, desc.blockY,     desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0,           0,           desc.blockX,     desc.blockY * 2, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0,           desc.blockY, desc.blockX * 2, desc.blockY,     desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, desc.blockX, 0,           desc.blockX,     desc.blockY * 2, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        // clang-format on

        // Test should pass when replaced region dimensions are not multiples of block dimensions
        // but reach the image boundaries. For example, it is valid to replace right-bottom 2x2
        // region of a 6x6 texture made of 4x4 blocks. To check this on all platforms with the same
        // code, level 1 should be used to avoid hitting a D3D11 limitation.
        if (getClientMajorVersion() >= 3 || EnsureGLExtensionEnabled("GL_OES_texture_npot"))
        {
            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);
            const int newW = desc.blockX * 3;
            const int newH = desc.blockY * 3;
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, desc.format, newW, newH, 0, desc.size * 9,
                                   nullptr);
            EXPECT_GL_NO_ERROR();

            glCompressedTexImage2D(GL_TEXTURE_2D, 1, desc.format, newW / 2, newH / 2, 0,
                                   desc.size * 4, nullptr);
            EXPECT_GL_NO_ERROR();

            // clang-format off
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, desc.blockX, 0,           desc.blockX / 2,     desc.blockY,         desc.format, desc.size * 1, data);
            EXPECT_GL_NO_ERROR();
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, desc.blockX, 0,           desc.blockX / 2,     desc.blockY * 3 / 2, desc.format, desc.size * 2, data);
            EXPECT_GL_NO_ERROR();
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, 0,           desc.blockY, desc.blockX,         desc.blockY / 2,     desc.format, desc.size * 1, data);
            EXPECT_GL_NO_ERROR();
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, 0,           desc.blockY, desc.blockX * 3 / 2, desc.blockY / 2,     desc.format, desc.size * 2, data);
            EXPECT_GL_NO_ERROR();
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, desc.blockX, desc.blockY, desc.blockX / 2,     desc.blockY / 2,     desc.format, desc.size * 1, data);
            EXPECT_GL_NO_ERROR();
            // clang-format on
        }
    }

    void checkSubImage3D(GLenum target, FormatDesc desc)
    {
        GLubyte data[128] = {};

        // The semantic of this call is to take uncompressed data, compress it on-the-fly,
        // and perform a partial update of an existing GPU-compressed texture. This
        // operation is not supported in OpenGL ES.
        glTexSubImage3D(target, 0, 0, 0, 0, desc.blockX, desc.blockY, 2, GL_RGBA, GL_UNSIGNED_BYTE,
                        nullptr);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        // Compressed texture extensions never extend TexSubImage3D.
        glTexSubImage3D(target, 0, 0, 0, 0, desc.blockX, desc.blockY, 2, desc.format,
                        GL_UNSIGNED_BYTE, nullptr);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        // The semantic of this call is to take pixel data from the current framebuffer, compress it
        // on-the-fly, and perform a partial update of an existing GPU-compressed texture. This
        // operation is not supported in OpenGL ES.
        glCopyTexSubImage3D(target, 0, 0, 0, 0, 0, 0, desc.blockX, desc.blockY);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        // All formats that are accepted for 3D entry points support updates.
        ASSERT(mSupportsUpdates);

        // Try whole image update. It is always valid for formats that support updates.
        glCompressedTexSubImage3D(target, 0, 0, 0, 0, desc.blockX * 2, desc.blockY * 2, 2,
                                  desc.format, desc.size * 8, data);
        EXPECT_GL_NO_ERROR();

        // Try a whole image update from a pixel unpack buffer.
        // Don't test non-emulated formats on Desktop GL.
        // TODO(anglebug.com/42264819): implement emulation on Desktop GL, then remove this check.
        if (!(IsDesktopOpenGL() && desc.mayBeEmulated()))
        {
            GLBuffer buffer;
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
            glBufferData(GL_PIXEL_UNPACK_BUFFER, 128, data, GL_STREAM_DRAW);
            EXPECT_GL_NO_ERROR();

            glCompressedTexSubImage3D(target, 0, 0, 0, 0, desc.blockX * 2, desc.blockY * 2, 2,
                                      desc.format, desc.size * 8, nullptr);
            EXPECT_GL_NO_ERROR();

            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }

        // All formats that are accepted for 3D entry points support partial updates.
        ASSERT(mSupportsPartialUpdates);

        // All compressed formats that support partial updates require the offsets to be
        // multiples of block dimensions.
        glCompressedTexSubImage3D(target, 0, 1, 0, 0, desc.blockX, desc.blockY, 1, desc.format,
                                  desc.size, data);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glCompressedTexSubImage3D(target, 0, 0, 1, 0, desc.blockX, desc.blockY, 1, desc.format,
                                  desc.size, data);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        // All compressed formats that support partial updates require the dimensions to be
        // multiples of block dimensions or reach the image boundaries.
        glCompressedTexSubImage3D(target, 0, 0, 0, 0, desc.blockX - 1, desc.blockY, 1, desc.format,
                                  desc.size, data);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glCompressedTexSubImage3D(target, 0, 0, 0, 0, desc.blockX, desc.blockY - 1, 1, desc.format,
                                  desc.size, data);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        // Valid partial updates
        // clang-format off
        glCompressedTexSubImage3D(target, 0, 0,           0,           0, desc.blockX, desc.blockY, 1, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, desc.blockX, 0,           0, desc.blockX, desc.blockY, 1, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, 0,           desc.blockY, 0, desc.blockX, desc.blockY, 1, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, desc.blockX, desc.blockY, 0, desc.blockX, desc.blockY, 1, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, 0,           0,           1, desc.blockX, desc.blockY, 1, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, desc.blockX, 0,           1, desc.blockX, desc.blockY, 1, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, 0,           desc.blockY, 1, desc.blockX, desc.blockY, 1, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, desc.blockX, desc.blockY, 1, desc.blockX, desc.blockY, 1, desc.format, desc.size, data);
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage3D(target, 0, 0,           0,           0, desc.blockX * 2, desc.blockY,     1, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, 0,           0,           0, desc.blockX,     desc.blockY * 2, 1, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, 0,           desc.blockY, 0, desc.blockX * 2, desc.blockY,     1, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, desc.blockX, 0,           0, desc.blockX,     desc.blockY * 2, 1, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, 0,           0,           1, desc.blockX * 2, desc.blockY,     1, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, 0,           0,           1, desc.blockX,     desc.blockY * 2, 1, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, 0,           desc.blockY, 1, desc.blockX * 2, desc.blockY,     1, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, desc.blockX, 0,           1, desc.blockX,     desc.blockY * 2, 1, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, 0,           0,           0, desc.blockX,     desc.blockY,     2, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, desc.blockX, 0,           0, desc.blockX,     desc.blockY,     2, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, 0,           desc.blockY, 0, desc.blockX,     desc.blockY,     2, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        glCompressedTexSubImage3D(target, 0, desc.blockX, desc.blockY, 0, desc.blockX,     desc.blockY,     2, desc.format, desc.size * 2, data);
        EXPECT_GL_NO_ERROR();
        // clang-format on

        // Test should pass when replaced region dimensions are not multiples of block dimensions
        // but reach the image boundaries. For example, it is valid to replace right-bottom 2x2
        // region of a 6x6 texture made of 4x4 blocks. To check this on all platforms with the same
        // code, level 1 should be used to avoid hitting a D3D11 limitation.
        {
            GLTexture texture;
            glBindTexture(target, texture);
            const int newW = desc.blockX * 3;
            const int newH = desc.blockY * 3;
            glCompressedTexImage3D(target, 0, desc.format, newW, newH, 2, 0, desc.size * 18,
                                   nullptr);
            EXPECT_GL_NO_ERROR();

            if (target == GL_TEXTURE_2D_ARRAY)
            {
                glCompressedTexImage3D(target, 1, desc.format, newW / 2, newH / 2, 2, 0,
                                       desc.size * 8, nullptr);
                EXPECT_GL_NO_ERROR();

                // clang-format off
                glCompressedTexSubImage3D(target, 1, desc.blockX, 0,           0, desc.blockX / 2,     desc.blockY,         1, desc.format, desc.size * 1, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, desc.blockX, 0,           0, desc.blockX / 2,     desc.blockY * 3 / 2, 1, desc.format, desc.size * 2, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, 0,           desc.blockY, 0, desc.blockX,         desc.blockY / 2,     1, desc.format, desc.size * 1, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, 0,           desc.blockY, 0, desc.blockX * 3 / 2, desc.blockY / 2,     1, desc.format, desc.size * 2, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, desc.blockX, desc.blockY, 0, desc.blockX / 2,     desc.blockY / 2,     1, desc.format, desc.size * 1, data);
                EXPECT_GL_NO_ERROR();

                glCompressedTexSubImage3D(target, 1, desc.blockX, 0,           1, desc.blockX / 2,     desc.blockY,         1, desc.format, desc.size * 1, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, desc.blockX, 0,           1, desc.blockX / 2,     desc.blockY * 3 / 2, 1, desc.format, desc.size * 2, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, 0,           desc.blockY, 1, desc.blockX,         desc.blockY / 2,     1, desc.format, desc.size * 1, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, 0,           desc.blockY, 1, desc.blockX * 3 / 2, desc.blockY / 2,     1, desc.format, desc.size * 2, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, desc.blockX, desc.blockY, 1, desc.blockX / 2,     desc.blockY / 2,     1, desc.format, desc.size * 1, data);
                EXPECT_GL_NO_ERROR();

                glCompressedTexSubImage3D(target, 1, desc.blockX, 0,           0, desc.blockX / 2,     desc.blockY,         2, desc.format, desc.size * 2, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, desc.blockX, 0,           0, desc.blockX / 2,     desc.blockY * 3 / 2, 2, desc.format, desc.size * 4, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, 0,           desc.blockY, 0, desc.blockX,         desc.blockY / 2,     2, desc.format, desc.size * 2, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, 0,           desc.blockY, 0, desc.blockX * 3 / 2, desc.blockY / 2,     2, desc.format, desc.size * 4, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, desc.blockX, desc.blockY, 0, desc.blockX / 2,     desc.blockY / 2,     2, desc.format, desc.size * 2, data);
                EXPECT_GL_NO_ERROR();
                // clang-format on
            }
            else
            {
                glCompressedTexImage3D(target, 1, desc.format, newW / 2, newH / 2, 1, 0,
                                       desc.size * 4, nullptr);
                EXPECT_GL_NO_ERROR();

                // clang-format off
                glCompressedTexSubImage3D(target, 1, desc.blockX, 0,           0, desc.blockX / 2,     desc.blockY,         1, desc.format, desc.size * 1, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, desc.blockX, 0,           0, desc.blockX / 2,     desc.blockY * 3 / 2, 1, desc.format, desc.size * 2, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, 0,           desc.blockY, 0, desc.blockX,         desc.blockY / 2,     1, desc.format, desc.size * 1, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, 0,           desc.blockY, 0, desc.blockX * 3 / 2, desc.blockY / 2,     1, desc.format, desc.size * 2, data);
                EXPECT_GL_NO_ERROR();
                glCompressedTexSubImage3D(target, 1, desc.blockX, desc.blockY, 0, desc.blockX / 2,     desc.blockY / 2,     1, desc.format, desc.size * 1, data);
                EXPECT_GL_NO_ERROR();
                // clang-format on
            }
        }
    }

    void check2D(const bool compressedFormatEnabled)
    {
        const FormatDesc desc = ::testing::get<1>(GetParam());

        {
            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);

            // The semantic of this call is to take uncompressed data and compress it on-the-fly.
            // This operation is not supported in OpenGL ES.
            glTexImage2D(GL_TEXTURE_2D, 0, desc.format, desc.blockX, desc.blockY, 0, GL_RGB,
                         GL_UNSIGNED_BYTE, nullptr);
            EXPECT_GL_ERROR(GL_INVALID_VALUE);

            // Try compressed enum as format. Compressed texture extensions never allow this.
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, desc.blockX, desc.blockY, 0, desc.format,
                         GL_UNSIGNED_BYTE, nullptr);
            EXPECT_GL_ERROR(GL_INVALID_ENUM);

            // The semantic of this call is to take pixel data from the current framebuffer
            // and create a compressed texture from it on-the-fly. This operation is not supported
            // in OpenGL ES.
            glCopyTexImage2D(GL_TEXTURE_2D, 0, desc.format, 0, 0, desc.blockX, desc.blockY, 0);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);

            glCompressedTexImage2D(GL_TEXTURE_2D, 0, desc.format, desc.blockX, desc.blockY, 0,
                                   desc.size, nullptr);
            if (compressedFormatEnabled)
            {
                int numX = 2;
                if (desc.isPVRTC1())
                {
                    if (mSquarePvrtc1 && desc.blockX == 8)
                    {
                        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
                        numX = 1;
                    }
                    else
                    {
                        // PVRTC1 formats require data size for at least 2x2 blocks.
                        EXPECT_GL_ERROR(GL_INVALID_VALUE);
                    }
                }
                else
                {
                    EXPECT_GL_NO_ERROR();
                }

                // Create a texture with more than one block to test partial updates
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, desc.format, desc.blockX * numX,
                                       desc.blockY * 2, 0, desc.size * 4, nullptr);
                EXPECT_GL_NO_ERROR();

                checkSubImage2D(desc, numX);
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_ENUM);
            }
        }

        if (getClientMajorVersion() >= 3)
        {
            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexStorage2D(GL_TEXTURE_2D, 1, desc.format, desc.blockX * 2, desc.blockY * 2);
            if (compressedFormatEnabled)
            {
                int numX = 2;
                if (desc.isPVRTC1() && mSquarePvrtc1 && desc.blockX == 8)
                {
                    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
                    numX = 1;
                    glTexStorage2D(GL_TEXTURE_2D, 1, desc.format, desc.blockX, desc.blockY * 2);
                }
                EXPECT_GL_NO_ERROR();

                checkSubImage2D(desc, numX);
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_ENUM);
            }
        }

        if (EnsureGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexStorage2DEXT(GL_TEXTURE_2D, 1, desc.format, desc.blockX * 2, desc.blockY * 2);
            if (compressedFormatEnabled)
            {
                int numX = 2;
                if (desc.isPVRTC1() && mSquarePvrtc1 && desc.blockX == 8)
                {
                    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
                    numX = 1;
                    glTexStorage2DEXT(GL_TEXTURE_2D, 1, desc.format, desc.blockX, desc.blockY * 2);
                }
                EXPECT_GL_NO_ERROR();

                checkSubImage2D(desc, numX);
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_ENUM);
            }
        }
    }

    void check3D(GLenum target, const bool compressedFormatEnabled, const bool supportsTarget)
    {
        const FormatDesc desc = ::testing::get<1>(GetParam());

        {
            GLTexture texture;
            glBindTexture(target, texture);

            // Try compressed enum as internalformat. The semantic of this call is to take
            // uncompressed data and compress it on-the-fly. This operation is not supported in
            // OpenGL ES.
            glTexImage3D(target, 0, desc.format, desc.blockX, desc.blockX, 1, 0, GL_RGB,
                         GL_UNSIGNED_BYTE, nullptr);
            EXPECT_GL_ERROR(GL_INVALID_VALUE);

            // Try compressed enum as format. Compressed texture extensions never allow this.
            glTexImage3D(target, 0, GL_RGB, desc.blockX, desc.blockX, 1, 0, desc.format,
                         GL_UNSIGNED_BYTE, nullptr);
            EXPECT_GL_ERROR(GL_INVALID_ENUM);

            glCompressedTexImage3D(target, 0, desc.format, desc.blockX * 2, desc.blockY * 2, 2, 0,
                                   desc.size * 8, nullptr);
            if (compressedFormatEnabled)
            {
                if (supportsTarget)
                {
                    EXPECT_GL_NO_ERROR();

                    checkSubImage3D(target, desc);
                }
                else
                {
                    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
                }
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_ENUM);
            }
        }

        {
            GLTexture texture;
            glBindTexture(target, texture);

            glTexStorage3D(target, 1, desc.format, desc.blockX * 2, desc.blockY * 2, 2);
            if (compressedFormatEnabled)
            {
                if (supportsTarget)
                {
                    EXPECT_GL_NO_ERROR();

                    checkSubImage3D(target, desc);
                }
                else
                {
                    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
                }
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_ENUM);
            }
        }
    }

    void testSamplerSliced3D(GLenum target)
    {
        ASSERT(target == GL_TEXTURE_2D_ARRAY || target == GL_TEXTURE_3D);

        const FormatDesc desc = ::testing::get<1>(GetParam());
        {
            int width                                    = desc.blockX;
            int height                                   = desc.blockY;
            int depth                                    = 9;
            static GLubyte red_RGBA_ASTC_block_data[144] = {
                252, 253, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255,
                252, 253, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255,
                252, 253, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255,
                252, 253, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255,
                252, 253, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255,
                252, 253, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255,
                252, 253, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255,
                252, 253, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255,
                252, 253, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255};

            GLTexture texID;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(target, texID);
            EXPECT_GL_NO_ERROR();
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glCompressedTexImage3D(target, 0, desc.format, width, height, depth, 0,
                                   desc.size * depth, red_RGBA_ASTC_block_data);
            EXPECT_GL_NO_ERROR();

            float layer = 0.0f;
            for (int i = 0; i < depth; i++)
            {
                GLFramebuffer fb;
                glBindFramebuffer(GL_FRAMEBUFFER, fb);
                GLRenderbuffer rb;
                glBindRenderbuffer(GL_RENDERBUFFER, rb);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                          rb);
                EXPECT_GL_NO_ERROR();

                glViewport(0, 0, width, height);
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                if (target == GL_TEXTURE_2D_ARRAY)
                {
                    layer = i * 1.0f;
                    draw2DArrayTexturedQuad(0.0f, 1.0f, false, layer);
                }
                else
                {
                    layer = ((float)i + 0.5f) / (float)depth;
                    draw3DTexturedQuad(0.0f, 1.0f, false, layer);
                }

                std::vector<GLColor> pixels(width * height);
                glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
                for (int y = 0; y < height; y++)
                {
                    for (int x = 0; x < width; x++)
                    {
                        const int curPos = y * width + x;
                        EXPECT_COLOR_NEAR(GLColor::red, pixels[curPos], 2);
                    }
                }
            }
        }
    }

    void test()
    {
        // ETC2/EAC formats always pass validation on ES3 contexts but in some cases fail in drivers
        // because their emulation is not implemented for OpenGL renderer.
        // https://crbug.com/angleproject/6300
        if (mAlwaysOnES3)
        {
            ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3 &&
                               !IsGLExtensionRequestable(mExtNames[0]));
        }

        // It's not possible to disable ETC2/EAC support on ES 3.0.
        const bool compressedFormatEnabled = mAlwaysOnES3 && getClientMajorVersion() >= 3;
        check2D(compressedFormatEnabled);
        if (getClientMajorVersion() >= 3)
        {
            check3D(GL_TEXTURE_2D_ARRAY, compressedFormatEnabled, mSupports2DArray);
            check3D(GL_TEXTURE_3D, compressedFormatEnabled, mSupports3D && !mDisableTexture3D);
        }

        for (const std::string &extName : mExtNames)
        {
            if (!extName.empty())
            {
                if (IsGLExtensionRequestable(extName))
                {
                    glRequestExtensionANGLE(extName.c_str());
                }
                ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(extName));
            }
        }

        // Repeat all checks after enabling the extensions.
        check2D(true);
        if (getClientMajorVersion() >= 3)
        {
            check3D(GL_TEXTURE_2D_ARRAY, true, mSupports2DArray);
            check3D(GL_TEXTURE_3D, true, mSupports3D && !mDisableTexture3D);
        }
    }

    void testSamplerASTCSliced3D()
    {
        for (const std::string &extName : mExtNames)
        {
            if (!extName.empty())
            {
                if (IsGLExtensionRequestable(extName))
                {
                    glRequestExtensionANGLE(extName.c_str());
                }
                ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(extName));
            }
        }

        if (getClientMajorVersion() >= 3)
        {
            testSamplerSliced3D(GL_TEXTURE_2D_ARRAY);
            testSamplerSliced3D(GL_TEXTURE_3D);
        }
    }

  private:
    bool mSquarePvrtc1     = false;
    bool mDisableTexture3D = false;
    const std::vector<std::string> mExtNames;
    const bool mSupportsUpdates;
    const bool mSupportsPartialUpdates;
    const bool mSupports2DArray;
    const bool mSupports3D;
    const bool mAlwaysOnES3;
};

template <char const *ext1,
          char const *ext2,
          bool supports_updates,
          bool supports_partial_updates,
          bool supports_2d_array,
          bool supports_3d,
          bool always_on_es3>
class _Test : public CompressedTextureFormatsTest
{
  public:
    _Test()
        : CompressedTextureFormatsTest(ext1,
                                       ext2,
                                       supports_updates,
                                       supports_partial_updates,
                                       supports_2d_array,
                                       supports_3d,
                                       always_on_es3)
    {}
};

const char kDXT1[]     = "GL_EXT_texture_compression_dxt1";
const char kDXT3[]     = "GL_ANGLE_texture_compression_dxt3";
const char kDXT5[]     = "GL_ANGLE_texture_compression_dxt5";
const char kS3TCSRGB[] = "GL_EXT_texture_compression_s3tc_srgb";
const char kRGTC[]     = "GL_EXT_texture_compression_rgtc";
const char kBPTC[]     = "GL_EXT_texture_compression_bptc";

const char kETC1[]    = "GL_OES_compressed_ETC1_RGB8_texture";
const char kETC1Sub[] = "GL_EXT_compressed_ETC1_RGB8_sub_texture";

const char kEACR11U[]  = "GL_OES_compressed_EAC_R11_unsigned_texture";
const char kEACR11S[]  = "GL_OES_compressed_EAC_R11_signed_texture";
const char kEACRG11U[] = "GL_OES_compressed_EAC_RG11_unsigned_texture";
const char kEACRG11S[] = "GL_OES_compressed_EAC_RG11_signed_texture";

const char kETC2RGB8[]       = "GL_OES_compressed_ETC2_RGB8_texture";
const char kETC2RGB8SRGB[]   = "GL_OES_compressed_ETC2_sRGB8_texture";
const char kETC2RGB8A1[]     = "GL_OES_compressed_ETC2_punchthroughA_RGBA8_texture";
const char kETC2RGB8A1SRGB[] = "GL_OES_compressed_ETC2_punchthroughA_sRGB8_alpha_texture";
const char kETC2RGBA8[]      = "GL_OES_compressed_ETC2_RGBA8_texture";
const char kETC2RGBA8SRGB[]  = "GL_OES_compressed_ETC2_sRGB8_alpha8_texture";

const char kASTC[]         = "GL_KHR_texture_compression_astc_ldr";
const char kASTCSliced3D[] = "GL_KHR_texture_compression_astc_sliced_3d";

const char kPVRTC1[]    = "GL_IMG_texture_compression_pvrtc";
const char kPVRTCSRGB[] = "GL_EXT_pvrtc_sRGB";

const char kEmpty[] = "";

// clang-format off
using CompressedTextureDXT1Test     = _Test<kDXT1,     kEmpty, true, true, true, false, false>;
using CompressedTextureDXT3Test     = _Test<kDXT3,     kEmpty, true, true, true, false, false>;
using CompressedTextureDXT5Test     = _Test<kDXT5,     kEmpty, true, true, true, false, false>;
using CompressedTextureS3TCSRGBTest = _Test<kS3TCSRGB, kEmpty, true, true, true, false, false>;
using CompressedTextureRGTCTest     = _Test<kRGTC,     kEmpty, true, true, true, false, false>;
using CompressedTextureBPTCTest     = _Test<kBPTC,     kEmpty, true, true, true, true,  false>;

using CompressedTextureETC1Test    = _Test<kETC1, kEmpty,   false, false, false, false, false>;
using CompressedTextureETC1SubTest = _Test<kETC1, kETC1Sub, true,  true,  true,  false, false>;

using CompressedTextureEACR11UTest  = _Test<kEACR11U,  kEmpty, true, true, true, false, true>;
using CompressedTextureEACR11STest  = _Test<kEACR11S,  kEmpty, true, true, true, false, true>;
using CompressedTextureEACRG11UTest = _Test<kEACRG11U, kEmpty, true, true, true, false, true>;
using CompressedTextureEACRG11STest = _Test<kEACRG11S, kEmpty, true, true, true, false, true>;

using CompressedTextureETC2RGB8Test       = _Test<kETC2RGB8,       kEmpty, true, true, true, false, true>;
using CompressedTextureETC2RGB8SRGBTest   = _Test<kETC2RGB8SRGB,   kEmpty, true, true, true, false, true>;
using CompressedTextureETC2RGB8A1Test     = _Test<kETC2RGB8A1,     kEmpty, true, true, true, false, true>;
using CompressedTextureETC2RGB8A1SRGBTest = _Test<kETC2RGB8A1SRGB, kEmpty, true, true, true, false, true>;
using CompressedTextureETC2RGBA8Test      = _Test<kETC2RGBA8,      kEmpty, true, true, true, false, true>;
using CompressedTextureETC2RGBA8SRGBTest  = _Test<kETC2RGBA8SRGB,  kEmpty, true, true, true, false, true>;

using CompressedTextureASTCTest         = _Test<kASTC, kEmpty,        true, true, true, false, false>;
using CompressedTextureASTCSliced3DTest = _Test<kASTC, kASTCSliced3D, true, true, true, true,  false>;
using CompressedTextureSamplerASTCSliced3DTest = _Test<kASTC, kASTCSliced3D, true, true, true, true,  false>;

using CompressedTexturePVRTC1Test     = _Test<kPVRTC1, kEmpty,     true, false, false, false, false>;
using CompressedTexturePVRTC1SRGBTest = _Test<kPVRTC1, kPVRTCSRGB, true, false, false, false, false>;
// clang-format on

std::string PrintToStringParamName(
    const ::testing::TestParamInfo<CompressedTextureTestParams> &info)
{
    std::string name = gl::GLinternalFormatToString(std::get<1>(info.param).format);
    if (name.find("GL_") == 0)
        name.erase(0, 3);
    if (name.find("COMPRESSED_") == 0)
        name.erase(0, 11);
    for (std::string str : {"_EXT", "_IMG", "_KHR", "_OES"})
    {
        if (name.find(str) != std::string::npos)
        {
            name.erase(name.length() - 4, 4);
            break;
        }
    }
    std::stringstream nameStr;
    nameStr << std::get<0>(info.param) << "__" << name;
    return nameStr.str();
}

static const FormatDesc kDXT1Formats[] = {{GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4, 4, 8},
                                          {GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 4, 4, 8}};

static const FormatDesc kDXT3Formats[] = {{GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 4, 4, 16}};

static const FormatDesc kDXT5Formats[] = {{GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 4, 4, 16}};

static const FormatDesc kS3TCSRGBFormats[] = {{GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, 4, 4, 8},
                                              {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 4, 4, 8},
                                              {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 4, 4, 16},
                                              {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 4, 4, 16}};

static const FormatDesc kRGTCFormats[] = {{GL_COMPRESSED_RED_RGTC1_EXT, 4, 4, 8},
                                          {GL_COMPRESSED_SIGNED_RED_RGTC1_EXT, 4, 4, 8},
                                          {GL_COMPRESSED_RED_GREEN_RGTC2_EXT, 4, 4, 16},
                                          {GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT, 4, 4, 16}};

static const FormatDesc kBPTCFormats[] = {{GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4, 16},
                                          {GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT, 4, 4, 16},
                                          {GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, 4, 4, 16},
                                          {GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT, 4, 4, 16}};

static const FormatDesc kETC1Formats[] = {{GL_ETC1_RGB8_OES, 4, 4, 8}};

// clang-format off
static const FormatDesc kEACR11UFormats[]        = {{GL_COMPRESSED_R11_EAC, 4, 4, 8}};
static const FormatDesc kEACR11SFormats[]        = {{GL_COMPRESSED_SIGNED_R11_EAC, 4, 4, 8}};
static const FormatDesc kEACRG11UFormats[]       = {{GL_COMPRESSED_RG11_EAC, 4, 4, 16}};
static const FormatDesc kEACRG11SFormats[]       = {{GL_COMPRESSED_SIGNED_RG11_EAC, 4, 4, 16}};
static const FormatDesc kETC2RGB8Formats[]       = {{GL_COMPRESSED_RGB8_ETC2, 4, 4, 8}};
static const FormatDesc kETC2RGB8SRGBFormats[]   = {{GL_COMPRESSED_SRGB8_ETC2, 4, 4, 8}};
static const FormatDesc kETC2RGB8A1Formats[]     = {{GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, 4, 4, 8}};
static const FormatDesc kETC2RGB8A1SRGBFormats[] = {{GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, 4, 4, 8}};
static const FormatDesc kETC2RGBA8Formats[]      = {{GL_COMPRESSED_RGBA8_ETC2_EAC, 4, 4, 16}};
static const FormatDesc kETC2RGBA8SRGBFormats[]  = {{GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, 4, 4, 16}};
// clang-format on

static const FormatDesc kASTCFormats[] = {{GL_COMPRESSED_RGBA_ASTC_4x4_KHR, 4, 4, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_5x4_KHR, 5, 4, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_5x5_KHR, 5, 5, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_6x5_KHR, 6, 5, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_6x6_KHR, 6, 6, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_8x5_KHR, 8, 5, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_8x6_KHR, 8, 6, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_8x8_KHR, 8, 8, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_10x5_KHR, 10, 5, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_10x6_KHR, 10, 6, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_10x8_KHR, 10, 8, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_10x10_KHR, 10, 10, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_12x10_KHR, 12, 10, 16},
                                          {GL_COMPRESSED_RGBA_ASTC_12x12_KHR, 12, 12, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR, 4, 4, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR, 5, 4, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR, 5, 5, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR, 6, 5, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR, 6, 6, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR, 8, 5, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR, 8, 6, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR, 8, 8, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR, 10, 5, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR, 10, 6, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR, 10, 8, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 10, 10, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 12, 10, 16},
                                          {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 12, 12, 16}};

static const FormatDesc kPVRTC1Formats[] = {{GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 4, 4, 8},
                                            {GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 8, 4, 8},
                                            {GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 4, 4, 8},
                                            {GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 8, 4, 8}};

static const FormatDesc kPVRTC1SRGBFormats[] = {
    {GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT, 8, 4, 8},
    {GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT, 4, 4, 8},
    {GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT, 8, 4, 8},
    {GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT, 4, 4, 8}};

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureDXT1Test,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kDXT1Formats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureDXT3Test,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kDXT3Formats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureDXT5Test,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kDXT5Formats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureS3TCSRGBTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kS3TCSRGBFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureRGTCTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kRGTCFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureBPTCTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kBPTCFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureETC1Test,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kETC1Formats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureETC1SubTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kETC1Formats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureEACR11UTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kEACR11UFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureEACR11STest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kEACR11SFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureEACRG11UTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kEACRG11UFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureEACRG11STest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kEACRG11SFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureETC2RGB8Test,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kETC2RGB8Formats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureETC2RGB8SRGBTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kETC2RGB8SRGBFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureETC2RGB8A1Test,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kETC2RGB8A1Formats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureETC2RGB8A1SRGBTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kETC2RGB8A1SRGBFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureETC2RGBA8Test,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kETC2RGBA8Formats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureETC2RGBA8SRGBTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kETC2RGBA8SRGBFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureASTCTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kASTCFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureASTCSliced3DTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kASTCFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTextureSamplerASTCSliced3DTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kASTCFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTexturePVRTC1Test,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kPVRTC1Formats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_1(CompressedTexturePVRTC1SRGBTest,
                                 PrintToStringParamName,
                                 testing::ValuesIn(kPVRTC1SRGBFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

// clang-format off
TEST_P(CompressedTextureDXT1Test,     Test) { test(); }
TEST_P(CompressedTextureDXT3Test,     Test) { test(); }
TEST_P(CompressedTextureDXT5Test,     Test) { test(); }
TEST_P(CompressedTextureS3TCSRGBTest, Test) { test(); }
TEST_P(CompressedTextureRGTCTest,     Test) { test(); }
TEST_P(CompressedTextureBPTCTest,     Test) { test(); }

TEST_P(CompressedTextureETC1Test,    Test) { test(); }
TEST_P(CompressedTextureETC1SubTest, Test) { test(); }

TEST_P(CompressedTextureEACR11UTest,  Test) { test(); }
TEST_P(CompressedTextureEACR11STest,  Test) { test(); }
TEST_P(CompressedTextureEACRG11UTest, Test) { test(); }
TEST_P(CompressedTextureEACRG11STest, Test) { test(); }

TEST_P(CompressedTextureETC2RGB8Test,       Test) { test(); }
TEST_P(CompressedTextureETC2RGB8SRGBTest,   Test) { test(); }
TEST_P(CompressedTextureETC2RGB8A1Test,     Test) { test(); }
TEST_P(CompressedTextureETC2RGB8A1SRGBTest, Test) { test(); }
TEST_P(CompressedTextureETC2RGBA8Test,      Test) { test(); }
TEST_P(CompressedTextureETC2RGBA8SRGBTest,  Test) { test(); }

TEST_P(CompressedTextureASTCTest,         Test) { test(); }
TEST_P(CompressedTextureASTCSliced3DTest, Test) { test(); }

// Check that texture sampling works correctly
TEST_P(CompressedTextureSamplerASTCSliced3DTest, Test) { testSamplerASTCSliced3D(); }

TEST_P(CompressedTexturePVRTC1Test,     Test) { test(); }
TEST_P(CompressedTexturePVRTC1SRGBTest, Test) { test(); }
// clang-format on
}  // namespace
