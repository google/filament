# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
We are chaning the way Vulkan buffers are handled. We need to switch over to a managed (or view-based) model where the data stored inside the object is a proxy to a Vulkan object that can dynamically be swapped around.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- engine: add a `linearFog` material parameter. [⚠️ **New Material Version**]
- opengl: When `Material::compile()` is called on a platform which doesn't support parallel compilation, shaders are automatically compiled over a number of frames
- engine: Added `useDefaultDepthVariant` material parameter to force Filament to use its default variant for
  depth-only passes. [**Requires recompiling materials**]
- material: fix specularFactor in `LOW_QUALITY` mode. [**Requires recompiling materials**] to take effect.
- material: Add CRC32 validation for material packages [⚠️ **New Material Version**]
