cmake_minimum_required(VERSION 3.19)

# ==================================================================================================
# Library
# ==================================================================================================
project(matlang)
set(TARGET matlang)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(HDRS
        src/matc/CommandlineConfig.h
        src/matc/Compiler.h
        src/matc/MaterialCompiler.h
        )

set(SRCS
        src/matc/Compiler.cpp
        src/matc/MaterialCompiler.cpp
        src/matc/CommandlineConfig.cpp
        )

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_library(${TARGET} STATIC ${SRCS} ${HDRS})

target_include_directories(${TARGET} PUBLIC src)
target_include_directories(${TARGET} PRIVATE ${filamat_SOURCE_DIR}/src)

target_link_libraries(${TARGET} getopt filamat filabridge utils matp)
set_target_properties(${TARGET} PROPERTIES FOLDER Tools)

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
set_target_properties(${TARGET} PROPERTIES FOLDER Tools)

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
