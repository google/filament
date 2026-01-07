# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- gltfio: Add optional support for webp textures (EXT_texture_webp), controlled via FILAMENT_SUPPORTS_WEBP_TEXTURES cmake option
- engine: Support custom attributes morphing, and allow for omitting position and/or normal data. [⚠️ **Recompile Materials**]