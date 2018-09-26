/*----------------------------------------------------------------------------*/
/**
 *	This confidential and proprietary software may be used only as
 *	authorised by a licensing agreement from ARM Limited
 *	(C) COPYRIGHT 2011-2012 ARM Limited
 *	ALL RIGHTS RESERVED
 *
 *	The entire notice above must be reproduced on all authorised
 *	copies and copies may only be made to the extent permitted
 *	by a licensing agreement from ARM Limited.
 *
 *	@brief	Functions to encode/decode data using Bounded Integer Sequence
 *			Encoding.
 */
/*----------------------------------------------------------------------------*/
#include "astc_codec_internals.h"
	// unpacked quint triplets <low,middle,high> for each packed-quint value
static const uint8_t quints_of_integer[128][3] = {
	{0, 0, 0},	{1, 0, 0},	{2, 0, 0},	{3, 0, 0},
	{4, 0, 0},	{0, 4, 0},	{4, 4, 0},	{4, 4, 4},
	{0, 1, 0},	{1, 1, 0},	{2, 1, 0},	{3, 1, 0},
	{4, 1, 0},	{1, 4, 0},	{4, 4, 1},	{4, 4, 4},
	{0, 2, 0},	{1, 2, 0},	{2, 2, 0},	{3, 2, 0},
	{4, 2, 0},	{2, 4, 0},	{4, 4, 2},	{4, 4, 4},
	{0, 3, 0},	{1, 3, 0},	{2, 3, 0},	{3, 3, 0},
	{4, 3, 0},	{3, 4, 0},	{4, 4, 3},	{4, 4, 4},
	{0, 0, 1},	{1, 0, 1},	{2, 0, 1},	{3, 0, 1},
	{4, 0, 1},	{0, 4, 1},	{4, 0, 4},	{0, 4, 4},
	{0, 1, 1},	{1, 1, 1},	{2, 1, 1},	{3, 1, 1},
	{4, 1, 1},	{1, 4, 1},	{4, 1, 4},	{1, 4, 4},
	{0, 2, 1},	{1, 2, 1},	{2, 2, 1},	{3, 2, 1},
	{4, 2, 1},	{2, 4, 1},	{4, 2, 4},	{2, 4, 4},
	{0, 3, 1},	{1, 3, 1},	{2, 3, 1},	{3, 3, 1},
	{4, 3, 1},	{3, 4, 1},	{4, 3, 4},	{3, 4, 4},
	{0, 0, 2},	{1, 0, 2},	{2, 0, 2},	{3, 0, 2},
	{4, 0, 2},	{0, 4, 2},	{2, 0, 4},	{3, 0, 4},
	{0, 1, 2},	{1, 1, 2},	{2, 1, 2},	{3, 1, 2},
	{4, 1, 2},	{1, 4, 2},	{2, 1, 4},	{3, 1, 4},
	{0, 2, 2},	{1, 2, 2},	{2, 2, 2},	{3, 2, 2},
	{4, 2, 2},	{2, 4, 2},	{2, 2, 4},	{3, 2, 4},
	{0, 3, 2},	{1, 3, 2},	{2, 3, 2},	{3, 3, 2},
	{4, 3, 2},	{3, 4, 2},	{2, 3, 4},	{3, 3, 4},
	{0, 0, 3},	{1, 0, 3},	{2, 0, 3},	{3, 0, 3},
	{4, 0, 3},	{0, 4, 3},	{0, 0, 4},	{1, 0, 4},
	{0, 1, 3},	{1, 1, 3},	{2, 1, 3},	{3, 1, 3},
	{4, 1, 3},	{1, 4, 3},	{0, 1, 4},	{1, 1, 4},
	{0, 2, 3},	{1, 2, 3},	{2, 2, 3},	{3, 2, 3},
	{4, 2, 3},	{2, 4, 3},	{0, 2, 4},	{1, 2, 4},
	{0, 3, 3},	{1, 3, 3},	{2, 3, 3},	{3, 3, 3},
	{4, 3, 3},	{3, 4, 3},	{0, 3, 4},	{1, 3, 4},
};

