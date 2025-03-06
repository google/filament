# ~~~
# Copyright (c) 2022 Valve Corporation
# Copyright (c) 2022 LunarG, Inc.
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

if (WIN32)
    #tests use dlsym and linux specific library names
else()
add_library(dynamic_library_a dynamic_library.cpp)
target_link_libraries(dynamic_library_a PUBLIC testing_framework_util)
target_compile_definitions(dynamic_library_a PRIVATE PRINT_OUTPUT_A)

add_library(dynamic_library_b dynamic_library.cpp)
target_link_libraries(dynamic_library_b PUBLIC testing_framework_util)
target_compile_definitions(dynamic_library_b PRIVATE PRINT_OUTPUT_B)

add_library(dynamic_library_c dynamic_library.cpp)
target_link_libraries(dynamic_library_c PUBLIC testing_framework_util)
target_compile_definitions(dynamic_library_c PRIVATE PRINT_OUTPUT_C)

add_executable(test_dynamic_linking_a_first test_dynamic_linking.cpp)
add_executable(test_dynamic_linking_b_first test_dynamic_linking.cpp)
add_executable(test_dynamic_linking_c_then_load test_dynamic_linking.cpp)
target_link_libraries(test_dynamic_linking_a_first PUBLIC dynamic_library_a dynamic_library_b)
target_link_libraries(test_dynamic_linking_b_first PUBLIC dynamic_library_b dynamic_library_a)
target_link_libraries(test_dynamic_linking_c_then_load PUBLIC dynamic_library_c)
target_compile_definitions(test_dynamic_linking_c_then_load PRIVATE PRINT_OUTPUT_C)


add_executable(test_dynamic_loading test_dynamic_loading.cpp)
target_link_libraries(test_dynamic_loading PUBLIC testing_framework_util)

add_executable(test_dynamic_loading_and_linking test_dynamic_loading_and_linking.cpp)
target_link_libraries(test_dynamic_loading_and_linking PUBLIC testing_framework_util)
target_link_libraries(test_dynamic_loading_and_linking PUBLIC dynamic_library_a)

endif()
