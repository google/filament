#!/bin/bash
set -e

# Host tools required by Android, WebGL, and iOS builds
MOBILE_HOST_TOOLS="matc resgen cmgen filamesh"
WEB_HOST_TOOLS="${MOBILE_HOST_TOOLS} mipgen filamesh"
IOS_TOOLCHAIN_URL="https://opensource.apple.com/source/clang/clang-800.0.38/src/cmake/platforms/iOS.cmake"

function print_help {
    local self_name=`basename $0`
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
    echo "    -f"
    echo "        Always invoke CMake before incremental builds."
    echo "    -i"
    echo "        Install build output"
    echo "    -j"
    echo "        Do not compile desktop Java projects"
    echo "    -m"
    echo "        Compile with make instead of ninja."
    echo "    -p platform1,platform2,..."
    echo "        Where platformN is [desktop|android|ios|webgl|all]."
    echo "        Platform(s) to build, defaults to desktop."
    echo "        Building for iOS will automatically generate / download"
    echo "        the toolchains if needed and perform a partial desktop build."
    echo "    -u"
    echo "        Run all unit tests, will trigger a debug build if needed."
    echo "    -v"
    echo "        Add Vulkan support to the Android build."
    echo "    -s"
    echo "        Add iOS simulator support to the iOS build."
    echo "    -w"
    echo "        Build Web documents (compiles .md.html files to .html)."
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
    echo "    Build gltf_viewer then immediately run it with no arguments:"
    echo "        \$ ./$self_name release run_gltf_viewer"
    echo ""
 }

# Requirements
CMAKE_MAJOR=3
CMAKE_MINOR=10
ANDROID_NDK_VERSION=21

# Internal variables
TARGET=release

ISSUE_CLEAN=false

ISSUE_DEBUG_BUILD=false
ISSUE_RELEASE_BUILD=false

# Default: build desktop only
ISSUE_ANDROID_BUILD=false
ISSUE_IOS_BUILD=false
ISSUE_DESKTOP_BUILD=true
ISSUE_WEBGL_BUILD=false

ISSUE_ARCHIVES=false
BUILD_JS_DOCS=false

ISSUE_CMAKE_ALWAYS=false

ISSUE_WEB_DOCS=false

RUN_TESTS=false

JS_DOCS_OPTION="-DGENERATE_JS_DOCS=OFF"

FILAMENT_ENABLE_JAVA=ON

INSTALL_COMMAND=

VULKAN_ANDROID_OPTION="-DFILAMENT_SUPPORTS_VULKAN=OFF"
VULKAN_ANDROID_GRADLE_OPTION=""

IOS_BUILD_SIMULATOR=false

BUILD_GENERATOR=Ninja
BUILD_COMMAND=ninja
BUILD_CUSTOM_TARGETS=

UNAME=`echo $(uname)`
LC_UNAME=`echo ${UNAME} | tr '[:upper:]' '[:lower:]'`

# Functions

function build_clean {
    echo "Cleaning build directories..."
    rm -Rf out
    rm -Rf android/filament-android/build android/filament-android/.externalNativeBuild android/filament-android/.cxx
    rm -Rf android/filamat-android/build android/filamat-android/.externalNativeBuild android/filamat-android/.cxx
    rm -Rf android/gltfio-android/build android/gltfio-android/.externalNativeBuild android/gltfio-android/.cxx
    rm -Rf android/filament-utils-android/build android/filament-utils-android/.externalNativeBuild android/filament-utils-android/.cxx
}

function build_desktop_target {
    local lc_target=`echo $1 | tr '[:upper:]' '[:lower:]'`
    local build_targets=$2

    if [[ ! "$build_targets" ]]; then
        build_targets=${BUILD_CUSTOM_TARGETS}
    fi

    echo "Building $lc_target in out/cmake-${lc_target}..."
    mkdir -p out/cmake-${lc_target}

    cd out/cmake-${lc_target}

    # On macOS, set the deployment target to 10.14.
    local name=`echo $(uname)`
    local lc_name=`echo $name | tr '[:upper:]' '[:lower:]'`
    if [[ "$lc_name" == "darwin" ]]; then
        local deployment_target="-DCMAKE_OSX_DEPLOYMENT_TARGET=10.14"
    fi

    if [[ ! -d "CMakeFiles" ]] || [[ "$ISSUE_CMAKE_ALWAYS" == "true" ]]; then
        cmake \
            -G "$BUILD_GENERATOR" \
            -DIMPORT_EXECUTABLES_DIR=out \
            -DCMAKE_BUILD_TYPE=$1 \
            -DCMAKE_INSTALL_PREFIX=../${lc_target}/filament \
            -DFILAMENT_ENABLE_JAVA=${FILAMENT_ENABLE_JAVA} \
            ${deployment_target} \
            ../..
    fi
    ${BUILD_COMMAND} ${build_targets}

    if [[ "$INSTALL_COMMAND" ]]; then
        echo "Installing ${lc_target} in out/${lc_target}/filament..."
        ${BUILD_COMMAND} ${INSTALL_COMMAND}
    fi

    if [[ -d "../${lc_target}/filament" ]]; then
        if [[ "$ISSUE_ARCHIVES" == "true" ]]; then
            echo "Generating out/filament-${lc_target}-${LC_UNAME}.tgz..."
            cd ../${lc_target}
            tar -czvf ../filament-${lc_target}-${LC_UNAME}.tgz filament
        fi
    fi

    cd ../..
}

