# Debugging Vulkan on Linux

## Enable Validation Logs

Simply install the LunarG SDK (it's fast and easy), then make sure you've got the following
environment variables set up in your **bashrc** file:

```
export VULKAN_SDK='/usr/local/google/home/MY_LDAP/VulkanSDK/1.1.70.1/x86_64'
export VK_LAYER_PATH="$VULKAN_SDK/etc/explicit_layer.d"
export PATH="$VULKAN_SDK/bin:$PATH"
```

As long as you're running a debug build of Filament, you should now see extra debugging spew in your
console if there are any errors or performance issues being caught by validation.

## Frame Capture in RenderDoc

The following instructions assume you've already installed the LunarG SDK and therefore have the
`VK_LAYER_PATH` environment variable.

1. Modify `VulkanDriver.cpp` by defining `ENABLE_RENDERDOC`
1. Download the RenderDoc tarball for Linux and unzip it somewhere.
1. Find `renderdoc_capture.json` in the unzipped folders and copy it to `VK_LAYER_PATH`. For
example:
```
cp ~/Downloads/renderdoc_1.0/etc/vulkan/implicit_layer.d/renderdoc_capture.json ${VK_LAYER_PATH}
```
1. Edit `${VK_LAYER_PATH}/renderdoc_capture.json` and update the `library_path` attribute.
1. Launch RenderDoc by running `renderdoc_1.0/bin/qrenderdoc`.
1. Go to the **Launch Application** tab and click the ellipses next to **Environment Variables**.
1. Add VK_LAYER_PATH so that it matches whatever you've got set in your **bashrc**.
1. Save yourself some time in the future by clicking **Save Settings** after setting up the working
directory, executable path, etc.
1. Click **Launch** in RenderDoc, then press **F12** in your app.  You should see a new capture show up in
RenderDoc.
