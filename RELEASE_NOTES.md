# Filament Release Notes log

This file contains one line summaries of commits that are worthy of mentioning in release notes.
A new header is inserted each time a *tag* is created.

## Next release

- Dielectrics now behave properly under a white furnace (energy preserving and conserving)
- Clear coat roughness now remains in the 0..1 (previously remapped to the 0..0.6 range)
- gltfio: Fixed several limitations with ubershader mode.
- gltfio: Fixed a transforms issue with non-uniform scale.
- webgl: Fixed an issue with JPEG textures.

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
