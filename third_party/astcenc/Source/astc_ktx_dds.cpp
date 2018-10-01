/*----------------------------------------------------------------------------*/
/**
 *	This confidential and proprietary software may be used only as
 *	authorised by a licensing agreement from ARM Limited
 *	(C) COPYRIGHT 2011-2013 ARM Limited
 *	ALL RIGHTS RESERVED
 *
 *	The entire notice above must be reproduced on all authorised
 *	copies and copies may only be made to the extent permitted
 *	by a licensing agreement from ARM Limited.
 *
 *	@brief	Functions to load/store KTX and DDS files
 *
 *			The files that are loaded and stored by these functions are
 *			UNCOMPRESSED files only; the intent is mainly to provide a
 *			standard way to get HDR and 3D texture data to/from the codec.
 *
 *			Status: only tested on our corpus of images.
 */
/*----------------------------------------------------------------------------*/
#include "astc_codec_internals.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "softfloat.h"

enum scanline_copy_method
{
	R8_TO_RGBA8,
	RG8_TO_RGBA8,
	RGB8_TO_RGBA8,
	RGBA8_TO_RGBA8,
	BGR8_TO_RGBA8,
	BGRA8_TO_RGBA8,
	L8_TO_RGBA8,
	LA8_TO_RGBA8,

	RGBX8_TO_RGBA8,
	BGRX8_TO_RGBA8,

	R16_TO_RGBA16F,
	RG16_TO_RGBA16F,
	RGB16_TO_RGBA16F,
	RGBA16_TO_RGBA16F,
	BGR16_TO_RGBA16F,
	BGRA16_TO_RGBA16F,
	L16_TO_RGBA16F,
	LA16_TO_RGBA16F,

	R16F_TO_RGBA16F,
	RG16F_TO_RGBA16F,
	RGB16F_TO_RGBA16F,
	RGBA16F_TO_RGBA16F,
	BGR16F_TO_RGBA16F,
	BGRA16F_TO_RGBA16F,
	L16F_TO_RGBA16F,
	LA16F_TO_RGBA16F,

	R32F_TO_RGBA16F,
	RG32F_TO_RGBA16F,
	RGB32F_TO_RGBA16F,
	RGBA32F_TO_RGBA16F,
	BGR32F_TO_RGBA16F,
	BGRA32F_TO_RGBA16F,
	L32F_TO_RGBA16F,
	LA32F_TO_RGBA16F
};



// scanline copying function: this function expands data to RGBA, either U8 or FP16.

