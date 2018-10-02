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

#pragma once

#include "EtcColor.h"

namespace Etc
{

	class Comparison;
	class Image;
	class Block4x4;

	class Analysis
	{
	public:

		static const unsigned int MAX_COMPARISONS = 4;

		Analysis(Image *a_pimage, const char *a_pstrOutputFolder);

		void Compare(const char *a_pstrFilename, int a_iPixelX = -1, int a_iPixelY = -1);

		inline const char * GetOutputFolder(void)
		{
			return m_pstrOutputFolder;
		}

		static void DrawImage(Image *a_pimage, const char *a_pstrOutputFolder,
								bool a_boolDrawModes);

		inline Image * GetImage(void)
		{
			return m_pimage;
		}

		static void DrawBlockPixels(Block4x4 *a_pblock,
									ColorR8G8B8A8 *a_pargba8Output,
									unsigned int a_uiOutputWidth,
									bool a_bGrayscale,
									unsigned int a_uiScale = 1);

		static float ConvertErrorToPSNR(float a_fError, unsigned int a_uiTotalComponents);

	private:

		static void DrawBlockMode2x(Block4x4 *a_pblock,
									ColorR8G8B8A8 *a_pargba8Output,
									unsigned int a_uiOutputWidth);

		Image			*m_pimage;
		const char		*m_pstrOutputFolder;
		unsigned int	m_uiComparisons;
		Comparison		*m_apcomparison[MAX_COMPARISONS];
	};

}
