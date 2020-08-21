# third_party/moltenvk

This folder contains prebuilt dylib files extracted from `macOS/lib` in the following LunarG SDK:

    vulkansdk-macos-1.2.148.1.dmg

The purpose of these files is to allow Filament developers to avoid installing the LunarG SDK.
However, to enable validation you must install the SDK.

# Enabling validation on macOS

If you wish to enable validation or use a newer version of MoltenVK than what is included in the
Filament repo, please do the following.

1. Download the latest dmg file from https://vulkan.lunarg.com/sdk/home

2. Unpack the dmg and run `sudo ./install_vulkan.py`

3. To set environment variables such as VK_ICD_FILENAMES etc, do `source setup-env.sh`.

4. If you see a dialog like "Apple cannot check it for malicious software..." then you might need
   to enable the dylib by ctrl-clicking it in Finder, clicking open, then clicking open again.
