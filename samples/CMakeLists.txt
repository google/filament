cmake_minimum_required(VERSION 3.1)
project(filament-samples)

# ==================================================================================================
# Build materials
# ==================================================================================================

set(MATERIAL_SRCS
        materials/bakedColor.mat
        materials/bakedTexture.mat
        materials/aiDefaultMat.mat
        materials/aiDefaultTrans.mat
        materials/depthVisualizer.mat
        materials/groundShadow.mat
        materials/sandboxCloth.mat
        materials/sandboxLit.mat
        materials/sandboxLitFade.mat
        materials/sandboxLitTransparent.mat
        materials/sandboxSubsurface.mat
        materials/sandboxUnlit.mat
        materials/transparentColor.mat)

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

set(MATERIAL_BINS)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
file(MAKE_DIRECTORY "${GENERATION_ROOT}/generated/material/")

set(MATC_TARGET desktop)

# Optimize materials for performance in Release mode.
set(MATC_FLAGS -a all)
if (CMAKE_BUILD_TYPE MATCHES Release)
    set(MATC_FLAGS -O ${MATC_FLAGS})
endif()

foreach (mat_src ${MATERIAL_SRCS})
    get_filename_component(localname "${mat_src}" NAME_WE)
    get_filename_component(fullname "${mat_src}" ABSOLUTE)
    set(output_path "${GENERATION_ROOT}/generated/material/${localname}.inc")
    add_custom_command(
            OUTPUT ${output_path}
            COMMAND matc ${MATC_FLAGS} -p ${MATC_TARGET} -m material -f header
            -o ${output_path} ${fullname}
            MAIN_DEPENDENCY ${mat_src}
            DEPENDS matc
            COMMENT "Compiling material ${mat_src} to ${output_path}"
    )
    list(APPEND MATERIAL_BINS ${output_path})
endforeach()

add_custom_target(sample_materials DEPENDS ${MATERIAL_BINS})

# ==================================================================================================
# Common library
# ==================================================================================================

set(APP_LIBS filament sdl2 stb math filamat utils getopt imgui filagui)
if (WIN32)
    list(APPEND APP_LIBS sdl2main)
endif()

set(APP_SRCS
        app/CameraManipulator.cpp
        app/Cube.cpp
        app/FilamentApp.cpp
        app/IBL.cpp
        app/Image.cpp
        app/IcoSphere.cpp
        app/Sphere.cpp)

if (APPLE)
    list(APPEND APP_SRCS app/NativeWindowHelperCocoa.mm)
    list(APPEND APP_LIBS "-framework Cocoa")
endif()

if (LINUX)
    list(APPEND APP_SRCS app/NativeWindowHelperLinux.cpp)
endif()

if (WIN32)
    list(APPEND APP_SRCS app/NativeWindowHelperWindows.cpp)
endif()

include_directories(app)

add_library(sample-app ${APP_SRCS})
add_dependencies(sample-app sample_materials)
target_include_directories(sample-app PUBLIC app)

target_link_libraries(sample-app PRIVATE ${APP_LIBS})

list(APPEND APP_LIBS sample-app)

# ==================================================================================================
# Compiler flags
# ==================================================================================================

set(COMPILER_FLAGS
        -Wno-extern-c-compat
        $<$<NOT:$<PLATFORM_ID:Linux>>:-Wno-address-of-packed-member>)

if (MSVC OR CLANG_CL)
    set(COMPILER_FLAGS ${COMPILER_FLAGS} $<$<CONFIG:Release>:/fp:fast>)
else()
    set(COMPILER_FLAGS ${COMPILER_FLAGS} $<$<CONFIG:Release>:-ffast-math>)
endif()

# ==================================================================================================
# Test apps
# ==================================================================================================

function(add_assimp_demo NAME)
    include_directories(${GENERATION_ROOT})
    add_executable(
            ${NAME}
            ${NAME}.cpp
            app/MeshAssimp.cpp)
    add_dependencies(${NAME} sample_materials)
    target_link_libraries(${NAME} PRIVATE ${APP_LIBS} assimp)
    target_compile_options(${NAME} PRIVATE ${COMPILER_FLAGS})
