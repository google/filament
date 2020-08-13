cmake_minimum_required(VERSION 3.10)
project(matdbg C ASM)

set(TARGET matdbg)
set(PUBLIC_HDR_DIR include)

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

# ==================================================================================================
# Sources and headers
# ==================================================================================================

set(PUBLIC_HDRS
    include/matdbg/DebugServer.h
    include/matdbg/JsonWriter.h
    include/matdbg/ShaderReplacer.h
    include/matdbg/ShaderExtractor.h
    include/matdbg/ShaderInfo.h
    include/matdbg/TextWriter.h
)

set(SRCS
    src/CommonWriter.h
    src/CommonWriter.cpp
    src/DebugServer.cpp
    src/JsonWriter.cpp
    src/ShaderReplacer.cpp
    src/ShaderExtractor.cpp
    src/ShaderInfo.cpp
    src/TextWriter.cpp
)

# ==================================================================================================
# Resources
# ==================================================================================================

set(RESOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(RESOURCE_BINS
        ${CMAKE_CURRENT_SOURCE_DIR}/web/style.css
        ${CMAKE_CURRENT_SOURCE_DIR}/web/script.js
        ${CMAKE_CURRENT_SOURCE_DIR}/web/index.html
)

get_resgen_vars(${RESOURCE_DIR} matdbg_resources)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen -t ${RESGEN_FLAGS} ${RESOURCE_BINS}
        DEPENDS resgen ${RESOURCE_BINS}
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

set(DUMMY_SRC "${RESOURCE_DIR}/dummy.c")
add_custom_command(OUTPUT ${DUMMY_SRC} COMMAND echo "//" > ${DUMMY_SRC})

add_library(matdbg_resources ${DUMMY_SRC} ${RESGEN_SOURCE})

# ==================================================================================================
# Include and target definitions
# ==================================================================================================

include_directories(${PUBLIC_HDR_DIR} ${RESOURCE_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC
        utils
        filaflat
        filabridge
        backend_headers
        civetweb
        utils
        SPIRV
        SPIRV-Tools
        spirv-cross-glsl
        matdbg_resources
)

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# ==================================================================================================
# Compiler flags
# ==================================================================================================

if (MSVC)
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
else()
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math>)
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================

# matdbg has dependencies on non-installed libraries. Here we bundle them all together into a single
# library that gets copied into the installation folder so users are only required to link against
# matdbg.
set(MATDBG_DEPS
    matdbg
    matdbg_resources
    civetweb
    SPIRV-Tools
    spirv-cross-core
    spirv-cross-glsl
    )

set(MATDBG_COMBINED_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libmatdbg_combined.a")
combine_static_libs(matdbg "${MATDBG_COMBINED_OUTPUT}" "${MATDBG_DEPS}")

set(MATDBG_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}matdbg${CMAKE_STATIC_LIBRARY_SUFFIX})
install(FILES "${MATDBG_COMBINED_OUTPUT}" DESTINATION lib/${DIST_DIR} RENAME ${MATDBG_LIB_NAME})
install(DIRECTORY ${PUBLIC_HDR_DIR}/matdbg DESTINATION include)