// packed quint-value for every unpacked quint-triplet
// indexed by [high][middle][low]
static const uint8_t integer_of_quints[5][5][5] = {
	{
	 {0, 1, 2, 3, 4,},
	 {8, 9, 10, 11, 12,},
	 {16, 17, 18, 19, 20,},
	 {24, 25, 26, 27, 28,},
	 {5, 13, 21, 29, 6,},
	 },
	{
	 {32, 33, 34, 35, 36,},
	 {40, 41, 42, 43, 44,},
	 {48, 49, 50, 51, 52,},
	 {56, 57, 58, 59, 60,},
	 {37, 45, 53, 61, 14,},
	 },
	{
	 {64, 65, 66, 67, 68,},
	 {72, 73, 74, 75, 76,},
	 {80, 81, 82, 83, 84,},
	 {88, 89, 90, 91, 92,},
	 {69, 77, 85, 93, 22,},
	 },
	{
	 {96, 97, 98, 99, 100,},
	 {104, 105, 106, 107, 108,},
	 {112, 113, 114, 115, 116,},
	 {120, 121, 122, 123, 124,},
	 {101, 109, 117, 125, 30,},
	 },
	{
	 {102, 103, 70, 71, 38,},
	 {110, 111, 78, 79, 46,},
	 {118, 119, 86, 87, 54,},
	 {126, 127, 94, 95, 62,},
	 {39, 47, 55, 63, 31,},
	 },
};

