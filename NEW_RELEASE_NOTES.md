# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- matc: fix VSM high precision option on mobile [⚠️ **Recompile materials**]
- vulkan: support sRGB swap chain
- Add new `getMaxAutomaticInstances()` API on `Engine` to get max supported automatic instances.
- UiHelper: fix jank when a `TextureView` is resized (fixes b\282220665)
