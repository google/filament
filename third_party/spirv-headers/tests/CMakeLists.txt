# Copyright (c) 2015-2024 The Khronos Group Inc.
# SPDX-License-Identifier: MIT
#
# MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
# KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
# SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
#    https://www.khronos.org/registry/

add_executable(spirv_headers_simple_test example.cpp)
target_compile_definitions(spirv_headers_simple_test PRIVATE SPV_ENABLE_UTILITY_CODE)
target_link_libraries(spirv_headers_simple_test PRIVATE SPIRV-Headers::SPIRV-Headers)
add_test(NAME spirv_headers_simple_test COMMAND spirv_headers_simple_test)

add_executable(spirv_headers_simple_test_cpp11 example11.cpp)
target_compile_definitions(spirv_headers_simple_test_cpp11 PRIVATE SPV_ENABLE_UTILITY_CODE)
target_link_libraries(spirv_headers_simple_test_cpp11 PRIVATE SPIRV-Headers::SPIRV-Headers)
set_target_properties(spirv_headers_simple_test_cpp11 PROPERTIES CXX_STANDARD 11)
add_test(NAME spirv_headers_simple_test_cpp11 COMMAND spirv_headers_simple_test_cpp11)

add_executable(spirv_headers_simple_test_c example.c)
target_compile_definitions(spirv_headers_simple_test_c PRIVATE SPV_ENABLE_UTILITY_CODE)
target_link_libraries(spirv_headers_simple_test_c PRIVATE SPIRV-Headers::SPIRV-Headers)
set_target_properties(spirv_headers_simple_test_c PROPERTIES C_STANDARD 99 LINKER_LANGUAGE C)
add_test(NAME spirv_headers_simple_test_c COMMAND spirv_headers_simple_test_c)

if (NOT TARGET SPIRV-Headers)
    message(FATAL_ERROR "SPIRV-Headers target not defined!")
endif()

if (NOT PROJECT_NAME STREQUAL "SPIRV-Headers")
    message(FATAL_ERROR "PROJECT_NAME changed!") # Needed for backcompat with SPIRV-Tools.
endif()

if (NOT DEFINED SPIRV-Headers_SOURCE_DIR)
    message(FATAL_ERROR "SPIRV-Headers_SOURCE_DIR not defined!") # Needed for backcompat with SPIRV-Tools.
endif()
