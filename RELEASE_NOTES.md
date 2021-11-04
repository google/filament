# Filament Release Notes log

This file contains one line summaries of commits that are worthy of mentioning in release notes.
A new header is inserted each time a *tag* is created.

## v1.14.0 (currently main branch)

- engine: internal materials can use structures as parameters [⚠️ **Material breakage**].

## v1.13.0

- Android: Gradle configuration caching is now enabled
- Android: Filament's Gradle properties have all been renamed to `com.google.android.filament.xxx`
  where `xxx` is the property name. See `android/build.gradle` for a complete list [⚠️]
- Android: The Gradle property `filament_tools_dir` (now called
  `com.google.android.filament.tools-dir`) does not have a default value anymore. Please specify
  one in your `gradle.properties` if you reuse the Gradle plugin in your projects  [⚠️]
- Engine: Fix spotlights direction and falloff.  [⚠️ **Material breakage**].

## v1.12.11

- Metal: Color grading performance improvement on M1 devices.
- samples: Fix glitchy animation seen in gltf-viewer iOS sample.

## v1.12.10

- engine: rewrite dynamic resolution scaling controller for better accuracy and less jittering.
- Java: fix missing ASTC texture enum.
- tools: Fix normal map issues in mipgen.
- WebGL: expose some `SurfaceOrientation` functions.

## v1.12.9

- engine: New API: `MultiSampleAntiAliasingOptions` and HDR-aware MSAA resolve. When `customResolve`
  is enabled, improves anti-aliasing quality [**NEW API**].
- engine: Fixes and improvements for FSR.
- engine: All APIs that take a callback as argument now also take a `CallbackHandler*`, a new
  interface that provides more flexibility around callback dispatch [**NEW API**].
- Android: Fix JNI bindings for `DepthOfFieldOptions`.
- Android: workarounds for Adreno-specific fraembuffer issue.
- JavaScript: updates to JS bindings.

## v1.12.8

- engine: Added picking API to `View`  [⚠️ **Materials need to be rebuilt to access this new feature**].
- engine: A new `Engine::pumpMessageQueues()` method can be used to trigger all pending user
  callbacks right away [**NEW API**].
- engine: new inline helpers to more easily use lambdas, functors and method callbacks with
  `{Pixel}BufferDescriptor`.
- Vulkan: fix vertical offset for `readPixels`.
- Vulkan: various internal improvements.
- Metal: support integer formats with `readPixels`.

## v1.12.7

- engine: Fix, apply emissive after shadowing.

## v1.12.6

- engine: Added concept of lod bias to materials.
  [⚠️ **Materials need to be rebuilt to access this new feature**].
- engine: Fix, BGRA ordering respected for external images with OpenGL on iOS.
- engine: Use more sensible defaults for spot light inner outer cone angles.
- engine: Fix potential race condition that caused stalls in `endFrame`.
- gltfio: Improved handling of transparent materials.
- Metal: Fix potential crash on earlier versions of iOS (<= 13.0).
- Android: Fix `filament-utils-android` 'lite' flavor.
- Java: Fix potential crash with `IBLPrefilter`.

## v1.12.5

- engine: work around a job scheduling issue in `endFrame` that caused stuttering on some Android
devices.

## v1.12.4

- engine: New night adaptation API on `ColorGrading`. This API can be used to create an effect that
  that simulates color and brightness shifts in human vision in low-light conditions.
- engine: improved performance of AMD FidelityFX FSR1 by 4.6x, it now runs in about 2ms in 4K.
- engine: Dynamic resolution quality `MEDIUM`, `HIGH` and `ULTRA` now all use AMD FidelityFX FSR1.
- engine: Fix crash when duplicating material instances.
- gltfio: generate tangents if requested by the material.

## v1.12.3

- engine: Support AMD FidelityFX Super Resolution for dynamic resolution scaling

## v1.12.2

- engine: New API on `ColorGrading` to enable or disable gamut mapping at will [**New API**].
- engine: Fix typo causing ShadowOptions::shadowFar to not work properly.
- engine: Fix, CSM glitch when using shadowMultiplier materials.
- engine: Improve precision when computing camera projection.
- engine: Increase the number of supported spot shadows to 14 (from 6).
- Metal: Add texture swizzling support for external textures.

## v1.12.1

- engine: `double` precision translation support in TransformManager. Disabled by default.
  Augment model (and view) matrix on `Camera` to accept double precision matrices. When enabled,
  double precision translations allow filament to handle a very large world space [**New API**].
