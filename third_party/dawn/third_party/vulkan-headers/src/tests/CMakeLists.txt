# ~~~
# Copyright 2022-2023 The Khronos Group Inc.
# Copyright 2022-2023 Valve Corporation
# Copyright 2022-2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0
# ~~~

# Test add_subdirectory suppport
add_test(NAME integration.add_subdirectory
    COMMAND ${CMAKE_CTEST_COMMAND}
        --build-and-test ${CMAKE_CURRENT_LIST_DIR}/integration
                         ${CMAKE_CURRENT_BINARY_DIR}/add_subdirectory
        --build-generator ${CMAKE_GENERATOR}
        --build-options -DFIND_PACKAGE_TESTING=OFF -DVULKAN_HEADERS_ENABLE_MODULE=${VULKAN_HEADERS_ENABLE_MODULE}
)

set(test_install_dir "${CMAKE_CURRENT_BINARY_DIR}/install")
add_test(NAME integration.install
    COMMAND ${CMAKE_COMMAND} --install ${VULKAN_HEADERS_BINARY_DIR} --prefix ${test_install_dir} --config $<CONFIG>
)

# Test find_package suppport
add_test(NAME integration.find_package
    COMMAND ${CMAKE_CTEST_COMMAND}
        --build-and-test ${CMAKE_CURRENT_LIST_DIR}/integration
                         ${CMAKE_CURRENT_BINARY_DIR}/find_package
        --build-generator ${CMAKE_GENERATOR}
        --build-options -DFIND_PACKAGE_TESTING=ON -DCMAKE_PREFIX_PATH=${test_install_dir} -DVULKAN_HEADERS_ENABLE_MODULE=${VULKAN_HEADERS_ENABLE_MODULE}
)

# Installing comes before testing
set_tests_properties(integration.find_package PROPERTIES DEPENDS integration.install)
