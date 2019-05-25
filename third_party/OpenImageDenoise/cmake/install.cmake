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

## ----------------------------------------------------------------------------
## Install library
## ----------------------------------------------------------------------------

install(TARGETS ${PROJECT_NAME}
  EXPORT
    ${PROJECT_NAME}_Export
  ARCHIVE
    DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
  # On Windows put the dlls into bin
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT lib
)

if(OIDN_STATIC_LIB)
  install(TARGETS common mkldnn
    EXPORT
      ${PROJECT_NAME}_Export
    ARCHIVE
      DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
  )
endif()

## ----------------------------------------------------------------------------
## Install headers
## ----------------------------------------------------------------------------

install(DIRECTORY include/OpenImageDenoise
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT devel
  PATTERN "*.in" EXCLUDE
)

## ----------------------------------------------------------------------------
## Install documentation
## ----------------------------------------------------------------------------

install(FILES ${PROJECT_SOURCE_DIR}/LICENSE.txt DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT lib)
install(FILES ${PROJECT_SOURCE_DIR}/CHANGELOG.md DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT lib)
install(FILES ${PROJECT_SOURCE_DIR}/README.md DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT lib)
install(FILES ${PROJECT_SOURCE_DIR}/readme.pdf DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT lib)

## ----------------------------------------------------------------------------
## Install dependencies
## ----------------------------------------------------------------------------

# Install TBB
if(OIDN_ZIP_MODE)
  if(WIN32)
    install(PROGRAMS ${TBB_BIN_DIR}/tbb.dll ${TBB_BIN_DIR}/tbbmalloc.dll DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT lib)
    install(PROGRAMS ${TBB_LIBRARY} ${TBB_LIBRARY_MALLOC} DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib)
  elseif(APPLE)
    install(PROGRAMS ${TBB_ROOT}/lib/libtbb.dylib ${TBB_ROOT}/lib/libtbbmalloc.dylib DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib)
  else()
    install(PROGRAMS ${TBB_ROOT}/lib/intel64/gcc4.4/libtbb.so.2 ${TBB_ROOT}/lib/intel64/gcc4.4/libtbbmalloc.so.2 DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib)
  endif()
endif()

## ----------------------------------------------------------------------------
## Install CMake configuration files
## ----------------------------------------------------------------------------

install(EXPORT ${PROJECT_NAME}_Export
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  #NAMESPACE ${PROJECT_NAME}::
  FILE ${PROJECT_NAME}Config.cmake
  COMPONENT devel
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${PROJECT_NAME}ConfigVersion.cmake
  COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  COMPONENT devel
)
