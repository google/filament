## ======================================================================== ##
## Copyright 2009-2019 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

option(OIDN_ZIP_MODE off)
mark_as_advanced(OIDN_ZIP_MODE)

## ----------------------------------------------------------------------------
## Set install directories
## ----------------------------------------------------------------------------

include(GNUInstallDirs)

if(OIDN_ZIP_MODE)
  set(CMAKE_INSTALL_BINDIR bin)
  set(CMAKE_INSTALL_LIBDIR lib)
  set(CMAKE_INSTALL_DOCDIR doc)
endif()

## ----------------------------------------------------------------------------
## Set rpath
## ----------------------------------------------------------------------------

if(OIDN_ZIP_MODE)
  # In tgz / zip let's have relative rpath
  set(CMAKE_SKIP_INSTALL_RPATH OFF)
  if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "@executable_path/" "@executable_path/../lib")
  else()
    set(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib")
  endif()
else()
  set(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
  if(APPLE)
    # Use rpath on macOS
    set(CMAKE_SKIP_INSTALL_RPATH OFF)
  else()
    # We do not want any rpath for installed binaries
    set(CMAKE_SKIP_INSTALL_RPATH ON)
  endif()
endif()

## ----------------------------------------------------------------------------
## Configure CPack
## ----------------------------------------------------------------------------

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Image Denoise library")
set(CPACK_PACKAGE_VENDOR "Intel Corporation")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME oidn-${CPACK_PACKAGE_VERSION})
set(CPACK_VERBATIM_VARIABLES YES)

if(WIN32)

  # Windows specific settings
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH x64)
    set(CPACK_PACKAGE_NAME "${CPACK_PACKAGE_NAME} x64")
  else()
    set(ARCH win32)
    set(CPACK_PACKAGE_NAME "${CPACK_PACKAGE_NAME} Win32")
  endif()

  if(MSVC12)
    set(VCVER vc12)
  elseif(MSVC14) # also for VC15, which is toolset v141
    set(VCVER vc14)
  endif()

  set(CPACK_GENERATOR ZIP)
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}.${ARCH}.${VCVER}.windows")
  set(CPACK_MONOLITHIC_INSTALL 1)

elseif(APPLE)

  # macOS specific settings
  set(CPACK_GENERATOR TGZ)
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}.x86_64.macos")
  set(CPACK_MONOLITHIC_INSTALL 1)

else()

  # Linux specific settings
  set(CPACK_GENERATOR TGZ)
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}.x86_64.linux")
  set(CPACK_MONOLITHIC_INSTALL 1)

endif()