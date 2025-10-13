# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

- filamat: Removed a dependency on Glslang's deprecated SPIR-V remapper.
  The functionality is already implemented by calling the CanonicalizeIds pass
  in the SPIRV-Tools, and should be a non-functional change.

## Release notes for next branch cut
