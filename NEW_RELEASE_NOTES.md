# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- fog: fixed fog height falloff and computation precision on mobile [⚠️ **Recompile Materials**]
- materials: new alphaToCoverage property can be used to control alpha to coverage behavior
- materials: added `getUserWorldFromWorldMatrix()` and `getUserWorldPosition()` to retrieve the 
           API-level (user) world position in materials. Deprecated `getWorldOffset()`. [⚠️ **Recompile Materials**]
