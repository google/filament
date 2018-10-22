cmake_minimum_required(VERSION 3.1)
project(filament-tests)

# ==================================================================================================
# Unit tests
# ==================================================================================================

if (ANDROID)
    add_executable(filament_calls filament_calls.cpp)
    target_link_libraries(filament_calls PRIVATE utils filament)
    target_compile_options(filament_calls PRIVATE ${COMPILER_FLAGS})

    add_executable(filament_benchmark filament_benchmark.cpp)
    target_link_libraries(filament_benchmark PRIVATE utils filament)
    target_compile_options(filament_benchmark PRIVATE ${COMPILER_FLAGS})
elseif(NOT IOS)
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
