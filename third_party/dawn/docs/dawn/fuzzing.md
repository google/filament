# Fuzzing Dawn

## Building

To enable building of the fuzzers, add `use_libfuzzer = true` to `args.gn`.

The fuzzers can be built using the `fuzzers` target (e.g. `ninja fuzzers`)

## `dawn_wire_server_and_frontend_fuzzer`

The `dawn_wire_server_and_frontend_fuzzer` sets up Dawn using the Null backend, and passes inputs to the wire server. This fuzzes the `dawn_wire` deserialization, as well as Dawn's frontend validation.

## `dawn_wire_server_and_vulkan_backend_fuzzer`

The `dawn_wire_server_and_vulkan_backend_fuzzer` is like `dawn_wire_server_and_frontend_fuzzer` but it runs using a Vulkan CPU backend such as Swiftshader. This fuzzer supports error injection by using the first bytes of the fuzzing input as a Vulkan call index for which to mock a failure.

## Automatic Seed Corpus Generation

Using a seed corpus significantly improves the efficiency of fuzzing. Dawn's fuzzers use interesting testcases discovered in previous fuzzing runs to seed future runs. Fuzzing can be further improved by using Dawn tests as a example of API usage which allows the fuzzer to quickly discover and use new API entrypoints and usage patterns.

Dawn has a CI builder [cron-linux-clang-rel-x64](https://ci.chromium.org/p/dawn/builders/ci/cron-linux-clang-rel-x64) which runs on a periodic schedule. This bot runs the `dawn_end2end_tests` and `dawn_unittests` using the wire and writes out traces of the commands. This can manually be done by running: `<test_binary> --use-wire --wire-trace-dir=tmp_dir`. The output directory will contain one trace for each test, where the traces are prepended with `0xFFFFFFFFFFFFFFFF`. The header is the callsite index at which the error injector should inject an error. If the fuzzer doesn't support error injection it will skip the header. [cron-linux-clang-rel-x64] then hashes the output files to produce unique names and uploads them to the fuzzer corpus directories.
Please see the `dawn.py`[https://source.chromium.org/chromium/chromium/tools/build/+/main:recipes/recipes/dawn.py] recipe for specific details.

Regenerating the seed corpus keeps it up to date when Dawn's API or wire protocol changes.
