# Filament Release Notes log

This file contains one line summaries of commits that are worthy of mentioning in release notes.
A new header is inserted each time a *tag* is created.


# Release notes

- Added specular ambient occlusion to compute a new AO term applied to specular reflections (see `specularAmbientOcclusion` property in materials)
- Added multi-bounce ambient occlusion to brighten AO and preserve local color (see `multiBounceAmbientOcclusion` property in materials)
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
