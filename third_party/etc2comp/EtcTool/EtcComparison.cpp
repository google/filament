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

#include "EtcComparison.h"

#include "EtcAnalysis.h"
#include "EtcTool.h"
#include "Etc.h"
#include "EtcFile.h"
#include "EtcMath.h"
#include "EtcImage.h"
#include "EtcBlock4x4.h"
#include "EtcBlock4x4Encoding_ETC1.h"
#include "EtcBlock4x4Encoding_RGB8.h"
#include "EtcBlock4x4Encoding_R11.h"
#include "EtcBlock4x4Encoding_RG11.h"

#include "lodepng.h"

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <limits>

namespace Etc
{
	const float Comparison::ERROR_EPSILON = 0.000001f;

	// ----------------------------------------------------------------------------------------------------
	//
	Comparison::Comparison(Analysis *a_panalysisParent, unsigned int a_uiIndex, const char *a_pstrFilename, int a_iPixelX, int a_iPixelY)
	{

		m_panalysisParent = a_panalysisParent;
		m_uiIndex = a_uiIndex;
		m_pstrFilename = new char[strlen(a_pstrFilename)+1];
		strcpy(m_pstrFilename, a_pstrFilename);

		SetName();

		m_pstrOutputFolder = new char[256];
		sprintf(m_pstrOutputFolder, "%s%cComparison_%s",
				m_panalysisParent->GetOutputFolder(), ETC_PATH_SLASH, m_pstrName);

		CreateNewDir(m_pstrOutputFolder);

		// read etc file
		Etc::File etcfile(a_pstrFilename, Etc::File::Format::INFER_FROM_FILE_EXTENSION);
		
		etcfile.UseSingleBlock(a_iPixelX, a_iPixelY);

		// construct image with encoding bits
		m_pimage = new Image(etcfile.GetImageFormat(),
								etcfile.GetSourceWidth(),
								etcfile.GetSourceHeight(),
								etcfile.GetEncodingBits(),
								etcfile.GetEncodingBitsBytes(),
								a_panalysisParent->GetImage(),
								a_panalysisParent->GetImage()->GetErrorMetric());

		if (m_pimage->GetExtendedWidth() != a_panalysisParent->GetImage()->GetExtendedWidth() ||
			m_pimage->GetExtendedHeight() != a_panalysisParent->GetImage()->GetExtendedHeight())
		{
			printf("Error: comparison image (%s) has different width or height\n", m_pstrFilename);
			exit(1);
		}

		// scale = 1
		Analysis::DrawImage(m_pimage, m_pstrOutputFolder, false);

		// scale = 2, with modes
		Analysis::DrawImage(m_pimage, m_pstrOutputFolder, true);

		DrawImageComparison(m_panalysisParent->GetImage());

		WriteLogFile();

	}

	// ----------------------------------------------------------------------------------------------------
	//
	void Comparison::DrawImageComparison(Image *a_pimageUnderTest)
	{

		assert(m_pimage->GetExtendedWidth() == a_pimageUnderTest->GetExtendedWidth() &&
			m_pimage->GetExtendedHeight() == a_pimageUnderTest->GetExtendedHeight());

		unsigned int uiPngWidth = 2 * m_pimage->GetExtendedWidth();
		unsigned int uiPngHeight = 2 * m_pimage->GetExtendedHeight();

		unsigned char* paucPngPixels = new unsigned char[uiPngWidth * uiPngHeight * 4];
		assert(paucPngPixels);

		ColorR8G8B8A8 *pargba8PngPixels = (ColorR8G8B8A8 *)paucPngPixels;

		for (unsigned int uiBlock = 0; uiBlock < m_pimage->GetNumberOfBlocks(); uiBlock++)
		{
			Block4x4 *pblock = &m_pimage->GetBlocks()[uiBlock];
			Block4x4 *pblockUnderTest = &a_pimageUnderTest->GetBlocks()[uiBlock];

			if (a_pimageUnderTest->GetFormat() == Image::Format::R11 || a_pimageUnderTest->GetFormat() == Image::Format::SIGNED_R11)
			{
				Analysis::DrawBlockPixels(pblock, pargba8PngPixels, uiPngWidth, true, 2);
			}
			else
			{
				Analysis::DrawBlockPixels(pblock, pargba8PngPixels, uiPngWidth, false, 2);
			}

			float fErrorBlockReference = pblock->GetError();
			float fErrorBlockUnderTest = pblockUnderTest->GetError();

			DrawBlockComparison2x(pblockUnderTest, pargba8PngPixels, uiPngWidth,
									fErrorBlockUnderTest, fErrorBlockReference);
		}

		char strFilename[200];
		sprintf(strFilename, "%s%cComparison.png", m_pstrOutputFolder, ETC_PATH_SLASH);

		unsigned iResult = lodepng_encode32_file(strFilename, paucPngPixels, uiPngWidth, uiPngHeight);
		
		if (iResult != 0)
		{
			printf("Error couldn't write modes image (%s)\n", strFilename);
		}

	}

