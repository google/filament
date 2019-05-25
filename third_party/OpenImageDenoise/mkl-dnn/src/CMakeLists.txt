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

file(GLOB_RECURSE HEADERS
    ${PROJECT_BINARY_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/*.hpp
    )
#file(GLOB_RECURSE SOURCES
#    ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/cpu/*.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/cpu/*.c
#    )
include_directories(
    ${PROJECT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu/xbyak
    )

# Add only files that are actually used
file(GLOB_RECURSE SOURCES
    common/*.cpp
    cpu/cpu_barrier.cpp
    #cpu/cpu_batch_normalization_utils.cpp
    cpu/cpu_concat.cpp
    cpu/cpu_engine.cpp
    cpu/cpu_memory.cpp
    cpu/cpu_primitive.cpp
    cpu/cpu_reducer.cpp
    cpu/cpu_reorder.cpp
    cpu/cpu_sum.cpp
    #cpu/gemm_convolution.cpp
    #cpu/gemm_convolution_utils.cpp
    #cpu/gemm/gemm.cpp
    #cpu/gemm/gemm_utils.cpp
    #cpu/gemm_inner_product.cpp
    #cpu/gemm/jit_avx512_common_gemm_f32.cpp
    #cpu/gemm/jit_avx_gemm_f32.cpp
    #cpu/gemm/ref_gemm.cpp
    #cpu/gemm_u8s8s32x_inner_product.cpp
    #cpu/gemm_x8s8s32x_convolution.cpp
    #cpu/jit_avx2_1x1_conv_kernel_f32.cpp
    #cpu/jit_avx2_1x1_convolution.cpp
    cpu/jit_avx2_conv_kernel_f32.cpp
    cpu/jit_avx2_convolution.cpp
    #cpu/jit_avx512_common_1x1_conv_kernel.cpp
    #cpu/jit_avx512_common_1x1_convolution.cpp
    cpu/jit_avx512_common_conv_kernel.cpp
    cpu/jit_avx512_common_convolution.cpp
    cpu/jit_avx512_common_convolution_winograd.cpp
    cpu/jit_avx512_common_conv_winograd_kernel_f32.cpp
    #cpu/jit_avx512_common_lrn.cpp
    cpu/jit_avx512_core_fp32_wino_conv_2x3.cpp
    cpu/jit_avx512_core_fp32_wino_conv_4x3.cpp
    cpu/jit_avx512_core_fp32_wino_conv_4x3_kernel.cpp
    #cpu/jit_avx512_core_i8i8_pooling.cpp
    #cpu/jit_avx512_core_u8s8s32x_deconvolution.cpp
    #cpu/jit_avx512_core_u8s8s32x_wino_convolution.cpp
    #cpu/jit_avx512_core_x8s8s32x_1x1_conv_kernel.cpp
    #cpu/jit_avx512_core_x8s8s32x_1x1_convolution.cpp
    #cpu/jit_avx512_core_x8s8s32x_conv_kernel.cpp
    #cpu/jit_avx512_core_x8s8s32x_convolution.cpp
    #cpu/jit_sse42_1x1_conv_kernel_f32.cpp
    #cpu/jit_sse42_1x1_convolution.cpp
    cpu/jit_sse42_conv_kernel_f32.cpp
    cpu/jit_sse42_convolution.cpp
    cpu/jit_transpose_src_utils.cpp
    #cpu/jit_uni_batch_normalization.cpp
    #cpu/jit_uni_dw_conv_kernel_f32.cpp
    #cpu/jit_uni_dw_convolution.cpp
    cpu/jit_uni_eltwise.cpp
    #cpu/jit_uni_lrn.cpp
    #cpu/jit_uni_lrn_kernel_f32.cpp
    cpu/jit_uni_pooling.cpp
    cpu/jit_uni_pool_kernel_f32.cpp
    cpu/jit_uni_reorder.cpp
    cpu/jit_uni_reorder_utils.cpp
    cpu/jit_utils/jitprofiling/jitprofiling.c
    cpu/jit_utils/jit_utils.cpp
    #cpu/nchw_pooling.cpp
    #cpu/ncsp_batch_normalization.cpp
    #cpu/nhwc_pooling.cpp
    #cpu/nspc_batch_normalization.cpp
    #cpu/ref_batch_normalization.cpp
    #cpu/ref_convolution.cpp
    #cpu/ref_deconvolution.cpp
    #cpu/ref_eltwise.cpp
    #cpu/ref_inner_product.cpp
    #cpu/ref_lrn.cpp
    #cpu/ref_pooling.cpp
    #cpu/ref_rnn.cpp
    #cpu/ref_shuffle.cpp
    #cpu/ref_softmax.cpp
    #cpu/simple_concat.cpp
    #cpu/simple_sum.cpp
    )

# propagate SRC specific flags
append(CMAKE_C_FLAGS "${CMAKE_SRC_CCXX_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_SRC_CCXX_FLAGS}")

# propagate no warning flags
append(CMAKE_C_FLAGS "${CMAKE_CCXX_NOWARN_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_CCXX_NOWARN_FLAGS}")

# propagate sanitizer flags
append(CMAKE_C_FLAGS "${CMAKE_CCXX_SANITIZER_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_CCXX_SANITIZER_FLAGS}")

if(NOT MKLDNN_VERBOSE)
    add_definitions(-DDISABLE_VERBOSE)
endif()

if(MKLDNN_ENABLE_CONCURRENT_EXEC)
    add_definitions(-DMKLDNN_ENABLE_CONCURRENT_EXEC)
endif()

if(NOT MKLDNN_ENABLE_JIT_PROFILING)
    # XXX: the profiling interface will still be built and present in the
    # library
    message(STATUS "VTune JIT profiling disabled")
    add_definitions(-DMKLDNN_ENABLE_JIT_PROFILING=0)
elseif(UNIX AND NOT APPLE)
    # Not every compiler adds -ldl automatically
    list(APPEND EXTRA_SHARED_LIBS "${CMAKE_DL_LIBS}")
endif()

if(WIN32)
    add_definitions(-D_WIN)
    add_definitions(-DNOMINMAX)
    # Correct 'jnl' macro/jit issue
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
        append(CMAKE_CXX_FLAGS "/Qlong-double")
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    # to make computations more stable and to align the jitted code
    # with the reference one use precise division and square root
    # by default
    file(GLOB FILES_REQUIRED_PREC_SQRT
        ${CMAKE_CURRENT_SOURCE_DIR}/cpu/*normalization*.cpp)
    file(GLOB FILES_REQUIRED_PREC_DIV
        ${CMAKE_CURRENT_SOURCE_DIR}/cpu/*normalization*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cpu/ref_rnn.cpp)
    if(WIN32)
        set_source_files_properties(${FILES_REQUIRED_PREC_SQRT}
            PROPERTIES COMPILE_FLAGS "/Qprec-sqrt")
        set_source_files_properties(${FILES_REQUIRED_PREC_DIV}
            PROPERTIES COMPILE_FLAGS "/Qprec-div")
    else()
        set_source_files_properties(${FILES_REQUIRED_PREC_SQRT}
            PROPERTIES COMPILE_FLAGS "-prec-sqrt")
        set_source_files_properties(${FILES_REQUIRED_PREC_DIV}
            PROPERTIES COMPILE_FLAGS "-prec-div")
    endif()
endif()

add_library(${LIB_NAME} ${MKLDNN_LIBRARY_TYPE} ${HEADERS} ${SOURCES})
set_property(TARGET ${LIB_NAME} PROPERTY VERSION "${PROJECT_VERSION}.0")
set_property(TARGET ${LIB_NAME} PROPERTY SOVERSION "0")
set_property(TARGET ${LIB_NAME} PROPERTY PUBLIC_HEADER ${HEADERS})

# Compile the library for SSE4.1
set_target_properties(${LIB_NAME} PROPERTIES COMPILE_FLAGS "${ISA_FLAGS_SSE41}")

target_include_directories(${LIB_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../include>
    # $<INSTALL_PREFIX> is required for compatibility with cmake 2.8
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries_build(${LIB_NAME}
    "${EXTRA_SHARED_LIBS};${EXTRA_STATIC_LIBS}")
target_link_libraries_install(${LIB_NAME} "${EXTRA_SHARED_LIBS}")
if(MKLDNN_LIBRARY_TYPE STREQUAL "STATIC")
    target_link_libraries_install(${LIB_NAME} "${EXTRA_STATIC_LIBS}")
endif()

set(LIB_EXPORT_NAME "${LIB_NAME}-targets")
install(TARGETS ${LIB_NAME}
    EXPORT "${LIB_EXPORT_NAME}"
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# Write version and package config files
set(LIB_CONFIG_GENERATE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(LIB_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME}")
set(LIB_VERSION_FILE
    "${LIB_CONFIG_GENERATE_DIR}/${LIB_NAME}-config-version.cmake")
set(LIB_CONFIG_FILE
    "${LIB_CONFIG_GENERATE_DIR}/${LIB_NAME}-config.cmake")
write_basic_package_version_file(
    "${LIB_VERSION_FILE}"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)
configure_package_config_file(
    "../cmake/config.cmake.in"
    "${LIB_CONFIG_FILE}"
    INSTALL_DESTINATION ${LIB_CONFIG_INSTALL_DIR})
install(FILES ${LIB_CONFIG_FILE} ${LIB_VERSION_FILE}
    DESTINATION ${LIB_CONFIG_INSTALL_DIR})
string(TOUPPER "${LIB_NAME}::" LIB_NAMESPACE)
install(EXPORT ${LIB_EXPORT_NAME}
    NAMESPACE ${LIB_NAMESPACE}
    DESTINATION ${LIB_CONFIG_INSTALL_DIR})

# On Windows we need to add mkldnn.dll path to CTESTCONFIG_PATH which is later
# passed to ctest and Visual Studio solutions
if(WIN32)
    if(NOT(MINGW))
        foreach(BUILD_TYPE Release Debug RelWithDebInfo MinSizeRel)
            append_to_windows_path_list(CTESTCONFIG_PATH
                "${CMAKE_CURRENT_BINARY_DIR}/${BUILD_TYPE}")
        endforeach()
    else()
        append_to_windows_path_list(CTESTCONFIG_PATH
            "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    set(CTESTCONFIG_PATH "${CTESTCONFIG_PATH}" PARENT_SCOPE)
endif()
