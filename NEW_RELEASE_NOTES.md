# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- engine: fix PCSS soft shadows of point and spot lights using the directional light's bulb radius
- engine: fix a shader program leak when rendering PCSS or VSM shadows with the program cache
  disabled
- engine: fix cached color grading material instances retaining destroyed FrameGraph textures
