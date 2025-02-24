# ~~~
# Copyright (c) 2023 Valve Corporation
# Copyright (c) 2023 LunarG, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~

# Our unit tests on Android repeatedly close/open the vulkan validation layers.
# This causes issues with issues with thread_local variables and overall performance.
#
# To run our tests without issue we can inform the linker to not unload VVL at runtime.
# This will prevent VVL from being unloaded by dlclose which will avoid the issue.
# 
# https://github.com/android/ndk/issues/360
# https://github.com/android/ndk/issues/360#issuecomment-339107521
# https://github.com/android/ndk/releases/tag/r26-beta1
target_link_options(vvl PRIVATE -Wl,-z,nodelete)

set_directory_properties(PROPERTIES "COMPILE_OPTIONS" "") # Disable compiler warnings for android glue

enable_language(C) # NOTE: We need to enable the C language for android_native_app_glue.c

set(native_app_glue_dir "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")

if (NOT EXISTS ${native_app_glue_dir})
    message(FATAL_ERROR "Couldn't find Android Native Glue directory!")
endif()

# https://stackoverflow.com/questions/57189936/compiling-native-app-glue-c-results-in-an-invalid-library-file/76963564#76963564
add_library(android_glue OBJECT)

target_include_directories(android_glue PUBLIC ${native_app_glue_dir})
target_sources(android_glue PRIVATE
    ${native_app_glue_dir}/android_native_app_glue.c
    ${native_app_glue_dir}/android_native_app_glue.h
)

target_link_libraries(vk_layer_validation_tests PRIVATE android_glue)

target_link_options(vk_layer_validation_tests PUBLIC LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/android.map)

set_target_properties(vk_layer_validation_tests PROPERTIES OUTPUT_NAME "VulkanLayerValidationTests")

install(TARGETS vk_layer_validation_tests DESTINATION ${CMAKE_INSTALL_LIBDIR})

# https://stackoverflow.com/questions/76631917/cmake-how-to-install-shared-stl-libraries-for-android/76656492#76656492
if ("${CMAKE_ANDROID_STL_TYPE}" MATCHES "shared")
    file(READ "${CMAKE_ANDROID_NDK}/meta/abis.json" JSON_FILE)
    string(JSON TRIPLE GET "${JSON_FILE}" "${CMAKE_ANDROID_ARCH_ABI}" "triple")
    install(FILES "${CMAKE_SYSROOT}/usr/lib/${TRIPLE}/libc++_shared.so" DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

find_program(GNU_NM NAMES nm)
if (GNU_NM)
    # Ensure ANativeActivity_onCreate is being exported
    add_test(NAME ANativeActivity_onCreate COMMAND ${GNU_NM} --dynamic $<TARGET_FILE:vk_layer_validation_tests>)
    set_tests_properties(ANativeActivity_onCreate
        PROPERTIES PASS_REGULAR_EXPRESSION "T ANativeActivity_onCreate"
    )

    # Ensure compatibility with older Android loaders
    add_test(NAME vkEnumerateDeviceExtensionProperties COMMAND ${GNU_NM} --dynamic $<TARGET_FILE:vvl>)
    set_tests_properties(vkEnumerateDeviceExtensionProperties
        PROPERTIES PASS_REGULAR_EXPRESSION "T vkEnumerateDeviceExtensionProperties"
    )

    # Ensure compatibility with older Android loaders
    add_test(NAME vkEnumerateDeviceLayerProperties COMMAND ${GNU_NM} --dynamic $<TARGET_FILE:vvl>)
    set_tests_properties(vkEnumerateDeviceLayerProperties
        PROPERTIES PASS_REGULAR_EXPRESSION "T vkEnumerateDeviceLayerProperties"
    )
endif()
