# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- build: add tvOS support (`appletvos`/`appletvsimulator`), Metal-only, via `./build.sh -p tvos`
- backend: re-enable the `backend_test` build on iOS (opt-in via `INSTALL_BACKEND_TEST`) and record its golden-image hashes
- vulkan: fix very slow `readPixels` on devices without host-cached staging memory (e.g. PowerVR)