// unpacked trit quintuplets <low,_,_,_,high> for each packed-quint value
static const uint8_t trits_of_integer[256][5] = {
	{0, 0, 0, 0, 0},	{1, 0, 0, 0, 0},	{2, 0, 0, 0, 0},	{0, 0, 2, 0, 0},
	{0, 1, 0, 0, 0},	{1, 1, 0, 0, 0},	{2, 1, 0, 0, 0},	{1, 0, 2, 0, 0},
	{0, 2, 0, 0, 0},	{1, 2, 0, 0, 0},	{2, 2, 0, 0, 0},	{2, 0, 2, 0, 0},
	{0, 2, 2, 0, 0},	{1, 2, 2, 0, 0},	{2, 2, 2, 0, 0},	{2, 0, 2, 0, 0},
	{0, 0, 1, 0, 0},	{1, 0, 1, 0, 0},	{2, 0, 1, 0, 0},	{0, 1, 2, 0, 0},
	{0, 1, 1, 0, 0},	{1, 1, 1, 0, 0},	{2, 1, 1, 0, 0},	{1, 1, 2, 0, 0},
	{0, 2, 1, 0, 0},	{1, 2, 1, 0, 0},	{2, 2, 1, 0, 0},	{2, 1, 2, 0, 0},
	{0, 0, 0, 2, 2},	{1, 0, 0, 2, 2},	{2, 0, 0, 2, 2},	{0, 0, 2, 2, 2},
	{0, 0, 0, 1, 0},	{1, 0, 0, 1, 0},	{2, 0, 0, 1, 0},	{0, 0, 2, 1, 0},
	{0, 1, 0, 1, 0},	{1, 1, 0, 1, 0},	{2, 1, 0, 1, 0},	{1, 0, 2, 1, 0},
	{0, 2, 0, 1, 0},	{1, 2, 0, 1, 0},	{2, 2, 0, 1, 0},	{2, 0, 2, 1, 0},
	{0, 2, 2, 1, 0},	{1, 2, 2, 1, 0},	{2, 2, 2, 1, 0},	{2, 0, 2, 1, 0},
	{0, 0, 1, 1, 0},	{1, 0, 1, 1, 0},	{2, 0, 1, 1, 0},	{0, 1, 2, 1, 0},
	{0, 1, 1, 1, 0},	{1, 1, 1, 1, 0},	{2, 1, 1, 1, 0},	{1, 1, 2, 1, 0},
	{0, 2, 1, 1, 0},	{1, 2, 1, 1, 0},	{2, 2, 1, 1, 0},	{2, 1, 2, 1, 0},
	{0, 1, 0, 2, 2},	{1, 1, 0, 2, 2},	{2, 1, 0, 2, 2},	{1, 0, 2, 2, 2},
	{0, 0, 0, 2, 0},	{1, 0, 0, 2, 0},	{2, 0, 0, 2, 0},	{0, 0, 2, 2, 0},
	{0, 1, 0, 2, 0},	{1, 1, 0, 2, 0},	{2, 1, 0, 2, 0},	{1, 0, 2, 2, 0},
	{0, 2, 0, 2, 0},	{1, 2, 0, 2, 0},	{2, 2, 0, 2, 0},	{2, 0, 2, 2, 0},
	{0, 2, 2, 2, 0},	{1, 2, 2, 2, 0},	{2, 2, 2, 2, 0},	{2, 0, 2, 2, 0},
	{0, 0, 1, 2, 0},	{1, 0, 1, 2, 0},	{2, 0, 1, 2, 0},	{0, 1, 2, 2, 0},
	{0, 1, 1, 2, 0},	{1, 1, 1, 2, 0},	{2, 1, 1, 2, 0},	{1, 1, 2, 2, 0},
	{0, 2, 1, 2, 0},	{1, 2, 1, 2, 0},	{2, 2, 1, 2, 0},	{2, 1, 2, 2, 0},
	{0, 2, 0, 2, 2},	{1, 2, 0, 2, 2},	{2, 2, 0, 2, 2},	{2, 0, 2, 2, 2},
	{0, 0, 0, 0, 2},	{1, 0, 0, 0, 2},	{2, 0, 0, 0, 2},	{0, 0, 2, 0, 2},
	{0, 1, 0, 0, 2},	{1, 1, 0, 0, 2},	{2, 1, 0, 0, 2},	{1, 0, 2, 0, 2},
	{0, 2, 0, 0, 2},	{1, 2, 0, 0, 2},	{2, 2, 0, 0, 2},	{2, 0, 2, 0, 2},
	{0, 2, 2, 0, 2},	{1, 2, 2, 0, 2},	{2, 2, 2, 0, 2},	{2, 0, 2, 0, 2},
	{0, 0, 1, 0, 2},	{1, 0, 1, 0, 2},	{2, 0, 1, 0, 2},	{0, 1, 2, 0, 2},
	{0, 1, 1, 0, 2},	{1, 1, 1, 0, 2},	{2, 1, 1, 0, 2},	{1, 1, 2, 0, 2},
	{0, 2, 1, 0, 2},	{1, 2, 1, 0, 2},	{2, 2, 1, 0, 2},	{2, 1, 2, 0, 2},
	{0, 2, 2, 2, 2},	{1, 2, 2, 2, 2},	{2, 2, 2, 2, 2},	{2, 0, 2, 2, 2},
	{0, 0, 0, 0, 1},	{1, 0, 0, 0, 1},	{2, 0, 0, 0, 1},	{0, 0, 2, 0, 1},
	{0, 1, 0, 0, 1},	{1, 1, 0, 0, 1},	{2, 1, 0, 0, 1},	{1, 0, 2, 0, 1},
	{0, 2, 0, 0, 1},	{1, 2, 0, 0, 1},	{2, 2, 0, 0, 1},	{2, 0, 2, 0, 1},
	{0, 2, 2, 0, 1},	{1, 2, 2, 0, 1},	{2, 2, 2, 0, 1},	{2, 0, 2, 0, 1},
	{0, 0, 1, 0, 1},	{1, 0, 1, 0, 1},	{2, 0, 1, 0, 1},	{0, 1, 2, 0, 1},
	{0, 1, 1, 0, 1},	{1, 1, 1, 0, 1},	{2, 1, 1, 0, 1},	{1, 1, 2, 0, 1},
	{0, 2, 1, 0, 1},	{1, 2, 1, 0, 1},	{2, 2, 1, 0, 1},	{2, 1, 2, 0, 1},
	{0, 0, 1, 2, 2},	{1, 0, 1, 2, 2},	{2, 0, 1, 2, 2},	{0, 1, 2, 2, 2},
	{0, 0, 0, 1, 1},	{1, 0, 0, 1, 1},	{2, 0, 0, 1, 1},	{0, 0, 2, 1, 1},
	{0, 1, 0, 1, 1},	{1, 1, 0, 1, 1},	{2, 1, 0, 1, 1},	{1, 0, 2, 1, 1},
	{0, 2, 0, 1, 1},	{1, 2, 0, 1, 1},	{2, 2, 0, 1, 1},	{2, 0, 2, 1, 1},
	{0, 2, 2, 1, 1},	{1, 2, 2, 1, 1},	{2, 2, 2, 1, 1},	{2, 0, 2, 1, 1},
	{0, 0, 1, 1, 1},	{1, 0, 1, 1, 1},	{2, 0, 1, 1, 1},	{0, 1, 2, 1, 1},
	{0, 1, 1, 1, 1},	{1, 1, 1, 1, 1},	{2, 1, 1, 1, 1},	{1, 1, 2, 1, 1},
	{0, 2, 1, 1, 1},	{1, 2, 1, 1, 1},	{2, 2, 1, 1, 1},	{2, 1, 2, 1, 1},
	{0, 1, 1, 2, 2},	{1, 1, 1, 2, 2},	{2, 1, 1, 2, 2},	{1, 1, 2, 2, 2},
	{0, 0, 0, 2, 1},	{1, 0, 0, 2, 1},	{2, 0, 0, 2, 1},	{0, 0, 2, 2, 1},
	{0, 1, 0, 2, 1},	{1, 1, 0, 2, 1},	{2, 1, 0, 2, 1},	{1, 0, 2, 2, 1},
	{0, 2, 0, 2, 1},	{1, 2, 0, 2, 1},	{2, 2, 0, 2, 1},	{2, 0, 2, 2, 1},
	{0, 2, 2, 2, 1},	{1, 2, 2, 2, 1},	{2, 2, 2, 2, 1},	{2, 0, 2, 2, 1},
	{0, 0, 1, 2, 1},	{1, 0, 1, 2, 1},	{2, 0, 1, 2, 1},	{0, 1, 2, 2, 1},
	{0, 1, 1, 2, 1},	{1, 1, 1, 2, 1},	{2, 1, 1, 2, 1},	{1, 1, 2, 2, 1},
	{0, 2, 1, 2, 1},	{1, 2, 1, 2, 1},	{2, 2, 1, 2, 1},	{2, 1, 2, 2, 1},
	{0, 2, 1, 2, 2},	{1, 2, 1, 2, 2},	{2, 2, 1, 2, 2},	{2, 1, 2, 2, 2},
	{0, 0, 0, 1, 2},	{1, 0, 0, 1, 2},	{2, 0, 0, 1, 2},	{0, 0, 2, 1, 2},
	{0, 1, 0, 1, 2},	{1, 1, 0, 1, 2},	{2, 1, 0, 1, 2},	{1, 0, 2, 1, 2},
	{0, 2, 0, 1, 2},	{1, 2, 0, 1, 2},	{2, 2, 0, 1, 2},	{2, 0, 2, 1, 2},
	{0, 2, 2, 1, 2},	{1, 2, 2, 1, 2},	{2, 2, 2, 1, 2},	{2, 0, 2, 1, 2},
	{0, 0, 1, 1, 2},	{1, 0, 1, 1, 2},	{2, 0, 1, 1, 2},	{0, 1, 2, 1, 2},
	{0, 1, 1, 1, 2},	{1, 1, 1, 1, 2},	{2, 1, 1, 1, 2},	{1, 1, 2, 1, 2},
	{0, 2, 1, 1, 2},	{1, 2, 1, 1, 2},	{2, 2, 1, 1, 2},	{2, 1, 2, 1, 2},
	{0, 2, 2, 2, 2},	{1, 2, 2, 2, 2},	{2, 2, 2, 2, 2},	{2, 1, 2, 2, 2},
};

