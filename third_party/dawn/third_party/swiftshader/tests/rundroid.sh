#!/bin/bash

# Run an Android NDK binary on the connected device.
#
# Example usage:
# $ cd <builddir>
# $ make vk-unittests
# $ ../rundroid vk-unittests

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <executable>"
    exit 1
fi

dst_dir=/data/local/tmp
path="$1"
name="$(basename "$path")"
shift

if [ -z "$ANDROID_HOME" ]; then
    ANDROID_HOME=$HOME/Android/Sdk
fi

set -e
set -x

for lib in libGLESv2_swiftshader.so libEGL_swiftshader.so libvk_swiftshader.so; do
    adb push --sync "$lib" "${dst_dir}/${lib}"
done

adb push --sync "$ANDROID_HOME/ndk-bundle/sources/cxx-stl/llvm-libc++/libs/arm64-v8a/libc++_shared.so" "${dst_dir}/libc++_shared.so"

adb push --sync "$path" "${dst_dir}/${name}"
adb shell "cd \"$dst_dir\"; chmod +x \"$name\"; LD_LIBRARY_PATH=. ./$name $*"
