cmake_minimum_required(VERSION 3.1)

# ==================================================================================================
# Library
# ==================================================================================================
project(matlang)
set(TARGET matlang)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE HDRS src/*.h)

set(SRCS
        src/matc/sca/ASTHelpers.cpp
        src/matc/sca/GLSLTools.cpp
        src/matc/sca/GLSLPostProcessor.cpp
        src/matc/Compiler.cpp
        src/matc/CommandlineConfig.cpp
        src/matc/Enums.cpp
        src/matc/JsonishLexer.cpp
        src/matc/JsonishParser.cpp
        src/matc/MaterialCompiler.cpp
        src/matc/MaterialLexer.cpp
        src/matc/ParametersProcessor.cpp
        src/matc/PostprocessMaterialCompiler.cpp
        src/matc/PostprocessMaterialBuilder.cpp
        )

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_library(${TARGET} STATIC ${SRCS} ${HDRS})
target_include_directories(${TARGET} PUBLIC src)

# We are being naughty and accessing private headers here
# For spirv-tools, we're just following glslang's example
# For filamat, well... it belongs to us so...
target_include_directories(${TARGET} PRIVATE ${spirv-tools_SOURCE_DIR}/include)
target_include_directories(${TARGET} PRIVATE ${filamat_SOURCE_DIR}/src)

# glslang libraries have circular dependencies. To make sure the proper object are part of the link
# we need to force archive re-scan on new symbol dependencies via start/end-group.
# Read more about this here https://eli.thegreenplace.net/2013/07/09/library-order-in-static-linking
set(COMMON_MATC_LIBS getopt filamat filabridge utils)
if (APPLE)
    target_link_libraries(${TARGET} ${COMMON_MATC_LIBS}
            glslang SPIRV SPVRemapper SPIRV-Tools-opt spirv-cross-glsl)
else()
    target_link_libraries(${TARGET} ${COMMON_MATC_LIBS}
            -Wl,--start-group glslang SPIRV SPVRemapper SPIRV-Tools-opt spirv-cross-glsl -Wl,--end-group)
endif()

# =================================================================================================
# Compiler flags
# ==================================================================================================
# this must match options enabled in glslang's CMakeLists.txt
target_compile_options(${TARGET} PRIVATE -DAMD_EXTENSIONS -DNV_EXTENSIONS )

# =================================================================================================
# Licenses
# ==================================================================================================
set(MODULE_LICENSES getopt glslang spirv-cross spirv-tools smol-v)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
list_licenses(${GENERATION_ROOT}/licenses/licenses.inc ${MODULE_LICENSES})
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})

# ==================================================================================================
# Binary
# ==================================================================================================
project(matc)
set(TARGET matc)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(SRCS src/main.cpp)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${SRCS} ${HDRS})
target_link_libraries(${TARGET} matlang)

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)

# ==================================================================================================
# Tests
# ==================================================================================================
project(test_matc)
set(TARGET test_matc)
set(SRCS
    tests/test_matc.cpp
    tests/test_lexersAndParsers.cpp
    tests/MockConfig.cpp
    tests/MockConfig.h)

add_executable(${TARGET} ${SRCS})

target_link_libraries(${TARGET} matlang gtest)
