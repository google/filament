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

#include "EtcAnalysis.h"

#include "EtcTool.h"
#include "EtcComparison.h"
#include "Etc.h"
#include "EtcFile.h"
#include "EtcMath.h"
#include "EtcImage.h"
#include "EtcBlock4x4.h"

#include "lodepng.h"
#include <stdlib.h>
#include <cmath> //sqrt fn()

namespace Etc
{

	// ----------------------------------------------------------------------------------------------------
	//
	Analysis::Analysis(Image *a_pimage, const char *a_pstrOutputFolder)
	{

		m_pimage = a_pimage;
		m_pstrOutputFolder = a_pstrOutputFolder;
		m_uiComparisons = 0;

		CreateNewDir(a_pstrOutputFolder);

		// write log file
		{
			char strFilename[200];
			sprintf(strFilename, "%s%cAnalysis.txt", m_pstrOutputFolder, ETC_PATH_SLASH);

			FILE *pfileTxt = fopen(strFilename, "wt");
			if (pfileTxt == nullptr)
			{
				printf("Error: couldn't create analysis log file (%s)\n", strFilename);
				exit(1);
			}

			float fImageError = m_pimage->GetError();
			unsigned int uiImagePixels = m_pimage->GetSourceWidth() * m_pimage->GetSourceHeight();
			
			// output stats to both stdout and the analysis file
			int numOutputs = 1;
			FILE *apfile[2];
			apfile[0] = pfileTxt;
			
			if (m_pimage->m_bVerboseOutput)
			{
				apfile[1] = stdout;
				numOutputs++;
			}

			for (int i = 0; i < numOutputs; i++)
			{			
				
				if (a_pimage->GetFormat() == Image::Format::R11 || a_pimage->GetFormat() == Image::Format::SIGNED_R11)
				{
					fprintf(apfile[i], "PSNR(r) = %.4f\n", ConvertErrorToPSNR(fImageError, 1 * uiImagePixels));
				}
				else if (a_pimage->GetFormat() == Image::Format::RG11 || a_pimage->GetFormat() == Image::Format::SIGNED_RG11)
				{
					fprintf(apfile[i], "PSNR(rg) = %.4f\n", ConvertErrorToPSNR(fImageError, 2 * uiImagePixels));
				}
				else
				{
					int iComponents=3;
					if (a_pimage->GetErrorMetric() == ErrorMetric::REC709)
					{
						iComponents = (int)Block4x4Encoding::LUMA_WEIGHT + 2;
					}
					fprintf(apfile[i], "PSNR(rgb) = %.4f\n", ConvertErrorToPSNR(fImageError, iComponents * uiImagePixels));
					fprintf(apfile[i], "PSNR(rgba) = %.4f\n", ConvertErrorToPSNR(fImageError, (iComponents+1) * uiImagePixels));
				}
				
				fprintf(apfile[i], "EncodeTime = %.3f seconds\n", (float)m_pimage->GetEncodingTimeMs() / 1000.0f);
			}


			fclose(pfileTxt);
		}

		// scale == 1
		DrawImage(m_pimage, m_pstrOutputFolder, false);

		// scale == 2, with modes
		DrawImage(m_pimage, m_pstrOutputFolder, true);

	}

	// ----------------------------------------------------------------------------------------------------
	//
	void Analysis::Compare(const char *a_pstrFilename, int a_iPixelX, int a_iPixelY)
	{
		if (m_uiComparisons >= MAX_COMPARISONS)
		{
			printf("Error: too many comparisons\n");
			exit(1);
		}

		m_apcomparison[m_uiComparisons] = new Comparison(this, m_uiComparisons, a_pstrFilename, a_iPixelX, a_iPixelY);

		m_uiComparisons++;

	}