// packed trit-value for every unpacked trit-quintuplet
// indexed by [high][][][][low]
static const uint8_t integer_of_trits[3][3][3][3][3] = {
	{
	 {
	  {
	   {0, 1, 2,},
	   {4, 5, 6,},
	   {8, 9, 10,},
	   },
	  {
	   {16, 17, 18,},
	   {20, 21, 22,},
	   {24, 25, 26,},
	   },
	  {
	   {3, 7, 15,},
	   {19, 23, 27,},
	   {12, 13, 14,},
	   },
	  },
	 {
	  {
	   {32, 33, 34,},
	   {36, 37, 38,},
	   {40, 41, 42,},
	   },
	  {
	   {48, 49, 50,},
	   {52, 53, 54,},
	   {56, 57, 58,},
	   },
	  {
	   {35, 39, 47,},
	   {51, 55, 59,},
	   {44, 45, 46,},
	   },
	  },
	 {
	  {
	   {64, 65, 66,},
	   {68, 69, 70,},
	   {72, 73, 74,},
	   },
	  {
	   {80, 81, 82,},
	   {84, 85, 86,},
	   {88, 89, 90,},
	   },
	  {
	   {67, 71, 79,},
	   {83, 87, 91,},
	   {76, 77, 78,},
	   },
	  },
	 },
	{
	 {
	  {
	   {128, 129, 130,},
	   {132, 133, 134,},
	   {136, 137, 138,},
	   },
	  {
	   {144, 145, 146,},
	   {148, 149, 150,},
	   {152, 153, 154,},
	   },
	  {
	   {131, 135, 143,},
	   {147, 151, 155,},
	   {140, 141, 142,},
	   },
	  },
	 {
	  {
	   {160, 161, 162,},
	   {164, 165, 166,},
	   {168, 169, 170,},
	   },
	  {
	   {176, 177, 178,},
	   {180, 181, 182,},
	   {184, 185, 186,},
	   },
	  {
	   {163, 167, 175,},
	   {179, 183, 187,},
	   {172, 173, 174,},
	   },
	  },
	 {
	  {
	   {192, 193, 194,},
	   {196, 197, 198,},
	   {200, 201, 202,},
	   },
	  {
	   {208, 209, 210,},
	   {212, 213, 214,},
	   {216, 217, 218,},
	   },
	  {
	   {195, 199, 207,},
	   {211, 215, 219,},
	   {204, 205, 206,},
	   },
	  },
	 },
	{
	 {
	  {
	   {96, 97, 98,},
	   {100, 101, 102,},
	   {104, 105, 106,},
	   },
	  {
	   {112, 113, 114,},
	   {116, 117, 118,},
	   {120, 121, 122,},
	   },
	  {
	   {99, 103, 111,},
	   {115, 119, 123,},
	   {108, 109, 110,},
	   },
	  },
	 {
	  {
	   {224, 225, 226,},
	   {228, 229, 230,},
	   {232, 233, 234,},
	   },
	  {
	   {240, 241, 242,},
	   {244, 245, 246,},
	   {248, 249, 250,},
	   },
	  {
	   {227, 231, 239,},
	   {243, 247, 251,},
	   {236, 237, 238,},
	   },
	  },
	 {
	  {
	   {28, 29, 30,},
	   {60, 61, 62,},
	   {92, 93, 94,},
	   },
	  {
	   {156, 157, 158,},
	   {188, 189, 190,},
	   {220, 221, 222,},
	   },
	  {
	   {31, 63, 127,},
	   {159, 191, 255,},
	   {252, 253, 254,},
	   },
	  },
	 },
};



