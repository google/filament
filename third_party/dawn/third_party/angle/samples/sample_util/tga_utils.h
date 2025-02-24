//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef SAMPLE_UTIL_TGA_UTILS_HPP
#define SAMPLE_UTIL_TGA_UTILS_HPP

#include <array>
#include <string>
#include <vector>

#include "util/gles_loader_autogen.h"

typedef std::array<unsigned char, 4> Byte4;

struct TGAImage
{
    size_t width;
    size_t height;
    std::vector<Byte4> data;

    TGAImage();
};

bool LoadTGAImageFromFile(const std::string &path, TGAImage *image);
GLuint LoadTextureFromTGAImage(const TGAImage &image);

#endif  // SAMPLE_UTIL_TGA_UTILS_HPP
