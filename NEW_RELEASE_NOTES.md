# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- `setFrameCompletedCallback` now takes a `backend::CallbackHandler`.
- Fix possible NPE when updating fog options from Java/Kotlin
- The `emissive` property was not applied properly to `MASKED` materials, and could cause
  dark fringes to appear (recompile materials)
- Fix potential `EXC_BAD_ACCESS` with Metal backend: b/297059776
- Allow glTF materials with transmission/volume extensions to choose their alpha mode
  instead of forcing `MASKED`
