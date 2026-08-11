cmake_minimum_required(VERSION 3.22)
# C is required: native_app_glue is a .c file and would otherwise be silently ignored.
project(helloxr-android C CXX)

# Consumes the Filament install produced by the arm64-v8a cross-build, exactly as an external
# client would. Produce it with:
#   cmake -S . -B out/cmake-android-release-aarch64 -G Ninja -DCMAKE_BUILD_TYPE=Release \
#         -DCMAKE_TOOLCHAIN_FILE=build/toolchain-aarch64-linux-android.cmake \
#         -DCMAKE_INSTALL_PREFIX=out/android-release/filament -DFILAMENT_SUPPORTS_VULKAN=ON \
#         -DFILAMENT_SAMPLES_STEREO_TYPE=multiview -DFILAMENT_SKIP_SAMPLES=ON \
#         -DFILAMENT_IMPORT_PREBUILT_EXECUTABLES_DIR=out
#   cmake --build out/cmake-android-release-aarch64 --target install
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../.." ABSOLUTE)
set(FILAMENT_DIST_DIR "${REPO_ROOT}/out/android-release/filament"
    CACHE PATH "Filament Android install directory")

if (NOT EXISTS "${FILAMENT_DIST_DIR}/include")
    message(FATAL_ERROR
        "Filament Android install not found at ${FILAMENT_DIST_DIR}. "
        "Build and install the arm64-v8a cross-build first; see the comment at the top of this "
        "file for the exact commands.")
endif()

find_package(OpenXR REQUIRED CONFIG)

# Filament's archives are mutually recursive, so let the linker resolve them as a group rather than
# trying to get the order right by hand.
file(GLOB FILAMENT_ARCHIVES "${FILAMENT_DIST_DIR}/lib/${ANDROID_ABI}/*.a")
if (NOT FILAMENT_ARCHIVES)
    message(FATAL_ERROR "No Filament archives found for ${ANDROID_ABI}")
endif()

add_library(helloxr SHARED
    "${REPO_ROOT}/samples/helloxr.cpp"
    "${REPO_ROOT}/samples/helloxr_features.cpp"
    "${REPO_ROOT}/samples/helloxr_foveation.cpp"
    "${REPO_ROOT}/samples/helloxr_jetpack_ui.cpp"
    "${REPO_ROOT}/samples/helloxr_quad_layer.cpp"
    "${REPO_ROOT}/samples/helloxr_rendermodel.cpp"
    "${REPO_ROOT}/samples/helloxr_handmesh.cpp"
    "${REPO_ROOT}/samples/helloxr_vertexstreaming.cpp"
    "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
)

target_include_directories(helloxr PRIVATE
    "${FILAMENT_DIST_DIR}/include"
    # bluevk is an internal dependency and is not part of the install, but VulkanPlatform.h needs it.
    "${REPO_ROOT}/libs/bluevk/include"
    "${ANDROID_NDK}/sources/android/native_app_glue"
)

# Match how the Filament archives themselves were compiled.
target_compile_options(helloxr PRIVATE -fno-exceptions -fno-rtti)

target_link_libraries(helloxr PRIVATE
    OpenXR::openxr_loader
    -Wl,--start-group ${FILAMENT_ARCHIVES} -Wl,--end-group
    android
    log
    m
    # The Android build of Filament always contains the GL backend, even though this sample only
    # ever creates a Vulkan one.
    GLESv3
    EGL
)

# native_app_glue reaches android_main through this entry point, which the linker would otherwise
# consider unreferenced.
target_link_options(helloxr PRIVATE -u ANativeActivity_onCreate)