void find_number_of_bits_trits_quints(int quantization_level, int *bits, int *trits, int *quints)
{
	*bits = 0;
	*trits = 0;
	*quints = 0;
	switch (quantization_level)
	{
	case QUANT_2:
		*bits = 1;
		break;
	case QUANT_3:
		*bits = 0;
		*trits = 1;
		break;
	case QUANT_4:
		*bits = 2;
		break;
	case QUANT_5:
		*bits = 0;
		*quints = 1;
		break;
	case QUANT_6:
		*bits = 1;
		*trits = 1;
		break;
	case QUANT_8:
		*bits = 3;
		break;
	case QUANT_10:
		*bits = 1;
		*quints = 1;
		break;
	case QUANT_12:
		*bits = 2;
		*trits = 1;
		break;
	case QUANT_16:
		*bits = 4;
		break;
	case QUANT_20:
		*bits = 2;
		*quints = 1;
		break;
	case QUANT_24:
		*bits = 3;
		*trits = 1;
		break;
	case QUANT_32:
		*bits = 5;
		break;
	case QUANT_40:
		*bits = 3;
		*quints = 1;
		break;
	case QUANT_48:
		*bits = 4;
		*trits = 1;
		break;
	case QUANT_64:
		*bits = 6;
		break;
	case QUANT_80:
		*bits = 4;
		*quints = 1;
		break;
	case QUANT_96:
		*bits = 5;
		*trits = 1;
		break;
	case QUANT_128:
		*bits = 7;
		break;
	case QUANT_160:
		*bits = 5;
		*quints = 1;
		break;
	case QUANT_192:
		*bits = 6;
		*trits = 1;
		break;
	case QUANT_256:
		*bits = 8;
		break;
	}
}


