# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- gltfio: fix crash when compute morph target without material
- matc: fix buggy `variant-filter` flag
- web: Added missing setMat3Parameter()/setMat4Parameter() to MaterialInstance
- opengl: fix b/290670707 : crash when using the blob cache
