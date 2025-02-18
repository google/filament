# Tint Fuzzers

Tint currently has two fuzzer target executables:
[`tint_wgsl_fuzzer`](#tint_wgsl_fuzzer) which takes WGSL source code
as input and [`tint_ir_fuzzer`](#tint_ir_fuzzer) which takes a
protobuf binary file as input.

Both fuzzers are implemented using
[libFuzzer](https://llvm.org/docs/LibFuzzer.html), and are intended to
be automatically and continuously run by Chromium's ClusterFuzz
infrastructure. Currently only `tint_wgsl_fuzzer` is deployed on
ClusterFuzz.

[Fuzzer targets are automatically found via `gn
refs`](https://chromium.googlesource.com/chromium/src/+/HEAD/testing/libfuzzer/libFuzzer_integration.md). See
[`tint.gni`](../../src/tint/tint.gni) for the core fuzzer target
rules.

Tint's fuzzers are implemented as functions registered with the
macros:

- [`TINT_WGSL_PROGRAM_FUZZER()`](#registering-a-new-tintprogram-fuzzer)
  registers a fuzzer function that is handed a `tint::Program`.
- [`TINT_IR_MODULE_FUZZER()`](#registering-a-new-tintcoreirmodule-fuzzer)
  registers a fuzzer function that is handed a
  `tint::core::ir::Module`.

## Building

The `tint_wgsl_fuzzer` target can be built with either CMake or GN:

- CMake: Define `TINT_BUILD_FUZZERS=1` (pass `-DTINT_BUILD_FUZZERS=1`
  to `cmake`)
- GN: Define `use_libfuzzer = true` in `args.gn`.

Building `tint_ir_fuzzer` requires additional flags to be set:

- CMake: Define `TINT_BUILD_IR_BINARY=1` and `TINT_BUILD_IR_FUZZER=1`
  (pass `-DTINT_BUILD_IR_BINARY=1` `-DTINT_BUILD_IR_FUZZER=1` to
  `cmake`)
- GN: Define `tint_build_ir_binary = true` and `tint_build_ir_fuzzer =
  true` in `args.gn`.

## Running fuzzers

### Local fuzzing

The [`tint_wgsl_fuzzer`](#tint_wgsl_fuzzer) and
[`tint_ir_fuzzer`](#tint_ir_fuzzer) executables accept the [standard
`libFuzzer` command line
arguments](https://llvm.org/docs/LibFuzzer.html#options) with
[extended command line arguments](#extended-command-line-arguments)
described below.

There's also a helper tool to run the fuzzers locally:

- To run the local fuzzers across the full number of CPU threads
  available on the system, seeded with the corpus in
  [`test/tint`](../../test/tint), and using the dictionary in
  `src/tint/cmd/fuzz/wgsl/dictionary.txt` run:

  `tools/run fuzz`

- To check that all the test files in the corpus directory,
  [`test/tint`](../../test/tint) by default, pass the fuzzers without
  error and then exit, run:

  `tools/run fuzz --check`

  Note: This is run by Dawn's CQ presubmit to check that fuzzers
  aren't accidentally broken.

- To run the local fuzzers using the same corpus used by ClusterFuzz:

  `tools/run fuzz -corpus out/libfuzz/gen/fuzzers/wgsl_corpus`

  Note that this corpus directory is generated when building the GN
  target `tint_generate_wgsl_corpus`.

### Generating the corpus

Generate the `tint_wgsl_fuzzer` corpus using the
`tint_generate_wgsl_corpus` GN target, which produces a corpus in
`<build_dir>/gen/fuzzers/wgsl_corpus`. Pass in the path to the corpus
directory as an argument to the fuzzer executable to use it. It's also
a good idea to pass in the dictionary with
`-dict=src/tint/cmd/fuzz/wgsl/dictionary.txt`:

```bash
autoninja -C out/libfuzz tint_generate_wgsl_corpus
out/libfuzz/tint_wgsl_fuzzer.exe -dict=src/tint/cmd/fuzz/wgsl/dictionary.txt out/libfuzz/gen/fuzzers/wgsl_corpus
```

Similarly, the `tint_ir_fuzzer` corpus can be generated using the
`tint_generate_ir_corpus` GN target, producing the corpus in
`<build_dir>/gen/fuzzers/ir_corpus`. For the IR fuzzer, we don't pass
in the dictionary, since we are using
[`libprotobuf-mutator`](https://github.com/google/libprotobuf-mutator)
for mutating (the proto file effectively defines the dictionary):

```bash
autoninja -C out/libfuzz tint_generate_ir_corpus
out/libfuzz/tint_ir_fuzzer.exe out/libfuzz/gen/fuzzers/ir_corpus
```

## Writing fuzzers

### Registering a new `tint::Program` fuzzer

1. Create a new source file with a `_fuzz.cc` suffix.
2. `#include "src/tint/cmd/fuzz/wgsl/fuzz.h"`
3. Define a function in a (possibly nested) anonymous namespace with
   one of the signatures:

   - `void MyFuzzer(const tint::Program& program /*, ...additional fuzzed parameters... */) {`
   - `void MyFuzzer(const tint::Program& program, const tint::fuzz::wgsl::Context& context /*, ...additional fuzzed parameters... */) {`

   The optional `context` parameter holds information about the
    `Program` and the environment used to run the fuzzers.

   [Note: Any number of additional fuzzer-populated parameters can be
   appended to the function signature.](#additional-fuzzer-data)

4. Implement your fuzzer function, using `TINT_ICE()` to catch invalid
   state. Return early if the fuzzer cannot handle the input.
5. At the bottom of the file, in the global namespace, register the
   fuzzer with: `TINT_WGSL_PROGRAM_FUZZER(MyFuzzer);`
6. Use `tools/run gen build` to generate the build files for this new
   fuzzer.

Example:

```c++
#include "src/tint/cmd/fuzz/wgsl/fuzz.h"

namespace tint::my_namespace {
namespace {

bool CanRun(const tint::Program& program) {
    if (program.AST().HasOverrides()) {
        return false;  // Overrides are not supported.
    }
    return true;
}

void MyWGSLFuzzer(const tint::Program& program, bool a_fuzzer_provided_value) {
    if (!CanRun(program)) {
        return;
    }

    // Check something with program.
}

}  // namespace
}  // namespace tint::my_namespace

TINT_WGSL_PROGRAM_FUZZER(tint::my_namespace::MyWGSLFuzzer);
```

### Registering a new `tint::core::ir::Module` fuzzer

1. Create a new source file with a `_fuzz.cc` suffix.
2. `#include "src/tint/cmd/fuzz/ir/fuzz.h"`
3. Define a function in a (possibly nested) anonymous namespace with
   the signature:

   - `void MyFuzzer(core::ir::Module& module /*, ...additional fuzzed parameters... */) {`

   [Note: Any number of additional fuzzer-populated parameters can be
   appended to the function signature.](#additional-fuzzer-data)

4. Implement your fuzzer function, using `TINT_ICE()` to catch invalid
   state. Return early if the fuzzer cannot handle the input.
5. At the bottom of the file, in the global namespace, register the
   fuzzer with: `TINT_IR_MODULE_FUZZER(MyFuzzer);`

  [Note: Often different capabilities need to be enabled before/after
  a fuzzer function runs to ensure validation passes, these can be
  declared using `pre_capabilities` and `post_capabilities` when
  registering the fuzzer ]

6. Use `tools/run gen build` to generate the build files for this new
   fuzzer.

Example:

```c++
#include "src/tint/cmd/fuzz/ir/fuzz.h"

namespace tint::my_namespace {
namespace {

void MyIRFuzzer(core::ir::Module& module) {
    // Do something interesting with module.
}

}  // namespace
}  // namespace tint::my_namespace

TINT_IR_MODULE_FUZZER(tint::my_namespace::MyIRFuzzer);
```

### Additional fuzzer data

WGSL and IR fuzzer functions can also declare any number of additional
parameters, which will be populated with fuzzer provided data. These
additional parameters must come at the end of the signatures described
above, and can be of the following types:

- Any integer, float or bool type.

- Any structure reflected with `TINT_REFLECT`.

  Note: It's recommended to use a `const` reference, for these to
  avoid pass-by-value overheads.

- Any enum reflected with `TINT_REFLECT_ENUM_RANGE`.

## Executable targets

Tint has two fuzzer executable targets:

### `tint_wgsl_fuzzer`

`tint_wgsl_fuzzer` [accepts WGSL textual
input](https://llvm.org/docs/LibFuzzer.html#options) and parses line
comments (`//`) as a base-64 binary encoded data stream for the
[additional fuzzer parameters](additional-fuzzer-data).

The entry point for the fuzzer lives at
[`src/tint/cmd/fuzz/wgsl/main_fuzz.cc`](../../src/tint/cmd/fuzz/wgsl/main_fuzz.cc).

#### Extended command line arguments

On top of the [standard `libFuzzer` command line
arguments](https://llvm.org/docs/LibFuzzer.html#options), the fuzzer
supports the following extended command line arguments:

- `--help`: lists the command line arguments.
- `--filter=<name>`: only runs the fuzzer functions that contain the
  given string in its name.
- `--concurrent`: each of the fuzzer functions will be run on a
  separate, concurrent thread. This potentially offers performance
  improvements, and also tests for concurrent execution.
- `--verbose` : prints verbose information about what the fuzzer is
  doing.
- `--dump` : prints shader source, including input WGSL, and generated
  HLSL, MSL, and GLSL.

#### Behavior

The `tint_wgsl_fuzzer` will do the following:

- Base-64 decode the line comments data from the WGSL source, used to
  populate the [additional fuzzer parameters](additional-fuzzer-data).
- Parse and resolve the WGSL input, and will early-return if there are
  any parser errors.
- Invoke each of the fuzzer functions registered with a call to
  `TINT_WGSL_PROGRAM_FUZZER()`
- Automatically convert the `Program` to an IR module and run the
  function for each function registered with
  `TINT_IR_MODULE_FUZZER()`. Note: The `Program` is converted to an IR
  module for each registered IR fuzzer as the module is mutable.

### `tint_ir_fuzzer`

`tint_ir_fuzzer` accepts binary [protocol
buffer](https://protobuf.dev/) inputs and uses
[`libprotobuf-mutator`](https://github.com/google/libprotobuf-mutator)
for mutating this binary format directly.

The suffix `.tirb` is sometimes used for IR fuzz test case files,
which helps our tooling infer the format. The fuzzer itself does not
generate/depend on this suffix though, instead using the `libFuzzer`
standard prefixes, i.e. `crash-...` and `slow-...`, when needed.

The input protobuf is defined in
[src/tint/utils/protos/ir_fuzz/ir_fuzz.proto](../../src/tint/utils/protos/ir_fuzz/ir_fuzz.proto)
is a composite of two elements, a protobuf
[src/tint/utils/protos/ir/ir.proto](../../src/tint/utils/protos/ir/ir.proto)
that defines the actual IR for the test case, and an opaque binary
blob that includes the [additional fuzzer
parameters](additional-fuzzer-data) akin to `tint_wgsl_fuzzer`.

The entry point for the fuzzer lives at
[`src/tint/cmd/fuzz/ir/main_fuzz.cc`](../../src/tint/cmd/fuzz/ir/main_fuzz.cc).

#### Extended command line arguments

This fuzzer accepts the same set of flags as `tint_wgsl_fuzzer` (both
`libFuzzer` and extended), expect `--dump` which is currently not
supported.

#### Behavior
The `tint_ir_fuzzer` will do the following:

- Decode the binary encoded protobuf into a `Program` for the
  IR module and a binary blob that will be passed into each function
  as options. If the input cannot be decoded due to being invalid or
  containing select constructs that can never occur in real world
  inputs, then the fuzzer will early return.
- Validate that decoded IR module, and will early-return if there are
  any errors.
- For each fuzzer function registered via `TINT_IR_PROGRAM_FUZZER()`,
  make a copy of the IR module, since it is mutable, and then invoke
  the fuzzer function.
- Validate that outputted IR module state, and raise an error if it
  fails, since this indicates a functional issue with a transform or
  the initial validation missed an illegal state.

#### Working with test cases

[Note: The tooling related to working with `tint_ir_fuzz` test cases
is a WIP and may not be complete]

Since the input test cases for `tint_ir_fuzzer` are in a non-human
readable format, there is additional tooling needed for working with
them.

##### ir_fuzz_dis

For displaying the IR contents of a test case file (i.e. `crash-...`,
or `foo.tirb`), there is a disassembler `ir_fuzz_dis`, which is the
easiest way to dump out the contents of a test case. This disassembler
supports a subset of the Tint CLI for outputting in various formats
and to files, etc.

The disassembler has two significant known limitations though.

First, it does not dump out the binary options blob. This is because
this data is interpreted on a per-fuzzer function basis, so there
isn't a general human readable interpretation of its contents. If you
need to understand how the binary blob is being handled by a fuzzer
you will either need to add logging or use a debugger.

The other known limitation is that this disassembler depends on Tint's
IR printing code, which assumes well formed inputs. Sometimes it will
fail to output anything, or more subtly not output values it did't
expect. For example if an instruction only expects 2 params and there
are actually 3 in the IR, then the disassembler may only print the
first two. The validator should catch these issues and give you useful
warnings, but since it is a WIP itself there may be omissions in its
implementation.

##### protoc

Due to the second limitation on the disassembler, there are times
where you will need to dump the raw contents of the binary protocol
buffer in a textual format. This can be done using the tool `protoc`
which is part of `libprotobuf` and either installed via a system
package or built as part building the fuzzers. A full tutorial on
using protoc is beyond the scope of this document, see [protobuf
docs](https://protobuf.dev/getting-started/cpptutorial/) for more
details, but the basic invocation looks like this:

```bash
out/Fuzzer/protoc --decode tint.cmd.fuzz.ir.pb.Root ./src/tint/utils/protos/ir_fuzz/ir_fuzz.proto < ./input.tirb
```

The important thing to note is the use of `--decode
tint.cmd.fuzz.ir.pb.Root` to decode just the IR portion of the input,
but still needing to supply top-level `ir_fuzz.proto` as the format.

##### ir_fuzz_as

For generating binary test cases a rudimentary assembler,
`ir_fuzz_as`, has been implemented.

It is primarily used for converting the Tint test shaders into a seed
corpus for `tint_ir_fuzzer`, but can be used to convert user supplied
shaders into IR tests cases.

Similar to the disassembler it only operates on the shader/IR portion
of the test case format, and does not support embedding/manipulating
the binary blob portion. (There is currently no roundtrip workflow for
taking a test case binary, unpacking it, modifying it, and repacking
it with original blob, other then manually unpacking and packing
protobufs, which is left as an exercise to the reader).

It also suffers from the same limitation of the disassembler with
regards to not handling malformed inputs particularly well, since it
depends on the general Tint CLI parsing/printing infrastructure.

It is capable of dumping raw text of the IR protobuf that it
generates, which can be useful for understanding how the IR is being
encoded/decoded, since you can take a snippet of valid WGSL and dump
out what the protobuf looks like for it.

## Debugging

To debug a specific registered fuzzer function, one strategy is to add
a `TINT_ICE` call at the top of the function, and then run the fuzzer
with `-filter <name>` to have it only run that specific fuzzer. When
the function is called, the libfuzzer harness will emit a crash file
that can be used as input on subsequent runs. Remove the `TINT_ICE`
and run the fuzzer again using this crash file.

For example, if we wish to debug `tint::msl::writer::IRFuzzer`, we
would first insert a `TINT_ICE` at the top:

```c++
Result<SuccessType> IRFuzzer(core::ir::Module& module,
                             const fuzz::ir::Context& context,
                             Options options) {
    TINT_ICE() << "Crash";
    // Comment out the rest of the body to avoid unreachable code warnings
}
```

Build and run the fuzzer, filtering in this function:

```bash
autoninja -C out/libfuzz tint_wgsl_fuzzer
out/libfuzz/tint_wgsl_fuzzer -filter=tint::msl::writer::IRFuzzer
```

It can take a little while before libfuzzer generates a valid input
WGSL, but eventually it will call into the function and crash on the
ICE:

```
...
#71607  NEW    cov: 3633 ft: 8316 corp: 1045/8248b lim: 25 exec/s: 2469 rss: 158Mb L: 8/25 MS: 2 ShuffleBytes-PersAutoDict- DE: "true"-
#71669  NEW    cov: 3633 ft: 8317 corp: 1046/8266b lim: 25 exec/s: 2471 rss: 158Mb L: 18/25 MS: 2 CMP-ChangeByte- DE: "if"-
#71697  REDUCE cov: 3633 ft: 8317 corp: 1046/8264b lim: 25 exec/s: 2472 rss: 158Mb L: 6/25 MS: 3 PersAutoDict-ChangeBit-EraseBytes- DE: "\001\002"-
ICE while running fuzzer: 'tint::msl::writer::IRFuzzer'
..\..\src\tint\lang\msl\writer\writer_fuzz.cc:63 internal compiler error: Crash
==25204== ERROR: libFuzzer: deadly signal
NOTE: libFuzzer has rudimentary signal handlers.
      Combine libFuzzer with AddressSanitizer or similar for better crash reports.
SUMMARY: libFuzzer: deadly signal
MS: 1 PersAutoDict- DE: "or"-; base unit: b34d87c378ebbbfbfd475303dcc75d1d1b2a7c7a
0x2f,0x2f,0x33,0x33,0x33,0x33,0x6f,0x72,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x2a,0x2a,0x30,0x32,
//3333or3333333333**02
artifact_prefix='./'; Test unit written to ./crash-21563a85afd5322d9e17c1c43fd3d4029778d6e7
Base64: Ly8zMzMzb3IzMzMzMzMzMzMzKiowMg==
```

Note that the second to last line specifies that the input test was
written to a file. Now we can remove the `TINT_ICE` and run the fuzzer
with just this file as input:

```bash
out/libfuzz/tint_wgsl_fuzzer ./crash-21563a85afd5322d9e17c1c43fd3d4029778d6e7
```
