cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)

project(PNGMINUS C)

option(PNGMINUS_USE_STATIC_LIBRARIES "Use the static library builds" ON)

# libpng
add_subdirectory(../.. libpng)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/libpng)
if(PNGMINUS_USE_STATIC_LIBRARIES)
  set(PNGMINUS_PNG_LIBRARY png_static)
else()
  set(PNGMINUS_PNG_LIBRARY png)
endif()

# png2pnm
add_executable(png2pnm png2pnm.c)
target_link_libraries(png2pnm ${PNGMINUS_PNG_LIBRARY})

# pnm2png
add_executable(pnm2png pnm2png.c)
target_link_libraries(pnm2png ${PNGMINUS_PNG_LIBRARY})
