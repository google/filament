/*******************************************************************************
* Copyright 2018 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include "mkldnn_test_common.hpp"
#include "gtest/gtest.h"

#include "mkldnn.h"
#include "test_gemm_common.hpp"

namespace mkldnn {

using gemm_test = gemm_test_common<int8_t, uint8_t, int32_t>;

TEST_P(gemm_test, TestGEMM)
{}

#define TEST_CASE_NAME_PREFIX s8u8s32
#define S8U8S32
#include "gemm_in.h"
}
