#!/usr/bin/env python3
# Copyright (c) 2023-2024 Valve Corporation
# Copyright (c) 2023-2024 LunarG, Inc.

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

# NOTE: Android this documentation is crucial for understanding the layout of the NDK.
# https://android.googlesource.com/platform/ndk/+/master/docs/BuildSystemMaintainers.md

# NOTE: Environment variables we can rely on users/environments setting.
# https://github.com/actions/runner-images/blob/main/images/linux/Ubuntu2204-Readme.md#environment-variables-2

import argparse
import os
import sys
import shutil
import common_ci

# Manifest file describing out test application
def get_android_manifest() -> str:
    manifest = common_ci.RepoRelative('tests/android/AndroidManifest.xml')
    if not os.path.isfile(manifest):
        print(f"Unable to find manifest for APK! {manifest}")
        sys.exit(-1)
    return manifest

# Generate the APK from the CMake binaries
def generate_apk(SDK_ROOT : str, CMAKE_INSTALL_DIR : str) -> str:
    apk_dir = common_ci.RepoRelative('build-android/bin')

    # Delete APK directory since it could contain files from old runs
    if os.path.isdir(apk_dir):
        shutil.rmtree(apk_dir)

    shutil.copytree(CMAKE_INSTALL_DIR, apk_dir)

    android_manifest = get_android_manifest()

    # 33 is matching the targetSdkVersion
    android_jar = f"{SDK_ROOT}/platforms/android-33/android.jar"
    if not os.path.isfile(android_jar):
        print(f"Unable to find {android_jar}!")
        sys.exit(-1)

    apk_name = 'VulkanLayerValidationTests'

    unaligned_apk = f'{apk_dir}/{apk_name}-unaligned.apk'
    test_apk = f'{apk_dir}/{apk_name}.apk'

    # Create APK
    common_ci.RunShellCmd(f'aapt package -f -M {android_manifest} -I {android_jar} -F {unaligned_apk} {CMAKE_INSTALL_DIR}')

    # Align APK
    common_ci.RunShellCmd(f'zipalign -f 4 {unaligned_apk} {test_apk}')

    # Create Key (If it doesn't already exist)
    debug_key = common_ci.RepoRelative(f'{apk_dir}/debug.keystore')
    ks_pass = 'android'
    if not os.path.isfile(debug_key):
        dname = 'CN=Android-Debug,O=Android,C=US'
        common_ci.RunShellCmd(f'keytool -genkey -v -keystore {debug_key} -alias androiddebugkey -storepass {ks_pass} -keypass {ks_pass} -keyalg RSA -keysize 2048 -validity 10000 -dname {dname}')

    # Sign APK
    common_ci.RunShellCmd(f'apksigner sign --verbose --ks {debug_key} --ks-pass pass:{ks_pass} {test_apk}')

# Android APKs can contain binaries for multiple ABIs (armeabi-v7a, arm64-v8a, x86, x86_64).
# https://en.wikipedia.org/wiki/Apk_(file_format)#Package_contents
#
# As a result CMake will need to be run multiple times to create a complete test APK that can be run on any Android device.
def main():
    configs = ['Release', 'Debug', 'MinSizeRel']

    parser = argparse.ArgumentParser()
    parser.add_argument('--config', type=str, choices=configs, default=configs[0])
    parser.add_argument('--app-abi', dest='android_abi', type=str, default="arm64-v8a")
    parser.add_argument('--app-stl', dest='android_stl', type=str, choices=["c++_static", "c++_shared"], default="c++_static")
    parser.add_argument('--apk', action='store_true', help='Generate an APK as a post build step.')
    parser.add_argument('--clean', action='store_true', help='Cleans CMake build artifacts')
    args = parser.parse_args()

    cmake_config = args.config
    android_abis = args.android_abi.split(" ")
    android_stl = args.android_stl
    create_apk = args.apk
    clean = args.clean

    if "ANDROID_NDK_HOME" not in os.environ:
        print("Cannot find ANDROID_NDK_HOME!")
        sys.exit(1)

    android_ndk_home = os.environ.get('ANDROID_NDK_HOME')
    android_toolchain = f'{android_ndk_home}/build/cmake/android.toolchain.cmake'

    # The only tool we require for building is CMake/Ninja
    required_cli_tools = ['cmake', 'ninja']

    # If we are building an APK we need a few more tools.
    if create_apk:
        if "ANDROID_SDK_ROOT" not in os.environ:
            print("Cannot find ANDROID_SDK_ROOT!")
            sys.exit(1)

        android_sdk_root = os.environ.get('ANDROID_SDK_ROOT')
        print(f"ANDROID_SDK_ROOT = {android_sdk_root}")
        required_cli_tools += ['aapt', 'zipalign', 'keytool', 'apksigner']

    print(f"ANDROID_NDK_HOME = {android_ndk_home}")
    print(f"Build configured for {cmake_config} | {android_stl} | {android_abis} | APK {create_apk}")

    if not os.path.isfile(android_toolchain):
        print(f'Unable to find android.toolchain.cmake at {android_toolchain}')
        exit(-1)

    for tool in required_cli_tools:
        path = shutil.which(tool)
        if path is None:
            print(f"Unable to find {tool}!")
            exit(-1)

        print(f"Using {tool} : {path}")

    cmake_install_dir = common_ci.RepoRelative('build-android/libs')

    # Delete install directory since it could contain files from old runs
    if os.path.isdir(cmake_install_dir):
        print("Cleaning CMake install")
        shutil.rmtree(cmake_install_dir)

    for abi in android_abis:
        build_dir = common_ci.RepoRelative(f'build-android/cmake/{abi}')
        lib_dir = f'lib/{abi}'

        if clean:
            print("Deleting CMakeCache.txt")

            # Delete CMakeCache.txt to ensure clean builds
            # NOTE: CMake 3.24 has --fresh which would be better to use in the future.
            cmake_cache = f'{build_dir}/CMakeCache.txt'
            if os.path.isfile(cmake_cache):
                os.remove(cmake_cache)

        cmake_cmd =  f'cmake -S . -B {build_dir} -G Ninja'

        cmake_cmd += f' -D CMAKE_BUILD_TYPE={cmake_config}'
        cmake_cmd += f' -D UPDATE_DEPS=ON -D UPDATE_DEPS_DIR={build_dir}'
        cmake_cmd += f' -D CMAKE_TOOLCHAIN_FILE={android_toolchain}'
        cmake_cmd += f' -D CMAKE_ANDROID_ARCH_ABI={abi}'
        cmake_cmd += f' -D CMAKE_INSTALL_LIBDIR={lib_dir}'
        cmake_cmd += f' -D BUILD_TESTS={create_apk}'
        cmake_cmd += f' -D CMAKE_ANDROID_STL_TYPE={android_stl}'

        cmake_cmd += ' -D ANDROID_PLATFORM=26'
        cmake_cmd += ' -D ANDROID_USE_LEGACY_TOOLCHAIN_FILE=NO'

        common_ci.RunShellCmd(cmake_cmd)

        build_cmd = f'cmake --build {build_dir}'
        common_ci.RunShellCmd(build_cmd)

        install_cmd = f'cmake --install {build_dir} --prefix {cmake_install_dir}'
        if cmake_config in ['Release', 'MinSizeRel']:
            install_cmd += f' --strip'
        common_ci.RunShellCmd(install_cmd)

    if create_apk:
        generate_apk(SDK_ROOT = android_sdk_root, CMAKE_INSTALL_DIR = cmake_install_dir)

if __name__ == '__main__':
    main()
