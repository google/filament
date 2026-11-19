# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- filagui: fix `ImGuiHelper` drawing nothing with Dear ImGui 1.92.9, which no longer updates the
  obsolete `ImDrawData::CmdListsCount` in `ImGui::Render()`
- gltfio: `AssetLoader::destroyAsset()` now destroys each instance root's transform component
  directly rather than relying on `Engine` garbage collection (#10852)
