# Copyright 2017 Google Inc. All rights reserved.
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

set(LIBXML2_TARGET external.libxml2)
set(LIBXML2_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${LIBXML2_TARGET})
set(LIBXML2_SRC_DIR ${LIBXML2_INSTALL_DIR}/src/${LIBXML2_TARGET})

set(LIBXML2_INCLUDE_DIRS ${LIBXML2_INSTALL_DIR}/include/libxml2)
include_directories(${LIBXML2_INCLUDE_DIRS})

list(APPEND LIBXML2_LIBRARIES xml2)

foreach(lib IN LISTS LIBXML2_LIBRARIES)
  list(APPEND LIBXML2_BUILD_BYPRODUCTS ${LIBXML2_INSTALL_DIR}/lib/lib${lib}.a)

  add_library(${lib} STATIC IMPORTED)
  set_property(TARGET ${lib} PROPERTY IMPORTED_LOCATION
               ${LIBXML2_INSTALL_DIR}/lib/lib${lib}.a)
  add_dependencies(${lib} ${LIBXML2_TARGET})
endforeach(lib)

include (ExternalProject)
ExternalProject_Add(${LIBXML2_TARGET}
    PREFIX ${LIBXML2_TARGET}
    GIT_REPOSITORY GIT_REPOSITORY https://gitlab.gnome.org/GNOME/libxml2
    GIT_TAG master
    UPDATE_COMMAND ""
    CMAKE_CACHE_ARGS -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
                     -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
                     -DCMAKE_C_COMPILER_LAUNCHER:FILEPATH=${CMAKE_C_COMPILER_LAUNCHER}
                     -DCMAKE_CXX_COMPILER_LAUNCHER:FILEPATH=${CMAKE_CXX_COMPILER_LAUNCHER}
    CMAKE_ARGS -DCMAKE_C_FLAGS=${LIBXML2_CFLAGS} -DCMAKE_CXX_FLAGS=${LIBXML2_CXXFLAGS}
               -DCMAKE_INSTALL_PREFIX=${LIBXML2_INSTALL_DIR}
               -DCMAKE_INSTALL_LIBDIR=lib
               -DBUILD_SHARED_LIBS=OFF
    BUILD_BYPRODUCTS ${LIBXML2_BUILD_BYPRODUCTS}
)