static void copy_scanline(void *dst, const void *src, int pixels, int method)
{

#define id(x) (x)
#define u16_sf16(x) float_to_sf16( x * (1.0f/65535.0f), SF_NEARESTEVEN )
#define f32_sf16(x) sf32_to_sf16( x, SF_NEARESTEVEN )

#define COPY_R( dsttype, srctype, convfunc, oneval ) \
	do { \
	srctype *s = (srctype *)src; \
	dsttype *d = (dsttype *)dst; \
	for(i=0;i<pixels;i++)\
		{\
		d[4*i] = convfunc(s[i]); \
		d[4*i+1] = 0; \
		d[4*i+2] = 0; \
		d[4*i+3] = oneval; \
		} \
	} while(0); \
	break;

#define COPY_RG( dsttype, srctype, convfunc, oneval ) \
	do { \
	srctype *s = (srctype *)src; \
	dsttype *d = (dsttype *)dst; \
	for(i=0;i<pixels;i++)\
		{\
		d[4*i] = convfunc(s[2*i]); \
		d[4*i+1] = convfunc(s[2*i+1]); \
		d[4*i+2] = 0; \
		d[4*i+3] = oneval; \
		} \
	} while(0); \
	break;

#define COPY_RGB( dsttype, srctype, convfunc, oneval ) \
	do { \
	srctype *s = (srctype *)src; \
	dsttype *d = (dsttype *)dst; \
	for(i=0;i<pixels;i++)\
		{\
		d[4*i] = convfunc(s[3*i]); \
		d[4*i+1] = convfunc(s[3*i+1]); \
		d[4*i+2] = convfunc(s[3*i+2]); \
		d[4*i+3] = oneval; \
		} \
	} while(0); \
	break;

#define COPY_BGR( dsttype, srctype, convfunc, oneval ) \
	do { \
	srctype *s = (srctype *)src; \
	dsttype *d = (dsttype *)dst; \
	for(i=0;i<pixels;i++)\
		{\
		d[4*i] = convfunc(s[3*i+2]); \
		d[4*i+1] = convfunc(s[3*i+1]); \
		d[4*i+2] = convfunc(s[3*i]); \
		d[4*i+3] = oneval; \
		} \
	} while(0); \
	break;

#define COPY_RGBX( dsttype, srctype, convfunc, oneval ) \
	do { \
	srctype *s = (srctype *)src; \
	dsttype *d = (dsttype *)dst; \
	for(i=0;i<pixels;i++)\
		{\
		d[4*i] = convfunc(s[4*i]); \
		d[4*i+1] = convfunc(s[4*i+1]); \
		d[4*i+2] = convfunc(s[4*i+2]); \
		d[4*i+3] = oneval; \
		} \
	} while(0); \
	break;

#define COPY_BGRX( dsttype, srctype, convfunc, oneval ) \
	do { \
	srctype *s = (srctype *)src; \
	dsttype *d = (dsttype *)dst; \
	for(i=0;i<pixels;i++)\
		{\
		d[4*i] = convfunc(s[4*i+2]); \
		d[4*i+1] = convfunc(s[4*i+1]); \
		d[4*i+2] = convfunc(s[4*i]); \
		d[4*i+3] = oneval; \
		} \
	} while(0); \
	break;

#define COPY_RGBA( dsttype, srctype, convfunc, oneval ) \
	do { \
	srctype *s = (srctype *)src; \
	dsttype *d = (dsttype *)dst; \
	for(i=0;i<pixels;i++)\
		{\
		d[4*i] = convfunc(s[4*i]); \
		d[4*i+1] = convfunc(s[4*i+1]); \
		d[4*i+2] = convfunc(s[4*i+2]); \
		d[4*i+3] = convfunc(s[4*i+3]); \
		} \
	} while(0); \
	break;

#define COPY_BGRA( dsttype, srctype, convfunc, oneval ) \
	do { \
	srctype *s = (srctype *)src; \
	dsttype *d = (dsttype *)dst; \
	for(i=0;i<pixels;i++)\
		{\
		d[4*i] = convfunc(s[4*i+2]); \
		d[4*i+1] = convfunc(s[4*i+1]); \
		d[4*i+2] = convfunc(s[4*i]); \
		d[4*i+3] = convfunc(s[4*i+3]); \
		} \
	} while(0); \
	break;

#define COPY_L( dsttype, srctype, convfunc, oneval ) \
	do { \
	srctype *s = (srctype *)src; \
	dsttype *d = (dsttype *)dst; \
	for(i=0;i<pixels;i++)\
		{\
		d[4*i] = convfunc(s[i]); \
		d[4*i+1] = convfunc(s[i]); \
		d[4*i+2] = convfunc(s[i]); \
		d[4*i+3] = oneval; \
		} \
	} while(0); \
	break;

#define COPY_LA( dsttype, srctype, convfunc, oneval ) \
	do { \
	srctype *s = (srctype *)src; \
	dsttype *d = (dsttype *)dst; \
	for(i=0;i<pixels;i++)\
		{\
		d[4*i] = convfunc(s[2*i]); \
		d[4*i+1] = convfunc(s[2*i]); \
		d[4*i+2] = convfunc(s[2*i]); \
		d[4*i+3] = convfunc(s[2*i+1]); \
		} \
	} while(0); \
	break;

	int i;
	switch (method)
	{
	case R8_TO_RGBA8:
		COPY_R(uint8_t, uint8_t, id, 0xFF);
	case RG8_TO_RGBA8:
		COPY_RG(uint8_t, uint8_t, id, 0xFF);
	case RGB8_TO_RGBA8:
		COPY_RGB(uint8_t, uint8_t, id, 0xFF);
	case RGBA8_TO_RGBA8:
		COPY_RGBA(uint8_t, uint8_t, id, 0xFF);
	case BGR8_TO_RGBA8:
		COPY_BGR(uint8_t, uint8_t, id, 0xFF);
	case BGRA8_TO_RGBA8:
		COPY_BGRA(uint8_t, uint8_t, id, 0xFF);
	case RGBX8_TO_RGBA8:
		COPY_RGBX(uint8_t, uint8_t, id, 0xFF);
	case BGRX8_TO_RGBA8:
		COPY_BGRX(uint8_t, uint8_t, id, 0xFF);
	case L8_TO_RGBA8:
		COPY_L(uint8_t, uint8_t, id, 0xFF);
	case LA8_TO_RGBA8:
		COPY_LA(uint8_t, uint8_t, id, 0xFF);

	case R16F_TO_RGBA16F:
		COPY_R(uint16_t, uint16_t, id, 0x3C00);
	case RG16F_TO_RGBA16F:
		COPY_RG(uint16_t, uint16_t, id, 0x3C00);
	case RGB16F_TO_RGBA16F:
		COPY_RGB(uint16_t, uint16_t, id, 0x3C00);
	case RGBA16F_TO_RGBA16F:
		COPY_RGBA(uint16_t, uint16_t, id, 0x3C00);
	case BGR16F_TO_RGBA16F:
		COPY_BGR(uint16_t, uint16_t, id, 0x3C00);
	case BGRA16F_TO_RGBA16F:
		COPY_BGRA(uint16_t, uint16_t, id, 0x3C00);
	case L16F_TO_RGBA16F:
		COPY_L(uint16_t, uint16_t, id, 0x3C00);
	case LA16F_TO_RGBA16F:
		COPY_LA(uint16_t, uint16_t, id, 0x3C00);

	case R16_TO_RGBA16F:
		COPY_R(uint16_t, uint16_t, u16_sf16, 0x3C00);
	case RG16_TO_RGBA16F:
		COPY_RG(uint16_t, uint16_t, u16_sf16, 0x3C00);
	case RGB16_TO_RGBA16F:
		COPY_RGB(uint16_t, uint16_t, u16_sf16, 0x3C00);
	case RGBA16_TO_RGBA16F:
		COPY_RGBA(uint16_t, uint16_t, u16_sf16, 0x3C00);
	case BGR16_TO_RGBA16F:
		COPY_BGR(uint16_t, uint16_t, u16_sf16, 0x3C00);
	case BGRA16_TO_RGBA16F:
		COPY_BGRA(uint16_t, uint16_t, u16_sf16, 0x3C00);
	case L16_TO_RGBA16F:
		COPY_L(uint16_t, uint16_t, u16_sf16, 0x3C00);
	case LA16_TO_RGBA16F:
		COPY_LA(uint16_t, uint16_t, u16_sf16, 0x3C00);

	case R32F_TO_RGBA16F:
		COPY_R(uint16_t, uint32_t, f32_sf16, 0x3C00);
	case RG32F_TO_RGBA16F:
		COPY_RG(uint16_t, uint32_t, f32_sf16, 0x3C00);
	case RGB32F_TO_RGBA16F:
		COPY_RGB(uint16_t, uint32_t, f32_sf16, 0x3C00);
	case RGBA32F_TO_RGBA16F:
		COPY_RGBA(uint16_t, uint32_t, f32_sf16, 0x3C00);
	case BGR32F_TO_RGBA16F:
		COPY_BGR(uint16_t, uint32_t, f32_sf16, 0x3C00);
	case BGRA32F_TO_RGBA16F:
		COPY_BGRA(uint16_t, uint32_t, f32_sf16, 0x3C00);
	case L32F_TO_RGBA16F:
		COPY_L(uint16_t, uint32_t, f32_sf16, 0x3C00);
	case LA32F_TO_RGBA16F:
		COPY_LA(uint16_t, uint32_t, f32_sf16, 0x3C00);
	};
}


// perform endianness switch on raw data
static void switch_endianness2(void *dataptr, int bytes)
{
	int i;
	uint8_t *data = (uint8_t *) dataptr;

	for (i = 0; i < bytes / 2; i++)
	{
		uint8_t d0 = data[0];
		uint8_t d1 = data[1];
		data[0] = d1;
		data[1] = d0;
		data += 2;
	}
}


static void switch_endianness4(void *dataptr, int bytes)
{
	int i;
	uint8_t *data = (uint8_t *) dataptr;

	for (i = 0; i < bytes / 4; i++)
	{
		uint8_t d0 = data[0];
		uint8_t d1 = data[1];
		uint8_t d2 = data[2];
		uint8_t d3 = data[3];
		data[0] = d3;
		data[1] = d2;
		data[2] = d1;
		data[3] = d0;
		data += 4;
	}
}


uint32_t u32_byterev(uint32_t v)
{
	return (v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24);
}





/*
	Notes about KTX:

	After the header and the key/value data area, the actual image data follows.
	Each image starts with a 4-byte "imageSize" value indicating the number of bytes of image data follow.
	(For cube-maps, this value appears only after first image; the remaining 5 images are all of equal size.)
	If the size of an image is not a multiple of 4, then it is padded to the next multiple of 4.
	Note that this padding is NOT included in the "imageSize" field.
	In a cubemap, the padding appears after each face note that in a 2D/3D texture, padding does
	NOT appear between the lines/planes of the texture!

	In a KTX file, there may be multiple images; they are organized as follows:

	For each mipmap_level in numberOfMipmapLevels
		UInt32 imageSize;
		For each array_element in numberOfArrayElements
		* for each face in numberOfFaces
			* for each z_slice in pixelDepth
				* for each row or row_of_blocks in pixelHeight
					* for each pixel or block_of_pixels in pixelWidth
						Byte data[format-specific-number-of-bytes]
					* end
				* end
			*end
			Byte cubePadding[0-3]
		*end
		Byte mipPadding[3 - ((imageSize+ 3) % 4)]
	*end

	In the ASTC codec, we will, for the time being only harvest the first image,
	and we will support only a limited set of formats:

	gl_type: UNSIGNED_BYTE UNSIGNED_SHORT HALF_FLOAT FLOAT UNSIGNED_INT_8_8_8_8 UNSIGNED_INT_8_8_8_8_REV
	gl_format: RED, RG. RGB, RGBA BGR, BGRA
	gl_internal_format: used for upload to OpenGL; we can ignore it on uncompressed-load, but
		need to provide a reasonable value on store: RGB8 RGBA8 RGB16F RGBA16F
   gl_base_internal_format: same as gl_format unless texture is compressed (well, BGR is turned into RGB)
		RED, RG, RGB, RGBA

 */


