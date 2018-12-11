cmake_minimum_required(VERSION 3.1)
project(websamples)

set(SERVER_DIR ${PROJECT_BINARY_DIR})
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

# ==================================================================================================
# Build Materials.
# ==================================================================================================

set(MATERIAL_NAMES
    parquet
    sandbox
    textured
    nonlit)

set(MATC_FLAGS -a opengl -m material -p mobile)
if (NOT CMAKE_BUILD_TYPE MATCHES Release)
    set(MATC_FLAGS -g ${MATC_FLAGS})
endif()

set(MATERIAL_BINS)
foreach (NAME ${MATERIAL_NAMES})
    set(mat_src "materials/${NAME}.mat")
    get_filename_component(localname "${mat_src}" NAME_WE)
    get_filename_component(fullname "${mat_src}" ABSOLUTE)
    set(output_bin "${SERVER_DIR}/${localname}.filamat")
    add_custom_command(
            OUTPUT ${output_bin}
            COMMAND matc ${MATC_FLAGS} -o ${output_bin} ${fullname}
            MAIN_DEPENDENCY ${mat_src}
            DEPENDS matc
            COMMENT "Compiling material ${mat_src} to ${output_path}")
    list(APPEND MATERIAL_BINS ${output_bin})
endforeach()

add_custom_target(sample_materials DEPENDS ${MATERIAL_BINS})

# ==================================================================================================
# Build Assets.
# ==================================================================================================

# Generate mipmapped KTX files from various PNG files using mipgen.
function(add_ktxfiles SOURCE TARGET EXTRA_ARGS)
    set(source_path "${ROOT_DIR}/${SOURCE}")
    set(target_path "${SERVER_DIR}/${TARGET}")
    set(target_ktxfiles ${target_ktxfiles} ${target_path} PARENT_SCOPE)
    add_custom_command(
        OUTPUT ${target_path}
        COMMAND mipgen --strip-alpha ${EXTRA_ARGS} ${source_path} ${target_path}
        MAIN_DEPENDENCY ${source_path}
        DEPENDS mipgen)
endfunction()

set(ETC_R11_ARGS "--grayscale;--compression=etc_r11_numeric_40")

# TODO: Instead of "rgb8" we should be using "rg11", but that causes an assertion in etc2comp.
set(ETC_NORMALS_ARGS "--kernel=NORMALS;--linear;--compression=etc_rgb8_normalxyz_40")

add_ktxfiles("assets/models/monkey/albedo.png" "albedo.ktx" "")
add_ktxfiles("assets/models/monkey/albedo.png" "albedo_astc.ktx" "--compression=astc_fast_ldr_4x4")
add_ktxfiles("assets/models/monkey/albedo.png" "albedo_s3tc.ktx" "--compression=s3tc_rgb_dxt1")
add_ktxfiles("assets/models/monkey/normal.png" "normal.ktx" "--kernel=NORMALS;--linear")
add_ktxfiles("assets/models/monkey/normal.png" "normal_etc.ktx" "${ETC_NORMALS_ARGS}")
add_ktxfiles("assets/models/monkey/roughness.png" "roughness.ktx" "--grayscale")
add_ktxfiles("assets/models/monkey/roughness.png" "roughness_etc.ktx" "${ETC_R11_ARGS}")
add_ktxfiles("assets/models/monkey/metallic.png" "metallic.ktx" "--grayscale")
add_ktxfiles("assets/models/monkey/metallic.png" "metallic_etc.ktx" "${ETC_R11_ARGS}")
add_ktxfiles("assets/models/monkey/ao.png" "ao.ktx" "--grayscale")
add_ktxfiles("assets/models/monkey/ao.png" "ao_etc.ktx" "${ETC_R11_ARGS}")

# Convert OBJ files into filamesh files.
function(add_mesh SOURCE TARGET)
    set(source_mesh "${ROOT_DIR}/${SOURCE}")
    set(target_mesh "${SERVER_DIR}/${TARGET}")
    set(target_meshes ${target_meshes} ${target_mesh} PARENT_SCOPE)
    add_custom_command(
        OUTPUT ${target_mesh}
        COMMAND filamesh --compress ${source_mesh} ${target_mesh}
        MAIN_DEPENDENCY ${source_mesh}
        DEPENDS filamesh)
endfunction()

add_mesh("assets/models/monkey/monkey.obj" "suzanne.filamesh")
add_mesh("third_party/shader_ball/shader_ball.obj" "shader_ball.filamesh")