	// ----------------------------------------------------------------------------------------------------
	// draw 2x image
	// optionally display encoding modes
	void Analysis::DrawImage(Image *a_pimage, const char *a_pstrOutputFolder, bool a_boolDrawModes)
	{
		unsigned int uiPngWidth = a_pimage->GetExtendedWidth();
		unsigned int uiPngHeight = a_pimage->GetExtendedHeight();
		if (a_boolDrawModes)
		{
			uiPngWidth *= 2;
			uiPngHeight *= 2;
		}

		unsigned char* paucPngPixels = new unsigned char[uiPngWidth * uiPngHeight * 4];
		assert(paucPngPixels);

		ColorR8G8B8A8 *pargba8PngPixels = (ColorR8G8B8A8 *)paucPngPixels;

		for (unsigned int uiBlock = 0; uiBlock < a_pimage->GetNumberOfBlocks(); uiBlock++)
		{
			Block4x4 *pblock = &a_pimage->GetBlocks()[uiBlock];
			if (a_boolDrawModes)
			{
				if (a_pimage->GetFormat() == Image::Format::R11 || a_pimage->GetFormat() == Image::Format::SIGNED_R11)
				{
					DrawBlockPixels(pblock, pargba8PngPixels, uiPngWidth, true, 2);
				}
				else
				{
					DrawBlockPixels(pblock, pargba8PngPixels, uiPngWidth, false, 2);
				}
				DrawBlockMode2x(pblock, pargba8PngPixels, uiPngWidth);
			}
			else
			{
				if (a_pimage->GetFormat() == Image::Format::R11 || a_pimage->GetFormat() == Image::Format::SIGNED_R11)
				{
					DrawBlockPixels(pblock, pargba8PngPixels, uiPngWidth, true);
				}
				else
				{
					DrawBlockPixels(pblock, pargba8PngPixels, uiPngWidth, false);
				}
			}
		}

		char strFilename[200];
		if (a_boolDrawModes)
		{
			sprintf(strFilename, "%s%cModes.png", a_pstrOutputFolder, ETC_PATH_SLASH);
		}
		else
		{
			sprintf(strFilename, "%s%cDecoded.png", a_pstrOutputFolder, ETC_PATH_SLASH);
		}

		unsigned iResult = lodepng_encode32_file(strFilename, paucPngPixels, uiPngWidth, uiPngHeight);

		if (iResult != 0)
		{
			if (a_boolDrawModes)
			{
				printf("Error couldn't write modes image (%s)\n", strFilename);
			}
			else
			{
				printf("Error couldn't write decoded image (%s)\n", strFilename);
			}

			exit(1);
		}

	}

	// ----------------------------------------------------------------------------------------------------
	//
	void Analysis::DrawBlockPixels(Block4x4 *a_pblock,
									ColorR8G8B8A8 *a_pargba8Output,
									unsigned int a_uiOutputWidth,
									bool a_bGrayscale,
									unsigned int a_uiScale)
	{

		// output pixel coord of upper left corner of block
		unsigned int uiBlockH = a_uiScale * a_pblock->GetSourceH();
		unsigned int uiBlockV = a_uiScale * a_pblock->GetSourceV();

		ColorR8G8B8A8 *pargba8Block = &a_pargba8Output[uiBlockV*a_uiOutputWidth + uiBlockH];
		ColorFloatRGBA *pafrgbaDecodedColor = a_pblock->GetDecodedColors();
		float *pafDecodedAlpha = a_pblock->GetDecodedAlphas();

		for (unsigned int uiPixel = 0; uiPixel < Block4x4::PIXELS; uiPixel++)
		{
			ColorFloatRGBA *pfrgba = &pafrgbaDecodedColor[uiPixel];
			int iR;
			int iG;
			int iB;
			if (a_bGrayscale)
			{
				iR = pfrgba->IntRed(255.0f);
				iG = pfrgba->IntRed(255.0f);
				iB = pfrgba->IntRed(255.0f);
			}
			else
			{
				iR = pfrgba->IntRed(255.0f);
				iG = pfrgba->IntGreen(255.0f);
				iB = pfrgba->IntBlue(255.0f);
			}

			int iA = (int) roundf((255.0f*pafDecodedAlpha[uiPixel]));

			ColorR8G8B8A8 *pargba8ScaledPixel = &pargba8Block[(a_uiScale * (uiPixel % 4))*a_uiOutputWidth +
												a_uiScale * (uiPixel / 4)];

			// draw scaled pixel
			for (unsigned int uiV = 0; uiV < a_uiScale; uiV++)
			{
				for (unsigned int uiH = 0; uiH < a_uiScale; uiH++)
				{
					ColorR8G8B8A8 *prgba8 = &pargba8ScaledPixel[uiV*a_uiOutputWidth + uiH];

					prgba8->ucR = (unsigned char)iR;
					prgba8->ucG = (unsigned char)iG;
					prgba8->ucB = (unsigned char)iB;
					prgba8->ucA = (unsigned char)iA;
				}
			}
		}

	}

