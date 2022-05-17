cmake_minimum_required(VERSION 3.19)
project(filament-tests)

if(IOS OR WEBGL)
    return()
endif()

# ==================================================================================================
# Filament test resources
# ==================================================================================================

set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set(RESOURCE_DIR  "${GENERATION_ROOT}/resources")
file(MAKE_DIRECTORY ${RESOURCE_DIR})

get_resgen_vars(${RESOURCE_DIR} filament_test_resources)

set(RESOURCE_BINS
        ${CMAKE_CURRENT_SOURCE_DIR}/test_material.filamat)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${RESOURCE_BINS}
        DEPENDS resgen ${RESOURCE_BINS}
        COMMENT "Aggregating resources"
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

# CMake fails to invoke ar on Windows unless there is at least one C/C++ file in the library.
set(DUMMY_SRC "${RESOURCE_DIR}/dummy.c")
add_custom_command(OUTPUT ${DUMMY_SRC} COMMAND echo "//" > ${DUMMY_SRC})
list(APPEND RESGEN_SOURCE ${DUMMY_SRC})

# ==================================================================================================
# Unit tests
# ==================================================================================================

# The following tests rely on private APIs that are stripped
# away in Release builds
if (TNT_DEV)
    add_executable(test_${TARGET}
            filament_AtlasAllocator_test.cpp
            filament_test_exposure.cpp
            filament_rendering_test.cpp
            filament_framegraph_test.cpp
            filament_test.cpp)

    target_link_libraries(test_${TARGET} PRIVATE filament gtest)
    target_compile_options(test_${TARGET} PRIVATE ${COMPILER_FLAGS})
    set_target_properties(test_${TARGET} PROPERTIES FOLDER Tests)

    add_executable(test_depth depth_test.cpp)
    target_link_libraries(test_depth PRIVATE utils)
endif()

if (ANDROID)
    add_executable(test_compiler compiler_test.cpp)
    target_link_libraries(test_compiler PRIVATE gtest)
    target_link_libraries(test_compiler PRIVATE utils)
    target_link_libraries(test_compiler PRIVATE EGL)
    target_link_libraries(test_compiler PRIVATE GLESv3)
endif()

add_executable(test_material_parser
        filament_test_material_parser.cpp
        ${RESGEN_SOURCE})
target_link_libraries(test_material_parser PRIVATE filament gtest)
target_compile_options(test_material_parser PRIVATE ${COMPILER_FLAGS})
target_include_directories(test_material_parser PRIVATE ${RESOURCE_DIR})
set_target_properties(test_material_parser PROPERTIES FOLDER Tests)
