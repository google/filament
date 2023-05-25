# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- matc: workaround a bug in spirv-tools causing vsm to fail [⚠️ **Recompile materials**]
- webgl: `web` is now a platform just like `desktop` or `mobile`. `matc` must now be invoked with `-p web` for WebGL support.   [⚠️ **Recompile materials**]
