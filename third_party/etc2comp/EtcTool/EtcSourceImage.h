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

#include "EtcColorFloatRGBA.h"

namespace Etc
{

	class SourceImage
	{
	public:

		SourceImage(const char *a_pstrFilename, int a_iPixelX = -1, int a_iPixelY = -1);

		SourceImage(ColorFloatRGBA *a_pafrgbaSource,
					unsigned int a_uiSourceWidth,
					unsigned int a_uiSourceHeight);
		
		~SourceImage();

		void SetName(const char *a_pstrFilename);

		void NormalizeXYZ(void);

		inline const char *GetFilename(void)
		{
			return m_pstrFilename; 
		}

		inline const char *GetName(void) 
		{ 
			return m_pstrName; 
		}

		inline const char *GetFileExtension(void) 
		{ 
			return m_pstrFileExtension; 
		}

		inline unsigned int GetWidth(void) 
		{ 
			return m_uiWidth; 
		}

		inline unsigned int GetHeight(void) 
		{ 
			return m_uiHeight; 
		}

		inline ColorFloatRGBA * GetPixels(void)
		{
			return m_pafrgbaPixels;
		}

		inline ColorFloatRGBA * GetPixel(unsigned int a_uiColumn, unsigned int a_uiRow)
		{
			if (m_pafrgbaPixels == nullptr)
			{
				return nullptr;
			}

			return &m_pafrgbaPixels[a_uiRow*m_uiWidth + a_uiColumn];
		}

	private:

		void Read(int a_iPixelX = -1, int a_iPixelY = -1);

		char *m_pstrFilename;				// includes directory path and file extension
		char *m_pstrName;					// file name with directory path and file extension removed
		char *m_pstrFileExtension;
		unsigned int m_uiWidth;				// not necessarily block aligned
		unsigned int m_uiHeight;			// not necessarily block aligned
		ColorFloatRGBA *m_pafrgbaPixels;

	};

}	// namespace Etc
