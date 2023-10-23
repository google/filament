# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- engine: Added parameter for configuring JobSystem thread count
- engine: In Java, introduce Engine.Builder
- gltfio: fix ubershader index for transmission&volume material
- engine: New tone mapper: `AgXTonemapper`.
- matinfo: Add support for viewing ESSL 1.0 shaders
- engine: Add `Renderer::getClearOptions()` [b/243846268]
- engine: Fix stable shadows (again) when an IBL rotation is used
