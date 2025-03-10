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

add_executable(dynamic_rendering_get_proc_addr dynamic_rendering_get_proc_addr.cpp)
target_link_libraries(dynamic_rendering_get_proc_addr testing_dependencies)

add_subdirectory(dynamic_loader_behavior)

if(APPLE_STATIC_LOADER)
    add_executable(macos_static_loader_build macos_static_loader_build.cpp)
    target_link_libraries(macos_static_loader_build loader_common_options vulkan Vulkan::Headers)
    if (TEST_USE_THREAD_SANITIZER)
        target_compile_options(macos_static_loader_build PUBLIC -fsanitize=thread)
        target_link_options(macos_static_loader_build PUBLIC -fsanitize=thread)
    endif()
endif()

add_executable(time_dynamic_loading time_dynamic_loading.cpp)
target_link_libraries(time_dynamic_loading Vulkan::Headers vulkan)
