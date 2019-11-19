cmake_minimum_required(VERSION 3.10)
project(filament-samples C ASM)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set(RESOURCE_DIR  "${GENERATION_ROOT}/generated/resources")
set(MATERIAL_DIR  "${GENERATION_ROOT}/generated/material")
set(TEXTURE_DIR "${GENERATION_ROOT}/generated/texture")
set(RESOURCE_BINS)

# ==================================================================================================
# Convert OBJ files into filamesh files.
# ==================================================================================================

function(add_mesh SOURCE TARGET)
    set(source_mesh "${ROOT_DIR}/${SOURCE}")
    set(target_mesh "${RESOURCE_DIR}/${TARGET}")
    set(RESOURCE_BINS ${RESOURCE_BINS} ${target_mesh} PARENT_SCOPE)
    add_custom_command(
        OUTPUT ${target_mesh}
        COMMAND filamesh --compress ${source_mesh} ${target_mesh}
        MAIN_DEPENDENCY ${source_mesh}
        DEPENDS filamesh)
endfunction()

add_mesh("assets/models/monkey/monkey.obj" "suzanne.filamesh")

# ==================================================================================================
# Build materials
# ==================================================================================================

set(MATERIAL_SRCS
        materials/bakedColor.mat
        materials/bakedTexture.mat
        materials/pointSprites.mat
        materials/aoPreview.mat
        materials/aiDefaultMat.mat
        materials/aiDefaultTrans.mat
        materials/depthVisualizer.mat
        materials/groundShadow.mat
        materials/sandboxCloth.mat
        materials/sandboxLit.mat
        materials/sandboxLitFade.mat
        materials/sandboxLitTransparent.mat
        materials/sandboxSpecGloss.mat
        materials/sandboxSubsurface.mat
        materials/sandboxUnlit.mat
        materials/texturedLit.mat
        materials/transparentColor.mat)

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

file(MAKE_DIRECTORY ${MATERIAL_DIR})

foreach (mat_src ${MATERIAL_SRCS})
    get_filename_component(localname "${mat_src}" NAME_WE)
    get_filename_component(fullname "${mat_src}" ABSOLUTE)
    set(output_path "${MATERIAL_DIR}/${localname}.filamat")
    add_custom_command(
            OUTPUT ${output_path}
            COMMAND matc ${MATC_BASE_FLAGS} -o ${output_path} ${fullname}
            MAIN_DEPENDENCY ${mat_src}
            DEPENDS matc
            COMMENT "Compiling material ${mat_src} to ${output_path}"
    )
    list(APPEND RESOURCE_BINS ${output_path})
endforeach()

# ==================================================================================================
# Build common resources
# ==================================================================================================

file(MAKE_DIRECTORY ${RESOURCE_DIR})

get_resgen_vars(${RESOURCE_DIR} resources)

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

add_library(common-resources ${DUMMY_SRC} ${RESGEN_SOURCE})

# ==================================================================================================
# Invoke cmgen to build KTX files for the default IBL and skybox
# ==================================================================================================

set(CMGEN_ARGS --quiet -x . --format=ktx --size=256 --extract-blur=0.1)

function(add_envmap SOURCE TARGET)
    set(source_envmap "${ROOT_DIR}/${SOURCE}")

    set(target_skybox   "${PROJECT_BINARY_DIR}/${TARGET}/${TARGET}_skybox.ktx")
    set(target_envmap   "${PROJECT_BINARY_DIR}/${TARGET}/${TARGET}_ibl.ktx")

    set(target_envmaps ${target_envmaps} ${target_skybox} PARENT_SCOPE)
    set(target_envmaps ${target_envmaps} ${target_envmap} PARENT_SCOPE)

    add_custom_command(OUTPUT ${target_skybox} ${target_envmap}
        COMMAND cmgen ${CMGEN_ARGS} ${source_envmap}
        MAIN_DEPENDENCY ${source_envmap}
        DEPENDS cmgen)
endfunction()

add_envmap("third_party/environments/venetian_crossroads_2k.hdr" "venetian_crossroads_2k")

# ==================================================================================================
# Build suzanne resources
# ==================================================================================================

function(add_ktxfiles SOURCE TARGET EXTRA_ARGS)
    set(source_path "${ROOT_DIR}/${SOURCE}")
    set(target_path "${TEXTURE_DIR}/${TARGET}")
    set(TEXTURE_FILES ${TEXTURE_FILES} ${target_path} PARENT_SCOPE)
    add_custom_command(
        OUTPUT ${target_path}
        COMMAND mipgen --quiet --strip-alpha ${EXTRA_ARGS} ${source_path} ${target_path}
        MAIN_DEPENDENCY ${source_path}
        DEPENDS mipgen)
endfunction()

function(add_pngfile SOURCE TARGET)
    set(source_path "${ROOT_DIR}/${SOURCE}")
    set(target_path "${TEXTURE_DIR}/${TARGET}")
    set(TEXTURE_FILES ${TEXTURE_FILES} ${target_path} PARENT_SCOPE)
    add_custom_command(
        OUTPUT ${target_path}
        COMMAND ${CMAKE_COMMAND} -E copy ${source_path} ${target_path}
        MAIN_DEPENDENCY ${source_path})
endfunction()

add_ktxfiles("assets/models/monkey/albedo.png" "albedo_s3tc.ktx" "--compression=s3tc_rgb_dxt1")
add_ktxfiles("assets/models/monkey/roughness.png" "roughness.ktx" "--grayscale")
add_ktxfiles("assets/models/monkey/metallic.png" "metallic.ktx" "--grayscale")
add_ktxfiles("assets/models/monkey/ao.png" "ao.ktx" "--grayscale")

