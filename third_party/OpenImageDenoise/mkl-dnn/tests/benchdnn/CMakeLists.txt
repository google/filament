#===============================================================================
# Copyright 2017-2018 Intel Corporation
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
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/*.hpp
    )
file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/conv
    ${CMAKE_CURRENT_SOURCE_DIR}/ip
    ${CMAKE_CURRENT_SOURCE_DIR}/shuffle
    ${CMAKE_CURRENT_SOURCE_DIR}/reorder
    )

if(BENCHDNN_USE_RDPMC)
    add_definitions(-DBENCHDNN_USE_RDPMC)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    append_if(WIN32 CMAKE_CXX_FLAGS "-Qprec-div -Qprec-sqrt")
    append_if(UNIX  CMAKE_CXX_FLAGS "-prec-div -prec-sqrt -fp-model precise")
endif()

if(UNIX AND NOT APPLE)
    find_library(LIBRT rt)
endif()
register_exe(benchdnn "${SOURCES}" "" "${LIBRT}")

file(COPY inputs DESTINATION .)

# register benchdnn test case
function(register_benchdnn_test name cmd)
    string(REPLACE " " ";" cmd ${cmd})
    string(REPLACE " " ";" ARGV2 "${ARGV2}")
    add_custom_target(${name}
        COMMAND ${cmd}
        COMMAND ${ARGV2} # if any
        DEPENDS benchdnn
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    maybe_configure_windows_test(${name} TARGET)
endfunction()

register_benchdnn_test(test_conv "benchdnn -v1 --conv --batch=inputs/test_conv_all")
register_benchdnn_test(test_benchdnn_conv "benchdnn -v1 --conv --batch=inputs/test_conv_all")
register_benchdnn_test(test_benchdnn_deconv "benchdnn -v1 --deconv --batch=inputs/deconv/test_deconv_all")
register_benchdnn_test(test_benchdnn_rnn "benchdnn -v1 --rnn --batch=inputs/rnn/test_rnn_small")
register_benchdnn_test(test_benchdnn_reorder "benchdnn --reorder --batch=inputs/reorder/test_default")
register_benchdnn_test(test_benchdnn_bnorm "benchdnn --bnorm  --batch=inputs/bnorm/test_bnorm_topo")
register_benchdnn_test(test_benchdnn_ip "benchdnn --ip --batch=inputs/ip/test_ip_all")
register_benchdnn_test(test_benchdnn_regression
    "benchdnn --conv --batch=inputs/test_conv_regression"
    "benchdnn --bnorm --batch=inputs/bnorm/test_bnorm_regressions"
    )
register_benchdnn_test(test_benchdnn_regression_large
    "benchdnn --bnorm --batch=inputs/bnorm/test_bnorm_regressions_large"
    )
