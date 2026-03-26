cmake_minimum_required(VERSION 3.19)
project(webexamples)

if (FILAMENT_SKIP_SAMPLES)
    return()
endif()

set(SERVER_DIR ${PROJECT_BINARY_DIR}/examples)
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

function(build_material MAT_FILE TARGET_DIR MAT_NAME OUT_LIST)
    set(MATC_FLAGS -a opengl -p mobile)
    if (NOT CMAKE_BUILD_TYPE MATCHES Release)
        set(MATC_FLAGS -g ${MATC_FLAGS})
    endif()
    set(mat_src "${CMAKE_CURRENT_SOURCE_DIR}/${MAT_FILE}")
    set(output_path "${SERVER_DIR}/${TARGET_DIR}/${MAT_NAME}.filamat")
    add_custom_command(
            OUTPUT ${output_path}
            COMMAND matc ${MATC_FLAGS} -o ${output_path} ${mat_src}
            MAIN_DEPENDENCY ${mat_src}
            DEPENDS matc
            COMMENT "Compiling material ${mat_src} to ${output_path}")
    set(${OUT_LIST} ${${OUT_LIST}} ${output_path} PARENT_SCOPE)
endfunction()

function(build_material_fl0 MAT_FILE TARGET_DIR MAT_NAME OUT_LIST)
    if (FILAMENT_ENABLE_FEATURE_LEVEL_0)
        set(MATC_FLAGS -a opengl -p mobile)
        if (NOT CMAKE_BUILD_TYPE MATCHES Release)
            set(MATC_FLAGS -g ${MATC_FLAGS})
        endif()
        set(mat_src "${CMAKE_CURRENT_SOURCE_DIR}/${MAT_FILE}")
        set(output_path "${SERVER_DIR}/${TARGET_DIR}/${MAT_NAME}_fl0.filamat")
        add_custom_command(
                OUTPUT ${output_path}
                COMMAND matc ${MATC_FLAGS} -PfeatureLevel=0 -o ${output_path} ${mat_src}
                MAIN_DEPENDENCY ${mat_src}
                DEPENDS matc
                COMMENT "Compiling material FL0 ${mat_src} to ${output_path}")
        set(${OUT_LIST} ${${OUT_LIST}} ${output_path} PARENT_SCOPE)
    endif()
endfunction()

function(add_ktxfiles SOURCE TARGET EXTRA_ARGS TARGET_DIR OUT_LIST)
    set(source_path "${ROOT_DIR}/${SOURCE}")
    set(target_path "${SERVER_DIR}/${TARGET_DIR}/${TARGET}")
    add_custom_command(
        OUTPUT ${target_path}
        COMMAND mipgen --quiet --strip-alpha ${EXTRA_ARGS} ${source_path} ${target_path}
        MAIN_DEPENDENCY ${source_path}
        DEPENDS mipgen)
    set(${OUT_LIST} ${${OUT_LIST}} ${target_path} PARENT_SCOPE)
endfunction()

function(add_rawfile SOURCE TARGET TARGET_DIR OUT_LIST)
    set(source_path "${ROOT_DIR}/${SOURCE}")
    set(target_path "${SERVER_DIR}/${TARGET_DIR}/${TARGET}")
    add_custom_command(
        OUTPUT ${target_path}
        COMMAND ${CMAKE_COMMAND} -E copy ${source_path} ${target_path}
        MAIN_DEPENDENCY ${source_path})
    set(${OUT_LIST} ${${OUT_LIST}} ${target_path} PARENT_SCOPE)
endfunction()

function(add_localfile SOURCE TARGET TARGET_DIR OUT_LIST)
    set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}")
    set(target_path "${SERVER_DIR}/${TARGET_DIR}/${TARGET}")
    add_custom_command(
        OUTPUT ${target_path}
        COMMAND ${CMAKE_COMMAND} -E copy ${source_path} ${target_path}
        MAIN_DEPENDENCY ${source_path})
    set(${OUT_LIST} ${${OUT_LIST}} ${target_path} PARENT_SCOPE)
endfunction()

function(add_mesh SOURCE TARGET TARGET_DIR OUT_LIST)
    set(source_mesh "${ROOT_DIR}/${SOURCE}")
    set(target_mesh "${SERVER_DIR}/${TARGET_DIR}/${TARGET}")
    add_custom_command(
        OUTPUT ${target_mesh}
        COMMAND filamesh --compress ${source_mesh} ${target_mesh}
        MAIN_DEPENDENCY ${source_mesh}
        DEPENDS filamesh)
    set(${OUT_LIST} ${${OUT_LIST}} ${target_mesh} PARENT_SCOPE)
endfunction()

set(CMGEN_ARGS --quiet --format=ktx --size=256 --extract-blur=0.1)
set(CMGEN_ARGS_TINY --quiet --format=ktx --size=64 --extract-blur=0.1)

