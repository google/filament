# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- engine: Support up to 4 side-by-side stereoscopic eyes, configurable at Engine creation time. See
  `Engine::Config::stereoscopicEyeCount`. [⚠️ **Recompile Materials**]
- engine: Fix critical GLES 2.0 bugs
