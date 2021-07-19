# third_party/moltenvk

This folder contains prebuilt dylib files extracted from `macOS/lib` in the following LunarG SDK:

    vulkansdk-macos-1.2.182.0.dmg

The purpose of these files is to allow Filament developers to avoid installing the LunarG SDK.
However, to enable validation you must install the SDK.

# Enabling validation on macOS

If you wish to enable validation or use a newer version of MoltenVK than what is included in the
Filament repo, please do the following.

1. Download and execute the latest dmg file from https://vulkan.lunarg.com/sdk/home

2. To set environment variables such as VK_ICD_FILENAMES etc, go to the SDK folder and do
   `source setup-env.sh`.