// routine to write up to 8 bits
static inline void write_bits(int value, int bitcount, int bitoffset, uint8_t * ptr)
{
	int mask = (1 << bitcount) - 1;
	value &= mask;
	ptr += bitoffset >> 3;
	bitoffset &= 7;
	value <<= bitoffset;
	mask <<= bitoffset;
	mask = ~mask;

	ptr[0] &= mask;
	ptr[0] |= value;
	ptr[1] &= mask >> 8;
	ptr[1] |= value >> 8;
}


// routine to read up to 8 bits
static inline int read_bits(int bitcount, int bitoffset, const uint8_t * ptr)
{
	int mask = (1 << bitcount) - 1;
	ptr += bitoffset >> 3;
	bitoffset &= 7;
	int value = ptr[0] | (ptr[1] << 8);
	value >>= bitoffset;
	value &= mask;
	return value;
}




void encode_ise(int quantization_level, int elements, const uint8_t * input_data, uint8_t * output_data, int bit_offset)
{
	int i;
	uint8_t lowparts[64];
	uint8_t highparts[69];		// 64 elements + 5 elements for padding
	uint8_t tq_blocks[22];		// trit-blocks or quint-blocks

	int bits, trits, quints;
	find_number_of_bits_trits_quints(quantization_level, &bits, &trits, &quints);

	for (i = 0; i < elements; i++)
	{
		lowparts[i] = input_data[i] & ((1 << bits) - 1);
		highparts[i] = input_data[i] >> bits;
	}
	for (i = elements; i < elements + 5; i++)
		highparts[i] = 0;		// padding before we start constructing trit-blocks or quint-blocks

	// construct trit-blocks or quint-blocks as necessary
	if (trits)
	{
		int trit_blocks = (elements + 4) / 5;
		for (i = 0; i < trit_blocks; i++)
			tq_blocks[i] = integer_of_trits[highparts[5 * i + 4]][highparts[5 * i + 3]][highparts[5 * i + 2]][highparts[5 * i + 1]][highparts[5 * i]];
	}
	if (quints)
	{
		int quint_blocks = (elements + 2) / 3;
		for (i = 0; i < quint_blocks; i++)
			tq_blocks[i] = integer_of_quints[highparts[3 * i + 2]][highparts[3 * i + 1]][highparts[3 * i]];
	}

	// then, write out the actual bits.
	int lcounter = 0;
	int hcounter = 0;
	for (i = 0; i < elements; i++)
	{
		write_bits(lowparts[i], bits, bit_offset, output_data);
		bit_offset += bits;
		if (trits)
		{
			static const int bits_to_write[5] = { 2, 2, 1, 2, 1 };
			static const int block_shift[5] = { 0, 2, 4, 5, 7 };
			static const int next_lcounter[5] = { 1, 2, 3, 4, 0 };
			static const int hcounter_incr[5] = { 0, 0, 0, 0, 1 };
			write_bits(tq_blocks[hcounter] >> block_shift[lcounter], bits_to_write[lcounter], bit_offset, output_data);
			bit_offset += bits_to_write[lcounter];
			hcounter += hcounter_incr[lcounter];
			lcounter = next_lcounter[lcounter];
		}
		if (quints)
		{
			static const int bits_to_write[3] = { 3, 2, 2 };
			static const int block_shift[3] = { 0, 3, 5 };
			static const int next_lcounter[3] = { 1, 2, 0 };
			static const int hcounter_incr[3] = { 0, 0, 1 };
			write_bits(tq_blocks[hcounter] >> block_shift[lcounter], bits_to_write[lcounter], bit_offset, output_data);
			bit_offset += bits_to_write[lcounter];
			hcounter += hcounter_incr[lcounter];
			lcounter = next_lcounter[lcounter];
		}
	}
}




