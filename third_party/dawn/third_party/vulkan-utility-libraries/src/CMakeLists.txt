# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.22.1)

# The VERSION field is generated with the "--generated-version" flag in the generate_source.py script
project(VUL VERSION 1.4.349 LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Remove when min is 3.26, see CMP0143

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN "YES")

# This variable enables downstream users to customize the target API
# variant (e.g. Vulkan SC)
set(API_TYPE "vulkan")

add_subdirectory(scripts)

find_package(VulkanHeaders ${PROJECT_VERSION} CONFIG)

option(VUL_ENABLE_ASAN "Use address sanitization")
if (VUL_ENABLE_ASAN)
    add_compile_options(-fsanitize=address)
    if (NOT MSVC)
        add_link_options(-fsanitize=address)
    endif()
endif()

option(VUL_ENABLE_UBSAN "Use undefined behavior sanitization")
if (VUL_ENABLE_UBSAN)
    if (NOT MSVC)
        add_compile_options(-fsanitize=undefined)
        add_link_options(-fsanitize=undefined)
    endif()
endif()

add_subdirectory(src)
add_subdirectory(include)

if (PROJECT_IS_TOP_LEVEL)
    option(BUILD_TESTS "Build tests")
    if (BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()

option(VUL_ENABLE_INSTALL "Enable install" ${PROJECT_IS_TOP_LEVEL})
if (VUL_ENABLE_INSTALL)

    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/vulkan"
    )

    # Create VulkanUtilityLibraries-targets.cmake
    set_target_properties(VulkanLayerSettings PROPERTIES EXPORT_NAME "LayerSettings")
    set_target_properties(VulkanUtilityHeaders PROPERTIES EXPORT_NAME "UtilityHeaders")
    set_target_properties(VulkanSafeStruct PROPERTIES EXPORT_NAME "SafeStruct")
    set_target_properties(VulkanCompilerConfiguration PROPERTIES EXPORT_NAME "CompilerConfiguration")
    install(
        TARGETS VulkanLayerSettings VulkanUtilityHeaders VulkanSafeStruct VulkanCompilerConfiguration
        EXPORT VulkanUtilityLibraries-targets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        EXPORT VulkanUtilityLibraries-targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VulkanUtilityLibraries
        NAMESPACE Vulkan::
    )

    # Create VulkanUtilityLibrariesConfig.cmake
    set(VUL_EXPORT_TARGETS ${CMAKE_INSTALL_LIBDIR}/cmake/VulkanUtilityLibraries/VulkanUtilityLibraries-targets.cmake)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/VulkanUtilityLibrariesConfig.cmake.in" [=[
        include(CMakeFindDependencyMacro)
        # NOTE: Because VulkanHeaders is a PUBLIC dependency it needs to be found prior to VulkanUtilityLibraries
        find_dependency(VulkanHeaders @PROJECT_VERSION@ REQUIRED)

        @PACKAGE_INIT@

        include(@PACKAGE_VUL_EXPORT_TARGETS@)
    ]=])

    configure_package_config_file(
        ${CMAKE_CURRENT_BINARY_DIR}/VulkanUtilityLibrariesConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/VulkanUtilityLibrariesConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VulkanUtilityLibraries
        PATH_VARS VUL_EXPORT_TARGETS
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/VulkanUtilityLibrariesConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion)

    install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/VulkanUtilityLibrariesConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/VulkanUtilityLibrariesConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VulkanUtilityLibraries)
endif()

option(VUL_CODEGEN "Enable Vulkan-Utility-Libraries code generation")
if(VUL_CODEGEN)
    find_package(Python3 REQUIRED)
    add_custom_target(vul_codegen
        COMMAND Python3::Interpreter ${PROJECT_SOURCE_DIR}/scripts/generate_source.py
            "${VULKAN_HEADERS_INSTALL_DIR}/${CMAKE_INSTALL_DATADIR}/vulkan/registry"
            --generated-version ${VulkanHeaders_VERSION} --api ${API_TYPE}
    )
endif()
