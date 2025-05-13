#!/bin/bash
set -e

# Host tools required by Android, WebGL, and iOS builds
MOBILE_HOST_TOOLS="matc resgen cmgen filamesh uberz"
WEB_HOST_TOOLS="${MOBILE_HOST_TOOLS} mipgen filamesh"

function print_help {
    local self_name=$(basename "$0")
    echo "Usage:"
    echo "    $self_name [options] <build_type1> [<build_type2> ...] [targets]"
    echo ""
    echo "Options:"
    echo "    -h"
    echo "        Print this help message."
    echo "    -a"
    echo "        Generate .tgz build archives, implies -i."
    echo "    -c"
    echo "        Clean build directories."
    echo "    -C"
    echo "        Clean build directories and revert android/ to a freshly sync'ed state."
    echo "        All (and only) git-ignored files under android/ are deleted."
    echo "        This is sometimes needed instead of -c (which still misses some clean steps)."
    echo "    -d"
    echo "        Enable matdbg."
    echo "    -t"
    echo "        Enable fgviewer."
    echo "    -f"
    echo "        Always invoke CMake before incremental builds."
    echo "    -g"
    echo "        Disable material optimization."
    echo "    -i"
    echo "        Install build output"
    echo "    -m"
    echo "        Compile with make instead of ninja."
    echo "    -p platform1,platform2,..."
    echo "        Where platformN is [desktop|android|ios|webgl|all]."
    echo "        Platform(s) to build, defaults to desktop."
    echo "        Building for iOS will automatically perform a partial desktop build."
    echo "    -q abi1,abi2,..."
    echo "        Where platformN is [armeabi-v7a|arm64-v8a|x86|x86_64|all]."
    echo "        ABIs to build when the platform is Android. Defaults to all."
    echo "    -u"
    echo "        Run all unit tests, will trigger a debug build if needed."
    echo "    -v"
    echo "        Exclude Vulkan support from the Android build."
    echo "    -W"
    echo "        Include WebGPU support for the target platform. (NOT functional atm)."
    echo "    -s"
    echo "        Add iOS simulator support to the iOS build."
    echo "    -e"
    echo "        Enable EGL on Linux support for desktop builds."
    echo "    -l"
    echo "        Build arm64/x86_64 universal libraries."
    echo "        For iOS, this builds universal binaries for devices and the simulator (implies -s)."
    echo "        For macOS, this builds universal binaries for both Apple silicon and Intel-based Macs."
    echo "    -w"
    echo "        Build Web documents (compiles .md.html files to .html)."
    echo "    -k sample1,sample2,..."
    echo "        When building for Android, also build select sample APKs."
    echo "        sampleN is an Android sample, e.g., sample-gltf-viewer."
    echo "        This automatically performs a partial desktop build and install."
    echo "    -b"
    echo "        Enable Address and Undefined Behavior Sanitizers (asan/ubsan) for debugging."
    echo "        This is only for the desktop build."
    echo "    -x value"
    echo "        Define a preprocessor flag FILAMENT_BACKEND_DEBUG_FLAG with [value]. This is useful for"
    echo "        enabling debug paths in the backend from the build script. For example, make a"
    echo "        systrace-enabled build without directly changing #defines. Remember to add -f when"
    echo "        changing this option."
    echo "    -X osmesa_path"
    echo "        Indicates a path to a completed OSMesa build. OSMesa is used to create an offscreen GL"
    echo "        context for software rasterization"
    echo "    -S type"
    echo "        Enable stereoscopic rendering where type is one of [instanced|multiview]. This is only"
    echo "        meant for building the samples."
    echo ""
    echo "Build types:"
    echo "    release"
    echo "        Release build only"
    echo "    debug"
    echo "        Debug build only"
    echo ""
    echo "Targets:"
    echo "    Any target supported by the underlying build system"
    echo ""
    echo "Examples:"
    echo "    Desktop release build:"
    echo "        \$ ./$self_name release"
    echo ""
    echo "    Desktop debug and release builds:"
    echo "        \$ ./$self_name debug release"
    echo ""
    echo "    Clean, desktop debug build and create archive of build artifacts:"
    echo "        \$ ./$self_name -c -a debug"
    echo ""
    echo "    Android release build type:"
    echo "        \$ ./$self_name -p android release"
    echo ""
    echo "    Desktop and Android release builds, with installation:"
    echo "        \$ ./$self_name -p desktop,android -i release"
    echo ""
    echo "    Desktop matc target, release build:"
    echo "        \$ ./$self_name release matc"
    echo ""
    echo "    Build gltf_viewer:"
    echo "        \$ ./$self_name release gltf_viewer"
    echo ""
 }

function print_matdbg_help {
    echo "matdbg is enabled in the build, but some extra steps are needed."
    echo ""
    echo "FOR DESKTOP BUILDS:"
    echo ""
    echo "Please set the port environment variable before launching. e.g., on macOS do:"
    echo "   export FILAMENT_MATDBG_PORT=8080"
    echo ""
    echo "FOR ANDROID BUILDS:"
    echo ""
    echo "1) For Android Studio builds, make sure to set:"
    echo "       -Pcom.google.android.filament.matdbg"
    echo "   option in Preferences > Build > Compiler > Command line options."
    echo ""
    echo "2) The port number is hardcoded to 8081 so you will need to do:"
    echo "       adb forward tcp:8081 tcp:8081"
    echo ""
    echo "3) Be sure to enable INTERNET permission in your app's manifest file."
    echo ""
}

