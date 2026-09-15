# Debugging Vulkan

This document describes how to enable Vulkan validation layers, debug marker naming, and RenderDoc
capture support in Filament.

---

## 1. Enable Validation Layers

To enable Vulkan validation logs, Filament must be compiled with the validation debug flag
(`FVK_DEBUG_VALIDATION`), and the Vulkan validation layers must be available on the system.

### Build with Validation Enabled

Validation support is gated by the backend debug preprocessor flag `FVK_DEBUG_VALIDATION` (`0x40`).

When building with `./build.sh`, pass `-x 0x40` (and `-f` if rebuilding):

```bash
./build.sh -f -x 0x40 -p desktop debug
```

Or pass `-DFILAMENT_BACKEND_DEBUG_FLAG=0x40` directly to CMake:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DFILAMENT_BACKEND_DEBUG_FLAG=0x40 ...
```

### Environment Setup (Desktop)

Make sure the LunarG Vulkan SDK is installed and the following environment variables are set (e.g.
in your `.bashrc` or `.zshrc`):

```bash
export VULKAN_SDK='/path_to_home/VulkanSDK/1.3.xxx.x/x86_64'
export VK_LAYER_PATH="$VULKAN_SDK/etc/explicit_layer.d"
export PATH="$VULKAN_SDK/bin:$PATH"
```

When running a debug build of Filament compiled with validation enabled, validation messages and
performance warnings will be logged to the console.

### Environment Setup (Android)

On Android, ensure the validation layer binary (`libVkLayer_khronos_validation.so`) is included in
your APK under `jniLibs`.

---

## 2. Debug Utils & Group Markers (`d.vulkan.debug_utils_names`)

When enabled, the Vulkan backend explicitly requests the `VK_EXT_debug_utils` extension at startup,
tracks group markers in the command buffer, and names render passes and shader modules with their
corresponding marker labels. This makes it easier to inspect command buffers and render passes in
tools like RenderDoc or Android GPU Inspector (AGI).

### Enable on Desktop

Set the `d_vulkan_debug_utils_names` environment variable when running your application:

```bash
d_vulkan_debug_utils_names=1 ./out/cmake-debug/samples/gltf_viewer -a vulkan <path-to-model>
```

### Enable on Android

Set the system property via `adb`:

```bash
# Enable
adb shell setprop debug.d.vulkan.debug_utils_names 1

# Disable
adb shell setprop debug.d.vulkan.debug_utils_names 0
```

---

## 3. RenderDoc Capture Mode (`d.vulkan.renderdoc_capture`)

When capturing frames with [RenderDoc](https://renderdoc.org/), certain Vulkan features (such as
lazily allocated memory and non-replayable external memory types on Android) can cause crashes or
capture failures. Enabling this flag:
- Enables the `VK_LAYER_RENDERDOC_Capture` instance layer if available.
- Disables lazily allocated memory (`VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT`).
- Adjusts Android external hardware buffer memory type selection so captures can be replayed
  cleanly.

### Enable on Desktop

Set the `d_vulkan_renderdoc_capture` environment variable:

```bash
d_vulkan_renderdoc_capture=1 ./out/cmake-debug/samples/gltf_viewer -a vulkan <path-to-model>
```

### Enable on Android

Set the system property via `adb`:

```bash
# Enable
adb shell setprop debug.d.vulkan.renderdoc_capture 1

# Disable
adb shell setprop debug.d.vulkan.renderdoc_capture 0
```

> **Tip:** You can combine both flags when taking RenderDoc captures to get meaningful render pass names alongside capture compatibility:
> ```bash
> d_vulkan_debug_utils_names=1 d_vulkan_renderdoc_capture=1 ./out/cmake-debug/samples/gltf_viewer -a vulkan <path-to-model>
> ```
