if(DRACO_CMAKE_DRACO_INSTALL_CMAKE_)
  return()
endif() # DRACO_CMAKE_DRACO_INSTALL_CMAKE_
set(DRACO_CMAKE_DRACO_INSTALL_CMAKE_ 1)

# Sets up the draco install targets. Must be called after the static library
# target is created.
macro(draco_setup_install_target)
  include(GNUInstallDirs)

  # pkg-config: draco.pc
  set(prefix "${CMAKE_INSTALL_PREFIX}")
  set(exec_prefix "\${prefix}")
  set(libdir "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
  set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
  set(draco_lib_name "draco")

  configure_file("${draco_root}/cmake/draco.pc.template"
                 "${draco_build}/draco.pc" @ONLY NEWLINE_STYLE UNIX)
  install(FILES "${draco_build}/draco.pc"
          DESTINATION "${prefix}/${CMAKE_INSTALL_LIBDIR}/pkgconfig")

  # CMake config: draco-config.cmake
  set(DRACO_INCLUDE_DIRS "${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
  configure_file("${draco_root}/cmake/draco-config.cmake.template"
                 "${draco_build}/draco-config.cmake" @ONLY NEWLINE_STYLE UNIX)
  install(
    FILES "${draco_build}/draco-config.cmake"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/cmake")

  foreach(file ${draco_sources})
    if(file MATCHES "h$")
      list(APPEND draco_api_includes ${file})
    endif()
  endforeach()

  # Strip $draco_src_root from the file paths: we need to install relative to
  # $include_directory.
  list(TRANSFORM draco_api_includes REPLACE "${draco_src_root}/" "")
  set(include_directory "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")

  foreach(draco_api_include ${draco_api_includes})
    get_filename_component(file_directory ${draco_api_include} DIRECTORY)
    set(target_directory "${include_directory}/draco/${file_directory}")
    install(FILES ${draco_src_root}/${draco_api_include}
            DESTINATION "${target_directory}")
  endforeach()

  install(
    FILES "${draco_build}/draco/draco_features.h"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/draco/")

  install(TARGETS draco_decoder DESTINATION
                  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
  install(TARGETS draco_encoder DESTINATION
                  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")

  if(WIN32)
    install(TARGETS draco DESTINATION
                    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
  else()
    install(TARGETS draco_static DESTINATION
                    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
    if(BUILD_SHARED_LIBS)
      install(TARGETS draco_shared DESTINATION
                      "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
    endif()
  endif()

  if(DRACO_UNITY_PLUGIN)
    install(TARGETS dracodec_unity DESTINATION
                    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
  endif()
  if(DRACO_MAYA_PLUGIN)
    install(TARGETS draco_maya_wrapper DESTINATION
                    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
  endif()

endmacro()
