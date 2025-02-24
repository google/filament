//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// png_utils: Wrapper around libpng.
//

#include "util/png_utils.h"

#include <array>
#include <cstring>

#include <png.h>

namespace angle
{
namespace
{
class ScopedFILE
{
  public:
    ScopedFILE(FILE *fp) : mFP(fp) {}
    ~ScopedFILE() { close(); }

    FILE *get() const { return mFP; }

    void close()
    {
        if (mFP)
        {
            fclose(mFP);
            mFP = nullptr;
        }
    }

  private:
    FILE *mFP;
};
}  // namespace

bool SavePNGRGB(const char *fileName,
                const char *title,
                uint32_t width,
                uint32_t height,
                const std::vector<uint8_t> &data)
{
    ScopedFILE fp(fopen(fileName, "wb"));
    if (!fp.get())
    {
        fprintf(stderr, "Error opening '%s'.\n", fileName);
        return false;
    }

    png_struct *writeStruct =
        png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!writeStruct)
    {
        fprintf(stderr, "Error on png_create_write_struct.\n");
        return false;
    }

    png_info *infoStruct = png_create_info_struct(writeStruct);
    if (!infoStruct)
    {
        fprintf(stderr, "Error on png_create_info_struct.\n");
        return false;
    }

    if (setjmp(png_jmpbuf(writeStruct)))
    {
        fp.close();
        png_free_data(writeStruct, infoStruct, PNG_FREE_ALL, -1);
        png_destroy_write_struct(&writeStruct, &infoStruct);
        return false;
    }

    png_init_io(writeStruct, fp.get());

    // Write header (8 bit colour depth)
    png_set_IHDR(writeStruct, infoStruct, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    // Set title
    if (title != nullptr && strlen(title) > 0)
    {
        std::array<char, 50> mutableKey = {};
        strcpy(mutableKey.data(), "Title");
        std::array<char, 200> mutableText = {};
        strncpy(mutableText.data(), title, 199);

        png_text titleText;
        titleText.compression = PNG_TEXT_COMPRESSION_NONE;
        titleText.key         = mutableKey.data();
        titleText.text        = mutableText.data();
        png_set_text(writeStruct, infoStruct, &titleText, 1);
    }

    png_write_info(writeStruct, infoStruct);

    // RGB 3-byte stride.
    const uint32_t rowStride = width * 3;
    for (uint32_t row = 0; row < height; ++row)
    {
        uint32_t rowOffset = row * rowStride;
        png_write_row(writeStruct, &data[rowOffset]);
    }

    png_write_end(writeStruct, infoStruct);

    fp.close();
    png_free_data(writeStruct, infoStruct, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&writeStruct, &infoStruct);
    return true;
}
}  // namespace angle
