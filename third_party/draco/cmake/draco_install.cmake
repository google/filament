# Copyright 2021 The Draco Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

if(DRACO_CMAKE_DRACO_INSTALL_CMAKE_)
  return()
endif() # DRACO_CMAKE_DRACO_INSTALL_CMAKE_
set(DRACO_CMAKE_DRACO_INSTALL_CMAKE_ 1)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Sets up the draco install targets. Must be called after the static library
# target is created.
macro(draco_setup_install_target)
  if(DRACO_INSTALL)
    set(bin_path "${CMAKE_INSTALL_BINDIR}")
    set(data_path "${CMAKE_INSTALL_DATAROOTDIR}")
    set(includes_path "${CMAKE_INSTALL_INCLUDEDIR}")
    set(libs_path "${CMAKE_INSTALL_LIBDIR}")

    foreach(file ${draco_sources})
      if(file MATCHES "h$")
        list(APPEND draco_api_includes ${file})
      endif()
    endforeach()

    list(REMOVE_DUPLICATES draco_api_includes)

    # Strip $draco_src_root from the file paths: we need to install relative to
    # $include_directory.
    list(TRANSFORM draco_api_includes REPLACE "${draco_src_root}/" "")

    foreach(draco_api_include ${draco_api_includes})
      get_filename_component(file_directory ${draco_api_include} DIRECTORY)
      set(target_directory "${includes_path}/draco/${file_directory}")
      install(FILES ${draco_src_root}/${draco_api_include}
              DESTINATION "${target_directory}")
    endforeach()

    install(FILES "${draco_build}/draco/draco_features.h"
            DESTINATION "${includes_path}/draco/")

    install(TARGETS draco_decoder DESTINATION "${bin_path}")
    install(TARGETS draco_encoder DESTINATION "${bin_path}")

    if(DRACO_TRANSCODER_SUPPORTED)
      install(TARGETS draco_transcoder DESTINATION "${bin_path}")
    endif()

    if(MSVC)
      install(
        TARGETS draco
        EXPORT dracoExport
        RUNTIME DESTINATION "${bin_path}"
        ARCHIVE DESTINATION "${libs_path}"
        LIBRARY DESTINATION "${libs_path}")
    else()
      install(
        TARGETS draco_static
        EXPORT dracoExport
        DESTINATION "${libs_path}")

      if(BUILD_SHARED_LIBS)
        install(
          TARGETS draco_shared
          EXPORT dracoExport
          RUNTIME DESTINATION "${bin_path}"
          ARCHIVE DESTINATION "${libs_path}"
          LIBRARY DESTINATION "${libs_path}")
      endif()
    endif()

    if(DRACO_UNITY_PLUGIN)
      install(TARGETS dracodec_unity DESTINATION "${libs_path}")
    endif()

    if(DRACO_MAYA_PLUGIN)
      install(TARGETS draco_maya_wrapper DESTINATION "${libs_path}")
    endif()

    # pkg-config: draco.pc
    configure_file("${draco_root}/cmake/draco.pc.template"
                  "${draco_build}/draco.pc" @ONLY NEWLINE_STYLE UNIX)
    install(FILES "${draco_build}/draco.pc" DESTINATION "${libs_path}/pkgconfig")

    # CMake config: draco-config.cmake
    configure_package_config_file(
      "${draco_root}/cmake/draco-config.cmake.template"
      "${draco_build}/draco-config.cmake"
      INSTALL_DESTINATION "${data_path}/cmake/draco")

    write_basic_package_version_file(
      "${draco_build}/draco-config-version.cmake"
      VERSION ${DRACO_VERSION}
      COMPATIBILITY AnyNewerVersion)

    export(
      EXPORT dracoExport
      NAMESPACE draco::
      FILE "${draco_build}/draco-targets.cmake")

    install(
      EXPORT dracoExport
      NAMESPACE draco::
      FILE draco-targets.cmake
      DESTINATION "${data_path}/cmake/draco")

    install(FILES "${draco_build}/draco-config.cmake"
                  "${draco_build}/draco-config-version.cmake"
            DESTINATION "${data_path}/cmake/draco")
  endif(DRACO_INSTALL)
endmacro()