function(add_envmap SOURCE TARGET TARGET_DIR OUT_LIST)
    set(source_envmap "${ROOT_DIR}/${SOURCE}")
    set(target_skybox "${SERVER_DIR}/${TARGET_DIR}/${TARGET}/${TARGET}_skybox.ktx")
    set(target_envmap "${SERVER_DIR}/${TARGET_DIR}/${TARGET}/${TARGET}_ibl.ktx")
    set(target_skybox_tiny "${SERVER_DIR}/${TARGET_DIR}/${TARGET}/${TARGET}_skybox_tiny.ktx")

    add_custom_command(OUTPUT ${target_skybox} ${target_skybox_tiny} ${target_envmap}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SERVER_DIR}/${TARGET_DIR}/${TARGET}"
        COMMAND cmgen -x ${TARGET} ${CMGEN_ARGS_TINY} ${source_envmap}
        COMMAND ${CMAKE_COMMAND} -E rename "${SERVER_DIR}/${TARGET_DIR}/${TARGET}/${TARGET}_skybox.ktx" ${target_skybox_tiny}
        COMMAND cmgen -x ${TARGET} ${CMGEN_ARGS} ${source_envmap}
        WORKING_DIRECTORY "${SERVER_DIR}/${TARGET_DIR}"
        MAIN_DEPENDENCY ${source_envmap}
        DEPENDS cmgen)
        
    set(${OUT_LIST} ${${OUT_LIST}} ${target_skybox} ${target_skybox_tiny} ${target_envmap} PARENT_SCOPE)
endfunction()

set(ALL_DEMO_ASSETS)

add_custom_command(OUTPUT ${SERVER_DIR}/filament.js DEPENDS filament-js COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/../filament-js/filament.js ${SERVER_DIR})
add_custom_command(OUTPUT ${SERVER_DIR}/filament.wasm DEPENDS filament-js COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/../filament-js/filament.wasm ${SERVER_DIR})
add_custom_command(OUTPUT ${SERVER_DIR}/gl-matrix-min.js COMMAND ${CMAKE_COMMAND} -E copy ${ROOT_DIR}/third_party/gl-matrix/gl-matrix-min.js ${SERVER_DIR})
add_custom_command(OUTPUT ${SERVER_DIR}/gltumble.min.js COMMAND ${CMAKE_COMMAND} -E copy ${ROOT_DIR}/third_party/gltumble/gltumble.min.js ${SERVER_DIR})
add_custom_command(OUTPUT ${SERVER_DIR}/main.css COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/main.css ${SERVER_DIR})
add_custom_command(OUTPUT ${SERVER_DIR}/favicon.png COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/assets/favicon.png ${SERVER_DIR})
list(APPEND ALL_DEMO_ASSETS ${SERVER_DIR}/filament.js ${SERVER_DIR}/filament.wasm ${SERVER_DIR}/gl-matrix-min.js ${SERVER_DIR}/gltumble.min.js ${SERVER_DIR}/main.css ${SERVER_DIR}/favicon.png)

set(TARGET_DIR "triangle")
add_localfile("triangle.md" "triangle.md" ${TARGET_DIR} ALL_DEMO_ASSETS)
build_material("materials/triangle.mat" ${TARGET_DIR} "triangle" ALL_DEMO_ASSETS)

set(TARGET_DIR "redball")
add_localfile("redball.md" "redball.md" ${TARGET_DIR} ALL_DEMO_ASSETS)
build_material("materials/plastic.mat" ${TARGET_DIR} "plastic" ALL_DEMO_ASSETS)
add_envmap("third_party/environments/pillars_2k.hdr" "pillars_2k" ${TARGET_DIR} ALL_DEMO_ASSETS)

