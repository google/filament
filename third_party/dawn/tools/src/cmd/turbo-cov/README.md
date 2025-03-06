# `turbo-cov`

## About

`turbo-cov` can be used by the `./tools/run cts run-cts` tool, when passing the `--coverage` flag.
`turbo-cov` is substantially faster at processing coverage data than using the standard LLVM tools.

## Requirements

To build `turbo-cov`, you will need to set the CMake define the CMake flag `LLVM_SOURCE_DIR` to the `/llvm` subdirectory of a LLVM checkout. `turbo-cov` requires LLVM 9+.

## Details

[Clang provides two tools](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html#creating-coverage-reports) for processing coverage data:

* `llvm-profdata` indexes the raw `.profraw` coverage profile file and emits a `.profdata` file.
* `llvm-cov` further processes the `.profdata` file into something human readable or machine parsable.

`llvm-cov` provides many options, including emitting an pretty HTML file, but is remarkably slow at producing easily machine-parsable data.
Fortunately the core of `llvm-cov` is [a few hundreds of lines of code](https://github.com/llvm/llvm-project/tree/master/llvm/tools/llvm-cov), as it relies on LLVM libraries to do the heavy lifting.

`turbo-cov` is a a simple `llvm-cov` replacement, which efficiently converts a `.profdata` into a simple binary stream which can be consumed by the `tools/src/cov` package.

## File structure

`turbo-cov` is a trivial binary stream, which takes the tightly-packed form:

```c++
struct Root {
    uint32_t num_files;
    File file[num_files];
};
struct File {
    uint32_t name_length
    uint8_t  name_data[name_length];
    uint32_t num_segments;
    Segment  segments[num_segments];
};
struct Segment {
    // The line where this segment begins.
    uint32_t line;
    // The column where this segment begins.
    uint32_t column;
    // The execution count, or zero if no count was recorded.
    uint32_t count;
    // When 0, the segment was uninstrumented or skipped.
    uint8_t  hasCount;
}
```
