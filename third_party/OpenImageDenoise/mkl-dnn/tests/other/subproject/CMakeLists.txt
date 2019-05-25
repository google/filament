#===============================================================================
# Copyright 2018 Intel Corporation
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
#===============================================================================

# Test Intel MKL-DNN for embeddability
# by imitating a project that includes the library
#
# To test run:
# mkdir -p build && cd build && cmake .. && make -j && ./project_app

cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME "Project")

# include Intel MKL-DNN
set(MKLDNN_DIR "../../..")
add_subdirectory(${MKLDNN_DIR} mkl-dnn)
include_directories(${MKLDNN_DIR}/include)

add_executable(project_app main.c)
target_link_libraries(project_app mkldnn)