function print_fgviewer_help {
    echo "fgviewer is enabled in the build, but some extra steps are needed."
    echo ""
    echo "FOR DESKTOP BUILDS:"
    echo ""
    echo "Please set the port environment variable before launching. e.g., on macOS do:"
    echo "   export FILAMENT_FGVIEWER_PORT=8085"
    echo ""
    echo "FOR ANDROID BUILDS:"
    echo ""
    echo "1) For Android Studio builds, make sure to set:"
    echo "       -Pcom.google.android.filament.fgviewer"
    echo "   option in Preferences > Build > Compiler > Command line options."
    echo ""
    echo "2) The port number is hardcoded to 8085 so you will need to do:"
    echo "       adb forward tcp:8085 tcp:8085"
    echo ""
    echo "3) Be sure to enable INTERNET permission in your app's manifest file."
    echo ""
}

# Unless explicitly specified, NDK version will be selected as highest available version within same major release chain
FILAMENT_NDK_VERSION=${FILAMENT_NDK_VERSION:-$(cat `dirname $0`/build/common/versions | grep GITHUB_NDK_VERSION | sed s/GITHUB_NDK_VERSION=//g | cut -f 1 -d ".")}

# Requirements
CMAKE_MAJOR=3
CMAKE_MINOR=19

# Internal variables
ISSUE_CLEAN=false
ISSUE_CLEAN_AGGRESSIVE=false

ISSUE_DEBUG_BUILD=false
ISSUE_RELEASE_BUILD=false

# Default: build desktop only
ISSUE_ANDROID_BUILD=false
ISSUE_IOS_BUILD=false
ISSUE_DESKTOP_BUILD=true
ISSUE_WEBGL_BUILD=false

# Default: all
ABI_ARMEABI_V7A=true
ABI_ARM64_V8A=true
ABI_X86=true
ABI_X86_64=true
ABI_GRADLE_OPTION="all"

ISSUE_ARCHIVES=false
BUILD_JS_DOCS=false

ISSUE_CMAKE_ALWAYS=false

ISSUE_WEB_DOCS=false

ANDROID_SAMPLES=()
BUILD_ANDROID_SAMPLES=false

RUN_TESTS=false

INSTALL_COMMAND=

VULKAN_ANDROID_OPTION="-DFILAMENT_SUPPORTS_VULKAN=ON"
VULKAN_ANDROID_GRADLE_OPTION=""

WEBGPU_OPTION="-DFILAMENT_SUPPORTS_WEBGPU=OFF"
WEBGPU_ANDROID_GRADLE_OPTION=""

EGL_ON_LINUX_OPTION="-DFILAMENT_SUPPORTS_EGL_ON_LINUX=OFF"

MATDBG_OPTION="-DFILAMENT_ENABLE_MATDBG=OFF"
MATDBG_GRADLE_OPTION=""
FGVIEWER_OPTION="-DFILAMENT_ENABLE_FGVIEWER=OFF"
FGVIEWER_GRADLE_OPTION=""

MATOPT_OPTION=""
MATOPT_GRADLE_OPTION=""

ASAN_UBSAN_OPTION=""

BACKEND_DEBUG_FLAG_OPTION=""

STEREOSCOPIC_OPTION=""

OSMESA_OPTION=""

IOS_BUILD_SIMULATOR=false
BUILD_UNIVERSAL_LIBRARIES=false

BUILD_GENERATOR=Ninja
BUILD_COMMAND=ninja
BUILD_CUSTOM_TARGETS=

UNAME=$(uname)
LC_UNAME=$(echo "${UNAME}" | tr '[:upper:]' '[:lower:]')

# Functions

function build_clean {
    echo "Cleaning build directories..."
    rm -Rf out
    rm -Rf android/filament-android/build android/filament-android/.externalNativeBuild android/filament-android/.cxx
    rm -Rf android/filamat-android/build android/filamat-android/.externalNativeBuild android/filamat-android/.cxx
    rm -Rf android/gltfio-android/build android/gltfio-android/.externalNativeBuild android/gltfio-android/.cxx
    rm -Rf android/filament-utils-android/build android/filament-utils-android/.externalNativeBuild android/filament-utils-android/.cxx
    rm -f compile_commands.json
}

function build_clean_aggressive {
    echo "Cleaning build directories..."
    rm -Rf out
    git clean -qfX android
}

function build_desktop_target {
    local lc_target=$(echo "$1" | tr '[:upper:]' '[:lower:]')
    local build_targets=$2

    if [[ ! "${build_targets}" ]]; then
        build_targets=${BUILD_CUSTOM_TARGETS}
    fi

    echo "Building ${lc_target} in out/cmake-${lc_target}..."
    mkdir -p "out/cmake-${lc_target}"

    pushd "out/cmake-${lc_target}" > /dev/null

    local lc_name=$(echo "${UNAME}" | tr '[:upper:]' '[:lower:]')
    if [[ "${lc_name}" == "darwin" ]]; then
        if [[ "${BUILD_UNIVERSAL_LIBRARIES}" == "true" ]]; then
            local architectures="-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64"
        fi
    fi

    if [[ ! -d "CMakeFiles" ]] || [[ "${ISSUE_CMAKE_ALWAYS}" == "true" ]]; then
        cmake \
            -G "${BUILD_GENERATOR}" \
            -DIMPORT_EXECUTABLES_DIR=out \
            -DCMAKE_BUILD_TYPE="$1" \
            -DCMAKE_INSTALL_PREFIX="../${lc_target}/filament" \
            ${EGL_ON_LINUX_OPTION} \
            ${FGVIEWER_OPTION} \
            ${WEBGPU_OPTION} \
            ${MATDBG_OPTION} \
            ${MATOPT_OPTION} \
            ${ASAN_UBSAN_OPTION} \
            ${BACKEND_DEBUG_FLAG_OPTION} \
            ${STEREOSCOPIC_OPTION} \
            ${OSMESA_OPTION} \
            ${architectures} \
            ../..
        ln -sf "out/cmake-${lc_target}/compile_commands.json" \
           ../../compile_commands.json
    fi
    ${BUILD_COMMAND} ${build_targets}

    if [[ "${INSTALL_COMMAND}" ]]; then
        echo "Installing ${lc_target} in out/${lc_target}/filament..."
        ${BUILD_COMMAND} ${INSTALL_COMMAND}
    fi

    if [[ -d "../${lc_target}/filament" ]]; then
        if [[ "${ISSUE_ARCHIVES}" == "true" ]]; then
            echo "Generating out/filament-${lc_target}-${LC_UNAME}.tgz..."
            pushd "../${lc_target}" > /dev/null
            tar -czvf "../filament-${lc_target}-${LC_UNAME}.tgz" filament
            popd > /dev/null
        fi
    fi

    popd > /dev/null
}

function build_desktop {
    if [[ "${ISSUE_DEBUG_BUILD}" == "true" ]]; then
        build_desktop_target "Debug" "$1"
    fi

    if [[ "${ISSUE_RELEASE_BUILD}" == "true" ]]; then
        build_desktop_target "Release" "$1"
    fi
}

function build_webgl_with_target {
    local lc_target=$(echo "$1" | tr '[:upper:]' '[:lower:]')

    echo "Building WebGL ${lc_target}..."
    mkdir -p "out/cmake-webgl-${lc_target}"
    pushd "out/cmake-webgl-${lc_target}" > /dev/null

    if [[ ! "${BUILD_TARGETS}" ]]; then
        BUILD_TARGETS=${BUILD_CUSTOM_TARGETS}
        ISSUE_CMAKE_ALWAYS=true
    fi

    if [[ ! -d "CMakeFiles" ]] || [[ "${ISSUE_CMAKE_ALWAYS}" == "true" ]]; then
        # Apply the emscripten environment within a subshell.
        (
        # shellcheck disable=SC1090
        source "${EMSDK}/emsdk_env.sh"
        cmake \
            -G "${BUILD_GENERATOR}" \
            -DIMPORT_EXECUTABLES_DIR=out \
            -DCMAKE_TOOLCHAIN_FILE="${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" \
            -DCMAKE_BUILD_TYPE="$1" \
            -DCMAKE_INSTALL_PREFIX="../webgl-${lc_target}/filament" \
            -DWEBGL=1 \
            ${WEBGPU_OPTION} \
            ${BACKEND_DEBUG_FLAG_OPTION} \
            ../..
        ln -sf "out/cmake-webgl-${lc_target}/compile_commands.json" \
           ../../compile_commands.json
        ${BUILD_COMMAND} ${BUILD_TARGETS}
        )
    fi

    if [[ -d "web/filament-js" ]]; then

        if [[ "${BUILD_JS_DOCS}" == "true" ]]; then
            echo "Generating JavaScript documentation..."
            local DOCS_FOLDER="web/docs"
            local DOCS_SCRIPT="../../web/docs/build.py"
            python3 ${DOCS_SCRIPT} --disable-demo \
                --output-folder "${DOCS_FOLDER}" \
                --build-folder "${PWD}"
        fi

        if [[ "${ISSUE_ARCHIVES}" == "true" ]]; then
            echo "Generating out/filament-${lc_target}-web.tgz..."
            pushd web/filament-js > /dev/null
            tar -cvf "../../../filament-${lc_target}-web.tar" filament.js
            tar -rvf "../../../filament-${lc_target}-web.tar" filament.wasm
            tar -rvf "../../../filament-${lc_target}-web.tar" filament.d.ts
            popd > /dev/null
            gzip -c "../filament-${lc_target}-web.tar" > "../filament-${lc_target}-web.tgz"
            rm "../filament-${lc_target}-web.tar"
        fi
    fi

    popd > /dev/null
}

function build_webgl {
    # For the host tools, suppress install and always use Release.
    local old_install_command=${INSTALL_COMMAND}; INSTALL_COMMAND=
    local old_issue_debug_build=${ISSUE_DEBUG_BUILD}; ISSUE_DEBUG_BUILD=false
    local old_issue_release_build=${ISSUE_RELEASE_BUILD}; ISSUE_RELEASE_BUILD=true

    build_desktop "${WEB_HOST_TOOLS}"

    INSTALL_COMMAND=${old_install_command}
    ISSUE_DEBUG_BUILD=${old_issue_debug_build}
    ISSUE_RELEASE_BUILD=${old_issue_release_build}

    if [[ "${ISSUE_DEBUG_BUILD}" == "true" ]]; then
        build_webgl_with_target "Debug"
    fi

    if [[ "${ISSUE_RELEASE_BUILD}" == "true" ]]; then
        build_webgl_with_target "Release"
    fi
}

function build_android_target {
    local lc_target=$(echo "$1" | tr '[:upper:]' '[:lower:]')
    local arch=$2

    echo "Building Android ${lc_target} (${arch})..."
    mkdir -p "out/cmake-android-${lc_target}-${arch}"

    pushd "out/cmake-android-${lc_target}-${arch}" > /dev/null

    if [[ ! -d "CMakeFiles" ]] || [[ "${ISSUE_CMAKE_ALWAYS}" == "true" ]]; then
        cmake \
            -G "${BUILD_GENERATOR}" \
            -DIMPORT_EXECUTABLES_DIR=out \
            -DCMAKE_BUILD_TYPE="$1" \
            -DFILAMENT_NDK_VERSION="${FILAMENT_NDK_VERSION}" \
            -DCMAKE_INSTALL_PREFIX="../android-${lc_target}/filament" \
            -DCMAKE_TOOLCHAIN_FILE="../../build/toolchain-${arch}-linux-android.cmake" \
            ${FGVIEWER_OPTION} \
            ${MATDBG_OPTION} \
            ${MATOPT_OPTION} \
            ${VULKAN_ANDROID_OPTION} \
            ${WEBGPU_OPTION} \
            ${BACKEND_DEBUG_FLAG_OPTION} \
            ${STEREOSCOPIC_OPTION} \
            ../..
        ln -sf "out/cmake-android-${lc_target}-${arch}/compile_commands.json" \
           ../../compile_commands.json
    fi

    # We must always install Android libraries to build the AAR
    ${BUILD_COMMAND} install

    popd > /dev/null
}

function build_android_arch {
    local arch=$1

    if [[ "${ISSUE_DEBUG_BUILD}" == "true" ]]; then
        build_android_target "Debug" "${arch}"
    fi

    if [[ "${ISSUE_RELEASE_BUILD}" == "true" ]]; then
        build_android_target "Release" "${arch}"
    fi
}

function archive_android {
    local lc_target=$(echo "$1" | tr '[:upper:]' '[:lower:]')

    if [[ -d "out/android-${lc_target}/filament" ]]; then
        if [[ "${ISSUE_ARCHIVES}" == "true" ]]; then
            echo "Generating out/filament-android-${lc_target}-${LC_UNAME}.tgz..."
            pushd "out/android-${lc_target}" > /dev/null
            tar -czvf "../filament-android-${lc_target}-${LC_UNAME}.tgz" filament
            popd > /dev/null
        fi
    fi
}

function ensure_android_build {
    if [[ "${ANDROID_HOME}" == "" ]]; then
        echo "Error: ANDROID_HOME is not set, exiting"
        exit 1
    fi

    # shellcheck disable=SC2012
    if [[ -z $(ls "${ANDROID_HOME}/ndk/" | sort -V | grep "^${FILAMENT_NDK_VERSION}") ]]; then
        echo "Error: Android NDK side-by-side version ${FILAMENT_NDK_VERSION} or compatible must be installed, exiting"
        exit 1
    fi
}

function build_android {
    ensure_android_build

    # Suppress intermediate desktop tools install
    local old_install_command=${INSTALL_COMMAND}
    INSTALL_COMMAND=

    build_desktop "${MOBILE_HOST_TOOLS}"

    # When building the samples, we need to partially "install" the host tools so Gradle can see
    # them.
    if [[ "${BUILD_ANDROID_SAMPLES}" == "true" ]]; then
        if [[ "${ISSUE_DEBUG_BUILD}" == "true" ]]; then
            mkdir -p out/debug/filament/bin
            for tool in ${MOBILE_HOST_TOOLS}; do
                cp out/cmake-debug/tools/${tool}/${tool} out/debug/filament/bin/
            done
        fi

        if [[ "${ISSUE_RELEASE_BUILD}" == "true" ]]; then
            mkdir -p out/release/filament/bin
            for tool in ${MOBILE_HOST_TOOLS}; do
                cp out/cmake-release/tools/${tool}/${tool} out/release/filament/bin/
            done
        fi
    fi

    INSTALL_COMMAND=${old_install_command}

    if [[ "${ABI_ARM64_V8A}" == "true" ]]; then
        build_android_arch "aarch64" "aarch64-linux-android"
    fi
    if [[ "${ABI_ARMEABI_V7A}" == "true" ]]; then
        build_android_arch "arm7" "arm-linux-androideabi"
    fi
    if [[ "${ABI_X86_64}" == "true" ]]; then
        build_android_arch "x86_64" "x86_64-linux-android"
    fi
    if [[ "${ABI_X86}" == "true" ]]; then
        build_android_arch "x86" "i686-linux-android"
    fi

    if [[ "${ISSUE_DEBUG_BUILD}" == "true" ]]; then
        archive_android "Debug"
    fi

    if [[ "${ISSUE_RELEASE_BUILD}" == "true" ]]; then
        archive_android "Release"
    fi

    pushd android > /dev/null

    if [[ "${ISSUE_DEBUG_BUILD}" == "true" ]]; then
        ./gradlew \
            -Pcom.google.android.filament.dist-dir=../out/android-debug/filament \
            -Pcom.google.android.filament.abis=${ABI_GRADLE_OPTION} \
            ${VULKAN_ANDROID_GRADLE_OPTION} \
            ${WEBGPU_ANDROID_GRADLE_OPTION} \
            ${MATDBG_GRADLE_OPTION} \
            ${FGVIEWER_GRADLE_OPTION} \
            ${MATOPT_GRADLE_OPTION} \
            :filament-android:assembleDebug \
            :gltfio-android:assembleDebug \
            :filament-utils-android:assembleDebug

        ./gradlew \
            -Pcom.google.android.filament.dist-dir=../out/android-debug/filament \
            -Pcom.google.android.filament.abis=${ABI_GRADLE_OPTION} \
            ${WEBGPU_ANDROID_GRADLE_OPTION} \
            :filamat-android:assembleDebug

        if [[ "${BUILD_ANDROID_SAMPLES}" == "true" ]]; then
            for sample in ${ANDROID_SAMPLES}; do
                ./gradlew \
                    -Pcom.google.android.filament.dist-dir=../out/android-debug/filament \
                    -Pcom.google.android.filament.abis=${ABI_GRADLE_OPTION} \
                    ${MATOPT_GRADLE_OPTION} \
                    :samples:${sample}:assembleDebug
            done
        fi

        if [[ "${INSTALL_COMMAND}" ]]; then
            echo "Installing out/filamat-android-debug.aar..."
            cp filamat-android/build/outputs/aar/filamat-android-debug.aar ../out/filamat-android-debug.aar

            echo "Installing out/filament-android-debug.aar..."
            cp filament-android/build/outputs/aar/filament-android-debug.aar ../out/

            echo "Installing out/gltfio-android-debug.aar..."
            cp gltfio-android/build/outputs/aar/gltfio-android-debug.aar ../out/gltfio-android-debug.aar

            echo "Installing out/filament-utils-android-debug.aar..."
            cp filament-utils-android/build/outputs/aar/filament-utils-android-debug.aar ../out/filament-utils-android-debug.aar

            if [[ "${BUILD_ANDROID_SAMPLES}" == "true" ]]; then
                for sample in ${ANDROID_SAMPLES}; do
                    echo "Installing out/${sample}-debug.apk"
                    cp samples/${sample}/build/outputs/apk/debug/${sample}-debug.apk \
                        ../out/${sample}-debug.apk
                done
            fi
        fi
    fi

    if [[ "${ISSUE_RELEASE_BUILD}" == "true" ]]; then
        ./gradlew \
            -Pcom.google.android.filament.dist-dir=../out/android-release/filament \
            -Pcom.google.android.filament.abis=${ABI_GRADLE_OPTION} \
            ${VULKAN_ANDROID_GRADLE_OPTION} \
            ${WEBGPU_ANDROID_GRADLE_OPTION} \
            ${MATDBG_GRADLE_OPTION} \
            ${FGVIEWER_GRADLE_OPTION} \
            ${MATOPT_GRADLE_OPTION} \
            :filament-android:assembleRelease \
            :gltfio-android:assembleRelease \
            :filament-utils-android:assembleRelease

        ./gradlew \
            -Pcom.google.android.filament.dist-dir=../out/android-release/filament \
            -Pcom.google.android.filament.abis=${ABI_GRADLE_OPTION} \
            ${WEBGPU_ANDROID_GRADLE_OPTION} \
            :filamat-android:assembleRelease

        if [[ "${BUILD_ANDROID_SAMPLES}" == "true" ]]; then
            for sample in ${ANDROID_SAMPLES}; do
                ./gradlew \
                    -Pcom.google.android.filament.dist-dir=../out/android-release/filament \
                    -Pcom.google.android.filament.abis=${ABI_GRADLE_OPTION} \
                    ${MATOPT_GRADLE_OPTION} \
                    :samples:${sample}:assembleRelease
            done
        fi

        if [[ "${INSTALL_COMMAND}" ]]; then
            echo "Installing out/filamat-android-release.aar..."
            cp filamat-android/build/outputs/aar/filamat-android-release.aar ../out/filamat-android-release.aar

            echo "Installing out/filament-android-release.aar..."
            cp filament-android/build/outputs/aar/filament-android-release.aar ../out/

            echo "Installing out/gltfio-android-release.aar..."
            cp gltfio-android/build/outputs/aar/gltfio-android-release.aar ../out/gltfio-android-release.aar

            echo "Installing out/filament-utils-android-release.aar..."
            cp filament-utils-android/build/outputs/aar/filament-utils-android-release.aar ../out/filament-utils-android-release.aar

            if [[ "${BUILD_ANDROID_SAMPLES}" == "true" ]]; then
                for sample in ${ANDROID_SAMPLES}; do
                    echo "Installing out/${sample}-release.apk"
                    cp samples/${sample}/build/outputs/apk/release/${sample}-release-unsigned.apk \
                        ../out/${sample}-release.apk
                done
            fi
        fi
    fi

    popd > /dev/null
}

function build_ios_target {
    local lc_target=$(echo "$1" | tr '[:upper:]' '[:lower:]')
    local arch=$2
    local platform=$3

    echo "Building iOS ${lc_target} (${arch}) for ${platform}..."
    mkdir -p "out/cmake-ios-${lc_target}-${arch}"

    pushd "out/cmake-ios-${lc_target}-${arch}" > /dev/null

    if [[ ! -d "CMakeFiles" ]] || [[ "${ISSUE_CMAKE_ALWAYS}" == "true" ]]; then
        cmake \
            -G "${BUILD_GENERATOR}" \
            -DIMPORT_EXECUTABLES_DIR=out \
            -DCMAKE_BUILD_TYPE="$1" \
            -DCMAKE_INSTALL_PREFIX="../ios-${lc_target}/filament" \
            -DIOS_ARCH="${arch}" \
            -DPLATFORM_NAME="${platform}" \
            -DIOS=1 \
            -DCMAKE_TOOLCHAIN_FILE=../../third_party/clang/iOS.cmake \
            ${FGVIEWER_OPTION} \
            ${WEBGPU_OPTION} \
            ${MATDBG_OPTION} \
            ${MATOPT_OPTION} \
            ${STEREOSCOPIC_OPTION} \
            ../..
        ln -sf "out/cmake-ios-${lc_target}-${arch}/compile_commands.json" \
           ../../compile_commands.json
    fi

    ${BUILD_COMMAND}

    if [[ "${INSTALL_COMMAND}" ]]; then
        echo "Installing ${lc_target} in out/${lc_target}/filament..."
        ${BUILD_COMMAND} ${INSTALL_COMMAND}
    fi

    popd > /dev/null
}

function archive_ios {
    local lc_target=$(echo "$1" | tr '[:upper:]' '[:lower:]')

    if [[ -d "out/ios-${lc_target}/filament" ]]; then
        if [[ "${ISSUE_ARCHIVES}" == "true" ]]; then
            echo "Generating out/filament-${lc_target}-ios.tgz..."
            pushd "out/ios-${lc_target}" > /dev/null
            tar -czvf "../filament-${lc_target}-ios.tgz" filament
            popd > /dev/null
        fi
    fi
}

function build_ios {
    # Suppress intermediate desktop tools install
    local old_install_command=${INSTALL_COMMAND}
    INSTALL_COMMAND=

    build_desktop "${MOBILE_HOST_TOOLS}"

    INSTALL_COMMAND=${old_install_command}

    # In theory, we could support iPhone architectures older than arm64, but
    # only arm64 devices support OpenGL 3.0 / Metal

    if [[ "${ISSUE_DEBUG_BUILD}" == "true" ]]; then
        build_ios_target "Debug" "arm64" "iphoneos"
        if [[ "${IOS_BUILD_SIMULATOR}" == "true" ]]; then
            build_ios_target "Debug" "x86_64" "iphonesimulator"
        fi

        if [[ "${BUILD_UNIVERSAL_LIBRARIES}" == "true" ]]; then
            build/ios/create-universal-libs.sh \
                -o out/ios-debug/filament/lib/universal \
                out/ios-debug/filament/lib/arm64 \
                out/ios-debug/filament/lib/x86_64
            rm -rf out/ios-debug/filament/lib/arm64
            rm -rf out/ios-debug/filament/lib/x86_64
        fi

        archive_ios "Debug"
    fi

    if [[ "${ISSUE_RELEASE_BUILD}" == "true" ]]; then
        build_ios_target "Release" "arm64" "iphoneos"
        if [[ "${IOS_BUILD_SIMULATOR}" == "true" ]]; then
            build_ios_target "Release" "x86_64" "iphonesimulator"
        fi

        if [[ "${BUILD_UNIVERSAL_LIBRARIES}" == "true" ]]; then
            build/ios/create-universal-libs.sh \
                -o out/ios-release/filament/lib/universal \
                out/ios-release/filament/lib/arm64 \
                out/ios-release/filament/lib/x86_64
            rm -rf out/ios-release/filament/lib/arm64
            rm -rf out/ios-release/filament/lib/x86_64
        fi

        archive_ios "Release"
    fi
}

function build_web_docs {
    echo "Building Web documents..."

    mkdir -p out/web-docs
    cp -f docs/web-docs-package.json out/web-docs/package.json
    pushd out/web-docs > /dev/null

    npm install > /dev/null

    # Generate documents
    npx markdeep-rasterizer ../../docs/Filament.md.html ../../docs/Materials.md.html  ../../docs/

    popd > /dev/null
}

function validate_build_command {
    set +e
    # Make sure CMake is installed
    local cmake_binary=$(command -v cmake)
    if [[ ! "${cmake_binary}" ]]; then
        echo "Error: could not find cmake, exiting"
        exit 1
    fi

    # Make sure Ninja is installed
    if [[ "${BUILD_COMMAND}" == "ninja" ]]; then
        local ninja_binary=$(command -v ninja)
        if [[ ! "${ninja_binary}" ]]; then
            echo "Warning: could not find ninja, using make instead"
            BUILD_GENERATOR="Unix Makefiles"
            BUILD_COMMAND="make"
        fi
    fi
    # Make sure Make is installed
    if [[ "${BUILD_COMMAND}" == "make" ]]; then
        local make_binary=$(command -v make)
        if [[ ! "${make_binary}" ]]; then
            echo "Error: could not find make, exiting"
            exit 1
        fi
    fi
    # If building a WebAssembly module, ensure we know where Emscripten lives.
    if [[ "${EMSDK}" == "" ]] && [[ "${ISSUE_WEBGL_BUILD}" == "true" ]]; then
        echo "Error: EMSDK is not set, exiting"
        exit 1
    fi
    # Web documents require node and npm for processing
    if [[ "${ISSUE_WEB_DOCS}" == "true" ]]; then
        local node_binary=$(command -v node)
        local npm_binary=$(command -v npm)
        local npx_binary=$(command -v npx)
        if [[ ! "${node_binary}" ]] || [[ ! "${npm_binary}" ]] || [[ ! "${npx_binary}" ]]; then
            echo "Error: Web documents require node, npm and npx to be installed"
            exit 1
        fi
    fi

    # Make sure FILAMENT_BACKEND_DEBUG_FLAG is only meant for debug builds
    if [[ "${ISSUE_DEBUG_BUILD}" != "true" ]] && [[ ! -z "${BACKEND_DEBUG_FLAG_OPTION}" ]]; then
        echo "Error: cannot specify FILAMENT_BACKEND_DEBUG_FLAG in non-debug build"
        exit 1
    fi

    set -e
}

function run_test {
    local test=$1
    # The input string might contain arguments, so we use "set -- $test" to replace $1 with the
    # first whitespace-separated token in the string.
    # shellcheck disable=SC2086
    set -- ${test}
    local test_name=$(basename "$1")
    # shellcheck disable=SC2086
    ./out/cmake-debug/${test} --gtest_output="xml:out/test-results/${test_name}/sponge_log.xml"
}

function run_tests {
    if [[ "${ISSUE_WEBGL_BUILD}" == "true" ]]; then
        if ! echo "TypeScript $(tsc --version)" ; then
            tsc --noEmit \
                third_party/gl-matrix/gl-matrix.d.ts \
                web/filament-js/filament.d.ts \
                web/filament-js/test.ts
        fi
    else
        while read -r test; do
            run_test "${test}"
        done < build/common/test_list.txt
    fi
}

function check_debug_release_build {
    if [[ "${ISSUE_DEBUG_BUILD}" == "true" || \
          "${ISSUE_RELEASE_BUILD}" == "true" || \
          "${ISSUE_CLEAN}" == "true" || \
          "${ISSUE_WEB_DOCS}" == "true" ]]; then
        "$@";
    else
        echo "You must declare a debug or release target for $@ builds."
        echo ""
        exit 1
    fi
}

# Beginning of the script

pushd "$(dirname "$0")" > /dev/null

while getopts ":hacCfgimp:q:uvWslwedtk:bx:S:X:" opt; do
    case ${opt} in
        h)
            print_help
            exit 0
            ;;
        a)
            ISSUE_ARCHIVES=true
            INSTALL_COMMAND=install
            ;;
        c)
            ISSUE_CLEAN=true
            ;;
        C)
            ISSUE_CLEAN_AGGRESSIVE=true
            ;;
        d)
            PRINT_MATDBG_HELP=true
            MATDBG_OPTION="-DFILAMENT_ENABLE_MATDBG=ON, -DFILAMENT_BUILD_FILAMAT=ON"
            MATDBG_GRADLE_OPTION="-Pcom.google.android.filament.matdbg"
            ;;
        t)
            PRINT_FGVIEWER_HELP=true
            FGVIEWER_OPTION="-DFILAMENT_ENABLE_FGVIEWER=ON"
            FGVIEWER_GRADLE_OPTION="-Pcom.google.android.filament.fgviewer"
            ;;
        f)
            ISSUE_CMAKE_ALWAYS=true
            ;;
        g)
            MATOPT_OPTION="-DFILAMENT_DISABLE_MATOPT=ON"
            MATOPT_GRADLE_OPTION="-Pcom.google.android.filament.matnopt"
            ;;
        i)
            INSTALL_COMMAND=install
            ;;
        m)
            BUILD_GENERATOR="Unix Makefiles"
            BUILD_COMMAND="make"
            ;;
        p)
            ISSUE_DESKTOP_BUILD=false
            platforms=$(echo "${OPTARG}" | tr ',' '\n')
            for platform in ${platforms}
            do
                case $(echo "${platform}" | tr '[:upper:]' '[:lower:]') in
                    desktop)
                        ISSUE_DESKTOP_BUILD=true
                    ;;
                    android)
                        ISSUE_ANDROID_BUILD=true
                    ;;
                    ios)
                        ISSUE_IOS_BUILD=true
                    ;;
                    webgl)
                        ISSUE_WEBGL_BUILD=true
                    ;;
                    all)
                        ISSUE_ANDROID_BUILD=true
                        ISSUE_IOS_BUILD=true
                        ISSUE_DESKTOP_BUILD=true
                        ISSUE_WEBGL_BUILD=false
                    ;;
                    *)
                        echo "Unknown platform ${platform}"
                        echo "Platform must be one of [desktop|android|ios|webgl|all]"
                        echo ""
                        exit 1
                    ;;
                esac
            done
            ;;
        q)
            ABI_ARMEABI_V7A=false
            ABI_ARM64_V8A=false
            ABI_X86=false
            ABI_X86_64=false
            ABI_GRADLE_OPTION="${OPTARG}"
            abis=$(echo "${OPTARG}" | tr ',' '\n')
            for abi in ${abis}
            do
                case $(echo "${abi}" | tr '[:upper:]' '[:lower:]') in
                    armeabi-v7a)
                        ABI_ARMEABI_V7A=true
                    ;;
                    arm64-v8a)
                        ABI_ARM64_V8A=true
                    ;;
                    x86)
                        ABI_X86=true
                    ;;
                    x86_64)
                        ABI_X86_64=true
                    ;;
                    all)
                        ABI_ARMEABI_V7A=true
                        ABI_ARM64_V8A=true
                        ABI_X86=true
                        ABI_X86_64=true
                    ;;
                    *)
                        echo "Unknown abi ${abi}"
                        echo "ABI must be one of [armeabi-v7a|arm64-v8a|x86|x86_64|all]"
                        echo ""
                        exit 1
                    ;;
                esac
            done
            ;;
        u)
            ISSUE_DEBUG_BUILD=true
            RUN_TESTS=true
            ;;
        v)
            VULKAN_ANDROID_OPTION="-DFILAMENT_SUPPORTS_VULKAN=OFF"
            VULKAN_ANDROID_GRADLE_OPTION="-Pcom.google.android.filament.exclude-vulkan"
            echo "Disabling support for Vulkan in the core Filament library."
            echo "Consider using -c after changing this option to clear the Gradle cache."
            ;;
        W)
            WEBGPU_OPTION="-DFILAMENT_SUPPORTS_WEBGPU=ON"
            WEBGPU_ANDROID_GRADLE_OPTION="-Pcom.google.android.filament.include-webgpu"
            echo "Enable support for WebGPU(Experimental) in the core Filament library."
            ;;
        s)
            IOS_BUILD_SIMULATOR=true
            echo "iOS simulator support enabled."
            ;;
        e)
            EGL_ON_LINUX_OPTION="-DFILAMENT_SUPPORTS_EGL_ON_LINUX=ON -DFILAMENT_SKIP_SDL2=ON -DFILAMENT_SKIP_SAMPLES=ON"
            echo "EGL on Linux support enabled; skipping SDL2."
            ;;
        l)
            IOS_BUILD_SIMULATOR=true
            BUILD_UNIVERSAL_LIBRARIES=true
            echo "Building universal libraries."
            ;;
        w)
            ISSUE_WEB_DOCS=true
            ;;
        k)
            BUILD_ANDROID_SAMPLES=true
            ANDROID_SAMPLES=$(echo "${OPTARG}" | tr ',' '\n')
            ;;
        b)  ASAN_UBSAN_OPTION="-DFILAMENT_ENABLE_ASAN_UBSAN=ON"
            echo "Enabled ASAN/UBSAN"
            ;;
        x)  BACKEND_DEBUG_FLAG_OPTION="-DFILAMENT_BACKEND_DEBUG_FLAG=${OPTARG}"
            ;;
        S)  case $(echo "${OPTARG}" | tr '[:upper:]' '[:lower:]') in
                instanced)
                    STEREOSCOPIC_OPTION="-DFILAMENT_SAMPLES_STEREO_TYPE=instanced"
                    ;;
                multiview)
                    STEREOSCOPIC_OPTION="-DFILAMENT_SAMPLES_STEREO_TYPE=multiview"
                    ;;
                *)
                    echo "Unknown stereoscopic type ${OPTARG}"
                    echo "Type must be one of [instanced|multiview]"
                    echo ""
                    exit 1
            esac
            ;;
        X)  OSMESA_OPTION="-DFILAMENT_OSMESA_PATH=${OPTARG}"
            ;;
        \?)
            echo "Invalid option: -${OPTARG}" >&2
            echo ""
            print_help
            exit 1
            ;;
        :)
            echo "Option -${OPTARG} requires an argument." >&2
            echo ""
            print_help
            exit 1
            ;;
    esac
