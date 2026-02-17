cmake_minimum_required(VERSION 3.10)

project(tsl-robin-map VERSION 1.4.1 LANGUAGES CXX)

include(GNUInstallDirs)


set(IS_MAIN_PROJECT FALSE)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(IS_MAIN_PROJECT TRUE)
endif()

# Enable the install target if the project is used main project (disable by default if subproject)
option(TSL_ROBIN_MAP_ENABLE_INSTALL "Enable install target" ${IS_MAIN_PROJECT})


add_library(robin_map INTERFACE)
# Use tsl::robin_map as target, more consistent with other libraries conventions (Boost, Qt, ...)
add_library(tsl::robin_map ALIAS robin_map)

target_include_directories(robin_map INTERFACE
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

list(APPEND headers "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/robin_growth_policy.h"
                    "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/robin_hash.h"
                    "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/robin_map.h"
                    "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/robin_set.h")
target_sources(robin_map INTERFACE "$<BUILD_INTERFACE:${headers}>")

if(MSVC)
    target_sources(robin_map INTERFACE
                   "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tsl-robin-map.natvis>"
                   "$<INSTALL_INTERFACE:${CMAKE_INSTALL_DATAROOTDIR}/tsl-robin-map.natvis>")
endif()

# Installation
if(TSL_ROBIN_MAP_ENABLE_INSTALL)
    include(CMakePackageConfigHelpers)

    ## Install include directory and potential natvis file
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

    if(MSVC)
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/tsl-robin-map.natvis"
                DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}")
    endif()



    ## Create and install tsl-robin-mapConfig.cmake
    configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/tsl-robin-mapConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/tsl-robin-mapConfig.cmake"
                                  INSTALL_DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/cmake/tsl-robin-map")

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tsl-robin-mapConfig.cmake"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/cmake/tsl-robin-map")


    ## Create local tsl-robin-mapTargets.cmake
    export(TARGETS robin_map NAMESPACE tsl:: FILE "${CMAKE_CURRENT_BINARY_DIR}/tsl-robin-mapTargets.cmake")

    ## Create and install global tsl-robin-mapTargets.cmake
    install(TARGETS robin_map
            EXPORT tsl-robin-mapTargets)

    install(EXPORT tsl-robin-mapTargets
            NAMESPACE tsl::
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/cmake/tsl-robin-map")



    ## Create and install tsl-robin-mapConfigVersion.cmake
    # tsl-robin-map is header-only and does not depend on the architecture.
    # Remove CMAKE_SIZEOF_VOID_P from tsl-robin-mapConfigVersion.cmake so that a
    # tsl-robin-mapConfig.cmake generated for a 64 bit target can be used for 32 bit
    # targets and vice versa.
    set(CMAKE_SIZEOF_VOID_P_BACKUP ${CMAKE_SIZEOF_VOID_P})
    unset(CMAKE_SIZEOF_VOID_P)
    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/tsl-robin-mapConfigVersion.cmake"
                                     COMPATIBILITY SameMajorVersion)
    set(CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P_BACKUP})

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tsl-robin-mapConfigVersion.cmake"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/cmake/tsl-robin-map")
endif()
