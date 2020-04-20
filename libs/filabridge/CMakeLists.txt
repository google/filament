cmake_minimum_required(VERSION 3.10)
project(filabridge)

set(TARGET         filabridge)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE PUBLIC_HDRS ${PUBLIC_HDR_DIR}/**/*.h)

set(SRCS
        src/SamplerBindingMap.cpp
        src/SamplerInterfaceBlock.cpp
        src/UniformInterfaceBlock.cpp
        src/UibGenerator.cpp
        src/SibGenerator.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

target_link_libraries(${TARGET} utils)
target_link_libraries(${TARGET} math)
target_link_libraries(${TARGET} backend_headers)

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
install(DIRECTORY ${PUBLIC_HDR_DIR}/filament DESTINATION include)
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
