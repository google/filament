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
        include/filamentapp/Config.h
        include/filamentapp/Cube.h
        include/filamentapp/FilamentApp.h
        include/filamentapp/IBL.h
        include/filamentapp/IcoSphere.h
        include/filamentapp/MeshAssimp.h
        include/filamentapp/NativeWindowHelper.h
        include/filamentapp/Sphere.h
)

set(SRCS
        src/Cube.cpp
        src/FilamentApp.cpp
        src/IBL.cpp
        src/IcoSphere.cpp
        src/MeshAssimp.cpp
        src/Sphere.cpp
)

set(LIBS
        assimp
        camutils
        filagui
        filamat
        filament
        filament-iblprefilter
        geometry
        getopt
        image
        imageio
        imgui
        ktxreader
        math
        sdl2
        stb
        utils
)

set(MATERIAL_SRCS
        materials/aiDefaultMat.mat
        materials/aiDefaultTrans.mat
        materials/depthVisualizer.mat
        materials/transparentColor.mat
)

if (APPLE)
    list(APPEND SRCS src/NativeWindowHelperCocoa.mm)
    list(APPEND LIBS "-framework Cocoa -framework QuartzCore")
endif()

if (LINUX)
    list(APPEND SRCS src/NativeWindowHelperLinux.cpp)
endif()

if (WIN32)
    list(APPEND SRCS src/NativeWindowHelperWindows.cpp)
    list(APPEND LIBS sdl2main)
endif()

# ==================================================================================================
# Compile resources
# ==================================================================================================

if (CMAKE_CROSSCOMPILING)
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

# ==================================================================================================
# Include and target definitions
# ==================================================================================================

include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC ${LIBS} filamentapp-resources)

target_include_directories(${TARGET} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>
    $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>
)
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})

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
