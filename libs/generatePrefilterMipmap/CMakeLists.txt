cmake_minimum_required(VERSION 3.19)
project(filament C ASM)

set(TARGET filament-generatePrefilterMipmap)
set(PUBLIC_HDR_DIR include)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/${TARGET}/generatePrefilterMipmap.h
)

set(SRCS
        src/generatePrefilterMipmap.cpp
)

# ==================================================================================================
# Includes & target definition
# ==================================================================================================
# specify where our headers are
include_directories(${PUBLIC_HDR_DIR})
include_directories(src)

# we're building a library
add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

# ==================================================================================================
# Dependencies
# ==================================================================================================

target_link_libraries(${TARGET} PUBLIC math)
target_link_libraries(${TARGET} PUBLIC utils)
target_link_libraries(${TARGET} PUBLIC ibl)
target_link_libraries(${TARGET} PRIVATE filament)

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC)
    set(OPTIMIZATION_FLAGS
        /fp:fast
    )
elseif(WEBGL)
    # Avoid strict-vtable-pointers here, it is broken in WebAssembly.
    set(OPTIMIZATION_FLAGS -fvisibility-inlines-hidden)
else()
    set(OPTIMIZATION_FLAGS
        -ffast-math
        -fno-finite-math-only
        -ffp-contract=fast
        # TODO: aggressive vectorization is currently broken on Android
        #        -fslp-vectorize-aggressive
        -fvisibility-inlines-hidden
        -fstrict-vtable-pointers
    )
endif()

target_compile_options(${TARGET} PRIVATE
        ${FILAMENT_WARNINGS}
        $<$<CONFIG:Release>:${OPTIMIZATION_FLAGS}>
        $<$<AND:$<PLATFORM_ID:Darwin>,$<CONFIG:Release>>:${DARWIN_OPTIMIZATION_FLAGS}>
)

target_link_libraries(${TARGET} PRIVATE
        $<$<AND:$<PLATFORM_ID:Linux>,$<CONFIG:Release>>:${LINUX_LINKER_OPTIMIZATION_FLAGS}>
)

# ==================================================================================================
# Installation
# ==================================================================================================
set(INSTALL_TYPE ARCHIVE)
install(TARGETS ${TARGET} ${INSTALL_TYPE} DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/${TARGET} DESTINATION include)
