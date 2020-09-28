# Filament Release Notes log

This file contains one line summaries of commits that are worthy of mentioning in release notes.
A new header is inserted each time a *tag* is created.

## v1.9.2

- Fixes / improvements for contact shadows, fog, and DOF
- Reduce SSAO creases caused by geometry tessellation
- Fix compilation warnings and issue with Clang 12
- Fix JNI crashes
- Rename .blurScale to .cocScale in DOF options

## v1.9.1

- Improvements to SSAO quality
- Fix unoptimized shader crashes with certain OpenGL drivers
- Add float versions of math constants to libmath
- filament-utils: fix, `CoroutineScope` job should be canceled before destroy

## v1.9.0

- `MASKED` mode now leaves destination alpha intact (useful for transparent targets).
- `MASKED` mode now benefit from smoothing in `unlit` materials.
- Small performance improvement to FXAA.
- Fixed `KHR_materials_transmission` to use the `FADE` blending mode.
- Fixed frame graph crash when more than 32 stages were required.
- Fixed several memory leaks in gltfio and the JavaScript bindings.
- Fixed several platform-specific Vulkan bugs and crashes.
- Temporal Anti-Aliasing (TAA) is now available as a complement to MSAA and FXAA. It can be turned
  on and controlled using `View.setTemporalAntiAliasingOptions()`.
- Added texture getters to `Skybox` and `IndirectLight` (C++, Java, JavaScript).
- Added APIs to create 3D textures and 2D texture arrays.
- Internal buffers can now be sized at compile times for applications that render very large
  numbers of objects.
- `View.setAmbientOcclusion()` is deprecated in favor of `View.setAmbientOcclusionOptions`
   (⚠️ **API change**).
- Switched to C++17.
- Variance Shadow Mapping (VSM) is now available as an alternative to PCF shadows (experimental).
- Reduced compiled material sizes by removing unnecessary variants.
- Many improvement and fixes in the Vulkan backend.
- Many improvement and fixes in the Metal backend.
- Fixed translucent views with custom render targets.
- Improved MSAA implementation compatibility on Android devices.
- Use "reverse-z" for the depth buffer.
- Added a way to create an `Engine` asynchronously.
- Highlights are now more stable under depth of field.
- New option to compress highlights before bloom.
- Improvements and fixes to SSAO and DOF.

## v1.8.1

- New CocoaPods sample for iOS.
- Filament for iOS now supports iOS 11.
- Updated the Emscripten SDK to 1.39.19.
- Fixed skinning issue with Emscripten.
- JavaScript APIs for color grading and the vignette effect.
- Added various missing APIs to Java and JavaScript bindings.
- Fixed camera aspect ratio when loading a camera from a glTF file.
- gltfio now uses specular anti-aliasing by default.
- gltfio now supports the KHR_materials_transmission extension.
- Compiled materials do not perform unnecessary fp32 operations anymore.
- Improved quality and performance of the depth of field effect.
- Fixed transform hierarchy memory corruption when a node is set to be parentless.
- Fixed crashed in some browsers and on some mobile devices caused by
  Google-style line directives in shaders.
- Color grading now has a quality option which affects the size and bit depth of the 3D LUT.
- Fixed crash in the Metal backend when more than 16 samplers are bound.
- Added validation in `Texture::setImage()`.
- Fixed refraction/transmission roughness when specular anti-aliasing is enabled.

## v1.8.0

- Improved JavaScript API for SurfaceOrientation and Scene.
- Updated JavaScript API around Camera construction / destruction (⚠️ **API change**)
- Add missing JavaScript API for `View::setVisibleLayers()`.
- Fixed regression in JavaScript IcoSphere that caused tutorial to fail.
- gltf_viewer now supports viewing with glTF cameras.
- gltfio now uses high precision for texture coordinates.
- gltfio now supports importing glTF cameras.
- gltfio now supports simple instancing of entire assets.
- gltfio has improved performance and assumes assets are well-formed.
- gltfio now supports name and prefix lookup for entities.
- ModelViewer now allows resources to be fetched off the UI thread.
- Add support for DOF with Metal backend.
- New Depth-of-Field (Dof) algorithm, which is more plausible and about an order of magnitude faster
  (about 4ms on Pixel4).
