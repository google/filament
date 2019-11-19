cmake_minimum_required(VERSION 3.10)
project(filament-tests)

# ==================================================================================================
# Unit tests
# ==================================================================================================

if(NOT IOS AND NOT WEBGL)
    # The following tests rely on private APIs that are stripped
    # away in Release builds
    if (TNT_DEV)
        add_executable(test_${TARGET} filament_test_exposure.cpp filament_rendering_test.cpp filament_framegraph_test.cpp filament_test.cpp)
        target_link_libraries(test_${TARGET} PRIVATE filament gtest)
        target_compile_options(test_${TARGET} PRIVATE ${COMPILER_FLAGS})

        add_executable(test_depth depth_test.cpp)
        target_link_libraries(test_depth PRIVATE utils)
    endif()
endif()
