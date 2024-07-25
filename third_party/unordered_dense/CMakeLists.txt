cmake_minimum_required(VERSION 3.12)
project("unordered_dense"
    VERSION 4.4.0
    DESCRIPTION "A fast & densely stored hashmap and hashset based on robin-hood backward shift deletion"
    HOMEPAGE_URL "https://github.com/martinus/unordered_dense")

include(GNUInstallDirs)

# determine whether this is a standalone project or included by other projects
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(_unordered_dense_is_toplevel_project TRUE)
else()
    set(_unordered_dense_is_toplevel_project FALSE)
endif()

add_library(unordered_dense INTERFACE)
add_library(unordered_dense::unordered_dense ALIAS unordered_dense)

target_include_directories(
    unordered_dense
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(unordered_dense INTERFACE cxx_std_17)

if(_unordered_dense_is_toplevel_project)
    # locations are provided by GNUInstallDirs
    install(
        TARGETS unordered_dense
        EXPORT unordered_dense_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "unordered_denseConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/unordered_denseConfig.cmake.in"
        "${PROJECT_BINARY_DIR}/unordered_denseConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

    install(
        EXPORT unordered_dense_Targets
        FILE unordered_denseTargets.cmake
        NAMESPACE unordered_dense::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

    install(
        FILES "${PROJECT_BINARY_DIR}/unordered_denseConfig.cmake"
        "${PROJECT_BINARY_DIR}/unordered_denseConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/include/ankerl
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()
