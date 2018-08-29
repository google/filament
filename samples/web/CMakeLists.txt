cmake_minimum_required(VERSION 3.1)
project(web-samples)

# ==================================================================================================
# Build materials into inc files.
# ==================================================================================================

set(MATERIAL_SRCS
        ../materials/texturedLit.mat
        ../materials/bakedColor.mat)

if (CMAKE_CROSSCOMPILING)
    set(IMPORT_EXECUTABLES ${FILAMENT}/${IMPORT_EXECUTABLES_DIR})
    set(IMPORT_EXECUTABLES ${IMPORT_EXECUTABLES}/ImportExecutables-${CMAKE_BUILD_TYPE}.cmake)
    include(${IMPORT_EXECUTABLES})
endif()

set(MATC_TARGET mobile)
set(MATC_FLAGS -a all)
if (CMAKE_BUILD_TYPE MATCHES Release)
    set(MATC_FLAGS -O ${MATC_FLAGS})
endif()

set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
file(MAKE_DIRECTORY "${GENERATION_ROOT}/generated/material/")

set(MATERIAL_BINS)
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
            COMMENT "Compiling material ${mat_src} to ${output_path}")
    list(APPEND MATERIAL_BINS ${output_path})
endforeach()

add_custom_target(sample_materials DEPENDS ${MATERIAL_BINS})

# ==================================================================================================
# Build Assets.
#
# The goal is to create a "public" folder that can be the root of a simple file server:
#
#    /suzanne.js                   built by em++
#    /suzanne.wasm                 built by em++
#    /suzanne.html                 copied from REPO/samples/web
#    /filaweb.js                   copied from REPO/samples/web
#    /favicon.png                  copied from REPO/samples/web
#    /monkey/*.png                 copied from REPO/assets/models
#    /monkey/*.filamesh            built by filamesh
#    /syferfontein_18d_clear_2k/*  built by cmgen
#
# ==================================================================================================

function(copy_assets PATH NAME)
    file(COPY ${PATH}/${NAME} DESTINATION ${PROJECT_BINARY_DIR}/public)
    add_custom_target(${NAME} ALL DEPENDS ${NAME})
    add_dependencies(filament ${NAME})
endfunction()

copy_assets("." filaweb.js)
copy_assets("." favicon.png)
copy_assets("../../assets/models" monkey)

# Convert OBJ into a simple binary format.
set(source_mesh "${CMAKE_CURRENT_SOURCE_DIR}/../../assets/models/monkey/monkey.obj")
set(target_mesh "public/monkey/mesh.filamesh")
add_custom_command(
    OUTPUT ${target_mesh}
    COMMAND filamesh ${source_mesh} public/monkey/mesh.filamesh
    MAIN_DEPENDENCY ${source_mesh}
    DEPENDS filamesh)

# Next build two cubemaps:
#  (1) single miplevel of blurry skybox (png)
#  (2) all miplevels for the IBL (rgbm)

set(source_envmap "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/environments/syferfontein_18d_clear_2k.hdr")

foreach(FACE nx ny nz px py pz)
    set(target_skybox ${target_skybox} public/syferfontein_18d_clear_2k/${FACE}.png)
    foreach(MIP 0 1 2 3 4 5 6 7 8)
        set(target_envmap ${target_envmap} public/syferfontein_18d_clear_2k/m${MIP}_${FACE}.rgbm)
    endforeach()
endforeach()

add_custom_command(
    OUTPUT ${target_envmap}
    COMMAND cmgen -x public --format=rgbm --size=256 ${source_envmap} 
    MAIN_DEPENDENCY ${source_envmap}
    DEPENDS cmgen)

add_custom_command(
    OUTPUT ${target_skybox}
    COMMAND cmgen -x public --format=png --size=512 --extract-blur=0.1 ${source_envmap} 
    MAIN_DEPENDENCY ${source_envmap}
    DEPENDS cmgen)

add_custom_target(sample_assets DEPENDS ${target_mesh} ${target_envmap} ${target_skybox})

# ==================================================================================================
# For each demo, build a pair of wasm/js files and copy over an HTML file.
# ==================================================================================================

set(EXPORTS "'_launch','_main','_render','_resize'")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXPORTED_FUNCTIONS=[${EXPORTS}]")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ASSERTIONS=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s SAFE_HEAP=1 ")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s DEMANGLE_SUPPORT=1")
endif()

function(add_demo NAME)
    include_directories(${GENERATION_ROOT})
    add_executable(${NAME} ${NAME}.cpp
            filamesh.h
            filamesh.cpp
            filaweb.h
            filaweb.cpp)
    add_dependencies(${NAME} sample_materials sample_assets)
    target_link_libraries(${NAME} PRIVATE filament math filamat utils)

    # Copy the generated js and wasm files into the public folder, as well as the app-specific
    # manually-written HTML file.
    add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/public/${NAME}.js
        DEPENDS ${NAME} ${PROJECT_BINARY_DIR}/${NAME}.js
        COMMAND cmake -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.html ${PROJECT_BINARY_DIR}/public
        COMMAND cmake -E copy ${PROJECT_BINARY_DIR}/${NAME}.* ${PROJECT_BINARY_DIR}/public)
    add_custom_target(${NAME}_public ALL DEPENDS ${PROJECT_BINARY_DIR}/public/${NAME}.js)

endfunction()

add_demo(suzanne)
add_demo(triangle)
