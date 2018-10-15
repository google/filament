#!/bin/bash
set -e

# NDK API level
API_LEVEL=24
# Host tools required by Android and WebGL builds
HOST_TOOLS="matc cmgen filamesh mipgen"

function print_help {
    local SELF_NAME=`basename $0`
    echo "Usage:"
    echo "    $SELF_NAME [options] <build_type1> [<build_type2> ...] [targets]"
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
    echo "    -p [desktop|android|webgl|all]"
    echo "        Platform(s) to build, defaults to desktop."
    echo "        Building android will automatically generate the toolchains if needed and"
    echo "        perform a partial desktop build."
    echo "    -r"
    echo "        Restrict the number of make/ninja jobs."
    echo "    -t"
    echo "        Generate the Android toolchain, requires \$ANDROID_HOME to be set."
    echo "    -u"
    echo "        Run all unit tests, will trigger a debug build if needed."
    echo "    -v"
    echo "        Add Vulkan support to the Android build."
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
    echo "        \$ ./$SELF_NAME release"
    echo "    Desktop debug and release builds:"
    echo "        \$ ./$SELF_NAME debug release"
    echo "    Clean, desktop debug build and create archive of build artifacts:"
    echo "        \$ ./$SELF_NAME -c -a debug"
    echo "    Android release build type:"
    echo "        \$ ./$SELF_NAME -p android release"
    echo "    Desktop matc target, release build:"
    echo "        \$ ./$SELF_NAME release matc"
    echo ""
 }

# Internal variables
TARGET=release

ISSUE_CLEAN=false

ISSUE_DEBUG_BUILD=false
ISSUE_RELEASE_BUILD=false

ISSUE_ANDROID_BUILD=false
ISSUE_DESKTOP_BUILD=true
ISSUE_WEBGL_BUILD=false

ISSUE_ARCHIVES=false

ISSUE_CMAKE_ALWAYS=false

RUN_TESTS=false

ENABLE_JAVA=ON

INSTALL_COMMAND=

GENERATE_TOOLCHAINS=false

VULKAN_ANDROID_OPTION="-DFILAMENT_SUPPORTS_VULKAN=OFF"

BUILD_GENERATOR=Ninja
BUILD_COMMAND=ninja
BUILD_CUSTOM_TARGETS=

UNAME=`echo $(uname)`
LC_UNAME=`echo ${UNAME} | tr '[:upper:]' '[:lower:]'`

TOOLCHAIN_SCRIPT=${ANDROID_HOME}/ndk-bundle/build/tools/make_standalone_toolchain.py

# Functions

function build_clean {
    echo "Cleaning build directories..."
    rm -Rf out
    rm -Rf android/filament-android/build
}

function generate_toolchain {
    local ARCH=$1
    local ARCH_NAME=$2
    local TOOLCHAIN_NAME=toolchains/${UNAME}/${ARCH_NAME}-4.9

    if [ -z "$ANDROID_HOME" ]; then
        echo "The environment variable \$ANDROID_HOME is not set." >&2
        exit 1
    fi

    if [ ! -f "$TOOLCHAIN_SCRIPT" ]; then
        echo "The NDK is not properly installed in $ANDROID_HOME." >&2
        exit 1
    fi

    if [ -d "$TOOLCHAIN_NAME" ]; then
        echo "Removing existing $ARCH toolchain..."
        rm -Rf ${TOOLCHAIN_NAME}
    fi

    echo "Generating $ARCH toolchain..."

    ${TOOLCHAIN_SCRIPT} \
        --arch ${ARCH} \
        --api ${API_LEVEL} \
        --stl libc++ \
        --force \
        --install-dir ${TOOLCHAIN_NAME}
}

function generate_toolchains {
    generate_toolchain "arm64" "aarch64-linux-android"
    generate_toolchain "arm" "arm-linux-androideabi"
    generate_toolchain "x86_64" "x86_64-linux-android"
    generate_toolchain "x86" "i686-linux-android"
}

