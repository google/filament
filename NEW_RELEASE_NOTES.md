# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- engine: add support for skinning with more than four bones per vertex.
- engine: remove `BloomOptions::anamorphism` which wasn't working well in most cases [**API CHANGE**] 
- engine: new API to return a Material's supported variants, C++ only (b/297456590)