# Generate IBL and skybox images using cmgen.
set(CMGEN_ARGS -x . --format=ktx --size=256 --extract-blur=0.1)
set(CMGEN_ARGS_TINY -x . --format=ktx --size=64 --extract-blur=0.1)
set(CMGEN_ARGS_S3TC ${CMGEN_ARGS} --compression=s3tc_rgba_dxt5)
set(CMGEN_ARGS_ETC ${CMGEN_ARGS} --compression=etc_rgba8_rgba_40)
function(add_envmap SOURCE TARGET)
    set(source_envmap "${ROOT_DIR}/${SOURCE}")

    file(MAKE_DIRECTORY "${SERVER_DIR}/${TARGET}")

    set(target_skybox "${SERVER_DIR}/${TARGET}/${TARGET}_skybox.ktx")
    set(target_envmap "${SERVER_DIR}/${TARGET}/${TARGET}_ibl.ktx")

    set(target_envmap_etc "${SERVER_DIR}/${TARGET}/${TARGET}_ibl_etc.ktx")
    set(target_envmap_s3tc "${SERVER_DIR}/${TARGET}/${TARGET}_ibl_s3tc.ktx")

    set(target_skybox_tiny "${SERVER_DIR}/${TARGET}/${TARGET}_skybox_tiny.ktx")

    set(target_skyboxes ${target_skyboxes} ${target_skybox} ${target_skybox_tiny} PARENT_SCOPE)

    set(target_envmaps ${target_envmaps}
            ${target_envmap} ${target_envmap_etc} ${target_envmap_s3tc} PARENT_SCOPE)

    add_custom_command(OUTPUT ${target_skybox} ${target_skybox_tiny}
                ${target_envmap} ${target_envmap_etc} ${target_envmap_s3tc}

        # First create an S3TC-encoded envmap, then rename it.
        COMMAND cmgen ${CMGEN_ARGS_S3TC} ${source_envmap}
        COMMAND mv ${target_envmap} ${target_envmap_s3tc}

        # Now create an ETC-encoded envmap, then rename it.
        COMMAND cmgen ${CMGEN_ARGS_ETC} ${source_envmap}
        COMMAND mv ${target_envmap} ${target_envmap_etc}

        # Create a low-resolution skybox, then rename it.
        COMMAND cmgen ${CMGEN_ARGS_TINY} ${source_envmap}
        COMMAND mv ${target_skybox} ${target_skybox_tiny}

        # Finally, create KTX files for the uncompressed envmap.
        COMMAND cmgen ${CMGEN_ARGS} ${source_envmap}

        MAIN_DEPENDENCY ${source_envmap}
        DEPENDS cmgen)
endfunction()

add_envmap("third_party/environments/syferfontein_18d_clear_2k.hdr" "syferfontein_18d_clear_2k")
add_envmap("third_party/environments/venetian_crossroads_2k.hdr" "venetian_crossroads_2k")
add_envmap("third_party/environments/pillars_2k.hdr" "pillars_2k")

add_custom_target(sample_assets DEPENDS
    ${target_ktxfiles}
    ${target_meshes}
    ${target_envmaps}
    ${target_skyboxes})

# ==================================================================================================
# Copy filament.{js,wasm} into the server folder.
# ==================================================================================================

add_custom_command(
    OUTPUT ${SERVER_DIR}/filament.js
    DEPENDS filament-js
    COMMAND cmake -E copy ${PROJECT_BINARY_DIR}/../filament-js/filament.js ${SERVER_DIR})

add_custom_command(
    OUTPUT ${SERVER_DIR}/filament.wasm
    DEPENDS filament-js
    COMMAND cmake -E copy ${PROJECT_BINARY_DIR}/../filament-js/filament.wasm ${SERVER_DIR})

add_custom_target(filamentjs_public DEPENDS ${SERVER_DIR}/filament.js ${SERVER_DIR}/filament.wasm)

# ==================================================================================================
# The websamples target depends on all HTML files, assets, and filament.{js,wasm}
# ==================================================================================================

set(HTML_FILES
    leaks.html
    parquet.html
    suzanne.html
    triangle.html)

set(DEMO_ASSETS)
foreach (NAME ${HTML_FILES})
    add_custom_command(
        OUTPUT ${SERVER_DIR}/${NAME}
        COMMAND cmake -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${NAME} ${SERVER_DIR}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${NAME})
    list(APPEND DEMO_ASSETS ${SERVER_DIR}/${NAME})
endforeach()

set(TEXTURE_FILES floor_ao_roughness_metallic.png floor_basecolor.png floor_normal.png)
foreach (NAME ${TEXTURE_FILES})
    add_custom_command(
        OUTPUT ${SERVER_DIR}/${NAME}
        COMMAND cmake -E copy ${CMAKE_CURRENT_SOURCE_DIR}/textures/${NAME} ${SERVER_DIR}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/textures/${NAME})
    list(APPEND DEMO_ASSETS ${SERVER_DIR}/${NAME})
endforeach()

add_custom_command(
    OUTPUT ${SERVER_DIR}/gl-matrix-min.js
    COMMAND cmake -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/gl-matrix/gl-matrix-min.js ${SERVER_DIR}/gl-matrix-min.js
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/gl-matrix/gl-matrix-min.js)
list(APPEND DEMO_ASSETS ${SERVER_DIR}/gl-matrix-min.js)

add_custom_target(${PROJECT_NAME} ALL DEPENDS
    ${DEMO_ASSETS}
    sample_materials
    sample_assets
    filamentjs_public)
