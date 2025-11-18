# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- engine: add `View::getLastDynamicResolutionScale()` (b/457753622)
- Metal: report GPU errors to the platform via `debugUpdateStat` (b/431665753).
- materials: Make Material Instances' UBO descriptor use dynamic offsets. [⚠️ **Recompile Materials**]
