# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- materials: fix alpha masked materials when MSAA is turned on [⚠️ **Recompile materials**]
- materials: better support materials with custom depth [**Recompile Materials**]
- engine: fade shadows at shadowFar distance instead of hard cutoff [⚠️ **New Material Version**]
