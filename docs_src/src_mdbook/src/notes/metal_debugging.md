# Debugging Metal

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
