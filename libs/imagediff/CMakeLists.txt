add_library(imagediff STATIC
    src/ImageDiff.cpp
)

target_include_directories(imagediff PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(imagediff PUBLIC
    image
    utils
)

target_link_libraries(imagediff PRIVATE
    jsmn
)

if (NOT ANDROID AND NOT WEBGL AND NOT IOS)
    add_executable(test_imagediff tests/test_ImageDiff.cpp)
    target_link_libraries(test_imagediff PRIVATE imagediff gtest image utils)
endif()