function build_desktop {
    if [[ "$ISSUE_DEBUG_BUILD" == "true" ]]; then
        build_desktop_target "Debug" "$1"
    fi

    if [[ "$ISSUE_RELEASE_BUILD" == "true" ]]; then
        build_desktop_target "Release" "$1"
    fi
}

function build_webgl_with_target {
    local lc_target=`echo $1 | tr '[:upper:]' '[:lower:]'`

    echo "Building WebGL $lc_target..."
    mkdir -p out/cmake-webgl-${lc_target}
    cd out/cmake-webgl-${lc_target}

    if [[ ! "$BUILD_TARGETS" ]]; then
        BUILD_TARGETS=${BUILD_CUSTOM_TARGETS}
        ISSUE_CMAKE_ALWAYS=true
    fi

    if [[ ! -d "CMakeFiles" ]] || [[ "$ISSUE_CMAKE_ALWAYS" == "true" ]]; then
        # Apply the emscripten environment within a subshell.
        (
        source ${EMSDK}/emsdk_env.sh
        cmake \
            -G "$BUILD_GENERATOR" \
            -DIMPORT_EXECUTABLES_DIR=out \
            -DCMAKE_TOOLCHAIN_FILE=${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
            -DCMAKE_BUILD_TYPE=$1 \
            -DCMAKE_INSTALL_PREFIX=../webgl-${lc_target}/filament \
            -DWEBGL=1 \
            ${JS_DOCS_OPTION} \
            ../..
        ${BUILD_COMMAND} ${BUILD_TARGETS}
        )
    fi

    if [[ -d "web/filament-js" ]]; then

        if [[ "$BUILD_JS_DOCS" == "true" ]]; then
            echo "Generating JavaScript documentation..."
            local DOCS_FOLDER="web/docs"
            local DOCS_SCRIPT="../../web/docs/build.py"
            python3 ${DOCS_SCRIPT} --disable-demo \
                --output-folder ${DOCS_FOLDER} \
                --build-folder ${PWD}
        fi

        if [[ "$ISSUE_ARCHIVES" == "true" ]]; then
            echo "Generating out/filament-${lc_target}-web.tgz..."
            # The web archive has the following subfolders:
            # dist...core WASM module and accompanying JS file.
            # docs...HTML tutorials for the JS API, accompanying demos, and a reference page.
            cd web
            tar -cvf ../../filament-${lc_target}-web.tar -s /^filament-js/dist/ \
                    filament-js/filament.js
            tar -rvf ../../filament-${lc_target}-web.tar -s /^filament-js/dist/ \
                    filament-js/filament.wasm
            cd -
            gzip -c ../filament-${lc_target}-web.tar > ../filament-${lc_target}-web.tgz
            rm ../filament-${lc_target}-web.tar
        fi
    fi

    cd ../..
}

function build_webgl {
    # For the host tools, supress install and always use Release.
    local old_install_command=${INSTALL_COMMAND}; INSTALL_COMMAND=
    local old_issue_debug_build=${ISSUE_DEBUG_BUILD}; ISSUE_DEBUG_BUILD=false
    local old_issue_release_build=${ISSUE_RELEASE_BUILD}; ISSUE_RELEASE_BUILD=true

    build_desktop "${WEB_HOST_TOOLS}"

    INSTALL_COMMAND=${old_install_command}
    ISSUE_DEBUG_BUILD=${old_issue_debug_build}
    ISSUE_RELEASE_BUILD=${old_issue_release_build}

    if [[ "$ISSUE_DEBUG_BUILD" == "true" ]]; then
        build_webgl_with_target "Debug"
    fi

    if [[ "$ISSUE_RELEASE_BUILD" == "true" ]]; then
        build_webgl_with_target "Release"
    fi
}

function build_android_target {
    local LC_TARGET=`echo $1 | tr '[:upper:]' '[:lower:]'`
    local ARCH=$2

    echo "Building Android $LC_TARGET ($ARCH)..."
    mkdir -p out/cmake-android-${LC_TARGET}-${ARCH}

    cd out/cmake-android-${LC_TARGET}-${ARCH}

    if [[ ! -d "CMakeFiles" ]] || [[ "$ISSUE_CMAKE_ALWAYS" == "true" ]]; then
        cmake \
            -G "$BUILD_GENERATOR" \
            -DIMPORT_EXECUTABLES_DIR=out \
            -DCMAKE_BUILD_TYPE=$1 \
            -DCMAKE_INSTALL_PREFIX=../android-${LC_TARGET}/filament \
            -DCMAKE_TOOLCHAIN_FILE=../../build/toolchain-${ARCH}-linux-android.cmake \
            ${VULKAN_ANDROID_OPTION} \
            ../..
    fi

    # We must always install Android libraries to build the AAR
    ${BUILD_COMMAND} install

    cd ../..
}

function build_android_arch {
    local arch=$1
    local arch_name=$2

    if [[ "$ISSUE_DEBUG_BUILD" == "true" ]]; then
        build_android_target "Debug" "$arch"
    fi

    if [[ "$ISSUE_RELEASE_BUILD" == "true" ]]; then
        build_android_target "Release" "$arch"
    fi
}

function archive_android {
    local lc_target=`echo $1 | tr '[:upper:]' '[:lower:]'`

    if [[ -d "out/android-${lc_target}/filament" ]]; then
        if [[ "$ISSUE_ARCHIVES" == "true" ]]; then
            echo "Generating out/filament-android-${lc_target}-${LC_UNAME}.tgz..."
            cd out/android-${lc_target}
            tar -czvf ../filament-android-${lc_target}-${LC_UNAME}.tgz filament
            cd ../..
        fi
    fi
}

function ensure_android_build {
    if [[ "$ANDROID_HOME" == "" ]]; then
        echo "Error: ANDROID_HOME is not set, exiting"
        exit 1
    fi

    local ndk_side_by_side="${ANDROID_HOME}/ndk/"
    if [[ -d $ndk_side_by_side ]]; then
        local ndk_version=`ls ${ndk_side_by_side} | sort -V | tail -n 1 | cut -f 1 -d "."`
        if [[ ${ndk_version} -lt ${ANDROID_NDK_VERSION} ]]; then
            echo "Error: Android NDK side-by-side version ${ANDROID_NDK_VERSION} or higher must be installed, exiting"
            exit 1
        fi
    else
        echo "Error: Android NDK side-by-side version ${ANDROID_NDK_VERSION} or higher must be installed, exiting"
        exit 1
    fi

    local cmake_version=`cmake --version`
    if [[ "$cmake_version" =~ ([0-9]+)\.([0-9]+)\.[0-9]+ ]]; then
        if [[ "${BASH_REMATCH[1]}" -lt "${CMAKE_MAJOR}" ]] || \
           [[ "${BASH_REMATCH[2]}" -lt "${CMAKE_MINOR}" ]]; then
            echo "Error: cmake version ${CMAKE_MAJOR}.${CMAKE_MINOR}+ is required," \
                 "${BASH_REMATCH[1]}.${BASH_REMATCH[2]} installed, exiting"
            exit 1
        fi
    fi
}

function build_android {
    ensure_android_build

    # Supress intermediate desktop tools install
    local old_install_command=${INSTALL_COMMAND}
    INSTALL_COMMAND=

    build_desktop "${MOBILE_HOST_TOOLS}"

    INSTALL_COMMAND=${old_install_command}

    build_android_arch "aarch64" "aarch64-linux-android"
    build_android_arch "arm7" "arm-linux-androideabi"
    build_android_arch "x86_64" "x86_64-linux-android"
    build_android_arch "x86" "i686-linux-android"

    if [[ "$ISSUE_DEBUG_BUILD" == "true" ]]; then
        archive_android "Debug"
    fi

    if [[ "$ISSUE_RELEASE_BUILD" == "true" ]]; then
        archive_android "Release"
    fi

    cd android

    if [[ "$ISSUE_DEBUG_BUILD" == "true" ]]; then
        ./gradlew \
            -Pfilament_dist_dir=../out/android-debug/filament \
            ${VULKAN_ANDROID_GRADLE_OPTION} \
            :filament-android:assembleDebug \
            :gltfio-android:assembleDebug \
            :filament-utils-android:assembleDebug

        ./gradlew \
            -Pfilament_dist_dir=../out/android-debug/filament \
            :filamat-android:assembleDebug

        if [[ "$INSTALL_COMMAND" ]]; then
            echo "Installing out/filamat-android-debug.aar..."
            cp filamat-android/build/outputs/aar/filamat-android-full-debug.aar ../out/
            cp filamat-android/build/outputs/aar/filamat-android-lite-debug.aar ../out/

            echo "Installing out/filament-android-debug.aar..."
            cp filament-android/build/outputs/aar/filament-android-debug.aar ../out/

            echo "Installing out/gltfio-android-*-debug.aar..."
            cp gltfio-android/build/outputs/aar/gltfio-android-lite-debug.aar ../out/
            cp gltfio-android/build/outputs/aar/gltfio-android-full-debug.aar ../out/gltfio-android-debug.aar

            echo "Installing out/filament-utils-android-debug.aar..."
            cp filament-utils-android/build/outputs/aar/filament-utils-android-debug.aar ../out/
        fi
    fi

    if [[ "$ISSUE_RELEASE_BUILD" == "true" ]]; then
        ./gradlew \
            -Pfilament_dist_dir=../out/android-release/filament \
            ${VULKAN_ANDROID_GRADLE_OPTION} \
            :filament-android:assembleRelease \
            :gltfio-android:assembleRelease \
            :filament-utils-android:assembleRelease

        ./gradlew \
            -Pfilament_dist_dir=../out/android-release/filament \
            :filamat-android:assembleRelease

        if [[ "$INSTALL_COMMAND" ]]; then
            echo "Installing out/filamat-android-release.aar..."
            cp filamat-android/build/outputs/aar/filamat-android-full-release.aar ../out/
            cp filamat-android/build/outputs/aar/filamat-android-lite-release.aar ../out/

            echo "Installing out/filament-android-release.aar..."
            cp filament-android/build/outputs/aar/filament-android-release.aar ../out/

            echo "Installing out/gltfio-android-*-release.aar..."
            cp gltfio-android/build/outputs/aar/gltfio-android-lite-release.aar ../out/
            cp gltfio-android/build/outputs/aar/gltfio-android-full-release.aar ../out/gltfio-android-release.aar

            echo "Installing out/filament-utils-android-release.aar..."
            cp filament-utils-android/build/outputs/aar/filament-utils-android-release.aar ../out/
        fi
    fi

    cd ..
}

function ensure_ios_toolchain {
    local toolchain_path="build/toolchain-mac-ios.cmake"
    if [[ -e ${toolchain_path} ]]; then
        echo "iOS toolchain file exists."
        return 0
    fi

    echo
    echo "iOS toolchain file does not exist."
    echo "It will automatically be downloaded from http://opensource.apple.com."

    if [[ "$GITHUB_WORKFLOW" ]]; then
        REPLY=y
    else
        read -p "Continue? (y/n) " -n 1 -r
        echo
    fi

    if [[ ! "$REPLY" =~ ^[Yy]$ ]]; then
        echo "Toolchain file must be downloaded to continue."
        exit 1
    fi

    curl -o "${toolchain_path}" "${IOS_TOOLCHAIN_URL}" || {
        echo "Error downloading iOS toolchain file."
        exit 1
    }

    # Apple's toolchain hard-codes the PLATFORM_NAME into the toolchain file. Instead, make this a
    # CACHE variable that can be overriden on the command line.
    local FIND='SET(PLATFORM_NAME iphoneos)'
    local REPLACE='SET(PLATFORM_NAME "iphoneos" CACHE STRING "iOS platform to build for")'
    sed -i '' "s/${FIND}/${REPLACE}/g" ./${toolchain_path}

    # Apple's toolchain specifies isysroot based on an environment variable, which we don't set.
    # The toolchain doesn't need to do this, however, as isysroot is implicitly set in the toolchain
    # via CMAKE_OSX_SYSROOT.
    local FIND='SET(IOS_COMMON_FLAGS "-isysroot $ENV{SDKROOT} '
    local REPLACE='SET(IOS_COMMON_FLAGS "'
    sed -i '' "s/${FIND}/${REPLACE}/g" ./${toolchain_path}

    # Prepend Filament-specific settings.
    (cat build/toolchain-mac-ios.filament.cmake; cat ${toolchain_path}) > tmp && mv tmp ${toolchain_path}

    echo "Successfully downloaded iOS toolchain file and prepended Filament-specific settings."
}

function build_ios_target {
    local lc_target=`echo $1 | tr '[:upper:]' '[:lower:]'`
    local arch=$2
    local platform=$3

    echo "Building iOS $lc_target ($arch) for $platform..."
    mkdir -p out/cmake-ios-${lc_target}-${arch}

    cd out/cmake-ios-${lc_target}-${arch}

    if [[ ! -d "CMakeFiles" ]] || [[ "$ISSUE_CMAKE_ALWAYS" == "true" ]]; then
        cmake \
            -G "$BUILD_GENERATOR" \
            -DIMPORT_EXECUTABLES_DIR=out \
            -DCMAKE_BUILD_TYPE=$1 \
            -DCMAKE_INSTALL_PREFIX=../ios-${lc_target}/filament \
            -DIOS_ARCH=${arch} \
            -DPLATFORM_NAME=${platform} \
            -DIOS_MIN_TARGET=12.0 \
            -DIOS=1 \
            -DCMAKE_TOOLCHAIN_FILE=../../build/toolchain-mac-ios.cmake \
            ../..
    fi

    ${BUILD_COMMAND}

    if [[ "$INSTALL_COMMAND" ]]; then
        echo "Installing ${lc_target} in out/${lc_target}/filament..."
        ${BUILD_COMMAND} ${INSTALL_COMMAND}
    fi

    if [[ -d "../ios-${lc_target}/filament" ]]; then
        if [[ "$ISSUE_ARCHIVES" == "true" ]]; then
            echo "Generating out/filament-${lc_target}-ios.tgz..."
            cd ../ios-${lc_target}
            tar -czvf ../filament-${lc_target}-ios.tgz filament
        fi
    fi

    cd ../..
}

function build_ios {
    # Supress intermediate desktop tools install
    local old_install_command=${INSTALL_COMMAND}
    INSTALL_COMMAND=

    build_desktop "${MOBILE_HOST_TOOLS}"

    INSTALL_COMMAND=${old_install_command}

    ensure_ios_toolchain

    # In theory, we could support iPhone architectures older than arm64, but
    # only arm64 devices support OpenGL 3.0 / Metal

    if [[ "$ISSUE_DEBUG_BUILD" == "true" ]]; then
        build_ios_target "Debug" "arm64" "iphoneos"
        if [[ "$IOS_BUILD_SIMULATOR" == "true" ]]; then
            build_ios_target "Debug" "x86_64" "iphonesimulator"
        fi
    fi

    if [[ "$ISSUE_RELEASE_BUILD" == "true" ]]; then
        build_ios_target "Release" "arm64" "iphoneos"
        if [[ "$IOS_BUILD_SIMULATOR" == "true" ]]; then
            build_ios_target "Release" "x86_64" "iphonesimulator"
        fi
    fi
}

function build_web_docs {
    echo "Building Web documents..."

    mkdir -p out/web-docs
    cd out/web-docs

    # Create an empty npm package to link markdeep-rasterizer into
    npm list | grep web-docs@1.0.0 > /dev/null || npm init --yes > /dev/null
    npm list | grep markdeep-rasterizer > /dev/null || npm install ../../third_party/markdeep-rasterizer > /dev/null

    # Generate documents
    npx markdeep-rasterizer ../../docs/Filament.md.html ../../docs/Materials.md.html  ../../docs/

    cd ../..
}

function validate_build_command {
    set +e
    # Make sure CMake is installed
    local cmake_binary=`which cmake`
    if [[ ! "$cmake_binary" ]]; then
        echo "Error: could not find cmake, exiting"
        exit 1
    fi

    # Make sure Ninja is installed
    if [[ "$BUILD_COMMAND" == "ninja" ]]; then
        local ninja_binary=`which ninja`
        if [[ ! "$ninja_binary" ]]; then
            echo "Warning: could not find ninja, using make instead"
            BUILD_GENERATOR="Unix Makefiles"
            BUILD_COMMAND="make"
        fi
    fi
    # Make sure Make is installed
    if [[ "$BUILD_COMMAND" == "make" ]]; then
        local make_binary=`which make`
        if [[ ! "$make_binary" ]]; then
            echo "Error: could not find make, exiting"
            exit 1
        fi
    fi
    # Make sure we have Java
    local javac_binary=`which javac`
    if [[ "$JAVA_HOME" == "" ]] || [[ ! "$javac_binary" ]]; then
        echo "Warning: JAVA_HOME is not set, skipping Java projects"
        FILAMENT_ENABLE_JAVA=OFF
    fi
    # If building a WebAssembly module, ensure we know where Emscripten lives.
    if [[ "$EMSDK" == "" ]] && [[ "$ISSUE_WEBGL_BUILD" == "true" ]]; then
        echo "Error: EMSDK is not set, exiting"
        exit 1
    fi
    # Web documents require node and npm for processing
    if [[ "$ISSUE_WEB_DOCS" == "true" ]]; then
        local node_binary=`which node`
        local npm_binary=`which npm`
        local npx_binary=`which npx`
        if [[ ! "$node_binary" ]] || [[ ! "$npm_binary" ]] || [[ ! "$npx_binary" ]]; then
            echo "Error: Web documents require node, npm and npx to be installed"
            exit 1
        fi
    fi
    set -e
}

function run_test {
    local test=$1
    # The input string might contain arguments, so we use "set -- $test" to replace $1 with the
    # first whitespace-separated token in the string.
    set -- ${test}
    local test_name=`basename $1`
    ./out/cmake-debug/${test} --gtest_output="xml:out/test-results/$test_name/sponge_log.xml"
}

function run_tests {
    if [[ "$ISSUE_WEBGL_BUILD" == "true" ]]; then
        if ! echo "TypeScript `tsc --version`" ; then
            tsc --noEmit \
                third_party/gl-matrix/gl-matrix.d.ts \
                web/filament-js/filament.d.ts \
                web/filament-js/test.ts
        fi
    else
        while read test; do
            run_test "$test"
        done < build/common/test_list.txt
    fi
}

# Beginning of the script

pushd `dirname $0` > /dev/null

while getopts ":hacfijmp:tuvslw" opt; do
    case ${opt} in
        h)
            print_help
            exit 1
            ;;
        a)
            ISSUE_ARCHIVES=true
            INSTALL_COMMAND=install
            ;;
        c)
            ISSUE_CLEAN=true
            ;;
        f)
            ISSUE_CMAKE_ALWAYS=true
            ;;
        i)
            INSTALL_COMMAND=install
            ;;
        j)
            FILAMENT_ENABLE_JAVA=OFF
            ;;
        m)
            BUILD_GENERATOR="Unix Makefiles"
            BUILD_COMMAND="make"
            ;;
        p)
            ISSUE_DESKTOP_BUILD=false
            platforms=$(echo "$OPTARG" | tr ',' '\n')
            for platform in ${platforms}
            do
                case ${platform} in
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
                esac
            done
            ;;
        u)
            ISSUE_DEBUG_BUILD=true
            RUN_TESTS=true
            ;;
        v)
            VULKAN_ANDROID_OPTION="-DFILAMENT_SUPPORTS_VULKAN=ON"
            VULKAN_ANDROID_GRADLE_OPTION="-Pfilament_supports_vulkan"
            echo "Enabling support for Vulkan in the core Filament library."
            echo ""
            ;;
        s)
            IOS_BUILD_SIMULATOR=true
            echo "iOS simulator support enabled."
            ;;
        w)
            ISSUE_WEB_DOCS=true
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            echo ""
            print_help
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument." >&2
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

shift $(($OPTIND - 1))

for arg; do
    if [[ "$arg" == "release" ]]; then
        ISSUE_RELEASE_BUILD=true
    elif [[ "$arg" == "debug" ]]; then
        ISSUE_DEBUG_BUILD=true
    else
        BUILD_CUSTOM_TARGETS="$BUILD_CUSTOM_TARGETS $arg"
    fi
done

validate_build_command

if [[ "$ISSUE_CLEAN" == "true" ]]; then
    build_clean
fi

if [[ "$ISSUE_DESKTOP_BUILD" == "true" ]]; then
    build_desktop
fi

if [[ "$ISSUE_ANDROID_BUILD" == "true" ]]; then
    build_android
fi

if [[ "$ISSUE_IOS_BUILD" == "true" ]]; then
    build_ios
fi

if [[ "$ISSUE_WEBGL_BUILD" == "true" ]]; then
    build_webgl
fi

if [[ "$ISSUE_WEB_DOCS" == "true" ]]; then
    build_web_docs
fi

if [[ "$RUN_TESTS" == "true" ]]; then
    run_tests
fi
