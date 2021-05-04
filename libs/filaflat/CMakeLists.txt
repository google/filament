cmake_minimum_required(VERSION 3.10)
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
        src/ShaderBuilder.cpp
        src/Unflattener.cpp)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} ${HDRS} ${SRCS})

if(HUNTER_ENABLED)
  target_include_directories(${TARGET} PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>  
    $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>)
else()
  target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
endif()

target_link_libraries(${TARGET} filabridge utils)

if (FILAMENT_SUPPORTS_VULKAN)
  if(HUNTER_ENABLED)
    target_link_libraries(${TARGET} smol-v::smol-v)
  else()
    target_link_libraries(${TARGET} smol-v)
  endif()
endif()

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (NOT WIN32)
    target_compile_options(${TARGET} PRIVATE
        -fvisibility=hidden
    )
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
# We do not need filaflat headers in the install directory
# install(DIRECTORY ${PUBLIC_HDR_DIR}/filaflat DESTINATION include)
if(HUNTER_ENABLED)
  install(TARGETS ${TARGET} EXPORT ${TARGETS_EXPORT_NAME} ARCHIVE DESTINATION lib/${DIST_DIR})
else()
  install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
endif()