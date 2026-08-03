cmake_minimum_required(VERSION 3.19)
project(filamentapp C ASM)

if (FILAMENT_SKIP_SAMPLES)
    return()
endif()

set(TARGET filamentapp)
set(PUBLIC_HDR_DIR include)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set(RESOURCE_DIR  "${GENERATION_ROOT}/generated/resources")
set(MATERIAL_DIR  "${GENERATION_ROOT}/generated/material")

# ==================================================================================================
# Headers, sources, libraries, and resources.
# ==================================================================================================

set(PUBLIC_HDRS
        include/filamentapp/AssetLoader.h
        include/filamentapp/DesktopAssetLoader.h
        include/filamentapp/Config.h
        include/filamentapp/Cube.h
        include/filamentapp/Grid.h
        include/filamentapp/FilamentApp.h
        include/filamentapp/FilamentApp2.h
        include/filamentapp/IBL.h
        include/filamentapp/IcoSphere.h
        include/filamentapp/MeshAssimp.h
        include/filamentapp/NativeWindowHelper.h
        include/filamentapp/Sphere.h
)

set(SRCS
        src/Cube.cpp
        src/DesktopAssetLoader.cpp
        src/Grid.cpp
        src/FilamentApp.cpp
        src/FilamentApp2.cpp
        src/IBL.cpp
        src/IcoSphere.cpp
        src/MeshAssimp.cpp
        src/Sphere.cpp
        src/PlatformHelper.cpp
        src/PlatformHelper.h
        include/filamentapp/DisplayManager.h
        include/filamentapp/AppEvent.h
        src/FilamentAppGui.h
        src/FilamentAppGui.cpp
)

if (IS_HOST_PLATFORM)
    list(APPEND SRCS
            src/display_managers/SDLDisplayManager.h
            src/display_managers/SDLDisplayManager.cpp
            src/display_managers/HtmlDisplayManager.h
            src/display_managers/HtmlDisplayManager.cpp
    )
endif()

if (FILAMENT_SUPPORTS_VULKAN)
    list(APPEND SRCS
            src/backend_platforms/VulkanPlatformHelperCommon.h
            src/backend_platforms/VulkanPlatformHelperCommon.cpp
    )
endif()

set(LIBS
        assimp
        camutils
        filamat
        filament
        filament-iblprefilter
        geometry
        image
        ktxreader
        math
        stb
        utils
)

if (IS_HOST_PLATFORM)
    list(APPEND LIBS
        civetweb
        filagui
        imgui
        imageio
        sdl2
    )
else()
    list(APPEND LIBS
        imageio-lite
    )
endif()

set(MATERIAL_SRCS
        materials/aiDefaultMat.mat
        materials/aiDefaultTrans.mat
        materials/depthVisualizer.mat
        materials/transparentColor.mat
)

if (APPLE)
    list(APPEND SRCS src/display_managers/NativeWindowHelperCocoa.mm)
    if (FILAMENT_SUPPORTS_VULKAN)
        list(APPEND SRCS src/backend_platforms/VulkanPlatformHelperApple.mm)
    endif()
    if (FILAMENT_SUPPORTS_WEBGPU)
        list(APPEND SRCS src/backend_platforms/WebGPUPlatformHelperApple.mm)
    endif()
    list(APPEND LIBS "-framework Cocoa -framework QuartzCore")
endif()

if (LINUX)
    list(APPEND SRCS src/display_managers/NativeWindowHelperLinux.cpp)
    if (FILAMENT_SUPPORTS_VULKAN)
        list(APPEND SRCS src/backend_platforms/VulkanPlatformHelperLinux.cpp)
    endif()
    if (FILAMENT_SUPPORTS_WEBGPU)
        list(APPEND SRCS src/backend_platforms/WebGPUPlatformHelperLinux.cpp)
    endif()
endif()

if (WIN32)
    list(APPEND SRCS src/display_managers/NativeWindowHelperWindows.cpp)

    if (FILAMENT_SUPPORTS_VULKAN)
        list(APPEND SRCS src/backend_platforms/VulkanPlatformHelperWindows.cpp)
    endif()
    if (FILAMENT_SUPPORTS_WEBGPU)
        list(APPEND SRCS src/backend_platforms/WebGPUPlatformHelperWindows.cpp)
    endif()

    list(APPEND LIBS sdl2main)
endif()

if (ANDROID)
    if (FILAMENT_SUPPORTS_VULKAN)
        list(APPEND SRCS src/backend_platforms/VulkanPlatformHelperAndroid.cpp)
    endif()
    if (FILAMENT_SUPPORTS_WEBGPU)
        list(APPEND SRCS src/backend_platforms/WebGPUPlatformHelperAndroid.cpp)
    endif()
endif()

# ==================================================================================================
# Compile resources
# ==================================================================================================

