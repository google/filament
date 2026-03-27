# Filament Image Upload & ReadPixels Logic

In modern graphics programming, coordinate systems (specifically the origin point) vary between APIs. **Filament** abstracts these differences, but understanding the underlying transformations is crucial for debugging texture uploads and `readPixels` operations.

## 1\. The Source Image

The process begins with a standard image buffer in CPU memory.

  - **Memory Layout:** Data is ordered from **low addresses** (top of the image) to **high addresses** (bottom of the image).
  - **Orientation:** The image is naturally "upright."

## 2\. Backend-Specific Behaviors

### OpenGL (The Bottom-Left API)

OpenGL is unique because its texture coordinate system origin \\((0, 0)\\) is at the **bottom-left**.

  - **Texture Upload:** To ensure texture sampling works "normally" (where $v=0$ is the top), images are typically flipped during upload. This results in the texture being stored **upside-down** in GPU memory.
  - **Standard `glReadPixels`:** Because OpenGL reads from the bottom-left, a standard `glReadPixels` call effectively flips the data back, resulting in an **upright** image in the CPU buffer.
  - **Filament's Handling:** To maintain consistency or specific internal alignment, the Filament OpenGL backend performs an additional **CPU flip** after the read. This results in a final buffer where the image is **upside-down** relative to the original source.

### Metal, Vulkan, & WebGPU (The Top-Left APIs)

Metal, Vulkan, and WebGPU use a \\((0, 0)\\) **top-left** coordinate system, which aligns more closely with standard image memory layouts.

  - **Texture Upload:** No flip is required. The image is stored in GPU memory in its **upright** orientation.
  - **Filament `ReadPixels`:** The data is read directly. Since the GPU storage and the memory layout both favor top-left, the resulting CPU buffer is **upright**.

-----

## 3\. Visual Flow Diagram

The following SVG diagram illustrates the transformation steps for each backend.

![Visual Flow Diagram](upload_readpixels.svg)

-----

## 4\. Key Takeaways

| Feature | OpenGL | Metal / Vulkan / WebGPU |
| :--- | :--- | :--- |
| **Coordinate Origin** | Bottom-Left \\((0, 0)\\) | Top-Left \\((0, 0)\\) |
| **GPU Storage** | Flipped (Internal) | Upright |
| **Raw Read Result** | Upright (due to double-flip) | Upright |
| **Filament Final Output** | **Flipped** (Post-processed) | **Upright** |

> **Note:** The "Filament CPU flip" in the OpenGL backend is an intentional step. To understand *why* this happens, we have to look at the primary use case for `readPixels`: taking screenshots of the rendered scene.
>
> In a rendered scene, OpenGL's $y=0$ represents the bottom of the screen. A raw `glReadPixels` call reads starting from $y=0$ and places that data at the beginning of the memory buffer (low addresses), which results in an upside-down screenshot. To fix this and ensure rendered screenshots are upright, Filament's OpenGL backend universally applies a vertical CPU flip after reading pixels.
>
> However, because this flip is applied universally, it creates a quirk when reading back from an offscreen `RenderTarget` backed by a `Texture` that was manually uploaded. Since the image was already flipped during the texture upload to account for OpenGL's coordinate system, the universal readback flip effectively inverts it *again*, resulting in a CPU buffer that is upside-down compared to the original source. Metal, Vulkan, and WebGPU natively use a top-left origin, so they do not require these compensating flips for either uploads or readbacks.
