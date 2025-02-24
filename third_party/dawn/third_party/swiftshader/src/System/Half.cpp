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

#include "Half.hpp"

namespace sw {

half::half(float fp32)
{
	unsigned int fp32i = bit_cast<unsigned int>(fp32);
	unsigned int sign = (fp32i & 0x80000000) >> 16;
	unsigned int abs = fp32i & 0x7FFFFFFF;

	if(abs > 0x47FFEFFF)  // Infinity
	{
		fp16i = sign | 0x7FFF;
	}
	else if(abs < 0x38800000)  // Denormal
	{
		unsigned int mantissa = (abs & 0x007FFFFF) | 0x00800000;
		unsigned int e = 113 - (abs >> 23);

		if(e < 24)
		{
			abs = mantissa >> e;
		}
		else
		{
			abs = 0;
		}

		fp16i = sign | (abs + 0x00000FFF + ((abs >> 13) & 1)) >> 13;
	}
	else
	{
		fp16i = sign | (abs + 0xC8000000 + 0x00000FFF + ((abs >> 13) & 1)) >> 13;
	}
}

half::operator float() const
{
	unsigned int fp32i;

	int s = (fp16i >> 15) & 0x00000001;
	int e = (fp16i >> 10) & 0x0000001F;
	int m = fp16i & 0x000003FF;

	if(e == 0)
	{
		if(m == 0)
		{
			fp32i = s << 31;

			return bit_cast<float>(fp32i);
		}
		else
		{
			while(!(m & 0x00000400))
			{
				m <<= 1;
				e -= 1;
			}

			e += 1;
			m &= ~0x00000400;
		}
	}

	e = e + (127 - 15);
	m = m << 13;

	fp32i = (s << 31) | (e << 23) | m;

	return bit_cast<float>(fp32i);
}

half &half::operator=(float f)
{
	*this = half(f);

	return *this;
}

}  // namespace sw
