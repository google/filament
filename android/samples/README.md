# Filament sample Android apps

This directory contains several sample Android applications that demonstrate how to use the
Filament APIs:

### `hello-triangle`

Demonstrates how to setup a rendering surface for Filament:

![Hello Triangle](../../docs/images/samples/sample_hello_triangle.jpg)

### `lit-cube`

Demonstrates how to create a light and a mesh with the attributes required for lighting:

![Lit Cube](../../docs/images/samples/sample_lit_cube.jpg)

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

Demonstrates how to programatically generate Filament materials, as opposed to compiling them on the
host machine:

![Material Builder](../../docs/images/samples/sample_image_based_lighting.jpg)

### `gltf-bloom`

Demonstrates how to load glTF models and render to an offscreen buffer:

![glTF Bloom](../../docs/images/samples/sample_gltf_bloom.jpg)

### `hello-camera`

Demonstrates how to use `Stream` with Android's Camera2 API:

![Hello Camera](../../docs/images/samples/sample_hello_camera.jpg)

## Prerequisites

Before you start, make sure to read [Filament's README](../../README.md). You need to be able to
compile Filament's native library and Filament's AAR for this project. The easiest way to proceed
is to install all the required dependencies and to run the following commands at the root of the
source tree:

```
$ ./build.sh -p desktop -i release
$ ./build.sh -p android release
```

This will build all the native components and the AAR required by this sample application.

If you do not use the build script, you must set the `filament_tools_dir` property when invoking
Gradle, either from the command line or from `local.properties`. This property must point to the
distribution/install directory for desktop (produced by make/ninja install). This directory must
contain `bin/matc` and `bin/cmgen`.

Example:
```
$ ./gradlew -Pfilament_tools_dir=../../dist-release assembleDebug
```

## Important: SDK location


Either ensure your `ANDROID_HOME` environment variable is set or make sure each project contains a
`local.properties` file with the `sdk.dir` property pointing to your installation of the Android
SDK. This includes the project `filament-android` in the parent directory.

## Android Studio

You must use Android Studio 3.3 or higher to open these projects.

## Compiling

To compile and run each sample make sure you have selected the appropriate build variant
(arm7, arm8, x86 or x86_64). If you are not sure you can simply select the "universal"
variant which includes all the other ones.
