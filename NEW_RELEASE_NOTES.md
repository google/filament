# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- backend: added 16-bit normalized texture formats (`R16`, `RG16`, `RGB16`, `RGBA16`, and `*_SNORM`).
- webgpu: preserve fragment depth writes and discard in depth-only render passes (#10717).
- gltf: `KHR_materials_iridescence` is now supported
- gltf: `KHR_materials_anisotropy` is now supported
- gltf: `KHR_materials_diffuse_transmission` is now supported
- materials: the new `diffuseTransmission` and `diffuseTransmissionColor` material properties can be
  used in the lit (standard) shading model to create diffusely transmitting (translucent) surfaces
- variant: completely remove FOG variant bit
- android: `Engine.destroySwapChain()` releases the `ANativeWindow` taken from a `Surface`, fixing a
  file descriptor leak. It now waits for the driver thread to run pending commands before returning.
- opengl: key the program binary cache on the final shader source instead of the user material
  code; remove the `MAT_UUID` material chunk. [⚠️ **Recompile Materials**]
- engine: fix `RenderTarget` validation rejecting layers of 2D-array textures at mip levels above 0
  (the layer count was halved per level as for 3D textures)
- metal: map the `SRGB8_ALPHA8_ASTC_*` texture formats to Metal's sRGB ASTC pixel formats on
  Apple-family GPUs (they previously had no Metal mapping)
