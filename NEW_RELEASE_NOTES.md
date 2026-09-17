# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- build: add tvOS support (`appletvos`/`appletvsimulator`), Metal-only, via `./build.sh -p tvos`
- vulkan: report depth and stencil render-target format support correctly
