# Copyright 2022 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(ROOT_PROJECT_COMPILE_OPTIONS
    ${SWIFTSHADER_COMPILE_OPTIONS}
)

set(ROOT_PROJECT_LINK_LIBRARIES
    ${OS_LIBS}
    ${SWIFTSHADER_LIBS}
)

set(PIPELINE_BENCHMARKS_SRC_FILES
    PipelineBenchmarks.cpp
)

add_executable(PipelineBenchmarks
    ${PIPELINE_BENCHMARKS_SRC_FILES}
)

set_target_properties(PipelineBenchmarks PROPERTIES
    FOLDER "Tests/Benchmarks"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_compile_options(PipelineBenchmarks
    PRIVATE
        ${ROOT_PROJECT_COMPILE_OPTIONS}
)

target_link_options(PipelineBenchmarks
    PRIVATE
        ${SWIFTSHADER_LINK_FLAGS}
)

target_link_libraries(PipelineBenchmarks
    PRIVATE
        benchmark::benchmark
        Reactor
        vk_pipeline
        ${ROOT_PROJECT_LINK_LIBRARIES}
)
