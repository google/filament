# libprotobuf-mutator

[![TravisCI Build Status](https://travis-ci.org/google/libprotobuf-mutator.svg?branch=master)](https://travis-ci.org/google/libprotobuf-mutator)
[![Fuzzing Status](https://oss-fuzz-build-logs.storage.googleapis.com/badges/libprotobuf-mutator.svg)](https://oss-fuzz-build-logs.storage.googleapis.com/index.html#libprotobuf-mutator)

## Overview
libprotobuf-mutator is a library to randomly mutate
[protobuffers](https://github.com/google/protobuf). <BR>
It could be used together with guided fuzzing engines, such as [libFuzzer](http://libfuzzer.info).

## Quick start on Debian/Ubuntu

Install prerequisites:

```
sudo apt-get update
sudo apt-get install protobuf-compiler libprotobuf-dev binutils cmake \
  ninja-build liblzma-dev libz-dev pkg-config autoconf libtool
```

Compile and test everything:

```
mkdir build
cd build
cmake .. -GNinja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
ninja check
```

Clang is only needed for libFuzzer integration. <BR>
By default, the system-installed version of
[protobuf](https://github.com/google/protobuf) is used.  However, on some
systems, the system version is too old.  You can pass
`LIB_PROTO_MUTATOR_DOWNLOAD_PROTOBUF=ON` to cmake to automatically download and
build a working version of protobuf.

Installation:

```
ninja
sudo ninja install
```

This installs the headers, pkg-config, and static library.
By default the headers are put in `/usr/local/include/libprotobuf-mutator`.

## Usage

To use libprotobuf-mutator simply include
[mutator.h](/src/mutator.h) and
[mutator.cc](/src/mutator.cc) into your build files.

The `ProtobufMutator` class implements mutations of the protobuf
tree structure and mutations of individual fields.
The field mutation logic is very basic --
for better results you should override the `ProtobufMutator::Mutate*`
methods with more sophisticated logic, e.g.
using [libFuzzer](http://libfuzzer.info)'s mutators.

To apply one mutation to a protobuf object do the following:

```
class MyProtobufMutator : public protobuf_mutator::Mutator {
 public:
  // Optionally redefine the Mutate* methods to perform more sophisticated mutations.
}
void Mutate(MyMessage* message) {
  MyProtobufMutator mutator;
  mutator.Seed(my_random_seed);
  mutator.Mutate(message, 200);
}
```

See also the `ProtobufMutatorMessagesTest.UsageExample` test from
[mutator_test.cc](/src/mutator_test.cc).

## Integrating with libFuzzer
LibFuzzerProtobufMutator can help to integrate with libFuzzer. For example 

```
#include "src/libfuzzer/libfuzzer_macro.h"

DEFINE_PROTO_FUZZER(const MyMessageType& input) {
  // Code which needs to be fuzzed.
  ConsumeMyMessageType(input);
}
```

Please see [libfuzzer_example.cc](/examples/libfuzzer/libfuzzer_example.cc) as an example.

### Mutation post-processing (experimental)
Sometimes it's necessary to keep particular values in some fields without which the proto
is going to be rejected by fuzzed code. E.g. code may expect consistency between some fields
or it may use some fields as checksums. Such constraints are going to be significant bottleneck
for fuzzer even if it's capable of inserting acceptable values with time.

PostProcessorRegistration can be used to avoid such issue and guide your fuzzer towards interesting
code. It registers callback which will be called for each message of particular type after each mutation.

```
static protobuf_mutator::libfuzzer::PostProcessorRegistration<MyMessageType> reg = {
    [](MyMessageType* message, unsigned int seed) {
      TweakMyMessage(message, seed);
    }};

DEFINE_PROTO_FUZZER(const MyMessageType& input) {
  // Code which needs to be fuzzed.
  ConsumeMyMessageType(input);
}
```
Optional: Use seed if callback uses random numbers. It may help later with debugging.

Important: Callbacks should be deterministic and avoid modifying good messages.
Callbacks are called for both: mutator generated and user provided inputs, like
corpus or bug reproducer. So if callback performs unnecessary transformation it
may corrupt the reproducer so it stops triggering the bug.

Note: You can add callback for any nested message and you can add multiple callbacks for
the same message type.
```
static PostProcessorRegistration<MyMessageType> reg1 = {
    [](MyMessageType* message, unsigned int seed) {
      TweakMyMessage(message, seed);
    }};
static PostProcessorRegistration<MyMessageType> reg2 = {
    [](MyMessageType* message, unsigned int seed) {
      DifferentTweakMyMessage(message, seed);
    }};
static PostProcessorRegistration<MyMessageType::Nested> reg_nested = {
    [](MyMessageType::Nested* message, unsigned int seed) {
      TweakMyNestedMessage(message, seed);
    }};

DEFINE_PROTO_FUZZER(const MyMessageType& input) {
  // Code which needs to be fuzzed.
  ConsumeMyMessageType(input);
}
```
## UTF-8 strings
"proto2" and "proto3" handle invalid UTF-8 strings differently. In both cases
string should be UTF-8, however only "proto3" enforces that. So if fuzzer is
applied to "proto2" type libprotobuf-mutator will generate any strings including
invalid UTF-8. If it's a "proto3" message type, only valid UTF-8 will be used.

## Users of the library
* [Chromium](https://cs.chromium.org/search/?q=DEFINE_.*._PROTO_FUZZER%5C\()
* [Envoy](https://github.com/envoyproxy/envoy/search?q=DEFINE_TEXT_PROTO_FUZZER+OR+DEFINE_PROTO_FUZZER+OR+DEFINE_BINARY_PROTO_FUZZER&unscoped_q=DEFINE_TEXT_PROTO_FUZZER+OR+DEFINE_PROTO_FUZZER+OR+DEFINE_BINARY_PROTO_FUZZER&type=Code)
* [LLVM](https://github.com/llvm-mirror/clang/search?q=DEFINE_TEXT_PROTO_FUZZER+OR+DEFINE_PROTO_FUZZER+OR+DEFINE_BINARY_PROTO_FUZZER&unscoped_q=DEFINE_TEXT_PROTO_FUZZER+OR+DEFINE_PROTO_FUZZER+OR+DEFINE_BINARY_PROTO_FUZZER&type=Code)

## Bugs found with help of the library

### Chromium
* [AppCache exploit](http://www.powerofcommunity.net/poc2018/ned.pdf) ([Actual still restricted bug](https://bugs.chromium.org/p/chromium/issues/detail?id=888926))
* [Stack Buffer Overflow in QuicClientPromisedInfo](https://bugs.chromium.org/p/chromium/issues/detail?id=777728)
* [null dereference in sqlite3ExprCompare](https://bugs.chromium.org/p/chromium/issues/detail?id=911251)
### Envoy
* [strftime overflow](https://github.com/envoyproxy/envoy/pull/4321)
* [Heap-use-after-free in Envoy::Upstream::SubsetLoadBalancer::updateFallbackSubset](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=8028)
* [Heap-use-after-free in Envoy::Secret::SecretManagerImpl](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=11231)
* [Heap-buffer-overflow in Envoy::Http::HeaderString](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=10038)

## Related materials
* [Attacking Chrome IPC: Reliably finding bugs to escape the Chrome sandbox](https://media.ccc.de/v/35c3-9579-attacking_chrome_ipc)
* [Structure-aware fuzzing for Clang and LLVM with libprotobuf-mutator](https://www.youtube.com/watch?v=U60hC16HEDY)
* [Structure-Aware Fuzzing with libFuzzer](https://github.com/google/fuzzer-test-suite/blob/master/tutorial/structure-aware-fuzzing.md)
