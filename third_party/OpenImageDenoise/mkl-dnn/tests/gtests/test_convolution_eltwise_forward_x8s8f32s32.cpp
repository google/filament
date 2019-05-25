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
#include "math_utils.hpp"
#include "mkldnn.hpp"
#include "test_convolution_eltwise_forward_common.hpp"

namespace mkldnn {

using convolution_test_u8s8s32f32 =
        convolution_eltwise_test<uint8_t, int8_t, int32_t, float>;
using convolution_test_s8s8s32f32 =
        convolution_eltwise_test<int8_t, int8_t, int32_t, float>;

#define EXPAND_FORMATS(src, weights, bias, dst) \
    { mkldnn::memory::format_tag::src, mkldnn::memory::format_tag::weights, \
    mkldnn::memory::format_tag::bias, mkldnn::memory::format_tag::dst }

#define CONCAT_WITH_UNDERSCORE_(a,b) a ## _ ## b
#define CONCAT_WITH_UNDERSCORE(a,b) CONCAT_WITH_UNDERSCORE_(a,b)

#define INST_TEST_CASE_(str, test, ...) INSTANTIATE_TEST_SUITE_P( \
        str, test, ::testing::Values(__VA_ARGS__))

#define INST_TEST_CASE(str, test, ...) INST_TEST_CASE_( \
        CONCAT_WITH_UNDERSCORE(CONCAT_WITH_UNDERSCORE(Convolution, \
        str), eltwise), test,  __VA_ARGS__)

#define EXPAND_ARGS(args) args

#define PARAMS(...) \
    EXPAND_ARGS(PARAMS_CONV(eltwise_relu, __VA_ARGS__)), \
    EXPAND_ARGS(PARAMS_CONV(eltwise_elu, __VA_ARGS__)), \
    EXPAND_ARGS(PARAMS_CONV(eltwise_tanh, __VA_ARGS__)), \
    EXPAND_ARGS(PARAMS_CONV(eltwise_square, __VA_ARGS__)), \
    EXPAND_ARGS(PARAMS_CONV(eltwise_abs, __VA_ARGS__)), \
    EXPAND_ARGS(PARAMS_CONV(eltwise_sqrt, __VA_ARGS__)), \
    EXPAND_ARGS(PARAMS_CONV(eltwise_linear, __VA_ARGS__)), \
    EXPAND_ARGS(PARAMS_CONV(eltwise_bounded_relu, __VA_ARGS__)), \
    EXPAND_ARGS(PARAMS_CONV(eltwise_soft_relu, __VA_ARGS__)), \
    EXPAND_ARGS(PARAMS_CONV(eltwise_logistic, __VA_ARGS__))

#define ELTWISE_ALPHA 0.5f
#define ELTWISE_BETA 0.f

#define PARAMS_CONV(alg, src, weights, bias, dst, ...) \
    test_convolution_eltwise_params_t {alg,  mkldnn::engine::kind::cpu, \
        mkldnn::convolution_direct, ELTWISE_ALPHA, ELTWISE_BETA, \
    EXPAND_FORMATS(src, weights, bias, dst), /* empty attributes */ {}, \
    {__VA_ARGS__} }

#define INST_TEST_CASE_P(test) \
TEST_P(test, TestConvolutionEltwise) {} \
INST_TEST_CASE(SimpleSmall_Blocked16, test, \
PARAMS(nhwc, OIhw4i16o4i, x, nhwc, 2, 1, 32, 13, 13, 32, 12, 12, 3, 3, 0, 0, 1, 1), \
PARAMS(nhwc, Goihw16g, x, nhwc, 2, 32, 32, 13, 13, 32, 13, 13, 1, 1, 0, 0, 1, 1), \
PARAMS(nhwc, OIhw4i16o4i, x, nhwc, 2, 1, 32, 13, 13, 32, 13, 13, 3, 3, 1, 1, 1, 1) \
);

INST_TEST_CASE_P(convolution_test_s8s8s32f32);
INST_TEST_CASE_P(convolution_test_u8s8s32f32);
}
