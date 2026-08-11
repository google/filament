# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- engine: support multiple directional lights, opt-in via
  `Engine::Config::enableMultipleDirectionalLights`; the dominant one still provides shadows and
  the sun disc, up to 4 additional directional lights are evaluated without shadows [⚠️ **New
  Material Version**]
