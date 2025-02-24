// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ASTC_Decoder.hpp"

#include "System/Math.hpp"

#ifdef SWIFTSHADER_ENABLE_ASTC
#	include "astc_codec_internals.h"
#endif

#include <memory>
#include <unordered_map>

namespace {

#ifdef SWIFTSHADER_ENABLE_ASTC
void write_imageblock(unsigned char *img,
                      // picture-block to initialize with image data. We assume that orig_data is valid
                      const imageblock *pb,
                      // output dimensions
                      int xsize, int ysize, int zsize,
                      // output format
                      int bytes, int destPitchB, int destSliceB, bool isUnsignedByte,
                      // block dimensions
                      int xdim, int ydim, int zdim,
                      // position to write the block to
                      int xpos, int ypos, int zpos)
{
	const float *fptr = pb->orig_data;
	const uint8_t *nptr = pb->nan_texel;

	for(int z = 0; z < zdim; z++)
	{
		for(int y = 0; y < ydim; y++)
		{
			for(int x = 0; x < xdim; x++)
			{
				int xi = xpos + x;
				int yi = ypos + y;
				int zi = zpos + z;

				if(xi >= 0 && yi >= 0 && zi >= 0 && xi < xsize && yi < ysize && zi < zsize)
				{
					unsigned char *pix = &img[zi * destSliceB + yi * destPitchB + xi * bytes];

					if(isUnsignedByte)
					{
						if(*nptr)
						{
							// NaN-pixel, but we can't display it. Display purple instead.
							pix[0] = 0xFF;
							pix[1] = 0x00;
							pix[2] = 0xFF;
							pix[3] = 0xFF;
						}
						else
						{
							pix[0] = static_cast<unsigned char>(sw::clamp(fptr[0], 0.0f, 1.0f) * 255.0f + 0.5f);
							pix[1] = static_cast<unsigned char>(sw::clamp(fptr[1], 0.0f, 1.0f) * 255.0f + 0.5f);
							pix[2] = static_cast<unsigned char>(sw::clamp(fptr[2], 0.0f, 1.0f) * 255.0f + 0.5f);
							pix[3] = static_cast<unsigned char>(sw::clamp(fptr[3], 0.0f, 1.0f) * 255.0f + 0.5f);
						}
					}
					else
					{
						if(*nptr)
						{
							unsigned int *pixu = reinterpret_cast<unsigned int *>(pix);
							pixu[0] = pixu[1] = pixu[2] = pixu[3] = 0x7FFFFFFF;  // QNaN
						}
						else
						{
							float *pixf = reinterpret_cast<float *>(pix);
							pixf[0] = fptr[0];
							pixf[1] = fptr[1];
							pixf[2] = fptr[2];
							pixf[3] = fptr[3];
						}
					}
				}
				fptr += 4;
				nptr++;
			}
		}
	}
}
#endif

}  // namespace

void ASTC_Decoder::Decode(const unsigned char *source, unsigned char *dest,
                          int destWidth, int destHeight, int destDepth,
                          int bytes, int destPitchB, int destSliceB,
                          int xBlockSize, int yBlockSize, int zBlockSize,
                          int xblocks, int yblocks, int zblocks, bool isUnsignedByte)
{
#ifdef SWIFTSHADER_ENABLE_ASTC
	build_quantization_mode_table();

	astc_decode_mode decode_mode = isUnsignedByte ? DECODE_LDR : DECODE_HDR;

	std::unique_ptr<block_size_descriptor> bsd(new block_size_descriptor);
	init_block_size_descriptor(xBlockSize, yBlockSize, zBlockSize, bsd.get());

	std::unique_ptr<imageblock> ib(new imageblock);
	std::unique_ptr<symbolic_compressed_block> scb(new symbolic_compressed_block);
	for(int z = 0; z < zblocks; z++)
	{
		for(int y = 0; y < yblocks; y++)
		{
			for(int x = 0; x < xblocks; x++, source += 16)
			{
				physical_to_symbolic(bsd.get(), *(physical_compressed_block *)source, scb.get());
				decompress_symbolic_block(decode_mode, bsd.get(), x * xBlockSize, y * yBlockSize, z * zBlockSize, scb.get(), ib.get());
				write_imageblock(dest, ib.get(), destWidth, destHeight, destDepth, bytes, destPitchB, destSliceB, isUnsignedByte,
				                 xBlockSize, yBlockSize, zBlockSize, x * xBlockSize, y * yBlockSize, z * zBlockSize);
			}
		}
	}

	term_block_size_descriptor(bsd.get());
#endif
}