	// ----------------------------------------------------------------------------------------------------
	//
	void Analysis::DrawBlockMode2x(Block4x4 *a_pblock,
									ColorR8G8B8A8 *a_pargba8Output,
									unsigned int a_uiOutputWidth)
	{
		static const unsigned int SCALE = 2;

		typedef struct
		{
			int iH;
			int iV;
		} PixelCoord;

		static const PixelCoord s_apixelcoordOutline[] = {
			{ 0,0 },{ 1,0 },{ 2,0 },{ 3,0 },{ 4,0 },{ 5,0 },{ 6,0 },{ 7,0 },
			{ 0,1 },{ 0,2 },{ 0,3 },{ 0,4 },{ 0,5 },{ 0,6 },{ 0,7 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordFlip0[] =
		{
			{ 4,2 },{ 4,4 },{ 4,6 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordFlip1[] =
		{
			{ 2,4 },{ 4,4 },{ 6,4 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordBentDiffFlip0[] =
		{
			{ 4,2 },{ 4,3 },{ 4,5 },{ 4,6 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordBentDiffFlip1[] =
		{
			{ 2,4 },{ 3,4 },{ 5,4 },{ 6,4 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordIndividual[] =
		{
			{ 1,1 },{ 1,7 },{ 7,1 },{ 7,7 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordT[] =
		{
			{ 3,2 },{ 5,2 },
			{ 4,2 },{ 4,3 },{ 4,4 },{ 4,5 },{ 4,6 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordH[] =
		{
			{ 4,4 },
			{ 3,2 },{ 3,3 },{ 3,4 },{ 3,5 },{ 3,6 },
			{ 5,2 },{ 5,3 },{ 5,4 },{ 5,5 },{ 5,6 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordPlanar[] =
		{
			{ 4,2 },{ 5,2 },{ 5,3 },{ 5,4 },{ 4,4 },
			{ 3,2 },{ 3,3 },{ 3,4 },{ 3,5 },{ 3,6 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordUnknown[] =
		{
			{ 3,2 },{ 4,2 },{ 5,2 },{ 5,3 },{ 5,4 },{ 4,4 },{ 4,6 },{ -1,-1 }
		};

		// output pixel coord of upper left corner of block
		unsigned int uiBlockH = SCALE * a_pblock->GetSourceH();
		unsigned int uiBlockV = SCALE * a_pblock->GetSourceV();

		ColorR8G8B8A8 *pargba8Block = &a_pargba8Output[uiBlockV*a_uiOutputWidth + uiBlockH];
		ColorR8G8B8A8 rgba8Gray;
		rgba8Gray.ucR = 128;
		rgba8Gray.ucG = 128;
		rgba8Gray.ucB = 128;
		rgba8Gray.ucA = 255;

		// outline
		for (const PixelCoord *pcoord = s_apixelcoordOutline; pcoord->iH >= 0; pcoord++)
		{
			ColorR8G8B8A8 *prgba8 = &pargba8Block[pcoord->iV*a_uiOutputWidth + pcoord->iH];
			*prgba8 = rgba8Gray;
		}

		const PixelCoord *pacoordMode = nullptr;

		switch (a_pblock->GetEncodingMode())
		{
		case Block4x4Encoding::MODE_ETC1:

			// H/V split
			pacoordMode = a_pblock->GetFlip() ? s_apixelcoordFlip1 : s_apixelcoordFlip0;

			// individial
			if (a_pblock->IsDifferential() == false)
			{
				for (const PixelCoord *pcoord = s_apixelcoordIndividual; pcoord->iH >= 0; pcoord++)
				{
					ColorR8G8B8A8 *prgba8 = &pargba8Block[pcoord->iV*a_uiOutputWidth + pcoord->iH];
					*prgba8 = rgba8Gray;
				}
			}
			else if (a_pblock->GetEncoding()->HasSeverelyBentDifferentialColors())
			{
				pacoordMode = a_pblock->GetFlip() ? s_apixelcoordBentDiffFlip1 : s_apixelcoordBentDiffFlip0;
			}
			break;

		case Block4x4Encoding::MODE_T:
			pacoordMode = s_apixelcoordT;
			break;

		case Block4x4Encoding::MODE_H:
			pacoordMode = s_apixelcoordH;
			break;

		case Block4x4Encoding::MODE_PLANAR:
			pacoordMode = s_apixelcoordPlanar;
			break;

		default:
			pacoordMode = s_apixelcoordUnknown;
			break;
		}

		// draw mode
		for (const PixelCoord *pcoord = pacoordMode; pcoord->iH >= 0; pcoord++)
		{
			ColorR8G8B8A8 *prgba8 = &pargba8Block[pcoord->iV*a_uiOutputWidth + pcoord->iH];
			*prgba8 = rgba8Gray;
		}

	}

	// ----------------------------------------------------------------------------------------------------
	//
	float Analysis::ConvertErrorToPSNR(float a_fError, unsigned int a_uiTotalComponents)
	{

		float fMSE = a_fError / (float)a_uiTotalComponents;
		float fPSNR = ConvertMSEToPSNR(fMSE);

		return fPSNR;
	}

	// ----------------------------------------------------------------------------------------------------
	//

}
