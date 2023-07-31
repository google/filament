# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- backend: Disable timer queries on all Mali GPUs (fixes b/233754398)
- engine: Add a way to query the validity of most filament objects (see `Engine::isValid`)
- opengl: fix b/290388359 : possible crash when shutting down the engine
- engine: Improve precision of frame time measurement when using emulated TimerQueries
- backend: Improve frame pacing on Android and Vulkan.
- backend: workaround b/291140208 (gltf_viewer crashes on Nexus 6P)
- engine: support `setDepthFunc` for `MaterialInstance`
- web: Added setDepthFunc()/getDepthFunc() to MaterialInstance
- android: Added setDepthFunc()/getDepthFunc() to MaterialInstance
