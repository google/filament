cmake_minimum_required(VERSION 3.19)
project(taa_harness)

if (FILAMENT_SKIP_SAMPLES)
    return()
endif()

set(TARGET taa_harness)

add_executable(${TARGET} taa_harness.cpp)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../samples)

target_link_libraries(${TARGET} PRIVATE
    samples-common
    uberarchive
    gltfio
    viewer
    imageio
    image
    filamentapp
    filament
    backend
    utils
    math
)

if (NOT MSVC)
    target_compile_options(${TARGET} PRIVATE
        -Wno-extern-c-compat
        $<$<NOT:$<PLATFORM_ID:Linux>>:-Wno-address-of-packed-member>
    )
endif()

if (APPLE)
    set_target_properties(${TARGET} PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH /usr/local/lib
    )
endif()

set_target_properties(${TARGET} PROPERTIES FOLDER Tools)