done

if [[ "$#" == "0" ]]; then
    print_help
    exit 1
fi

shift $((OPTIND - 1))

for arg; do
    if [[ $(echo "${arg}" | tr '[:upper:]' '[:lower:]') == "release" ]]; then
        ISSUE_RELEASE_BUILD=true
    elif [[ $(echo "${arg}" | tr '[:upper:]' '[:lower:]') == "debug" ]]; then
        ISSUE_DEBUG_BUILD=true
    else
        BUILD_CUSTOM_TARGETS="${BUILD_CUSTOM_TARGETS} ${arg}"
    fi
done

validate_build_command

if [[ "${ISSUE_CLEAN}" == "true" ]]; then
    build_clean
fi

if [[ "${ISSUE_CLEAN_AGGRESSIVE}" == "true" ]]; then
    build_clean_aggressive
fi

if [[ "${ISSUE_DESKTOP_BUILD}" == "true" ]]; then
    check_debug_release_build build_desktop
fi

if [[ "${ISSUE_ANDROID_BUILD}" == "true" ]]; then
    check_debug_release_build build_android
fi

if [[ "${ISSUE_IOS_BUILD}" == "true" ]]; then
    check_debug_release_build build_ios
fi

if [[ "${ISSUE_WEBGL_BUILD}" == "true" ]]; then
    check_debug_release_build build_webgl
fi

if [[ "${ISSUE_WEB_DOCS}" == "true" ]]; then
    build_web_docs
fi

if [[ "${RUN_TESTS}" == "true" ]]; then
    run_tests
fi

if [[ "${PRINT_MATDBG_HELP}" == "true" ]]; then
    print_matdbg_help
fi

if [[ "${PRINT_FGVIEWER_HELP}" == "true" ]]; then
    print_fgviewer_help
fi