// enums copied from GL/GL.h
#define GL_RED    0x1903
#define GL_RG     0x8227
#define GL_RGB    0x1907
#define GL_RGBA   0x1908
#define GL_BGR    0x80E0
#define GL_BGRA   0x80E1
#define GL_LUMINANCE        0x1909
#define GL_LUMINANCE_ALPHA  0x190A

#define GL_UNSIGNED_BYTE   0x1401
#define GL_UNSIGNED_SHORT  0x1403
#define GL_HALF_FLOAT      0x140B
#define GL_FLOAT           0x1406

struct ktx_header
{
	uint8_t magic[12];
	uint32_t endianness;		// should be 0x04030201; if it is instead 0x01020304, then the endianness of everything must be switched.
	uint32_t gl_type;			// 0 for compressed textures, otherwise value from table 3.2 (page 162) of OpenGL 4.0 spec
	uint32_t gl_type_size;		// size of data elements to do endianness swap on (1=endian-neutral data)
	uint32_t gl_format;			// 0 for compressed textures, otherwise value from table 3.3 (page 163) of OpenGLl spec
	uint32_t gl_internal_format;	// sized-internal-format, corresponding to table 3.12 to 3.14 (pages 182-185) of OpenGL spec
	uint32_t gl_base_internal_format;	// unsized-internal-format: corresponding to table 3.11 (page 179) of OpenGL spec
	uint32_t pixel_width;		// texture dimensions; not rounded up to block size for compressed.
	uint32_t pixel_height;		// must be 0 for 1D textures.
	uint32_t pixel_depth;		// must be 0 for 1D, 2D and cubemap textures.
	uint32_t number_of_array_elements;	// 0 if not a texture array
	uint32_t number_of_faces;	// 6 for cubemaps, 1 for non-cubemaps
	uint32_t number_of_mipmap_levels;	// 0 or 1 for non-mipmapped textures; 0 indicates that auto-mipmap-gen should be done at load time.
	uint32_t bytes_of_key_value_data;	// size in bytes of the key-and-value area immediately following the header.
};


