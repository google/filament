# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- fog: added an option to disable the fog after a certain distance [⚠️ **Recompile Materials**].
- fog: fog color now takes exposure and IBL intensity into account [⚠️ **Recompile Materials**].
- materials: implement cascades debugging as a post-process [⚠️ **Recompile Materials**].
- materials: use 9 digits or less for floats [⚠️ **Recompile Materials**].
- gltfio: fix skinning when objects are far from the origin
- materials: remove 4 unneeded variants from `unlit` materials [⚠️ **Recompile Materials**].