function build_desktop_target {
    local LC_TARGET=`echo $1 | tr '[:upper:]' '[:lower:]'`
    local BUILD_TARGETS=$2

    if [ ! "$BUILD_TARGETS" ]; then
        BUILD_TARGETS=${BUILD_CUSTOM_TARGETS}
    fi

    echo "Building $LC_TARGET in out/cmake-${LC_TARGET}..."
    mkdir -p out/cmake-${LC_TARGET}

    cd out/cmake-${LC_TARGET}

    if [ ! -d "CMakeFiles" ] || [ "$ISSUE_CMAKE_ALWAYS" == "true" ]; then
        cmake \
            -G "$BUILD_GENERATOR" \
            -DCMAKE_BUILD_TYPE=$1 \
            -DCMAKE_INSTALL_PREFIX=../${LC_TARGET}/filament \
            -DFILAMENT_REQUIRES_CXXABI=${FILAMENT_REQUIRES_CXXABI} \
            -DENABLE_JAVA=${ENABLE_JAVA} \
            ../..
    fi
    ${BUILD_COMMAND} ${BUILD_TARGETS}

    if [ "$INSTALL_COMMAND" ]; then
        echo "Installing ${LC_TARGET} in out/${LC_TARGET}/filament..."
        ${BUILD_COMMAND} ${INSTALL_COMMAND}
    fi

    if [ -d "../${LC_TARGET}/filament" ]; then
        if [ "$ISSUE_ARCHIVES" == "true" ]; then
            echo "Generating out/filament-${LC_TARGET}-${LC_UNAME}.tgz..."
            cd ../${LC_TARGET}
            tar -czvf ../filament-${LC_TARGET}-${LC_UNAME}.tgz filament
        fi
    fi

    cd ../..
}

function build_desktop {
    if [ "$ISSUE_DEBUG_BUILD" == "true" ]; then
        build_desktop_target "Debug" "$1"
    fi

    if [ "$ISSUE_RELEASE_BUILD" == "true" ]; then
        build_desktop_target "Release" "$1"
    fi
}

function build_webgl_with_target {
    local LC_TARGET=`echo $1 | tr '[:upper:]' '[:lower:]'`
    echo "Building WebGL $LC_TARGET..."
    mkdir -p out/cmake-webgl-${LC_TARGET}
    cd out/cmake-webgl-${LC_TARGET}
    if [ ! "$BUILD_TARGETS" ]; then
        BUILD_TARGETS=${BUILD_CUSTOM_TARGETS}
        ISSUE_CMAKE_ALWAYS=true
    fi
    if [ ! -d "CMakeFiles" ] || [ "$ISSUE_CMAKE_ALWAYS" == "true" ]; then
        source ${EMSDK}/emsdk_env.sh
        cmake \
            -G "$BUILD_GENERATOR" \
            -DCMAKE_TOOLCHAIN_FILE=${EMSCRIPTEN}/cmake/Modules/Platform/Emscripten.cmake \
            -DCMAKE_BUILD_TYPE=$1 \
            -DCMAKE_INSTALL_PREFIX=../webgl-${LC_TARGET}/filament \
            -DWEBGL=1 \
            ../..
        ${BUILD_COMMAND} ${BUILD_TARGETS}
    fi

    if [ -d "samples/web/public" ]; then
        if [ "$ISSUE_ARCHIVES" == "true" ]; then
            echo "Generating out/filament-${LC_TARGET}-web.tgz..."
            cd samples/web
            tar -cvf ../../../filament-${LC_TARGET}-web.tar public
            cd -
            cd libs
            tar -rvf ../../filament-${LC_TARGET}-web.tar filamentjs/filament.js
            tar -rvf ../../filament-${LC_TARGET}-web.tar filamentjs/filament.wasm
            cd -
            gzip -c ../filament-${LC_TARGET}-web.tar > ../filament-${LC_TARGET}-web.tgz
            rm ../filament-${LC_TARGET}-web.tar
        fi
    fi

    cd ../..
}

function build_webgl {
    # For the host tools, supress install and always use Release.
    OLD_INSTALL_COMMAND=${INSTALL_COMMAND}; INSTALL_COMMAND=
    OLD_ISSUE_DEBUG_BUILD=${ISSUE_DEBUG_BUILD}; ISSUE_DEBUG_BUILD=false
    OLD_ISSUE_RELEASE_BUILD=${ISSUE_RELEASE_BUILD}; ISSUE_RELEASE_BUILD=true
    build_desktop "${HOST_TOOLS}"
    INSTALL_COMMAND=${OLD_INSTALL_COMMAND}
    ISSUE_DEBUG_BUILD=${OLD_ISSUE_DEBUG_BUILD}
    ISSUE_RELEASE_BUILD=${OLD_ISSUE_RELEASE_BUILD}

    if [ "$ISSUE_DEBUG_BUILD" == "true" ]; then
        build_webgl_with_target "Debug"
    fi

    if [ "$ISSUE_RELEASE_BUILD" == "true" ]; then
        build_webgl_with_target "Release"
    fi
}

