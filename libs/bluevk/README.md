# bluevk

## Updating Vulkan headers

To update the Vulkan headers, perform the following steps.

First, find the latest version of the Vulkan headers here:
https://github.com/KhronosGroup/Vulkan-Headers/tags

Replace `v1.3.296` with the latest version of the headers in the following commands.
```
cd libs/bluevk
curl -OL https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v1.3.296.zip
unzip v1.3.296.zip
rsync -r Vulkan-Headers-1.3.296/include/vulkan/ include/vulkan --delete
rsync -r Vulkan-Headers-1.3.296/include/vk_video/ include/vk_video --delete
rm include/vulkan/*.hpp
rm -r Vulkan-Headers-1.3.296 v1.3.296.zip
```
