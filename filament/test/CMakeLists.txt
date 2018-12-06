cmake_minimum_required(VERSION 3.1)
project(filament-tests)

# ==================================================================================================
# Unit tests
# ==================================================================================================

if(NOT IOS)
    # The following tests rely on private APIs that are stripped
    # away in Release builds
    if (TNT_DEV)
        add_executable(test_${TARGET} filament_test.cpp)
        target_link_libraries(test_${TARGET} PRIVATE filament gtest)
        target_compile_options(test_${TARGET} PRIVATE ${COMPILER_FLAGS})

        add_executable(test_${TARGET}_exposure filament_test_exposure.cpp)
        target_link_libraries(test_${TARGET}_exposure PRIVATE filament gtest)
        target_compile_options(test_${TARGET}_exposure PRIVATE ${COMPILER_FLAGS})

        add_executable(test_depth depth_test.cpp)
    endif()
endif()
