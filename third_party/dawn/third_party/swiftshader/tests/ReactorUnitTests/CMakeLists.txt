# Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

set(REACTOR_UNIT_TESTS_SRC_FILES
    main.cpp
    ReactorSIMD.cpp
    ReactorUnitTests.cpp
)

add_executable(ReactorUnitTests
    ${REACTOR_UNIT_TESTS_SRC_FILES}
)

target_compile_features(ReactorUnitTests
    PUBLIC
        cxx_std_17
)

set_target_properties(ReactorUnitTests PROPERTIES
    FOLDER "Tests"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_compile_options(ReactorUnitTests
    PRIVATE
        ${ROOT_PROJECT_COMPILE_OPTIONS}
)

target_link_options(ReactorUnitTests
    PRIVATE
        ${SWIFTSHADER_LINK_FLAGS}
)

target_link_libraries(ReactorUnitTests
    PRIVATE
        Reactor
        gtest
        ${ROOT_PROJECT_LINK_LIBRARIES}
)
