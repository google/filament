# Filament Release Notes log

This file contains one line summaries of commits that are worthy of mentioning in release notes.
A new header is inserted each time a *tag* is created.

## Next release

## v1.5.2

- gltfio: fixed null pointer exception seen with some Android clients.
- Engine now exposes its JobSystem to C++ clients.
- Expose setCulling() in public RenderableManager API.

## v1.5.1

- Fixed "no texture bound" warning in WebGL.
- Fixed a clearing bug with imported render targets.
- Fixed the creation potentially invalid entities during shadow map initialization.
- Fixed Maven dependencies for the `filament-utils` library.

## v1.5.0

⚠️ This release breaks compiled materials, use matc to recompile.

- The Android support libraries (gltfio and filament-utils) now use dynamic linking.
- Removed depth-prepass related APIs. (⚠ API Change)
- gltfio: add asynchronous API to ResourceLoader.
- gltfio: generate normals for flat-shaded models that do not have normals.
- Material instances now allow dynamic depth testing and other rasterization state.
- Unlit materials now apply emissive in the same way as lit materials.
- Screen-space refraction is now supported.
- Support for HDR Bloom as a post-process effect.
- Alpha masked objects are now part of the SSAO pass.
- Added Java bindings for geometry::SurfaceOrientation.
- Fixed bug rendering transparent objects with Metal backend.
- Fixed crash on macOS Catalina when rendering with Metal backend.
- Fixed bug in Camera::setLensProjection() and added the aspect ratio parameter. (⚠ API Change)
- WebGL: Improved TypeScript annotations.
- WebGL: Simplified callback API for glTF. (⚠ API Change)
- gltfio: Removed deprecated "Bindings" API. (⚠ API Change)
- gltfio: Added support for Draco.
- gltfio: Reduced the size of the library.
- Improved performance of SSAO.
- Added support for screen-space contact shadows.
- Added support for global fog.
- Added support for bent normal maps and specular occlusion from bent normal maps.
- Added support for shadow-casting spot lights.

## v1.4.5

- The depth prepass setting in View is now ignored and deprecated.
- Fixed a threading bug with the NOOP backend.
- Improved memory management for gltfio on Android.
- Introduced `filament-utils` library with `TextureLoader`, `ModelViewer`, and Java bindings for `camutils`.
- Fix out-of-bounds bug when glTF has many UV sets.
- Added new `setMediaOverlay` API to `UiHelper` for controlling surface ordering.
- Implemented sRGB support for DXT encoded textures.
- Fix bug with incorrect world transforms computed in `TransformManager`.
- gltfio: support external resources on Android.


## v1.4.4

- Added support for solid and thin layer cubemap and screen-space refraction.
- Improved high roughness material rendering by default when regenerating environments maps.
- Fix bad instruction exception with macOS Catalina.
- Fixed bad state after removing an IBL from the Scene.
- Fixed incorrect punctual light binning (affected Metal and Vulkan backends).
- Fixed crash when using a Metal headless SwapChain with an Intel integrated GPU.
- Added support for ASTC textures on iOS with Metal backend.
- Added new heightfield sample.
- Removed `<iostream>` from math headers.
- cmgen now places KTX files directly in the specified deployment folder.

## v1.4.3

- Fixed an assertion when a parameter array occurs last in a material definition.
- Fixed morph shapes not rendering in WebGL.
- Added support for the latest version of emscripten.
- gltfio: fixed blackness seen with default material.
- Added ETC2 and BC compressed texture support to Metal backend.
- Rendering a `SAMPLER_EXTERNAL` texture before setting an external image no longer results in GPU errors.
- Fixed a normals issue when skinning without a normal map or anisotropy.
- Fixed an issue where transparent views couldn't be used with post-processing.
- Always use higher quality 3-bands SH for indirect lighting, even on mobile.
- The Metal backend can now handle binding individual planes of YUV external images.
- Added support for depth buffer when post-processing is turned off
- Improved performance on GPUs that use tile-based rendering

## v1.4.2

- Cleaned up the validation strategy in Engine (checks for use-after-destroy etc).
- OpenGL: Fixed ES 3.0 support on iOS.
- OpenGL: Added support for KHR_debug in debug builds.
- gltfio: Added Java / Kotlin bindings for Animator.
- gltfio: Fixed panic with the Android gltf-bloom demo.
- gltfio: Java clients should no longer call Filament#init.
- Improved IBL diffuse by allowing to use the specular cubemap at `roughness` = 1 instead of Spherical Harmonics

## v1.4.1

