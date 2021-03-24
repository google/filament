cmake_minimum_required(VERSION 3.10)
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
)

set(SRCS
        src/jsonParseUtils.h
        src/AutomationEngine.cpp
        src/AutomationSpec.cpp
        src/RemoteServer.cpp
        src/Settings.cpp
        src/SimpleViewer.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_link_libraries(${TARGET} PUBLIC imgui filament gltfio_core filagui jsmn civetweb)
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# ==================================================================================================
# Generate AST (experimental)
# ==================================================================================================

set(filament_includes "-I$<JOIN:$<TARGET_PROPERTY:filament,INCLUDE_DIRECTORIES>,;-I>")
set(viewer_includes "-I$<JOIN:$<TARGET_PROPERTY:viewer,INCLUDE_DIRECTORIES>,;-I>")

add_custom_command(
    OUTPUT SettingsAST.json
    COMMAND clang
        "-Xclang;-ast-dump=json;-fsyntax-only;-std=c++17"
        ";${filament_includes};${viewer_includes}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Stub.cpp;>;SettingsAST.json"
    COMMAND_EXPAND_LISTS
    DEPENDS include/viewer/Settings.h
    COMMENT "Dumping AST"
)

add_custom_target(SettingsAST DEPENDS SettingsAST.json)
add_dependencies(${TARGET} SettingsAST)

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC)
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
else()
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math>)
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