- engine: Fix, Views with custom render targets are now blendable.

## v1.12.0

- engine: Option to automatically compute bent normals from SSAO & apply to specular AO
  [⚠️ **Material breakage**].
- engine: New APIs: Light channels. Geometry and lights now have a channel associated to them, at
  least one channel must match for lighting to occur [⚠️ **Material breakage**].
- engine: Fix potential GPU crash with punctual lights near the far clipping plane.
- materials: The `inverseTonemap` API is now an exact inverse of the Filmic tonemapper.
- Metal: Better support for texture formats on M1 Macs.

## v1.11.2

- engine: New API: `ColorGrading::Builder::toneMapper(const ToneMapper*)`.
- engine: New tone mapper: `GenericToneMapper`, a configurable tone mapper.
- engine: `ColorGrading::Builder::toneMapping(ColorGrading::ToneMapping)` is now deprecated.
- engine: Removed `REINHARD` tonemap operator[⚠️ **API Change**].
- engine: Improve s3tc_srgb detection on desktop.
- engine: Add bilateral threshold in SSAO options.
- gltfio: Fix AssetLoader leak, remove unwanted destructor.
- Metal/Vulkan: Fix uploading texture data with padding or offset.
- Metal: fix GPU crash seen with large amounts of geometry.

## v1.11.1

- engine: Luminance scaling can now be used with any tone mapping operator. It was previously tied
  to the "EVILS" tone mapping operator.
- engine: Removed the "EVILS" tone mapping operator [⚠️ **API Change**].
- engine: Improvements to Skinning. A new `SkinningBuffer` API allows bone sharing between
  renderables.
- engine: Improvements to internal memory allocation for Metal and Vulkan backends.
- engine: Default to OpenGL backend when Windows does not support Vulkan.
- samples: Add new sample app: image_viewer.

## v1.11.0

- engine: Added support for transparent shadows. Add `transparentShadow : true` in the material file.
- engine: honor user-defined precision in material files for non-samplers, rename `SamplerPrecision`
  to `ParameterPrecicion`. [⚠️ **API Change**]
- engine: Work around Qualcomm issue with point lights.
- engine: Allow MSAA when post-processing is disabled.
- engine: enable up to 6 spot-light shadows.
- gltfio: Added support for `KHR_materials_volume`.
- gltfio: fix precision in KHR_texture_transform.
- java: Removed support for Java desktop targets (macOS, Linux, and Windows) [⚠️ **API Change**].

## v1.10.7

- engine: Spot-light position calculation moved to fragment shader.
- engine: Small shadow mapping fixes and improvements.
- gltfio: Add fast path for native material providers.
- gltfio: Allow Java / Kotlin clients to customize MaterialProvider.
- engine: Fix out of bounds access with `RenderTarget` java bindings.
- Metal: `TextureFormat::DEPTH24_STENCIL8` now maps to a 32 bit depth format on iOS.

## v1.10.6

- engine: Use exponential VSM and improve VSM user settings [⚠️ **Recompile Materials for VSM**].
- engine: Optional blurring of VSM shadowmaps.
- engine: Fix a crash when using lens flares.
- engine: Fix backend crashes when using an unsupported sample count.
- gltfio: Add new `getAsset`API to `FilamentInstance`.
- gltfio: Introduce support for extras strings.
- OpenGL: Increase OpenGL backend handle arena from 2 to 4 MiB.
- Vulkan: Fix Texture swizzle support.

## v1.10.5

