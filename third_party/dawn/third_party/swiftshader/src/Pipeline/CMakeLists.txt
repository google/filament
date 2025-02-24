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
    ${WARNINGS_AS_ERRORS}
)

set(PIPELINE_SRC_FILES
    ComputeProgram.cpp
    ComputeProgram.hpp
    Constants.cpp
    Constants.hpp
    PixelProgram.cpp
    PixelProgram.hpp
    PixelRoutine.cpp
    PixelRoutine.hpp
    SamplerCore.cpp
    SamplerCore.hpp
    SetupRoutine.cpp
    SetupRoutine.hpp
    ShaderCore.cpp
    ShaderCore.hpp
    SpirvBinary.hpp
    SpirvBinary.cpp
    SpirvID.hpp
    SpirvProfiler.hpp
    SpirvProfiler.cpp
    SpirvShader.cpp
    SpirvShader.hpp
    SpirvShaderArithmetic.cpp
    SpirvShaderControlFlow.cpp
    SpirvShaderDebugger.cpp
    SpirvShaderGLSLstd450.cpp
    SpirvShaderGroup.cpp
    SpirvShaderImage.cpp
    SpirvShaderInstructions.cpp
    SpirvShaderMemory.cpp
    SpirvShaderSampling.cpp
    SpirvShaderSpec.cpp
    VertexProgram.cpp
    VertexProgram.hpp
    VertexRoutine.cpp
    VertexRoutine.hpp
)

set(PIPELINE_COMPILE_OPTIONS "")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND PIPELINE_COMPILE_OPTIONS
        "-Wexit-time-destructors"  # declaration requires an exit-time destructor
    )
endif()

add_library(vk_pipeline EXCLUDE_FROM_ALL
    ${PIPELINE_SRC_FILES}
)

# Add SPIRV-Tools dep
if (NOT TARGET SPIRV-Tools)
    message(FATAL_ERROR "Missing required target: SPIRV-Tools")
endif()

set_target_properties(core_tables PROPERTIES FOLDER "SPIRV-Tools build")
set_target_properties(enum_string_mapping PROPERTIES FOLDER "SPIRV-Tools build")
set_target_properties(extinst_tables PROPERTIES FOLDER "SPIRV-Tools build")

set_target_properties(vk_pipeline PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "SwiftShader VK"
)

target_include_directories(vk_pipeline
    PUBLIC
        ".."
        "${SWIFTSHADER_DIR}/include"
        "${SPIRV-Headers_SOURCE_DIR}/include"
        "${SPIRV_TOOLS_EXT_INC_DIR}"
)

target_compile_options(vk_pipeline
    PRIVATE
        ${ROOT_PROJECT_COMPILE_OPTIONS}
        ${PIPELINE_COMPILE_OPTIONS}
)

target_link_options(vk_pipeline
    PUBLIC
        ${SWIFTSHADER_LINK_FLAGS}
)

target_link_libraries(vk_pipeline
    PUBLIC
        vk_base
        vk_system
        marl
        Reactor
        SPIRV-Tools
        SPIRV-Tools-opt
)
