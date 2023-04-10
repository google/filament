# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- engine: Add support for _constant parameters_, which are constants that can be specialized after material compilation.
- materials: improved size reduction of OpenGL/Metal shaders by ~65% when compiling materials with
             size optimizations (`matc -S`) [⚠️ **Recompile Materials**]
- engine: fix potential crash on Metal devices with A8X GPU (iPad Air 2)
opengl: support the external image on macOS
