# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
for next branch cut* header.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- engine: a local transform can now be supplied for each GPU instance [⚠️ **Recompile materials**]
- everything: Add limited support for OpenGL ES 2.0 devices. [⚠️ **Recompile Materials**]
- platform: New virtual on `OpenGLPlatform` to preserve ancillary buffers