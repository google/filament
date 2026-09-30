# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- engine: `TransformManager` now incrementally reorders its components during
  `Renderer::endFrame()` / `Renderer::skipFrame()`. `TransformManager::Instance` values must not
  be kept across frames (or across `create()`, `destroy()`, `setParent()`); keep the `Entity` and
  call `getInstance()` again instead. [⚠️ **API Change**]
- engine: children orphaned by `TransformManager::destroy()` now have their world transform
  updated to their local transform, as documented.
