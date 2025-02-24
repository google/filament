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
 * @brief Data tables for quantization transfer.
 */

#include "astc_codec_internals.h"

#define _ 0 // using _ to indicate an entry that will not be used.

const quantization_and_transfer_table quant_and_xfer_tables[12] = {
	// quantization method 0, range 0..1
	{
		{0, 64},
	},
	// quantization method 1, range 0..2
	{
		{0, 32, 64},
	},
	// quantization method 2, range 0..3
	{
		{0, 21, 43, 64},
	},
	// quantization method 3, range 0..4
	{
		{0, 16, 32, 48, 64},
	},
	// quantization method 4, range 0..5
	{
		{0, 64, 12, 52, 25, 39},
	},
	// quantization method 5, range 0..7
	{
		{0, 9, 18, 27, 37, 46, 55, 64},
	},
	// quantization method 6, range 0..9
	{
		{0, 64, 7, 57, 14, 50, 21, 43, 28, 36},
	},
	// quantization method 7, range 0..11
	{
		{0, 64, 17, 47, 5, 59, 23, 41, 11, 53, 28, 36},
	},
	// quantization method 8, range 0..15
	{
		{0, 4, 8, 12, 17, 21, 25, 29, 35, 39, 43, 47, 52, 56, 60, 64},
	},
	// quantization method 9, range 0..19
	{
		{0, 64, 16, 48, 3, 61, 19, 45, 6, 58, 23, 41, 9, 55, 26, 38, 13, 51,
		 29, 35},
	},
	// quantization method 10, range 0..23
	{
		{0, 64, 8, 56, 16, 48, 24, 40, 2, 62, 11, 53, 19, 45, 27, 37, 5, 59,
		 13, 51, 22, 42, 30, 34},
	},
	// quantization method 11, range 0..31
	{
		{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 34, 36, 38,
		 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64},
	}
};
