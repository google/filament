cmake_minimum_required(VERSION 3.10)
project(matdbg C ASM)

set(TARGET matdbg)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================

set(PUBLIC_HDRS
    include/matdbg/JsonWriter.h
    include/matdbg/ShaderExtractor.h
    include/matdbg/ShaderInfo.h
    include/matdbg/TextWriter.h
)

set(SRCS
    src/CommonWriter.h
    src/JsonWriter.cpp
    src/ShaderExtractor.cpp
    src/ShaderInfo.cpp
    src/TextWriter.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================

include_directories(${PUBLIC_HDR_DIR} ${RESOURCE_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC
        utils
        filaflat
        filabridge
        backend
        utils
        SPIRV
        SPIRV-Tools
        spirv-cross-glsl
)

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# ==================================================================================================
# Compiler flags
# ==================================================================================================

target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)

if (MSVC OR CLANG_CL)
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
else()
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math>)
endif()