	// ----------------------------------------------------------------------------------------------------
	//
	void Comparison::DrawBlockComparison2x(Block4x4 *a_pblockUnderTest,
											ColorR8G8B8A8 *a_pargba8Output,
											unsigned int a_uiOutputWidth,
											float a_fErrorBlockUnderTest,
											float a_fErrorBlockReference)
	{
		static const unsigned int SCALE = 2;

		typedef struct
		{
			int iH;
			int iV;
		} PixelCoord;

		static const PixelCoord s_apixelcoordDifference0[] = {
			{ 0,0 },{ 1,0 },{ 2,0 },{ 3,0 },{ 4,0 },{ 5,0 },{ 6,0 },{ 7,0 },
			{ 0,1 },{ 0,2 },{ 0,3 },{ 0,4 },{ 0,5 },{ 0,6 },{ 0,7 },
			{ 1,7 },{ 2,7 },{ 3,7 },{ 4,7 },{ 5,7 },{ 6,7 },{ 7,7 },
			{ 7,1 },{ 7,2 },{ 7,3 },{ 7,4 },{ 7,5 },{ 7,6 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordDifference5[] = {
			{ 3,3 },
			{ 0,0 },{ 1,0 },{ 2,0 },{ 3,0 },{ 4,0 },{ 5,0 },{ 6,0 },{ 7,0 },
			{ 0,1 },{ 0,2 },{ 0,3 },{ 0,4 },{ 0,5 },{ 0,6 },{ 0,7 },
			{ 1,7 },{ 2,7 },{ 3,7 },{ 4,7 },{ 5,7 },{ 6,7 },{ 7,7 },
			{ 7,1 },{ 7,2 },{ 7,3 },{ 7,4 },{ 7,5 },{ 7,6 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordDifference10[] = {
			{ 3,3 },{ 3,4 },{ 4,3 },{ 4,4 },
			{ 0,0 },{ 1,0 },{ 2,0 },{ 3,0 },{ 4,0 },{ 5,0 },{ 6,0 },{ 7,0 },
			{ 0,1 },{ 0,2 },{ 0,3 },{ 0,4 },{ 0,5 },{ 0,6 },{ 0,7 },
			{ 1,7 },{ 2,7 },{ 3,7 },{ 4,7 },{ 5,7 },{ 6,7 },{ 7,7 },
			{ 7,1 },{ 7,2 },{ 7,3 },{ 7,4 },{ 7,5 },{ 7,6 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordDifference20[] = {
			{ 2,2 },{ 2,3 },{ 3,2 },{ 3,3 },{ 4,4 },{ 4,5 },{ 5,4 },{ 5,5 },
			{ 0,0 },{ 1,0 },{ 2,0 },{ 3,0 },{ 4,0 },{ 5,0 },{ 6,0 },{ 7,0 },
			{ 0,1 },{ 0,2 },{ 0,3 },{ 0,4 },{ 0,5 },{ 0,6 },{ 0,7 },
			{ 1,7 },{ 2,7 },{ 3,7 },{ 4,7 },{ 5,7 },{ 6,7 },{ 7,7 },
			{ 7,1 },{ 7,2 },{ 7,3 },{ 7,4 },{ 7,5 },{ 7,6 },{ -1,-1 }
		};

		static const PixelCoord s_apixelcoordCorners[] = {
			{ 0,0 },{ 7,0 },{ 0,7 },{ 7,7 },{ -1,-1 }
		};

		// output pixel coord of upper left corner of block
		unsigned int uiBlockH = SCALE * a_pblockUnderTest->GetSourceH();
		unsigned int uiBlockV = SCALE * a_pblockUnderTest->GetSourceV();

		ColorR8G8B8A8 *pargba8Block = &a_pargba8Output[uiBlockV*a_uiOutputWidth + uiBlockH];

		float fRelativeError = 0.0f;
		if (a_fErrorBlockUnderTest != a_fErrorBlockReference)
		{
			fRelativeError = fabs((a_fErrorBlockUnderTest - a_fErrorBlockReference) /
								(a_fErrorBlockUnderTest + a_fErrorBlockReference));

			if (fabsf(a_fErrorBlockUnderTest - a_fErrorBlockReference) < ERROR_EPSILON)
			{
				fRelativeError = 0.0f;
			}
		}

		ColorR8G8B8A8 rgb8Draw;
		rgb8Draw.ucA = 255;

		// equal
		if (fRelativeError == 0.0f)
		{
			rgb8Draw.ucR = 128;
			rgb8Draw.ucG = 128;
			rgb8Draw.ucB = 128;
		}
		// better tthan reference
		else if (a_fErrorBlockUnderTest < a_fErrorBlockReference)
		{
			rgb8Draw.ucR = 0;
			rgb8Draw.ucG = 255;
			rgb8Draw.ucB = 0;
		}
		// worse than reference
		else if (a_fErrorBlockUnderTest > a_fErrorBlockReference)
		{
			rgb8Draw.ucR = 255;
			rgb8Draw.ucG = 0;
			rgb8Draw.ucB = 0;
		}

		const PixelCoord *papixelcoordDraw = s_apixelcoordCorners;

		// if 20% worse
		if (fRelativeError >= 1.44f)
		{
			papixelcoordDraw = s_apixelcoordDifference20;
		}
		// if 10% worse
		else if (fRelativeError >= 1.21f)
		{
			papixelcoordDraw = s_apixelcoordDifference10;
		}
		// if 5% worse
		else if (fRelativeError >= 1.1025f)
		{
			papixelcoordDraw = s_apixelcoordDifference5;
		}
		else if (fRelativeError > 0.0f)
		{
			papixelcoordDraw = s_apixelcoordDifference0;
		}

		// outline
		for (const PixelCoord *pcoord = papixelcoordDraw; pcoord->iH >= 0; pcoord++)
		{
			ColorR8G8B8A8 *prgba8 = &pargba8Block[pcoord->iV*a_uiOutputWidth + pcoord->iH];

			*prgba8 = rgb8Draw;
		}

	}

	// ----------------------------------------------------------------------------------------------------
	//
	void Comparison::SetName(void)
	{
		// alloc memory for name
		unsigned int uiNameBytes = (unsigned int)strlen(m_pstrFilename) + 1;
		if (uiNameBytes < 4)
		{
			uiNameBytes = 4;
		}
		m_pstrName = new char[uiNameBytes];
		m_pstrName[0] = 0;

		// first, try to find folder name of comparison image
		{
			char *pstr = new char[strlen(m_pstrFilename) + 1];
			strcpy(pstr, m_pstrFilename);

			int iLastSlash;
			int iPenultimateSlash;

			// find last slash
			for (iLastSlash = (int)strlen(pstr) - 1; iLastSlash >= 0; iLastSlash--)
			{
				if (pstr[iLastSlash] == ETC_PATH_SLASH)
				{
					break;
				}
			}

			// find penultimate slash
			for (iPenultimateSlash = iLastSlash - 1; iPenultimateSlash >= 0; iPenultimateSlash--)
			{
				if (pstr[iPenultimateSlash] == ETC_PATH_SLASH)
				{
					break;
				}
			}

			if (iLastSlash > 0)
			{
				pstr[iLastSlash] = 0;
				strcpy(m_pstrName, &pstr[iPenultimateSlash] + 1);
				assert(strlen(m_pstrName) > 0);
			}

			delete[] pstr;
		}

		// otherwise use index as name
		if (m_pstrName[0] == 0)
		{
			sprintf(m_pstrName, "%u", m_uiIndex);
		}

	}
		
	// ----------------------------------------------------------------------------------------------------
	//
	void Comparison::WriteLogFile(void)
	{

		char strPath[200];
		sprintf(strPath, "%s%cComparison.txt", m_pstrOutputFolder, ETC_PATH_SLASH);

		FILE *pfile = fopen(strPath, "wt");
		if (pfile == nullptr)
		{
			printf("Error couldn't open comparison log file (%s)\n", strPath);
			exit(1);
		}

		float fImageError = m_panalysisParent->GetImage()->GetError();
		unsigned int uiImagePixels = m_panalysisParent->GetImage()->GetSourceWidth() * 
										m_panalysisParent->GetImage()->GetSourceHeight();

		if (m_pimage->GetFormat() == Image::Format::R11 || m_pimage->GetFormat() == Image::Format::SIGNED_R11)
		{
			fprintf(pfile, "PSNR(r) = %.4f\n", Analysis::ConvertErrorToPSNR(fImageError, 1 * uiImagePixels));
			fprintf(pfile, "\n");
		}
		else if (m_pimage->GetFormat() == Image::Format::RG11 || m_pimage->GetFormat() == Image::Format::SIGNED_RG11)
		{
			fprintf(pfile, "PSNR(rg) = %.4f\n", Analysis::ConvertErrorToPSNR(fImageError, 2 * uiImagePixels));
			fprintf(pfile, "\n");
		}
		else
		{
			fprintf(pfile, "PSNR(rgb) = %.4f\n", Analysis::ConvertErrorToPSNR(fImageError, 3 * uiImagePixels));
			fprintf(pfile, "PSNR(rgba) = %.4f\n", Analysis::ConvertErrorToPSNR(fImageError, 4 * uiImagePixels));
			fprintf(pfile, "\n");
		}
		float fReferenceImageError = m_pimage->GetError();

		fprintf(pfile, "reference image %s\n", m_pstrFilename);
		if (m_pimage->GetFormat() == Image::Format::R11 || m_pimage->GetFormat() == Image::Format::SIGNED_R11)
		{
			fprintf(pfile, "reference PSNR(r) = %.4f\n",
				Analysis::ConvertErrorToPSNR(fReferenceImageError, 1 * uiImagePixels));
		}
		else if (m_pimage->GetFormat() == Image::Format::RG11 || m_pimage->GetFormat() == Image::Format::SIGNED_RG11)
		{
			fprintf(pfile, "reference PSNR(rg) = %.4f\n",
				Analysis::ConvertErrorToPSNR(fReferenceImageError, 2 * uiImagePixels));
		}
		else
		{
			fprintf(pfile, "reference PSNR(rgb) = %.4f\n",
				Analysis::ConvertErrorToPSNR(fReferenceImageError, 3 * uiImagePixels));
			fprintf(pfile, "reference PSNR(rgba) = %.4f\n",
				Analysis::ConvertErrorToPSNR(fReferenceImageError, 4 * uiImagePixels));
		}
		WriteBetterOrWorseBlocks(pfile, false);
		WriteBetterOrWorseBlocks(pfile, true);

		fclose(pfile);

	}

	// ----------------------------------------------------------------------------------------------------
	//
	void Comparison::WriteBetterOrWorseBlocks(FILE *a_pfile, bool a_boolWorse)
	{

		fprintf(a_pfile, "\n");
		if (a_boolWorse)
		{
			fprintf(a_pfile, "Worse Blocks\n");
		}
		else
		{
			fprintf(a_pfile, "Better Blocks\n");
		}

		for (unsigned int uiBlock = 0; uiBlock < m_pimage->GetNumberOfBlocks(); uiBlock++)
		{
			Block4x4 *pblockReference = &m_pimage->GetBlocks()[uiBlock];
			Block4x4 *pblockUnderTest = &m_panalysisParent->GetImage()->GetBlocks()[uiBlock];

			float fErrorBlockReference = pblockReference->GetError();
			float fErrorBlockUnderTest = pblockUnderTest->GetError();

			if (fabsf(fErrorBlockUnderTest - fErrorBlockReference) >= ERROR_EPSILON)
			{
				if ((a_boolWorse && (fErrorBlockUnderTest > fErrorBlockReference)) ||
					(!a_boolWorse && (fErrorBlockUnderTest < fErrorBlockReference)))
				{
					fprintf(a_pfile, "HV=%u,%u\n", 
								pblockUnderTest->GetSourceH(), pblockUnderTest->GetSourceV());

					WriteBlockInfo(a_pfile, pblockUnderTest, "bsi");
					WriteBlockInfo(a_pfile, pblockReference, "ref");

				}
			}

		}

	}

	// ----------------------------------------------------------------------------------------------------
	//
	void Comparison::WriteBlockInfo(FILE *a_pfile, Block4x4 *a_pblock, const char *a_pstrEncoder)
	{

		Block4x4Encoding *pencoding = a_pblock->GetEncoding();

		Block4x4Encoding_R11 *pencoding_r11 = static_cast<Block4x4Encoding_R11 *>(pencoding);
		Block4x4Encoding_RG11 *pencoding_rg11 = static_cast<Block4x4Encoding_RG11 *>(pencoding);
		Block4x4Encoding_RGB8 *pencoding_rgb8 = static_cast<Block4x4Encoding_RGB8 *>(pencoding);
		
		fprintf(a_pfile, "    %s:", a_pstrEncoder);

		float fPSNR = 0.0f;
		if (m_pimage->GetFormat() == Image::Format::R11 || m_pimage->GetFormat() == Image::Format::SIGNED_R11)
		{
			fPSNR = Analysis::ConvertErrorToPSNR(a_pblock->GetError(), 1 * 16);
		}
		else if (m_pimage->GetFormat() == Image::Format::RG11 || m_pimage->GetFormat() == Image::Format::SIGNED_RG11)
		{
			fPSNR = Analysis::ConvertErrorToPSNR(a_pblock->GetError(), 2 * 16);
		}
		else
		{
			fPSNR = Analysis::ConvertErrorToPSNR(a_pblock->GetError(), 3 * 16);
		}
		if (a_pblock->GetEncodingMode() == Block4x4Encoding::MODE_RG11)
		{
			fprintf(a_pfile, "psnr=%6.3f\n", fPSNR);
		}
		else
		{
			fprintf(a_pfile, "psnr=%6.3f ", fPSNR);
		}
		bool boolWriteSelectors = false;

		if (m_pimage->GetFormat() == Image::Format::RGBA8)
		{
			fprintf(a_pfile, "RGBA8 ");
			fprintf(a_pfile, "alpha base(%4.0f) ", pencoding_r11->GetRedBase());
			fprintf(a_pfile, "alpha multiplier(%3.0f) ", pencoding_r11->GetRedMultiplier());
			fprintf(a_pfile, "alpha table index(%2d) ", pencoding_r11->GetRedTableIndex());
		}
		if (a_pblock->GetEncodingMode() == Block4x4Encoding::MODE_ETC1)
		{
			fprintf(a_pfile, "ETC1(%c,%c) ", 
						a_pblock->IsDifferential() ? 'D' : 'I',
						a_pblock->GetFlip() ? 'V' : 'H' );

			if (a_pblock->IsDifferential())
			{
				fprintf(a_pfile, "color1(%2d,%2d,%2d) ",
					pencoding_rgb8->GetColor1().IntRed(31.0f),
					pencoding_rgb8->GetColor1().IntGreen(31.0f),
					pencoding_rgb8->GetColor1().IntBlue(31.0f) );
				fprintf(a_pfile, "color2(%2d,%2d,%2d) ",
					pencoding_rgb8->GetColor2().IntRed(31.0f),
					pencoding_rgb8->GetColor2().IntGreen(31.0f),
					pencoding_rgb8->GetColor2().IntBlue(31.0f));
			}
			else
			{
				fprintf(a_pfile, "color1(%2d,%2d,%2d) ",
					pencoding_rgb8->GetColor1().IntRed(15.0f),
					pencoding_rgb8->GetColor1().IntGreen(15.0f),
					pencoding_rgb8->GetColor1().IntBlue(15.0f));
				fprintf(a_pfile, "color2(%2d,%2d,%2d) ",
					pencoding_rgb8->GetColor2().IntRed(15.0f),
					pencoding_rgb8->GetColor2().IntGreen(15.0f),
					pencoding_rgb8->GetColor2().IntBlue(15.0f));
			}

			fprintf(a_pfile, "cw1(%d) cw2(%d) ", pencoding_rgb8->GetCW1(), pencoding_rgb8->GetCW2());

			boolWriteSelectors = true;
		}
		else if (a_pblock->GetEncodingMode() == Block4x4Encoding::MODE_T ||
			a_pblock->GetEncodingMode() == Block4x4Encoding::MODE_H)
		{
			fprintf(a_pfile, "%-9s ", a_pblock->GetEncodingModeName());

			fprintf(a_pfile, "color1(%2d,%2d,%2d) ",
				pencoding_rgb8->GetColor1().IntRed(15.0f),
				pencoding_rgb8->GetColor1().IntGreen(15.0f),
				pencoding_rgb8->GetColor1().IntBlue(15.0f));
			fprintf(a_pfile, "color2(%2d,%2d,%2d) ",
				pencoding_rgb8->GetColor2().IntRed(15.0f),
				pencoding_rgb8->GetColor2().IntGreen(15.0f),
				pencoding_rgb8->GetColor2().IntBlue(15.0f));

			fprintf(a_pfile, "cw1(%d)        ", pencoding_rgb8->GetCW1() );

			boolWriteSelectors = true;
		}
		else if (a_pblock->GetEncodingMode() == Block4x4Encoding::MODE_PLANAR)
		{
			fprintf(a_pfile, "%-9s ", a_pblock->GetEncodingModeName());

			fprintf(a_pfile, "color1(%2d,%2d,%2d) ",
				pencoding_rgb8->GetColor1().IntRed(63.0f),
				pencoding_rgb8->GetColor1().IntGreen(127.0f),
				pencoding_rgb8->GetColor1().IntBlue(63.0f));
			fprintf(a_pfile, "color2(%2d,%2d,%2d) ",
				pencoding_rgb8->GetColor2().IntRed(63.0f),
				pencoding_rgb8->GetColor2().IntGreen(127.0f),
				pencoding_rgb8->GetColor2().IntBlue(63.0f));
			fprintf(a_pfile, "color3(%2d,%2d,%2d) ",
				pencoding_rgb8->GetColor3().IntRed(63.0f),
				pencoding_rgb8->GetColor3().IntGreen(127.0f),
				pencoding_rgb8->GetColor3().IntBlue(63.0f));
		}
		else if (a_pblock->GetEncodingMode() == Block4x4Encoding::MODE_R11)
		{
			fprintf(a_pfile, "R11 ");
			fprintf(a_pfile, "base(%4.0f) ", pencoding_r11->GetRedBase());
			fprintf(a_pfile, "multiplier(%3.0f) ", pencoding_r11->GetRedMultiplier());
			fprintf(a_pfile, "table index(%2d) ", pencoding_r11->GetRedTableIndex());

			boolWriteSelectors = true;
		}
		else if (a_pblock->GetEncodingMode() == Block4x4Encoding::MODE_RG11)
		{
			fprintf(a_pfile, "RG11 image: %s\n", a_pstrEncoder);
			fprintf(a_pfile, "red base(%4.0f)\n", pencoding_rg11->GetRedBase());
			fprintf(a_pfile, "grn base(%4.0f)\n\n", pencoding_rg11->GetGrnBase());

			fprintf(a_pfile, "red multiplier(%3.0f)\n", pencoding_rg11->GetRedMultiplier());
			fprintf(a_pfile, "grn multiplier(%3.0f)\n\n", pencoding_rg11->GetGrnMultiplier());

			fprintf(a_pfile, "red table index(%2d)\n", pencoding_rg11->GetRedTableIndex());
			fprintf(a_pfile, "grn table index(%2d)\n\n", pencoding_rg11->GetGrnTableIndex());

			boolWriteSelectors = true;
		}
		else
		{
			assert(0);
		}

		if (boolWriteSelectors)
		{
			size_t selectorStringSize = Block4x4Encoding::PIXELS*4;
			char *redSelectors = new char[selectorStringSize];
			char *grnSelectors = new char[selectorStringSize];
			memset(&redSelectors[0], 0, selectorStringSize);
			memset(&grnSelectors[0], 0, selectorStringSize);
			
			if (a_pblock->GetEncodingMode() != Block4x4Encoding::MODE_RG11)
			{
				fprintf(a_pfile, "selectors(");
			}
			
			for (unsigned int uiPixel = 0; uiPixel < Block4x4Encoding::PIXELS; uiPixel++)
			{
				if (a_pblock->GetEncodingMode() == Block4x4Encoding::MODE_R11)
				{
					fprintf(a_pfile, "%u", pencoding_r11->GetRedSelectors()[uiPixel]);
				}
				else if (a_pblock->GetEncodingMode() == Block4x4Encoding::MODE_RG11)
				{
					char temp[2];
					sprintf(temp,"%d",pencoding_rg11->GetRedSelectors()[uiPixel]);
					strcat(redSelectors,temp);
					sprintf(temp, "%d", pencoding_rg11->GetGrnSelectors()[uiPixel]);
					strcat(grnSelectors, temp);

					strcat(redSelectors, ",\0");
					strcat(grnSelectors, ",\0");

					fprintf(a_pfile, "pixel[%d]:(%f,%f,%f)\n", uiPixel,
						pencoding->GetDecodedColors()[uiPixel].fR,
						pencoding->GetDecodedColors()[uiPixel].fG,
						pencoding->GetDecodedColors()[uiPixel].fB);

				}
				else
				{
					fprintf(a_pfile, "%u", pencoding_rgb8->GetSelectors()[uiPixel]);
				}
			}
			if (a_pblock->GetEncodingMode() == Block4x4Encoding::MODE_RG11)
			{
				redSelectors[selectorStringSize - 1] = '\0';
				grnSelectors[selectorStringSize - 1] = '\0';
				fprintf(a_pfile, "selectors(red: %s)\n", redSelectors);
				fprintf(a_pfile, "selectors(grn: %s)\n,", grnSelectors);
				delete[] redSelectors;
				redSelectors = NULL;
				delete[] grnSelectors;
				grnSelectors = NULL;
			}
			else
			{
				fprintf(a_pfile, ")");
			}
		}

		fprintf(a_pfile, "\n");

	}

	// ----------------------------------------------------------------------------------------------------
	//
}
