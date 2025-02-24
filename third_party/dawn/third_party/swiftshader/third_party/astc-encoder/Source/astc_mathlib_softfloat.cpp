// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2011-2020 Arm Limited
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy
// of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
// ----------------------------------------------------------------------------

/**
 * @brief Soft-float library for IEEE-754.
 */

#include "astc_mathlib.h"

/******************************************
  helper functions and their lookup tables
 ******************************************/
/* count leading zeros functions. Only used when the input is nonzero. */

#if defined(__GNUC__) && (defined(__i386) || defined(__amd64))
#elif defined(__arm__) && defined(__ARMCC_VERSION)
#elif defined(__arm__) && defined(__GNUC__)
#else
	/* table used for the slow default versions. */
	static const uint8_t clz_table[256] =
	{
		8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
#endif

/*
   32-bit count-leading-zeros function: use the Assembly instruction whenever possible. */
uint32_t clz32(uint32_t inp)
{
	#if defined(__GNUC__) && (defined(__i386) || defined(__amd64))
		uint32_t bsr;
	__asm__("bsrl %1, %0": "=r"(bsr):"r"(inp | 1));
		return 31 - bsr;
	#else
		#if defined(__arm__) && defined(__ARMCC_VERSION)
			return __clz(inp);			/* armcc builtin */
		#else
			#if defined(__arm__) && defined(__GNUC__)
				uint32_t lz;
			__asm__("clz %0, %1": "=r"(lz):"r"(inp));
				return lz;
			#else
				/* slow default version */
				uint32_t summa = 24;
				if (inp >= UINT32_C(0x10000))
				{
					inp >>= 16;
					summa -= 16;
				}
				if (inp >= UINT32_C(0x100))
				{
					inp >>= 8;
					summa -= 8;
				}
				return summa + clz_table[inp];
			#endif
		#endif
	#endif
}

/* convert from FP16 to FP32. */
sf32 sf16_to_sf32(sf16 inp)
{
	uint32_t inpx = inp;

	/*
		This table contains, for every FP16 sign/exponent value combination,
		the difference between the input FP16 value and the value obtained
		by shifting the correct FP32 result right by 13 bits.
		This table allows us to handle every case except denormals and NaN
		with just 1 table lookup, 2 shifts and 1 add.
	*/

	#define WITH_MB(a) INT32_C((a) | (1 << 31))
	static const int32_t tbl[64] =
	{
		WITH_MB(0x00000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000),
		INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000),
		INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000),
		INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), INT32_C(0x1C000), WITH_MB(0x38000),
		WITH_MB(0x38000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000),
		INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000),
		INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000),
		INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), INT32_C(0x54000), WITH_MB(0x70000)
	};

	int32_t res = tbl[inpx >> 10];
	res += inpx;

	/* the normal cases: the MSB of 'res' is not set. */
	if (res >= 0)				/* signed compare */
		return res << 13;

	/* Infinity and Zero: the bottom 10 bits of 'res' are clear. */
	if ((res & UINT32_C(0x3FF)) == 0)
		return res << 13;

	/* NaN: the exponent field of 'inp' is not zero; NaNs must be quietened. */
	if ((inpx & 0x7C00) != 0)
		return (res << 13) | UINT32_C(0x400000);

	/* the remaining cases are Denormals. */
	{
		uint32_t sign = (inpx & UINT32_C(0x8000)) << 16;
		uint32_t mskval = inpx & UINT32_C(0x7FFF);
		uint32_t leadingzeroes = clz32(mskval);
		mskval <<= leadingzeroes;
		return (mskval >> 8) + ((0x85 - leadingzeroes) << 23) + sign;
	}
}

/* convert from soft-float to native-float */
float sf16_to_float(sf16 p)
{
	if32 i;
	i.u = sf16_to_sf32(p);
	return i.f;
}

