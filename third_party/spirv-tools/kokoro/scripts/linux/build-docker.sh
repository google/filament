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

. /bin/using.sh # Declare the bash `using` function for configuring toolchains.

if [ $COMPILER = "clang" ]; then
  using clang-10.0.0
elif [ $COMPILER = "gcc" ]; then
  using gcc-9
fi

cd $ROOT_DIR

function clone_if_missing() {
  url=$1
  dir=$2
  if [[ ! -d "$dir" ]]; then
    git clone ${@:3} "$url" "$dir"
  fi
}

function clean_dir() {
  dir=$1
  if [[ -d "$dir" ]]; then
    rm -fr "$dir"
  fi
  mkdir "$dir"
}

clone_if_missing https://github.com/KhronosGroup/SPIRV-Headers external/spirv-headers --depth=1
clone_if_missing https://github.com/google/googletest          external/googletest
pushd external/googletest; git reset --hard 1fb1bb23bb8418dc73a5a9a82bbed31dc610fec7; popd
clone_if_missing https://github.com/google/effcee              external/effcee        --depth=1
clone_if_missing https://github.com/google/re2                 external/re2           --depth=1
clone_if_missing https://github.com/protocolbuffers/protobuf   external/protobuf      --branch v3.13.0.1

if [ $TOOL = "cmake" ]; then
  using cmake-3.17.2
  using ninja-1.10.0

  # Possible configurations are:
  # ASAN, UBSAN, COVERAGE, RELEASE, DEBUG, DEBUG_EXCEPTION, RELEASE_MINGW
  BUILD_TYPE="Debug"
  if [ $CONFIG = "RELEASE" ] || [ $CONFIG = "RELEASE_MINGW" ]; then
    BUILD_TYPE="RelWithDebInfo"
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

  if [ $COMPILER = "clang" ]; then
    ADDITIONAL_CMAKE_FLAGS="$ADDITIONAL_CMAKE_FLAGS -DSPIRV_BUILD_LIBFUZZER_TARGETS=ON"
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
elif [ $TOOL = "cmake-smoketest" ]; then
  using cmake-3.17.2
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

  cd $SHADERC_DIR
  mkdir build
  cd $SHADERC_DIR/build

  # Invoke the build.
  echo $(date): Starting build...
  cmake -GNinja -DRE2_BUILD_TESTING=OFF -DCMAKE_BUILD_TYPE="Release" ..

  echo $(date): Build glslang...
  ninja glslangValidator

  echo $(date): Build everything...
  ninja
  echo $(date): Build completed.

  echo $(date): Check Shaderc for copyright notices...
  ninja check-copyright

  echo $(date): Starting ctest...
  ctest --output-on-failure -j4
  echo $(date): ctest completed.
elif [ $TOOL = "cmake-android-ndk" ]; then
  using cmake-3.17.2
  using ndk-r21d
  using ninja-1.10.0

  clean_dir "$ROOT_DIR/build"
  cd "$ROOT_DIR/build"

  echo $(date): Starting build...
  cmake -DCMAKE_BUILD_TYPE=Release \
        -DANDROID_NATIVE_API_LEVEL=android-16 \
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
  using ndk-r21d

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
  using bazel-3.1.0

  echo $(date): Build everything...
  bazel build :all
  echo $(date): Build completed.

  echo $(date): Starting bazel test...
  bazel test :all
  echo $(date): Bazel test completed.
fi
