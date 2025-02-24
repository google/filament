// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#include "Math.hpp"

namespace sw {

inline uint64_t FNV_1a(uint64_t hash, unsigned char data)
{
	return (hash ^ data) * 1099511628211;
}

uint64_t FNV_1a(const unsigned char *data, int size)
{
	int64_t hash = 0xCBF29CE484222325;

	for(int i = 0; i < size; i++)
	{
		hash = FNV_1a(hash, data[i]);
	}

	return hash;
}

unsigned char sRGB8toLinear8(unsigned char value)
{
	static unsigned char sRGBtoLinearTable[256] = { 255 };
	if(sRGBtoLinearTable[0] == 255)
	{
		for(int i = 0; i < 256; i++)
		{
			sRGBtoLinearTable[i] = static_cast<unsigned char>(sw::sRGBtoLinear(static_cast<float>(i) / 255.0f) * 255.0f + 0.5f);
		}
	}

	return sRGBtoLinearTable[value];
}

}  // namespace sw
