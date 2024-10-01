// targalib.h - v0.1.0 - Targa image manipulation library - public domain
// Levent Kaya, Apr 2024
//
// Designed to be compact
// Easy to use
// Utilized throug several Computer Graphics projects
//
// Documentation
// TAR Spesification document : https://www.gamers.org/dEngine/quake3/TGA.txt
//
// Version History
// 17-04-2024   v0.1.0  Initial Release
//
// License
// See the end of the file

#ifndef __TARGALIB_H__
#define __TARGALIB_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TARGALIB_ERROR "[ERROR] targalib.h  |"
#define RETURN_FAIL 1
#define RETURN_SUCCESS 0

typedef struct tga_header {
    uint8_t id_length;
    uint8_t color_map_type;
    uint8_t image_type;
    uint16_t color_map_origin;
    uint16_t color_map_length;
    uint8_t color_map_depth;
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t width;
    uint16_t height;
    uint8_t bits_per_pixel;
    uint8_t image_descriptor;
} tga_header_t;

typedef struct tga_color {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} tga_color_t;

typedef struct tga_image {
    tga_header_t header;
    tga_color_t* image_data;
} tga_image_t;

#ifdef __cplusplus
extern "C" {
#endif

tga_image_t* tga_new(uint16_t width, uint16_t height);
void tga_free(tga_image_t* image);
int tga_read(const char* filename, tga_image_t* image);
int tga_write(const char* filename, const tga_image_t* image);
void tga_set_pixel(const tga_image_t* image, int x, int y, tga_color_t color);
void tga_set_bg(const tga_image_t* image, tga_color_t color);
void tga_flip_horizontally();
void tga_flip_vertically();
int tga_resize_image(tga_image_t* image, int new_width, int new_height);
void tga_dump_headers(const tga_image_t* image, const char* output_file);

#ifdef __cplusplus
}
#endif

#endif //__TARGALIB_H__

#ifdef TARGALIB_IMPLEMENTATION

int tga_read(const char *filename, tga_image_t *image)
{
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "%s Unable to open file '%s' for reading.\n",
            TARGALIB_ERROR, filename);
        return RETURN_FAIL;
    }

    // Read TGA header
    fread(&image->header.id_length, sizeof(image->header.id_length), 1,
          file);
    fread(&image->header.color_map_type,
          sizeof(image->header.color_map_type), 1, file);
    fread(&image->header.image_type, sizeof(image->header.image_type), 1,
          file);
    fread(&image->header.color_map_origin,
          sizeof(image->header.color_map_origin), 1, file);
    fread(&image->header.color_map_length,
          sizeof(image->header.color_map_length), 1, file);
    fread(&image->header.color_map_depth,
          sizeof(image->header.color_map_depth), 1, file);
    fread(&image->header.x_origin, sizeof(image->header.x_origin), 1, file);
    fread(&image->header.y_origin, sizeof(image->header.y_origin), 1, file);
    fread(&image->header.width, sizeof(image->header.width), 1, file);
    fread(&image->header.height, sizeof(image->header.height), 1, file);
    fread(&image->header.bits_per_pixel,
          sizeof(image->header.bits_per_pixel), 1, file);
    fread(&image->header.image_descriptor,
          sizeof(image->header.image_descriptor), 1, file);

    // Skip image ID field if present
    if (image->header.id_length > 0) {
        fseek(file, image->header.id_length, SEEK_CUR);
    }

    // Allocate memory for image data
    image->image_data = (tga_color_t *)malloc(
        image->header.width * image->header.height *
        (image->header.bits_per_pixel / 8));
    if (!image->image_data) {
        fclose(file);
        fprintf(stderr,
            "%s Failed to allocate memory for image(%s) data.\n,",
            TARGALIB_ERROR, filename);
        return RETURN_FAIL;
    }

    // Read image data
    fread(image->image_data, 1,
          image->header.width * image->header.height *
              (image->header.bits_per_pixel / 8),
          file);

    fclose(file);

    return RETURN_SUCCESS;
}

