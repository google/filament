# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- backend: async completion callbacks now take a `backend::AsyncCallStatus` argument reporting
  whether the operation ran (`COMPLETED`) or was canceled (`CANCELED`). Callers chaining work from
  a completion callback must check it [⚠️ **API Change**]
- backend: canceling an async call now always invokes its completion callback, with
  `AsyncCallStatus::CANCELED`. Previously the callback was silently dropped
- engine: support multiple directional lights, opt-in via
  `Engine::Config::enableMultipleDirectionalLights`; the dominant one still provides shadows and
  the sun disc, up to 4 additional directional lights are evaluated without shadows [⚠️ **New
  Material Version**]
- materials: a second specular lobe can be defined by setting the `secondRoughness` and
  `secondRoughnessWeight` properties. This can be used to create hazy materials. These properties
  require material to declare `apiLevel : 2` (or greater).
- filamat: completely remove the DYN variant bit.[⚠️ **Recompile Materials**]
