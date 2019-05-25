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

# Locates Doxygen and configures documentation generation
#===============================================================================

if(Doxygen_cmake_included)
    return()
endif()
set(Doxygen_cmake_included true)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/reference)
    set(DOXYGEN_STAMP_FILE ${CMAKE_CURRENT_BINARY_DIR}/doc.stamp)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/doc/header.html.in
        ${CMAKE_CURRENT_BINARY_DIR}/header.html
        @ONLY)
    file(GLOB_RECURSE HEADERS
        ${PROJECT_SOURCE_DIR}/include/*.h
        ${PROJECT_SOURCE_DIR}/include/*.hpp
        )
    file(GLOB_RECURSE DOX
        ${PROJECT_SOURCE_DIR}/doc/*
        )
    add_custom_command(
        OUTPUT ${DOXYGEN_STAMP_FILE}
        DEPENDS ${HEADERS} ${DOX}
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
        COMMAND cmake -E touch ${DOXYGEN_STAMP_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM)
    add_custom_target(doc DEPENDS ${DOXYGEN_STAMP_FILE})
    install(
        DIRECTORY ${DOXYGEN_OUTPUT_DIR}
        DESTINATION share/doc/${LIB_NAME} OPTIONAL)
endif(DOXYGEN_FOUND)


