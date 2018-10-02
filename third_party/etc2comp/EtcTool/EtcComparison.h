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
#include <stdio.h>

namespace Etc
{
	class Analysis;
	class Image;
	class Block4x4;

	class Comparison
	{
	public:

		Comparison(Analysis *a_panalysisParent, unsigned int a_uiIndex, const char *a_pstrFilename, int a_iPixelX = -1, int a_iPixelY = -1);

	private:

		static const float ERROR_EPSILON;

		void SetName(void);

		void DrawImageComparison(Image *a_pimageUnderTest);

		void DrawBlockComparison2x(Block4x4 *a_pblockUnderTest,
									ColorR8G8B8A8 *a_pargba8Output,
									unsigned int a_uiOutputWidth,
									float a_fErrorBlockUnderTest,
									float a_fErrorBlockReference);

		void WriteLogFile(void);
		void WriteBetterOrWorseBlocks(FILE *a_pfile, bool a_boolWorse);
		void WriteBlockInfo(FILE *a_pfile, Block4x4 *a_pblock, const char *a_pstrEncoder);

		Analysis		*m_panalysisParent;
		unsigned int	m_uiIndex;
		char			*m_pstrFilename;
		Image			*m_pimage;
		char			*m_pstrName;
		char			*m_pstrOutputFolder;
	};


}
