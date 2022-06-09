cmake_minimum_required(VERSION 3.19)
project(filaflat)

set(TARGET         filaflat)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE HDRS include/filaflat/*.h)

set(SRCS
        src/ChunkContainer.cpp
        src/DictionaryReader.cpp
        src/MaterialChunk.cpp
        src/Unflattener.cpp)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} ${HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

if (CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(${TARGET} PROPERTIES
                          C_VISIBILITY_PRESET default
                          CXX_VISIBILITY_PRESET default)
endif()

target_link_libraries(${TARGET} filabridge utils)

if (FILAMENT_SUPPORTS_VULKAN)
    target_link_libraries(${TARGET} smol-v)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
# We do not need filaflat headers in the install directory
# install(DIRECTORY ${PUBLIC_HDR_DIR}/filaflat DESTINATION include)
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
