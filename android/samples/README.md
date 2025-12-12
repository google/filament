# Filament sample Android apps

This directory contains several sample Android applications that demonstrate how to use the
Filament APIs:

### `hello-triangle`

Demonstrates how to setup a rendering surface for Filament:

![Hello Triangle](../../docs/images/samples/sample_hello_triangle.jpg)

### `lit-cube`

Demonstrates how to create a light and a mesh with the attributes required for lighting:

![Lit Cube](../../docs/images/samples/sample_lit_cube.jpg)

### `live-wallpaper`

Demonstrates how to use Filament as renderer for an Android Live Wallpaper.

![Live Wallpaper](../../docs/images/samples/example_live_wallpaper.jpg)

### `image-based-lighting`

Demonstrates how to create image-based lights and load complex meshes:

![Image-based Lighting](../../docs/images/samples/sample_image_based_lighting.jpg)

### `textured-object`

Demonstrates how to load and use textures for complex materials:

![Textured Object](../../docs/images/samples/sample_textured_object.jpg)

### `transparent-rendering`

Demonstrates how to render into a transparent `SurfaceView`:

![Transparent Rendering](../../docs/images/samples/sample_transparent_rendering.jpg)

### `texture-view`

Demonstrates how to render into a `TextureView` instead of a `SurfaceView`:

![Texture View](../../docs/images/samples/sample_texture_view.jpg)

### `material-builder`

Demonstrates how to programmatically generate Filament materials, as opposed to compiling them on
the host machine:

![Material Builder](../../docs/images/samples/sample_image_based_lighting.jpg)

### `gltf-viewer`

Demonstrates how to load glTF models and use the camera manipulator:

![glTF Viewer](../../docs/images/samples/sample_gltf_viewer.jpg)

### `hello-camera`

Demonstrates how to use `Stream` with Android's Camera2 API:

![Hello Camera](../../docs/images/samples/sample_hello_camera.jpg)

### `page-curl`

Pure Java app that demonstrates custom vertex shader animation and two-sided texturing.
Applies the deformation described in "Deforming Pages of Electronic Books" by Hong et al.
Users can drag horizontally to turn the page.

![Page Curl](../../docs/images/samples/sample_page_curl.jpg)

### `stream-test`

Tests the various ways to interact with `Stream` by drawing into an external texture using Canvas.
See the following screenshot; if the two sets of stripes are perfectly aligned, then the Filament
frame and the external texture are perfectly synchronized.

![Stream Test](../../docs/images/samples/sample_stream_test.jpg)

## Building Samples

Before you start, make sure to read [Filament's README](../../README.md). You need to be able to
compile Filament's native library and Filament's AAR for this project. The easiest way to proceed
is to install all the required dependencies and to run the following commands at the root of the
source tree.

To build the samples, please follow the steps described in [BUILDING.md](../../BUILDING.md#android)