- Added missing API documentation.
- Fixed crash for sandboxed macOS apps using Filament.
- Fixed an issue that limited the camera near plane to ~1mm.
- Added Android sample for Camera Stream.
- Fixed an Xcode assertion when rendering skinned meshes using the Metal backend.
- Added support for Core Animation / Metal frame synchronization with Metal backend.
- Fixed an issue with culling in `MaterialInstance`.
- Fix additional compatibility issues with MSVC, including the Vulkan backend.
- matdbg: fixed missing symbol issue when linking against debug builds.
- filamat: fixed crash when using the "lite" version of the library.
- matinfo: Fix a crash with on Windows.
- gltfio: fixed an animation loop bug.
- gltfio: added support for sparse accessors.
- Add JS binding to unary `Camera::setExposure`.

## v1.4.0

- API Breakage: Simplified public-facing Fence API.
- Minimum API level on Android is now API 19 instead of API 21.
- Filament can now be built with msvc 2019.
- Added the ability to modify clip space coordinates in the vertex shader.
- Added missing API documentation.
- Improved existing API documentation.
- Added `Camera::setExposure(float)` to directly control the camera's exposure.
- Backface culling can now be toggled on material instances.
- Face direction is now reversed when transforms have negative scale.
- Dielectrics now behave properly under a white furnace (energy preserving and conserving).
- Clear coat roughness now remains in the 0..1 (previously remapped to the 0..0.6 range).
- gltfio: Fixed several limitations with ubershader mode.
- gltfio: Fixed a transforms issue with non-uniform scale.
- webgl: Fixed an issue with JPEG textures.
- Windows: Fix link error in debug builds.
- matdbg: Web server must now be enabled with an environment variable.
- matdbg: Added support for editing GLSL and MSL code.

## v1.3.2

- Added optional web server for real-time inspection of shader code.
- Added basic #include support in material files.
- Fixed potential Metal memory leak.
- Fixed intermittent memory overflow in wasm builds.
- Fix bad normal mapping with skinning.
- Java clients can now call getNativeObject().

## v1.3.1

- Unified Filament Sceneform and npm releases.
- Improved cmgen SH with HDR images.
- IndirectLight can now be queried for dominant direction and color.
- Added support for vertex morphing.
- Introduced custom attributes, accessible from the vertex shader.
- Added Java / Kotlin bindings for KtxLoader.
- Added JavaScript / Typescript bindings for the new `RenderTarget` class.
- Added base path to glTF loadResources method for JavaScript.
- Added support for iOS `CVPixelBuffer` external images with the OpenGL backend.

## sceneform-1.9pr4

- Added `gltf_bloom` Android sample to show gltfio and the `RenderTarget` API.
- Added `getMaterialInstanceAt` to the Java version of RenderableManager.
- Fix JNI bindings for setting values in parameter arrays.
- Added JNI bindings for the gltfio library.
- Fix support for parameter arrays in `.mat` files.
- Added support for `RGB_11_11_10`
- Removed support for `RGBM` (**warning:** source compatibility breakage)
- IBL cubemap can now be of any size
- `Texture::generatePrefilterMipmap` can be used for runtime generation of a reflection cubemap

## sceneform-1.9pr3

- Added `Scene.addEntities()` to the Java / Kotlin bindings.
- Improved robustness in the tangents utility for meshes that have tangents *and* normals.
- Introduced `RenderTarget` API that allows View to reference an offscreen render target.
- Added `lucy_bloom` sample to demonstrate the new `RenderTarget` API.
- Added Screen Space Ambient Occlusion support (SAO)
- New blending modes: `multiply` and `screen`
- Fixed an issue when sorting blended objects with different blending modes
- The material property `curvatureToRoughness` has been replaced with `specularAntiAliasing`.
  This new specular anti-aliasing solution offers more control via two new properties:
  `specularAntiAliasingVariance` and `specularAntiAliasingThreshold`. They can also be set on
  material instances if needed
- Added specular ambient occlusion to compute a new AO term applied to specular reflections
  (see `specularAmbientOcclusion` property in materials)
- Added multi-bounce ambient occlusion to brighten AO and preserve local color
  (see `multiBounceAmbientOcclusion` property in materials)
- Micro-shadowing is now applied to material ambient occlusion
- Use a smaller 64x64 DFG LUT on mobile to reduce binary size
- Added a distance field generator to libimage.
- JavaScript MaterialInstance now supports vec4 colors.
- Further reduced `filamat` binary size by removing reliance on stdlib.
- Added a new, smaller, version of the `filamat` library, `filamat_lite`. Material optimization and
  compiling for non-OpenGL backends have been removed in favor of a smaller binary size.
- Implemented hard fences for the Metal backend, enablying dynamic resolution support.
- Improved `SurfaceOrientation` robustness when using UVs to generate tangents.
- Created a `RELEASE_NOTES.md` file, to be updated with significant PRs.

## sceneform-1.9pr2