void tga_dump_headers(const tga_image_t *image, const char *output_file)
{
    FILE *dump_file = fopen(output_file, "w");
    if (!dump_file) {
        fprintf(stderr, "%s Failed to create header dump file.",
            TARGALIB_ERROR);
        fclose(dump_file);
        return;
    }
    fprintf(dump_file, "TGA HEADER DUMP:\n");
    fprintf(dump_file, "\tid_length:\t%d\n", image->header.id_length);
    fprintf(dump_file, "\tcolor_map_type:\t%d\n",
        image->header.color_map_type);
    fprintf(dump_file, "\timage_type:\t%d\n", image->header.image_type);
    fprintf(dump_file, "\tcolor_map_origin:\t%d\n",
        image->header.color_map_origin);
    fprintf(dump_file, "\tcolor_map_length:\t%d\n",
        image->header.color_map_length);
    fprintf(dump_file, "\tcolor_map_depth:\t%d\n",
        image->header.color_map_depth);
    fprintf(dump_file, "\tx_origin:\t%d\n", image->header.x_origin);
    fprintf(dump_file, "\ty_origin:\t%d\n", image->header.y_origin);
    fprintf(dump_file, "\twidth:\t%d\n", image->header.width);
    fprintf(dump_file, "\theight:\t%d\n", image->header.height);
    fprintf(dump_file, "\tbits_per_pixel:\t%d\n",
        image->header.bits_per_pixel);
    fprintf(dump_file, "\timage_descriptor:\t%d\n",
        image->header.image_descriptor);

    fclose(dump_file);
}

tga_image_t *tga_new(uint16_t width, uint16_t height)
{
    tga_image_t *image = (tga_image_t *)malloc(sizeof(tga_image_t));
    if (!image) {
        fprintf(stderr, "%s Failed to allocate memory for TGA image.\n",
            TARGALIB_ERROR);
        return NULL;
    }
    // Initialize TGA header fields
    image->header.id_length = 0;
    image->header.color_map_type = 0;
    image->header.image_type = 2; // Uncompressed true-color image
    image->header.color_map_origin = 0;
    image->header.color_map_length = 0;
    image->header.color_map_depth = 0;
    image->header.x_origin = 0;
    image->header.y_origin = 0;
    image->header.width = width;
    image->header.height = height;
    image->header.bits_per_pixel = 24; // 24 bits per pixel (RGB)
    image->header.image_descriptor = 0x20; // Default image descriptor

    image->image_data =
        (tga_color_t *)malloc(width * height * sizeof(tga_color_t));
    if (!image->image_data) {
        fprintf(stderr,
            "%s Failed to allocate memory for image data.\n",
            TARGALIB_ERROR);
        free(image);
        return NULL;
    }

    memset(image->image_data, 0, width * height * sizeof(tga_color_t));

    return image;
}

int tga_write(const char *filename, const tga_image_t *image)
{
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "%s Unable to open file '%s' for writing.\n",
            TARGALIB_ERROR, filename);
        return RETURN_FAIL;
    }

    // Write TGA header
    fwrite(&image->header.id_length, sizeof(image->header.id_length), 1,
           file);
    fwrite(&image->header.color_map_type,
           sizeof(image->header.color_map_type), 1, file);
    fwrite(&image->header.image_type, sizeof(image->header.image_type), 1,
           file);
    fwrite(&image->header.color_map_origin,
           sizeof(image->header.color_map_origin), 1, file);
    fwrite(&image->header.color_map_length,
           sizeof(image->header.color_map_length), 1, file);
    fwrite(&image->header.color_map_depth,
           sizeof(image->header.color_map_depth), 1, file);
    fwrite(&image->header.x_origin, sizeof(image->header.x_origin), 1,
           file);
    fwrite(&image->header.y_origin, sizeof(image->header.y_origin), 1,
           file);
    fwrite(&image->header.width, sizeof(image->header.width), 1, file);
    fwrite(&image->header.height, sizeof(image->header.height), 1, file);
    fwrite(&image->header.bits_per_pixel,
           sizeof(image->header.bits_per_pixel), 1, file);
    fwrite(&image->header.image_descriptor,
           sizeof(image->header.image_descriptor), 1, file);

    // Write image data
    fwrite(image->image_data, 1,
           image->header.width * image->header.height *
               (image->header.bits_per_pixel / 8),
           file);

    fclose(file);
    return RETURN_SUCCESS;
}

void tga_free(tga_image_t *image)
{
    if (image) {
        free(image->image_data);
        free(image);
    }
}

void tga_set_bg(const tga_image_t *image, tga_color_t color)
{
    for (int y = 0; y < image->header.height; y++) {
        for (int x = 0; x < image->header.width; x++) {
            image->image_data[y * image->header.width + x] = color;
        }
    }
}

void tga_set_pixel(const tga_image_t *image, int x, int y, tga_color_t color)
{
    if (x >= 0 && x < image->header.width && y >= 0 &&
        y < image->header.height) {
        image->image_data[y * image->header.width + x] = color;
    } else {
        fprintf(stderr, "%s Invalid dot coordinates. Dot not drawn.\n",
            TARGALIB_ERROR);
        return;
    }
}

#endif //TARGALIB_IMPLEMENTATION
/*
-------------------------------------------------------------------------------
This software available under unlicense
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>
-------------------------------------------------------------------------------
*/