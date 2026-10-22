#!/bin/bash
# Copyright (c) 2018 Google LLC.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Linux Build Script.

# Fail on any error.
set -e
# Display commands being run.
set -x

# This is required to run any git command in the docker since owner will
# have changed between the clone environment, and the docker container.
# Mark all repositories as safe for ownership changes.
git config --global --add safe.directory '*'

. /bin/using.sh # Declare the bash `using` function for configuring toolchains.

using python-3.12

if [ $COMPILER = "clang" ]; then
  using clang-18
elif [ $COMPILER = "gcc" ]; then
  using gcc-15
fi

cd $ROOT_DIR

function clean_dir() {
  dir=$1
  if [[ -d "$dir" ]]; then
    rm -fr "$dir"
  fi
  mkdir -p "$dir"
}

case $TOOL in
  cmake-shaderc-smoketest|cmake-dxc-smoketest|gn)
    ;;
  *)
    # Get source for dependencies, as specified in the DEPS file
    python3 utils/git-sync-deps --prefix=external --treeless
esac


if [ $TOOL = "cmake" ]; then
  using cmake-3.31.2
  using ninja-1.10.0

  # Possible configurations are:
  # ASAN, UBSAN, COVERAGE, RELEASE, DEBUG, DEBUG_EXCEPTION, RELEASE_MINGW
  BUILD_TYPE="Debug"
  if [ $CONFIG = "RELEASE" ] || [ $CONFIG = "RELEASE_MINGW" ]; then
    BUILD_TYPE="Release"
  fi

  SKIP_TESTS="False"
  ADDITIONAL_CMAKE_FLAGS=""
  if [ $CONFIG = "ASAN" ]; then
    ADDITIONAL_CMAKE_FLAGS="-DSPIRV_USE_SANITIZER=address,bounds,null"
    [ $COMPILER = "clang" ] || { echo "$CONFIG requires clang"; exit 1; }
  elif [ $CONFIG = "UBSAN" ]; then
    # UBSan requires RTTI, and by default UBSan does not exit when errors are
    # encountered - additional compiler options are required to force this.
    # The -DSPIRV_USE_SANITIZER=undefined option instructs SPIR-V Tools to be
    # built with UBSan enabled.
    ADDITIONAL_CMAKE_FLAGS="-DSPIRV_USE_SANITIZER=undefined -DENABLE_RTTI=ON -DCMAKE_C_FLAGS=-fno-sanitize-recover=all -DCMAKE_CXX_FLAGS=-fno-sanitize-recover=all"
    [ $COMPILER = "clang" ] || { echo "$CONFIG requires clang"; exit 1; }
  elif [ $CONFIG = "COVERAGE" ]; then
    ADDITIONAL_CMAKE_FLAGS="-DENABLE_CODE_COVERAGE=ON"
    SKIP_TESTS="True"
  elif [ $CONFIG = "DEBUG_EXCEPTION" ]; then
    ADDITIONAL_CMAKE_FLAGS="-DDISABLE_EXCEPTIONS=ON -DDISABLE_RTTI=ON"
  elif [ $CONFIG = "RELEASE_MINGW" ]; then
    ADDITIONAL_CMAKE_FLAGS="-Dgtest_disable_pthreads=ON -DCMAKE_TOOLCHAIN_FILE=$SRC/cmake/linux-mingw-toolchain.cmake"
    SKIP_TESTS="True"
  fi

  # Build fuzzers on selected configurations.
  if [ $COMPILER-$CONFIG = "clang-RELEASE" ]; then
    # Build targets that fuzz the assembler, binary parser, disassembler,
    # optimizer, and validator.
    ADDITIONAL_CMAKE_FLAGS="$ADDITIONAL_CMAKE_FLAGS -DSPIRV_BUILD_LIBFUZZER_TARGETS=ON"
    # Build spirv-fuzz, including its protobuf dependency
    ADDITIONAL_CMAKE_FLAGS="$ADDITIONAL_CMAKE_FLAGS -DSPIRV_BUILD_FUZZER=ON"
  fi

  clean_dir "$ROOT_DIR/build"
  cd "$ROOT_DIR/build"

  # Invoke the build.
  BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}
  echo $(date): Starting build...
  cmake -DPYTHON_EXECUTABLE:FILEPATH=/usr/bin/python3 -GNinja -DCMAKE_INSTALL_PREFIX=$KOKORO_ARTIFACTS_DIR/install -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DRE2_BUILD_TESTING=OFF -DSPIRV_BUILD_FUZZER=ON $ADDITIONAL_CMAKE_FLAGS ..

  echo $(date): Build everything...
  ninja
  echo $(date): Build completed.

  if [ $CONFIG = "COVERAGE" ]; then
    echo $(date): Check coverage...
    ninja report-coverage
    echo $(date): Check coverage completed.
  fi

  echo $(date): Starting ctest...
  if [ $SKIP_TESTS = "False" ]; then
    ctest -j4 --output-on-failure --timeout 300
  fi
  echo $(date): ctest completed.

  # Package the build.
  ninja install
  cd $KOKORO_ARTIFACTS_DIR
  tar czf install.tgz install
