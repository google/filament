#!/bin/bash

#  Copyright The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.
#
# Generates a roll CL within the ANGLE repository of AOSP.
#
# WARNING: Running this script without args may mess up the checkout.
#   See --genAndroidBp for testing just the code generation.

# exit when any command fails
set -eE -o functrace

failure() {
  local lineno=$1
  local msg=$2
  echo "Failed at $lineno: $msg"
}
trap 'failure ${LINENO} "$BASH_COMMAND"' ERR

# Change the working directory to the ANGLE root directory
cd "${0%/*}/.."

GN_OUTPUT_DIRECTORY=out/Android

function generate_Android_bp_file() {
    abis=(
        "arm"
        "arm64"
        "x86"
        "x64"
    )

    for abi in "${abis[@]}"; do
        # generate gn build files and convert them to blueprints
        gn_args=(
            "target_os = \"android\""
            "is_component_build = false"
            "is_debug = false"
            "dcheck_always_on = false"
            "symbol_level = 0"
            "angle_standalone = false"
            "angle_build_all = false"

            # Build for 64-bit CPUs
            "target_cpu = \"$abi\""

            # Target ndk API 26 to make sure ANGLE can use the Vulkan backend on Android
            "android32_ndk_api_level = 26"
            "android64_ndk_api_level = 26"

            # Disable all backends except Vulkan
            "angle_enable_vulkan = true"
            "angle_enable_gl = false"
            "angle_enable_d3d9 = false"
            "angle_enable_d3d11 = false"
            "angle_enable_null = false"
            "angle_enable_metal = false"
            "angle_enable_wgpu = false"

            # SwiftShader is loaded as the system Vulkan driver on Android, not compiled by ANGLE
            "angle_enable_swiftshader = false"

            # Disable all shader translator targets except desktop GL (for Vulkan)
            "angle_enable_essl = false"
            "angle_enable_glsl = false"
            "angle_enable_hlsl = false"

            "angle_enable_commit_id = false"

            # Disable histogram/protobuf support
            "angle_has_histograms = false"

            # Use system lib(std)c++, since the Chromium library breaks std::string
            "use_custom_libcxx = false"

            # rapidJSON is used for ANGLE's frame capture (among other things), which is unnecessary for AOSP builds.
            "angle_has_rapidjson = false"

            # TODO(b/279980674): re-enable end2end tests
            "build_angle_end2end_tests_aosp = true"
            "build_angle_trace_tests = false"
            "angle_test_enable_system_egl = true"
        )

        if [[ "$1" == "--enableApiTrace" ]]; then
            gn_args=(
                "${gn_args[@]}"
                "angle_enable_trace = true"
                "angle_enable_trace_android_logcat = true"
            )
        fi

        gn gen ${GN_OUTPUT_DIRECTORY} --args="${gn_args[*]}"
        gn desc ${GN_OUTPUT_DIRECTORY} --format=json "*" > ${GN_OUTPUT_DIRECTORY}/desc.$abi.json
    done

    python3 scripts/generate_android_bp.py \
        --gn_json_arm=${GN_OUTPUT_DIRECTORY}/desc.arm.json \
        --gn_json_arm64=${GN_OUTPUT_DIRECTORY}/desc.arm64.json \
        --gn_json_x86=${GN_OUTPUT_DIRECTORY}/desc.x86.json \
        --gn_json_x64=${GN_OUTPUT_DIRECTORY}/desc.x64.json \
        --output=Android.bp
}

function generate_angle_commit_file() {
    # Output chromium ANGLE git hash during ANGLE to Android roll into
    # {AndroidANGLERoot}/angle_commit.h.
    # In Android repos, we stop generating the angle_commit.h at compile time,
    # because in Android repos, access to .git is not guaranteed, running
    # commit_id.py at compile time will generate "unknown hash" for ANGLE_COMMIT_HASH.
    # Instead, we generate angle_commit.h during ANGLE to Android roll time.
    # Before roll_aosp.sh is called during the roll, ANGLE_UPSTREAM_HASH environment
    # variable is set to {rolling_to} git hash, and that can be used by below
    # script commit_id.py as the ANGLE_COMMIT_HASH written to the angle_commit.h.
    # See b/348044346.
    python3 src/commit_id.py \
        gen \
        angle_commit.h
}

if [[ "$1" == "--genAndroidBp" ]];then
    generate_Android_bp_file "$2"
    exit 0
fi

# Check out depot_tools locally and add it to the path
DEPOT_TOOLS_DIR=_depot_tools
rm -rf ${DEPOT_TOOLS_DIR}
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git ${DEPOT_TOOLS_DIR}
export PATH=`pwd`/${DEPOT_TOOLS_DIR}:$PATH


