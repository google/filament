# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- build: add tvOS support (`appletvos`/`appletvsimulator`), Metal-only, via `./build.sh -p tvos`
- vulkan: with the `backend.vulkan.enable_pipeline_cache_persistence` feature flag, the backend
  keeps its `VkPipelineCache` across runs through `Platform::setBlobFunc`, as the GL backend does
  for program binaries; the blob functions must be set before the `Engine` is created
