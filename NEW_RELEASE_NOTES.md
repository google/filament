# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- build: add tvOS support (`appletvos`/`appletvsimulator`), Metal-only, via `./build.sh -p tvos`
- utils: `bitset` is no longer over-aligned to 16 bytes, which fixes a crash (SIGBUS) creating
  Vulkan descriptor-set layouts on 32-bit ARM
