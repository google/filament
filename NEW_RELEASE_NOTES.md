# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- engine: Allow instantiating Engine at a given feature level via `Engine::Builder::featureLevel`
- engine: Support up to 4 side-by-side stereoscopic eyes, configurable at Engine creation time. See
  `Engine::Config::stereoscopicEyeCount`. [⚠️ **Recompile Materials**]
