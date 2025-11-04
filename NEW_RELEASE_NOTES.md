# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- Metal: Add support for the `SwapChain::CONFIG_MSAA_4_SAMPLES` flag.
- gltfio: Add optional support for webp textures (EXT_texture_webp)
  - Controlled via FILAMENT_SUPPORTS_WEBP_TEXTURES cmake option
    - Defaults to ON
  - When enabled also builds third_party/libwebp
