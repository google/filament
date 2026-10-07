# Filament sample Android apps

This directory contains several sample Android applications that demonstrate how to use the
Filament APIs:

### `hello-triangle`

Demonstrates how to set up a rendering surface for Filament and draw a simple animated triangle.
This is the best starting point for learning the basic structure of a Filament app.

![Hello Triangle](../../docs_src/src_mdbook/src/images/samples/sample_hello_triangle.jpg)

### `lit-cube`

Demonstrates how to create a light and a mesh with the attributes required for lighting.

![Lit Cube](../../docs_src/src_mdbook/src/images/samples/sample_lit_cube.jpg)

### `textured-object`

Demonstrates how to load and use textures for complex materials.

![Textured Object](../../docs_src/src_mdbook/src/images/samples/sample_textured_object.jpg)

### `image-based-lighting`

Demonstrates how to create image-based lights and load complex meshes.

![Image-based Lighting](../../docs_src/src_mdbook/src/images/samples/sample_image_based_lighting.jpg)

### `material-builder`

Demonstrates how to programmatically generate Filament materials on the device with
`MaterialBuilder`, as opposed to compiling them on the host machine with `matc`. This approach
increases the size of the app because it requires the `filamat` library.

![Material Builder](../../docs_src/src_mdbook/src/images/samples/sample_material_builder.jpg)

### `gltf-viewer`

Demonstrates how to load glTF models with `ModelViewer` and how to use the camera manipulator.
The app can also receive models, zipped glTF files, and HDR environments from a desktop browser
through the [remote page](https://google.github.io/filament/remote/).

![glTF Viewer](../../docs_src/src_mdbook/src/images/samples/sample_gltf_viewer.jpg)

### `transparent-view`

Demonstrates how to render into a transparent `SurfaceView` so that Android views placed beneath
it remain visible.

![Transparent View](../../docs_src/src_mdbook/src/images/samples/sample_transparent_rendering.jpg)

### `texture-view`

Demonstrates how to render into a `TextureView` instead of a `SurfaceView`.

![Texture View](../../docs_src/src_mdbook/src/images/samples/sample_texture_view.jpg)

### `multi-view`

Demonstrates how to render a single scene with multiple Filament `View`s, each with its own
viewport. One of the views is drawn on top of the others with translucent blending.

![Multi View](../../docs_src/src_mdbook/src/images/samples/sample_multi_view.jpg)

### `texture-target`

Demonstrates how to render a scene off-screen into a texture with a `RenderTarget`, and then use
that texture on a quad drawn on screen. By default, the texture is backed by an Android
`HardwareBuffer` and sampled as an external texture.

![Texture Target](../../docs_src/src_mdbook/src/images/samples/sample_texture_target.jpg)

### `procedural-effect`

Demonstrates attribute-less rendering, where the vertex shader generates a full-screen quad from
the vertex index without any vertex attributes. The fragment shader then draws an animated
procedural pattern driven by a time parameter.

![Procedural Effect](../../docs_src/src_mdbook/src/images/samples/sample_procedural_effect.jpg)

### `procedural-texture-quad`

Demonstrates how to draw a textured quad whose geometry is generated entirely in the vertex shader
from the vertex index. This shows that attribute-less geometry can still be combined with regular
texture sampling.

![Procedural Texture Quad](../../docs_src/src_mdbook/src/images/samples/sample_procedural_texture_quad.jpg)

### `hello-camera`

Demonstrates how to use `Stream` with Android's Camera2 API.

![Hello Camera](../../docs_src/src_mdbook/src/images/samples/sample_hello_camera.jpg)

### `stream-test`

Tests the various ways to interact with `Stream` by drawing into an external texture using Canvas.
If the two sets of stripes in the screenshot are perfectly aligned, then the Filament frame and the
external texture are perfectly synchronized.

![Stream Test](../../docs_src/src_mdbook/src/images/samples/sample_stream_test.jpg)

### `time-projection`

Demonstrates frame pacing with `FramePacer` and how to drive material animation from the expected
presentation time instead of the vsync time. The clock hand advances by exactly one step per frame,
so dropped or late frames are easy to spot. Switches let you inject pauses, change the target frame
rate, and compare the two timing modes.

![Time Projection](../../docs_src/src_mdbook/src/images/samples/sample_time_projection.jpg)

### `material-instance-stress`

Stress-tests the engine by rendering a grid of 1,000 cubes, each with its own `MaterialInstance`.
The color, roughness, and scale of every cube are updated on each frame.

![Material Instance Stress](../../docs_src/src_mdbook/src/images/samples/sample_material_instance_stress.jpg)

### `page-curl`

Pure Java app that demonstrates custom vertex shader animation and two-sided texturing.
Applies the deformation described in "Deforming Pages of Electronic Books" by Hong et al.
Users can drag horizontally to turn the page.

![Page Curl](../../docs_src/src_mdbook/src/images/samples/sample_page_curl.jpg)

### `live-wallpaper`

Demonstrates how to use Filament as the renderer for an Android Live Wallpaper.

![Live Wallpaper](../../docs_src/src_mdbook/src/images/samples/example_live_wallpaper.jpg)

### `render-validation`

A tool that renders glTF test scenes with `ModelViewer` on each configured backend, such as OpenGL
and Vulkan, and compares the results against golden images. Tests are packaged as zip bundles that
contain a JSON configuration, models, and goldens, and the app can generate the goldens on the
device. Failing tests are shown with their difference images.

![Render Validation](../../docs_src/src_mdbook/src/images/samples/sample_render_validation.jpg)

## Building Samples

Before you start, make sure to read [Filament's README](../../README.md). You need to be able to
compile Filament's native library and Filament's AAR for this project. The easiest way to proceed
is to install all the required dependencies and to run the following commands at the root of the
source tree.

To build the samples, please follow the steps described in [BUILDING.md](../../BUILDING.md#android)