set(TARGET_DIR "suzanne")
add_localfile("suzanne.md" "suzanne.md" ${TARGET_DIR} ALL_DEMO_ASSETS)
build_material("materials/textured.mat" ${TARGET_DIR} "textured" ALL_DEMO_ASSETS)
add_mesh("assets/models/monkey/monkey.obj" "suzanne.filamesh" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_ktxfiles("assets/models/monkey/color.png" "albedo.ktx2" "--compression=uastc" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_ktxfiles("assets/models/monkey/normal.png" "normal.ktx2" "--compression=uastc_normals;--kernel=NORMALS;--linear" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_ktxfiles("assets/models/monkey/roughness.png" "roughness.ktx2" "--compression=uastc;--grayscale;--linear" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_ktxfiles("assets/models/monkey/metallic.png" "metallic.ktx2" "--compression=uastc;--grayscale;--linear" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_ktxfiles("assets/models/monkey/ao.png" "ao.ktx2" "--compression=uastc;--grayscale;--linear" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_envmap("third_party/environments/venetian_crossroads_2k.hdr" "venetian_crossroads_2k" ${TARGET_DIR} ALL_DEMO_ASSETS)

set(TARGET_DIR "animation")
add_localfile("animation.html" "animation.html" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/AnimatedTriangle/animation.bin" "animation.bin" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/AnimatedTriangle/simpleTriangle.bin" "simpleTriangle.bin" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/AnimatedTriangle/AnimatedTriangle.gltf" "AnimatedTriangle.gltf" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_envmap("third_party/environments/lightroom_14b.hdr" "default_env" ${TARGET_DIR} ALL_DEMO_ASSETS)

set(TARGET_DIR "cube_fl0")
add_localfile("cube_fl0.html" "cube_fl0.html" ${TARGET_DIR} ALL_DEMO_ASSETS)
build_material_fl0("materials/nonlit.mat" ${TARGET_DIR} "nonlit" ALL_DEMO_ASSETS)

set(TARGET_DIR "helmet")
add_localfile("helmet.html" "helmet.html" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_baseColor.png" "FlightHelmet_baseColor.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_baseColor1.png" "FlightHelmet_baseColor1.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_baseColor2.png" "FlightHelmet_baseColor2.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_baseColor3.png" "FlightHelmet_baseColor3.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_baseColor4.png" "FlightHelmet_baseColor4.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_normal.png" "FlightHelmet_normal.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_normal1.png" "FlightHelmet_normal1.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_normal2.png" "FlightHelmet_normal2.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_normal3.png" "FlightHelmet_normal3.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_normal4.png" "FlightHelmet_normal4.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_occlusionRoughnessMetallic.png" "FlightHelmet_occlusionRoughnessMetallic.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_occlusionRoughnessMetallic1.png" "FlightHelmet_occlusionRoughnessMetallic1.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_occlusionRoughnessMetallic2.png" "FlightHelmet_occlusionRoughnessMetallic2.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_occlusionRoughnessMetallic3.png" "FlightHelmet_occlusionRoughnessMetallic3.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet_occlusionRoughnessMetallic4.png" "FlightHelmet_occlusionRoughnessMetallic4.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet.bin" "FlightHelmet.bin" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/FlightHelmet/FlightHelmet.gltf" "FlightHelmet.gltf" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_envmap("third_party/environments/lightroom_14b.hdr" "default_env" ${TARGET_DIR} ALL_DEMO_ASSETS)

set(TARGET_DIR "morphing")
add_localfile("morphing.html" "morphing.html" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_rawfile("third_party/models/AnimatedMorphCube/AnimatedMorphCube.glb" "AnimatedMorphCube.glb" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_envmap("third_party/environments/lightroom_14b.hdr" "default_env" ${TARGET_DIR} ALL_DEMO_ASSETS)

set(TARGET_DIR "parquet")
add_localfile("parquet.html" "parquet.html" ${TARGET_DIR} ALL_DEMO_ASSETS)
build_material("materials/parquet.mat" ${TARGET_DIR} "parquet" ALL_DEMO_ASSETS)
add_mesh("third_party/models/shader_ball/shader_ball.obj" "shader_ball.filamesh" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_localfile("textures/floor_ao_roughness_metallic.png" "floor_ao_roughness_metallic.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_localfile("textures/floor_basecolor.jpg" "floor_basecolor.jpg" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_localfile("textures/floor_normal.png" "floor_normal.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_envmap("third_party/environments/lightroom_14b.hdr" "default_env" ${TARGET_DIR} ALL_DEMO_ASSETS)

set(TARGET_DIR "skinning")
add_localfile("skinning.html" "skinning.html" ${TARGET_DIR} ALL_DEMO_ASSETS)
build_material("materials/skinning.mat" ${TARGET_DIR} "skinning" ALL_DEMO_ASSETS)
add_rawfile("third_party/models/AnimatedTriangle/simpleTriangle.bin" "simpleTriangle.bin" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_envmap("third_party/environments/lightroom_14b.hdr" "default_env" ${TARGET_DIR} ALL_DEMO_ASSETS)

set(TARGET_DIR "remote")
add_localfile("remote.html" "remote.html" ${TARGET_DIR} ALL_DEMO_ASSETS)

set(TARGET_DIR "sky")
add_localfile("sky.html" "sky.html" ${TARGET_DIR} ALL_DEMO_ASSETS)
build_material("materials/simulated_skybox.mat" ${TARGET_DIR} "simulated_skybox" ALL_DEMO_ASSETS)
add_localfile("sky/lil-gui.js" "lil-gui.js" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_localfile("sky/suncalc_global.js" "suncalc_global.js" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_localfile("sky/main.js" "main.js" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_localfile("sky/SimulatedSkybox.js" "SimulatedSkybox.js" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_localfile("textures/sky/moon_disk.png" "moon_disk.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_localfile("textures/sky/moon_normal.png" "moon_normal.png" ${TARGET_DIR} ALL_DEMO_ASSETS)
add_localfile("textures/sky/milkyway.png" "milkyway.png" ${TARGET_DIR} ALL_DEMO_ASSETS)

add_custom_target(${PROJECT_NAME} ALL DEPENDS ${ALL_DEMO_ASSETS})
