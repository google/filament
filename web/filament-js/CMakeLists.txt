cmake_minimum_required(VERSION 3.19)

project(filament-js)

set(EXTERN_POSTJS_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/wasmloader.js
  ${CMAKE_CURRENT_SOURCE_DIR}/extensions_generated.js
  ${CMAKE_CURRENT_SOURCE_DIR}/extensions.js
  ${CMAKE_CURRENT_SOURCE_DIR}/utilities.js)

set(CPP_SRC
  jsenums.cpp
  jsenums_generated.cpp
  jsbindings.cpp
  jsbindings_generated.cpp)

# The emcc options are not documented well, the best place to find them is the source:
# https://github.com/kripken/emscripten/blob/main/src/settings.js

if (WEBGL_PTHREADS)
  set(COPTS "${COPTS} -pthread")
  set(LOPTS "${LOPTS} -pthread")
endif()

# The following setting is required because we disable RTTI.
set(COPTS "${COPTS} -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0")

# Uncomment the following line to check for leaks
# set(LOPTS "${LOPTS} --memoryprofiler")

set(LOPTS "${LOPTS} --bind")
set(LOPTS "${LOPTS} -s ALLOW_MEMORY_GROWTH=1")
set(LOPTS "${LOPTS} -s MODULARIZE=1")
set(LOPTS "${LOPTS} -s EXPORT_NAME=Filament")
set(LOPTS "${LOPTS} -s USE_WEBGL2=1")
set(LOPTS "${LOPTS} -s FULL_ES3")
set(LOPTS "${LOPTS} -s MIN_WEBGL_VERSION=2")
set(LOPTS "${LOPTS} -s MAX_WEBGL_VERSION=2")

foreach (JS_FILENAME ${EXTERN_POSTJS_SRC})
  set(LOPTS "${LOPTS} --extern-post-js ${JS_FILENAME}")
endforeach()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LOPTS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COPTS}")

add_executable(filament-js ${CPP_SRC})

set_target_properties(filament-js PROPERTIES
    LINK_DEPENDS "${EXTERN_POSTJS_SRC}"
    OUTPUT_NAME filament)

target_link_libraries(filament-js PRIVATE filament math utils ktxreader filameshio uberarchive gltfio_core viewer)

# Copy package.json (et al) into the output folder to allow "npm publish" to gather the
# artifacts that it needs.

set(RESOURCES )
foreach (RESOURCE package.json README.md filament.d.ts filament-viewer.js)
    add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/${RESOURCE}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE} ${PROJECT_BINARY_DIR})
    list(APPEND RESOURCES ${PROJECT_BINARY_DIR}/${RESOURCE})
endforeach()

add_custom_target(npm_package ALL DEPENDS ${RESOURCES})
