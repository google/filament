cmake_minimum_required(VERSION 3.1)

project(filamentjs)

# The emcc options are not documented well, the best place to find them is the source:
# https://github.com/kripken/emscripten/blob/master/src/settings.js

set(COPTS "${COPTS} -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0")

set(LOPTS "${LOPTS} --bind")
set(LOPTS "${LOPTS} --post-js ${CMAKE_CURRENT_SOURCE_DIR}/wasmloader.js")
set(LOPTS "${LOPTS} -s ALLOW_MEMORY_GROWTH=1")
set(LOPTS "${LOPTS} -s MODULARIZE_INSTANCE=1")
set(LOPTS "${LOPTS} -s EXPORT_NAME=Filament")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LOPTS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COPTS}")

add_executable(filamentjs jsenums.cpp jsbindings.cpp)
set_target_properties(filamentjs PROPERTIES OUTPUT_NAME filament)
target_link_libraries(filamentjs PRIVATE filament math utils image)
