# Copyright 2023-2024 The Khronos Group Inc.
# Copyright 2023-2024 Valve Corporation
# Copyright 2023-2024 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0

find_package(GTest REQUIRED CONFIG)
find_package(magic_enum REQUIRED CONFIG)

include(GoogleTest)

add_executable(vul_tests)

target_include_directories(vul_tests PRIVATE
    ${VUL_SOURCE_DIR}/src/layer
)

target_sources(vul_tests PRIVATE
    safe_struct.cpp
    small_containers.cpp
    sparse_range_map.cpp
    struct_helper.cpp
    test_formats.cpp
    test_interface.cpp
    test_setting_api.cpp
    test_setting_cast.cpp
    test_setting_cpp.cpp
    test_setting_env.cpp
    test_setting_file.cpp
    test_setting_util.cpp
    vk_enum_string_helper.cpp
)

target_link_libraries(vul_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    magic_enum::magic_enum
    Vulkan::UtilityHeaders
    Vulkan::LayerSettings
    Vulkan::CompilerConfiguration
    Vulkan::SafeStruct
)

# Test add_subdirectory suppport
add_test(NAME integration.add_subdirectory
    COMMAND ${CMAKE_CTEST_COMMAND}
        --build-and-test ${CMAKE_CURRENT_LIST_DIR}/integration
                         ${CMAKE_CURRENT_BINARY_DIR}/add_subdirectory
        --build-generator ${CMAKE_GENERATOR}
        --build-options -DFIND_PACKAGE_TESTING=OFF -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
)

set(test_install_dir "${CMAKE_CURRENT_BINARY_DIR}/install")
add_test(NAME integration.install
    COMMAND ${CMAKE_COMMAND} --install ${PROJECT_BINARY_DIR} --prefix ${test_install_dir} --config $<CONFIG>
)

# Test find_package suppport
add_test(NAME integration.find_package
    COMMAND ${CMAKE_CTEST_COMMAND}
        --build-and-test ${CMAKE_CURRENT_LIST_DIR}/integration
                         ${CMAKE_CURRENT_BINARY_DIR}/find_package
        --build-generator ${CMAKE_GENERATOR}
        --build-options -DFIND_PACKAGE_TESTING=ON "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH};${test_install_dir}"
)

# Installing comes before testing
set_tests_properties(integration.find_package PROPERTIES DEPENDS integration.install)

if (CMAKE_CROSSCOMPILING)
    return()
endif()

gtest_discover_tests(vul_tests DISCOVERY_TIMEOUT 100)