elif [ $TOOL = "cmake-shaderc-smoketest" ]; then
  using cmake-3.31.2
  using ninja-1.10.0

  # Get shaderc.
  SHADERC_DIR=/tmp/shaderc
  clean_dir "$SHADERC_DIR"
  cd $SHADERC_DIR
  git clone https://github.com/google/shaderc.git .
  cd $SHADERC_DIR/third_party

  # Get shaderc dependencies. Link the appropriate SPIRV-Tools.
  git clone https://github.com/google/googletest.git
  git clone https://github.com/KhronosGroup/glslang.git
  ln -s $ROOT_DIR spirv-tools
  git clone https://github.com/KhronosGroup/SPIRV-Headers.git spirv-headers
  git clone https://github.com/google/re2
  git clone https://github.com/google/effcee
  git clone https://github.com/abseil/abseil-cpp abseil_cpp

  cd $SHADERC_DIR
  mkdir build
  cd $SHADERC_DIR/build

  # Invoke the build.
  echo $(date): Starting build...
  cmake -GNinja -DRE2_BUILD_TESTING=OFF -DCMAKE_BUILD_TYPE="Release" ..

  echo $(date): Build glslang...
  ninja glslang-standalone

  echo $(date): Build everything...
  ninja
  echo $(date): Build completed.

  echo $(date): Check Shaderc for copyright notices...
  ninja check-copyright

  echo $(date): Starting ctest...
  ctest --output-on-failure -j4
  echo $(date): ctest completed.
elif [ $TOOL = "cmake-dxc-smoketest" ]; then
  using cmake-3.31.2
  using ninja-1.10.0

  # Get shaderc.
  DXC_DIR=/tmp/dxc
  clean_dir "$DXC_DIR"
  cd $DXC_DIR
  git clone https://github.com/microsoft/DirectXShaderCompiler.git .
  cd $DXC_DIR/external

  # Get DXC dependencies. Link the appropriate SPIRV-Tools.
  git submodule update --init DirectX-Headers
  rm -rf SPIRV-Tools
  ln -s $ROOT_DIR SPIRV-Tools
  git clone https://github.com/KhronosGroup/SPIRV-Headers.git SPIRV-Headers

  cd $DXC_DIR
  mkdir build
  cd $DXC_DIR/build

  # Invoke the build.
  echo $(date): Configuring build...
  cmake $DXC_DIR \
  -C $DXC_DIR/cmake/caches/PredefinedParams.cmake \
  -DCMAKE_BUILD_TYPE="Release" \
  -G Ninja

  echo $(date): Building ClangSPIRVTests...
  ninja ClangSPIRVTests

  echo $(date): Testing ClangSPIRVTests...
  tools/clang/unittests/SPIRV/ClangSPIRVTests

  echo $(date): Testing check-clang-codegenspirv...
  ninja check-clang-codegenspirv

elif [ $TOOL = "cmake-android-ndk" ]; then
  using cmake-3.31.2
  using ndk-r29
  using ninja-1.10.0

  clean_dir "$ROOT_DIR/build"
  cd "$ROOT_DIR/build"

  echo $(date): Starting build...
  cmake -DCMAKE_BUILD_TYPE=Release \
        -DANDROID_NATIVE_API_LEVEL=android-24 \
        -DANDROID_ABI="armeabi-v7a with NEON" \
        -DSPIRV_SKIP_TESTS=ON \
        -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
        -GNinja \
        -DANDROID_NDK=$ANDROID_NDK \
        ..

  echo $(date): Build everything...
  ninja
  echo $(date): Build completed.
elif [ $TOOL = "android-ndk-build" ]; then
  using ndk-r29

  clean_dir "$ROOT_DIR/build"
  cd "$ROOT_DIR/build"

  echo $(date): Starting ndk-build ...
  $ANDROID_NDK_HOME/ndk-build \
    -C $ROOT_DIR/android_test \
    NDK_PROJECT_PATH=. \
    NDK_LIBS_OUT=./libs \
    NDK_APP_OUT=./app \
    -j4

  echo $(date): ndk-build completed.
elif [ $TOOL = "bazel" ]; then
  using bazel-8.7.0

  echo $(date): Build everything...
  bazel build --cxxopt=-std=c++17 :all
  echo $(date): Build completed.

  echo $(date): Starting bazel test...
  bazel test --cxxopt=-std=c++17 :all
  echo $(date): Bazel test completed.

elif [ $TOOL = "gn" ]; then
  using ninja-1.10.0
  echo $(date -Iseconds): Start GN build...

  echo "$(date -Iseconds): Fetching depot_tools..."
  rm -rf /tmp/depot_tools
  mkdir -p /tmp/depot_tools
  git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git /tmp/depot_tools
  export PATH="/tmp/depot_tools:$PATH"

  echo "$(date -Iseconds): Syncing client..."
  # Silence gn related babble
  export GCLIENT_SUPPRESS_GIT_VERSION_WARNING=1
  # For the 'root' user, silence gclient metrics collection.
  if [[ -z "$USER" ]]; then
    mkdir -p $HOME/.config/depot_tools
    echo  >$HOME/.config/depot_tools/metrics.cfg '{"is-googler": false, "countdown": 0, "opt-in": false, "version": 1}'
  fi

  # Erase the GN args from any previous run of gclient.
  # This is important for local testing.
  rm -f build/config/gclient_args.gni

  # Sync dependencies and generate default GN args from the DEPS file.
  cp utils/standalone.gclient .
  gclient sync -v -D --gclientfile=standalone.gclient

  echo "$(date -Iseconds): Generate Ninja build plan..."

  # Ensure the gn binary is on the path
  export PATH=$(pwd)/buildtools/linux64:$PATH
  # which -a gn  # There should be two

  if [ $CONFIG = "RELEASE" ]; then
    arg="is_debug=false"
    BUILD_DIR=out/release
  else
    arg="is_debug=true"
    BUILD_DIR=out/debug
  fi
  clean_dir "$BUILD_DIR"
  gn gen "$BUILD_DIR" --args="$arg"

  echo "$(date -Iseconds): Building..."
  ninja -v -C "$BUILD_DIR"
  echo "$(date -Iseconds): Done"

else
  echo "Unknown TOOL '$TOOL'"
fi
