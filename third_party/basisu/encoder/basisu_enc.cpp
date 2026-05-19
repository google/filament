// basisu_enc.cpp
// Copyright (C) 2019-2026 Binomial LLC. All Rights Reserved.
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
#include "basisu_enc.h"
#include "basisu_resampler.h"
#include "basisu_resampler_filters.h"
#include "basisu_etc.h"
#include "../transcoder/basisu_transcoder.h"
#include "basisu_bc7enc.h"
#include "jpgd.h"
#include "pvpngreader.h"
#include "basisu_opencl.h"
#include "basisu_uastc_hdr_4x4_enc.h"
#include "basisu_astc_hdr_6x6_enc.h"
#include "basisu_astc_ldr_common.h"
#include "basisu_astc_ldr_encode.h"

#include <vector>

#ifndef TINYEXR_USE_ZFP
#define TINYEXR_USE_ZFP (1)
#endif
#include "3rdparty/tinyexr.h"

#ifndef MINIZ_HEADER_FILE_ONLY
#define MINIZ_HEADER_FILE_ONLY
#endif
#ifndef MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#endif
#include "basisu_miniz.h"

#define QOI_IMPLEMENTATION
#include "3rdparty/qoi.h"

#if defined(_WIN32)
// For QueryPerformanceCounter/QueryPerformanceFrequency
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace basisu
{
	uint64_t interval_timer::g_init_ticks, interval_timer::g_freq;
	double interval_timer::g_timer_freq;

#if BASISU_SUPPORT_SSE
	bool g_cpu_supports_sse41;
#endif

	fast_linear_to_srgb g_fast_linear_to_srgb;
		
	uint8_t g_hamming_dist[256] =
	{
		0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
	};

	// This is a Public Domain 8x8 font from here:
	// https://github.com/dhepper/font8x8/blob/master/font8x8_basic.h
	const uint8_t g_debug_font8x8_basic[127 - 32 + 1][8] = 
	{
	 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},	// U+0020 ( )
	 { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},   // U+0021 (!)
	 { 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0022 (")
	 { 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},   // U+0023 (#)
	 { 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},   // U+0024 ($)
	 { 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},   // U+0025 (%)
	 { 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},   // U+0026 (&)
	 { 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0027 (')
	 { 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},   // U+0028 (()
	 { 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},   // U+0029 ())
	 { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},   // U+002A (*)
	 { 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},   // U+002B (+)
	 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+002C (,)
	 { 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},   // U+002D (-)
	 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+002E (.)
	 { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},   // U+002F (/)
	 { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},   // U+0030 (0)
	 { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},   // U+0031 (1)
	 { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},   // U+0032 (2)
	 { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},   // U+0033 (3)
	 { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},   // U+0034 (4)
	 { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},   // U+0035 (5)
	 { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},   // U+0036 (6)
	 { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},   // U+0037 (7)
	 { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+0038 (8)
	 { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},   // U+0039 (9)
	 { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+003A (:)
	 { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+003B (;)
	 { 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},   // U+003C (<)
	 { 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},   // U+003D (=)
	 { 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},   // U+003E (>)
	 { 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},   // U+003F (?)
	 { 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},   // U+0040 (@)
	 { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   // U+0041 (A)
	 { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   // U+0042 (B)
	 { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},   // U+0043 (C)
	 { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},   // U+0044 (D)
	 { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   // U+0045 (E)
	 { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},   // U+0046 (F)
	 { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},   // U+0047 (G)
	 { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   // U+0048 (H)
	 { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0049 (I)
	 { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},   // U+004A (J)
	 { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   // U+004B (K)
	 { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},   // U+004C (L)
	 { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   // U+004D (M)
	 { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   // U+004E (N)
	 { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   // U+004F (O)
	 { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   // U+0050 (P)
	 { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},   // U+0051 (Q)
	 { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},   // U+0052 (R)
	 { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},   // U+0053 (S)
	 { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0054 (T)
	 { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},   // U+0055 (U)
	 { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0056 (V)
	 { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},   // U+0057 (W)
	 { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   // U+0058 (X)
	 { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+0059 (Y)
	 { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   // U+005A (Z)
	 { 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},   // U+005B ([)
	 { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},   // U+005C (\)
	 { 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},   // U+005D (])
	 { 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},   // U+005E (^)
	 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   // U+005F (_)
	 { 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0060 (`)
	 { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},   // U+0061 (a)
	 { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},   // U+0062 (b)
	 { 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},   // U+0063 (c)
	 { 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},   // U+0064 (d)
	 { 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},   // U+0065 (e)
	 { 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},   // U+0066 (f)
	 { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0067 (g)
	 { 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},   // U+0068 (h)
	 { 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0069 (i)
	 { 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},   // U+006A (j)
	 { 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},   // U+006B (k)
	 { 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+006C (l)
	 { 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},   // U+006D (m)
	 { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},   // U+006E (n)
	 { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+006F (o)
	 { 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},   // U+0070 (p)
	 { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},   // U+0071 (q)
	 { 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},   // U+0072 (r)
	 { 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},   // U+0073 (s)
	 { 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+0074 (t)
	 { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},   // U+0075 (u)
	 { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0076 (v)
	 { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},   // U+0077 (w)
	 { 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},   // U+0078 (x)
	 { 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0079 (y)
	 { 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},   // U+007A (z)
	 { 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},   // U+007B ({)
	 { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   // U+007C (|)
	 { 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},   // U+007D (})
	 { 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+007E (~)
	 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}    // U+007F
	};

	float g_srgb_to_linear_table[256];

	void init_srgb_to_linear_table()
	{
		for (int i = 0; i < 256; ++i)
			g_srgb_to_linear_table[i] = srgb_to_linear((float)i * (1.0f / 255.0f));
	}

	bool g_library_initialized;
	std::mutex g_encoder_init_mutex;
					
	// Encoder library initialization (just call once at startup)
	bool basisu_encoder_init(bool use_opencl, bool opencl_force_serialization)
	{
		std::lock_guard<std::mutex> lock(g_encoder_init_mutex);

		if (g_library_initialized)
			return true;

		detect_sse41();
				
		basist::basisu_transcoder_init();
		pack_etc1_solid_color_init();
		//uastc_init();
		bc7enc_compress_block_init(); // must be after uastc_init()

		// Don't bother initializing the OpenCL module at all if it's been completely disabled.
		if (use_opencl)
		{
			opencl_init(opencl_force_serialization);
		}

		interval_timer::init(); // make sure interval_timer globals are initialized from main thread to avoid TSAN reports

		astc_hdr_enc_init();
		basist::bc6h_enc_init();
		astc_6x6_hdr::global_init();
		astc_ldr::global_init();
		astc_ldr::encoder_init();

		init_srgb_to_linear_table();
				
		g_library_initialized = true;
		return true;
	}

	void basisu_encoder_deinit()
	{
		opencl_deinit();

		g_library_initialized = false;
	}
		
	void error_vprintf(const char* pFmt, va_list args)
	{
		const uint32_t BUF_SIZE = 256;
		char buf[BUF_SIZE];

		va_list args_copy;
		va_copy(args_copy, args);
		int total_chars = vsnprintf(buf, sizeof(buf), pFmt, args_copy);
		va_end(args_copy);

		if (total_chars < 0)
		{
			assert(0);
			return;
		}

		fflush(stdout);

		if (total_chars >= (int)BUF_SIZE)
		{
			basisu::vector<char> var_buf(total_chars + 1);
			
			va_copy(args_copy, args);
			int total_chars_retry = vsnprintf(var_buf.data(), var_buf.size(), pFmt, args_copy);
			va_end(args_copy);

			if (total_chars_retry < 0)
			{
				assert(0);
				return;
			}

			fprintf(stderr, "ERROR: %s", var_buf.data());
		}
		else
		{
			fprintf(stderr, "ERROR: %s", buf);
		}
	}

	void error_printf(const char *pFmt, ...)
	{
		va_list args;
		va_start(args, pFmt);
		error_vprintf(pFmt, args);
		va_end(args);
	}

#if defined(_WIN32)
	void platform_sleep(uint32_t ms)
	{
		Sleep(ms);
	}
#else
	void platform_sleep(uint32_t ms)
	{
		// TODO
		BASISU_NOTE_UNUSED(ms);
	}
#endif

#if defined(_WIN32)
	inline void query_counter(timer_ticks* pTicks)
	{
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(pTicks));
	}
	inline void query_counter_frequency(timer_ticks* pTicks)
	{
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(pTicks));
	}
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__EMSCRIPTEN__)
#include <sys/time.h>
	inline void query_counter(timer_ticks* pTicks)
	{
		struct timeval cur_time;
		gettimeofday(&cur_time, NULL);
		*pTicks = static_cast<unsigned long long>(cur_time.tv_sec) * 1000000ULL + static_cast<unsigned long long>(cur_time.tv_usec);
	}
	inline void query_counter_frequency(timer_ticks* pTicks)
	{
		*pTicks = 1000000;
	}
#elif defined(__GNUC__)
#include <sys/timex.h>
	inline void query_counter(timer_ticks* pTicks)
	{
		struct timeval cur_time;
		gettimeofday(&cur_time, NULL);
		*pTicks = static_cast<unsigned long long>(cur_time.tv_sec) * 1000000ULL + static_cast<unsigned long long>(cur_time.tv_usec);
	}
	inline void query_counter_frequency(timer_ticks* pTicks)
	{
		*pTicks = 1000000;
	}
#else
#error TODO
#endif
				
	interval_timer::interval_timer() : m_start_time(0), m_stop_time(0), m_started(false), m_stopped(false)
	{
		if (!g_timer_freq)
			init();
	}

	void interval_timer::start()
	{
		query_counter(&m_start_time);
		m_started = true;
		m_stopped = false;
	}

	void interval_timer::stop()
	{
		assert(m_started);
		query_counter(&m_stop_time);
		m_stopped = true;
	}

	double interval_timer::get_elapsed_secs() const
	{
		assert(m_started);
		if (!m_started)
			return 0;

		timer_ticks stop_time = m_stop_time;
		if (!m_stopped)
			query_counter(&stop_time);

		timer_ticks delta = stop_time - m_start_time;
		return delta * g_timer_freq;
	}
		
	void interval_timer::init()
	{
		if (!g_timer_freq)
		{
			query_counter_frequency(&g_freq);
			g_timer_freq = 1.0f / g_freq;
			query_counter(&g_init_ticks);
		}
	}

	timer_ticks interval_timer::get_ticks()
	{
		if (!g_timer_freq)
			init();
		timer_ticks ticks;
		query_counter(&ticks);
		return ticks - g_init_ticks;
	}

	double interval_timer::ticks_to_secs(timer_ticks ticks)
	{
		if (!g_timer_freq)
			init();
		return ticks * g_timer_freq;
	}

	// Note this is linear<->sRGB, NOT REC709 which uses slightly different equations/transfer functions. 
	// However the gamuts/white points of REC709 and sRGB are the same.
	float linear_to_srgb(float l)
	{
		assert(l >= 0.0f && l <= 1.0f);
		if (l < .0031308f)
			return saturate(l * 12.92f);
		else
			return saturate(1.055f * powf(l, 1.0f / 2.4f) - .055f);
	}
		
	float srgb_to_linear(float s)
	{
		assert(s >= 0.0f && s <= 1.0f);
		if (s < .04045f)
			return saturate(s * (1.0f / 12.92f));
		else
			return saturate(powf((s + .055f) * (1.0f / 1.055f), 2.4f));
	}
		
	const uint32_t MAX_32BIT_ALLOC_SIZE = 250000000;
		
	bool load_tga(const char* pFilename, image& img)
	{
		int w = 0, h = 0, n_chans = 0;
		uint8_t* pImage_data = read_tga(pFilename, w, h, n_chans);
				
		if ((!pImage_data) || (!w) || (!h) || ((n_chans != 3) && (n_chans != 4)))
		{
			error_printf("Failed loading .TGA image \"%s\"!\n", pFilename);

			if (pImage_data)
				free(pImage_data);
						
			return false;
		}

		if (sizeof(void *) == sizeof(uint32_t))
		{
			if (((uint64_t)w * h * n_chans) > MAX_32BIT_ALLOC_SIZE)
			{
				error_printf("Image \"%s\" is too large (%ux%u) to process in a 32-bit build!\n", pFilename, w, h);

				if (pImage_data)
					free(pImage_data);

				return false;
			}
		}
		
		img.resize(w, h);

		const uint8_t *pSrc = pImage_data;
		for (int y = 0; y < h; y++)
		{
			color_rgba *pDst = &img(0, y);

			for (int x = 0; x < w; x++)
			{
				pDst->r = pSrc[0];
				pDst->g = pSrc[1];
				pDst->b = pSrc[2];
				pDst->a = (n_chans == 3) ? 255 : pSrc[3];

				pSrc += n_chans;
				++pDst;
			}
		}

		free(pImage_data);

		return true;
	}

	bool load_qoi(const char* pFilename, image& img)
	{
		qoi_desc desc;
		clear_obj(desc);

		void* p = qoi_read(pFilename, &desc, 4);
		if (!p)
			return false;

		img.grant_ownership(static_cast<color_rgba *>(p), desc.width, desc.height);

		return true;
	}

	bool load_png(const uint8_t *pBuf, size_t buf_size, image &img, const char *pFilename)
	{
		interval_timer tm;
		tm.start();
		
		if (!buf_size)
			return false;

		uint32_t width = 0, height = 0, num_chans = 0;
		void* pImage = pv_png::load_png(pBuf, buf_size, 4, width, height, num_chans);

		if (!pImage)
		{
			error_printf("pv_png::load_png failed while loading image \"%s\"\n", pFilename);
			return false;
		}

		img.grant_ownership(reinterpret_cast<color_rgba*>(pImage), width, height);

		//debug_printf("Total load_png() time: %3.3f secs\n", tm.get_elapsed_secs());

		return true;
	}
		
	bool load_png(const char* pFilename, image& img)
	{
		uint8_vec buffer;
		if (!read_file_to_vec(pFilename, buffer))
		{
			error_printf("load_png: Failed reading file \"%s\"!\n", pFilename);
			return false;
		}

		return load_png(buffer.data(), buffer.size(), img, pFilename);
	}

	bool load_jpg(const char *pFilename, image& img)
	{
		int width = 0, height = 0, actual_comps = 0;
		uint8_t *pImage_data = jpgd::decompress_jpeg_image_from_file(pFilename, &width, &height, &actual_comps, 4, jpgd::jpeg_decoder::cFlagLinearChromaFiltering);
		if (!pImage_data)
			return false;
		
		img.init(pImage_data, width, height, 4);
		
		free(pImage_data);

		return true;
	}

	bool load_jpg(const uint8_t* pBuf, size_t buf_size, image& img)
	{
		if (buf_size > INT_MAX)
		{
			assert(0);
			return false;
		}

		int width = 0, height = 0, actual_comps = 0;
		uint8_t* pImage_data = jpgd::decompress_jpeg_image_from_memory(pBuf, (int)buf_size, &width, &height, &actual_comps, 4, jpgd::jpeg_decoder::cFlagLinearChromaFiltering);
		if (!pImage_data)
			return false;

		img.init(pImage_data, width, height, 4);

		free(pImage_data);

		return true;
	}

	bool load_image(const char* pFilename, image& img)
	{
		std::string ext(string_get_extension(std::string(pFilename)));

		if (ext.length() == 0)
			return false;

		const char *pExt = ext.c_str();

		if (strcasecmp(pExt, "png") == 0)
			return load_png(pFilename, img);
		if (strcasecmp(pExt, "tga") == 0)
			return load_tga(pFilename, img);
		if (strcasecmp(pExt, "qoi") == 0)
			return load_qoi(pFilename, img);
		if ( (strcasecmp(pExt, "jpg") == 0) || (strcasecmp(pExt, "jfif") == 0) || (strcasecmp(pExt, "jpeg") == 0) )
			return load_jpg(pFilename, img);

		return false;
	}

	void convert_ldr_to_hdr_image(imagef &img, const image &ldr_img, bool ldr_srgb_to_linear, float linear_nit_multiplier, float ldr_black_bias)
	{
		img.resize(ldr_img.get_width(), ldr_img.get_height());

		for (uint32_t y = 0; y < ldr_img.get_height(); y++)
		{
			for (uint32_t x = 0; x < ldr_img.get_width(); x++)
			{
				const color_rgba& c = ldr_img(x, y);

				vec4F& d = img(x, y);
				if (ldr_srgb_to_linear)
				{
					float r = (float)c[0];
					float g = (float)c[1];
					float b = (float)c[2];

					if (ldr_black_bias > 0.0f)
					{
						// ASTC HDR is noticeably weaker dealing with blocks containing some pixels with components set to 0.
						// Add a very slight bias less than .5 to avoid this difficulity. When the HDR image is mapped to SDR sRGB and rounded back to 8-bits, this bias will still result in zero.
						// (FWIW, in reality, a physical monitor would be unlikely to have a perfectly zero black level.)
						// This is purely optional and on most images it doesn't matter visually.
						if (r == 0.0f)
							r = ldr_black_bias;
						if (g == 0.0f)
							g = ldr_black_bias;
						if (b == 0.0f)
							b = ldr_black_bias;
					}

					// Compute how much linear light would be emitted by a SDR 80-100 nit monitor.
					d[0] = srgb_to_linear(r * (1.0f / 255.0f)) * linear_nit_multiplier;
					d[1] = srgb_to_linear(g * (1.0f / 255.0f)) * linear_nit_multiplier;
					d[2] = srgb_to_linear(b * (1.0f / 255.0f)) * linear_nit_multiplier;
				}
				else
				{
					d[0] = c[0] * (1.0f / 255.0f) * linear_nit_multiplier;
					d[1] = c[1] * (1.0f / 255.0f) * linear_nit_multiplier;
					d[2] = c[2] * (1.0f / 255.0f) * linear_nit_multiplier;
				}
				d[3] = c[3] * (1.0f / 255.0f);
			}
		}
	}

	bool load_image_hdr(const void* pMem, size_t mem_size, imagef& img, uint32_t width, uint32_t height, hdr_image_type img_type, bool ldr_srgb_to_linear, float linear_nit_multiplier, float ldr_black_bias)
	{
		if ((!pMem) || (!mem_size))
		{
			assert(0);
			return false;
		}

		switch (img_type)
		{
		case hdr_image_type::cHITRGBAHalfFloat:
		{
			if (mem_size != width * height * sizeof(basist::half_float) * 4)
			{
				assert(0);
				return false;
			}

			if ((!width) || (!height))
			{
				assert(0);
				return false;
			}

			const basist::half_float* pSrc_image_h = static_cast<const basist::half_float *>(pMem);

			img.resize(width, height);
			for (uint32_t y = 0; y < height; y++)
			{
				for (uint32_t x = 0; x < width; x++)
				{
					const basist::half_float* pSrc_pixel = &pSrc_image_h[x * 4];

					vec4F& dst = img(x, y);
					dst[0] = basist::half_to_float(pSrc_pixel[0]);
					dst[1] = basist::half_to_float(pSrc_pixel[1]);
					dst[2] = basist::half_to_float(pSrc_pixel[2]);
					dst[3] = basist::half_to_float(pSrc_pixel[3]);
				}
			
				pSrc_image_h += (width * 4);
			}

			break;
		}
		case hdr_image_type::cHITRGBAFloat:
		{
			if (mem_size != width * height * sizeof(float) * 4)
			{
				assert(0);
				return false;
			}

			if ((!width) || (!height))
			{
				assert(0);
				return false;
			}

			img.resize(width, height);
			memcpy((void *)img.get_ptr(), pMem, width * height * sizeof(float) * 4);

			break;
		}
		case hdr_image_type::cHITJPGImage:
		{
			image ldr_img;
			if (!load_jpg(static_cast<const uint8_t*>(pMem), mem_size, ldr_img))
				return false;

			convert_ldr_to_hdr_image(img, ldr_img, ldr_srgb_to_linear, linear_nit_multiplier, ldr_black_bias);
			break;
		}
		case hdr_image_type::cHITPNGImage:
		{
			image ldr_img;
			if (!load_png(static_cast<const uint8_t *>(pMem), mem_size, ldr_img))
				return false;

			convert_ldr_to_hdr_image(img, ldr_img, ldr_srgb_to_linear, linear_nit_multiplier, ldr_black_bias);
			break;
		}
		case hdr_image_type::cHITEXRImage:
		{
			if (!read_exr(pMem, mem_size, img))
				return false;

			break;
		}
		case hdr_image_type::cHITHDRImage:
		{
			uint8_vec buf(mem_size);
			memcpy(buf.get_ptr(), pMem, mem_size);

			rgbe_header_info hdr;
			if (!read_rgbe(buf, img, hdr))
				return false;

			break;
		}
		default:
			assert(0);
			return false;
		}

		return true;
	}

	bool is_image_filename_hdr(const char *pFilename)
	{
		std::string ext(string_get_extension(std::string(pFilename)));

		if (ext.length() == 0)
			return false;

		const char* pExt = ext.c_str();

		return ((strcasecmp(pExt, "hdr") == 0) || (strcasecmp(pExt, "exr") == 0));
	}
	
	// TODO: move parameters to struct, add a HDR clean flag to eliminate NaN's/Inf's
	bool load_image_hdr(const char* pFilename, imagef& img, bool ldr_srgb_to_linear, float linear_nit_multiplier, float ldr_black_bias)
	{
		std::string ext(string_get_extension(std::string(pFilename)));

		if (ext.length() == 0)
			return false;

		const char* pExt = ext.c_str();

		if (strcasecmp(pExt, "hdr") == 0)
		{
			rgbe_header_info rgbe_info;
			if (!read_rgbe(pFilename, img, rgbe_info))
				return false;
			return true;
		}
					
		if (strcasecmp(pExt, "exr") == 0)
		{
			int n_chans = 0;
			if (!read_exr(pFilename, img, n_chans))
				return false;
			return true;
		}

		// Try loading image as LDR, then optionally convert to linear light.
		{
			image ldr_img;
			if (!load_image(pFilename, ldr_img))
				return false;

			convert_ldr_to_hdr_image(img, ldr_img, ldr_srgb_to_linear, linear_nit_multiplier, ldr_black_bias);
		}

		return true;
	}
			
	bool save_png(const char* pFilename, const image &img, uint32_t image_save_flags, uint32_t grayscale_comp)
	{
		if (!img.get_total_pixels())
			return false;
				
		void* pPNG_data = nullptr;
		size_t PNG_data_size = 0;

		if (image_save_flags & cImageSaveGrayscale)
		{
			uint8_vec g_pixels(img.get_total_pixels());
			uint8_t* pDst = &g_pixels[0];

			for (uint32_t y = 0; y < img.get_height(); y++)
				for (uint32_t x = 0; x < img.get_width(); x++)
					*pDst++ = img(x, y)[grayscale_comp];

			pPNG_data = buminiz::tdefl_write_image_to_png_file_in_memory_ex(g_pixels.data(), img.get_width(), img.get_height(), 1, &PNG_data_size, 1, false);
		}
		else
		{
			bool has_alpha = false;
			
			if ((image_save_flags & cImageSaveIgnoreAlpha) == 0)
				has_alpha = img.has_alpha();

			if (!has_alpha)
			{
				uint8_vec rgb_pixels(img.get_total_pixels() * 3);
				uint8_t* pDst = &rgb_pixels[0];

				for (uint32_t y = 0; y < img.get_height(); y++)
				{
					const color_rgba* pSrc = &img(0, y);
					for (uint32_t x = 0; x < img.get_width(); x++)
					{
						pDst[0] = pSrc->r;
						pDst[1] = pSrc->g;
						pDst[2] = pSrc->b;
						
						pSrc++;
						pDst += 3;
					}
				}

				pPNG_data = buminiz::tdefl_write_image_to_png_file_in_memory_ex(rgb_pixels.data(), img.get_width(), img.get_height(), 3, &PNG_data_size, 1, false);
			}
			else
			{
				pPNG_data = buminiz::tdefl_write_image_to_png_file_in_memory_ex(img.get_ptr(), img.get_width(), img.get_height(), 4, &PNG_data_size, 1, false);
			}
		}

		if (!pPNG_data)
			return false;

		bool status = write_data_to_file(pFilename, pPNG_data, PNG_data_size);
		if (!status)
		{
			error_printf("save_png: Failed writing to filename \"%s\"!\n", pFilename);
		}

		free(pPNG_data);
						
		return status;
	}

	bool save_qoi(const char* pFilename, const image& img, uint32_t qoi_colorspace)
	{
		assert(img.get_width() && img.get_height());

		qoi_desc desc;
		clear_obj(desc);

		desc.width = img.get_width();
		desc.height = img.get_height();
		desc.channels = 4;
		desc.colorspace = (uint8_t)qoi_colorspace;
		
		int out_len = 0;
		void* pData = qoi_encode(img.get_ptr(), &desc, &out_len);
		if ((!pData) || (!out_len))
			return false;

		const bool status = write_data_to_file(pFilename, pData, out_len);

		QOI_FREE(pData);
		pData = nullptr;

		return status;
	}
		
	bool read_file_to_vec(const char* pFilename, uint8_vec& data)
	{
		FILE* pFile = nullptr;
#ifdef _WIN32
		fopen_s(&pFile, pFilename, "rb");
#else
		pFile = fopen(pFilename, "rb");
#endif
		if (!pFile)
			return false;
				
		fseek(pFile, 0, SEEK_END);
#ifdef _WIN32
		int64_t filesize = _ftelli64(pFile);
#else
		int64_t filesize = ftello(pFile);
#endif
		if (filesize < 0)
		{
			fclose(pFile);
			return false;
		}
		fseek(pFile, 0, SEEK_SET);

		if (sizeof(size_t) == sizeof(uint32_t))
		{
			if (filesize > 0x70000000)
			{
				// File might be too big to load safely in one alloc
				fclose(pFile);
				return false;
			}
		}

		if (!data.try_resize((size_t)filesize))
		{
			fclose(pFile);
			return false;
		}

		if (filesize)
		{
			if (fread(&data[0], 1, (size_t)filesize, pFile) != (size_t)filesize)
			{
				fclose(pFile);
				return false;
			}
		}

		fclose(pFile);
		return true;
	}

	bool read_file_to_data(const char* pFilename, void *pData, size_t len)
	{
		assert(pData && len);
		if ((!pData) || (!len))
			return false;

		FILE* pFile = nullptr;
#ifdef _WIN32
		fopen_s(&pFile, pFilename, "rb");
#else
		pFile = fopen(pFilename, "rb");
#endif
		if (!pFile)
			return false;

		fseek(pFile, 0, SEEK_END);
#ifdef _WIN32
		int64_t filesize = _ftelli64(pFile);
#else
		int64_t filesize = ftello(pFile);
#endif

		if ((filesize < 0) || ((size_t)filesize < len))
		{
			fclose(pFile);
			return false;
		}
		fseek(pFile, 0, SEEK_SET);
				
		if (fread(pData, 1, (size_t)len, pFile) != (size_t)len)
		{
			fclose(pFile);
			return false;
		}

		fclose(pFile);
		return true;
	}

	bool write_data_to_file(const char* pFilename, const void* pData, size_t len)
	{
		FILE* pFile = nullptr;
#ifdef _WIN32
		fopen_s(&pFile, pFilename, "wb");
#else
		pFile = fopen(pFilename, "wb");
#endif
		if (!pFile)
			return false;

		if (len)
		{
			if (fwrite(pData, 1, len, pFile) != len)
			{
				fclose(pFile);
				return false;
			}
		}

		return fclose(pFile) != EOF;
	}
		
	bool image_resample(const image &src, image &dst, bool srgb,
		const char *pFilter, float filter_scale, 
		bool wrapping,
		uint32_t first_comp, uint32_t num_comps, 
		float filter_scale_y)
	{
		assert((first_comp + num_comps) <= 4);

		const int cMaxComps = 4;
				
		const uint32_t src_w = src.get_width(), src_h = src.get_height();
		const uint32_t dst_w = dst.get_width(), dst_h = dst.get_height();
				
		if (maximum(src_w, src_h) > BASISU_RESAMPLER_MAX_DIMENSION)
		{
			printf("Image is too large!\n");
			return false;
		}

		if (!src_w || !src_h || !dst_w || !dst_h)
			return false;
				
		if ((num_comps < 1) || (num_comps > cMaxComps))
			return false;
				
		if ((minimum(dst_w, dst_h) < 1) || (maximum(dst_w, dst_h) > BASISU_RESAMPLER_MAX_DIMENSION))
		{
			printf("Image is too large!\n");
			return false;
		}

		if ( (src_w == dst_w) && (src_h == dst_h) && 
			(filter_scale == 1.0f) &&
			((filter_scale_y < 0.0f) || (filter_scale_y == 1.0f)) )
		{
			dst = src;
			return true;
		}

		float srgb_to_linear_table[256];
		if (srgb)
		{
			for (int i = 0; i < 256; ++i)
				srgb_to_linear_table[i] = srgb_to_linear((float)i * (1.0f/255.0f));
		}

		const int LINEAR_TO_SRGB_TABLE_SIZE = 8192;
		uint8_t linear_to_srgb_table[LINEAR_TO_SRGB_TABLE_SIZE];

		if (srgb)
		{
			for (int i = 0; i < LINEAR_TO_SRGB_TABLE_SIZE; ++i)
				linear_to_srgb_table[i] = (uint8_t)clamp<int>((int)(255.0f * linear_to_srgb((float)i * (1.0f / (LINEAR_TO_SRGB_TABLE_SIZE - 1))) + .5f), 0, 255);
		}

		std::vector<float> samples[cMaxComps];
		Resampler *resamplers[cMaxComps];
		
		resamplers[0] = new Resampler(src_w, src_h, dst_w, dst_h,
			wrapping ? Resampler::BOUNDARY_WRAP : Resampler::BOUNDARY_CLAMP, 0.0f, 1.0f,
			pFilter, nullptr, nullptr, 
			filter_scale, (filter_scale_y >= 0.0f) ? filter_scale_y : filter_scale, 0, 0);
		samples[0].resize(src_w);

		for (uint32_t i = 1; i < num_comps; ++i)
		{
			resamplers[i] = new Resampler(src_w, src_h, dst_w, dst_h,
				wrapping ? Resampler::BOUNDARY_WRAP : Resampler::BOUNDARY_CLAMP, 0.0f, 1.0f,
				pFilter, resamplers[0]->get_clist_x(), resamplers[0]->get_clist_y(), 
				filter_scale, (filter_scale_y >= 0.0f) ? filter_scale_y : filter_scale, 0, 0);
			samples[i].resize(src_w);
		}

		uint32_t dst_y = 0;

		for (uint32_t src_y = 0; src_y < src_h; ++src_y)
		{
			const color_rgba *pSrc = &src(0, src_y);

			// Put source lines into resampler(s)
			for (uint32_t x = 0; x < src_w; ++x)
			{
				for (uint32_t c = 0; c < num_comps; ++c)
				{
					const uint32_t comp_index = first_comp + c;
					const uint32_t v = (*pSrc)[comp_index];

					if (!srgb || (comp_index == 3))
						samples[c][x] = v * (1.0f / 255.0f);
					else
						samples[c][x] = srgb_to_linear_table[v];
				}

				pSrc++;
			}

			for (uint32_t c = 0; c < num_comps; ++c)
			{
				if (!resamplers[c]->put_line(&samples[c][0]))
				{
					for (uint32_t i = 0; i < num_comps; i++)
						delete resamplers[i];
					return false;
				}
			}

			// Now retrieve any output lines
			for (;;)
			{
				uint32_t c;
				for (c = 0; c < num_comps; ++c)
				{
					const uint32_t comp_index = first_comp + c;

					const float *pOutput_samples = resamplers[c]->get_line();
					if (!pOutput_samples)
						break;

					const bool linear_flag = !srgb || (comp_index == 3);
					
					color_rgba *pDst = &dst(0, dst_y);

					for (uint32_t x = 0; x < dst_w; x++)
					{
						// TODO: Add dithering
						if (linear_flag)
						{
							int j = (int)(255.0f * pOutput_samples[x] + .5f);
							(*pDst)[comp_index] = (uint8_t)clamp<int>(j, 0, 255);
						}
						else
						{
							int j = (int)((LINEAR_TO_SRGB_TABLE_SIZE - 1) * pOutput_samples[x] + .5f);
							(*pDst)[comp_index] = linear_to_srgb_table[clamp<int>(j, 0, LINEAR_TO_SRGB_TABLE_SIZE - 1)];
						}

						pDst++;
					}
				}
				if (c < num_comps)
					break;

				++dst_y;
			}
		}

		for (uint32_t i = 0; i < num_comps; ++i)
			delete resamplers[i];

		return true;
	}

	bool image_resample(const imagef& src, imagef& dst, 
		const char* pFilter, float filter_scale,
		bool wrapping,
		uint32_t first_comp, uint32_t num_comps)
	{
		assert((first_comp + num_comps) <= 4);

		const int cMaxComps = 4;

		const uint32_t src_w = src.get_width(), src_h = src.get_height();
		const uint32_t dst_w = dst.get_width(), dst_h = dst.get_height();

		if (maximum(src_w, src_h) > BASISU_RESAMPLER_MAX_DIMENSION)
		{
			printf("Image is too large!\n");
			return false;
		}

		if (!src_w || !src_h || !dst_w || !dst_h)
			return false;

		if ((num_comps < 1) || (num_comps > cMaxComps))
			return false;

		if ((minimum(dst_w, dst_h) < 1) || (maximum(dst_w, dst_h) > BASISU_RESAMPLER_MAX_DIMENSION))
		{
			printf("Image is too large!\n");
			return false;
		}

		if ((src_w == dst_w) && (src_h == dst_h) && (filter_scale == 1.0f))
		{
			dst = src;
			return true;
		}

		std::vector<float> samples[cMaxComps];
		Resampler* resamplers[cMaxComps];

		resamplers[0] = new Resampler(src_w, src_h, dst_w, dst_h,
			wrapping ? Resampler::BOUNDARY_WRAP : Resampler::BOUNDARY_CLAMP, 1.0f, 0.0f, // no clamping
			pFilter, nullptr, nullptr, filter_scale, filter_scale, 0, 0);
		samples[0].resize(src_w);

		for (uint32_t i = 1; i < num_comps; ++i)
		{
			resamplers[i] = new Resampler(src_w, src_h, dst_w, dst_h,
				wrapping ? Resampler::BOUNDARY_WRAP : Resampler::BOUNDARY_CLAMP, 1.0f, 0.0f, // no clamping
				pFilter, resamplers[0]->get_clist_x(), resamplers[0]->get_clist_y(), filter_scale, filter_scale, 0, 0);
			samples[i].resize(src_w);
		}

		uint32_t dst_y = 0;

		for (uint32_t src_y = 0; src_y < src_h; ++src_y)
		{
			const vec4F* pSrc = &src(0, src_y);

			// Put source lines into resampler(s)
			for (uint32_t x = 0; x < src_w; ++x)
			{
				for (uint32_t c = 0; c < num_comps; ++c)
				{
					const uint32_t comp_index = first_comp + c;
					const float v = (*pSrc)[comp_index];

					samples[c][x] = v;
				}

				pSrc++;
			}

			for (uint32_t c = 0; c < num_comps; ++c)
			{
				if (!resamplers[c]->put_line(&samples[c][0]))
				{
					for (uint32_t i = 0; i < num_comps; i++)
						delete resamplers[i];
					return false;
				}
			}

			// Now retrieve any output lines
			for (;;)
			{
				uint32_t c;
				for (c = 0; c < num_comps; ++c)
				{
					const uint32_t comp_index = first_comp + c;

					const float* pOutput_samples = resamplers[c]->get_line();
					if (!pOutput_samples)
						break;
										
					vec4F* pDst = &dst(0, dst_y);

					for (uint32_t x = 0; x < dst_w; x++)
					{
						(*pDst)[comp_index] = pOutput_samples[x];
						pDst++;
					}
				}
				if (c < num_comps)
					break;

				++dst_y;
			}
		}

		for (uint32_t i = 0; i < num_comps; ++i)
			delete resamplers[i];

		return true;
	}

	void canonical_huffman_calculate_minimum_redundancy(sym_freq *A, int num_syms)
	{
		// See the paper "In-Place Calculation of Minimum Redundancy Codes" by Moffat and Katajainen
		if (!num_syms)
			return;

		if (1 == num_syms)
		{
			A[0].m_key = 1;
			return;
		}
		
		A[0].m_key += A[1].m_key;
		
		int s = 2, r = 0, next;
		for (next = 1; next < (num_syms - 1); ++next)
		{
			if ((s >= num_syms) || (A[r].m_key < A[s].m_key))
			{
				A[next].m_key = A[r].m_key;
				A[r].m_key = next;
				++r;
			}
			else
			{
				A[next].m_key = A[s].m_key;
				++s;
			}

			if ((s >= num_syms) || ((r < next) && A[r].m_key < A[s].m_key))
			{
				A[next].m_key = A[next].m_key + A[r].m_key;
				A[r].m_key = next;
				++r;
			}
			else
			{
				A[next].m_key = A[next].m_key + A[s].m_key;
				++s;
			}
		}
		A[num_syms - 2].m_key = 0;

		for (next = num_syms - 3; next >= 0; --next)
		{
			A[next].m_key = 1 + A[A[next].m_key].m_key;
		}

		int num_avail = 1, num_used = 0, depth = 0;
		r = num_syms - 2;
		next = num_syms - 1;
		while (num_avail > 0)
		{
			for ( ; (r >= 0) && ((int)A[r].m_key == depth); ++num_used, --r )
				;

			for ( ; num_avail > num_used; --next, --num_avail)
				A[next].m_key = depth;

			num_avail = 2 * num_used;
			num_used = 0;
			++depth;
		}
	}

	void canonical_huffman_enforce_max_code_size(int *pNum_codes, int code_list_len, int max_code_size)
	{
		int i;
		uint32_t total = 0;
		if (code_list_len <= 1)
			return;

		for (i = max_code_size + 1; i <= cHuffmanMaxSupportedInternalCodeSize; i++)
			pNum_codes[max_code_size] += pNum_codes[i];

		for (i = max_code_size; i > 0; i--)
			total += (((uint32_t)pNum_codes[i]) << (max_code_size - i));

		while (total != (1UL << max_code_size))
		{
			pNum_codes[max_code_size]--;
			for (i = max_code_size - 1; i > 0; i--)
			{
				if (pNum_codes[i])
				{
					pNum_codes[i]--;
					pNum_codes[i + 1] += 2;
					break;
				}
			}

			total--;
		}
	}

	sym_freq *canonical_huffman_radix_sort_syms(uint32_t num_syms, sym_freq *pSyms0, sym_freq *pSyms1)
	{
		uint32_t total_passes = 2, pass_shift, pass, i, hist[256 * 2];
		sym_freq *pCur_syms = pSyms0, *pNew_syms = pSyms1;

		clear_obj(hist);

		for (i = 0; i < num_syms; i++)
		{
			uint32_t freq = pSyms0[i].m_key;
			
			// We scale all input frequencies to 16-bits.
			assert(freq <= UINT16_MAX);

			hist[freq & 0xFF]++;
			hist[256 + ((freq >> 8) & 0xFF)]++;
		}

		while ((total_passes > 1) && (num_syms == hist[(total_passes - 1) * 256]))
			total_passes--;

		for (pass_shift = 0, pass = 0; pass < total_passes; pass++, pass_shift += 8)
		{
			const uint32_t *pHist = &hist[pass << 8];
			uint32_t offsets[256], cur_ofs = 0;
			for (i = 0; i < 256; i++)
			{
				offsets[i] = cur_ofs;
				cur_ofs += pHist[i];
			}

			for (i = 0; i < num_syms; i++)
				pNew_syms[offsets[(pCur_syms[i].m_key >> pass_shift) & 0xFF]++] = pCur_syms[i];

			sym_freq *t = pCur_syms;
			pCur_syms = pNew_syms;
			pNew_syms = t;
		}

		return pCur_syms;
	}

	bool huffman_encoding_table::init(uint32_t num_syms, const uint16_t *pFreq, uint32_t max_code_size)
	{
		if (max_code_size > cHuffmanMaxSupportedCodeSize)
			return false;
		if ((!num_syms) || (num_syms > cHuffmanMaxSyms))
			return false;

		uint32_t total_used_syms = 0;
		for (uint32_t i = 0; i < num_syms; i++)
			if (pFreq[i])
				total_used_syms++;

		if (!total_used_syms)
			return false;

		std::vector<sym_freq> sym_freq0(total_used_syms), sym_freq1(total_used_syms);
		for (uint32_t i = 0, j = 0; i < num_syms; i++)
		{
			if (pFreq[i])
			{
				sym_freq0[j].m_key = pFreq[i];
				sym_freq0[j++].m_sym_index = static_cast<uint16_t>(i);
			}
		}

		sym_freq *pSym_freq = canonical_huffman_radix_sort_syms(total_used_syms, &sym_freq0[0], &sym_freq1[0]);

		canonical_huffman_calculate_minimum_redundancy(pSym_freq, total_used_syms);

		int num_codes[cHuffmanMaxSupportedInternalCodeSize + 1];
		clear_obj(num_codes);

		for (uint32_t i = 0; i < total_used_syms; i++)
		{
			if (pSym_freq[i].m_key > cHuffmanMaxSupportedInternalCodeSize)
				return false;

			num_codes[pSym_freq[i].m_key]++;
		}

		canonical_huffman_enforce_max_code_size(num_codes, total_used_syms, max_code_size);

		m_code_sizes.resize(0);
		m_code_sizes.resize(num_syms);

		m_codes.resize(0);
		m_codes.resize(num_syms);

		for (uint32_t i = 1, j = total_used_syms; i <= max_code_size; i++)
			for (uint32_t l = num_codes[i]; l > 0; l--)
				m_code_sizes[pSym_freq[--j].m_sym_index] = static_cast<uint8_t>(i);

		uint32_t next_code[cHuffmanMaxSupportedInternalCodeSize + 1];

		next_code[1] = 0;
		for (uint32_t j = 0, i = 2; i <= max_code_size; i++)
			next_code[i] = j = ((j + num_codes[i - 1]) << 1);

		for (uint32_t i = 0; i < num_syms; i++)
		{
			uint32_t rev_code = 0, code, code_size;
			if ((code_size = m_code_sizes[i]) == 0)
				continue;
			if (code_size > cHuffmanMaxSupportedInternalCodeSize)
				return false;
			code = next_code[code_size]++;
			for (uint32_t l = code_size; l > 0; l--, code >>= 1)
				rev_code = (rev_code << 1) | (code & 1);
			m_codes[i] = static_cast<uint16_t>(rev_code);
		}

		return true;
	}

	bool huffman_encoding_table::init(uint32_t num_syms, const uint32_t *pSym_freq, uint32_t max_code_size)
	{
		if ((!num_syms) || (num_syms > cHuffmanMaxSyms))
			return false;

		uint16_vec sym_freq(num_syms);

		uint32_t max_freq = 0;
		for (uint32_t i = 0; i < num_syms; i++)
			max_freq = maximum(max_freq, pSym_freq[i]);

		if (max_freq < UINT16_MAX)
		{
			for (uint32_t i = 0; i < num_syms; i++)
				sym_freq[i] = static_cast<uint16_t>(pSym_freq[i]);
		}
		else
		{
			for (uint32_t i = 0; i < num_syms; i++)
			{
				if (pSym_freq[i])
				{
					uint32_t f = static_cast<uint32_t>((static_cast<uint64_t>(pSym_freq[i]) * 65534U + (max_freq >> 1)) / max_freq);
					sym_freq[i] = static_cast<uint16_t>(clamp<uint32_t>(f, 1, 65534));
				}
			}
		}

		return init(num_syms, &sym_freq[0], max_code_size);
	}

	void bitwise_coder::end_nonzero_run(uint16_vec &syms, uint32_t &run_size, uint32_t len)
	{
		if (run_size)
		{
			if (run_size < cHuffmanSmallRepeatSizeMin)
			{
				while (run_size--)
					syms.push_back(static_cast<uint16_t>(len));
			}
			else if (run_size <= cHuffmanSmallRepeatSizeMax)
			{
				syms.push_back(static_cast<uint16_t>(cHuffmanSmallRepeatCode | ((run_size - cHuffmanSmallRepeatSizeMin) << 6)));
			}
			else
			{
				assert((run_size >= cHuffmanBigRepeatSizeMin) && (run_size <= cHuffmanBigRepeatSizeMax));
				syms.push_back(static_cast<uint16_t>(cHuffmanBigRepeatCode | ((run_size - cHuffmanBigRepeatSizeMin) << 6)));
			}
		}

		run_size = 0;
	}

	void bitwise_coder::end_zero_run(uint16_vec &syms, uint32_t &run_size)
	{
		if (run_size)
		{
			if (run_size < cHuffmanSmallZeroRunSizeMin)
			{
				while (run_size--)
					syms.push_back(0);
			}
			else if (run_size <= cHuffmanSmallZeroRunSizeMax)
			{
				syms.push_back(static_cast<uint16_t>(cHuffmanSmallZeroRunCode | ((run_size - cHuffmanSmallZeroRunSizeMin) << 6)));
			}
			else
			{
				assert((run_size >= cHuffmanBigZeroRunSizeMin) && (run_size <= cHuffmanBigZeroRunSizeMax));
				syms.push_back(static_cast<uint16_t>(cHuffmanBigZeroRunCode | ((run_size - cHuffmanBigZeroRunSizeMin) << 6)));
			}
		}

		run_size = 0;
	}

	uint32_t bitwise_coder::emit_huffman_table(const huffman_encoding_table &tab)
	{
		const uint64_t start_bits = m_total_bits;

		const uint8_vec &code_sizes = tab.get_code_sizes();

		uint32_t total_used = tab.get_total_used_codes();
		put_bits(total_used, cHuffmanMaxSymsLog2);
			
		if (!total_used)
			return 0;

		uint16_vec syms;
		syms.reserve(total_used + 16);

		uint32_t prev_code_len = UINT_MAX, zero_run_size = 0, nonzero_run_size = 0;

		for (uint32_t i = 0; i <= total_used; ++i)
		{
			const uint32_t code_len = (i == total_used) ? 0xFF : code_sizes[i];
			assert((code_len == 0xFF) || (code_len <= 16));

			if (code_len)
			{
				end_zero_run(syms, zero_run_size);

				if (code_len != prev_code_len)
				{
					end_nonzero_run(syms, nonzero_run_size, prev_code_len);
					if (code_len != 0xFF)
						syms.push_back(static_cast<uint16_t>(code_len));
				}
				else if (++nonzero_run_size == cHuffmanBigRepeatSizeMax)
					end_nonzero_run(syms, nonzero_run_size, prev_code_len);
			}
			else
			{
				end_nonzero_run(syms, nonzero_run_size, prev_code_len);

				if (++zero_run_size == cHuffmanBigZeroRunSizeMax)
					end_zero_run(syms, zero_run_size);
			}

			prev_code_len = code_len;
		}

		histogram h(cHuffmanTotalCodelengthCodes);
		for (uint32_t i = 0; i < syms.size(); i++)
			h.inc(syms[i] & 63);

		huffman_encoding_table ct;
		if (!ct.init(h, 7))
			return 0;

		assert(cHuffmanTotalSortedCodelengthCodes == cHuffmanTotalCodelengthCodes);

		uint32_t total_codelength_codes;
		for (total_codelength_codes = cHuffmanTotalSortedCodelengthCodes; total_codelength_codes > 0; total_codelength_codes--)
			if (ct.get_code_sizes()[g_huffman_sorted_codelength_codes[total_codelength_codes - 1]])
				break;

		assert(total_codelength_codes);

		put_bits(total_codelength_codes, 5);
		for (uint32_t i = 0; i < total_codelength_codes; i++)
			put_bits(ct.get_code_sizes()[g_huffman_sorted_codelength_codes[i]], 3);

		for (uint32_t i = 0; i < syms.size(); ++i)
		{
			const uint32_t l = syms[i] & 63, e = syms[i] >> 6;

			put_code(l, ct);
				
			if (l == cHuffmanSmallZeroRunCode)
				put_bits(e, cHuffmanSmallZeroRunExtraBits);
			else if (l == cHuffmanBigZeroRunCode)
				put_bits(e, cHuffmanBigZeroRunExtraBits);
			else if (l == cHuffmanSmallRepeatCode)
				put_bits(e, cHuffmanSmallRepeatExtraBits);
			else if (l == cHuffmanBigRepeatCode)
				put_bits(e, cHuffmanBigRepeatExtraBits);
		}

		return (uint32_t)(m_total_bits - start_bits);
	}

	bool huffman_test(int rand_seed)
	{
		histogram h(19);

		// Feed in a fibonacci sequence to force large codesizes
		h[0] += 1; h[1] += 1; h[2] += 2; h[3] += 3;
		h[4] += 5; h[5] += 8; h[6] += 13; h[7] += 21;
		h[8] += 34; h[9] += 55; h[10] += 89; h[11] += 144;
		h[12] += 233; h[13] += 377; h[14] += 610; h[15] += 987;
		h[16] += 1597; h[17] += 2584; h[18] += 4181;

		huffman_encoding_table etab;
		etab.init(h, 16);
		
		{
			bitwise_coder c;
			c.init(1024);

			c.emit_huffman_table(etab);
			for (int i = 0; i < 19; i++)
				c.put_code(i, etab);

			c.flush();

			basist::bitwise_decoder d;
			d.init(&c.get_bytes()[0], static_cast<uint32_t>(c.get_bytes().size()));

			basist::huffman_decoding_table dtab;
			bool success = d.read_huffman_table(dtab);
			if (!success)
			{
				assert(0);
				printf("Failure 5\n");
				return false;
			}

			for (uint32_t i = 0; i < 19; i++)
			{
				uint32_t s = d.decode_huffman(dtab);
				if (s != i)
				{
					assert(0);
					printf("Failure 5\n");
					return false;
				}
			}
		}

		basisu::rand r;
		r.seed(rand_seed);

		for (int iter = 0; iter < 500000; iter++)
		{
			printf("%u\n", iter);

			uint32_t max_sym = r.irand(0, 8193);
			uint32_t num_codes = r.irand(1, 10000);
			uint_vec syms(num_codes);

			for (uint32_t i = 0; i < num_codes; i++)
			{
				if (r.bit())
					syms[i] = r.irand(0, max_sym);
				else
				{
					int s = (int)(r.gaussian((float)max_sym / 2, (float)maximum<int>(1, max_sym / 2)) + .5f);
					s = basisu::clamp<int>(s, 0, max_sym);

					syms[i] = s;
				}

			}

			histogram h1(max_sym + 1);
			for (uint32_t i = 0; i < num_codes; i++)
				h1[syms[i]]++;

			huffman_encoding_table etab2;
			if (!etab2.init(h1, 16))
			{
				assert(0);
				printf("Failed 0\n");
				return false;
			}

			bitwise_coder c;
			c.init(1024);

			c.emit_huffman_table(etab2);

			for (uint32_t i = 0; i < num_codes; i++)
				c.put_code(syms[i], etab2);

			c.flush();

			basist::bitwise_decoder d;
			d.init(&c.get_bytes()[0], (uint32_t)c.get_bytes().size());

			basist::huffman_decoding_table dtab;
			bool success = d.read_huffman_table(dtab);
			if (!success)
			{
				assert(0);
				printf("Failed 2\n");
				return false;
			}

			for (uint32_t i = 0; i < num_codes; i++)
			{
				uint32_t s = d.decode_huffman(dtab);
				if (s != syms[i])
				{
					assert(0);
					printf("Failed 4\n");
					return false;
				}
			}

		}
		return true;
	}

	void palette_index_reorderer::init(uint32_t num_indices, const uint32_t *pIndices, uint32_t num_syms, pEntry_dist_func pDist_func, void *pCtx, float dist_func_weight)
	{
		assert((num_syms > 0) && (num_indices > 0));
		assert((dist_func_weight >= 0.0f) && (dist_func_weight <= 1.0f));

		clear();

		m_remap_table.resize(num_syms);
		m_entries_picked.reserve(num_syms);
		m_total_count_to_picked.resize(num_syms);

		if (num_indices <= 1)
			return;

		prepare_hist(num_syms, num_indices, pIndices);
		find_initial(num_syms);

		while (m_entries_to_do.size())
		{
			// Find the best entry to move into the picked list.
			uint32_t best_entry;
			double best_count;
			find_next_entry(best_entry, best_count, pDist_func, pCtx, dist_func_weight);

			// We now have chosen an entry to place in the picked list, now determine which side it goes on.
			const uint32_t entry_to_move = m_entries_to_do[best_entry];
								
			float side = pick_side(num_syms, entry_to_move, pDist_func, pCtx, dist_func_weight);
								
			// Put entry_to_move either on the "left" or "right" side of the picked entries
			if (side <= 0)
				m_entries_picked.push_back(entry_to_move);
			else
				m_entries_picked.insert(m_entries_picked.begin(), entry_to_move);

			// Erase best_entry from the todo list
			m_entries_to_do.erase(m_entries_to_do.begin() + best_entry);

			// We've just moved best_entry to the picked list, so now we need to update m_total_count_to_picked[] to factor the additional count to best_entry
			for (uint32_t i = 0; i < m_entries_to_do.size(); i++)
				m_total_count_to_picked[m_entries_to_do[i]] += get_hist(m_entries_to_do[i], entry_to_move, num_syms);
		}

		for (uint32_t i = 0; i < num_syms; i++)
			m_remap_table[m_entries_picked[i]] = i;
	}

	void palette_index_reorderer::prepare_hist(uint32_t num_syms, uint32_t num_indices, const uint32_t *pIndices)
	{
		m_hist.resize(0);
		m_hist.resize(num_syms * num_syms);

		for (uint32_t i = 0; i < num_indices; i++)
		{
			const uint32_t idx = pIndices[i];
			inc_hist(idx, (i < (num_indices - 1)) ? pIndices[i + 1] : -1, num_syms);
			inc_hist(idx, (i > 0) ? pIndices[i - 1] : -1, num_syms);
		}
	}

	void palette_index_reorderer::find_initial(uint32_t num_syms)
	{
		uint32_t max_count = 0, max_index = 0;
		for (uint32_t i = 0; i < num_syms * num_syms; i++)
			if (m_hist[i] > max_count)
				max_count = m_hist[i], max_index = i;

		uint32_t a = max_index / num_syms, b = max_index % num_syms;

		const size_t ofs = m_entries_picked.size();

		m_entries_picked.push_back(a);
		m_entries_picked.push_back(b);

		for (uint32_t i = 0; i < num_syms; i++)
			if ((i != m_entries_picked[ofs + 1]) && (i != m_entries_picked[ofs]))
				m_entries_to_do.push_back(i);

		for (uint32_t i = 0; i < m_entries_to_do.size(); i++)
			for (uint32_t j = 0; j < m_entries_picked.size(); j++)
				m_total_count_to_picked[m_entries_to_do[i]] += get_hist(m_entries_to_do[i], m_entries_picked[j], num_syms);
	}

	void palette_index_reorderer::find_next_entry(uint32_t &best_entry, double &best_count, pEntry_dist_func pDist_func, void *pCtx, float dist_func_weight)
	{
		best_entry = 0;
		best_count = 0;

		for (uint32_t i = 0; i < m_entries_to_do.size(); i++)
		{
			const uint32_t u = m_entries_to_do[i];
			double total_count = m_total_count_to_picked[u];

			if (pDist_func)
			{
				float w = maximum<float>((*pDist_func)(u, m_entries_picked.front(), pCtx), (*pDist_func)(u, m_entries_picked.back(), pCtx));
				assert((w >= 0.0f) && (w <= 1.0f));
				total_count = (total_count + 1.0f) * lerp(1.0f - dist_func_weight, 1.0f + dist_func_weight, w);
			}

			if (total_count <= best_count)
				continue;

			best_entry = i;
			best_count = total_count;
		}
	}

	float palette_index_reorderer::pick_side(uint32_t num_syms, uint32_t entry_to_move, pEntry_dist_func pDist_func, void *pCtx, float dist_func_weight)
	{
		float which_side = 0;

		int l_count = 0, r_count = 0;
		for (uint32_t j = 0; j < m_entries_picked.size(); j++)
		{
			const int count = get_hist(entry_to_move, m_entries_picked[j], num_syms), r = ((int)m_entries_picked.size() + 1 - 2 * (j + 1));
			which_side += static_cast<float>(r * count);
			if (r >= 0)
				l_count += r * count;
			else
				r_count += -r * count;
		}

		if (pDist_func)
		{
			float w_left = lerp(1.0f - dist_func_weight, 1.0f + dist_func_weight, (*pDist_func)(entry_to_move, m_entries_picked.front(), pCtx));
			float w_right = lerp(1.0f - dist_func_weight, 1.0f + dist_func_weight, (*pDist_func)(entry_to_move, m_entries_picked.back(), pCtx));
			which_side = w_left * l_count - w_right * r_count;
		}
		return which_side;
	}
	
	void image_metrics::calc(const imagef& a, const imagef& b, uint32_t first_chan, uint32_t total_chans, bool avg_comp_error, bool log)
	{
		assert((first_chan < 4U) && (first_chan + total_chans <= 4U));

		const uint32_t width = basisu::minimum(a.get_width(), b.get_width());
		const uint32_t height = basisu::minimum(a.get_height(), b.get_height());

		double max_e = -1e+30f;
		double sum = 0.0f, sum_sqr = 0.0f;

		m_width = width;
		m_height = height;
		
		m_has_neg = false;
		m_any_abnormal = false;
		m_hf_mag_overflow = false;
				
		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				const vec4F& ca = a(x, y), &cb = b(x, y);
								
				if (total_chans)
				{
					for (uint32_t c = 0; c < total_chans; c++)
					{
						float fa = ca[first_chan + c], fb = cb[first_chan + c];

						if ((fabs(fa) > basist::MAX_HALF_FLOAT) || (fabs(fb) > basist::MAX_HALF_FLOAT))
							m_hf_mag_overflow = true;

						if ((fa < 0.0f) || (fb < 0.0f))
							m_has_neg = true;

						if (std::isinf(fa) || std::isinf(fb) || std::isnan(fa) || std::isnan(fb))
							m_any_abnormal = true;
												
						const double delta = fabs(fa - fb);
						max_e = basisu::maximum<double>(max_e, delta);

						if (log)
						{
							double log2_delta = log2f(basisu::maximum(0.0f, fa) + 1.0f) - log2f(basisu::maximum(0.0f, fb) + 1.0f);

							sum += fabs(log2_delta);
							sum_sqr += log2_delta * log2_delta;
						}
						else
						{
							sum += fabs(delta);
							sum_sqr += delta * delta;
						}
					}
				}
				else
				{
					for (uint32_t c = 0; c < 3; c++)
					{
						float fa = ca[c], fb = cb[c];

						if ((fabs(fa) > basist::MAX_HALF_FLOAT) || (fabs(fb) > basist::MAX_HALF_FLOAT))
							m_hf_mag_overflow = true;

						if ((fa < 0.0f) || (fb < 0.0f))
							m_has_neg = true;

						if (std::isinf(fa) || std::isinf(fb) || std::isnan(fa) || std::isnan(fb))
							m_any_abnormal = true;
					}

					double ca_l = get_luminance(ca), cb_l = get_luminance(cb);
					
					double delta = fabs(ca_l - cb_l);
					max_e = basisu::maximum(max_e, delta);
					
					if (log)
					{
						double log2_delta = log2(basisu::maximum<double>(0.0f, ca_l) + 1.0f) - log2(basisu::maximum<double>(0.0f, cb_l) + 1.0f);

						sum += fabs(log2_delta);
						sum_sqr += log2_delta * log2_delta;
					}
					else
					{
						sum += delta;
						sum_sqr += delta * delta;
					}
				}
			}
		}

		m_max = (double)(max_e);

		double total_values = (double)width * (double)height;
		if (avg_comp_error)
			total_values *= (double)clamp<uint32_t>(total_chans, 1, 4);

		m_mean = (float)(sum / total_values);
		m_mean_squared = (float)(sum_sqr / total_values);
		m_rms = (float)sqrt(sum_sqr / total_values);
		
		const double max_val = 1.0f;
		m_psnr = m_rms ? (float)clamp<double>(log10(max_val / m_rms) * 20.0f, 0.0f, 1000.0f) : 1000.0f;
	}

	void image_metrics::calc_half(const imagef& a, const imagef& b, uint32_t first_chan, uint32_t total_chans, bool avg_comp_error)
	{
		assert(total_chans);
		assert((first_chan < 4U) && (first_chan + total_chans <= 4U));

		const uint32_t width = basisu::minimum(a.get_width(), b.get_width());
		const uint32_t height = basisu::minimum(a.get_height(), b.get_height());

		m_width = width;
		m_height = height;

		m_has_neg = false;
		m_hf_mag_overflow = false;
		m_any_abnormal = false;

		uint_vec hist(65536);
		
		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				const vec4F& ca = a(x, y), &cb = b(x, y);

				for (uint32_t i = 0; i < 4; i++)
				{
					if ((ca[i] < 0.0f) || (cb[i] < 0.0f))
						m_has_neg = true;
					
					if ((fabs(ca[i]) > basist::MAX_HALF_FLOAT) || (fabs(cb[i]) > basist::MAX_HALF_FLOAT))
						m_hf_mag_overflow = true;

					if (std::isnan(ca[i]) || std::isnan(cb[i]) || std::isinf(ca[i]) || std::isinf(cb[i]))
						m_any_abnormal = true;
				}

				int cah[4] = { basist::float_to_half(ca[0]), basist::float_to_half(ca[1]), basist::float_to_half(ca[2]), basist::float_to_half(ca[3]) };
				int cbh[4] = { basist::float_to_half(cb[0]), basist::float_to_half(cb[1]), basist::float_to_half(cb[2]), basist::float_to_half(cb[3]) };

				for (uint32_t c = 0; c < total_chans; c++)
					hist[iabs(cah[first_chan + c] - cbh[first_chan + c]) & 65535]++;

			} // x
		} // y

		m_max = 0;
		double sum = 0.0f, sum2 = 0.0f;
		for (uint32_t i = 0; i < 65536; i++)
		{
			if (hist[i])
			{
				m_max = basisu::maximum<double>(m_max, (double)i);
				double v = (double)i * (double)hist[i];
				sum += v;
				sum2 += (double)i * v;
			}
		}

		double total_values = (double)width * (double)height;
		if (avg_comp_error)
			total_values *= (double)clamp<uint32_t>(total_chans, 1, 4);

		const float max_val = 65535.0f;
		m_mean = (float)clamp<double>(sum / total_values, 0.0f, max_val);
		m_mean_squared = (float)clamp<double>(sum2 / total_values, 0.0f, max_val * max_val);
		m_rms = (float)sqrt(m_mean_squared);
		m_psnr = m_rms ? (float)clamp<double>(log10(max_val / m_rms) * 20.0f, 0.0f, 1000.0f) : 1000.0f;
	}

	// Alt. variant, same as calc_half(), for validation.
	void image_metrics::calc_half2(const imagef& a, const imagef& b, uint32_t first_chan, uint32_t total_chans, bool avg_comp_error)
	{
		assert(total_chans);
		assert((first_chan < 4U) && (first_chan + total_chans <= 4U));

		const uint32_t width = basisu::minimum(a.get_width(), b.get_width());
		const uint32_t height = basisu::minimum(a.get_height(), b.get_height());

		m_width = width;
		m_height = height;

		m_has_neg = false;
		m_hf_mag_overflow = false;
		m_any_abnormal = false;
				
		double sum = 0.0f, sum2 = 0.0f;
		m_max = 0;

		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				const vec4F& ca = a(x, y), & cb = b(x, y);

				for (uint32_t i = 0; i < 4; i++)
				{
					if ((ca[i] < 0.0f) || (cb[i] < 0.0f))
						m_has_neg = true;

					if ((fabs(ca[i]) > basist::MAX_HALF_FLOAT) || (fabs(cb[i]) > basist::MAX_HALF_FLOAT))
						m_hf_mag_overflow = true;

					if (std::isnan(ca[i]) || std::isnan(cb[i]) || std::isinf(ca[i]) || std::isinf(cb[i]))
						m_any_abnormal = true;
				}

				int cah[4] = { basist::float_to_half(ca[0]), basist::float_to_half(ca[1]), basist::float_to_half(ca[2]), basist::float_to_half(ca[3]) };
				int cbh[4] = { basist::float_to_half(cb[0]), basist::float_to_half(cb[1]), basist::float_to_half(cb[2]), basist::float_to_half(cb[3]) };

				for (uint32_t c = 0; c < total_chans; c++)
				{
					int diff = iabs(cah[first_chan + c] - cbh[first_chan + c]);
					if (diff)
						m_max = std::max<double>(m_max, (double)diff);

					sum += diff;
					sum2 += squarei(cah[first_chan + c] - cbh[first_chan + c]);
				}

			} // x
		} // y
						
		double total_values = (double)width * (double)height;
		if (avg_comp_error)
			total_values *= (double)clamp<uint32_t>(total_chans, 1, 4);

		const float max_val = 65535.0f;
		m_mean = (float)clamp<double>(sum / total_values, 0.0f, max_val);
		m_mean_squared = (float)clamp<double>(sum2 / total_values, 0.0f, max_val * max_val);
		m_rms = (float)sqrt(m_mean_squared);
		m_psnr = m_rms ? (float)clamp<double>(log10(max_val / m_rms) * 20.0f, 0.0f, 1000.0f) : 1000.0f;
	}

	void image_metrics::calc(const image &a, const image &b, uint32_t first_chan, uint32_t total_chans, bool avg_comp_error, bool use_601_luma)
	{
		assert((first_chan < 4U) && (first_chan + total_chans <= 4U));

		const uint32_t width = basisu::minimum(a.get_width(), b.get_width());
		const uint32_t height = basisu::minimum(a.get_height(), b.get_height());

		m_width = width;
		m_height = height;

		double hist[256];
		clear_obj(hist);

		m_has_neg = false;
		m_any_abnormal = false;
		m_hf_mag_overflow = false;
		m_sum_a = 0;
		m_sum_b = 0;

		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				const color_rgba &ca = a(x, y), &cb = b(x, y);

				if (total_chans)
				{
					for (uint32_t c = 0; c < total_chans; c++)
					{
						hist[iabs(ca[first_chan + c] - cb[first_chan + c])]++;
						m_sum_a += ca[first_chan + c];
						m_sum_b += cb[first_chan + c];
					}
				}
				else
				{
					if (use_601_luma)
						hist[iabs(ca.get_601_luma() - cb.get_601_luma())]++;
					else
						hist[iabs(ca.get_709_luma() - cb.get_709_luma())]++;

					for (uint32_t c = 0; c < 3; c++)
					{
						m_sum_a += ca[c];
						m_sum_b += cb[c];
					}
				}
			}
		}

		m_max = 0;
		double sum = 0.0f, sum2 = 0.0f;
		for (uint32_t i = 0; i < 256; i++)
		{
			if (hist[i])
			{
				m_max = basisu::maximum<double>(m_max, (double)i);
				double v = i * hist[i];
				sum += v;
				sum2 += i * v;
			}
		}

		double total_values = (double)width * (double)height;
		if (avg_comp_error)
			total_values *= (double)clamp<uint32_t>(total_chans, 1, 4);

		m_mean = (float)clamp<double>(sum / total_values, 0.0f, 255.0);
		m_mean_squared = (float)clamp<double>(sum2 / total_values, 0.0f, 255.0f * 255.0f);
		m_rms = (float)sqrt(m_mean_squared);
		m_psnr = m_rms ? (float)clamp<double>(log10(255.0 / m_rms) * 20.0f, 0.0f, 100.0f) : 100.0f;
	}

	void print_image_metrics(const image& a, const image& b)
	{
		image_metrics im;
		im.calc(a, b, 0, 3);
		im.print("RGB    ");

		im.calc(a, b, 0, 4);
		im.print("RGBA   ");

		im.calc(a, b, 0, 1);
		im.print("R      ");

		im.calc(a, b, 1, 1);
		im.print("G      ");

		im.calc(a, b, 2, 1);
		im.print("B      ");

		im.calc(a, b, 3, 1);
		im.print("A      ");

		im.calc(a, b, 0, 0);
		im.print("Y 709  ");

		im.calc(a, b, 0, 0, true, true);
		im.print("Y 601  ");
	}

	void fill_buffer_with_random_bytes(void *pBuf, size_t size, uint32_t seed)
	{
		rand r(seed);

		uint8_t *pDst = static_cast<uint8_t *>(pBuf);

		while (size >= sizeof(uint32_t))
		{
			*(uint32_t *)pDst = r.urand32();
			pDst += sizeof(uint32_t);
			size -= sizeof(uint32_t);
		}

		while (size)
		{
			*pDst++ = r.byte();
			size--;
		}
	}

	job_pool::job_pool(uint32_t num_threads) : 
		m_num_active_jobs(0)
	{
		m_kill_flag.store(false);
		m_num_active_workers.store(0);

		assert(num_threads >= 1U);

		debug_printf("job_pool::job_pool: %u total threads\n", num_threads);

		if (num_threads > 1)
		{
			m_threads.resize(num_threads - 1);

			for (int i = 0; i < ((int)num_threads - 1); i++)
			   m_threads[i] = std::thread([this, i] { job_thread(i); });
		}
	}

	job_pool::~job_pool()
	{
		debug_printf("job_pool::~job_pool\n");
		
		// Notify all workers that they need to die right now.
		{
			std::lock_guard<std::mutex> lk(m_mutex);
			m_kill_flag.store(true);
		}
		
		m_has_work.notify_all();

#ifdef __EMSCRIPTEN__
		for ( ; ; )
		{
			if (m_num_active_workers.load() <= 0)
				break;
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		
		// At this point all worker threads should be exiting or exited.
		// We could call detach(), but this seems to just call join() anyway.
#endif

		// Wait for all worker threads to exit.
		for (uint32_t i = 0; i < m_threads.size(); i++)
			m_threads[i].join();
	}
				
	void job_pool::add_job(const std::function<void()>& job)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		m_queue.emplace_back(job);

		const size_t queue_size = m_queue.size();

		lock.unlock();

		if (queue_size > 1)
			m_has_work.notify_one();
	}

	void job_pool::add_job(std::function<void()>&& job)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		m_queue.emplace_back(std::move(job));
						
		const size_t queue_size = m_queue.size();

		lock.unlock();

		if (queue_size > 1)
		{
			m_has_work.notify_one();
		}
	}

	void job_pool::wait_for_all()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		// Drain the job queue on the calling thread.
		while (!m_queue.empty())
		{
			std::function<void()> job(m_queue.back());
			m_queue.pop_back();

			lock.unlock();

			job();

			lock.lock();
		}

		// The queue is empty, now wait for all active jobs to finish up.
#ifndef __EMSCRIPTEN__
		m_no_more_jobs.wait(lock, [this]{ return !m_num_active_jobs; } );
#else
		// Avoid infinite blocking
		for (; ; )
		{
			if (m_no_more_jobs.wait_for(lock, std::chrono::milliseconds(50), [this] { return !m_num_active_jobs; }))
			{
				break;
			}
		}
#endif
	}

	void job_pool::job_thread(uint32_t index)
	{
		BASISU_NOTE_UNUSED(index);
		//debug_printf("job_pool::job_thread: starting %u\n", index);

		m_num_active_workers.fetch_add(1);
		
		while (!m_kill_flag)
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			// Wait for any jobs to be issued.
#if 0
			m_has_work.wait(lock, [this] { return m_kill_flag || m_queue.size(); } );
#else
			// For more safety vs. buggy RTL's. Worse case we stall for a second vs. locking up forever if something goes wrong.
			m_has_work.wait_for(lock, std::chrono::milliseconds(1000), [this] {
				return m_kill_flag || !m_queue.empty();
				});
#endif

			// Check to see if we're supposed to exit.
			if (m_kill_flag)
				break;

			if (m_queue.empty())
				continue;

			// Get the job and execute it.
			std::function<void()> job(m_queue.back());
			m_queue.pop_back();

			++m_num_active_jobs;

			lock.unlock();

			job();

			lock.lock();

			--m_num_active_jobs;

			// Now check if there are no more jobs remaining. 
			const bool all_done = m_queue.empty() && !m_num_active_jobs;
			
			lock.unlock();

			if (all_done)
				m_no_more_jobs.notify_all();
		}

		m_num_active_workers.fetch_add(-1);

		//debug_printf("job_pool::job_thread: exiting\n");
	}

	// .TGA image loading
	#pragma pack(push)
	#pragma pack(1)
	struct tga_header
	{
		uint8_t			m_id_len;
		uint8_t			m_cmap;
		uint8_t			m_type;
		packed_uint<2>	m_cmap_first;
		packed_uint<2> m_cmap_len;
		uint8_t			m_cmap_bpp;
		packed_uint<2> m_x_org;
		packed_uint<2> m_y_org;
		packed_uint<2> m_width;
		packed_uint<2> m_height;
		uint8_t			m_depth;
		uint8_t			m_desc;
	};
	#pragma pack(pop)

	const uint32_t MAX_TGA_IMAGE_SIZE = 16384;

	enum tga_image_type
	{
		cITPalettized = 1,
		cITRGB = 2,
		cITGrayscale = 3
	};

	uint8_t *read_tga(const uint8_t *pBuf, uint32_t buf_size, int &width, int &height, int &n_chans)
	{
		width = 0;
		height = 0;
		n_chans = 0;

		if (buf_size <= sizeof(tga_header))
			return nullptr;

		const tga_header &hdr = *reinterpret_cast<const tga_header *>(pBuf);

		if ((!hdr.m_width) || (!hdr.m_height) || (hdr.m_width > MAX_TGA_IMAGE_SIZE) || (hdr.m_height > MAX_TGA_IMAGE_SIZE))
			return nullptr;

		if (hdr.m_desc >> 6)
			return nullptr;

		// Simple validation
		if ((hdr.m_cmap != 0) && (hdr.m_cmap != 1))
			return nullptr;
		
		if (hdr.m_cmap)
		{
			if ((hdr.m_cmap_bpp == 0) || (hdr.m_cmap_bpp > 32))
				return nullptr;

			// Nobody implements CMapFirst correctly, so we're not supporting it. Never seen it used, either.
			if (hdr.m_cmap_first != 0)
				return nullptr;
		}

		const bool x_flipped = (hdr.m_desc & 0x10) != 0;
		const bool y_flipped = (hdr.m_desc & 0x20) == 0;

		bool rle_flag = false;
		int file_image_type = hdr.m_type;
		if (file_image_type > 8)
		{
			file_image_type -= 8;
			rle_flag = true;
		}

		const tga_image_type image_type = static_cast<tga_image_type>(file_image_type);

		switch (file_image_type)
		{
		case cITRGB:
			if (hdr.m_depth == 8)
				return nullptr;
			break;
		case cITPalettized:
			if ((hdr.m_depth != 8) || (hdr.m_cmap != 1) || (hdr.m_cmap_len == 0))
				return nullptr;
			break;
		case cITGrayscale:
			if ((hdr.m_cmap != 0) || (hdr.m_cmap_len != 0))
				return nullptr;
			if ((hdr.m_depth != 8) && (hdr.m_depth != 16))
				return nullptr;
			break;
		default:
			return nullptr;
		}

		uint32_t tga_bytes_per_pixel = 0;

		switch (hdr.m_depth)
		{
		case 32:
			tga_bytes_per_pixel = 4;
			n_chans = 4;
			break;
		case 24:
			tga_bytes_per_pixel = 3;
			n_chans = 3;
			break;
		case 16:
		case 15:
			tga_bytes_per_pixel = 2;
			// For compatibility with stb_image_write.h
			n_chans = ((file_image_type == cITGrayscale) && (hdr.m_depth == 16)) ? 4 : 3;
			break;
		case 8:
			tga_bytes_per_pixel = 1;
			// For palettized RGBA support, which both FreeImage and stb_image support.
			n_chans = ((file_image_type == cITPalettized) && (hdr.m_cmap_bpp == 32)) ? 4 : 3;
			break;
		default:
			return nullptr;
		}

		//const uint32_t bytes_per_line = hdr.m_width * tga_bytes_per_pixel;

		const uint8_t *pSrc = pBuf + sizeof(tga_header);
		uint32_t bytes_remaining = buf_size - sizeof(tga_header);

		if (hdr.m_id_len)
		{
			if (bytes_remaining < hdr.m_id_len)
				return nullptr;
			pSrc += hdr.m_id_len;
			bytes_remaining += hdr.m_id_len;
		}

		color_rgba pal[256];
		for (uint32_t i = 0; i < 256; i++)
			pal[i].set(0, 0, 0, 255);

		if ((hdr.m_cmap) && (hdr.m_cmap_len))
		{
			if (image_type == cITPalettized)
			{
				// Note I cannot find any files using 32bpp palettes in the wild (never seen any in ~30 years).
				if ( ((hdr.m_cmap_bpp != 32) && (hdr.m_cmap_bpp != 24) && (hdr.m_cmap_bpp != 15) && (hdr.m_cmap_bpp != 16)) || (hdr.m_cmap_len > 256) )
					return nullptr;

				if (hdr.m_cmap_bpp == 32)
				{
					const uint32_t pal_size = hdr.m_cmap_len * 4;
					if (bytes_remaining < pal_size)
						return nullptr;

					for (uint32_t i = 0; i < hdr.m_cmap_len; i++)
					{
						pal[i].r = pSrc[i * 4 + 2];
						pal[i].g = pSrc[i * 4 + 1];
						pal[i].b = pSrc[i * 4 + 0];
						pal[i].a = pSrc[i * 4 + 3];
					}

					bytes_remaining -= pal_size;
					pSrc += pal_size;
				}
				else if (hdr.m_cmap_bpp == 24)
				{
					const uint32_t pal_size = hdr.m_cmap_len * 3;
					if (bytes_remaining < pal_size)
						return nullptr;

					for (uint32_t i = 0; i < hdr.m_cmap_len; i++)
					{
						pal[i].r = pSrc[i * 3 + 2];
						pal[i].g = pSrc[i * 3 + 1];
						pal[i].b = pSrc[i * 3 + 0];
						pal[i].a = 255;
					}

					bytes_remaining -= pal_size;
					pSrc += pal_size;
				}
				else
				{
					const uint32_t pal_size = hdr.m_cmap_len * 2;
					if (bytes_remaining < pal_size)
						return nullptr;

					for (uint32_t i = 0; i < hdr.m_cmap_len; i++)
					{
						const uint32_t v = pSrc[i * 2 + 0] | (pSrc[i * 2 + 1] << 8);

						pal[i].r = (((v >> 10) & 31) * 255 + 15) / 31;
						pal[i].g = (((v >> 5) & 31) * 255 + 15) / 31;
						pal[i].b = ((v & 31) * 255 + 15) / 31;
						pal[i].a = 255;
					}

					bytes_remaining -= pal_size;
					pSrc += pal_size;
				}
			}
			else
			{
				const uint32_t bytes_to_skip = (hdr.m_cmap_bpp >> 3) * hdr.m_cmap_len;
				if (bytes_remaining < bytes_to_skip)
					return nullptr;
				pSrc += bytes_to_skip;
				bytes_remaining += bytes_to_skip;
			}
		}
		
		width = hdr.m_width;
		height = hdr.m_height;

		const uint32_t source_pitch = width * tga_bytes_per_pixel;
		const uint32_t dest_pitch = width * n_chans;
		
		uint8_t *pImage = (uint8_t *)malloc(dest_pitch * height);
		if (!pImage)
			return nullptr;

		std::vector<uint8_t> input_line_buf;
		if (rle_flag)
			input_line_buf.resize(source_pitch);

		int run_type = 0, run_remaining = 0;
		uint8_t run_pixel[4];
		memset(run_pixel, 0, sizeof(run_pixel));

		for (int y = 0; y < height; y++)
		{
			const uint8_t *pLine_data;

			if (rle_flag)
			{
				int pixels_remaining = width;
				uint8_t *pDst = &input_line_buf[0];

				do 
				{
					if (!run_remaining)
					{
						if (bytes_remaining < 1)
						{
							free(pImage);
							return nullptr;
						}

						int v = *pSrc++;
						bytes_remaining--;

						run_type = v & 0x80;
						run_remaining = (v & 0x7F) + 1;

						if (run_type)
						{
							if (bytes_remaining < tga_bytes_per_pixel)
							{
								free(pImage);
								return nullptr;
							}

							memcpy(run_pixel, pSrc, tga_bytes_per_pixel);
							pSrc += tga_bytes_per_pixel;
							bytes_remaining -= tga_bytes_per_pixel;
						}
					}

					const uint32_t n = basisu::minimum<uint32_t>(pixels_remaining, run_remaining);
					pixels_remaining -= n;
					run_remaining -= n;

					if (run_type)
					{
						for (uint32_t i = 0; i < n; i++)
							for (uint32_t j = 0; j < tga_bytes_per_pixel; j++)
								*pDst++ = run_pixel[j];
					}
					else
					{
						const uint32_t bytes_wanted = n * tga_bytes_per_pixel;

						if (bytes_remaining < bytes_wanted)
						{
							free(pImage);
							return nullptr;
						}

						memcpy(pDst, pSrc, bytes_wanted);
						pDst += bytes_wanted;

						pSrc += bytes_wanted;
						bytes_remaining -= bytes_wanted;
					}

				} while (pixels_remaining);

				assert((pDst - &input_line_buf[0]) == (int)(width * tga_bytes_per_pixel));

				pLine_data = &input_line_buf[0];
			}
			else
			{
				if (bytes_remaining < source_pitch)
				{
					free(pImage);
					return nullptr;
				}

				pLine_data = pSrc;
				bytes_remaining -= source_pitch;
				pSrc += source_pitch;
			}

			// Convert to 24bpp RGB or 32bpp RGBA.
			uint8_t *pDst = pImage + (y_flipped ? (height - 1 - y) : y) * dest_pitch + (x_flipped ? (width - 1) * n_chans : 0);
			const int dst_stride = x_flipped ? -((int)n_chans) : n_chans;

			switch (hdr.m_depth)
			{
			case 32:
				assert(tga_bytes_per_pixel == 4 && n_chans == 4);
				for (int i = 0; i < width; i++, pLine_data += 4, pDst += dst_stride)
				{
					pDst[0] = pLine_data[2];
					pDst[1] = pLine_data[1];
					pDst[2] = pLine_data[0];
					pDst[3] = pLine_data[3];
				}
				break;
			case 24:
				assert(tga_bytes_per_pixel == 3 && n_chans == 3);
				for (int i = 0; i < width; i++, pLine_data += 3, pDst += dst_stride)
				{
					pDst[0] = pLine_data[2];
					pDst[1] = pLine_data[1];
					pDst[2] = pLine_data[0];
				}
				break;
			case 16:
			case 15:
				if (image_type == cITRGB)
				{
					assert(tga_bytes_per_pixel == 2 && n_chans == 3);
					for (int i = 0; i < width; i++, pLine_data += 2, pDst += dst_stride)
					{
						const uint32_t v = pLine_data[0] | (pLine_data[1] << 8);
						pDst[0] = (((v >> 10) & 31) * 255 + 15) / 31;
						pDst[1] = (((v >> 5) & 31) * 255 + 15) / 31;
						pDst[2] = ((v & 31) * 255 + 15) / 31;
					}
				}
				else
				{
					assert(image_type == cITGrayscale && tga_bytes_per_pixel == 2 && n_chans == 4);
					for (int i = 0; i < width; i++, pLine_data += 2, pDst += dst_stride)
					{
						pDst[0] = pLine_data[0];
						pDst[1] = pLine_data[0];
						pDst[2] = pLine_data[0];
						pDst[3] = pLine_data[1];
					}
				}
				break;
			case 8:
				assert(tga_bytes_per_pixel == 1);
				if (image_type == cITPalettized)
				{
					if (hdr.m_cmap_bpp == 32)
					{
						assert(n_chans == 4);
						for (int i = 0; i < width; i++, pLine_data++, pDst += dst_stride)
						{
							const uint32_t c = *pLine_data;
							pDst[0] = pal[c].r;
							pDst[1] = pal[c].g;
							pDst[2] = pal[c].b;
							pDst[3] = pal[c].a;
						}
					}
					else
					{
						assert(n_chans == 3);
						for (int i = 0; i < width; i++, pLine_data++, pDst += dst_stride)
						{
							const uint32_t c = *pLine_data;
							pDst[0] = pal[c].r;
							pDst[1] = pal[c].g;
							pDst[2] = pal[c].b;
						}
					}
				}
				else
				{
					assert(n_chans == 3);
					for (int i = 0; i < width; i++, pLine_data++, pDst += dst_stride)
					{
						const uint8_t c = *pLine_data;
						pDst[0] = c;
						pDst[1] = c;
						pDst[2] = c;
					}
				}
				break;
			default:
				assert(0);
				break;
			}
		} // y

		return pImage;
	}

	uint8_t *read_tga(const char *pFilename, int &width, int &height, int &n_chans)
	{
		width = height = n_chans = 0;

		uint8_vec filedata;
		if (!read_file_to_vec(pFilename, filedata))
			return nullptr;

		if (!filedata.size() || (filedata.size() > UINT32_MAX))
			return nullptr;
		
		return read_tga(&filedata[0], (uint32_t)filedata.size(), width, height, n_chans);
	}

	static inline void hdr_convert(const color_rgba& rgbe, vec4F& c)
	{
		if (rgbe[3] != 0)
		{
			float scale = ldexp(1.0f, rgbe[3] - 128 - 8);
			c.set((float)rgbe[0] * scale, (float)rgbe[1] * scale, (float)rgbe[2] * scale, 1.0f);
		}
		else
		{
			c.set(0.0f, 0.0f, 0.0f, 1.0f);
		}
	}

	bool string_begins_with(const std::string& str, const char* pPhrase)
	{
		const size_t str_len = str.size();

		const size_t phrase_len = strlen(pPhrase);
		assert(phrase_len);

		if (str_len >= phrase_len)
		{
#ifdef _MSC_VER
			if (_strnicmp(pPhrase, str.c_str(), phrase_len) == 0)
#else
			if (strncasecmp(pPhrase, str.c_str(), phrase_len) == 0)
#endif
				return true;
		}

		return false;
	}

	// Radiance RGBE (.HDR) image reading.
	// This code tries to preserve the original logic in Radiance's ray/src/common/color.c code:
	// https://www.radiance-online.org/cgi-bin/viewcvs.cgi/ray/src/common/color.c?revision=2.26&view=markup&sortby=log
	// Also see: https://flipcode.com/archives/HDR_Image_Reader.shtml.
	// https://github.com/LuminanceHDR/LuminanceHDR/blob/master/src/Libpfs/io/rgbereader.cpp.
	// https://radsite.lbl.gov/radiance/refer/filefmts.pdf
	// Buggy readers:
	// stb_image.h: appears to be a clone of rgbe.c, but with goto's (doesn't support old format files, doesn't support mixture of RLE/non-RLE scanlines)
	// http://www.graphics.cornell.edu/~bjw/rgbe.html - rgbe.c/h
	// http://www.graphics.cornell.edu/online/formats/rgbe/ - rgbe.c/.h - buggy
	bool read_rgbe(const uint8_vec &filedata, imagef& img, rgbe_header_info& hdr_info)
	{
		hdr_info.clear();

		const uint32_t MAX_SUPPORTED_DIM = 65536;

		if (filedata.size() < 4)
			return false;

		// stb_image.h checks for the string "#?RADIANCE" or "#?RGBE" in the header.
		// The original Radiance header code doesn't care about the specific string.
		// opencv's reader only checks for "#?", so that's what we're going to do.
		if ((filedata[0] != '#') || (filedata[1] != '?'))
			return false;

		//uint32_t width = 0, height = 0;
		bool is_rgbe = false;
		size_t cur_ofs = 0;

		// Parse the lines until we encounter a blank line.
		std::string cur_line;
		for (; ; )
		{
			if (cur_ofs >= filedata.size())
				return false;

			const uint32_t HEADER_TOO_BIG_SIZE = 4096;
			if (cur_ofs >= HEADER_TOO_BIG_SIZE)
			{
				// Header seems too large - something is likely wrong. Return failure.
				return false;
			}

			uint8_t c = filedata[cur_ofs++];

			if (c == '\n')
			{
				if (!cur_line.size())
					break;

				if ((cur_line[0] == '#') && (!string_begins_with(cur_line, "#?")) && (!hdr_info.m_program.size()))
				{
					cur_line.erase(0, 1);
					while (cur_line.size() && (cur_line[0] == ' '))
						cur_line.erase(0, 1);

					hdr_info.m_program = cur_line;
				}
				else if (string_begins_with(cur_line, "EXPOSURE=") && (cur_line.size() > 9))
				{
					hdr_info.m_exposure = atof(cur_line.c_str() + 9);
					hdr_info.m_has_exposure = true;
				}
				else if (string_begins_with(cur_line, "GAMMA=") && (cur_line.size() > 6))
				{
					hdr_info.m_exposure = atof(cur_line.c_str() + 6);
					hdr_info.m_has_gamma = true;
				}
				else if (cur_line == "FORMAT=32-bit_rle_rgbe")
				{
					is_rgbe = true;
				}

				cur_line.resize(0);
			}
			else
				cur_line.push_back((char)c);
		}

		if (!is_rgbe)
			return false;

		// Assume and require the final line to have the image's dimensions. We're not supporting flipping.
		for (; ; )
		{
			if (cur_ofs >= filedata.size())
				return false;
			uint8_t c = filedata[cur_ofs++];
			if (c == '\n')
				break;
			cur_line.push_back((char)c);
		}

		int comp[2] = { 1, 0 }; // y, x (major, minor)
		int dir[2] = { -1, 1 }; // -1, 1, (major, minor), for y -1=up
		uint32_t major_dim = 0, minor_dim = 0;

		// Parse the dimension string, normally it'll be "-Y # +X #" (major, minor), rarely it differs
		for (uint32_t d = 0; d < 2; d++) // 0=major, 1=minor
		{
			const bool is_neg_x = (strncmp(&cur_line[0], "-X ", 3) == 0);
			const bool is_pos_x = (strncmp(&cur_line[0], "+X ", 3) == 0);
			const bool is_x = is_neg_x || is_pos_x;

			const bool is_neg_y = (strncmp(&cur_line[0], "-Y ", 3) == 0);
			const bool is_pos_y = (strncmp(&cur_line[0], "+Y ", 3) == 0);
			const bool is_y = is_neg_y || is_pos_y;

			if (cur_line.size() < 3)
				return false;
			
			if (!is_x && !is_y)
				return false;

			comp[d] = is_x ? 0 : 1;
			dir[d] = (is_neg_x || is_neg_y) ? -1 : 1;
			
			uint32_t& dim = d ? minor_dim : major_dim;

			cur_line.erase(0, 3);

			while (cur_line.size())
			{
				char c = cur_line[0];
				if (c != ' ')
					break;
				cur_line.erase(0, 1);
			}

			bool has_digits = false;
			while (cur_line.size())
			{
				char c = cur_line[0];
				cur_line.erase(0, 1);

				if (c == ' ')
					break;

				if ((c < '0') || (c > '9'))
					return false;

				const uint32_t prev_dim = dim;
				dim = dim * 10 + (c - '0');
				if (dim < prev_dim)
					return false;

				has_digits = true;
			}
			if (!has_digits)
				return false;

			if ((dim < 1) || (dim > MAX_SUPPORTED_DIM))
				return false;
		}
				
		// temp image: width=minor, height=major
		img.resize(minor_dim, major_dim);

		std::vector<color_rgba> temp_scanline(minor_dim);

		// Read the scanlines.
		for (uint32_t y = 0; y < major_dim; y++)
		{
			vec4F* pDst = &img(0, y);

			if ((filedata.size() - cur_ofs) < 4)
				return false;

			// Determine if the line uses the new or old format. See the logic in color.c.
			bool old_decrunch = false;
			if ((minor_dim < 8) || (minor_dim > 0x7FFF))
			{
				// Line is too short or long; must be old format.
				old_decrunch = true;
			}
			else if (filedata[cur_ofs] != 2)
			{
				// R is not 2, must be old format
				old_decrunch = true;
			}
			else
			{
				// c[0]/red is 2.Check GB and E for validity.				
				color_rgba c;
				memcpy(&c, &filedata[cur_ofs], 4);

				if ((c[1] != 2) || (c[2] & 0x80))
				{
					// G isn't 2, or the high bit of B is set which is impossible (image's > 0x7FFF pixels can't get here). Use old format.
					old_decrunch = true;
				}
				else
				{
					// Check B and E. If this isn't the minor_dim in network order, something is wrong. The pixel would also be denormalized, and invalid.
					uint32_t w = (c[2] << 8) | c[3];
					if (w != minor_dim)
						return false;

					cur_ofs += 4;
				}
			}

			if (old_decrunch)
			{
				uint32_t rshift = 0, x = 0;

				while (x < minor_dim)
				{
					if ((filedata.size() - cur_ofs) < 4)
						return false;

					color_rgba c;
					memcpy(&c, &filedata[cur_ofs], 4);
					cur_ofs += 4;

					if ((c[0] == 1) && (c[1] == 1) && (c[2] == 1))
					{
						// We'll allow RLE matches to cross scanlines, but not on the very first pixel.
						if ((!x) && (!y))
							return false;

						const uint32_t run_len = c[3] << rshift;
						const vec4F run_color(pDst[-1]);

						if ((x + run_len) > minor_dim)
							return false;

						for (uint32_t i = 0; i < run_len; i++)
							*pDst++ = run_color;

						rshift += 8;
						x += run_len;
					}
					else
					{
						rshift = 0;

						hdr_convert(c, *pDst);
						pDst++;
						x++;
					}
				}
				continue;
			}

			// New format
			for (uint32_t s = 0; s < 4; s++)
			{
				uint32_t x_ofs = 0;
				while (x_ofs < minor_dim)
				{
					uint32_t num_remaining = minor_dim - x_ofs;

					if (cur_ofs >= filedata.size())
						return false;

					uint8_t count = filedata[cur_ofs++];
					if (count > 128)
					{
						count -= 128;
						if (count > num_remaining)
							return false;

						if (cur_ofs >= filedata.size())
							return false;
						const uint8_t val = filedata[cur_ofs++];

						for (uint32_t i = 0; i < count; i++)
							temp_scanline[x_ofs + i][s] = val;

						x_ofs += count;
					}
					else
					{
						if ((!count) || (count > num_remaining))
							return false;

						for (uint32_t i = 0; i < count; i++)
						{
							if (cur_ofs >= filedata.size())
								return false;
							const uint8_t val = filedata[cur_ofs++];

							temp_scanline[x_ofs + i][s] = val;
						}

						x_ofs += count;
					}
				} // while (x_ofs < minor_dim)
			} // c

			// Convert all the RGBE pixels to float now
			for (uint32_t x = 0; x < minor_dim; x++, pDst++)
				hdr_convert(temp_scanline[x], *pDst);

			assert((pDst - &img(0, y)) == (int)minor_dim);

		} // y

		// at here:
		// img(width,height)=image pixels as read from file, x=minor axis, y=major axis
		// width=minor axis dimension
		// height=major axis dimension
		// in file, pixels are emitted in minor order, them major (so major=scanlines in the file)
		
		imagef final_img;
		if (comp[0] == 0) // if major axis is X
			final_img.resize(major_dim, minor_dim);
		else // major axis is Y, minor is X
			final_img.resize(minor_dim, major_dim);

		// TODO: optimize the identity case
		for (uint32_t major_iter = 0; major_iter < major_dim; major_iter++)
		{
			for (uint32_t minor_iter = 0; minor_iter < minor_dim; minor_iter++)
			{
				const vec4F& p = img(minor_iter, major_iter);

				uint32_t dst_x = 0, dst_y = 0;

				// is the minor dim output x?
				if (comp[1] == 0) 
				{
					// minor axis is x, major is y
					
					// is minor axis (which is output x) flipped?
					if (dir[1] < 0)
						dst_x = minor_dim - 1 - minor_iter;
					else
						dst_x = minor_iter;

					// is major axis (which is output y) flipped? -1=down in raster order, 1=up
					if (dir[0] < 0)
						dst_y = major_iter;
					else
						dst_y = major_dim - 1 - major_iter;
				}
				else
				{
					// minor axis is output y, major is output x

					// is minor axis (which is output y) flipped?
					if (dir[1] < 0)
						dst_y = minor_iter;
					else
						dst_y = minor_dim - 1 - minor_iter;

					// is major axis (which is output x) flipped?
					if (dir[0] < 0)
						dst_x = major_dim - 1 - major_iter;
					else
						dst_x = major_iter;
				}

				final_img(dst_x, dst_y) = p;
			}
		}

		final_img.swap(img);

		return true;
	}

	bool read_rgbe(const char* pFilename, imagef& img, rgbe_header_info& hdr_info)
	{
		uint8_vec filedata;
		if (!read_file_to_vec(pFilename, filedata))
			return false;
		return read_rgbe(filedata, img, hdr_info);
	}

	static uint8_vec& append_string(uint8_vec& buf, const char* pStr)
	{
		const size_t str_len = strlen(pStr);
		if (!str_len)
			return buf;

		const size_t ofs = buf.size();
		buf.resize(ofs + str_len);
		memcpy(&buf[ofs], pStr, str_len);

		return buf;
	}
	
	static uint8_vec& append_string(uint8_vec& buf, const std::string& str)
	{
		if (!str.size())
			return buf;
		return append_string(buf, str.c_str());
	}

	static inline void float2rgbe(color_rgba &rgbe, const vec4F &c)
	{
		const float red = c[0], green = c[1], blue = c[2];
		assert(red >= 0.0f && green >= 0.0f && blue >= 0.0f);

		const float max_v = basisu::maximumf(basisu::maximumf(red, green), blue);

		if (max_v < 1e-32f)
			rgbe.clear();
		else 
		{
			int e;
			const float scale = frexp(max_v, &e) * 256.0f / max_v;
			rgbe[0] = (uint8_t)(clamp<int>((int)(red * scale), 0, 255));
			rgbe[1] = (uint8_t)(clamp<int>((int)(green * scale), 0, 255));
			rgbe[2] = (uint8_t)(clamp<int>((int)(blue * scale), 0, 255));
			rgbe[3] = (uint8_t)(e + 128);
		}
	}

	const bool RGBE_FORCE_RAW = false;
	const bool RGBE_FORCE_OLD_CRUNCH = false; // note must readers (particularly stb_image.h's) don't properly support this, when they should
		
	bool write_rgbe(uint8_vec &file_data, imagef& img, rgbe_header_info& hdr_info)
	{
		if (!img.get_width() || !img.get_height())
			return false;

		const uint32_t width = img.get_width(), height = img.get_height();
		
		file_data.resize(0);
		file_data.reserve(1024 + img.get_width() * img.get_height() * 4);

		append_string(file_data, "#?RADIANCE\n");

		if (hdr_info.m_has_exposure)
			append_string(file_data, string_format("EXPOSURE=%g\n", hdr_info.m_exposure));

		if (hdr_info.m_has_gamma)
			append_string(file_data, string_format("GAMMA=%g\n", hdr_info.m_gamma));

		append_string(file_data, "FORMAT=32-bit_rle_rgbe\n\n");
		append_string(file_data, string_format("-Y %u +X %u\n", height, width));

		if (((width < 8) || (width > 0x7FFF)) || (RGBE_FORCE_RAW))
		{
			for (uint32_t y = 0; y < height; y++)
			{
				for (uint32_t x = 0; x < width; x++)
				{
					color_rgba rgbe;
					float2rgbe(rgbe, img(x, y));
					append_vector(file_data, (const uint8_t *)&rgbe, sizeof(rgbe));
				}
			}
		}
		else if (RGBE_FORCE_OLD_CRUNCH)
		{
			for (uint32_t y = 0; y < height; y++)
			{
				int prev_r = -1, prev_g = -1, prev_b = -1, prev_e = -1;
				uint32_t cur_run_len = 0;
				
				for (uint32_t x = 0; x < width; x++)
				{
					color_rgba rgbe;
					float2rgbe(rgbe, img(x, y));

					if ((rgbe[0] == prev_r) && (rgbe[1] == prev_g) && (rgbe[2] == prev_b) && (rgbe[3] == prev_e))
					{
						if (++cur_run_len == 255)
						{
							// this ensures rshift stays 0, it's lame but this path is only for testing readers
							color_rgba f(1, 1, 1, cur_run_len - 1);
							append_vector(file_data, (const uint8_t*)&f, sizeof(f));
							append_vector(file_data, (const uint8_t*)&rgbe, sizeof(rgbe)); 
							cur_run_len = 0;
						}
					}
					else
					{
						if (cur_run_len > 0)
						{
							color_rgba f(1, 1, 1, cur_run_len);
							append_vector(file_data, (const uint8_t*)&f, sizeof(f));
							
							cur_run_len = 0;
						}
						
						append_vector(file_data, (const uint8_t*)&rgbe, sizeof(rgbe));
																		
						prev_r = rgbe[0];
						prev_g = rgbe[1];
						prev_b = rgbe[2];
						prev_e = rgbe[3];
					}
				} // x

				if (cur_run_len > 0)
				{
					color_rgba f(1, 1, 1, cur_run_len);
					append_vector(file_data, (const uint8_t*)&f, sizeof(f));
				}
			} // y
		}
		else
		{
			uint8_vec temp[4];
			for (uint32_t c = 0; c < 4; c++)
				temp[c].resize(width);

			for (uint32_t y = 0; y < height; y++)
			{
				color_rgba rgbe(2, 2, width >> 8, width & 0xFF);
				append_vector(file_data, (const uint8_t*)&rgbe, sizeof(rgbe));
								
				for (uint32_t x = 0; x < width; x++)
				{
					float2rgbe(rgbe, img(x, y));

					for (uint32_t c = 0; c < 4; c++)
						temp[c][x] = rgbe[c];
				}

				for (uint32_t c = 0; c < 4; c++)
				{
					int raw_ofs = -1;
					
					uint32_t x = 0;
					while (x < width)
					{
						const uint32_t num_bytes_remaining = width - x;
						const uint32_t max_run_len = basisu::minimum<uint32_t>(num_bytes_remaining, 127);
						const uint8_t cur_byte = temp[c][x];

						uint32_t run_len = 1;
						while (run_len < max_run_len)
						{
							if (temp[c][x + run_len] != cur_byte)
								break;
							run_len++;
						}
												
						const uint32_t cost_to_keep_raw = ((raw_ofs != -1) ? 0 : 1) + run_len; // 0 or 1 bytes to start a raw run, then the repeated bytes issued as raw
						const uint32_t cost_to_take_run = 2 + 1; // 2 bytes to issue the RLE, then 1 bytes to start whatever follows it (raw or RLE)

						if ((run_len >= 3) && (cost_to_take_run < cost_to_keep_raw))
						{
							file_data.push_back((uint8_t)(128 + run_len));
							file_data.push_back(cur_byte);

							x += run_len;
							raw_ofs = -1;
						}
						else
						{
							if (raw_ofs < 0)
							{
								raw_ofs = (int)file_data.size();
								file_data.push_back(0);
							}

							if (++file_data[raw_ofs] == 128)
								raw_ofs = -1;

							file_data.push_back(cur_byte);
							
							x++;
						}
					} // x

				} // c
			} // y
		}

		return true;
	}

	bool write_rgbe(const char* pFilename, imagef& img, rgbe_header_info& hdr_info)
	{
		uint8_vec file_data;
		if (!write_rgbe(file_data, img, hdr_info))
			return false;
		return write_vec_to_file(pFilename, file_data);
	}
		
	bool read_exr(const char* pFilename, imagef& img, int& n_chans)
	{
		n_chans = 0;

		int width = 0, height = 0;
		float* out_rgba = nullptr;
		const char* err = nullptr;
		
		int status = LoadEXRWithLayer(&out_rgba, &width, &height, pFilename, nullptr, &err, &n_chans);
		if (status != 0)
		{
			error_printf("Failed loading .EXR image \"%s\"! (TinyEXR error: %s)\n", pFilename, err ? err : "?");
			FreeEXRErrorMessage(err);
			free(out_rgba);
			return false;
		}

		const uint32_t MAX_SUPPORTED_DIM = 65536;
		if ((width < 1) || (height < 1) || (width > (int)MAX_SUPPORTED_DIM) || (height > (int)MAX_SUPPORTED_DIM))
		{
			error_printf("Invalid dimensions of .EXR image \"%s\"!\n", pFilename);
			free(out_rgba);
			return false;
		}

		img.resize(width, height);
		
		if (n_chans == 1)
		{
			const float* pSrc = out_rgba;
			vec4F* pDst = img.get_ptr();

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					(*pDst)[0] = pSrc[0];
					(*pDst)[1] = pSrc[1];
					(*pDst)[2] = pSrc[2];
					(*pDst)[3] = 1.0f;

					pSrc += 4;
					++pDst;
				}
			}
		}
		else
		{
			memcpy((void *)img.get_ptr(), out_rgba, static_cast<size_t>(sizeof(float) * 4 * img.get_total_pixels()));
		}

		free(out_rgba);
		return true;
	}

	bool read_exr(const void* pMem, size_t mem_size, imagef& img)
	{
		float* out_rgba = nullptr;
		int width = 0, height = 0;
		const char* pErr = nullptr;
		int res = LoadEXRFromMemory(&out_rgba, &width, &height, (const uint8_t*)pMem, mem_size, &pErr);
		if (res < 0)
		{
			error_printf("Failed loading .EXR image from memory! (TinyEXR error: %s)\n", pErr ? pErr : "?");
			FreeEXRErrorMessage(pErr);
			free(out_rgba);
			return false;
		}

		img.resize(width, height);
		memcpy((void *)img.get_ptr(), out_rgba, width * height * sizeof(float) * 4);
		free(out_rgba);

		return true;
	}

	bool write_exr(const char* pFilename, const imagef& img, uint32_t n_chans, uint32_t flags)
	{
		assert((n_chans == 1) || (n_chans == 3) || (n_chans == 4));

		const bool linear_hint = (flags & WRITE_EXR_LINEAR_HINT) != 0, 
			store_float = (flags & WRITE_EXR_STORE_FLOATS) != 0,
			no_compression = (flags & WRITE_EXR_NO_COMPRESSION) != 0;
								
		const uint32_t width = img.get_width(), height = img.get_height();
		assert(width && height);
		
		if (!width || !height)
			return false;
		
		float_vec layers[4];
		float* image_ptrs[4];
		for (uint32_t c = 0; c < n_chans; c++)
		{
			layers[c].resize(width * height);
			image_ptrs[c] = layers[c].get_ptr();
		}

		// ABGR
		int chan_order[4] = { 3, 2, 1, 0 };

		if (n_chans == 1)
		{
			// Y
			chan_order[0] = 0;
		}
		else if (n_chans == 3)
		{
			// BGR
			chan_order[0] = 2;
			chan_order[1] = 1;
			chan_order[2] = 0;
		}
		else if (n_chans != 4)
		{
			assert(0);
			return false;
		}
		
		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				const vec4F& p = img(x, y);

				for (uint32_t c = 0; c < n_chans; c++)
					layers[c][x + y * width] = p[chan_order[c]];
			} // x
		} // y

		EXRHeader header;
		InitEXRHeader(&header);

		EXRImage image;
		InitEXRImage(&image);

		image.num_channels = n_chans;
		image.images = (unsigned char**)image_ptrs;
		image.width = width;
		image.height = height;

		header.num_channels = n_chans;
		
		header.channels = (EXRChannelInfo*)calloc(header.num_channels, sizeof(EXRChannelInfo));

		// Must be (A)BGR order, since most of EXR viewers expect this channel order.
		for (uint32_t i = 0; i < n_chans; i++)
		{
			char c = 'Y';
			if (n_chans == 3)
				c = "BGR"[i];
			else if (n_chans == 4)
				c = "ABGR"[i];
						
			header.channels[i].name[0] = c;
			header.channels[i].name[1] = '\0';

			header.channels[i].p_linear = linear_hint;
		}
		
		header.pixel_types = (int*)calloc(header.num_channels, sizeof(int));
		header.requested_pixel_types = (int*)calloc(header.num_channels, sizeof(int));
		
		if (!no_compression)
			header.compression_type = TINYEXR_COMPRESSIONTYPE_ZIP;

		for (int i = 0; i < header.num_channels; i++) 
		{
			// pixel type of input image
			header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; 

			// pixel type of output image to be stored in .EXR
			header.requested_pixel_types[i] = store_float ? TINYEXR_PIXELTYPE_FLOAT : TINYEXR_PIXELTYPE_HALF; 
		}

		const char* pErr_msg = nullptr;

		int ret = SaveEXRImageToFile(&image, &header, pFilename, &pErr_msg);
		if (ret != TINYEXR_SUCCESS) 
		{
			error_printf("Save EXR err: %s\n", pErr_msg);
			FreeEXRErrorMessage(pErr_msg);
		}
				
		free(header.channels);
		free(header.pixel_types);
		free(header.requested_pixel_types);

		return (ret == TINYEXR_SUCCESS);
	}

	void image::debug_text(uint32_t x_ofs, uint32_t y_ofs, uint32_t scale_x, uint32_t scale_y, const color_rgba& fg, const color_rgba* pBG, bool alpha_only, const char* pFmt, ...)
	{
		char buf[2048];

		va_list args;
		va_start(args, pFmt);
#ifdef _WIN32		
		vsprintf_s(buf, sizeof(buf), pFmt, args);
#else
		vsnprintf(buf, sizeof(buf), pFmt, args);
#endif
		va_end(args);

		const char* p = buf;

		const uint32_t orig_x_ofs = x_ofs;

		while (*p)
		{
			uint8_t c = *p++;
			if ((c < 32) || (c > 127))
				c = '.';

			const uint8_t* pGlpyh = &g_debug_font8x8_basic[c - 32][0];

			for (uint32_t y = 0; y < 8; y++)
			{
				uint32_t row_bits = pGlpyh[y];
				for (uint32_t x = 0; x < 8; x++)
				{
					const uint32_t q = row_bits & (1 << x);
										
					const color_rgba* pColor = q ? &fg : pBG;
					if (!pColor)
						continue;

					if (alpha_only)
						fill_box_alpha(x_ofs + x * scale_x, y_ofs + y * scale_y, scale_x, scale_y, *pColor);
					else
						fill_box(x_ofs + x * scale_x, y_ofs + y * scale_y, scale_x, scale_y, *pColor);
				}
			}

			x_ofs += 8 * scale_x;
			if ((x_ofs + 8 * scale_x) > m_width)
			{
				x_ofs = orig_x_ofs;
				y_ofs += 8 * scale_y;
			}
		}
	}
	
	// Very basic global Reinhard tone mapping, output converted to sRGB with no dithering, alpha is carried through unchanged. 
	// Only used for debugging/development.
	void tonemap_image_reinhard(image &ldr_img, const imagef &hdr_img, float exposure, bool add_noise, bool per_component, bool luma_scaling)
	{
		uint32_t width = hdr_img.get_width(), height = hdr_img.get_height();

		ldr_img.resize(width, height);

		rand r;
		r.seed(128);
				
		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				vec4F c(hdr_img(x, y));

				if (per_component)
				{
					for (uint32_t t = 0; t < 3; t++)
					{
						if (c[t] <= 0.0f)
						{
							c[t] = 0.0f;
						}
						else
						{
							c[t] *= exposure;
							c[t] = c[t] / (1.0f + c[t]);
						}
					}
				}
				else
				{
					c[0] *= exposure;
					c[1] *= exposure;
					c[2] *= exposure;

					const float L = 0.2126f * c[0] + 0.7152f * c[1] + 0.0722f * c[2];

					float Lmapped = 0.0f;
					if (L > 0.0f)
					{
						//Lmapped = L / (1.0f + L);
						//Lmapped /= L;
						
						Lmapped = 1.0f / (1.0f + L);
					}

					c[0] = c[0] * Lmapped;
					c[1] = c[1] * Lmapped;
					c[2] = c[2] * Lmapped;

					if (luma_scaling)
					{
						// Keeps the ratio of r/g/b intact
						float m = maximum(c[0], c[1], c[2]);
						if (m > 1.0f)
						{
							c /= m;
						}
					}
				}

				c.clamp(0.0f, 1.0f);

				c[3] = c[3] * 255.0f;

				color_rgba& o = ldr_img(x, y);

				if (add_noise)
				{
					c[0] = linear_to_srgb(c[0]) * 255.0f;
					c[1] = linear_to_srgb(c[1]) * 255.0f;
					c[2] = linear_to_srgb(c[2]) * 255.0f;

					const float NOISE_AMP = .5f;
					c[0] += r.frand(-NOISE_AMP, NOISE_AMP);
					c[1] += r.frand(-NOISE_AMP, NOISE_AMP);
					c[2] += r.frand(-NOISE_AMP, NOISE_AMP);

					c.clamp(0.0f, 255.0f);

					o[0] = (uint8_t)fast_roundf_int(c[0]);
					o[1] = (uint8_t)fast_roundf_int(c[1]);
					o[2] = (uint8_t)fast_roundf_int(c[2]);
					o[3] = (uint8_t)fast_roundf_int(c[3]);
				}
				else
				{
					o[0] = g_fast_linear_to_srgb.convert(c[0]);
					o[1] = g_fast_linear_to_srgb.convert(c[1]);
					o[2] = g_fast_linear_to_srgb.convert(c[2]);
					o[3] = (uint8_t)fast_roundf_int(c[3]);
				}
			}
		}
	}

	bool tonemap_image_compressive(image& dst_img, const imagef& hdr_test_img)
	{
		const uint32_t width = hdr_test_img.get_width();
		const uint32_t height = hdr_test_img.get_height();

		uint16_vec orig_half_img(width * 3 * height);
		uint16_vec half_img(width * 3 * height);

		int max_shift = 32;

		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				const vec4F& p = hdr_test_img(x, y);

				for (uint32_t i = 0; i < 3; i++)
				{
					if (p[i] < 0.0f)
						return false;
					if (p[i] > basist::MAX_HALF_FLOAT)
						return false;

					uint32_t h = basist::float_to_half(p[i]);
					//uint32_t orig_h = h;

					orig_half_img[(x + y * width) * 3 + i] = (uint16_t)h;

					// Rotate sign bit into LSB
					//h = rot_left16((uint16_t)h, 1);
					//assert(rot_right16((uint16_t)h, 1) == orig_h);
					h <<= 1;

					half_img[(x + y * width) * 3 + i] = (uint16_t)h;

					// Determine # of leading zero bits, ignoring the sign bit
					if (h)
					{
						int lz = clz(h) - 16;
						assert(lz >= 0 && lz <= 16);

						assert((h << lz) <= 0xFFFF);

						max_shift = basisu::minimum<int>(max_shift, lz);
					}
				} // i
			} // x
		} // y

		//printf("tonemap_image_compressive: Max leading zeros: %i\n", max_shift);

		uint32_t high_hist[256];
		clear_obj(high_hist);

		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				for (uint32_t i = 0; i < 3; i++)
				{
					uint16_t& hf = half_img[(x + y * width) * 3 + i];

					assert(((uint32_t)hf << max_shift) <= 65535);

					hf <<= max_shift;

					uint32_t h = (uint8_t)(hf >> 8);
					high_hist[h]++;
				}
			} // x
		} // y

		uint32_t total_vals_used = 0;
		int remap_old_to_new[256];
		for (uint32_t i = 0; i < 256; i++)
			remap_old_to_new[i] = -1;

		for (uint32_t i = 0; i < 256; i++)
		{
			if (high_hist[i] != 0)
			{
				remap_old_to_new[i] = total_vals_used;
				total_vals_used++;
			}
		}

		assert(total_vals_used >= 1);

		//printf("tonemap_image_compressive: Total used high byte values: %u, unused: %u\n", total_vals_used, 256 - total_vals_used);

		bool val_used[256];
		clear_obj(val_used);

		int remap_new_to_old[256];
		for (uint32_t i = 0; i < 256; i++)
			remap_new_to_old[i] = -1;
		BASISU_NOTE_UNUSED(remap_new_to_old);

		int prev_c = -1;
		BASISU_NOTE_UNUSED(prev_c);
		for (uint32_t i = 0; i < 256; i++)
		{
			if (remap_old_to_new[i] >= 0)
			{
				int c;
				if (total_vals_used <= 1)
					c = remap_old_to_new[i];
				else
				{
					c = (remap_old_to_new[i] * 255 + ((total_vals_used - 1) / 2)) / (total_vals_used - 1);

					assert(c > prev_c);
				}

				assert(!val_used[c]);

				remap_new_to_old[c] = i;

				remap_old_to_new[i] = c;
				prev_c = c;

				//printf("%u ", c);

				val_used[c] = true;
			}
		} // i
		//printf("\n");

		dst_img.resize(width, height);

		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				for (uint32_t c = 0; c < 3; c++)
				{
					uint16_t& v16 = half_img[(x + y * width) * 3 + c];

					uint32_t hb = v16 >> 8;
					//uint32_t lb = v16 & 0xFF;

					assert(remap_old_to_new[hb] != -1);
					assert(remap_old_to_new[hb] <= 255);
					assert(remap_new_to_old[remap_old_to_new[hb]] == (int)hb);

					hb = remap_old_to_new[hb];

					//v16 = (uint16_t)((hb << 8) | lb);

					dst_img(x, y)[c] = (uint8_t)hb;
				}
			} // x
		} // y

		return true;
	}

	bool tonemap_image_compressive2(image& dst_img, const imagef& hdr_test_img)
	{
		const uint32_t width = hdr_test_img.get_width();
		const uint32_t height = hdr_test_img.get_height();

		dst_img.resize(width, height);
		dst_img.set_all(color_rgba(0, 0, 0, 255));

		basisu::vector<basist::half_float> half_img(width * 3 * height);
				
		uint32_t low_h = UINT32_MAX, high_h = 0;

		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				const vec4F& p = hdr_test_img(x, y);

				for (uint32_t i = 0; i < 3; i++)
				{
					float f = p[i];

					if (std::isnan(f) || std::isinf(f))
						f = 0.0f;
					else if (f < 0.0f)
						f = 0.0f;
					else if (f > basist::MAX_HALF_FLOAT)
						f = basist::MAX_HALF_FLOAT;

					uint32_t h = basist::float_to_half(f);

					low_h = minimum(low_h, h);
					high_h = maximum(high_h, h);
					
					half_img[(x + y * width) * 3 + i] = (basist::half_float)h;

				} // i
			} // x
		} // y

		if (low_h == high_h)
			return false;

		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				for (uint32_t i = 0; i < 3; i++)
				{
					basist::half_float h = half_img[(x + y * width) * 3 + i];
					
					float f = (float)(h - low_h) / (float)(high_h - low_h);

					int iv = basisu::clamp<int>((int)std::round(f * 255.0f), 0, 255);

					dst_img(x, y)[i] = (uint8_t)iv;

				} // i
			} // x
		} // y

		return true;
	}

	bool arith_test()
	{
		basist::arith_fastbits_f32::init();

		fmt_printf("random bit test\n");

		const uint32_t N = 1000;

		// random bit test
		for (uint32_t i = 0; i < N; i++)
		{
			basist::arith::arith_enc enc;
			enc.init(4096);

			{
				basisu::rand r;
				r.seed(i + 1);
				uint32_t num_vals = r.irand(1, 20000);

				for (uint32_t j = 0; j < num_vals; j++)
					enc.put_bit(r.bit());

				enc.flush();
			}

			{
				basisu::rand r;
				r.seed(i + 1);
				uint32_t num_vals = r.irand(1, 20000);

				basist::arith::arith_dec dec;
				dec.init(enc.get_data_buf().get_ptr(), enc.get_data_buf().size());

				for (uint32_t j = 0; j < num_vals; j++)
				{
					uint32_t t = r.bit();

					uint32_t a = dec.get_bit();
					if (t != a)
					{
						fmt_printf("error!");
						return false;
					}
				}
			}
		}

		fmt_printf("Random bit test OK\n");

		fmt_printf("random bits test\n");

		// random bits test
		for (uint32_t i = 0; i < N; i++)
		{
			basist::arith::arith_enc enc;
			enc.init(4096);

			{
				basisu::rand r;
				r.seed(i + 1);
				uint32_t num_vals = r.irand(1, 20000);
				uint32_t num_bits = r.irand(1, 20);

				for (uint32_t j = 0; j < num_vals; j++)
					enc.put_bits(r.urand32() & ((1 << num_bits) - 1), num_bits);

				enc.flush();
			}

			{
				basisu::rand r;
				r.seed(i + 1);
				uint32_t num_vals = r.irand(1, 20000);
				uint32_t num_bits = r.irand(1, 20);

				basist::arith::arith_dec dec;
				dec.init(enc.get_data_buf().get_ptr(), enc.get_data_buf().size());

				for (uint32_t j = 0; j < num_vals; j++)
				{
					uint32_t t = r.urand32() & ((1 << num_bits) - 1);

					uint32_t a = dec.get_bits(num_bits);
					if (t != a)
					{
						fmt_printf("error!");
						return false;
					}
				}
			}
		}

		fmt_printf("Random bits test OK\n");

		fmt_printf("random adaptive bit model test\n");

		// adaptive bit model random test
		for (uint32_t i = 0; i < N; i++)
		{
			basist::arith::arith_enc enc;
			enc.init(4096);

			{
				basisu::rand r;
				r.seed(i + 1);
				uint32_t num_vals = r.irand(1, 20000);

				basist::arith::arith_bit_model bm;
				bm.init();

				for (uint32_t j = 0; j < num_vals; j++)
					enc.encode(r.bit(), bm);

				enc.flush();
			}

			{
				basisu::rand r;
				r.seed(i + 1);
				uint32_t num_vals = r.irand(1, 20000);

				basist::arith::arith_dec dec;
				dec.init(enc.get_data_buf().get_ptr(), enc.get_data_buf().size());

				basist::arith::arith_bit_model bm;
				bm.init();

				for (uint32_t j = 0; j < num_vals; j++)
				{
					uint32_t t = r.bit();

					uint32_t a = dec.decode_bit(bm);
					if (t != a)
					{
						fmt_printf("error!");
						return false;
					}
				}
			}
		}
		fmt_printf("Random adaptive bits test OK\n");

		fmt_printf("random adaptive bit model 0 or 1 run test\n");

		// adaptive bit model 0 or 1 test
		for (uint32_t i = 0; i < N; i++)
		{
			basist::arith::arith_enc enc;
			enc.init(4096);

			{
				basisu::rand r;
				r.seed(i + 1);
				uint32_t num_vals = r.irand(1, 20000);

				basist::arith::arith_bit_model bm;
				bm.init();

				for (uint32_t j = 0; j < num_vals; j++)
					enc.encode(i & 1, bm);

				enc.flush();
			}

			{
				basisu::rand r;
				r.seed(i + 1);
				uint32_t num_vals = r.irand(1, 20000);

				basist::arith::arith_dec dec;
				dec.init(enc.get_data_buf().get_ptr(), enc.get_data_buf().size());

				basist::arith::arith_bit_model bm;
				bm.init();

				for (uint32_t j = 0; j < num_vals; j++)
				{
					uint32_t t = i & 1;

					uint32_t a = dec.decode_bit(bm);
					if (t != a)
					{
						fmt_printf("error!");
						return false;
					}
				}
			}
		}

		fmt_printf("Adaptive bit model 0 or 1 run test OK\n");

		fmt_printf("random adaptive bit model 0 or 1 run 2 test\n");

		// adaptive bit model 0 or 1 run test
		for (uint32_t i = 0; i < N; i++)
		{
			basist::arith::arith_enc enc;
			enc.init(4096);

			{
				basisu::rand r;
				r.seed(i + 1);
				uint32_t num_vals = r.irand(1, 2000);

				basist::arith::arith_bit_model bm;
				bm.init();

				for (uint32_t j = 0; j < num_vals; j++)
				{
					const uint32_t run_len = r.irand(1, 128);
					const uint32_t t = r.bit();
					for (uint32_t k = 0; k < run_len; k++)
						enc.encode(t, bm);
				}

				if (r.frand(0.0f, 1.0f) < .1f)
				{
					for (uint32_t q = 0; q < 1000; q++)
						enc.encode(0, bm);
				}

				enc.flush();
			}

			{
				basisu::rand r;
				r.seed(i + 1);
				uint32_t num_vals = r.irand(1, 2000);

				basist::arith::arith_dec dec;
				dec.init(enc.get_data_buf().get_ptr(), enc.get_data_buf().size());

				basist::arith::arith_bit_model bm;
				bm.init();

				for (uint32_t j = 0; j < num_vals; j++)
				{
					const uint32_t run_len = r.irand(1, 128);
					const uint32_t t = r.bit();

					for (uint32_t k = 0; k < run_len; k++)
					{
						uint32_t a = dec.decode_bit(bm);
						if (a != t)
						{
							fmt_printf("adaptive bit model random run test failed!\n");
							return false;
						}
					}
				}

				if (r.frand(0.0f, 1.0f) < .1f)
				{
					for (uint32_t q = 0; q < 1000; q++)
					{
						uint32_t d = dec.decode_bit(bm);
						if (d != 0)
						{
							fmt_printf("adaptive bit model random run test failed!\n");
							return false;
						}
					}
				}
			}
		}

		fmt_printf("Random data model test\n");

		// random data model test
		for (uint32_t i = 0; i < N; i++)
		{
			basist::arith::arith_enc enc;
			enc.init(4096);

			{
				basisu::rand r;
				r.seed(i + 1);
				const uint32_t num_vals = r.irand(1, 60000);

				uint32_t num_syms = r.irand(2, basist::arith::ArithMaxSyms);

				basist::arith::arith_data_model dm;
				dm.init(num_syms);

				for (uint32_t j = 0; j < num_vals; j++)
					enc.encode(r.irand(0, num_syms - 1), dm);

				enc.flush();
			}

			{
				basisu::rand r;
				r.seed(i + 1);
				uint32_t num_vals = r.irand(1, 60000);

				const uint32_t num_syms = r.irand(2, basist::arith::ArithMaxSyms);

				basist::arith::arith_dec dec;
				dec.init(enc.get_data_buf().get_ptr(), enc.get_data_buf().size());

				basist::arith::arith_data_model dm;
				dm.init(num_syms);

				for (uint32_t j = 0; j < num_vals; j++)
				{
					uint32_t expected = r.irand(0, num_syms - 1);
					uint32_t actual = dec.decode_sym(dm);
					if (actual != expected)
					{
						fmt_printf("adaptive data model random test failed!\n");
						return false;
					}
				}
			}
		}

		fmt_printf("Adaptive data model random test OK\n");

		fmt_printf("Overall OK\n");
		return true;
	}

	static void rasterize_line(image& dst, int xs, int ys, int xe, int ye, int pred, int inc_dec, int e, int e_inc, int e_no_inc, const color_rgba& color)
	{
		int start, end, var;

		if (pred)
		{
			start = ys; end = ye; var = xs;
			for (int i = start; i <= end; i++)
			{
				dst.set_clipped(var, i, color);
				if (e < 0)
					e += e_no_inc;
				else
				{
					var += inc_dec;
					e += e_inc;
				}
			}
		}
		else
		{
			start = xs; end = xe; var = ys;
			for (int i = start; i <= end; i++)
			{
				dst.set_clipped(i, var, color);
				if (e < 0)
					e += e_no_inc;
				else
				{
					var += inc_dec;
					e += e_inc;
				}
			}
		}
	}

	void draw_line(image& dst, int xs, int ys, int xe, int ye, const color_rgba& color)
	{
		if (xs > xe)
		{
			std::swap(xs, xe);
			std::swap(ys, ye);
		}

		int dx = xe - xs, dy = ye - ys;
		if (!dx)
		{
			if (ys > ye)
				std::swap(ys, ye);
			for (int i = ys; i <= ye; i++)
				dst.set_clipped(xs, i, color);
		}
		else if (!dy)
		{
			for (int i = xs; i < xe; i++)
				dst.set_clipped(i, ys, color);
		}
		else if (dy > 0)
		{
			if (dy <= dx)
			{
				int e = 2 * dy - dx, e_no_inc = 2 * dy, e_inc = 2 * (dy - dx);
				rasterize_line(dst, xs, ys, xe, ye, 0, 1, e, e_inc, e_no_inc, color);
			}
			else
			{
				int e = 2 * dx - dy, e_no_inc = 2 * dx, e_inc = 2 * (dx - dy);
				rasterize_line(dst, xs, ys, xe, ye, 1, 1, e, e_inc, e_no_inc, color);
			}
		}
		else
		{
			dy = -dy;
			if (dy <= dx)
			{
				int e = 2 * dy - dx, e_no_inc = 2 * dy, e_inc = 2 * (dy - dx);
				rasterize_line(dst, xs, ys, xe, ye, 0, -1, e, e_inc, e_no_inc, color);
			}
			else
			{
				int e = 2 * dx - dy, e_no_inc = (2 * dx), e_inc = 2 * (dx - dy);
				rasterize_line(dst, xe, ye, xs, ys, 1, -1, e, e_inc, e_no_inc, color);
			}
		}
	}

	// Used for generating random test data
	void draw_circle(image& dst, int cx, int cy, int r, const color_rgba& color)
	{
		assert(r >= 0);
		if (r < 0)
			return;

		int x = r;
		int y = 0;
		int err = 1 - x;

		while (x >= y) 
		{
			dst.set_clipped(cx + x, cy + y, color);
			dst.set_clipped(cx + y, cy + x, color);
			dst.set_clipped(cx - y, cy + x, color);
			dst.set_clipped(cx - x, cy + y, color);
			dst.set_clipped(cx - x, cy - y, color);
			dst.set_clipped(cx - y, cy - x, color);
			dst.set_clipped(cx + y, cy - x, color);
			dst.set_clipped(cx + x, cy - y, color);

			++y;

			if (err < 0) 
			{
				err += 2 * y + 1;
			}
			else 
			{
				--x;
				err += 2 * (y - x) + 1;
			}
		}
	}

	void set_image_alpha(image& img, uint32_t a)
	{
		for (uint32_t y = 0; y < img.get_height(); y++)
			for (uint32_t x = 0; x < img.get_width(); x++)
				img(x, y).a = (uint8_t)a;
	}

	// red=3 subsets, blue=2 subsets, green=mode 6, white=mode 7, purple = 2 plane
	const color_rgba g_bc7_mode_vis_colors[8] =
	{
		color_rgba(190, 0,   0, 255), // 0
		color_rgba(0, 0, 255, 255), // 1
		color_rgba(255, 0, 0, 255), // 2
		color_rgba(0, 0,  130, 255), // 3
		color_rgba(255, 0, 255, 255), // 4 
		color_rgba(190,  0,  190, 255), // 5
		color_rgba(50, 167, 30, 255), // 6
		color_rgba(255,   255,   255, 255)  // 7
	};

	void create_bc7_debug_images(
		uint32_t width, uint32_t height, 
		const void *pBlocks, 
		const char *pFilename_prefix)
	{
		assert(width && height && pBlocks );

		const uint32_t num_bc7_blocks_x = (width + 3) >> 2;
		const uint32_t num_bc7_blocks_y = (height + 3) >> 2;
		const uint32_t total_bc7_blocks = num_bc7_blocks_x * num_bc7_blocks_y;

		image bc7_mode_vis(width, height);

		uint32_t bc7_mode_hist[9] = {};

		uint32_t mode4_index_hist[2] = {};
		uint32_t mode4_rot_hist[4] = {};
		uint32_t mode5_rot_hist[4] = {};

		uint32_t num_2subsets = 0, num_3subsets = 0, num_dp = 0;

		uint32_t total_solid_bc7_blocks = 0;
		uint32_t num_unpack_failures = 0;

		for (uint32_t by = 0; by < num_bc7_blocks_y; by++)
		{
			const uint32_t base_y = by * 4;

			for (uint32_t bx = 0; bx < num_bc7_blocks_x; bx++)
			{
				const uint32_t base_x = bx * 4;
								
				const basist::bc7_block& blk = ((const basist::bc7_block *)pBlocks)[bx + by * num_bc7_blocks_x];

				color_rgba unpacked_pixels[16];
				bool status = basist::bc7u::unpack_bc7(&blk, (basist::color_rgba*)unpacked_pixels);
				if (!status)
					num_unpack_failures++;

				int mode_index = basist::bc7u::determine_bc7_mode(&blk);

				bool is_solid = false;
				
				// assumes our transcoder's analytical BC7 encoder wrote the solid block
				if (mode_index == 5)
				{
					const uint8_t* pBlock_bytes = (const uint8_t *)&blk;
										
					if (pBlock_bytes[0] == 0b00100000)
					{
						static const uint8_t s_tail_bytes[8] = { 0xac, 0xaa, 0xaa, 0xaa, 0, 0, 0, 0 };
						if ((pBlock_bytes[8] & ~3) == (s_tail_bytes[0] & ~3))
						{
							if (memcmp(pBlock_bytes + 9, s_tail_bytes + 1, 7) == 0)
							{
								is_solid = true;
							}
						}
					}
				}

				total_solid_bc7_blocks += is_solid;

				if ((mode_index == 0) || (mode_index == 2))
					num_3subsets++;
				else if ((mode_index == 1) || (mode_index == 3))
					num_2subsets++;

				bc7_mode_hist[mode_index + 1]++;

				if (mode_index == 4)
				{
					num_dp++;
					mode4_index_hist[range_check(basist::bc7u::determine_bc7_mode_4_index_mode(&blk), 0, 1)]++;
					mode4_rot_hist[range_check(basist::bc7u::determine_bc7_mode_4_or_5_rotation(&blk), 0, 3)]++;
				}
				else if (mode_index == 5)
				{
					num_dp++;
					mode5_rot_hist[range_check(basist::bc7u::determine_bc7_mode_4_or_5_rotation(&blk), 0, 3)]++;
				}

				color_rgba c((mode_index < 0) ? g_black_color : g_bc7_mode_vis_colors[mode_index]);

				if (is_solid)
					c.set(64, 0, 64, 255);

				bc7_mode_vis.fill_box(base_x, base_y, 4, 4, c);

			} // bx

		} // by

		fmt_debug_printf("--------- BC7 statistics:\n");
		fmt_debug_printf("\nTotal BC7 unpack failures: {}\n", num_unpack_failures);
		fmt_debug_printf("Total solid blocks: {} {3.2}%\n", total_solid_bc7_blocks, (float)total_solid_bc7_blocks * (float)100.0f / (float)total_bc7_blocks);

		fmt_debug_printf("\nTotal 2-subsets: {} {3.2}%\n", num_2subsets, (float)num_2subsets * 100.0f / (float)total_bc7_blocks);
		fmt_debug_printf("Total 3-subsets: {} {3.2}%\n", num_3subsets, (float)num_3subsets * 100.0f / (float)total_bc7_blocks);
		fmt_debug_printf("Total Dual Plane: {} {3.2}%\n", num_dp, (float)num_dp * 100.0f / (float)total_bc7_blocks);

		fmt_debug_printf("\nBC7 mode histogram:\n");
		for (int i = -1; i <= 7; i++)
		{
			fmt_debug_printf(" {}: {} {3.3}%\n", i, bc7_mode_hist[1 + i], (float)bc7_mode_hist[1 + i] * 100.0f / (float)total_bc7_blocks);
		}

		fmt_debug_printf("\nMode 4 index bit histogram: {} {3.2}%, {} {3.2}%\n",
			mode4_index_hist[0], (float)mode4_index_hist[0] * 100.0f / (float)total_bc7_blocks,
			mode4_index_hist[1], (float)mode4_index_hist[1] * 100.0f / (float)total_bc7_blocks);

		fmt_debug_printf("\nMode 4 rotation histogram:\n");
		for (uint32_t i = 0; i < 4; i++)
		{
			fmt_debug_printf(" {}: {} {3.2}%\n", i, mode4_rot_hist[i], (float)mode4_rot_hist[i] * 100.0f / (float)total_bc7_blocks);
		}

		fmt_debug_printf("\nMode 5 rotation histogram:\n");
		for (uint32_t i = 0; i < 4; i++)
		{
			fmt_debug_printf(" {}: {} {3.2}%\n", i, mode5_rot_hist[i], (float)mode5_rot_hist[i] * 100.0f / (float)total_bc7_blocks);
		}
				
		if (pFilename_prefix)
		{
			std::string mode_vis_filename(std::string(pFilename_prefix) + "bc7_mode_vis.png");
			save_png(mode_vis_filename, bc7_mode_vis);

			fmt_debug_printf("Wrote BC7 mode visualization to PNG file {}\n", mode_vis_filename);
		}
		
		fmt_debug_printf("--------- End BC7 statistics\n");
		fmt_debug_printf("\n");
	}

	static inline float edge(const vec2F& a, const vec2F& b, const vec2F& pos)
	{
		return (pos[0] - a[0]) * (b[1] - a[1]) - (pos[1] - a[1]) * (b[0] - a[0]);
	}

	void draw_tri2(image& dst, const image* pTex, const tri2& tri, bool alpha_blend)
	{
		assert(dst.get_total_pixels());

		float area = edge(tri.p0, tri.p1, tri.p2);
		if (std::fabs(area) < 1e-6f)
			return;

		const float oo_area = 1.0f / area;

		int minx = (int)std::floor(basisu::minimum(tri.p0[0], tri.p1[0], tri.p2[0] ));
		int miny = (int)std::floor(basisu::minimum(tri.p0[1], tri.p1[1], tri.p2[1] ));

		int maxx = (int)std::ceil(basisu::maximum(tri.p0[0], tri.p1[0], tri.p2[0]));
		int maxy = (int)std::ceil(basisu::maximum(tri.p0[1], tri.p1[1], tri.p2[1]));

		auto clamp8 = [&](float fv) { int v = (int)(fv + .5f); if (v < 0) v = 0; else if (v > 255) v = 255;  return (uint8_t)v; };

		if ((maxx < 0) || (maxy < 0))
			return;
		if ((minx >= (int)dst.get_width()) || (miny >= (int)dst.get_height()))
			return;

		if (minx < 0)
			minx = 0;
		if (maxx >= (int)dst.get_width())
			maxx = dst.get_width() - 1;
		if (miny < 0)
			miny = 0;
		if (maxy >= (int)dst.get_height())
			maxy = dst.get_height() - 1;

		vec4F tex(1.0f);

		for (int y = miny; y <= maxy; ++y)
		{
			assert((y >= 0) && (y < (int)dst.get_height()));

			for (int x = minx; x <= maxx; ++x)
			{
				assert((x >= 0) && (x < (int)dst.get_width()));

				vec2F p{ (float)x + 0.5f, (float)y + 0.5f };

				float w0 = edge(tri.p1, tri.p2, p) * oo_area;
				float w1 = edge(tri.p2, tri.p0, p) * oo_area;
				float w2 = edge(tri.p0, tri.p1, p) * oo_area;

				if ((w0 < 0) || (w1 < 0) || (w2 < 0))
					continue;

				float u = tri.t0[0] * w0 + tri.t1[0] * w1 + tri.t2[0] * w2;
				float v = tri.t0[1] * w0 + tri.t1[1] * w1 + tri.t2[1] * w2;

				if (pTex)
					tex = pTex->get_filtered_vec4F(u * float(pTex->get_width()), v * float(pTex->get_height())) * (1.0f / 255.0f);

				float r = (float)tri.c0.r * w0 + (float)tri.c1.r * w1 + (float)tri.c2.r * w2;
				float g = (float)tri.c0.g * w0 + (float)tri.c1.g * w1 + (float)tri.c2.g * w2;
				float b = (float)tri.c0.b * w0 + (float)tri.c1.b * w1 + (float)tri.c2.b * w2;
				float a = (float)tri.c0.a * w0 + (float)tri.c1.a * w1 + (float)tri.c2.a * w2;

				r *= tex[0];
				g *= tex[1];
				b *= tex[2];
				a *= tex[3];

				if (alpha_blend)
				{
					color_rgba dst_color(dst(x, y));

					const float fa = (float)a * (1.0f / 255.0f);

					r = lerp((float)dst_color[0], r, fa);
					g = lerp((float)dst_color[1], g, fa);
					b = lerp((float)dst_color[2], b, fa);
					a = lerp((float)dst_color[3], a, fa);

					dst(x, y) = color_rgba(clamp8(r), clamp8(g), clamp8(b), clamp8(a));
				}
				else
				{
					dst(x, y) = color_rgba(clamp8(r), clamp8(g), clamp8(b), clamp8(a));
				}

			} // x
		} // y
	}

	// macro sent by CMakeLists.txt file when (TARGET_WASM AND WASM_THREADING)
#if BASISU_WASI_THREADS
	// Default to 8 - seems reasonable.
	static int g_num_wasi_threads = 8;
#else
	static int g_num_wasi_threads = 0;
#endif

	void set_num_wasi_threads(uint32_t num_threads)
	{
		g_num_wasi_threads = num_threads;
	}

	int get_num_hardware_threads()
	{
#ifdef __wasi__
		int num_threads = g_num_wasi_threads;
#else
		int num_threads = std::thread::hardware_concurrency();
#endif
				
		return num_threads;
	}
							
} // namespace basisu
