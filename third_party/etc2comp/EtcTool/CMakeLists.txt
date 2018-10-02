# Copyright 2015 The Etc2Comp Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

project(EtcTool)
include_directories(../EtcLib/Etc)
include_directories(../EtcLib/EtcCodec)
include_directories(../third_party/lodepng)

file(GLOB SOURCES
	${PROJECT_SOURCE_DIR}/*.h
	${PROJECT_SOURCE_DIR}/*.cpp
	../third_party/lodepng/*.h
	../third_party/lodepng/*.cpp)
add_executable(EtcTool ${SOURCES})

target_link_libraries (EtcTool EtcLib)