- android: AAR libraries now properly include their ProGuard rules.
- engine: User materials can now provide custom lighting/surface shading, please consult
  the [materials documentation](https://google.github.io/filament/Materials.html) for details.
- engine: `Backend::DEFAULT` now selects the most appropriate backend for the platform, rather than
  always `OPENGL`. On Android the default is `OPENGL`, on Apple platforms the default is `METAL` and
  on all other platforms that default is `VULKAN`.
- engine: Fix a potential memory corruption when using more than 4 render targets.
- engine: Fix a possible crash when bloom is enabled.
- engine: Fix and refactor support for S3TC + SRGB with OpenGL.
- engine: Fix automatic clearing of rendertargets.
- engine: Fix imported render target discard and clear flags.
- engine: Fix opaque blit with imported render targets.

## v1.10.4

- engine: improvements to internal job system.
- Vulkan: performance improvements on Mali.
- gltfio: improvements to load time for large models.
- WebGL: remove bogus stride argument, fix `BindingType` TypeScript definition.

## v1.10.3

- android: use `debug.filament.backend` system property to select the desired backend.
- engine: fix `LightManager::getFalloff`.
- gltfio: fix crash with non-triangles.
- macOS: fix main thread checker warnings with OpenGL.
- vulkan: fix crash on Windows machines with NVIDIA GPUs.

## v1.10.2

- Vulkan: validation and diagnostic improvements
- engine: improvements for scenes with many renderables.
- gltfio: added support for `KHR_materials_ior`.
- java: Add bindings for `IBLPrefilterContext`.
- java: add `KTXLoader.getSphericalHarmonics` JNI binding
- libimage: fix, respect sRGB option for compressed formats.
- sample-gltf-viewer: fix lifetime cycle for RemoteServer.

## v1.10.1

- engine: Add `getPlatform` API to Engine.
- engine: Add a new cone angles API to `LightManager`.
- engine: Attachments of custom RendereTargets are not systematically discarded.
- engine: Fix a crash when using custom rendertargets.
- engine: New API to duplicate a `MaterialInstance`.
- filagui: fix support for custom images in ImGuiHelper.
- java: Add bindings for HDRLoader.

## v1.10.0

- engine: User materials can now use 9 samplers instead of 8 [⚠️ **Material breakage**].
- engine: Remove `populateTangentQuaternions`  [⚠️ **API change**].
- engine: Deprecate `Stream::Builder::stream(intptr_t)` [⚠️ **API Change**].
- engine: Remove deprecated APIs: `Camera::setScaling`, `Engine::destroy(Camera*)`,
  `Engine::createCamera`, `Renderer::beginFrame(SwapChain*, uint64_t,
  backend::FrameScheduledCallback, void*)`, and `View::setShadowsEnabled` [⚠️ **API Change**].
- engine: Remove `focusDistance` from `View::BloomOptions` [⚠️ **API Change**].
- engine: Add a `FILAMENT_SUPPORTS_OPENGL` CMake option to enable/disable OpenGL support.
- Vulkan: fixes and improvements for large scenes.
- gltfio: fix morphing bugs uncovered by MorphStressTest.
- Java: add API for `Texture::Builder::import()`.
- WebGL: Fix a potential INVALID_OPERATION.

## v1.9.25

- NEW API: Screen-space lens flare effect.
- engine: Fix several memory leaks in the GL backend.
- Vulkan: General bug fixes and improvements.
- Vulkan: Fix some problems seen on Mali devices.
- ios: Fix VSM shadows not working.
- webgl: Fix black screen seen with some samples.

## v1.9.24

- engine: Fix memory leaks in OpenGL driver.
- engine: new experimental tone mapper, `EVILS` (name will most likely change).
- engine: Improvements to Vulkan backend.
- engine: Fix incorrect units documented for `LightManager.getIntensity`.
- engine: fix high quality upsampling for SSAO.
- engine: implement accurate normal reconstruction for SSAO.
- engine: improve LOW and HIGH quality levels for SSAO.
- libs: improvements to `libiblprefilter`.
- materials: New `quality` property.
- samples: Add new gltf-viewer iOS sample.
- samples: clear the background in lightbulb sample.

## v1.9.23

- Vulkan: various fixes.
- android: fix crash seen using VSM with MSAA on Adreno devices.
- engine: Add `Engine::getEntityManager()`.
- engine: Fix desktop crash seen with some GPU drivers.
- engine: improve importance sampling.
- gltfio: robustness improvements for Draco meshes.
- libs: Add new Transcoder API for C++ clients (part of `libgeometry`).
- libs: New `iblprefilter` library to compute IBL pre-integration on the GPU using filament.
- materials: Fix documentation for `getNormalizedViewportCoord`.
- samples: fix rendertarget sample crash on launch.

## v1.9.22

- NEW API: `Renderer::renderStandaloneView()` is a new method that can be used outside of
  beginFrame/endFrame on Views that have a RenderTarget associated. This can be used as a
  pseudo-compute API.
- Vulkan: bug fixes and improvements.
- engine: RenderTarget API can now use MRT.
- sample-gltf-viewer: improvements for reading zip files.
- sample-gltf-viewer: enable contact-shadows functionality in mobile gltf-viewer.
- windows: fix build error in filament_framegraph_test.

## v1.9.21

- JavaScript: add missing TextureSampler bindings.
- Metal: Fix texture swizzling crash with older Nvidia GPUs.
- Vulkan: fix image layout validation error on Android.
- android: fix MSAA w/ multisampled_render_to_texture on Mali.
- engine: better anisotropic filtering with various drivers.
- gltfio: Use BufferObject API, simplify MorphHelper.
- gltfio: honor stride in normalizeSkinningWeights.
- samples: Add web component demo.

## v1.9.20

- Android: Fix VSM.
- engine: Introduce BufferObject API.
- engine: Add new isTextureSwizzleSupported API on Texture.
- engine: Add support to Metal and Vulkan backends for texture swizzling.
- engine: Add new DoF settings (native/half res, gather kernel ring counts, CoC radius clamp).
- engine: DoF quality and performance improvements.
- engine: Fix high-quality upsampling issue with SSAO.
- Java: Expose `TransformManager.getParent(int)`.
- samples: Add Metal and Vulkan backend support to Suzanne sample.
- WebGL: expose fitIntoUnitCube to JS.
- WebGL: support for multiple `<canvas>` elements.

## v1.9.19

- engine: Fix Metal bug when setGeometryAt is called multiple times.
- engine: Improvements to DoF.
- engine: Fix RenderTarget NPE when depth is not present.
- engine: Improvements to Camera APIs. Move focus distance from DofOptions to Camera.
- engine: VSM shadows now support `shadowMultiplier`.
- java: Expose severla MaterialInstance APIs (setColorWrite, setDepthWrite, setDepthCulling) that
  should have been public.
- java: fix bug with Texture::setImage buffer size calculation.

## v1.9.18

- engine: Fix a DoF bug that caused black dots around the fast tiles.
- engine: Minor DoF optimizations.
- engine: Fix blanking windows not being drawn into on macOS.
- gltfio: Add support for data:// in image URI's.
- gltfio: Add internal MorphHelper, enable up to 255 targets.
- engine: Fix a hang in JobSystem.
- samples: Fix rendertarget sample app.

## v1.9.17

- engine: New shift parameter on `Camera` to translate the viewport and emulate a tilt/shift lens.
- engine: `Camera::setCustomProjection()` now allows to set a different projection for culling and rendering.
- engine: Fixed depth of field rendering with custom projection matrices.
- engine: Fix a rare indefinite hang.
- gltfio: `SimpleViewer` now exposes more rendering parameters, including `ColorGrading`.
- gltfio: Fix tangents when morphing is enabled.
- Metal/Vulkan: fix incorrect dominant light shadows rendering.
- Fixe some issues with imported rendertargets.

## v1.9.16

gltfio: Add ResourceLoader evict API.
gltfio: Fix ResourceLoader cache bug.
iOS: Disable exceptions to reduce binary size.

## v1.9.15

- filamat/matc: fix sporadic crash.

## v1.9.14

- Improve bloom/emissive with glTF files.
- Publicly expose Exposure API for gltfio.

## v1.9.13

- Android: fix "No implementation found" error.
- Android: fix compilation error in UbershaderLoader.
- engine: computeDataSize now returns correct value for USHORT_565.
- Vulkan: various internal improvements.

## v1.9.12

- engine: Fixed GL errors seen with MSAA on WebGL.
  Warning: this can affect multisampling behavior on devices that do not support OpenGL ES 3.1
- materials: Added new `getVertexIndex()` API for vertex shaders.
- samples: RenderTarget demo now disables post-processing in offscreen view and creates depth attachment.
- gltfio: Fix, animation jolt when time delta is very small.

## v1.9.11

- Added support for Apple silicon Macs. build.sh can now be used to build on either Apple silicon or
  Intel-based Macs. Pass the `-l` flag to build universal binaries.
- Added `sheenColor` and `sheenRoughness` properties to materials to create cloth/fabric.
- Materials generation using `libfilamat` is now multi-threaded.
- `MaterialBuilder::build()` now expects a reference to a `JobSystem` to multi-thread shaders
  generation. A `JobSystem` can be obtained with `Engine::getJobSystem()` when using Filament,
  or created directly otherwise. (⚠️ **API change**)
- Add planar reflection RenderTarget demo.
- Metal: honor inverseFrontFaces RasterState.
- Metal: Fix crash when switching between views with shadowing enabled.
- Metal: Fix crash when calling Texture::setImage() on SAMPLER_2D_ARRAY texture.
- gltfio: added support for `KHR_materials_sheen`.
- gltfio: shader optimizations are now disabled by default, unless opting in or using ubershaders.
- gltfio: Fix "_maskThreshold not found" error.
- gltfio on Java: fix potential memory leak in AssetLoader#destroy.
- gltfio: fix crash during async texture decode.
- gltfio: support animation in dynamically-added instances.
- gltfio: Improve robustness when decoding textures.
- gltfio: Fix animator crash for orphaned nodes.
- gltfio: fix tangents with morphing.
- gltf_viewer: fix very sporadic crash when exiting.
- gltf_viewer: fix crash when rapidly switching between glTF models.
- WebGL: Fix samples erroring on Windows with Chrome.
- WebGL: Support `highlight` for setBloomOptions in JavaScript.
- WebGL: Include TypeScript bindings in releases.
- engine: Fix, punctual lights get clipped at certain angles.
- engine: Fix memory leak when calling `View::setViewport` frequently.
- engine: Fix, materials not working on some Qualcomm devices.
- engine: Modulate emissive by alpha on blended objects.
- engine: Fix, RenderTarget cleared multiple times.
- Java: Fix JNI bindings for color grading.
- Android: reduced binary size.

## v1.9.10

- Introduce `libibl_lite` library.
- engine: Fix `EXC_BAD_INSTRUCTION` seen when using headless SwapChains on macOS with OpenGL.
- engine: Add new callback API to `SwapChain`.
- engine: Fix SwiftShader crash when using an IBL without a reflections texture.
- filamat: Shrink internal `Skybox` material size.
- filamat: improvements to generated material size.
- filamat: silence spirv-opt warnings in release builds.
- matc: Add fog variant filter.
- matc: Fix crash when building mobile materials.
- math: reduce template bloat for matrices.

## v1.9.9

- Vulkan: internal robustness improvements.
- Metal: Support CVPixelBuffer SwapChains.
- Metal: Support copyFrame.
- Fix clear behavior with RenderTarget API.
- Fix GetRefractionMode JNI binding.
- Additional fixes for Fence bug.

## v1.9.8

- Fix a few Fence-related bugs
- gltfio: add createInstance() to AssetLoader.
- gltfio: fix ASAN issue when consuming invalid animation.
- gltfio: do not segfault on invalid primitives.
- gltfio: add safety checks to getAnimator.
- gltfio: fix segfault when consuming invalid file.
- Vulkan: various internal refactoring and improvements
- mathio: add ostream operator for quaternions.
- Fix color grading not applied when dithering is off.

## v1.9.7

- Vulkan: improvements to the ReadPixels implementation.
- Vulkan: warn instead of panic for sampler overflow.
- Vulkan: fix leak with headless swap chain.
- PlatformVkLinux now supports all combos of XLIB and XCB.
- Fix TypeScript binding for TextureUsage.

## v1.9.6

- Added View::setVsmShadowOptions (experimental)
- Add anisotropic shadow map sampling with VSM (experimental)
- matc: fixed bug where some compilation failures still exited with code 0
- Vulkan + Android: fix build break
- Add optional XCB support to PlatformVkLinux
- Fix Vulkan black screen on Windows with NVIDIA hardware

## v1.9.5

- Added a new Live Wallpaper Android sample
- `UiHelper` now supports managing a `SurfaceHolder`
- Fix: an internal texture resource was never destroyed
- Fix: hang on 2-CPU machines
- Fix: Vulkan crash when using shadow cascades
- Linux fixes for headless SwiftShader
- Fix null pointer dereference in `FIndirectLight`
- Fix Windows build by avoiding nested initializers
- Vulkan: support readPixels and headless swap chains
- VSM improvements

## v1.9.4

- Add screen space cone tracing (SSCT)
- Improvements to VSM shadow quality
- New `ShadowOptions` control to render Variance Shadow Maps (VSM) with MSAA (experimental)
- Improvements and fixes to screen-space ambient occlusion
- gltf_viewer: add --headless option
- gltf_viewer: Add new automation UI and functionality

## v1.9.3

- engine: Added new APIs to enable/disable screen space refraction
- engine: Fix, flip the shading normal when det < 0.
- gltfio: Fix animation by clamping the per-channel interpolant.
- gltfio: add async cancellation API
- gltfio: Fix "uniform not found" errors.
- gltfio: Disable clear coat layer IOR change in glTF files (#3104)
- Vulkan: fix final image barrier used for swap chain.
- matdbg: Various improvements
- JavaScript bindings: fix TextureUsage bitmask.
- cmgen / mipgen: add opt-in for ASTC / ETC support.

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