function build_android_target {
    local LC_TARGET=`echo $1 | tr '[:upper:]' '[:lower:]'`
    local ARCH=$2

    echo "Building Android $LC_TARGET ($ARCH)..."
    mkdir -p out/cmake-android-${LC_TARGET}-${ARCH}

    cd out/cmake-android-${LC_TARGET}-${ARCH}

    if [ ! -d "CMakeFiles" ] || [ "$ISSUE_CMAKE_ALWAYS" == "true" ]; then
        cmake \
            -G "$BUILD_GENERATOR" \
            -DCMAKE_BUILD_TYPE=$1 \
            -DCMAKE_INSTALL_PREFIX=../android-${LC_TARGET}/filament \
            -DCMAKE_TOOLCHAIN_FILE=../../build/toolchain-${ARCH}-linux-android.cmake \
            $VULKAN_ANDROID_OPTION \
            ../..
    fi

    # We must always install Android libraries to build the AAR
    ${BUILD_COMMAND} install

    cd ../..
}

function build_android_arch {
    local ARCH=$1
    local ARCH_NAME=$2
    local TOOLCHAIN_ARCH=$3
    local TOOLCHAIN_NAME=toolchains/${UNAME}/${ARCH_NAME}-4.9

    if [ ! -d "$TOOLCHAIN_NAME" ]; then
        generate_toolchain ${TOOLCHAIN_ARCH} ${ARCH_NAME}
    fi

    if [ "$ISSUE_DEBUG_BUILD" == "true" ]; then
        build_android_target "Debug" "$ARCH"
    fi

    if [ "$ISSUE_RELEASE_BUILD" == "true" ]; then
        build_android_target "Release" "$ARCH"
    fi
}

function archive_android {
    local LC_TARGET=`echo $1 | tr '[:upper:]' '[:lower:]'`

    if [ -d "out/android-${LC_TARGET}/filament" ]; then
        if [ "$ISSUE_ARCHIVES" == "true" ]; then
            echo "Generating out/filament-android-${LC_TARGET}-${LC_UNAME}.tgz..."
            cd out/android-${LC_TARGET}
            tar -czvf ../filament-android-${LC_TARGET}-${LC_UNAME}.tgz filament
            cd ../..
        fi
    fi
}

function build_android {
    # Supress intermediate desktop tools install
    OLD_INSTALL_COMMAND=${INSTALL_COMMAND}
    INSTALL_COMMAND=
    build_desktop "${HOST_TOOLS}"
    INSTALL_COMMAND=${OLD_INSTALL_COMMAND}

    build_android_arch "aarch64" "aarch64-linux-android" "arm64"
    build_android_arch "arm7" "arm-linux-androideabi" "arm"
    build_android_arch "x86_64" "x86_64-linux-android" "x86_64"
    build_android_arch "x86" "i686-linux-android" "x86"

    if [ "$ISSUE_DEBUG_BUILD" == "true" ]; then
        archive_android "Debug"
    fi

    if [ "$ISSUE_RELEASE_BUILD" == "true" ]; then
        archive_android "Release"
    fi

    cd android/filament-android

    if [ "$ISSUE_DEBUG_BUILD" == "true" ]; then
        ./gradlew -Pfilament_dist_dir=../../out/android-debug/filament assembleDebug \
            -Pextra_cmake_args=${VULKAN_ANDROID_OPTION}

        if [ "$INSTALL_COMMAND" ]; then
            echo "Installing out/filament-android-debug.aar..."
            cp build/outputs/aar/filament-android-debug.aar ../../out/
        fi
    fi

    if [ "$ISSUE_RELEASE_BUILD" == "true" ]; then
        ./gradlew -Pfilament_dist_dir=../../out/android-release/filament assembleRelease \
            -Pextra_cmake_args=${VULKAN_ANDROID_OPTION}

        if [ "$INSTALL_COMMAND" ]; then
            echo "Installing out/filament-android-release.aar..."
            cp build/outputs/aar/filament-android-release.aar ../../out/
        fi
    fi

    cd ../..
}