// magic 12-byte sequence that must appear at the beginning of every KTX file.
uint8_t ktx_magic[12] = {
	0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

static void ktx_header_switch_endianness(ktx_header * kt)
{
	#define REV(x) kt->x = u32_byterev( kt->x )
	REV(endianness);
	REV(gl_type);
	REV(gl_type_size);
	REV(gl_format);
	REV(gl_internal_format);
	REV(gl_base_internal_format);
	REV(pixel_width);
	REV(pixel_height);
	REV(pixel_depth);
	REV(number_of_array_elements);
	REV(number_of_faces);
	REV(number_of_mipmap_levels);
	REV(bytes_of_key_value_data);
	#undef REV
}





astc_codec_image *load_ktx_uncompressed_image(const char *filename, int padding, int *result)
{
	int y, z;

	FILE *f = fopen(filename, "rb");
	if (!f)
	{
		printf("Failed to open file %s\n", filename);
		*result = -1;
		return NULL;
	}

	ktx_header hdr;
	size_t header_bytes_read = fread(&hdr, 1, sizeof(hdr), f);

	if (header_bytes_read != sizeof(hdr))
	{
		printf("Failed to read header of KTX file %s\n", filename);
		fclose(f);
		*result = -2;
		return NULL;
	}

	if (memcmp(hdr.magic, ktx_magic, 12) != 0 || (hdr.endianness != 0x04030201 && hdr.endianness != 0x01020304))
	{
		printf("File %s does not have a valid KTX header\n", filename);
		fclose(f);
		*result = -3;
		return NULL;
	}

	int switch_endianness = 0;
	if (hdr.endianness == 0x01020304)
	{
		ktx_header_switch_endianness(&hdr);
		switch_endianness = 1;
	}

	if (hdr.gl_type == 0 || hdr.gl_format == 0)
	{
		printf("File %s appears to be compressed, not supported as input\n", filename);
		fclose(f);
		*result = -4;
		return NULL;
	}

	// the formats we support are:

	// Cartesian product of gl_type=(UNSIGNED_BYTE, UNSIGNED_SHORT, HALF_FLOAT, FLOAT) x gl_format=(RED, RG, RGB, RGBA, BGR, BGRA)

	int components;
	switch (hdr.gl_format)
	{
	case GL_RED:
		components = 1;
		break;
	case GL_RG:
		components = 2;
		break;
	case GL_RGB:
		components = 3;
		break;
	case GL_RGBA:
		components = 4;
		break;
	case GL_BGR:
		components = 3;
		break;
	case GL_BGRA:
		components = 4;
		break;
	case GL_LUMINANCE:
		components = 1;
		break;
	case GL_LUMINANCE_ALPHA:
		components = 2;
		break;
	default:
		printf("KTX file %s has unsupported GL type\n", filename);
		fclose(f);
		*result = -5;
		return NULL;
	};


	// Although these are set up later, we include a default initializer to remove warnings

	int bytes_per_component = 1;	// bytes per component in the KTX file.
	int bitness = 8;			// internal precision we will use in the codec.
	scanline_copy_method cm = R8_TO_RGBA8;

	switch (hdr.gl_type)
	{
	case GL_UNSIGNED_BYTE:
		{
			bitness = 8;
			bytes_per_component = 1;
			switch (hdr.gl_format)
			{
			case GL_RED:
				cm = R8_TO_RGBA8;
				break;
			case GL_RG:
				cm = RG8_TO_RGBA8;
				break;
			case GL_RGB:
				cm = RGB8_TO_RGBA8;
				break;
			case GL_RGBA:
				cm = RGBA8_TO_RGBA8;
				break;
			case GL_BGR:
				cm = BGR8_TO_RGBA8;
				break;
			case GL_BGRA:
				cm = BGRA8_TO_RGBA8;
				break;
			case GL_LUMINANCE:
				cm = L8_TO_RGBA8;
				break;
			case GL_LUMINANCE_ALPHA:
				cm = LA8_TO_RGBA8;
				break;
			}
			break;
		}
	case GL_UNSIGNED_SHORT:
		{
			bitness = 16;
			bytes_per_component = 2;
			switch (hdr.gl_format)
			{
			case GL_RED:
				cm = R16_TO_RGBA16F;
				break;
			case GL_RG:
				cm = RG16_TO_RGBA16F;
				break;
			case GL_RGB:
				cm = RGB16_TO_RGBA16F;
				break;
			case GL_RGBA:
				cm = RGBA16_TO_RGBA16F;
				break;
			case GL_BGR:
				cm = BGR16_TO_RGBA16F;
				break;
			case GL_BGRA:
				cm = BGRA16_TO_RGBA16F;
				break;
			case GL_LUMINANCE:
				cm = L16_TO_RGBA16F;
				break;
			case GL_LUMINANCE_ALPHA:
				cm = LA16_TO_RGBA16F;
				break;
			}
			break;
		}
	case GL_HALF_FLOAT:
		{
			bitness = 16;
			bytes_per_component = 2;
			switch (hdr.gl_format)
			{
			case GL_RED:
				cm = R16F_TO_RGBA16F;
				break;
			case GL_RG:
				cm = RG16F_TO_RGBA16F;
				break;
			case GL_RGB:
				cm = RGB16F_TO_RGBA16F;
				break;
			case GL_RGBA:
				cm = RGBA16F_TO_RGBA16F;
				break;
			case GL_BGR:
				cm = BGR16F_TO_RGBA16F;
				break;
			case GL_BGRA:
				cm = BGRA16F_TO_RGBA16F;
				break;
			case GL_LUMINANCE:
				cm = L16F_TO_RGBA16F;
				break;
			case GL_LUMINANCE_ALPHA:
				cm = LA16F_TO_RGBA16F;
				break;
			}
			break;
		}
	case GL_FLOAT:
		{
			bitness = 16;
			bytes_per_component = 4;
			switch (hdr.gl_format)
			{
			case GL_RED:
				cm = R32F_TO_RGBA16F;
				break;
			case GL_RG:
				cm = RG32F_TO_RGBA16F;
				break;
			case GL_RGB:
				cm = RGB32F_TO_RGBA16F;
				break;
			case GL_RGBA:
				cm = RGBA32F_TO_RGBA16F;
				break;
			case GL_BGR:
				cm = BGR32F_TO_RGBA16F;
				break;
			case GL_BGRA:
				cm = BGRA32F_TO_RGBA16F;
				break;
			case GL_LUMINANCE:
				cm = L32F_TO_RGBA16F;
				break;
			case GL_LUMINANCE_ALPHA:
				cm = LA32F_TO_RGBA16F;
				break;
			}
			break;
		}
	default:
		printf("KTX file %s has unsupported GL format\n", filename);
		fclose(f);
		*result = -5;
		return NULL;
	}

	if (hdr.number_of_mipmap_levels > 1)
		printf("warning: KTX file %s has %d mipmap levels; only the first one will be encoded.\n", filename, hdr.number_of_mipmap_levels);

	if (hdr.number_of_array_elements > 1)
		printf("warning: KTX file %s contains a texture array with %d layers; only the first one will be encoded.\n", filename, hdr.number_of_array_elements);

	if (hdr.number_of_faces > 1)
		printf("warning: KTX file %s contains a cubemap with 6 faces; only the first one will be encoded.\n", filename);


	int xsize = hdr.pixel_width;
	int ysize = hdr.pixel_height;
	int zsize = hdr.pixel_depth;

	if (ysize == 0)
		ysize = 1;
	if (zsize == 0)
		zsize = 1;

	// ignore the key/value data
	fseek(f, hdr.bytes_of_key_value_data, SEEK_CUR);

	uint32_t specified_bytes_of_surface = 0;
	size_t sb_read = fread(&specified_bytes_of_surface, 1, 4, f);
	if (sb_read != 4)
	{
		printf("Failed to read header of KTX file %s\n", filename);
		fclose(f);
		*result = -2;
		return NULL;
	}


	if (switch_endianness)
		specified_bytes_of_surface = u32_byterev(specified_bytes_of_surface);

	// read the surface
	uint32_t xstride = bytes_per_component * components * xsize;
	uint32_t ystride = xstride * ysize;
	uint32_t computed_bytes_of_surface = zsize * ystride;
	if (computed_bytes_of_surface != specified_bytes_of_surface)
	{
		fclose(f);
		printf("%s: KTX file inconsistency: computed surface size is %d bytes, but specified size is %d bytes\n", filename, computed_bytes_of_surface, specified_bytes_of_surface);
		*result = -5;
		return NULL;
	}



	uint8_t *buf = (uint8_t *) malloc(specified_bytes_of_surface);
	size_t bytes_read = fread(buf, 1, specified_bytes_of_surface, f);
	fclose(f);
	if (bytes_read != specified_bytes_of_surface)
	{
		free(buf);
		printf("Failed to read file %s\n", filename);
		*result = -6;
		return NULL;
	}

	// perform an endianness swap on the surface if needed.
	if (switch_endianness)
	{
		if (hdr.gl_type_size == 2)
			switch_endianness2(buf, specified_bytes_of_surface);
		if (hdr.gl_type_size == 4)
			switch_endianness4(buf, specified_bytes_of_surface);
	}



	// then transfer data from the surface to our own image-data-structure.
	astc_codec_image *astc_img = allocate_image(bitness, xsize, ysize, zsize, padding);

	for (z = 0; z < zsize; z++)
	{
		int zdst = (zsize == 1) ? z : z + padding;
		for (y = 0; y < ysize; y++)
		{
			int ydst = y + padding;
			void *dst;
			if (bitness == 16)
				dst = (void *)(astc_img->imagedata16[zdst][ydst] + 4 * padding);
			else
				dst = (void *)(astc_img->imagedata8[zdst][ydst] + 4 * padding);

			uint8_t *src = buf + (z * ystride) + (y * xstride);
			copy_scanline(dst, src, xsize, cm);
		}
	}

	free(buf);
	fill_image_padding_area(astc_img);
	*result = components + (bitness == 16 ? 0x80 : 0);
	return astc_img;
}




int store_ktx_uncompressed_image(const astc_codec_image * img, const char *ktx_filename, int bitness)
{
	int x, y, z;
	int i, j;

	int xsize = img->xsize;
	int ysize = img->ysize;
	int zsize = img->zsize;

	int image_channels = determine_image_channels(img);

	ktx_header hdr;

	int gl_format_of_channels[4] = { GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };

	memcpy(hdr.magic, ktx_magic, 12);
	hdr.endianness = 0x04030201;
	hdr.gl_type = (bitness == 16) ? GL_HALF_FLOAT : GL_UNSIGNED_BYTE;
	hdr.gl_type_size = bitness / 8;
	hdr.gl_format = gl_format_of_channels[image_channels - 1];
	hdr.gl_internal_format = gl_format_of_channels[image_channels - 1];
	hdr.gl_base_internal_format = gl_format_of_channels[image_channels - 1];
	hdr.pixel_width = xsize;
	hdr.pixel_height = ysize;
	hdr.pixel_depth = (zsize == 1) ? 0 : zsize;
	hdr.number_of_array_elements = 0;
	hdr.number_of_faces = 1;
	hdr.number_of_mipmap_levels = 1;
	hdr.bytes_of_key_value_data = 0;


	// collect image data to write

	uint8_t ***row_pointers8 = NULL;
	uint16_t ***row_pointers16 = NULL;
	if (bitness == 8)
	{
		row_pointers8 = new uint8_t **[zsize];
		row_pointers8[0] = new uint8_t *[ysize * zsize];
		row_pointers8[0][0] = new uint8_t[xsize * ysize * zsize * image_channels + 3];

		for (i = 1; i < zsize; i++)
		{
			row_pointers8[i] = row_pointers8[0] + ysize * i;
			row_pointers8[i][0] = row_pointers8[0][0] + ysize * xsize * image_channels * i;
		}
		for (i = 0; i < zsize; i++)
			for (j = 1; j < ysize; j++)
				row_pointers8[i][j] = row_pointers8[i][0] + xsize * image_channels * j;

		for (z = 0; z < zsize; z++)
		{
			for (y = 0; y < ysize; y++)
			{
				switch (image_channels)
				{
				case 1:		// single-component, treated as Luminance
					for (x = 0; x < xsize; x++)
					{
						row_pointers8[z][y][x] = img->imagedata8[z][y][4 * x];
					}
					break;
				case 2:		// two-component, treated as Luminance-Alpha
					for (x = 0; x < xsize; x++)
					{
						row_pointers8[z][y][2 * x] = img->imagedata8[z][y][4 * x];
						row_pointers8[z][y][2 * x + 1] = img->imagedata8[z][y][4 * x + 3];
					}
					break;
				case 3:		// three-component, treated as RGB
					for (x = 0; x < xsize; x++)
					{
						row_pointers8[z][y][3 * x] = img->imagedata8[z][y][4 * x];
						row_pointers8[z][y][3 * x + 1] = img->imagedata8[z][y][4 * x + 1];
						row_pointers8[z][y][3 * x + 2] = img->imagedata8[z][y][4 * x + 2];
					}
					break;
				case 4:		// four-component, treated as RGBA
					for (x = 0; x < xsize; x++)
					{
						row_pointers8[z][y][4 * x] = img->imagedata8[z][y][4 * x];
						row_pointers8[z][y][4 * x + 1] = img->imagedata8[z][y][4 * x + 1];
						row_pointers8[z][y][4 * x + 2] = img->imagedata8[z][y][4 * x + 2];
						row_pointers8[z][y][4 * x + 3] = img->imagedata8[z][y][4 * x + 3];
					}
					break;
				}
			}
		}
	}
	else						// if bitness == 16
	{
		row_pointers16 = new uint16_t **[zsize];
		row_pointers16[0] = new uint16_t *[ysize * zsize];
		row_pointers16[0][0] = new uint16_t[xsize * ysize * zsize * image_channels + 1];

		for (i = 1; i < zsize; i++)
		{
			row_pointers16[i] = row_pointers16[0] + ysize * i;
			row_pointers16[i][0] = row_pointers16[0][0] + ysize * xsize * image_channels * i;
		}
		for (i = 0; i < zsize; i++)
			for (j = 1; j < ysize; j++)
				row_pointers16[i][j] = row_pointers16[i][0] + xsize * image_channels * j;

		for (z = 0; z < zsize; z++)
		{
			for (y = 0; y < ysize; y++)
			{
				switch (image_channels)
				{
				case 1:		// single-component, treated as Luminance
					for (x = 0; x < xsize; x++)
					{
						row_pointers16[z][y][x] = img->imagedata16[z][y][4 * x];
					}
					break;
				case 2:		// two-component, treated as Luminance-Alpha
					for (x = 0; x < xsize; x++)
					{
						row_pointers16[z][y][2 * x] = img->imagedata16[z][y][4 * x];
						row_pointers16[z][y][2 * x + 1] = img->imagedata16[z][y][4 * x + 3];
					}
					break;
				case 3:		// three-component, treated as RGB
					for (x = 0; x < xsize; x++)
					{
						row_pointers16[z][y][3 * x] = img->imagedata16[z][y][4 * x];
						row_pointers16[z][y][3 * x + 1] = img->imagedata16[z][y][4 * x + 1];
						row_pointers16[z][y][3 * x + 2] = img->imagedata16[z][y][4 * x + 2];
					}
					break;
				case 4:		// four-component, treated as RGBA
					for (x = 0; x < xsize; x++)
					{
						row_pointers16[z][y][4 * x] = img->imagedata16[z][y][4 * x];
						row_pointers16[z][y][4 * x + 1] = img->imagedata16[z][y][4 * x + 1];
						row_pointers16[z][y][4 * x + 2] = img->imagedata16[z][y][4 * x + 2];
						row_pointers16[z][y][4 * x + 3] = img->imagedata16[z][y][4 * x + 3];
					}
					break;
				}
			}
		}
	}

	int retval = image_channels + (bitness == 16 ? 0x80 : 0);
	uint32_t image_bytes = xsize * ysize * zsize * image_channels * (bitness / 8);
	uint32_t image_write_bytes = (image_bytes + 3) & ~3;

	FILE *wf = fopen(ktx_filename, "wb");
	if (wf)
	{
		void *dataptr = (bitness == 16) ? (void *)(row_pointers16[0][0]) : (void *)(row_pointers8[0][0]);

		size_t expected_bytes_written = sizeof(ktx_header) + image_write_bytes + 4;
		size_t hdr_bytes_written = fwrite(&hdr, 1, sizeof(ktx_header), wf);
		size_t bytecount_bytes_written = fwrite(&image_bytes, 1, 4, wf);
		size_t data_bytes_written = fwrite(dataptr, 1, image_write_bytes, wf);
		fclose(wf);
		if (hdr_bytes_written + bytecount_bytes_written + data_bytes_written != expected_bytes_written)
			retval = -1;

	}
	else
	{
		retval = -1;
	}

	if (row_pointers8)
	{
		delete[]row_pointers8[0][0];
		delete[]row_pointers8[0];
		delete[]row_pointers8;
	}
	if (row_pointers16)
	{
		delete[]row_pointers16[0][0];
		delete[]row_pointers16[0];
		delete[]row_pointers16;
	}

	return retval;
}




/*
	Loader for DDS files.

	Note that after the header, data are densely packed with no padding;
	in the case of multiple surfaces, they appear one after another in
	the file, again with no padding.

	This code is NOT endian-neutral.
*/

struct dds_pixelformat
{
	uint32_t size;				// structure size, set to 32.
	/*
	   flags bits are a combination of the following: 0x1 : Texture contains alpha data 0x2 : ---- (older files: texture contains alpha data, for Alpha-only texture) 0x4 : The fourcc field is valid,
	   indicating a compressed or DX10 texture format 0x40 : texture contains uncompressed RGB data 0x200 : ---- (YUV in older files) 0x20000 : Texture contains Luminance data (can be combined with
	   0x1 for Lum-Alpha) */
	uint32_t flags;
	uint32_t fourcc;			// "DX10" to indicate a DX10 format, "DXTn" for the DXT formats
	uint32_t rgbbitcount;		// number of bits per texel; up to 32 for non-DX10 formats.
	uint32_t rbitmask;			// bitmap indicating position of red/luminance color component
	uint32_t gbitmask;			// bitmap indicating position of green color component
	uint32_t bbitmask;			// bitmap indicating position of blue color component
	uint32_t abitmask;			// bitmap indicating position of alpha color component
};


struct dds_header
{
	uint32_t size;				// header size; must be exactly 124.
	/*
	   flag field is an OR or the following bits, that indicate fields containing valid data:
		1: caps/caps2/caps3/caps4 (set in all DDS files, ignore on read)
		2: height (set in all DDS files, ignore on read)
		4: width (set in all DDS files, ignore on read)
		8: pitch (for uncompressed texture)
		0x1000: the pixel format field (set in all DDS files, ignore on read)
		0x20000: mipmap count (for mipmapped textures with >1 level)
		0x80000: pitch (for compressed texture)
		0x800000: depth (for 3d textures)
	*/
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitch_or_linear_size;	// scanline pitch for uncompressed; total size in bytes for compressed
	uint32_t depth;
	uint32_t mipmapcount;
	// unused, set to 0
	uint32_t reserved1[11];
	dds_pixelformat ddspf;
	/*
	   caps field is an OR of the following values:
		8 : should be set for a file that contains more than 1 surface (ignore on read)
		0x400000 : should be set for a mipmapped texture
		0x1000 : should be set if the surface is a texture at all (all DDS files, ignore on read)
	*/
	uint32_t caps;
	/*
	   caps2 field is an OR of the following values:
		0x200 : texture is cubemap
		0x400 : +X face of cubemap is present
		0x800 : -X face of cubemap is present
		0x1000 : +Y face of cubemap is present
		0x2000 : -Y face of cubemap is present
		0x4000 : +Z face of cubemap is present
		0x8000 : -Z face of cubemap is present
		0x200000 : texture is a 3d texture.
	*/
	uint32_t caps2;
	// unused, set to 0
	uint32_t caps3;
	// unused, set to 0
	uint32_t caps4;
	// unused, set to 0
	uint32_t reserved2;
};


struct dds_header_dx10
{
	uint32_t dxgi_format;
	uint32_t resource_dimension;	// 2=1d-texture, 3=2d-texture or cubemap, 4=3d-texture
	uint32_t misc_flag;			// 4 if cubemap, else 0
	uint32_t array_size;		// size of array in case of a texture array; set to 1 for a non-array
	uint32_t reserved;			// set to 0.
};


#define DDS_MAGIC 0x20534444
#define DX10_MAGIC 0x30315844


astc_codec_image *load_dds_uncompressed_image(const char *filename, int padding, int *result)
{
	int i;
	int y, z;

	FILE *f = fopen(filename, "rb");
	if (!f)
	{
		printf("Failed to open file %s\n", filename);
		*result = -1;
		return NULL;
	}

	uint8_t magic[4];

	dds_header hdr;
	size_t magic_bytes_read = fread(magic, 1, 4, f);
	size_t header_bytes_read = fread(&hdr, 1, sizeof(hdr), f);
	if (magic_bytes_read != 4 || header_bytes_read != sizeof(hdr))
	{
		printf("Failed to read header of DDS file %s\n", filename);
		fclose(f);
		*result = -2;
		return NULL;
	}

	uint32_t magicx = magic[0] | (magic[1] << 8) | (magic[2] << 16) | (magic[3] << 24);

	if (magicx != DDS_MAGIC || hdr.size != 124)
	{
		printf("File %s does not have a valid DDS header\n", filename);
		fclose(f);
		*result = -3;
		return NULL;
	}

	int use_dx10_header = 0;
	if (hdr.ddspf.flags & 4)
	{
		if (hdr.ddspf.fourcc == DX10_MAGIC)
		{
			use_dx10_header = 1;
		}
		else
		{
			printf("DDS file %s is compressed, not supported\n", filename);
			fclose(f);
			*result = -4;
			return NULL;
		}
	}

	dds_header_dx10 dx10_header;
	if (use_dx10_header)
	{
		size_t dx10_header_bytes_read = fread(&dx10_header, 1, sizeof(dx10_header), f);
		if (dx10_header_bytes_read != sizeof(dx10_header))
		{
			printf("Failed to read header of DDS file %s\n", filename);
			fclose(f);
			*result = -2;
			return NULL;
		}
	}


	int xsize = hdr.width;
	int ysize = hdr.height;
	int zsize = (hdr.flags & 0x800000) ? hdr.depth : 1;


	int bitness;				// the bitcount that we will use internally in the codec
	int bytes_per_component;	// the bytes per component in the DDS file itself
	int components;
	int copy_method;

	// figure out the format actually used in the DDS file.
	if (use_dx10_header)
	{

		// DX10 header present; use the DXGI format.

		#define DXGI_FORMAT_R32G32B32A32_FLOAT   2
		#define DXGI_FORMAT_R32G32B32_FLOAT      6
		#define DXGI_FORMAT_R16G16B16A16_FLOAT  10
		#define DXGI_FORMAT_R16G16B16A16_UNORM  11
		#define DXGI_FORMAT_R32G32_FLOAT        16
		#define DXGI_FORMAT_R8G8B8A8_UNORM      28
		#define DXGI_FORMAT_R16G16_FLOAT    34
		#define DXGI_FORMAT_R16G16_UNORM    35
		#define DXGI_FORMAT_R32_FLOAT       41
		#define DXGI_FORMAT_R8G8_UNORM      49
		#define DXGI_FORMAT_R16_FLOAT       54
		#define DXGI_FORMAT_R16_UNORM       56
		#define DXGI_FORMAT_R8_UNORM        61
		#define DXGI_FORMAT_B8G8R8A8_UNORM  86
		#define DXGI_FORMAT_B8G8R8X8_UNORM  87

		struct dxgi_params
		{
			int bitness;
			int bytes_per_component;
			int components;
			int copy_method;
			uint32_t dxgi_format_number;
		};

		static const dxgi_params format_params[] = {
			{16, 4, 4, RGBA32F_TO_RGBA16F, DXGI_FORMAT_R32G32B32A32_FLOAT},
			{16, 4, 3, RGB32F_TO_RGBA16F, DXGI_FORMAT_R32G32B32_FLOAT},
			{16, 2, 4, RGBA16F_TO_RGBA16F, DXGI_FORMAT_R16G16B16A16_FLOAT},
			{16, 2, 4, RGBA16_TO_RGBA16F, DXGI_FORMAT_R16G16B16A16_UNORM},
			{16, 4, 2, RG32F_TO_RGBA16F, DXGI_FORMAT_R32G32_FLOAT},
			{8, 1, 4, RGBA8_TO_RGBA8, DXGI_FORMAT_R8G8B8A8_UNORM},
			{16, 2, 2, RG16F_TO_RGBA16F, DXGI_FORMAT_R16G16_FLOAT},
			{16, 2, 2, RG16_TO_RGBA16F, DXGI_FORMAT_R16G16_UNORM},
			{16, 4, 1, R32F_TO_RGBA16F, DXGI_FORMAT_R32_FLOAT},
			{8, 1, 2, RG8_TO_RGBA8, DXGI_FORMAT_R8G8_UNORM},
			{16, 2, 1, R16F_TO_RGBA16F, DXGI_FORMAT_R16_FLOAT},
			{16, 2, 1, R16_TO_RGBA16F, DXGI_FORMAT_R16_UNORM},
			{8, 1, 1, R8_TO_RGBA8, DXGI_FORMAT_R8_UNORM},
			{8, 1, 4, BGRA8_TO_RGBA8, DXGI_FORMAT_B8G8R8A8_UNORM},
			{8, 1, 4, BGRX8_TO_RGBA8, DXGI_FORMAT_B8G8R8X8_UNORM},
		};


		int dxgi_modes_supported = sizeof(format_params) / sizeof(format_params[0]);
		int did_select_format = 0;
		for (i = 0; i < dxgi_modes_supported; i++)
		{
			if (dx10_header.dxgi_format == format_params[i].dxgi_format_number)
			{
				bitness = format_params[i].bitness;
				bytes_per_component = format_params[i].bytes_per_component;
				components = format_params[i].components;
				copy_method = format_params[i].copy_method;
				did_select_format = 1;
				break;
			}
		}

		if (did_select_format == 0)
		{
			printf("DDS file %s: DXGI format not supported by codec\n", filename);
			fclose(f);
			*result = -4;
			return NULL;
		}
	}

	else
	{
		// No DX10 header present. Then try to match the bitcount and bitmask against
		// a set of prepared patterns.
		uint32_t flags = hdr.ddspf.flags;
		uint32_t bitcount = hdr.ddspf.rgbbitcount;
		uint32_t rmask = hdr.ddspf.rbitmask;
		uint32_t gmask = hdr.ddspf.gbitmask;
		uint32_t bmask = hdr.ddspf.bbitmask;
		uint32_t amask = hdr.ddspf.abitmask;


		// RGBA-unorm8
		if ((flags & 0x41) == 0x41 && bitcount == 32 && rmask == 0xFF && gmask == 0xFF00 && bmask == 0xFF0000 && amask == 0xFF000000)
		{
			bytes_per_component = 1;
			components = 4;
			copy_method = RGBA8_TO_RGBA8;
		}
		// BGRA-unorm8
		else if ((flags & 0x41) == 0x41 && bitcount == 32 && rmask == 0xFF0000 && gmask == 0xFF00 && bmask == 0xFF && amask == 0xFF000000)
		{
			bytes_per_component = 1;
			components = 4;
			copy_method = BGRA8_TO_RGBA8;
		}
		// RGBX-unorm8
		else if ((flags & 0x40) && bitcount == 32 && rmask == 0xFF && gmask == 0xFF00 && bmask == 0xFF0000)
		{
			bytes_per_component = 1;
			components = 4;
			copy_method = RGBX8_TO_RGBA8;
		}
		// BGRX-unorm8
		else if ((flags & 0x40) && bitcount == 32 && rmask == 0xFF0000 && gmask == 0xFF00 && bmask == 0xFF)
		{
			bytes_per_component = 1;
			components = 4;
			copy_method = BGRX8_TO_RGBA8;
		}
		// RGB-unorm8
		else if ((flags & 0x40) && bitcount == 24 && rmask == 0xFF && gmask == 0xFF00 && bmask == 0xFF0000)
		{
			bytes_per_component = 1;
			components = 3;
			copy_method = RGB8_TO_RGBA8;
		}
		// BGR-unorm8
		else if ((flags & 0x40) && bitcount == 24 && rmask == 0xFF0000 && gmask == 0xFF00 && bmask == 0xFF)
		{
			bytes_per_component = 1;
			components = 3;
			copy_method = BGR8_TO_RGBA8;
		}
		// RG-unorm16
		else if ((flags & 0x40) && bitcount == 16 && rmask == 0xFFFF && gmask == 0xFFFF0000)
		{
			bytes_per_component = 2;
			components = 2;
			copy_method = RG16_TO_RGBA16F;
		}
		// A8L8
		else if ((flags & 0x20001) == 0x20001 && bitcount == 16 && rmask == 0xFF && amask == 0xFF00)
		{
			bytes_per_component = 1;
			components = 2;
			copy_method = LA8_TO_RGBA8;
		}
		// L8
		else if ((flags & 0x20000) && bitcount == 8 && rmask == 0xFF)
		{
			bytes_per_component = 1;
			components = 1;
			copy_method = L8_TO_RGBA8;
		}
		// L16
		else if ((flags & 0x20000) && bitcount == 16 && rmask == 0xFFFF)
		{
			bytes_per_component = 2;
			components = 1;
			copy_method = L16_TO_RGBA16F;
		}
		else
		{
			printf("DDS file %s: Non-DXGI format not supported by codec\n", filename);
			fclose(f);
			*result = -4;
			return NULL;
		}

		bitness = bytes_per_component * 8;
	}


	// then, load the actual file.
	uint32_t xstride = bytes_per_component * components * xsize;
	uint32_t ystride = xstride * ysize;
	uint32_t bytes_of_surface = zsize * ystride;

	uint8_t *buf = (uint8_t *) malloc(bytes_of_surface);
	size_t bytes_read = fread(buf, 1, bytes_of_surface, f);
	fclose(f);
	if (bytes_read != bytes_of_surface)
	{
		free(buf);
		printf("Failed to read file %s\n", filename);
		*result = -6;
		return NULL;
	}

	// then transfer data from the surface to our own image-data-structure.
	astc_codec_image *astc_img = allocate_image(bitness, xsize, ysize, zsize, padding);

	for (z = 0; z < zsize; z++)
	{
		int zdst = zsize == 1 ? z : z + padding;
		for (y = 0; y < ysize; y++)
		{
			int ydst = y + padding;
			void *dst;
			if (bitness == 16)
				dst = (void *)(astc_img->imagedata16[zdst][ydst] + 4 * padding);
			else
				dst = (void *)(astc_img->imagedata8[zdst][ydst] + 4 * padding);

			uint8_t *src = buf + (z * ystride) + (y * xstride);
			copy_scanline(dst, src, xsize, copy_method);
		}
	}

	free(buf);
	fill_image_padding_area(astc_img);
	*result = components + (bitness == 16 ? 0x80 : 0);
	return astc_img;
}



int store_dds_uncompressed_image(const astc_codec_image * img, const char *dds_filename, int bitness)
{
	int i, j;
	int x, y, z;

	int xsize = img->xsize;
	int ysize = img->ysize;
	int zsize = img->zsize;

	int image_channels = (bitness == 16) ? 4 : determine_image_channels(img);


	// DDS-pixel-format structures to use when storing LDR image with 1,2,3 or 4 components.
	static const dds_pixelformat format_of_image_channels[4] =
	{
		{32, 0x20000, 0, 8, 0xFF, 0, 0, 0},	// luminance
		{32, 0x20001, 0, 16, 0xFF, 0, 0, 0xFF00},	// L8A8
		{32, 0x40, 0, 24, 0xFF, 0xFF00, 0xFF0000, 0},	// RGB8
		{32, 0x41, 0, 32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000}	// RGBA8
	};

	// DDS-pixel-format structures to use when storing HDR image.
	static const dds_pixelformat dxt10_diverter =
	{
		32, 4, DX10_MAGIC, 0, 0, 0, 0, 0
	};


	// header handling. We will write:
	// * DDS magic value
	// * DDS header
	// * DDS DX10 header, if the file is floating-point
	// * pixel data.

	// main header data
	dds_header hdr;
	hdr.size = 124;
	hdr.flags = 0x100F | (zsize > 1 ? 0x800000 : 0);
	hdr.height = ysize;
	hdr.width = xsize;
	hdr.pitch_or_linear_size = image_channels * (bitness / 8) * xsize;
	hdr.depth = zsize;
	hdr.mipmapcount = 1;
	for (i = 0; i < 11; i++)
		hdr.reserved1[i] = 0;
	hdr.caps = 0x1000;
	hdr.caps2 = (zsize > 1) ? 0x200000 : 0;
	hdr.caps3 = 0;
	hdr.caps4 = 0;

	// pixel-format data
	if (bitness == 8)
		hdr.ddspf = format_of_image_channels[image_channels - 1];
	else
		hdr.ddspf = dxt10_diverter;

	// DX10 data
	dds_header_dx10 dx10;
	dx10.dxgi_format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	dx10.resource_dimension = (zsize > 1) ? 4 : 3;
	dx10.misc_flag = 0;
	dx10.array_size = 1;
	dx10.reserved = 0;


	// collect image data to write

	uint8_t ***row_pointers8 = NULL;
	uint16_t ***row_pointers16 = NULL;
	if (bitness == 8)
	{
		row_pointers8 = new uint8_t **[zsize];
		row_pointers8[0] = new uint8_t *[ysize * zsize];
		row_pointers8[0][0] = new uint8_t[xsize * ysize * zsize * image_channels];

		for (i = 1; i < zsize; i++)
		{
			row_pointers8[i] = row_pointers8[0] + ysize * i;
			row_pointers8[i][0] = row_pointers8[0][0] + ysize * xsize * image_channels * i;
		}
		for (i = 0; i < zsize; i++)
			for (j = 1; j < ysize; j++)
				row_pointers8[i][j] = row_pointers8[i][0] + xsize * image_channels * j;

		for (z = 0; z < zsize; z++)
		{
			for (y = 0; y < ysize; y++)
			{
				switch (image_channels)
				{
				case 1:		// single-component, treated as Luminance
					for (x = 0; x < xsize; x++)
					{
						row_pointers8[z][y][x] = img->imagedata8[z][y][4 * x];
					}
					break;
				case 2:		// two-component, treated as Luminance-Alpha
					for (x = 0; x < xsize; x++)
					{
						row_pointers8[z][y][2 * x] = img->imagedata8[z][y][4 * x];
						row_pointers8[z][y][2 * x + 1] = img->imagedata8[z][y][4 * x + 3];
					}
					break;
				case 3:		// three-component, treated as RGB
					for (x = 0; x < xsize; x++)
					{
						row_pointers8[z][y][3 * x] = img->imagedata8[z][y][4 * x];
						row_pointers8[z][y][3 * x + 1] = img->imagedata8[z][y][4 * x + 1];
						row_pointers8[z][y][3 * x + 2] = img->imagedata8[z][y][4 * x + 2];
					}
					break;
				case 4:		// four-component, treated as RGBA
					for (x = 0; x < xsize; x++)
					{
						row_pointers8[z][y][4 * x] = img->imagedata8[z][y][4 * x];
						row_pointers8[z][y][4 * x + 1] = img->imagedata8[z][y][4 * x + 1];
						row_pointers8[z][y][4 * x + 2] = img->imagedata8[z][y][4 * x + 2];
						row_pointers8[z][y][4 * x + 3] = img->imagedata8[z][y][4 * x + 3];
					}
					break;
				}
			}
		}
	}
	else						// if bitness == 16
	{
		row_pointers16 = new uint16_t **[zsize];
		row_pointers16[0] = new uint16_t *[ysize * zsize];
		row_pointers16[0][0] = new uint16_t[xsize * ysize * zsize * image_channels];

		for (i = 1; i < zsize; i++)
		{
			row_pointers16[i] = row_pointers16[0] + ysize * i;
			row_pointers16[i][0] = row_pointers16[0][0] + ysize * xsize * image_channels * i;
		}
		for (i = 0; i < zsize; i++)
			for (j = 1; j < ysize; j++)
				row_pointers16[i][j] = row_pointers16[i][0] + xsize * image_channels * j;

		for (z = 0; z < zsize; z++)
		{
			for (y = 0; y < ysize; y++)
			{
				switch (image_channels)
				{
				case 1:		// single-component, treated as Luminance
					for (x = 0; x < xsize; x++)
					{
						row_pointers16[z][y][x] = img->imagedata16[z][y][4 * x];
					}
					break;
				case 2:		// two-component, treated as Luminance-Alpha
					for (x = 0; x < xsize; x++)
					{
						row_pointers16[z][y][2 * x] = img->imagedata16[z][y][4 * x];
						row_pointers16[z][y][2 * x + 1] = img->imagedata16[z][y][4 * x + 3];
					}
					break;
				case 3:		// three-component, treated as RGB
					for (x = 0; x < xsize; x++)
					{
						row_pointers16[z][y][3 * x] = img->imagedata16[z][y][4 * x];
						row_pointers16[z][y][3 * x + 1] = img->imagedata16[z][y][4 * x + 1];
						row_pointers16[z][y][3 * x + 2] = img->imagedata16[z][y][4 * x + 2];
					}
					break;
				case 4:		// four-component, treated as RGBA
					for (x = 0; x < xsize; x++)
					{
						row_pointers16[z][y][4 * x] = img->imagedata16[z][y][4 * x];
						row_pointers16[z][y][4 * x + 1] = img->imagedata16[z][y][4 * x + 1];
						row_pointers16[z][y][4 * x + 2] = img->imagedata16[z][y][4 * x + 2];
						row_pointers16[z][y][4 * x + 3] = img->imagedata16[z][y][4 * x + 3];
					}
					break;
				}
			}
		}
	}

	int retval = image_channels;
	uint32_t image_bytes = xsize * ysize * zsize * image_channels * (bitness / 8);

	uint32_t dds_magic = DDS_MAGIC;

	FILE *wf = fopen(dds_filename, "wb");
	if (wf)
	{
		void *dataptr = (bitness == 16) ? (void *)(row_pointers16[0][0]) : (void *)(row_pointers8[0][0]);

		size_t expected_bytes_written = 4 + sizeof(dds_header) + (bitness > 8 ? sizeof(dds_header_dx10) : 0) + image_bytes;

		size_t magic_bytes_written = fwrite(&dds_magic, 1, 4, wf);
		size_t hdr_bytes_written = fwrite(&hdr, 1, sizeof(dds_header), wf);

		size_t dx10_bytes_written;
		if (bitness > 8)
			dx10_bytes_written = fwrite(&dx10, 1, sizeof(dx10), wf);
		else
			dx10_bytes_written = 0;

		size_t data_bytes_written = fwrite(dataptr, 1, image_bytes, wf);

		fclose(wf);
		if (magic_bytes_written + hdr_bytes_written + dx10_bytes_written + data_bytes_written != expected_bytes_written)
			retval = -1;
	}
	else
	{
		retval = -1;
	}

	if (row_pointers8)
	{
		delete[]row_pointers8[0][0];
		delete[]row_pointers8[0];
		delete[]row_pointers8;
	}
	if (row_pointers16)
	{
		delete[]row_pointers16[0][0];
		delete[]row_pointers16[0];
		delete[]row_pointers16;
	}

	return retval;
}
