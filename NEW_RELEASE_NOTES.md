# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- resgen, bluegl: generated ELF assembly and the BlueGL Linux trampolines now carry a
  `.note.GNU-stack` section, so linking Filament's static libraries with GNU ld no longer marks
  the output as needing an executable stack
- gltfio: `AssetLoader::destroyAsset()` now destroys each instance root's transform component
  directly rather than relying on `Engine` garbage collection (#10852)
