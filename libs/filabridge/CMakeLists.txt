cmake_minimum_required(VERSION 3.19)
project(filabridge)

set(TARGET         filabridge)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE PUBLIC_HDRS ${PUBLIC_HDR_DIR}/**/*.h)

set(SRCS
        src/BufferInterfaceBlock.cpp
        src/DescriptorSets.cpp
        src/SamplerInterfaceBlock.cpp
        src/Variant.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>
        $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>
)
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

target_link_libraries(${TARGET} utils)
target_link_libraries(${TARGET} math)
target_link_libraries(${TARGET} backend_headers)

# ==================================================================================================
# Installation
# ==================================================================================================
install(DIRECTORY ${PUBLIC_HDR_DIR}/filament DESTINATION include)
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
