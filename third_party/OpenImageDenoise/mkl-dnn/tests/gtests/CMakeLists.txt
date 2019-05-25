#===============================================================================
# Copyright 2016-2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#===============================================================================

add_subdirectory (gtest)

set(APP_NAME "gtest")
set(MAIN_SRC_GTEST main.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/gtest
                    ${CMAKE_CURRENT_SOURCE_DIR}/in
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/common
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpu
                    )
if(WIN32)
    # Correct 'jnl' macro/jit issue
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
        append(CMAKE_CXX_FLAGS "/Qlong-double")
    endif()
endif()

# TODO: enable me!
file(GLOB PRIM_TEST_CASES_SRC
                              test_iface_pd_iter.cpp
                              test_iface_attr.cpp
                              test_mkldnn_threading.cpp
                              test_memory.cpp
                              test_sum.cpp
                              test_reorder.cpp
                              test_concat.cpp
                              test_softmax_forward.cpp
                              test_softmax_backward.cpp
                              test_eltwise.cpp
                              test_lrn_forward.cpp
                              test_lrn_backward.cpp
                              test_pooling_forward.cpp
                              test_pooling_backward.cpp
                              test_batch_normalization_f32.cpp
                              test_batch_normalization_s8.cpp
                              test_inner_product_forward.cpp
                              test_inner_product_backward_data.cpp
                              test_inner_product_backward_weights.cpp
                              test_shuffle.cpp
                              test_convolution_format_any.cpp
                              test_convolution_forward_f32.cpp
                              test_convolution_forward_u8s8s32.cpp
                              test_convolution_forward_u8s8fp.cpp
                              test_convolution_eltwise_forward_f32.cpp
                              test_convolution_eltwise_forward_x8s8f32s32.cpp
                              test_convolution_backward_data_f32.cpp
                              test_convolution_backward_weights_f32.cpp
                              test_deconvolution.cpp
                              test_gemm_f32.cpp
                              test_gemm_s8u8s32.cpp
                              test_gemm_s8s8s32.cpp
                              test_rnn_forward.cpp
                              )

foreach(TEST_FILE ${PRIM_TEST_CASES_SRC})
    get_filename_component(TEST_FILE_WE ${TEST_FILE} NAME_WE)
    set(CURR_APP_NAME ${TEST_FILE_WE})
    register_exe(${CURR_APP_NAME} "${MAIN_SRC_GTEST};${TEST_FILE}" "test" "mkldnn_gtest")
endforeach()

# vim: et ts=4 sw=4
