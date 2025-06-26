cmake_minimum_required(VERSION 3.19)
project(filament C ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLTF_COMPARISON_SOURCES
        src/test_CompareGLTF.cpp
        src/GLTFViewer.cpp
        src/ImageExpectations.cpp
        src/main.cpp
    )

file(COPY glTF_cases DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/glTF_cases)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/images/actual_images)
file(COPY expected_images DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/images)

enable_testing()

add_executable(gltf_comparison ${GLTF_COMPARISON_SOURCES})
target_include_directories(gltf_comparison PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(gltf_comparison
        gtest
        filamentapp
        gltfio
        absl::str_format
)



include(GoogleTest)
gtest_discover_tests(gltf_comparison)