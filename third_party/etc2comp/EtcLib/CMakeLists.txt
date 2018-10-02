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

project(EtcLib)

file(GLOB SOURCES
	${PROJECT_SOURCE_DIR}/Etc/*.h
	${PROJECT_SOURCE_DIR}/EtcCodec/*.h
	${PROJECT_SOURCE_DIR}/Etc/*.cpp
	${PROJECT_SOURCE_DIR}/EtcCodec/*.cpp)
ADD_LIBRARY(EtcLib ${SOURCES})

target_include_directories(EtcLib PUBLIC Etc EtcCodec)
