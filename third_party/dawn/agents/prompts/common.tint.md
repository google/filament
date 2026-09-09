# Directives for src/tint

Instructions that are useful for development for files under `src/tint`.

## Testing

- When told to run tint end2end tests, or tint e2e tests, unless otherwise
  instructed, run these tests with `tools/run tests --tint {PATH_TO_TINT}`.

## Modifying build files

- When source files are added, deleted, renamed, or moved, do not update
  BUILD.gn, BUILD.cmake, or BUILD.bazel files directly. Instead, run
  `./tools/run gen` to update these files.
