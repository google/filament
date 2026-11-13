cmake_minimum_required(VERSION 3.19)
project(taa_harness)

add_library(taa_metrics STATIC
    taa_metrics.cpp
    taa_metrics.h
)
target_include_directories(taa_metrics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(taa_metrics PUBLIC image math utils)
set_target_properties(taa_metrics PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    FOLDER Tools
)

if (FILAMENT_BUILD_TESTING AND NOT ANDROID AND NOT WASM AND NOT IOS)
    add_executable(test_taa_metrics tests/test_taa_metrics.cpp)
    target_link_libraries(test_taa_metrics PRIVATE taa_metrics gtest)
    set_target_properties(test_taa_metrics PROPERTIES FOLDER Tests)
endif()

if (FILAMENT_SKIP_SAMPLES)
    return()
endif()

set(TARGET taa_harness)

if (ANDROID)
    add_library(${TARGET} SHARED taa_harness.cpp)
    install(TARGETS ${TARGET} LIBRARY DESTINATION lib/${DIST_DIR})
else()
    add_executable(${TARGET} taa_harness.cpp)
endif()

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../samples)

target_link_libraries(${TARGET} PRIVATE
    taa_metrics
    samples-common
    uberarchive
    viewer
    filamentapp
)

if (ANDROID)
    target_link_libraries(${TARGET} PRIVATE android log)
endif()

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
