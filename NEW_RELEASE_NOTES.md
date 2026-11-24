# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- filagui: fix `ImGuiHelper` drawing nothing with Dear ImGui 1.92.9, which no longer updates the
  obsolete `ImDrawData::CmdListsCount` in `ImGui::Render()`
- resgen, bluegl: generated ELF assembly and the BlueGL Linux trampolines now carry a
  `.note.GNU-stack` section, so linking Filament's static libraries with GNU ld no longer marks
  the output as needing an executable stack
- gltfio: `AssetLoader::destroyAsset()` now destroys each instance root's transform component
  directly rather than relying on `Engine` garbage collection (#10852)
- engine: on Vulkan, a View rendered into the swap chain after others now clears its viewport
  instead of loading it when `ClearOptions::clear` is set and no earlier View overlaps it
