# third_party/moltenvk

This folder contains prebuilt dylib files extracted from the following LunarG SDK:

    vulkansdk-macos-1.2.135.0.tar.gz

The purpose of these files is to allow Filament developers to avoid installing the LunarG SDK.
However, to enable validation you must install the SDK.

# Enabling validation on macOS

If you wish to enable validation or use a newer version of MoltenVK than what is included in the
Filament repo, please do the following.

1. Download the latest tarball from https://vulkan.lunarg.com/sdk/home

2. Unpack the tarball and run `./install_vulkan.py`

    - If you see a complaint about `/usr/local/lib/cmake`, do a force install.
    - It might complain about permissions, in which case you could do
      `sudo chown -R $(whoami) /usr/local/bin`.

3. To set environment variables such as VK_ICD_FILENAMES etc, do `source MY_SDK/setup-env.sh`.

4. If you see a dialog like "Apple cannot check it for malicious software..." then you might need
   enable the dylib by ctrl-clicking it in Finder, clicking open, then clicking open again.
