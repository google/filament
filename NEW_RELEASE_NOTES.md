# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- materials: Add a new API getParameterTransformName that will return the value of the transformName field of a sampler
parameter. [⚠️ **Recompile Materials**]
- Metal: Add support for the `SwapChain::CONFIG_MSAA_4_SAMPLES` flag.
