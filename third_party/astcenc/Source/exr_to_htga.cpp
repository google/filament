/*----------------------------------------------------------------------------*/
/**
 *	This confidential and proprietary software may be used only as
 *	authorised by a licensing agreement from ARM Limited
 *	(C) COPYRIGHT 2011-2012 ARM Limited
 *	ALL RIGHTS RESERVED
 *
 *	The entire notice above must be reproduced on all authorised
 *	copies and copies may only be made to the extent permitted
 *	by a licensing agreement from ARM Limited.
 *
 *	@brief	Program to convert FP16 images between EXR and HTGA (half-float TGA)
 *
 *			This program uses TGA imagetypes 130/131 (non-standard);
 *			the data are stored as FP16 B,G,R,A (B in first two bytes, then G
 *			in next two, then R in the next two, then alpha in the final two
 *			bytes.
 */
/*----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>
#include <ImfRgba.h>

#include <iostream>


using namespace std;
using namespace Imf;
using namespace Imath;

struct tga_header
{
	uint8_t identsize;
	uint8_t colormaptype;
	uint8_t imagetype;
	uint8_t dummied[5];
	uint16_t xstart;
	uint16_t ystart;
	uint16_t xsize;
	uint16_t ysize;
	uint8_t bitsperpixel;
	uint8_t descriptor;
};


void convert_exr_to_htga(const char *exr_filename, const char *htga_filename)
{
	tga_header hdr = { 0, 0, 128 + 2, {0, 0, 0, 0, 0}, 0, 0, 0, 0, 64, 0x00 };

	RgbaInputFile file(exr_filename);
	Box2i dw = file.dataWindow();

	int width = dw.max.x - dw.min.x + 1;
	int height = dw.max.y - dw.min.y + 1;
	Array2D < Rgba > pixels(height, width);

	pixels.resizeErase(height, width);

	file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
	file.readPixels(dw.min.y, dw.max.y);


	uint16_t *buffer = (uint16_t *) malloc(width * height * 8);

	uint16_t *b2 = buffer;

	int x, y;
	for (y = 0; y < height; y++)
	{
		int y_src = y;
		// int y_src = height - y - 1;
		for (x = 0; x < width; x++)
		{
			*b2++ = pixels[y_src][x].b.bits();
			*b2++ = pixels[y_src][x].g.bits();
			*b2++ = pixels[y_src][x].r.bits();
			*b2++ = pixels[y_src][x].a.bits();
		}
	}

	hdr.xsize = width;
	hdr.ysize = height;

	FILE *f = fopen(htga_filename, "wb");
	fwrite(&hdr, 1, 18, f);
	fwrite(buffer, 1, 8 * width * height, f);
	fclose(f);

	return;
}

int convert_htga_to_exr(const char *htga_filename, const char *exr_filename)
{
	int i;
	tga_header hdr;

	FILE *f = fopen(htga_filename, "rb");
	if (!f)
		return -1;

	int bytes_read;
	bytes_read = fread(&hdr, 1, 18, f);
	if (bytes_read < 18)
	{
		fclose(f);
		return -1;
	}

	int width = hdr.xsize;
	int height = hdr.ysize;
	int bitsperpixel = hdr.bitsperpixel;

	fseek(f, hdr.identsize, SEEK_CUR);

	uint16_t *buf = new uint16_t[width * height * (bitsperpixel / 16)];
	int bytestoread = width * height * (bitsperpixel / 16) * 2;
	bytes_read = fread(buf, 1, bytestoread, f);
	fclose(f);
	if (bytes_read != bytestoread)
	{
		delete[]buf;
		return -1;
	}

	int pixelcount = width * height;
	Rgba *pixels = new Rgba[pixelcount];

	int wordsperpixel = bitsperpixel / 16;

	int y;
	for (y = 0; y < height; y++)
	{
		uint16_t *src = buf + (width * y * wordsperpixel);

		int dst = width * y;
		switch (hdr.bitsperpixel)
		{
		case 16:				// Luminance
			for (i = 0; i < width; i++)
			{
				pixels[i + dst].r.setBits(src[0]);
				pixels[i + dst].g.setBits(src[0]);
				pixels[i + dst].b.setBits(src[0]);
				pixels[i + dst].a.setBits(0x3C00);
				src++;
			}
			break;
		case 32:				// Luminance-Alpha
			for (i = 0; i < width; i++)
			{
				pixels[i + dst].r.setBits(src[0]);
				pixels[i + dst].g.setBits(src[0]);
				pixels[i + dst].b.setBits(src[0]);
				pixels[i + dst].a.setBits(src[1]);
				src += 2;
			}
			break;
		case 48:				// RGB
			for (i = 0; i < width; i++)
			{
				pixels[i + dst].r.setBits(src[2]);
				pixels[i + dst].g.setBits(src[1]);
				pixels[i + dst].b.setBits(src[0]);
				pixels[i + dst].a.setBits(0x3C00);
				src += 3;
			}
			break;
		case 64:
			for (i = 0; i < width; i++)
			{
				pixels[i + dst].r.setBits(src[2]);
				pixels[i + dst].g.setBits(src[1]);
				pixels[i + dst].b.setBits(src[0]);
				pixels[i + dst].a.setBits(src[3]);
				src += 4;
			}
		}
	}
	RgbaOutputFile file(exr_filename, width, height, WRITE_RGBA);
	file.setFrameBuffer(pixels, 1, width);
	file.writePixels(height);
	delete[]buf;
	delete[]pixels;
	return 0;
}

int main(int argc, char **argv)
{

	if (argc < 4)
	{
		printf("EXR <-> HTGA (half-float-TGA) conversion tool\n"
			   "Usage:\n" "    %s <operation> <input_filename> <output_filename>\n" "where operation may be\n" " -q : convert from EXR to HTGA\n" " -e : convert from HTGA to EXR\n", argv[0]);
		return 1;
	}

	if (!strcmp(argv[1], "-q"))
		convert_exr_to_htga(argv[2], argv[3]);
	else if (!strcmp(argv[1], "-e"))
		convert_htga_to_exr(argv[2], argv[3]);


	return 0;

}
