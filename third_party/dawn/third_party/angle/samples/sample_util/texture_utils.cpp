//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "texture_utils.h"
#include <array>
#include <cstddef>

GLuint CreateSimpleTexture2D()
{
    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, texture);

    // Load the texture: 2x2 Image, 3 bytes per pixel (R, G, B)
    const size_t width                 = 2;
    const size_t height                = 2;
    GLubyte pixels[width * height * 3] = {
        255, 0,   0,    // Red
        0,   255, 0,    // Green
        0,   0,   255,  // Blue
        255, 255, 0,    // Yellow
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

GLuint CreateSimpleTextureCubemap()
{
    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    // Load the texture faces
    GLubyte pixels[6][3] = {// Face 0 - Red
                            {255, 0, 0},
                            // Face 1 - Green,
                            {0, 255, 0},
                            // Face 3 - Blue
                            {0, 0, 255},
                            // Face 4 - Yellow
                            {255, 255, 0},
                            // Face 5 - Purple
                            {255, 0, 255},
                            // Face 6 - White
                            {255, 255, 255}};

    for (size_t i = 0; i < 6; i++)
    {
        glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, GL_RGB, 1, 1, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, &pixels[i]);
    }

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

GLuint CreateMipMappedTexture2D()
{
    // Texture object handle
    const size_t width  = 256;
    const size_t height = 256;
    std::array<GLubyte, width * height * 3> pixels;

    const size_t checkerSize = 8;
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            GLubyte rColor = 0;
            GLubyte bColor = 0;

            if ((x / checkerSize) % 2 == 0)
            {
                rColor = 255 * ((y / checkerSize) % 2);
                bColor = 255 * (1 - ((y / checkerSize) % 2));
            }
            else
            {
                bColor = 255 * ((y / checkerSize) % 2);
                rColor = 255 * (1 - ((y / checkerSize) % 2));
            }

            pixels[(y * height + x) * 3]     = rColor;
            pixels[(y * height + x) * 3 + 1] = 0;
            pixels[(y * height + x) * 3 + 2] = bColor;
        }
    }

    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, texture);

    // Load mipmap level 0
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 pixels.data());

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}
