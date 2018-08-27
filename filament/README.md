# Filament

This package contains several executables and libraries you can use to build applications using
Filament. Latest versions are available on the [project page](https://github.com/google/filament).

## Binaries

- `cmgen`, Image-based lighting asset generator
- `filamesh`, Mesh converter
- `matc`, Material compiler
- `matinfo`, Displays information about materials compiled with `matc`
- `mipgen`, Generates a series of miplevels from a source image.
- `normal-blending`, Tool to blend normal maps
- `roughness-prefilter`, Pre-filters a roughness map from a normal map to reduce aliasing 
- `skygen`, Physically-based sky environment texture generator
- `specular-color`, Computes the specular color of conductors based on spectral data

You can refer to the individual documentation files in `docs/` for more information.

## Libraries

Filament is distributed as a set of static libraries you must link against:

- `bluegl`, Required to render with OpenGL
- `bluevk`, Required to render with Vulkan
- `filabridge`, Support library for Filament
- `filaflat`, Support library for Filament
- `filament`, Main Filament library
- `utils`, Support library for Filament

To use Filament from Java you must use the following two libraries instead:
- `filament-java.jar`, Contains Filament's Java classes 
- `filament-jni`, Filament's JNI bindings

To use the Vulkan backend on macOS you must also make the following libraries available at runtime:
- `MoltenVK_icd.json`
- `libMoltenVK.dylib` 
- `vulkan.1.dylib`