- SSAO now has an optional high(er) quality upsampler.
- Tone mappping now uses the real ACES tone mapper, applied in the proper color space.
- Tone mapping is now applied via a LUT.
- `View::setToneMapping` is deprecated, use `View::setColorGrading` instead. (⚠️ **API change**)
- Color grading capabilities per View: white balance (temperature/tint), channel mixer,
  tonal ranges (shadows/mid-tones/highlights), ASC CDL (slope/offset/power), contrast, vibrance,
  saturation, and curves.
- New vignette effect.
- Improved MSAA performance on mobile.
- Improved performance of the post-process pass when bloom is disabled on mobile.
- Added support for 3D textures.
- Fixed private API access on some versions of Android.
- Many improvements and bug fixes in Metal and Vulkan backends.
- Fixed bug in the Metal backend when SSR and MSAA were turned on.
- Fixed Metal issue with `BufferDescriptor` and `PixelBufferDescriptor`s not being called on
  the application thread.

## v1.7.0

- MaterialInstances now have optional names.
- Improved Depth of Field effect: bokeh rotates with the aperture diameter, improved CoC calculation, feather blur radius.
- Introduced `getNormalizedViewportCoord` shader API.
- Added basic SwiftShader support.
- Fixed SwapChain resizing issues in Vulkan.
- Added debug option to track `Entities`.
- Fixed `Camera` entity leaks.
- Removed problematic `CreateEliminateDeadMembersPass`, which broke UBO layout.
- Added assert that the engine is not terminated in `flushAndWait()`.
- Added several fixes and improvements around objects lifetime management
- `gltfio`: AssetLoader now loads names for mesh-free nodes
- `gltfio`: Material names are now preserved in ubershader mode
- Fixed JNI objects allocation and memory corruption
- JNI constructors are now "package private" unless they take an Engine.

## v1.6.0

- gltfio: fixed incorrect cone angles with lights.
- Specular ambient occlusion now offers 3 modes: off, simple (default on desktop) and bent normals.
  The latter is more accurate but more expensive and requires a bent normal to be specified in the
  material. If selected and not bent normal is specified, Filament falls back to the simple mode.
- Specular ambient occlusion from bent normals now smoothly disappears as roughness goes from 0.3
  to 0.1. Specular ambient occlusion can completely remove specular light which looks bad on glossy
  metals. Use the simple specular occlusion mode for glossy metals instead.
- Refraction can now be set on `MaterialBuilder` from Java.
- Refraction mode and type can now be set by calling `MaterialBuilder::refractionMode()`.
  and `MaterialBuilder::refractionType()` instad of `materialRefraction()` and
  `materialRefractionType()` (️⚠️ **API change**).
- Fixed documentation confusion about focused spot vs spot lights.
- Fixed a race condition in the job system.
- Fixed support for 565 bitmaps on Android.
- Added support for timer queries in the Metal backend.
- Improved dynamic resolution implementation to be more accurate and target more platforms.
- `beginFrame()` now accepts a v-sync timestamp for accurate frame time measurement (used for
  frame skipping and dynamic resolution). You can pass `0` to get the old behavior (⚠️ **API change**).
- Fixed several issues related to multi-view support: removed
  `View::setClearColor()`, a similar functionality is now handled by `Renderer::setClearOptions()`
  and `Skybox`, the later now can be set to a constant color (⚠️ **API breakage**).
- Fixed spot/point lights rendering bug depending on Viewport position.
- Textures can now be swizzled.
- The emissive property of materials is now expressed in nits and the alpha channel contains the
  exposure weight (at 0.0 the exposure is not applied to the emissive component of a surface, at
  1.0 the exposure is applied just like with any regular light) (⚠️ **API breakage**).
- Added new `intensityCandela` and `setIntensityCandela` API to `LightManager` for setting a punctual
  light's intensity in candela.
- Fixed an issue where some `ShadowOptions` were not being respected when passed to
  `LightManager::Builder`.
- Added a Depth of Field post-processing effect

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
