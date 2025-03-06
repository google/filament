cmake_minimum_required(VERSION 3.19)
project(fgviewer C ASM)

set(TARGET fgviewer)
set(PUBLIC_HDR_DIR include)

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

# ==================================================================================================
# Sources and headers
# ==================================================================================================

set(PUBLIC_HDRS
    include/fgviewer/DebugServer.h
    include/fgviewer/JsonWriter.h
    include/fgviewer/FrameGraphInfo.h
)

set(SRCS
    src/ApiHandler.cpp
    src/ApiHandler.h
    src/DebugServer.cpp
    src/FrameGraphInfo.cpp
    src/JsonWriter.cpp
)

# ==================================================================================================
# Resources
# ==================================================================================================

set(RESOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(RESOURCE_BINS
        ${CMAKE_CURRENT_SOURCE_DIR}/web/api.js
        ${CMAKE_CURRENT_SOURCE_DIR}/web/app.js
        ${CMAKE_CURRENT_SOURCE_DIR}/web/index.html
)

get_resgen_vars(${RESOURCE_DIR} fgviewer_resources)

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

add_library(fgviewer_resources ${DUMMY_SRC} ${RESGEN_SOURCE})
set_target_properties(fgviewer_resources PROPERTIES FOLDER Libs)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================

include_directories(${PUBLIC_HDR_DIR} ${RESOURCE_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC
        civetweb
        fgviewer_resources
        utils
)

target_include_directories(${TARGET} PRIVATE ${filamat_SOURCE_DIR}/src)

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

# ==================================================================================================
# Compiler flags
# ==================================================================================================

if (MSVC)
else()
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================

# fgviewer has dependencies on non-installed libraries. Here we bundle them all together into a single
# library that gets copied into the installation folder so users are only required to link against
# fgviewer.
set(FGVIEWER_DEPS
    fgviewer
    fgviewer_resources
    civetweb
    )

set(FGVIEWER_COMBINED_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libfgviewer_combined.a")
combine_static_libs(fgviewer "${FGVIEWER_COMBINED_OUTPUT}" "${FGVIEWER_DEPS}")

set(FGVIEWER_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}fgviewer${CMAKE_STATIC_LIBRARY_SUFFIX})
install(FILES "${FGVIEWER_COMBINED_OUTPUT}" DESTINATION lib/${DIST_DIR} RENAME ${FGVIEWER_LIB_NAME})
# We do not need fgviewer headers in the install directory
# install(DIRECTORY ${PUBLIC_HDR_DIR}/fgviewer DESTINATION include)