if (CMAKE_CROSSCOMPILING OR FILAMENT_IMPORT_PREBUILT_EXECUTABLES)
    include(${IMPORT_EXECUTABLES})
endif()

file(MAKE_DIRECTORY ${MATERIAL_DIR})
file(MAKE_DIRECTORY ${RESOURCE_DIR})

set(RESOURCE_BINS)

set (MATC_FLAGS ${MATC_BASE_FLAGS})
if (FILAMENT_SAMPLES_STEREO_TYPE STREQUAL "instanced")
    set (MATC_FLAGS ${MATC_FLAGS} -PstereoscopicType=instanced)
    add_definitions(-DFILAMENT_SAMPLES_STEREO_TYPE_INSTANCED)
elseif (FILAMENT_SAMPLES_STEREO_TYPE STREQUAL "multiview")
    set (MATC_FLAGS ${MATC_FLAGS} -PstereoscopicType=multiview)
    add_definitions(-DFILAMENT_SAMPLES_STEREO_TYPE_MULTIVIEW)
endif ()

foreach (mat_src ${MATERIAL_SRCS})
    get_filename_component(localname "${mat_src}" NAME_WE)
    get_filename_component(fullname "${mat_src}" ABSOLUTE)
    set(output_path "${MATERIAL_DIR}/${localname}.filamat")
    add_custom_command(
            OUTPUT ${output_path}
            COMMAND matc ${MATC_FLAGS} -o ${output_path} ${fullname}
            MAIN_DEPENDENCY ${mat_src}
            DEPENDS matc
            COMMENT "Compiling material ${mat_src} to ${output_path}"
    )
    list(APPEND RESOURCE_BINS ${output_path})
endforeach()

get_resgen_vars(${RESOURCE_DIR} filamentapp)

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

add_library(filamentapp-resources ${DUMMY_SRC} ${RESGEN_SOURCE})
set_target_properties(filamentapp-resources PROPERTIES FOLDER Samples/Resources)

# Web Display Manager resources
set(WEB_RESOURCE_DIR "${GENERATION_ROOT}/generated/web_resources")
file(MAKE_DIRECTORY ${WEB_RESOURCE_DIR})

set(WEB_RESOURCE_BINS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/display_managers/web/index.html
        ${CMAKE_CURRENT_SOURCE_DIR}/src/display_managers/web/style.css
        ${CMAKE_CURRENT_SOURCE_DIR}/src/display_managers/web/app.js
)

get_resgen_vars(${WEB_RESOURCE_DIR} filamentapp_web_resources)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen -t ${RESGEN_FLAGS} ${WEB_RESOURCE_BINS}
        DEPENDS resgen ${WEB_RESOURCE_BINS}
        COMMENT "Bundling display manager web resources"
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

set(WEB_DUMMY_SRC "${WEB_RESOURCE_DIR}/dummy.c")
add_custom_command(OUTPUT ${WEB_DUMMY_SRC} COMMAND echo "//" > ${WEB_DUMMY_SRC})

add_library(filamentapp-web-resources ${WEB_DUMMY_SRC} ${RESGEN_SOURCE})
set_target_properties(filamentapp-web-resources PROPERTIES FOLDER Samples/Resources)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================

include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

if (FILAMENT_SUPPORTS_WEBGPU)
target_link_libraries(${TARGET} PRIVATE dawncpp_headers)
endif()

if (IS_HOST_PLATFORM)
    target_link_libraries(${TARGET} PUBLIC ${LIBS} filamentapp-resources filamentapp-web-resources)
else()
    target_link_libraries(${TARGET} PUBLIC ${LIBS} filamentapp-resources)
endif()

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)
target_include_directories(${TARGET} PRIVATE
        ${GENERATION_ROOT}
        ${ROOT_DIR}/imageio/include
        ${ROOT_DIR}/../third_party/libassimp/include
)

# ==================================================================================================
# Compiler flags
# ==================================================================================================

if (MSVC)
else()
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

# Multi-configuration generators, like Visual Studio or Xcode, place executable binaries in a
# sub-directory named after the configuration, like "Debug" or "Release".
# For these generators, in order to find assets, we must "walk" up an additional directory.
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (_isMultiConfig)
    target_compile_definitions(${TARGET} PRIVATE RELATIVE_ASSET_PATH="..")
else()
    target_compile_definitions(${TARGET} PRIVATE RELATIVE_ASSET_PATH=".")
endif()

if (IS_HOST_PLATFORM)
    target_compile_definitions(${TARGET} PRIVATE FILAMENTAPP_HAS_SDL FILAMENTAPP_HAS_IMGUI FILAMENTAPP_HAS_WEB_UI)
endif()


# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS filamentapp filamentapp-resources ARCHIVE DESTINATION lib/${DIST_DIR} LIBRARY DESTINATION lib/${DIST_DIR})
