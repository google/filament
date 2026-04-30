# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- iOS: add Apple silicon (`arm64`) iOS Simulator support. The sample Xcode projects now require Xcode 16+ (CI is pinned to Xcode 16.2).
- WEBGL_PTHREADS renamed to WASM_PTHREADS in CMakeLists.txt
