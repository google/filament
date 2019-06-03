project(mkldnn)

set(PROJECT_FULL_NAME "Intel(R) Math Kernel Library for Deep Neural Networks (Intel(R) MKL-DNN)")
set(PROJECT_VERSION "0.90.0")

string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})
list(GET VERSION_LIST 0 MKLDNN_VERSION_MAJOR)
list(GET VERSION_LIST 1 MKLDNN_VERSION_MINOR)
list(GET VERSION_LIST 2 MKLDNN_VERSION_PATCH)
configure_file(
    "${PROJECT_SOURCE_DIR}/../include/mkldnn_version.h.in"
    "${PROJECT_BINARY_DIR}/include/mkldnn_version.h"
    )

file(GLOB_RECURSE HEADERS
    ${PROJECT_BINARY_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/*.hpp
    )

include_directories(
    ${PROJECT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/cpu
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/cpu/xbyak
    )

file(GLOB_RECURSE SOURCES
    ../src/common/*.cpp
    ../src/cpu/cpu_barrier.cpp
    ../src/cpu/cpu_concat.cpp
    ../src/cpu/cpu_engine.cpp
    ../src/cpu/cpu_memory.cpp
    ../src/cpu/cpu_primitive.cpp
    ../src/cpu/cpu_reducer.cpp
    ../src/cpu/cpu_reorder.cpp
    ../src/cpu/cpu_sum.cpp
    ../src/cpu/jit_avx2_conv_kernel_f32.cpp
    ../src/cpu/jit_avx2_convolution.cpp
    ../src/cpu/jit_avx512_common_conv_kernel.cpp
    ../src/cpu/jit_avx512_common_convolution.cpp
    ../src/cpu/jit_avx512_common_convolution_winograd.cpp
    ../src/cpu/jit_avx512_common_conv_winograd_kernel_f32.cpp
    ../src/cpu/jit_avx512_core_fp32_wino_conv_2x3.cpp
    ../src/cpu/jit_avx512_core_fp32_wino_conv_4x3.cpp
    ../src/cpu/jit_avx512_core_fp32_wino_conv_4x3_kernel.cpp
    ../src/cpu/jit_sse42_conv_kernel_f32.cpp
    ../src/cpu/jit_sse42_convolution.cpp
    ../src/cpu/jit_transpose_src_utils.cpp
    ../src/cpu/jit_uni_eltwise.cpp
    ../src/cpu/jit_uni_pooling.cpp
    ../src/cpu/jit_uni_pool_kernel_f32.cpp
    ../src/cpu/jit_uni_reorder.cpp
    ../src/cpu/jit_uni_reorder_utils.cpp
    ../src/cpu/jit_utils/jitprofiling/jitprofiling.c
    ../src/cpu/jit_utils/jit_utils.cpp
    )

add_library(${PROJECT_NAME} ${MKLDNN_LIBRARY_TYPE} ${HEADERS} ${SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION "${PROJECT_VERSION}.0")
set_property(TARGET ${PROJECT_NAME} PROPERTY SOVERSION "0")
set_property(TARGET ${PROJECT_NAME} PROPERTY PUBLIC_HEADER ${HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${ISA_FLAGS_SSE41}")

target_link_libraries(${PROJECT_NAME} PUBLIC ${TBB_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
    )
