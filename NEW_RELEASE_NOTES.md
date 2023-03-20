# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- lighting: the sun disc was computed in low/medium quality instead of high quality. This will
  provide performance improvements to mobile devices [⚠️ **Recompile Materials**]
