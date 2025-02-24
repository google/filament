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
cmake_minimum_required(VERSION 3.22.1)

project(INTEGRATION LANGUAGES C)

find_package(VulkanLoader REQUIRED CONFIG)

# The intent is ensuring we don't accidentally change the names of these
# targets. Since it would break downstream users.
if (NOT TARGET Vulkan::Loader)
    message(FATAL_ERROR "Vulkan::Loader target not defined!")
endif()

if (NOT DEFINED VulkanLoader_VERSION)
    message(FATAL_ERROR "VulkanLoader_VERSION not defined!")
endif()
message(STATUS "VulkanLoader_VERSION = ${VulkanLoader_VERSION}")

# NOTE: This check is NOT sufficient to ensure vulkan.pc is actually valid
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    find_file(VULKAN_PC vulkan.pc PATH_SUFFIXES lib/pkgconfig REQUIRED)
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --validate ${VULKAN_PC})
endif()
