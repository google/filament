//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "tga_utils.h"

#include <fstream>
#include <iostream>
#include <limits.h>
#include <stdint.h>
#include <string>

TGAImage::TGAImage()
    : width(0), height(0), data(0)
{
}

struct TGAHeader
{
    uint8_t idSize;
    uint8_t mapType;
    uint8_t imageType;
    uint16_t paletteStart;
    uint16_t paletteSize;
    uint8_t paletteEntryDepth;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint8_t colorDepth;
    uint8_t descriptor;
};

#define INVERTED_BIT (1 << 5)

template <typename dataType>
void readBinary(std::ifstream &stream, dataType &item)
{
    stream.read(reinterpret_cast<char *>(&item), sizeof(dataType));
}

template <typename dataType>
void readBinary(std::ifstream &stream, std::vector<dataType> &items)
{
    stream.read(reinterpret_cast<char *>(items.data()), sizeof(dataType) * items.size());
}

bool LoadTGAImageFromFile(const std::string &path, TGAImage *image)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream)
    {
        std::cerr << "error opening tga file " << path << " for reading.\n";
        return false;
    }

    TGAHeader header;
    readBinary(stream, header.idSize);
    readBinary(stream, header.mapType);
    readBinary(stream, header.imageType);
    readBinary(stream, header.paletteStart);
    readBinary(stream, header.paletteSize);
    readBinary(stream, header.paletteEntryDepth);
    readBinary(stream, header.x);
    readBinary(stream, header.y);
    readBinary(stream, header.width);
    readBinary(stream, header.height);
    readBinary(stream, header.colorDepth);
    readBinary(stream, header.descriptor);

    image->width = header.width;
    image->height = header.height;

    size_t pixelComponentCount = header.colorDepth / CHAR_BIT;
    std::vector<unsigned char> buffer(header.width * header.height * pixelComponentCount);
    readBinary(stream, buffer);

    image->data.reserve(header.width * header.height);

    for (size_t y = 0; y < header.height; y++)
    {
        size_t rowIdx = ((header.descriptor & INVERTED_BIT) ? (header.height - 1 - y) : y) * header.width * pixelComponentCount;
        for (size_t x = 0; x < header.width; x++)
        {
            size_t pixelIdx = rowIdx + x * pixelComponentCount;

            Byte4 pixel;
            pixel[0] = (pixelComponentCount > 2) ? buffer[pixelIdx + 2] : 0;
            pixel[2] = (pixelComponentCount > 0) ? buffer[pixelIdx + 0] : 0;
            pixel[1] = (pixelComponentCount > 1) ? buffer[pixelIdx + 1] : 0;
            pixel[3] = (pixelComponentCount > 3) ? buffer[pixelIdx + 3] : 255;

            image->data.push_back(pixel);
        }
    }

    std::cout << "loaded image " << path << ".\n";

    return true;
}

GLuint LoadTextureFromTGAImage(const TGAImage &image)
{
    if (image.width > 0 && image.height > 0)
    {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(image.width), static_cast<GLsizei>(image.height), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, image.data.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        return texture;
    }
    else
    {
        return 0;
    }
}
