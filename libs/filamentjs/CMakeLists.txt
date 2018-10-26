cmake_minimum_required(VERSION 3.1)

project(filamentjs)

set(JAVASCRIPT_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/wasmloader.js
  ${CMAKE_CURRENT_SOURCE_DIR}/extensions.js
  ${CMAKE_CURRENT_SOURCE_DIR}/utilities.js)

set(CPP_SRC
  jsenums.cpp
  jsbindings.cpp)

# The emcc options are not documented well, the best place to find them is the source:
# https://github.com/kripken/emscripten/blob/master/src/settings.js

set(COPTS "${COPTS} -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0")

# Uncomment the following line to check for leaks with test_leaks.html
# set(LOPTS "${LOPTS} --memoryprofiler")

set(LOPTS "${LOPTS} --bind")
set(LOPTS "${LOPTS} -s FILESYSTEM=0")
set(LOPTS "${LOPTS} -s ALLOW_MEMORY_GROWTH=1")
set(LOPTS "${LOPTS} -s MODULARIZE_INSTANCE=1")
set(LOPTS "${LOPTS} -s EXPORT_NAME=Filament")
foreach (JS_FILENAME ${JAVASCRIPT_SRC})
  set(LOPTS "${LOPTS} --post-js ${JS_FILENAME}")
endforeach()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LOPTS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COPTS}")

add_executable(filamentjs ${CPP_SRC})

set_target_properties(filamentjs PROPERTIES
    LINK_DEPENDS "${JAVASCRIPT_SRC}"
    OUTPUT_NAME filament)

target_link_libraries(filamentjs PRIVATE filament math utils image filameshio stb)
