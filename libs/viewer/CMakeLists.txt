cmake_minimum_required(VERSION 3.19)
project(viewer)

set(TARGET viewer)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/viewer/AutomationEngine.h
        include/viewer/AutomationSpec.h
        include/viewer/RemoteServer.h
        include/viewer/Settings.h
        include/viewer/SimpleViewer.h
        include/viewer/TweakableProperty.h
        include/viewer/TweakableMaterial.h
        include/viewer/CustomFileDialogs.h
        include/viewer/json.hpp
)

set(SRCS
        src/jsonParseUtils.h
        src/AutomationEngine.cpp
        src/AutomationSpec.cpp
        src/RemoteServer.cpp
        src/Settings.cpp
        src/SimpleViewer.cpp
        src/TweakableMaterial.cpp
        src/CustomFileDialogs.cpp
)

if (APPLE)
    list(APPEND SRCS
        src/MacosCustomFileDialogs.mm
    )

    find_library(UTI_FRAMEWORK UniformTypeIdentifiers)
    mark_as_advanced(UTI_FRAMEWORK)

    set(PLATFORM_ADDITIONAL_LIBS ${UTI_FRAMEWORK})
endif()

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_link_libraries(${TARGET} PUBLIC imgui filament gltfio_core filagui jsmn civetweb ${PLATFORM_ADDITIONAL_LIBS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC)
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
else()
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math>)
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/viewer DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================
if (NOT ANDROID AND NOT WEBGL AND NOT IOS)
    add_executable(test_settings tests/test_settings.cpp)
    target_link_libraries(test_settings PRIVATE ${TARGET} gtest)
endif()