function validate_build_command {
    set +e
    # Make sure CMake is installed
    cmake_binary=`which cmake`
    if [ ! "$cmake_binary" ]; then
        echo "Error: could not find cmake, exiting"
        exit 1
    fi

    # Make sure Ninja is installed
    if [ "$BUILD_COMMAND" == "ninja" ]; then
        ninja_binary=`which ninja`
        if [ ! "$ninja_binary" ]; then
            echo "Warning: could not find ninja, using make instead"
            BUILD_GENERATOR="Unix Makefiles"
            BUILD_COMMAND="make"
        fi
    fi
    # Make sure Make is installed
    if [ "$BUILD_COMMAND" == "make" ]; then
        make_binary=`which make`
        if [ ! "$make_binary" ]; then
            echo "Error: could not find make, exiting"
            exit 1
        fi
    fi
    # Make sure we have Java
    javac_binary=`which javac`
    if [ "$JAVA_HOME" == "" ] || [ ! "$javac_binary" ]; then
        echo "Warning: JAVA_HOME is not set, skipping Java projects"
        ENABLE_JAVA=OFF
    fi
    # If building a WebAssembly module, ensure we know where Emscripten lives.
    if [ "$EMSDK" == "" ] && [ "$ISSUE_WEBGL_BUILD" == "true" ]; then
        echo "Error: EMSDK is not set, exiting"
        exit 1
    fi
    set -e
}

function run_test {
    test=$1
    # The input string might contain arguments, so we use "set -- $test" to replace $1 with the
    # first whitespace-separated token in the string.
    set -- $test
    test_name=`basename $1`
    ./out/cmake-debug/${test} --gtest_output="xml:out/test-results/$test_name/sponge_log.xml"
}

function run_tests {
    while read test; do
        run_test "$test"
    done < build/common/test_list.txt
}

# Beginning of the script

pushd `dirname $0` > /dev/null

while getopts ":hacfijmp:tuv" opt; do
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
            ENABLE_JAVA=OFF
            ;;
        m)
            BUILD_GENERATOR="Unix Makefiles"
            BUILD_COMMAND="make"
            ;;
        p)
            case $OPTARG in
                desktop)
                    ISSUE_ANDROID_BUILD=false
                    ISSUE_DESKTOP_BUILD=true
                    ISSUE_WEBGL_BUILD=false
                ;;
                android)
                    ISSUE_ANDROID_BUILD=true
                    ISSUE_DESKTOP_BUILD=false
                    ISSUE_WEBGL_BUILD=false
                ;;
                webgl)
                    ISSUE_ANDROID_BUILD=false
                    ISSUE_DESKTOP_BUILD=false
                    ISSUE_WEBGL_BUILD=true
                ;;
                all)
                    ISSUE_ANDROID_BUILD=true
                    ISSUE_DESKTOP_BUILD=true
                    ISSUE_WEBGL_BUILD=false
                ;;
            esac
            ;;
        t)
            GENERATE_TOOLCHAINS=true
            ;;
        u)
            ISSUE_DEBUG_BUILD=true
            RUN_TESTS=true
            ;;
        v)
            VULKAN_ANDROID_OPTION="-DFILAMENT_SUPPORTS_VULKAN=ON"
            echo "Enabling support for Vulkan in the core Filament library."
            echo ""
            echo "To switch your application to Vulkan, in Android Studio go to "
            echo "File > Settings > Build > Compiler. In the command-line options field, "
            echo "add -Pextra_cmake_args=-DFILAMENT_SUPPORTS_VULKAN=ON."
            echo "Also be sure to pass Backend::VULKAN to Engine::create."
            echo ""
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

if [ "$#" == "0" ]; then
    print_help
    exit 1
fi

shift $(($OPTIND - 1))

for arg; do
    if [ "$arg" == "release" ]; then
        ISSUE_RELEASE_BUILD=true
    elif [ "$arg" == "debug" ]; then
        ISSUE_DEBUG_BUILD=true
    else
        BUILD_CUSTOM_TARGETS="$BUILD_CUSTOM_TARGETS $arg"
    fi
done

validate_build_command

if [ "$ISSUE_CLEAN" == "true" ]; then
    build_clean
fi

if [ "$GENERATE_TOOLCHAINS" == "true" ]; then
    generate_toolchains
fi

if [ "$ISSUE_DESKTOP_BUILD" == "true" ]; then
    build_desktop
fi

if [ "$ISSUE_ANDROID_BUILD" == "true" ]; then
    build_android
fi

if [ "$ISSUE_WEBGL_BUILD" == "true" ]; then
    build_webgl
fi

if [ "$RUN_TESTS" == "true" ]; then
    run_tests
fi
