# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- gltf: `KHR_materials_iridescence` is now supported
- gltf: `KHR_materials_anisotropy` is now supported
- engine: `Renderer::skipFrame()` takes a new `Renderer::SkipFrameHint` (`IDLE`, `CADENCE`,
  `GPU_CATCH_UP` or `RECLAIM_MEMORY`) that tells Filament why the frame is skipped, so it can keep
  transient caches warm, defer amortized asynchronous work, or free transient memory immediately.
  The hint is also available in Java and JavaScript.