void decode_ise(int quantization_level, int elements, const uint8_t * input_data, uint8_t * output_data, int bit_offset)
{
	int i;
	// note: due to how the trit/quint-block unpacking is done in this function,
	// we may write more temporary results than the number of outputs
	// The maximum actual number of results is 64 bit, but we keep 4 additional elements
	// of padding.
	uint8_t results[68];
	uint8_t tq_blocks[22];		// trit-blocks or quint-blocks

	int bits, trits, quints;
	find_number_of_bits_trits_quints(quantization_level, &bits, &trits, &quints);

	int lcounter = 0;
	int hcounter = 0;

	// trit-blocks or quint-blocks must be zeroed out before we collect them in the loop below.
	for (i = 0; i < 22; i++)
		tq_blocks[i] = 0;

	// collect bits for each element, as well as bits for any trit-blocks and quint-blocks.
	for (i = 0; i < elements; i++)
	{
		results[i] = read_bits(bits, bit_offset, input_data);
		bit_offset += bits;
		if (trits)
		{
			static const int bits_to_read[5] = { 2, 2, 1, 2, 1 };
			static const int block_shift[5] = { 0, 2, 4, 5, 7 };
			static const int next_lcounter[5] = { 1, 2, 3, 4, 0 };
			static const int hcounter_incr[5] = { 0, 0, 0, 0, 1 };
			int tdata = read_bits(bits_to_read[lcounter], bit_offset, input_data);
			bit_offset += bits_to_read[lcounter];
			tq_blocks[hcounter] |= tdata << block_shift[lcounter];
			hcounter += hcounter_incr[lcounter];
			lcounter = next_lcounter[lcounter];
		}
		if (quints)
		{
			static const int bits_to_read[3] = { 3, 2, 2 };
			static const int block_shift[3] = { 0, 3, 5 };
			static const int next_lcounter[3] = { 1, 2, 0 };
			static const int hcounter_incr[3] = { 0, 0, 1 };
			int tdata = read_bits(bits_to_read[lcounter], bit_offset, input_data);
			bit_offset += bits_to_read[lcounter];
			tq_blocks[hcounter] |= tdata << block_shift[lcounter];
			hcounter += hcounter_incr[lcounter];
			lcounter = next_lcounter[lcounter];
		}
	}


	// unpack trit-blocks or quint-blocks as needed
	if (trits)
	{
		int trit_blocks = (elements + 4) / 5;
		for (i = 0; i < trit_blocks; i++)
		{
			const uint8_t *tritptr = trits_of_integer[tq_blocks[i]];
			results[5 * i] |= tritptr[0] << bits;
			results[5 * i + 1] |= tritptr[1] << bits;
			results[5 * i + 2] |= tritptr[2] << bits;
			results[5 * i + 3] |= tritptr[3] << bits;
			results[5 * i + 4] |= tritptr[4] << bits;
		}
	}

	if (quints)
	{
		int quint_blocks = (elements + 2) / 3;
		for (i = 0; i < quint_blocks; i++)
		{
			const uint8_t *quintptr = quints_of_integer[tq_blocks[i]];
			results[3 * i] |= quintptr[0] << bits;
			results[3 * i + 1] |= quintptr[1] << bits;
			results[3 * i + 2] |= quintptr[2] << bits;
		}
	}

	for (i = 0; i < elements; i++)
		output_data[i] = results[i];
}




int compute_ise_bitcount(int items, quantization_method quant)
{
	switch (quant)
	{
	case QUANT_2:
		return items;
	case QUANT_3:
		return (8 * items + 4) / 5;
	case QUANT_4:
		return 2 * items;
	case QUANT_5:
		return (7 * items + 2) / 3;
	case QUANT_6:
		return (13 * items + 4) / 5;
	case QUANT_8:
		return 3 * items;
	case QUANT_10:
		return (10 * items + 2) / 3;
	case QUANT_12:
		return (18 * items + 4) / 5;
	case QUANT_16:
		return items * 4;
	case QUANT_20:
		return (13 * items + 2) / 3;
	case QUANT_24:
		return (23 * items + 4) / 5;
	case QUANT_32:
		return 5 * items;
	case QUANT_40:
		return (16 * items + 2) / 3;
	case QUANT_48:
		return (28 * items + 4) / 5;
	case QUANT_64:
		return 6 * items;
	case QUANT_80:
		return (19 * items + 2) / 3;
	case QUANT_96:
		return (33 * items + 4) / 5;
	case QUANT_128:
		return 7 * items;
	case QUANT_160:
		return (22 * items + 2) / 3;
	case QUANT_192:
		return (38 * items + 4) / 5;
	case QUANT_256:
		return 8 * items;
	default:
		return 100000;
	}
}
