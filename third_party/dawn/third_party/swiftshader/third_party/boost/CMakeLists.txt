# Copyright 2020 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Boost is downloaded if necessary

# From https://www.boost.org/users/download/
set(BOOST_VER 1.78.0)
set(BOOST_HASH_TARGZ "94ced8b72956591c4775ae2207a9763d3600b30d9d7446562c552f0a14a63be7")
set(BOOST_HASH_ZIP   "f22143b5528e081123c3c5ed437e92f648fe69748e95fa6e2bd41484e2986cc3")

string(REPLACE "." "_" BOOST_VER_FNAME ${BOOST_VER})
set(BOOST_PARENT_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(BOOST_DIR ${BOOST_PARENT_DIR}/boost_${BOOST_VER_FNAME})

function(DownloadBoost)
    if (LINUX)
        set(BOOST_EXT "tar.gz")
        set(BOOST_HASH ${BOOST_HASH_TARGZ})
    else()
        set(BOOST_EXT "zip")
        set(BOOST_HASH ${BOOST_HASH_ZIP})
    endif()

    # set(BOOST_URL https://dl.bintray.com/boostorg/release/${BOOST_VER}/source/boost_${BOOST_VER_FNAME}.${BOOST_EXT})  # Note: bintray.com has rate limiting
    # set(BOOST_URL https://iweb.dl.sourceforge.net/project/boost/boost/${BOOST_VER}/boost_${BOOST_VER_FNAME}.${BOOST_EXT})  # Note: iweb.dl.sourceforge.net no longer available
    set(BOOST_URL https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VER}/source/boost_${BOOST_VER_FNAME}.${BOOST_EXT})

    if (NOT TARGET Boost::boost)
        if(NOT EXISTS ${BOOST_DIR})
            message(WARNING "
          ${BOOST_DIR} is missing.
          Downloading and extracting boost:
            ")

            set(BOOST_ARCHIVE ${CMAKE_BINARY_DIR}/temp/boost_${BOOST_VER_FNAME}.${BOOST_EXT})
            message(STATUS "Downloading ${BOOST_URL} to ${BOOST_ARCHIVE}...")
            file(DOWNLOAD ${BOOST_URL} ${BOOST_ARCHIVE} EXPECTED_HASH SHA256=${BOOST_HASH})
            message(STATUS "Extracting ${BOOST_ARCHIVE} to ${BOOST_DIR}...")
            execute_process(
                WORKING_DIRECTORY ${BOOST_PARENT_DIR}
                COMMAND cmake -E tar xf ${BOOST_ARCHIVE}
            )
        endif()
    endif()
endfunction()

DownloadBoost()

set(BOOST_INCLUDEDIR ${BOOST_DIR})
find_package(Boost REQUIRED)

# Expose Boost::boost target for parent project
add_library(boost INTERFACE)
target_link_libraries(boost INTERFACE
    Boost::boost
)
add_library(Boost::boost ALIAS boost)