add_pngfile("assets/models/monkey/normal.png" "normal.png")

get_resgen_vars(${RESOURCE_DIR} textures)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${TEXTURE_FILES}
        DEPENDS resgen ${TEXTURE_FILES}
        COMMENT "Aggregating textures"
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

add_library(suzanne-resources ${DUMMY_SRC} ${RESGEN_SOURCE})

# ==================================================================================================
# Resources for the gltf_viewer and lucy demos
# ==================================================================================================

get_resgen_vars(${RESOURCE_DIR} gltf)

set(GLTF_RESOURCE_FILES
    ${ROOT_DIR}/third_party/models/DamagedHelmet/DamagedHelmet.glb
    ${ROOT_DIR}/third_party/models/lucy/lucy.glb
    ${ROOT_DIR}/third_party/textures/Blue_tiles_01/Blue_tiles_01_AO.png
    ${ROOT_DIR}/third_party/textures/Blue_tiles_01/Blue_tiles_01_Color.png
    ${ROOT_DIR}/third_party/textures/Blue_tiles_01/Blue_tiles_01_Normal.png
    ${ROOT_DIR}/third_party/textures/Blue_tiles_01/Blue_tiles_01_Roughness.png
)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${GLTF_RESOURCE_FILES}
        DEPENDS resgen ${TEXTURE_FILES}
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

add_library(gltf-resources ${DUMMY_SRC} ${RESGEN_SOURCE})

# ==================================================================================================
# Common library
# ==================================================================================================

set(APP_LIBS filament sdl2 stb math filamat utils getopt imgui filagui image common-resources)
if (WIN32)
    list(APPEND APP_LIBS sdl2main)
endif()

set(APP_SRCS
        app/CameraManipulator.cpp
        app/Cube.cpp
        app/FilamentApp.cpp
        app/IBL.cpp
        app/IcoSphere.cpp
        app/Sphere.cpp
        app/Image.cpp
        )

if (APPLE)
    list(APPEND APP_SRCS app/NativeWindowHelperCocoa.mm)
    list(APPEND APP_LIBS "-framework Cocoa -framework QuartzCore")
endif()

if (LINUX)
    list(APPEND APP_SRCS app/NativeWindowHelperLinux.cpp)
endif()

if (WIN32)
    list(APPEND APP_SRCS app/NativeWindowHelperWindows.cpp)
endif()

include_directories(app)

add_library(sample-app ${APP_SRCS})
target_include_directories(sample-app PUBLIC app)

# Multi-configuration generators, like Visual Studio or Xcode, place executable binaries in a
# sub-directory named after the configuration, like "Debug" or "Release".
# For these generators, in order to find assets, we must "walk" up an additional directory.
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (_isMultiConfig)
    target_compile_definitions(sample-app PRIVATE RELATIVE_ASSET_PATH="..")
else()
    target_compile_definitions(sample-app PRIVATE RELATIVE_ASSET_PATH=".")
endif()

target_link_libraries(sample-app PRIVATE ${APP_LIBS})

list(APPEND APP_LIBS sample-app)

# ==================================================================================================
# Compiler flags
# ==================================================================================================

if (NOT MSVC_NATIVE)
    set(COMPILER_FLAGS
            -Wno-extern-c-compat
            $<$<NOT:$<PLATFORM_ID:Linux>>:-Wno-address-of-packed-member>)
endif()

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
    target_link_libraries(${NAME} PRIVATE ${APP_LIBS} assimp)
    target_compile_options(${NAME} PRIVATE ${COMPILER_FLAGS})
endfunction()

function(add_filamesh_demo NAME)
    include_directories(${GENERATION_ROOT})
    add_executable(
            ${NAME}
            ${NAME}.cpp)
    target_link_libraries(${NAME} PRIVATE ${APP_LIBS} filameshio)
    target_compile_options(${NAME} PRIVATE ${COMPILER_FLAGS})
endfunction()

if (NOT ANDROID)
    add_assimp_demo(frame_generator)
    add_assimp_demo(lightbulb)
    add_assimp_demo(material_sandbox)
    add_assimp_demo(sample_full_pbr)

    add_filamesh_demo(point_sprites)
    add_filamesh_demo(sample_cloth)
    add_filamesh_demo(sample_normal_map)
    add_filamesh_demo(suzanne)
    add_filamesh_demo(gltf_viewer)
    add_filamesh_demo(gltf_baker)

    # Sample app specific
    target_link_libraries(frame_generator PRIVATE imageio)
    target_link_libraries(suzanne PRIVATE suzanne-resources)
    target_link_libraries(gltf_viewer PRIVATE gltf-resources gltfio)
    target_link_libraries(gltf_baker PRIVATE gltf-resources gltfio_pipeline imageio)

    add_executable(lucy_bloom lucy_bloom.cpp lucy_utils.h lucy_utils.cpp)
    target_compile_options(lucy_bloom PRIVATE ${COMPILER_FLAGS})
    target_link_libraries(lucy_bloom PRIVATE ${APP_LIBS} gltf-resources gltfio)
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
    add_assimp_demo(vk_shadowtest)
    add_filamesh_demo(vk_animation)
    add_filamesh_demo(vk_depthtesting)
    add_filamesh_demo(vk_hellopbr)
    add_filamesh_demo(vk_hellotriangle)
    add_filamesh_demo(vk_imgui)
    add_filamesh_demo(vk_strobecolor)
    add_filamesh_demo(vk_texturedquad)
    add_filamesh_demo(vk_vbotest)
    add_filamesh_demo(vk_viewtest)
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

add_custom_target(envs DEPENDS ${target_envmaps})
add_dependencies(filament envs)