endfunction()

function(add_filamesh_demo NAME)
    include_directories(${GENERATION_ROOT})
    add_executable(
            ${NAME}
            ${NAME}.cpp
            app/MeshIO.cpp)
    add_dependencies(${NAME} sample_materials)
    target_link_libraries(${NAME} PRIVATE ${APP_LIBS})
    target_compile_options(${NAME} PRIVATE ${COMPILER_FLAGS})
endfunction()

if (NOT ANDROID)
    add_assimp_demo(frame_generator)
    add_assimp_demo(lightbulb)
    add_assimp_demo(material_sandbox)
    add_assimp_demo(sample_full_pbr)
    add_assimp_demo(sample_pbr)
    add_assimp_demo(sample_position_offset)

    add_filamesh_demo(sample_cloth)
    add_filamesh_demo(sample_subsurface)
    add_filamesh_demo(vk_imgui)
    add_filamesh_demo(sample_normal_map)
    add_filamesh_demo(sample_opacity_mask)

    # Sample app specific
    target_link_libraries(frame_generator PRIVATE image imageio)
endif()

# ==================================================================================================
# Copy the MoltenVK dylibs and JSON on MacOS
# ==================================================================================================

if (APPLE AND NOT Vulkan_LIBRARY)
    message(STATUS "No Vulkan SDK was found, using prebuilt MoltenVK.")
    set(MOLTENVK_DIR "../third_party/moltenvk")
    configure_file(
            ${MOLTENVK_DIR}/libvulkan.1.dylib
            ${PROJECT_BINARY_DIR}/libvulkan.1.dylib COPYONLY)
    configure_file(
            ${MOLTENVK_DIR}/MoltenVK_icd.json
            ${PROJECT_BINARY_DIR}/MoltenVK_icd.json COPYONLY)
    configure_file(
            ${MOLTENVK_DIR}/libMoltenVK.dylib
            ${PROJECT_BINARY_DIR}/libMoltenVK.dylib COPYONLY)
    list(APPEND APP_SRCS
            ${PROJECT_BINARY_DIR}/libvulkan.1.dylib
            ${PROJECT_BINARY_DIR}/MoltenVK_icd.json
            ${PROJECT_BINARY_DIR}/libMoltenVK.dylib)
endif()

# ==================================================================================================
# Build Vulkan Executables
# ==================================================================================================

if (FILAMENT_SUPPORTS_VULKAN)
    add_assimp_demo(vk_strobecolor)
    add_assimp_demo(vk_hellotriangle)
    add_assimp_demo(vk_depthtesting)
    add_assimp_demo(vk_texturedquad)
    add_assimp_demo(vk_hellopbr)
    add_assimp_demo(vk_shadowtest)
    add_assimp_demo(vk_animation)
    add_assimp_demo(vk_vbotest)
    add_assimp_demo(vk_viewtest)
endif()

# ==================================================================================================
# Copy Assets
#
# This creates a structure like:
#
#    /lightbulb (etc)  sample app executable
#    /assets/fonts     copied from REPO/assets
#    /assets/models    copied from REPO/assets
#    /textures         copied from REPO/third_party
#    /envs             copied from REPO/samples/envs
#
# ==================================================================================================

file(COPY ../third_party/textures DESTINATION ${PROJECT_BINARY_DIR})
add_custom_target(textures ALL DEPENDS textures)
add_dependencies(filament textures)

file(COPY ../assets DESTINATION ${PROJECT_BINARY_DIR}
        PATTERN "reference" EXCLUDE
        PATTERN "environments" EXCLUDE)
add_custom_target(assets ALL DEPENDS assets)
add_dependencies(filament assets)

file(COPY ../samples/envs DESTINATION ${PROJECT_BINARY_DIR})
add_custom_target(envs ALL DEPENDS envs)
add_dependencies(filament envs)
