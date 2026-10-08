# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- build: add tvOS support (`appletvos`/`appletvsimulator`), Metal-only, via `./build.sh -p tvos`
- engine: on Vulkan, a View rendered into the swap chain after others now clears its viewport
  instead of loading it when `ClearOptions::clear` is set and no earlier View overlaps it
