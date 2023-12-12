# Filament Debugging

## Debugging Vulkan on Linux

### Enable Validation Logs

Simply install the LunarG SDK (it's fast and easy), then make sure you've got the following
environment variables set up in your **bashrc** file. For example:

```
export VULKAN_SDK='/path_to_home/VulkanSDK/1.3.216.0/x86_64'
export VK_LAYER_PATH="$VULKAN_SDK/etc/explicit_layer.d"
export PATH="$VULKAN_SDK/bin:$PATH"
```

As long as you're running a debug build of Filament, you should now see extra debugging spew in your
console if there are any errors or performance issues being caught by validation.

### Frame Capture in RenderDoc

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

## Enable Metal Validation

To enable the Metal validation layers when running a sample through the command-line, set the
following environment variable:

```
export METAL_DEVICE_WRAPPER_TYPE=1
```

You should then see the following output when running a sample with the Metal backend:

```
2020-10-13 18:01:44.101 gltf_viewer[73303:4946828] Metal API Validation Enabled
```

## Metal Frame Capture from gltf_viewer

To capture Metal frames from within gltf_viewer:

### 1. Create an Info.plist file

Create an `Info.plist` file in the same directory as `gltf_viewer` (`cmake/samples`). Set its
contents to:

```
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>MetalCaptureEnabled</key>
    <true/>
</dict>
</plist>
```

### 2. Capture a frame

Run gltf_viewer as normal, and hit the "Capture frame" button under the Debug menu. The captured
frame will be saved to `filament.gputrace` in the current working directory. This file can then be
opened with Xcode for inspection.
