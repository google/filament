# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- [⚠️ New Material Version] Convert DYN variant into a specialization constant.

- engine: Optimize Color Grading with NEON on armv8+ devices. Performance improvements between 1.3x and 4.5x
- New `coloredPenumbra` material property can be used to simulate light scattering in shadow 
  transitions. See Filament's material guide for more information
