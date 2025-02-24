# Downloads

## Vulkan SDK

The official releases for SPIRV-Tools can be found on LunarG's
[SDK download page](https://vulkan.lunarg.com/sdk/home).
The Vulkan SDK is updated approximately every six weeks.

## Android NDK

SPIRV-Tools host executables, and library sources are published as
part of the [Android NDK](https://developer.android.com/ndk/downloads).

## Automated builds

For convenience, here are also links to the latest builds (HEAD).
Those are untested automated builds. Those are not official releases, nor
are guaranteed to work. Official releases builds are in the Android NDK or
Vulkan SDK.

Download the latest builds of the [main](https://github.com/KhronosGroup/SPIRV-Tools/tree/main) branch.

| Platform | Processor | Compiler | Release build | Debug build |
| --- | --- | --- | --- | --- |
| Windows | x86-64 | VisualStudio 2022 (MSVC v143) | Download: <a href="https://storage.googleapis.com/spirv-tools/badges/build_link_windows_vs2022_release.html"> <img src="https://storage.googleapis.com/spirv-tools/badges/build_status_windows_vs2022_release.svg" alt="status of VS 2022 release build"></a> | Download: <a href="https://storage.googleapis.com/spirv-tools/badges/build_link_windows_vs2022_debug.html"> <img src="https://storage.googleapis.com/spirv-tools/badges/build_status_windows_vs2022_debug.svg" alt="status of VS 2022 debug build"></a> |
| Linux | x86-64 | GCC 9.4 | Download: <a href="https://storage.googleapis.com/spirv-tools/badges/build_link_linux_gcc_release.html"> <img src="https://storage.googleapis.com/spirv-tools/badges/build_status_linux_gcc_release.svg" alt="status of Linux GCC build"></a> | Download: <a href="https://storage.googleapis.com/spirv-tools/badges/build_link_linux_gcc_debug.html"> <img src="https://storage.googleapis.com/spirv-tools/badges/build_status_linux_gcc_debug.svg" alt="status of Linux GCC debug build"></a> |
| macOS | x86-64 | Clang 15 | Download: <a href="https://storage.googleapis.com/spirv-tools/badges/build_link_macos_clang_release.html"> <img src="https://storage.googleapis.com/spirv-tools/badges/build_status_macos_clang_release.svg" alt="status of macOS Clang build"></a> | Download: <a href="https://storage.googleapis.com/spirv-tools/badges/build_link_macos_clang_debug.html"> <img src="https://storage.googleapis.com/spirv-tools/badges/build_status_macos_clang_debug.svg" alt="status of macOS Clang build"></a> |

Note: If you suspect something is wrong with the compiler versions mentioned,
check the scripts and configurations in the [kokoro](../kokoro) source tree,
or the results of the checks on the latest commits on the `main` branch.