# This script is executed by the Skia roller in *AOSP* checkout (with new ANGLE changes merged into it by git merge).
# The Skia roller merges ANGLE changes and also has code to remove submodules and extra files.
# For local setup including the additional Skia roller actions see go/angle-skia-roller-local
#
# Some caveats due to the way this is set up:
# * If a file is deleted from AOSP by a previous commit to AOSP *and* is changed upstream in ANGLE,
#   git merge will fail in Skia roller before even reaching this script. This sometimes happens with e.g. .gitmodules
#
# * If a file is needed to run gn gen (commonly BUILD.gn) it might need to take a different path:
#
#   * If this file comes from ANGLE, it is already git-merged into AOSP by Skia roller by the time we get to this script
#      - example: ANGLE source files, but also e.g. third_party/jdk/BUILD.gn (see note below)
#
#   * If this file comes from a third_party dep (pulled by gclient in generate_Android_bp_file), it is either:
#      1. deleted after the codegen ensuring it is *not* in AOSP (delete_after_codegen_paths)
#      2. automatically ignored due to it being part of a sub-repo not this repo (.git subdirs in git deps but not cipd)
#      3. copied to AOSP intentionally by this script (copy_to_aosp_paths, which deletes .git subdirs to avoid the case 2)
#      4. copied to AOSP implicitly, without being in copy_to_aosp_paths:
#         - dep does not have a .git subdir (dep_type cipd in DEPS)
#         - not listed in delete_after_codegen_paths
#         - example: files under third_party/r8
#
#   Note: a file being under third_party/ does *not* necessarily imply it comes from a dep. For example third_party/jdk/BUILD.gn
#         comes from the ANGLE repo, not from the upstream Chromium third_pary/jdk repo.. but it can be different in other cases.
#         In cases like third_party/jdk/BUILD.gn, we need to make sure it is *not* part of delete_after_codegen_paths as
#         this script would delete this file from AOSP after one roll and the next roll would fail gn gen.

# Deps copied to AOSP by having .git removed from them then git add (via git_add_paths)
# .git removed so that it's not an "embedded repository" https://gist.github.com/claraj/e5563befe6c2fb108ad0efb6de47f265
copy_to_aosp_paths=(
    "build"
    "third_party/abseil-cpp"
    "third_party/glslang/src"
    "third_party/spirv-headers/src"
    "third_party/spirv-tools/src"
    "third_party/vulkan-headers/src"
    "third_party/vulkan_memory_allocator"
)

# Dirs and files deleted after codegen so that they don't get added to AOSP.
# We don't need this for dirs with .git in them as those are already ignored by git.
delete_after_codegen_paths=(
   "third_party/android_build_tools"
   "third_party/android_sdk"
   "third_party/android_toolchain"
   "third_party/jdk/current"  # subdirs only to keep third_party/jdk/BUILD.gn (not pulled by gclient as it comes from ANGLE repo)
   "third_party/jdk/extras"
   "third_party/llvm-build"
   "third_party/rust"
   "third_party/rust-toolchain"
   "third_party/zlib"  # Replaced by Android's zlib

   # build/linux is hundreds of megs that aren't needed.
   "build/linux"
   # Debuggable APKs cannot be merged into AOSP as a prebuilt
   "build/android/CheckInstallApk-debug.apk"
   # Remove Android.mk files to prevent automated CLs:
   #   "[LSC] Add LOCAL_LICENSE_KINDS to external/angle"
   "Android.mk"
   "third_party/glslang/src/Android.mk"
   "third_party/glslang/src/ndk_test/Android.mk"
   "third_party/spirv-tools/src/Android.mk"
   "third_party/spirv-tools/src/android_test/Android.mk"
   "third_party/siso" # Not needed
)

# Dirs added to the commit with `git add -f`. Applies to both copy_to_aosp_paths and delete_after_codegen_paths.
git_add_paths=(
  "build"
  "third_party"
)


# Delete first to get a clean checkout by gclient
for path in "${copy_to_aosp_paths[@]}"; do
    rm -rf "$path"
done

# Remove cruft from any previous bad rolls (https://anglebug.com/42266781)
find third_party -wholename "*/_gclient_*" -delete

# Workaround to avoid gclient errors https://crbug.com/skia/14155#c3
rm -rf "third_party/zlib"

# Sync all of ANGLE's deps so that 'gn gen' works
python3 scripts/bootstrap.py
gclient sync --reset --force --delete_unversioned_trees

# Delete outdir to ensure a clean gn run.
rm -rf ${GN_OUTPUT_DIRECTORY}

generate_Android_bp_file
git add Android.bp

generate_angle_commit_file
git add angle_commit.h

# Delete outdir to cleanup after gn.
rm -rf ${GN_OUTPUT_DIRECTORY}

# Delete files that we do not want in AOSP.
# Some of them are needed for codegen so this happens after generate_Android_bp_file.
for path in "${delete_after_codegen_paths[@]}"; do
   rm -rf "$path"
done

# Delete the .git files in each dep so that it can be copied to this repo. Some deps like jsoncpp
# have multiple layers of deps so delete everything before adding them.
for dep in "${copy_to_aosp_paths[@]}"; do
   rm -rf "$dep"/.git
done

# Delete all the .gitmodules files, since they are not allowed in AOSP external projects.
find . -name \.gitmodules -exec rm {} \;

# Add all changes under git_add_paths to sync changes (including deletion) in those dirs to AOSP.
for path in "${git_add_paths[@]}"; do
    git add -f $path
done

# Done with depot_tools
rm -rf $DEPOT_TOOLS_DIR
