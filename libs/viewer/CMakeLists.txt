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
        include/viewer/ViewerGui.h
)

set(SRCS
        src/jsonParseUtils.h
        src/AutomationEngine.cpp
        src/AutomationSpec.cpp
        src/RemoteServer.cpp
        src/Settings.cpp
        src/Settings_generated.cpp
        src/Settings_generated.h
        src/ViewerGui.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_link_libraries(${TARGET} PUBLIC imgui filament gltfio_core filagui jsmn civetweb)
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

if (FILAMENT_SAMPLES_STEREO_TYPE STREQUAL "instanced")
    add_definitions(-DFILAMENT_SAMPLES_STEREO_TYPE_INSTANCED)
elseif (FILAMENT_SAMPLES_STEREO_TYPE STREQUAL "multiview")
    add_definitions(-DFILAMENT_SAMPLES_STEREO_TYPE_MULTIVIEW)
endif ()

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC)
else()
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
if(FILAMENT_TESTS AND NOT ANDROID AND NOT WEBGL AND NOT IOS)
    add_executable(test_settings tests/test_settings.cpp)
    target_link_libraries(test_settings PRIVATE ${TARGET} gtest)
    set_target_properties(test_settings PROPERTIES FOLDER Tests)
endif()
