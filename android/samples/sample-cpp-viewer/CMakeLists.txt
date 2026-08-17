cmake_minimum_required(VERSION 3.19)
project(sample-cpp-viewer)

set(CMAKE_CXX_STANDARD 20)
# set(FILAMENT_DIR ${FILAMENT_DIST_DIR})

set(FILAMENT_DIR ${FILAMENT_DIST_DIR})

add_library(filament-samples STATIC IMPORTED)
set_target_properties(filament-samples PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilament-samples.a)

add_library(sample-resources STATIC IMPORTED)
set_target_properties(sample-resources PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libsample-resources.a)

add_library(suzanne-resources STATIC IMPORTED)
set_target_properties(suzanne-resources PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libsuzanne-resources.a)

add_library(samples-common STATIC IMPORTED)
set_target_properties(samples-common PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libsamples-common.a)

add_library(filamentapp STATIC IMPORTED)
set_target_properties(filamentapp PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilamentapp.a)

add_library(filamentapp-resources STATIC IMPORTED)
set_target_properties(filamentapp-resources PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilamentapp-resources.a)

add_library(filameshio STATIC IMPORTED)
set_target_properties(filameshio PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilameshio.a)

add_library(imageio-lite STATIC IMPORTED)
set_target_properties(imageio-lite PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libimageio-lite.a)

add_library(image STATIC IMPORTED)
set_target_properties(image PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libimage.a)

add_library(ktxreader STATIC IMPORTED)
set_target_properties(ktxreader PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libktxreader.a)

add_library(camutils STATIC IMPORTED)
set_target_properties(camutils PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libcamutils.a)

add_library(geometry STATIC IMPORTED)
set_target_properties(geometry PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libgeometry.a)

add_library(mikktspace STATIC IMPORTED)
set_target_properties(mikktspace PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libmikktspace.a)

add_library(filamat STATIC IMPORTED)
set_target_properties(filamat PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilamat.a)

add_library(shaders STATIC IMPORTED)
set_target_properties(shaders PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libshaders.a)

add_library(filament STATIC IMPORTED)
set_target_properties(filament PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilament.a)

add_library(backend STATIC IMPORTED)
set_target_properties(backend PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libbackend.a)

add_library(bluevk STATIC IMPORTED)
set_target_properties(bluevk PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libbluevk.a)

add_library(filaflat STATIC IMPORTED)
set_target_properties(filaflat PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilaflat.a)

add_library(smol-v STATIC IMPORTED)
set_target_properties(smol-v PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libsmol-v.a)

add_library(filabridge STATIC IMPORTED)
set_target_properties(filabridge PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilabridge.a)

add_library(utils STATIC IMPORTED)
set_target_properties(utils PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libutils.a)

add_library(perfetto STATIC IMPORTED)
set_target_properties(perfetto PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libperfetto.a)

add_library(stb STATIC IMPORTED)
set_target_properties(stb PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libstb.a)

add_library(filament-iblprefilter STATIC IMPORTED)
set_target_properties(filament-iblprefilter PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilament-iblprefilter.a)

add_library(basis_transcoder STATIC IMPORTED)
set_target_properties(basis_transcoder PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libbasis_transcoder.a)

add_library(assimp STATIC IMPORTED)
set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libassimp.a)

add_library(sample-cpp-viewer-jni SHARED 
        src/main/cpp/SampleAppJni.cpp
        src/main/cpp/AndroidDisplayManager.cpp
        src/main/cpp/NativeViewer.cpp)

target_include_directories(sample-cpp-viewer-jni PRIVATE ../../../samples ../../../samples/include ${FILAMENT_DIR}/include ../../../libs/filamentapp/include ../../../libs/utils/include ../../../libs/math/include ../../../filament/backend/include)

target_link_libraries(sample-cpp-viewer-jni PRIVATE
        filament-samples
        sample-resources
        suzanne-resources
        samples-common
        filamentapp
        filamentapp-resources
        filameshio
        imageio-lite
        image
        ktxreader
        camutils
        geometry
        mikktspace
        filamat
        shaders
        filament
        backend
        bluevk
        filaflat
        smol-v
        filabridge
        utils
        perfetto
        stb
        basis_transcoder
        assimp
        filament-iblprefilter
        EGL GLESv3 android log z)
