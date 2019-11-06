# Filament Release Notes log

This file contains one line summaries of commits that are worthy of mentioning in release notes.
A new header is inserted each time a *tag* is created.

## Next release

- Fixed an assertion when a parameter array occurs last in a material definition.
- Fixed morph shapes not rendering in WebGL.
- Added support for the latest version of emscripten.
- gltfio: fixed blackness seen with default material.
- Added ETC2 and BC compressed texture support to Metal backend.
- Rendering a SAMPLER_EXTERNAL texture before setting an external image no longer results in GPU errors.

## v1.4.2

- Cleaned up the validation strategy in Engine (checks for use-after-destroy etc).
- OpenGL: Fixed ES 3.0 support on iOS.
- OpenGL: Added support for KHR_debug in debug builds.
- gltfio: Added Java / Kotlin bindings for Animator.
- gltfio: Fixed panic with the Android gltf-bloom demo.
- gltfio: Java clients should no longer call Filament#init.

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
