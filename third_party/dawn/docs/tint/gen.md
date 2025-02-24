# Code generation

Tint makes use of code generation tooling to emit source code and build files
used to build Tint.

All code is generated with `./tools/run gen`.

The generator uses heavy use of Go's [text/template](https://pkg.go.dev/text/template)
package. This is augmented with the functions declared in [`tools/src/template/template.go`](../../tools/src/template/template.go), and the data types provided by the tooling.

## Build file generation

The generator will scan all the Tint source directories to produce `BUILD.gn`
and `BUILD.cmake` files. These files contain the per-directory targets,
conditionals and inter-target dependency information which is labour-intensive
to maintain by hand.

The bulk of the build generator logic is in the file: [`tools/src/cmd/gen/build/build.go`](../../tools/src/cmd/gen/build/build.go)

### Targets

There are 8 kinds of build target:

* `cmd` targets are executables.
* `lib` targets are libraries used in production code, and can also be used as
  a dependency by any other target kind .
* `test` targets are libraries used by Tint unittests. Must not be used by
  production code.
* `test_cmd` are test executables.
* `bench` targets are libraries used by Tint benchmarks. Must not be used by
  production code.
* `bench_cmd` are benchmark executables.
* `fuzz` targets are libraries used by Tint fuzzers. Must not be used by
  production code.
* `fuzz_cmd` are fuzzer executables.

The build generator uses a file naming convention based on the file name before the extension to classify each source file to a single target kind.

* Source files named `test` or with a `_test` suffix are classed as `test` library targets. \
  Example: `parser_test.cc`.
* Source files named `bench` or with a `_bench` suffix are classed as `bench` library targets. \
  Example: `writer_bench.cc`.
* Source files named `fuzz` or with a `_fuzz` suffix are classed as `fuzz` library targets. \
  Example: `writer_fuzz.cc`.
* Source files with the name `main` are classed as executable targets.
  These typically exist under `src/tint/cmd`. \
  Example: `cmd/tint/main.cc`.
* Source files with the name `main_test` are classed as test executable targets.
  These typically exist under `src/tint/cmd/test`. \
  Example: `cmd/test/main_test.cc`.
* Source files with the name `main_bench` are classed as benchmark executable targets.
  These typically exist under `src/tint/cmd/bench`. \
  Example: `cmd/benchmark/main_bench.cc`.
* Source files with the name `main_fuzz` are classed as fuzzer executable targets.
  These typically exist under `src/tint/cmd/fuzz`. \
  Example: `cmd/benchmark/main_bench.cc`.
* All other files are considered `lib` targets. \
  Example: `parser.cc`.

Each source directory can have at most one `lib`, `test`, `test_main`, `bench`, `bench_main` or `cmd`
target.

The graph of target dependencies must be acyclic (DAG).

Target dependencies are automatically inferred from `#include`s made by the source files.
Additional target dependencies can be added with the use of a [`BUILD.cfg` file](#buildcfg-files).

### External dependencies

All external dependencies must be declared in [`src/tint/externals.json`](../../src/tint/externals.json).

The syntax of this file is:

```json
{
    "external-target-name": {
        "IncludePatterns": [
          /*
            A list of #include path patterns that refer to this external target.
            You may use the '*' wildcard for a single directory, or '**' as a multi-directory wildcard.
          */
          "myexternal/**.h",
        ],
        /* An optional build condition expression to wrap all uses of this external dependency */
        "Condition": "expression",
    },
    /* Repeat the above for all external targets */
}
```

### `GEN_BUILD` directives

Source and build files can be annotated with special directives in comments to control the build file generation.

| Directive | Description |
|-----------|-------------|
| `GEN_BUILD:IGNORE_FILE` | Add to a source file to have the file ignored by the generator <br> Example: `// GEN_BUILD:IGNORE_FILE` |
| `GEN_BUILD:IGNORE_INCLUDE` | Apply to the end of a `#include` in a source file to ignore this include for dependency analysis <br> Example: `#include "foo/bar.h"  // GEN_BUILD:IGNORE_INCLUDE` |
| `GEN_BUILD:CONDITION(`_cond_`)` | Applies the build condition for this source file. <br> Example: `// GEN_BUILD:CONDITION(is_linux)` |
| `GEN_BUILD:DO_NOT_GENERATE` | Prevents the `BUILD.*` file from being generated. <br> Example: `# GEN_BUILD:DO_NOT_GENERATE` |

### `BUILD.cfg` files

A source directory may have an optional `BUILD.cfg` JSON file. The syntax of this file is:

```json
{
  /* Build condition to apply to all targets of this directory. */
  "Condition": "cond",
  /* Options for the 'lib' target */
  "lib": { /* see TargetConfig */ },
  /* Options for the 'test' target */
  "test": { /* see TargetConfig */ },
  /* Options for the 'bench' target */
  "bench": { /* see TargetConfig */ },
  /* Options for the 'cmd' target */
  "cmd": { /* see TargetConfig */ },
}
```

All fields are optional.

The syntax of `TargetConfig` is:

```json
{
  /* An override for the output file name for the target */
  "OutputName": "name",
  /* An additional condition for building this target */
  "Condition": "cond",
  "AdditionalDependencies": {
    "Internal": [
      /*
        A list of target name patterns that should in added as dependencies to this target.
        You may use the '*' wildcard for a single directory, or '**' as a multi-directory wildcard.
      */
    ],
    "External": [
      /*
        A list of external targets that should in added as dependencies to this target.
        Must match an external dependency declared in src/tint/externals.json
      */
    ]
  },
}
```

All fields are optional.

## Templates

The build generator will emit a `BUILD.`_ext_ file in each source directory, for each `BUILD.`_ext_`.tmpl` file found in [`tools/src/cmd/gen/build`](../../tools/src/cmd/gen/build).

The template will be passed the [`Directory`](../../tools/src/cmd/gen/build/directory.go) as `$`.
