# Copyright 2022 The Draco Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

cmake_minimum_required(VERSION 3.12)
project(install_test C CXX)

include(GNUInstallDirs)

find_package(draco REQUIRED CONFIG)

add_executable(install_check main.cc)
target_link_libraries(install_check PRIVATE draco::draco)

install(TARGETS install_check DESTINATION ${CMAKE_INSTALL_BINDIR})
