/*
 * Copyright 2015 The Etc2Comp Authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS (1)
#endif

#include "EtcConfig.h"
#include "EtcSourceImage.h"
#include "Etc.h"

#if USE_STB_IMAGE_LOAD
#include "stb_image.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "lodepng.h"

namespace Etc
{

	// ----------------------------------------------------------------------------------------------------
	//
	SourceImage::SourceImage(const char *a_pstrFilename, int a_iPixelX, int a_iPixelY)
	{
		m_pstrFilename = nullptr;
		m_pstrName = nullptr;
		m_pstrFileExtension = nullptr;
		m_uiWidth = 0;
		m_uiHeight = 0;
		m_pafrgbaPixels = nullptr;

		SetName(a_pstrFilename);

		Read(a_iPixelX, a_iPixelY);
	}

	// ----------------------------------------------------------------------------------------------------
	//
	SourceImage::SourceImage(ColorFloatRGBA *a_pafrgbaSource,
								unsigned int a_uiSourceWidth,
								unsigned int a_uiSourceHeight)
	{
		m_pstrFilename = nullptr;
		m_pstrName = nullptr;
		m_pstrFileExtension = nullptr;
		m_uiWidth = a_uiSourceWidth;
		m_uiHeight = a_uiSourceHeight;
		m_pafrgbaPixels = a_pafrgbaSource;

	}
	// ----------------------------------------------------------------------------------------------------
	//
	SourceImage::~SourceImage()
	{
		if (m_pstrFilename != nullptr)
		{
			delete[] m_pstrFilename;
			m_pstrFilename = nullptr;
		}

		if (m_pstrName != nullptr)
		{
			delete[] m_pstrName;
			m_pstrName = nullptr;
			m_pstrFileExtension = nullptr;
		}

		if (m_pafrgbaPixels != nullptr)
		{
			delete[] m_pafrgbaPixels;
			m_pafrgbaPixels = nullptr;
		}
		m_uiWidth = 0;
		m_uiHeight = 0;
	}
	// ----------------------------------------------------------------------------------------------------
	//
	void SourceImage::Read(int a_iPixelX, int a_iPixelY)
	{
		unsigned char* paucPixels = nullptr;

		int iWidth = 0;
		int iHeight = 0;
		bool bool16BitImage = false;
	
#if USE_STB_IMAGE_LOAD
		int iBitsPerPixel;
		//if stb_iamge is available, only use it to load files other than png
		char *fileExt = strrchr(m_pstrFilename, '.');

		if (strcmp(fileExt, ".png") != 0)
		{
			paucPixels = stbi_load(m_pstrFilename, &iWidth, &iHeight, &iBitsPerPixel, 4);

			if (paucPixels == nullptr)
			{
				printf("stb_image error %s\n", stbi_failure_reason());
				assert(0);
				exit(1);
			}
		}
#endif

		if (paucPixels == nullptr)
		{
			//we can load 8 or 16 bit pngs
			int iBitDepth = 16;
			int error = lodepng_decode_file(&paucPixels,
				(unsigned int*)&iWidth, (unsigned int*)&iHeight,
				m_pstrFilename,
				LCT_RGBA, iBitDepth);

			bool16BitImage = (iBitDepth == 16) ? true : false;
			if (error)
			{
				printf("lodePNG error %u: %s\n", error, lodepng_error_text(error));
				assert(0);
				exit(1);
			}
		}

		//the pixel cords for the top left corner of the block
		int iBlockX = 0;
		int iBlockY = 0;
		if (a_iPixelX > -1 && a_iPixelY > -1)
		{
			// in 1 block mode, we basically will have an img thats 4x4
			m_uiWidth = 4;
			m_uiHeight = 4;

			if(a_iPixelX > iWidth)
				a_iPixelX = iWidth;
			if (a_iPixelY > iHeight)
				a_iPixelY = iHeight;

			// remove the bottom 2 bits to get the block coordinates 
			iBlockX = (a_iPixelX & 0xFFFFFFFC);
			iBlockY = (a_iPixelY & 0xFFFFFFFC);
		}
		else
		{
			m_uiWidth = iWidth;
			m_uiHeight = iHeight;
		}

		m_pafrgbaPixels = new ColorFloatRGBA[m_uiWidth * m_uiHeight];
		assert(m_pafrgbaPixels);

		int iBytesPerPixel = bool16BitImage ? 8 : 4;
		unsigned char *pucPixel;	// = &paucPixels[(iBlockY * iWidth + iBlockX) * iBytesPerPixel];
		ColorFloatRGBA *pfrgbaPixel = m_pafrgbaPixels;

		// convert pixels from RGBA* to ColorFloatRGBA
		for (unsigned int uiV = iBlockY; uiV < (iBlockY+m_uiHeight); ++uiV)
		{
			// reset coordinate for each row
			pucPixel = &paucPixels[(uiV * iWidth + iBlockX) * iBytesPerPixel];

			// read each row
			for (unsigned int uiH = iBlockX; uiH < (iBlockX+m_uiWidth); ++uiH)
			{
				if (bool16BitImage)
				{
						unsigned short ushR = (pucPixel[0]<<8) + pucPixel[1];
						unsigned short ushG = (pucPixel[2]<<8) + pucPixel[3];
						unsigned short ushB = (pucPixel[4]<<8) + pucPixel[5];
						unsigned short ushA = (pucPixel[6]<<8) + pucPixel[7];

						*pfrgbaPixel++ = ColorFloatRGBA((float)ushR / 65535.0f,
														(float)ushG / 65535.0f,
														(float)ushB / 65535.0f,
														(float)ushA / 65535.0f);
				}
				else
				{
						*pfrgbaPixel++ = ColorFloatRGBA::ConvertFromRGBA8(pucPixel[0], pucPixel[1],
																			pucPixel[2], pucPixel[3]);
				}

				pucPixel += iBytesPerPixel;
			}
		}

#if USE_STB_IMAGE_LOAD
		stbi_image_free(paucPixels);
#else
		free(paucPixels);
#endif
	}

	// ----------------------------------------------------------------------------------------------------
	// sets m_pstrFilename, m_pstrName and m_pstrFileExtension
	//
	void SourceImage::SetName(const char *a_pstrFilename)
	{
		if (a_pstrFilename == nullptr)
		{
			return;
		}

		m_pstrFilename = new char[strlen(a_pstrFilename) + 1];
		strcpy(m_pstrFilename, a_pstrFilename);

		m_pstrName = new char[strlen(m_pstrFilename) + 1];

		// ignore directory path
		char *pcLastSlash = strrchr(m_pstrFilename, '/');
		char *pcLastBackSlash = strrchr(m_pstrFilename, '\\');
		if (pcLastSlash == nullptr && pcLastBackSlash == nullptr)
		{
			strcpy(m_pstrName, m_pstrFilename);
		}
		else if (pcLastSlash > pcLastBackSlash)
		{
			strcpy(m_pstrName, pcLastSlash + 1);
		}
		else
		{
			strcpy(m_pstrName, pcLastBackSlash + 1);
		}

		// find file extension and remove it from image name
		char *pcLastPeriod = strrchr(m_pstrName, '.');
		if (pcLastPeriod != nullptr)
		{
			m_pstrFileExtension = pcLastPeriod + 1;
			*strrchr(m_pstrName, '.') = 0;
		}

	}

	// ----------------------------------------------------------------------------------------------------
	//
	void SourceImage::NormalizeXYZ(void)
	{
		int iPixels = m_uiWidth * m_uiHeight;

		ColorFloatRGBA *pfrgbaPixel = m_pafrgbaPixels;
		for (int iPixel = 0; iPixel < iPixels; iPixel++)
		{
			float fX = 2.0f*pfrgbaPixel->fR - 1.0f;
			float fY = 2.0f*pfrgbaPixel->fG - 1.0f;
			float fZ = 2.0f*pfrgbaPixel->fB - 1.0f;

			float fLength2 = fX*fX + fY*fY + fZ*fZ;

			if (fLength2 == 0.0f)
			{
				pfrgbaPixel->fR = 1.0f;
				pfrgbaPixel->fG = 0.0f;
				pfrgbaPixel->fB = 0.0f;
			}
			else
			{
				float fLength = sqrtf(fLength2);

				float fNormalizedX = fX / fLength;
				float fNormalizedY = fY / fLength;
				float fNormalizedZ = fZ / fLength;

				pfrgbaPixel->fR = 0.5f * (fNormalizedX + 1.0f);
				pfrgbaPixel->fG = 0.5f * (fNormalizedY + 1.0f);
				pfrgbaPixel->fB = 0.5f * (fNormalizedZ + 1.0f);
			}

			pfrgbaPixel++;
		}

	}

	// ----------------------------------------------------------------------------------------------------
	//

}	// namespace Etc
